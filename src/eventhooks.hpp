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
 * @param errors Vector to collect any error messages during registration
 */
void registerEventHooks(std::vector<std::string>& errors);

/**
 * @brief Clean up and unregister all event hooks.
 */
void unregisterEventHooks();
