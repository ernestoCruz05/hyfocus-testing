# Architecture

**Analysis Date:** 2026-01-08

## Pattern Overview

**Overall:** Plugin Architecture with Modular Component Design

**Key Characteristics:**
- Single shared library plugin for Hyprland window manager
- Modular service components (Timer, Enforcer, Shake, Challenge)
- Event-driven with function hooking for interception
- Callback-based communication between components
- Thread-safe design with atomics and mutexes

## Layers

**Plugin Entry Layer:**
- Purpose: Plugin lifecycle management and configuration
- Contains: `PLUGIN_INIT()`, `PLUGIN_EXIT()`, config loading
- Location: `src/main.cpp`
- Depends on: All service components
- Used by: Hyprland plugin loader

**Command Layer (Dispatchers):**
- Purpose: User-facing command handlers
- Contains: Dispatcher functions for keybinds (start, stop, pause, etc.)
- Location: `src/dispatchers.cpp`, `src/dispatchers.hpp`
- Depends on: Service layer (Timer, Enforcer, Challenge)
- Used by: Hyprland keybind system

**Event Layer (Hooks):**
- Purpose: Intercept and validate Hyprland events
- Contains: Workspace switch hooks, spawn hooks
- Location: `src/eventhooks.cpp`, `src/eventhooks.hpp`
- Depends on: WorkspaceEnforcer, WindowShake
- Used by: Hyprland's function dispatch system

**Service Layer:**
- Purpose: Core business logic components
- Contains: FocusTimer, WorkspaceEnforcer, WindowShake, ExitChallenge
- Location: `src/FocusTimer.cpp/hpp`, `src/WorkspaceEnforcer.cpp/hpp`, `src/WindowShake.cpp/hpp`, `src/ExitChallenge.cpp/hpp`
- Depends on: Globals, Hyprland API
- Used by: Command and Event layers

**State Layer:**
- Purpose: Centralized shared state and utilities
- Contains: Global variables (atomics), helper functions, forward declarations
- Location: `src/globals.hpp`, `src/log.hpp`
- Depends on: Hyprland headers
- Used by: All other layers

## Data Flow

**Workspace Switch Attempt:**

1. User presses keybind (e.g., Super+Right)
2. Hyprland invokes `changeworkspace()` function
3. Our hook `hkChangeworkspace()` intercepts (installed via `HyprlandAPI::createFunctionHook`)
4. Check `g_fe_is_session_active` atomic flag
5. If session active: Query `WorkspaceEnforcer::shouldBlockSwitch(targetId)`
6. Decision branch:
   - ALLOWED: Call original function, update `lastValidWorkspace`
   - BLOCKED: Skip original call, invoke `WindowShake::shake()` for feedback
7. Return to Hyprland

**Focus Session Lifecycle:**

1. User triggers `hyfocus:start 3,5`
2. `dispatch_startSession()` parses workspace list [3, 5]
3. Configure enforcer: `g_fe_enforcer->setAllowedWorkspaces([3, 5])`
4. Configure timer: `g_fe_timer->configure(total, work, break)`
5. Register callbacks (onWorkStart, onBreakStart, onSessionComplete)
6. Start timer: `g_fe_timer->start()`
7. Timer background thread runs `timerLoop()`:
   - Sleeps in 100ms intervals
   - Checks elapsed time vs work/break intervals
   - Invokes callbacks on state transitions
8. Set `g_fe_is_session_active = true`
9. WorkspaceEnforcer now blocks unauthorized switches
10. On completion: `g_fe_is_session_active = false`, notification shown

**State Management:**
- Atomic flags: `g_fe_is_session_active`, `g_fe_is_break_time`, `g_fe_is_shaking`
- Runtime memory only - no persistent storage
- State lost on plugin unload

## Key Abstractions

**FocusTimer:**
- Purpose: Pomodoro-style timer with work/break cycles
- Examples: `src/FocusTimer.cpp`, `src/FocusTimer.hpp`
- Pattern: State machine with background thread
- States: `Idle`, `Working`, `Break`, `Paused`, `Completed`
- Key methods: `start()`, `stop()`, `pause()`, `resume()`, `configure()`

**WorkspaceEnforcer:**
- Purpose: Validate workspace access during focus sessions
- Examples: `src/WorkspaceEnforcer.cpp`, `src/WorkspaceEnforcer.hpp`
- Pattern: Access Control List (ACL) validator
- Key methods: `setAllowedWorkspaces()`, `isWorkspaceAllowed()`, `shouldBlockSwitch()`

**WindowShake:**
- Purpose: Visual feedback animation when switch blocked
- Examples: `src/WindowShake.cpp`, `src/WindowShake.hpp`
- Pattern: Async animation with dedicated thread
- Mechanism: Sinusoidal oscillation with decay
- Key methods: `shake()`, `shakeWindow()`, `configure()`

**ExitChallenge:**
- Purpose: Minigame to prevent impulsive session stops
- Examples: `src/ExitChallenge.cpp`, `src/ExitChallenge.hpp`
- Pattern: Challenge/response system
- Types: None, TypePhrase, MathProblem, Countdown
- Key methods: `initiateChallenge()`, `validateAnswer()`

## Entry Points

**Plugin Entry:**
- Location: `src/main.cpp:191` - `PLUGIN_INIT()`
- Triggers: Plugin loaded by Hyprland
- Responsibilities: Load config, instantiate components, register hooks/dispatchers

**Plugin Exit:**
- Location: `src/main.cpp:289` - `PLUGIN_EXIT()`
- Triggers: Plugin unloaded
- Responsibilities: Stop timer, cleanup components, unregister hooks

**User Commands:**
- Location: `src/dispatchers.cpp`, `src/dispatchers.hpp`
- Triggers: User keybinds (e.g., `hyfocus:start`, `hyfocus:stop`)
- Responsibilities: Parse input, call service methods, show notifications

**Event Hooks:**
- Location: `src/eventhooks.cpp`, `src/eventhooks.hpp`
- Triggers: Hyprland events (workspace switch, window spawn)
- Responsibilities: Intercept, validate, allow/block

## Error Handling

**Strategy:** Return early with notification on errors, exceptions caught at boundaries

**Patterns:**
- Guard clauses with early return and user notification
- Atomic flag checks before operations
- Try/catch at hook boundaries (with `catch(...)` - see CONCERNS.md)
- Validation before action (workspace ID parsing, session state)

## Cross-Cutting Concerns

**Logging:**
- Custom macros: `FE_DEBUG()`, `FE_INFO()`, `FE_WARN()`, `FE_ERR()` (`src/log.hpp`)
- Output to Hyprland's logging system

**Thread Safety:**
- Atomic flags for session state: `std::atomic<bool>` in `src/globals.hpp`
- Mutex protection in FocusTimer, WorkspaceEnforcer, WindowShake
- Condition variables for thread coordination

**Notifications:**
- Helper functions: `showNotification()`, `showError()`, `showWarning()` (`src/globals.hpp`)
- Color-coded via `HyprlandAPI::addNotification()`

---

*Architecture analysis: 2026-01-08*
*Update when major patterns change*
