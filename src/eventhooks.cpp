/**
 * @file eventhooks.cpp
 * @brief Event hook implementations for workspace interception.
 * 
 * This file contains the core event interception logic that monitors
 * workspace switch attempts and blocks/reverts them when focus enforcement
 * is active. It hooks into Hyprland's changeworkspace function to intercept
 * and validate workspace switches before they occur.
 * 
 * ## Event Interception Flow
 * 
 * When a user attempts to switch workspaces (via keybind, mouse, etc.):
 * 
 * 1. Hyprland calls the `changeworkspace` function
 * 2. Our hook (`hkChangeworkspace`) is invoked INSTEAD
 * 3. We check if focus enforcement is active via `WorkspaceEnforcer`
 * 4. If the target workspace is ALLOWED:
 *    - Call the original function to complete the switch
 *    - Update `lastValidWorkspace` for potential future reverts
 * 5. If the target workspace is BLOCKED:
 *    - Do NOT call the original function (switch is prevented)
 *    - Trigger the window shake animation for visual feedback
 *    - Log the blocked attempt
 * 
 * ## Window Shake Logic
 * 
 * When a switch is blocked, we shake the focused window to provide
 * immediate visual feedback. This works by:
 * 
 * 1. Getting the currently focused window from Hyprland's state
 * 2. Passing it to `WindowShake::shake()` which runs asynchronously
 * 3. The shake animation temporarily oscillates the window position
 * 4. After animation completes, the window returns to its original position
 * 
 * The animation is non-blocking - the event hook returns immediately
 * while the shake continues on a background thread.
 */
#include "eventhooks.hpp"
#include "dispatchers.hpp"
#include "FocusTimer.hpp"
#include "WorkspaceEnforcer.hpp"
#include "WindowShake.hpp"

// Original function type for changeworkspace
typedef void (*origChangeworkspace)(std::string args);
typedef void (*origSpawn)(std::string args);

// Store the original function pointer after hooking
static void* g_origChangeworkspace = nullptr;
static void* g_origSpawn = nullptr;

/**
 * @brief Hook function that intercepts workspace change requests.
 * 
 * This function is called instead of Hyprland's original changeworkspace.
 * It validates the switch against the enforcer and either allows it
 * (by calling the original) or blocks it (by returning early + shaking).
 * 
 * @param args The workspace argument string (e.g., "3", "+1", "name:browser")
 */
static void hkChangeworkspace(std::string args) {
    // If no session active, always allow
    if (!g_fe_is_session_active.load()) {
        if (g_origChangeworkspace) {
            (*(origChangeworkspace)g_origChangeworkspace)(args);
        }
        return;
    }
    
    // Parse the target workspace ID from args
    // This handles various formats: "3", "+1", "-1", "name:xxx", etc.
    WORKSPACEID targetId = 0;
    bool validTarget = false;
    
    // Try to parse as direct ID first
    try {
        targetId = std::stoi(args);
        validTarget = true;
    } catch (...) {
        // Not a simple number, try other formats
    }
    
    // Handle relative workspace notation (+1, -1)
    if (!validTarget && (args[0] == '+' || args[0] == '-')) {
        try {
            int offset = std::stoi(args);
            auto pMonitor = Desktop::focusState()->monitor();
            if (pMonitor && pMonitor->m_activeWorkspace) {
                targetId = pMonitor->m_activeWorkspace->m_id + offset;
                validTarget = true;
            }
        } catch (...) {}
    }
    
    // Handle workspace by name
    if (!validTarget && args.find("name:") == 0) {
        std::string name = args.substr(5);
        for (auto& ws : g_pCompositor->m_workspaces) {
            if (ws->m_name == name) {
                targetId = ws->m_id;
                validTarget = true;
                break;
            }
        }
    }
    
    // If we couldn't parse the target, allow the switch (safety fallback)
    if (!validTarget) {
        FE_DEBUG("Could not parse workspace target '{}', allowing switch", args);
        if (g_origChangeworkspace) {
            (*(origChangeworkspace)g_origChangeworkspace)(args);
        }
        return;
    }
    
    // Check with the enforcer if this switch should be blocked
    if (g_fe_enforcer && g_fe_enforcer->shouldBlockSwitch(targetId)) {
        // BLOCKED! Trigger visual feedback
        FE_INFO("Blocked workspace switch to {} (args: '{}')", targetId, args);
        
        // Trigger shake animation on focused window
        if (g_fe_shaker) {
            g_fe_shaker->shake();
        }
        
        // Show a subtle notification
        showWarning("Focus mode: Workspace " + std::to_string(targetId) + " is restricted!");
        
        // Do NOT call the original function - the switch is prevented
        return;
    }
    
    // Switch is allowed - call the original function
    if (g_origChangeworkspace) {
        (*(origChangeworkspace)g_origChangeworkspace)(args);
        
        // Update last valid workspace
        if (g_fe_enforcer) {
            g_fe_enforcer->setLastValidWorkspace(targetId);
        }
    }
}

