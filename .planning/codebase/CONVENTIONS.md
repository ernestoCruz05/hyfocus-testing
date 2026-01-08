# Coding Conventions

**Analysis Date:** 2026-01-08

## Naming Patterns

**Files:**
- PascalCase for class files: `FocusTimer.cpp`, `WorkspaceEnforcer.hpp`, `WindowShake.cpp`
- lowercase for utility/shared files: `main.cpp`, `dispatchers.cpp`, `globals.hpp`, `log.hpp`
- `.cpp` for implementation, `.hpp` for headers

**Functions:**
- camelCase for class methods: `startSession()`, `setAllowedWorkspaces()`, `isWorkspaceAllowed()`
- snake_case not used
- Dispatcher prefix: `dispatch_*` for user command handlers (`dispatch_startSession()`, `dispatch_stopSession()`)
- Hook prefix: `hk*` for intercepted functions (`hkChangeworkspace()`, `hkSpawn()`)

**Variables:**
- camelCase for local variables: `targetId`, `allowedWorkspaces`, `validTarget`
- Global prefix: `g_fe_` (focus enforcement) for all globals: `g_fe_is_session_active`, `g_fe_timer`, `g_fe_enforcer`
- Private member prefix: `m_` for class members: `m_state`, `m_mutex`, `m_shouldStop`, `m_intensity`
- No underscore suffix pattern

**Types:**
- PascalCase for classes: `FocusTimer`, `WorkspaceEnforcer`, `WindowShake`, `ExitChallenge`
- PascalCase for enums: `TimerState`, `ChallengeType`
- PascalCase for enum values: `Idle`, `Working`, `Break`, `Paused`, `Completed`

## Code Style

**Formatting:**
- 4-space indentation
- Braces on same line as control structures
- No explicit formatter tool (.clang-format not present)
- Line length: No strict limit observed

**Quotes & Separators:**
- Double quotes for strings
- Semicolons at end of statements (standard C++)

**Linting:**
- Compiler warnings: `-Wall -Wextra` enabled in `CMakeLists.txt`
- Warning level 2 in Meson: `'warning_level=2'`
- No clang-tidy or static analyzer configured

## Import Organization

**Order:**
1. Own header file (for .cpp files): `#include "FocusTimer.hpp"`
2. Project headers: `#include "globals.hpp"`, `#include "dispatchers.hpp"`
3. System/external headers: `<atomic>`, `<chrono>`, `<hyprland/src/...>`

**Grouping:**
- Blank lines between groups optional
- No strict sorting within groups

**Include Guards:**
- `#pragma once` for all headers (no traditional ifndef guards)

## Error Handling

**Patterns:**
- Guard clauses with early return
- User notifications for recoverable errors: `showError("message")`
- Try/catch at boundaries (hook functions)
- Atomic flag checks before operations

**Error Types:**
- Return void with notification for user errors
- Silent fallback for non-critical parsing failures
- Exceptions caught with `catch(...)` (should be more specific - see CONCERNS.md)

**Logging:**
- Use logging macros for debugging: `FE_DEBUG()`, `FE_INFO()`, `FE_WARN()`, `FE_ERR()`

## Logging

**Framework:**
- Custom macros in `src/log.hpp`
- Levels: DEBUG, INFO, WARN, ERR (maps to Hyprland's logger)

**Patterns:**
- Format string with arguments: `FE_INFO("Timer configured: {}min total", totalMinutes)`
- Log at state transitions and important events
- Debug level for detailed tracing

**Macros:**
```cpp
FE_DEBUG("message {}", arg)  // Debug level
FE_INFO("message {}", arg)   // Info level
FE_WARN("message {}", arg)   // Warning level
FE_ERR("message {}", arg)    // Error level
```

## Comments

**When to Comment:**
- File-level Doxygen headers with `@file`, `@brief`
- Class and function documentation with `@param`, `@return`
- Inline comments for non-obvious logic
- ASCII diagrams for visual concepts (see `src/WindowShake.hpp`)

**Doxygen Style:**
```cpp
/**
 * @file FocusTimer.hpp
 * @brief Pomodoro-style timer for focus sessions.
 */

/**
 * @brief Start the focus timer.
 * @return true if started successfully, false if already running
 */
bool start();
```

**TODO Comments:**
- No specific format observed
- Not frequently used in codebase

## Function Design

**Size:**
- Most functions under 50 lines
- Larger functions exist but are well-commented (e.g., `hkChangeworkspace` ~80 lines)

**Parameters:**
- Use const references for strings: `const std::string& args`
- No strict max parameter count

**Return Values:**
- void for commands/actions with side effects
- bool for operations that can fail
- Explicit returns

## Module Design

**Exports:**
- Headers expose public class interface
- Implementation details in .cpp files
- Forward declarations in `globals.hpp`

**Class Structure:**
- Public methods first
- Private methods second
- Private members last (with `m_` prefix)

**Thread Safety:**
- Classes managing threads have mutex members
- Atomic flags for shared state
- Document thread safety in comments (could be improved)

---

*Convention analysis: 2026-01-08*
*Update when patterns change*
