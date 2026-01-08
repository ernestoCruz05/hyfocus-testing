/**
 * @file dispatchers.hpp
 * @brief Dispatcher function declarations for HyFocus commands.
 * 
 * Dispatchers are the interface between Hyprland's keybind system and
 * the plugin's functionality. Each dispatcher handles a specific command
 * that can be triggered via keybinds or hyprctl.
 */
#pragma once

#include "globals.hpp"
#include "FocusTimer.hpp"
#include "WorkspaceEnforcer.hpp"
#include "WindowShake.hpp"
#include "ExitChallenge.hpp"

/**
 * @brief Start a focus session.
 * 
 * Format: "hyfocus:start [workspaces]"
 * Example: "hyfocus:start 3,5" - Allow only workspaces 3 and 5
 * 
 * @param args Comma-separated list of allowed workspace IDs
 */
void dispatch_startSession(std::string args);

/**
 * @brief Stop the current focus session.
 * 
 * Format: "hyfocus:stop"
 */
void dispatch_stopSession(std::string args);

/**
 * @brief Pause the focus timer.
 * 
 * Format: "hyfocus:pause"
 */
void dispatch_pauseSession(std::string args);

/**
 * @brief Resume a paused focus session.
 * 
 * Format: "hyfocus:resume"
 */
void dispatch_resumeSession(std::string args);

/**
 * @brief Toggle the focus session (start if stopped, stop if running).
 * 
 * Format: "hyfocus:toggle [workspaces]"
 * 
 * @param args Comma-separated list of allowed workspace IDs (for starting)
 */
void dispatch_toggleSession(std::string args);

/**
 * @brief Add a workspace to the allowed list.
 * 
 * Format: "hyfocus:allow <workspace_id>"
 * 
 * @param args Single workspace ID to add
 */
void dispatch_allowWorkspace(std::string args);

/**
 * @brief Remove a workspace from the allowed list.
 * 
 * Format: "hyfocus:disallow <workspace_id>"
 * 
 * @param args Single workspace ID to remove
 */
void dispatch_disallowWorkspace(std::string args);

/**
 * @brief Add a window class to the exception list.
 * 
 * Format: "hyfocus:except <window_class>"
 * Example: "hyfocus:except eww" - Allow EWW widgets to remain interactive
 * 
 * @param args Window class name to exempt
 */
void dispatch_addException(std::string args);

/**
 * @brief Show the current session status.
 * 
 * Format: "hyfocus:status"
 * 
 * Displays remaining time, current state, and allowed workspaces.
 */
void dispatch_showStatus(std::string args);

/**
 * @brief Confirm/submit answer for exit challenge.
 * 
 * Format: "hyfocus:confirm <answer>"
 * 
 * Used to submit answers for the exit minigame when stopping.
 * 
 * @param args The answer to the challenge
 */
void dispatch_confirmStop(std::string args);

/**
 * @brief Add an app to the spawn whitelist.
 * 
 * Format: "hyfocus:allowapp <app_command>"
 * Example: "hyfocus:allowapp firefox" - Allow launching firefox during focus
 * 
 * @param args App command/name to whitelist
 */
void dispatch_allowApp(std::string args);

/**
 * @brief Remove an app from the spawn whitelist.
 * 
 * Format: "hyfocus:disallowapp <app_command>"
 * 
 * @param args App command/name to remove from whitelist
 */
void dispatch_disallowApp(std::string args);

/**
 * @brief Register all dispatchers with Hyprland.
 * 
 * This function is called during plugin initialization to register
 * all the dispatcher functions with the keybind manager.
 */
void registerDispatchers();