/**
 * @brief Callback for workspace change events (post-change notification).
 * 
 * This callback is called AFTER a workspace change has occurred.
 * We use it to track the current valid workspace state.
 */
static void onWorkspaceChange(void* self, SCallbackInfo& info, std::any data) {
    (void)self;
    (void)info;
    
    try {
        auto pWorkspace = std::any_cast<PHLWORKSPACE>(data);
        if (pWorkspace && g_fe_enforcer) {
            // If the workspace is in our allowed list, update last valid
            if (g_fe_enforcer->isWorkspaceAllowed(pWorkspace->m_id)) {
                g_fe_enforcer->setLastValidWorkspace(pWorkspace->m_id);
                FE_DEBUG("Updated last valid workspace to {}", pWorkspace->m_id);
            }
        }
    } catch (...) {
        FE_DEBUG("Could not process workspace change event");
    }
}

/**
 * @brief Hook function that intercepts spawn (app launch) requests.
 * 
 * When focus enforcement is active and spawn blocking is enabled,
 * this prevents launching new applications to keep the user focused.
 * 
 * ## Spawn Blocking Logic
 * 
 * 1. If no session is active, allow all spawns
 * 2. If blocking is disabled (g_fe_block_spawn = false), allow all spawns
 * 3. If during break and enforcement during break is disabled, allow spawns
 * 4. Check if the command is in the spawn whitelist (e.g., "firefox", "kitty")
 * 5. If not whitelisted, block the spawn and show visual feedback
 * 
 * The whitelist checks if any whitelisted app name is contained in the
 * spawn command, allowing for flexibility (e.g., "firefox" matches
 * "firefox --new-window https://example.com").
 * 
 * @param args The spawn command string
 */
