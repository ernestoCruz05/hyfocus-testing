# HyFocus

## What This Is

A Hyprland plugin for pomodoro-style focus sessions with workspace enforcement. When active, it locks users to designated workspaces, shakes windows on blocked switch attempts, and optionally requires completing a challenge to stop early. Helps users stay productive by adding intentional friction to distractions.

## Core Value

The plugin must never crash Hyprland, even in edge cases. Stability is the foundation — everything else depends on it.

## Requirements

### Validated

- ✓ Pomodoro timer with configurable work/break intervals — existing
- ✓ Workspace enforcement (lock to allowed workspaces during focus) — existing
- ✓ Window shake animation for visual feedback on blocked switches — existing
- ✓ Exit challenge minigames (type phrase, math problem, countdown) — existing
- ✓ App spawn blocking with whitelist support — existing
- ✓ Exception classes for floating widgets (eww, rofi) — existing
- ✓ Configuration via hyprland.conf (`plugin:hyfocus:*`) — existing
- ✓ User commands (start, stop, pause, resume, toggle, status) — existing

### Active

- [ ] Fix null pointer risks in Desktop::focusState() calls
- [ ] Fix null pointer risks in g_pCompositor access
- [ ] Fix null pointer risks in g_pHyprRenderer access
- [ ] Add bounds check for string indexing (args[0])
- [ ] Replace catch(...) with specific exception types
- [ ] Add input validation for workspace IDs (>= 1)
- [ ] Switch to std::unique_ptr for plugin components
- [ ] Add Hyprland version compatibility check

### Out of Scope

- New features — focus purely on stability for this milestone
- Persistent state/session resume — deferred to future milestone
- Statistics/progress tracking — deferred to future milestone
- Test suite — nice to have but not blocking stability fixes

## Context

**Current state:** v0.1.0 prototype. Core functionality works but has stability risks identified in codebase analysis. The plugin hooks into Hyprland's internal APIs using `#define private public` hack, creating brittle coupling.

**Codebase:** ~2,500 lines of C++23 across 7 source files. Modular architecture with FocusTimer, WorkspaceEnforcer, WindowShake, and ExitChallenge components. Thread-safe design using atomics and mutexes.

**Known fragile areas:**
- `src/WindowShake.cpp` (line 37): Desktop::focusState()->window() without null check
- `src/eventhooks.cpp` (lines 84, 87, 98): String bounds and null pointer risks
- `src/dispatchers.cpp` (lines 69, 88): Desktop::focusState()->monitor() without null check
- `src/main.cpp` (lines 235-238): Raw pointer allocation

## Constraints

- **Hyprland version**: v53.0+ only — no backwards compatibility needed
- **API stability**: Plugin uses undocumented Hyprland internals that may change between versions
- **Build system**: CMake 3.19+ or Meson, C++23 compiler required

## Key Decisions

| Decision | Rationale | Outcome |
|----------|-----------|---------|
| Stability before features | Crashes undermine trust and usability | — Pending |
| Hyprland v53.0+ only | Simplifies maintenance, no legacy support burden | — Pending |
| Keep current user API | Existing config and commands work, no user disruption | — Pending |

---
*Last updated: 2026-01-08 after initialization*
