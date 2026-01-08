# External Integrations

**Analysis Date:** 2026-01-08

## APIs & External Services

**Payment Processing:**
- Not applicable - This is a desktop plugin, no payment integration

**Email/SMS:**
- Not applicable - No email or messaging services

**External APIs:**
- Not detected - No external API calls
- No HTTP/REST clients
- No network operations

## Data Storage

**Databases:**
- Not detected - No database integration
- Runtime memory only

**File Storage:**
- Not detected - No file persistence
- No configuration files written
- State lost on plugin unload

**Caching:**
- Not detected - No caching layer
- In-memory state only (atomic variables)

## Authentication & Identity

**Auth Provider:**
- Not applicable - System plugin, no user authentication

**OAuth Integrations:**
- Not applicable

## System-Level Integrations

**Hyprland Window Manager (Required):**
- Deep integration with Hyprland internals
- SDK/Client: Hyprland development headers
- Access method: Direct function calls via plugin API

**Integration Points:**

1. **Plugin API** (`src/globals.hpp`, `src/main.cpp`)
   - `HyprlandAPI::registerPlugin()` - Plugin registration
   - `HyprlandAPI::addConfigValue()` - Configuration registration
   - `HyprlandAPI::getConfigValue()` - Configuration retrieval
   - `HyprlandAPI::addDispatcher()` - Command registration
   - `HyprlandAPI::addNotification()` - User notifications
   - `HyprlandAPI::createFunctionHook()` - Function interception

2. **Compositor Access** (`src/eventhooks.cpp`, `src/globals.hpp`)
   - `g_pCompositor` - Global compositor instance
   - `g_pCompositor->m_workspaces` - Workspace list access
   - `Desktop::focusState()->monitor()` - Current monitor
   - `Desktop::focusState()->window()` - Focused window

3. **Renderer Access** (`src/WindowShake.cpp`)
   - `g_pHyprRenderer->damageWindow()` - Mark window for redraw

4. **Keybind System** (`src/dispatchers.cpp`)
   - Dispatcher functions registered as keybind handlers
   - Format: `hyfocus:command` (e.g., `hyfocus:start`, `hyfocus:stop`)

## Monitoring & Observability

**Error Tracking:**
- Not detected - No external error tracking service

**Analytics:**
- Not detected - No analytics or telemetry

**Logs:**
- Hyprland's logging system via custom macros (`src/log.hpp`)
- Levels: DEBUG, INFO, WARN, ERR
- Output destination: Hyprland's log output

## CI/CD & Deployment

**Hosting:**
- Not applicable - Local plugin installation

**Distribution:**
- Hyprland Plugin Manager (hyprpm)
- Configuration: `hyprpm.toml`
- Build commands specified in manifest

**Installation:**
```bash
hyprpm add https://github.com/ernestoCruz05/hyfocus
hyprpm enable hyfocus
```

**CI Pipeline:**
- Not detected - No GitHub Actions or CI configuration

## Environment Configuration

**Development:**
- Required: Hyprland development headers
- Required: C++23 compiler (GCC 13+, Clang 17+)
- Required: CMake 3.19+ or Meson
- No environment variables needed

**Staging:**
- Not applicable - No staging environment

**Production:**
- Plugin loaded by Hyprland at runtime
- Configuration in user's `hyprland.conf`
- No secrets or credentials required

## Configuration Integration

**Hyprland Config File:**
- Configuration keys: `plugin:hyfocus:*`
- Location: User's `hyprland.conf`
- Loaded via: `HyprlandAPI::getConfigValue()` in `src/main.cpp`

**Configuration Options:**
```conf
plugin:hyfocus {
    total_duration = 50          # Total session length (minutes)
    work_interval = 25           # Work period length (minutes)
    break_interval = 5           # Break period length (minutes)
    enforce_during_break = false # Block switches during breaks
    shake_intensity = 20         # Shake animation pixels
    shake_duration = 300         # Shake animation ms
    shake_frequency = 50         # Shake oscillation period
    exception_classes = eww,rofi # Floating windows to allow
    block_spawn = false          # Block new app launches
    spawn_whitelist = alacritty  # Apps allowed during focus
    exit_challenge_type = none   # none/type_phrase/math/countdown
    exit_challenge_phrase = ...  # Custom phrase for type challenge
}
```

## Webhooks & Callbacks

**Incoming:**
- Not applicable - No webhook endpoints

**Outgoing:**
- Not applicable - No external webhook calls

**Internal Callbacks:**
- Timer callbacks: `setOnWorkStart()`, `setOnBreakStart()`, `setOnSessionComplete()`
- Registered in `src/dispatchers.cpp` during session start
- Invoked from timer background thread

## Event Interception

**Function Hooks Installed:**
1. `changeworkspace` hook (`src/eventhooks.cpp`)
   - Intercepts workspace switch commands
   - Validates against enforcer
   - Blocks or allows based on session state

2. `spawn` hook (`src/eventhooks.cpp`)
   - Intercepts new window creation (when `block_spawn = true`)
   - Validates against whitelist
   - Blocks non-whitelisted apps during focus

---

*Integration audit: 2026-01-08*
*Update when adding/removing external services*