static void hkSpawn(std::string args) {
    // If no session active, always allow
    if (!g_fe_is_session_active.load()) {
        if (g_origSpawn) {
            (*(origSpawn)g_origSpawn)(args);
        }
        return;
    }
    
    // Check if spawn blocking is enabled
    if (!g_fe_block_spawn) {
        if (g_origSpawn) {
            (*(origSpawn)g_origSpawn)(args);
        }
        return;
    }
    
    // During breaks, check if enforcement is active
    if (g_fe_is_break_time.load() && !g_fe_enforce_during_break) {
        if (g_origSpawn) {
            (*(origSpawn)g_origSpawn)(args);
        }
        return;
    }
    
    // Check whitelist - see if any whitelisted app is in the command
    std::string argsLower = args;
    std::transform(argsLower.begin(), argsLower.end(), argsLower.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    
    for (const auto& allowed : g_fe_spawn_whitelist) {
        std::string allowedLower = allowed;
        std::transform(allowedLower.begin(), allowedLower.end(), allowedLower.begin(),
                       [](unsigned char c) { return std::tolower(c); });
        
        if (argsLower.find(allowedLower) != std::string::npos) {
            // Whitelisted app found, allow spawn
            FE_DEBUG("Spawn allowed (whitelisted): {}", args);
            if (g_origSpawn) {
                (*(origSpawn)g_origSpawn)(args);
            }
            return;
        }
    }
    
    // BLOCKED! Trigger visual feedback
    FE_INFO("Blocked spawn: {}", args);
    
    // Trigger shake animation
    if (g_fe_shaker) {
        g_fe_shaker->shake();
    }
    
    showWarning("Focus mode: App launching is blocked!");
    
    // Do NOT call the original function - spawn is prevented
}

/**
 * @brief Safely create a function hook with error handling.
 */
template<typename T>
static CFunctionHook* createHookSafe(
    const char* name,
    T memberFunc,
    void* hookFunc,
    std::vector<std::string>& errors
) {
    auto hook = HyprlandAPI::createFunctionHook(PHANDLE, memberFunc, hookFunc);
    if (!hook) {
        errors.push_back(std::string("Failed to create hook for ") + name);
        return nullptr;
    }
    if (!hook->hook()) {
        errors.push_back(std::string("Failed to enable hook for ") + name);
        return nullptr;
    }
    FE_DEBUG("Successfully hooked {}", name);
    return hook;
}

void registerEventHooks(std::vector<std::string>& errors) {
    FE_INFO("Registering event hooks...");
    
    // Hook the changeworkspace function to intercept workspace switches
    // We need to find it by name since it's a dispatcher function
    static const auto changeworkspaceMethods = HyprlandAPI::findFunctionsByName(PHANDLE, "changeworkspace");
    
    if (!changeworkspaceMethods.empty()) {
        g_fe_pChangeworkspaceHook = HyprlandAPI::createFunctionHook(
            PHANDLE,
            changeworkspaceMethods[0].address,
            (void*)&hkChangeworkspace
        );
        
        if (g_fe_pChangeworkspaceHook) {
            // Store original function pointer before enabling
            g_origChangeworkspace = g_fe_pChangeworkspaceHook->m_original;
            
            if (g_fe_pChangeworkspaceHook->hook()) {
                FE_INFO("Successfully hooked changeworkspace");
            } else {
                errors.push_back("Failed to enable changeworkspace hook");
            }
        } else {
            errors.push_back("Failed to create changeworkspace hook");
        }
    } else {
        errors.push_back("Could not find changeworkspace function");
    }
    
    // Hook the spawn function to block app launching during focus
    static const auto spawnMethods = HyprlandAPI::findFunctionsByName(PHANDLE, "spawn");
    
    if (!spawnMethods.empty()) {
        g_fe_pSpawnHook = HyprlandAPI::createFunctionHook(
            PHANDLE,
            spawnMethods[0].address,
            (void*)&hkSpawn
        );
        
        if (g_fe_pSpawnHook) {
            g_origSpawn = g_fe_pSpawnHook->m_original;
            
            if (g_fe_pSpawnHook->hook()) {
                FE_INFO("Successfully hooked spawn");
            } else {
                errors.push_back("Failed to enable spawn hook");
            }
        } else {
            errors.push_back("Failed to create spawn hook");
        }
    } else {
        errors.push_back("Could not find spawn function");
    }
    
    // Register callback for post-workspace-change events
    static auto workspaceCallback = HyprlandAPI::registerCallbackDynamic(
        PHANDLE,
        "workspace",
        onWorkspaceChange
    );
    
    if (!workspaceCallback) {
        errors.push_back("Failed to register workspace callback");
    }
    
    FE_INFO("Event hook registration complete ({} errors)", errors.size());
}

void unregisterEventHooks() {
    FE_INFO("Unregistering event hooks...");
    
    if (g_fe_pChangeworkspaceHook) {
        g_fe_pChangeworkspaceHook->unhook();
        g_fe_pChangeworkspaceHook = nullptr;
    }
    
    if (g_fe_pSpawnHook) {
        g_fe_pSpawnHook->unhook();
        g_fe_pSpawnHook = nullptr;
    }
    
    g_origChangeworkspace = nullptr;
    g_origSpawn = nullptr;
    
    FE_INFO("Event hooks unregistered");
}
