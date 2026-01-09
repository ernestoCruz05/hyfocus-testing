# Roadmap: HyFocus Stability

## Overview

A focused stability milestone to eliminate crash risks in HyFocus before adding new features. Addresses null pointer dereferences, input validation gaps, exception handling, memory management, and version compatibility. The goal is a plugin that never crashes Hyprland, even in edge cases.

## Domain Expertise

None

## Phases

**Phase Numbering:**
- Integer phases (1, 2, 3): Planned milestone work
- Decimal phases (2.1, 2.2): Urgent insertions (marked with INSERTED)

- [ ] **Phase 1: Null Safety** - Fix all null pointer dereference risks
- [ ] **Phase 2: Input Validation** - Add bounds checking and workspace ID validation
- [ ] **Phase 3: Exception Handling** - Replace catch(...) with specific types
- [ ] **Phase 4: Memory Safety** - Switch to smart pointers
- [ ] **Phase 5: Version Check** - Add Hyprland compatibility verification

## Phase Details

### Phase 1: Null Safety
**Goal**: Eliminate all null pointer dereference risks in Desktop::focusState(), g_pCompositor, and g_pHyprRenderer access
**Depends on**: Nothing (first phase)
**Research**: Unlikely (internal code fixes using standard null checks)
**Plans**: TBD

Key files:
- `src/WindowShake.cpp` (line 37)
- `src/eventhooks.cpp` (lines 87, 98)
- `src/dispatchers.cpp` (lines 69, 88)

### Phase 2: Input Validation
**Goal**: Add bounds checking for string operations and validate workspace IDs
**Depends on**: Phase 1
**Research**: Unlikely (standard bounds checking patterns)
**Plans**: TBD

Key files:
- `src/eventhooks.cpp` (line 84): args[0] without empty check
- `src/dispatchers.cpp`: workspace ID validation

### Phase 3: Exception Handling
**Goal**: Replace overly broad catch(...) with specific exception types for better debugging
**Depends on**: Phase 2
**Research**: Unlikely (C++ standard exception types)
**Plans**: TBD

Key files:
- `src/eventhooks.cpp` (lines 79, 92, ~155): catch(...) blocks

### Phase 4: Memory Safety
**Goal**: Switch from raw pointers to std::unique_ptr for plugin components
**Depends on**: Phase 3
**Research**: Unlikely (standard C++ patterns)
**Plans**: TBD

Key files:
- `src/main.cpp` (lines 235-238): g_fe_timer, g_fe_enforcer, g_fe_shaker, g_fe_exitChallenge
- `src/globals.hpp`: pointer declarations

### Phase 5: Version Check
**Goal**: Add Hyprland version compatibility check to prevent loading on unsupported versions
**Depends on**: Phase 4
**Research**: Likely (need Hyprland API for version detection)
**Research topics**: Hyprland plugin API version checking, PLUGIN_API_VERSION usage, version comparison patterns
**Plans**: TBD

Key files:
- `src/main.cpp`: PLUGIN_INIT function

## Progress

**Execution Order:**
Phases execute in numeric order: 1 → 2 → 3 → 4 → 5

| Phase | Plans Complete | Status | Completed |
|-------|----------------|--------|-----------|
| 1. Null Safety | 1/1 | Complete | 2026-01-08 |
| 2. Input Validation | 1/1 | Complete | 2026-01-08 |
| 3. Exception Handling | 1/1 | Complete | 2026-01-08 |
| 4. Memory Safety | 1/1 | Complete | 2026-01-08 |
| 5. Version Check | 1/1 | Complete | 2026-01-08 |
