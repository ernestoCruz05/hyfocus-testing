/**
 * @file eventhooks.hpp
 * @brief Event hook declarations for workspace interception.
 */
#pragma once

#include "globals.hpp"
#include <vector>
#include <string>

/**
 * @brief Register all event hooks for workspace enforcement.
 * 
 * This creates the hooks but does NOT enable them. Hooks are only
 * enabled when a focus session starts via enableEnforcementHooks().
 * 
 * @param errors Vector to collect any error messages during registration
 */
void registerEventHooks(std::vector<std::string>& errors);

/**
 * @brief Enable the enforcement hooks (called when session starts).
 */
void enableEnforcementHooks();

/**
 * @brief Disable the enforcement hooks (called when session stops).
 */
void disableEnforcementHooks();

/**
 * @brief Clean up and unregister all event hooks.
 */
void unregisterEventHooks();
