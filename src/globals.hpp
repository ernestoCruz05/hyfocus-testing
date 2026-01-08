/**
 * @file globals.hpp
 * @brief Global declarations for the HyFocus Hyprland plugin.
 * 
 * This file contains all global state variables, configuration pointers,
 * and shared resources used throughout the plugin. The plugin implements
 * a Pomodoro-style focus enforcement system that restricts workspace
 * switching during active focus sessions.
 */
#pragma once

#include <hyprland/src/includes.hpp>
#include <any>

// Include std headers BEFORE the private->public hack to avoid template issues
#include <atomic>
#include <chrono>
#include <functional>
#include <memory>
#include <mutex>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#define private public
#include <hyprland/src/Compositor.hpp>
#include <hyprland/src/plugins/PluginAPI.hpp>
#include <hyprland/src/managers/KeybindManager.hpp>
#include <hyprland/src/managers/LayoutManager.hpp>
#include <hyprland/src/managers/EventManager.hpp>
#include <hyprland/src/managers/HookSystemManager.hpp>
#include <hyprland/src/render/Renderer.hpp>
#include <hyprland/src/devices/Keyboard.hpp>
#include <hyprland/src/devices/IPointer.hpp>
#include <hyprland/src/desktop/state/FocusState.hpp>
#include <hyprland/src/helpers/Monitor.hpp>
#include <hyprutils/string/String.hpp>
#undef private

#include "log.hpp"

// Forward declarations
class FocusTimer;
class WorkspaceEnforcer;
class WindowShake;
class ExitChallenge;

// ============================================================================
// Plugin Handle
// ============================================================================
inline HANDLE PHANDLE = nullptr;

// ============================================================================
// Timer Configuration (in minutes)
// ============================================================================
inline int g_fe_total_duration = 120;      // Total session duration (default: 2 hours)
inline int g_fe_work_interval = 25;        // Work interval (default: 25 minutes)
inline int g_fe_break_interval = 5;        // Break interval (default: 5 minutes)

// ============================================================================
// Workspace Enforcement
// ============================================================================
inline std::vector<WORKSPACEID> g_fe_allowed_workspaces;     // List of allowed workspace IDs
inline std::set<std::string> g_fe_exception_classes;          // Window classes exempt from enforcement
inline bool g_fe_enforce_during_break = false;                // Whether to enforce during breaks

// ============================================================================
// App Spawn Blocking
// ============================================================================
inline bool g_fe_block_spawn = true;                          // Block launching new apps during focus
inline std::set<std::string> g_fe_spawn_whitelist;            // Apps allowed to launch during focus

// ============================================================================
// Exit Challenge (Minigame)
// ============================================================================
inline int g_fe_exit_challenge_type = 0;                      // 0=none, 1=phrase, 2=math, 3=countdown
inline std::string g_fe_exit_challenge_phrase = "I want to stop focusing";

// ============================================================================
// Animation/Visual Feedback
// ============================================================================
inline int g_fe_shake_intensity = 15;      // Pixels to shake
inline int g_fe_shake_duration = 300;      // Shake duration in ms
inline int g_fe_shake_frequency = 50;      // Shake oscillation frequency in ms

// ============================================================================
// State Variables
// ============================================================================
inline std::atomic<bool> g_fe_is_session_active{false};       // Is a focus session running?
inline std::atomic<bool> g_fe_is_break_time{false};           // Are we in a break period?
inline std::atomic<bool> g_fe_is_shaking{false};              // Is shake animation in progress?
inline WORKSPACEID g_fe_last_valid_workspace = 1;             // Last allowed workspace

// ============================================================================
// Shared Resources (thread-safe)
// Use raw pointers with forward declarations; ownership managed in main.cpp
// ============================================================================
inline FocusTimer* g_fe_timer = nullptr;
inline WorkspaceEnforcer* g_fe_enforcer = nullptr;
inline WindowShake* g_fe_shaker = nullptr;
inline ExitChallenge* g_fe_exitChallenge = nullptr;
inline std::mutex g_fe_mutex;  // Protects shared state during concurrent access

// ============================================================================
// Function Hooks
// ============================================================================
// NOTE: We no longer hook changeworkspace - we use a callback-based revert strategy
// which is more stable. Only spawn hook remains for blocking app launches.
inline CFunctionHook* g_fe_pSpawnHook = nullptr;

// ============================================================================
// Utility Macros
// ============================================================================
inline void showNotification(const std::string& msg, CHyprColor color = {0.2, 0.8, 0.2, 1.0}, uint64_t timeMs = 3000) {
    HyprlandAPI::addNotification(PHANDLE, "[hyfocus] " + msg, color, timeMs);
}

inline void showError(const std::string& msg) {
    showNotification(msg, {1.0, 0.2, 0.2, 1.0}, 5000);
}

inline void showWarning(const std::string& msg) {
    showNotification(msg, {1.0, 0.7, 0.0, 1.0}, 4000);
}
