# Codebase Concerns

**Analysis Date:** 2026-01-08

## Tech Debt

**Raw pointer allocation for plugin components:**
- Issue: Using `new` without smart pointers for component instantiation
- Files: `src/main.cpp` (lines 235-238)
- Why: Quick implementation during development
- Impact: If PLUGIN_INIT throws after partial allocation, memory leaks
- Fix approach: Use `std::unique_ptr` for `g_fe_timer`, `g_fe_enforcer`, `g_fe_shaker`, `g_fe_exitChallenge`

**Private member access hack:**
- Issue: `#define private public` to access Hyprland internals
- Files: `src/globals.hpp` (line 26)
- Why: Needed access to internal compositor state not exposed via API
- Impact: Brittle coupling to Hyprland internals, breaks on internal changes
- Fix approach: Request public API additions upstream or use safer accessor patterns

## Known Bugs

**No known bugs at this time.**

The codebase is at v0.1.0 (prototype stage) - bugs may exist but none are documented.

## Security Considerations

**No input sanitization on workspace IDs:**
- Risk: Negative or invalid workspace IDs could be added to allowed list
- Files: `src/dispatchers.cpp` (lines 214-216)
- Current mitigation: None
- Recommendations: Validate workspace ID >= 1 before adding to enforcer

**Overly permissive exception classes:**
- Risk: Malicious apps could be named to match exception classes
- Files: `src/main.cpp` (config loading for `exception_classes`)
- Current mitigation: None (user configures list)
- Recommendations: Document security implications in README

## Performance Bottlenecks

**No significant performance concerns detected.**

The plugin:
- Uses efficient atomic operations for state checks
- Timer thread sleeps in 100ms intervals (minimal CPU usage)
- Shake animation uses minimal compositor calls
- No blocking operations on main thread

## Fragile Areas

**Desktop::focusState() calls without null checks:**
- Why fragile: Returns pointer that could be null during shutdown or edge cases
- Files:
  - `src/WindowShake.cpp` (line 37): `Desktop::focusState()->window()`
  - `src/eventhooks.cpp` (line 87): `Desktop::focusState()->monitor()`
  - `src/dispatchers.cpp` (lines 69, 88): `Desktop::focusState()->monitor()`
- Common failures: Crash if called during Hyprland shutdown
- Safe modification: Add null check on `focusState()` return before dereferencing
- Test coverage: None

**Global compositor access:**
- Why fragile: Direct access to `g_pCompositor->m_workspaces` without null check
- Files: `src/eventhooks.cpp` (line 98)
- Common failures: Crash if compositor unavailable
- Safe modification: Add `if (g_pCompositor)` guard
- Test coverage: None

**Exception handling with catch(...):**
- Why fragile: Silently swallows all exceptions including programming errors
- Files:
  - `src/eventhooks.cpp` (lines 79, 92, ~155)
- Common failures: Masks real bugs, makes debugging difficult
- Safe modification: Catch specific exceptions (`std::bad_any_cast`, `std::invalid_argument`)
- Test coverage: None

## Scaling Limits

**Not applicable** - This is a local desktop plugin, not a server application.

## Dependencies at Risk

**Hyprland internal API dependency:**
- Risk: Plugin uses undocumented internals that may change
- Impact: Plugin breaks on Hyprland updates
- Migration plan: Monitor Hyprland releases, test against new versions

**No version pinning:**
- Risk: Building against any Hyprland version without compatibility checks
- Impact: Silent failures or crashes on API changes
- Migration plan: Add Hyprland version check in PLUGIN_INIT

## Missing Critical Features

**No persistent state:**
- Problem: Session state lost on plugin unload or Hyprland restart
- Current workaround: Users restart sessions manually
- Blocks: Can't resume interrupted focus sessions
- Implementation complexity: Low (save state to file)

**No progress tracking:**
- Problem: No way to see completed sessions/statistics
- Current workaround: Users track manually
- Blocks: Can't build habit tracking or analytics
- Implementation complexity: Medium (add state file + display commands)

## Test Coverage Gaps

**No test suite exists:**
- What's not tested: Everything
- Risk: Regressions undetected, refactoring dangerous
- Priority: High
- Difficulty to test: Medium (need to mock Hyprland APIs)

**Critical untested paths:**
1. Timer state transitions (Working → Break → Working)
2. Workspace parsing edge cases (negative IDs, invalid formats)
3. Exit challenge answer validation
4. Thread cleanup during plugin unload
5. Null pointer scenarios for Desktop::focusState()

## Additional Concerns

**String indexing without bounds check:**
- Issue: `args[0]` accessed without checking if `args` is empty
- Files: `src/eventhooks.cpp` (line 84)
- Risk: Undefined behavior / crash on empty string
- Fix: Add `!args.empty() &&` before check

**Renderer null check missing:**
- Issue: `g_pHyprRenderer->damageWindow()` called without null check
- Files: `src/WindowShake.cpp` (line 152)
- Risk: Crash if renderer unavailable
- Fix: Add `if (g_pHyprRenderer)` guard

**Thread safety in callbacks:**
- Issue: Timer callbacks access globals from background thread
- Files: `src/dispatchers.cpp` (lines 97-111), `src/FocusTimer.cpp` (timerLoop)
- Risk: Race conditions if dispatcher called during callback
- Impact: Likely low (atomics used for flags) but not provably safe
- Fix: Document thread safety guarantees or add mutex protection

---

*Concerns audit: 2026-01-08*
*Update as issues are fixed or new ones discovered*
