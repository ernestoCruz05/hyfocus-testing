/**
 * @file WorkspaceEnforcer.hpp
 * @brief Workspace restriction logic for focus sessions.
 * 
 * The WorkspaceEnforcer class manages the list of allowed workspaces
 * and determines whether a workspace switch should be permitted or blocked.
 * It also handles exception rules for floating widgets and specific window classes.
 */
#pragma once

#include "globals.hpp"
#include <algorithm>
#include <mutex>
#include <set>
#include <vector>

/**
 * @class WorkspaceEnforcer
 * @brief Manages workspace access control during focus sessions.
 * 
 * This class provides thread-safe methods to configure allowed workspaces,
 * check if a switch should be blocked, and manage exception lists for
 * specific window classes (e.g., EWW widgets for media control).
 */
class WorkspaceEnforcer {
public:
    WorkspaceEnforcer() = default;
    ~WorkspaceEnforcer() = default;

    /**
     * @brief Set the list of allowed workspace IDs.
     * @param workspaceIds Vector of workspace IDs that are permitted during focus
     */
    void setAllowedWorkspaces(const std::vector<WORKSPACEID>& workspaceIds);

    /**
     * @brief Add a single workspace to the allowed list.
     * @param workspaceId Workspace ID to allow
     */
    void addAllowedWorkspace(WORKSPACEID workspaceId);

    /**
     * @brief Remove a workspace from the allowed list.
     * @param workspaceId Workspace ID to remove
     */
    void removeAllowedWorkspace(WORKSPACEID workspaceId);

    /**
     * @brief Clear all allowed workspaces.
     */
    void clearAllowedWorkspaces();

    /**
     * @brief Get the list of currently allowed workspaces.
     * @return Vector of allowed workspace IDs
     */
    std::vector<WORKSPACEID> getAllowedWorkspaces() const;

    /**
     * @brief Check if a workspace is in the allowed list.
     * @param workspaceId Workspace ID to check
     * @return true if workspace is allowed
     */
    bool isWorkspaceAllowed(WORKSPACEID workspaceId) const;

    /**
     * @brief Add a window class to the exception list.
     * 
     * Windows with these classes will remain interactive even when
     * workspace enforcement is active. Useful for floating widgets.
     * 
     * @param windowClass The window class string (case-sensitive)
     */
    void addExceptionClass(const std::string& windowClass);

    /**
     * @brief Remove a window class from the exception list.
     * @param windowClass The window class to remove
     */
    void removeExceptionClass(const std::string& windowClass);

    /**
     * @brief Clear all exception classes.
     */
    void clearExceptionClasses();

    /**
     * @brief Check if a window class is in the exception list.
     * @param windowClass The window class to check
     * @return true if the window class is exempt from enforcement
     */
    bool isWindowClassExempt(const std::string& windowClass) const;

    /**
     * @brief Check if a window is exempt from enforcement.
     * 
     * A window is exempt if:
     * - Its class is in the exception list
     * - It's a floating window (configurable)
     * - It's on a special workspace
     * 
     * @param pWindow Pointer to the window to check
     * @return true if window is exempt
     */
    bool isWindowExempt(PHLWINDOW pWindow) const;

    /**
     * @brief Validate a workspace switch attempt.
     * 
     * This is the main method called when intercepting workspace changes.
     * It determines if the switch should be allowed based on:
     * - Whether a focus session is active
     * - Whether it's break time (and enforcement during breaks is disabled)
     * - Whether the target workspace is in the allowed list
     * 
     * @param targetWorkspaceId The workspace the user is trying to switch to
     * @return true if the switch should be BLOCKED
     */
    bool shouldBlockSwitch(WORKSPACEID targetWorkspaceId) const;

    /**
     * @brief Get the last known valid workspace.
     * @return The workspace ID to revert to when blocking a switch
     */
    WORKSPACEID getLastValidWorkspace() const { return m_lastValidWorkspace; }

    /**
     * @brief Update the last valid workspace.
     * @param workspaceId The current valid workspace
     */
    void setLastValidWorkspace(WORKSPACEID workspaceId) { m_lastValidWorkspace = workspaceId; }

    /**
     * @brief Set whether floating windows are always exempt.
     * @param exempt true to always allow floating windows
     */
    void setFloatingExempt(bool exempt) { m_floatingExempt = exempt; }

    /**
     * @brief Set whether to enforce during break intervals.
     * @param enforce true to also enforce during breaks
     */
    void setEnforceDuringBreak(bool enforce) { m_enforceDuringBreak = enforce; }

private:
    mutable std::mutex m_mutex;
    std::set<WORKSPACEID> m_allowedWorkspaces;
    std::set<std::string> m_exceptionClasses;
    WORKSPACEID m_lastValidWorkspace{1};
    bool m_floatingExempt{true};
    bool m_enforceDuringBreak{false};
};
