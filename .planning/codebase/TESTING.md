# Testing Patterns

**Analysis Date:** 2026-01-08

## Test Framework

**Runner:**
- Not detected - No test framework configured

**Assertion Library:**
- Not detected

**Run Commands:**
```bash
# No test commands available
# Build commands only:
cmake -B build && cmake --build build  # CMake
meson setup builddir && ninja -C builddir  # Meson
```

## Test File Organization

**Location:**
- No test files found
- No `tests/`, `test/`, or `__tests__/` directories
- No `*.test.cpp` or `*.spec.cpp` files

**Naming:**
- Not applicable

**Structure:**
```
# Current (no tests):
src/
  FocusTimer.cpp
  FocusTimer.hpp
  ...

# Suggested (if tests added):
src/
  FocusTimer.cpp
  FocusTimer.hpp
tests/
  FocusTimer.test.cpp
  WorkspaceEnforcer.test.cpp
```

## Test Structure

**Suite Organization:**
- Not applicable (no tests)

**Suggested Pattern (if implementing with Google Test):**
```cpp
#include <gtest/gtest.h>
#include "FocusTimer.hpp"

class FocusTimerTest : public ::testing::Test {
protected:
    void SetUp() override {
        timer = std::make_unique<FocusTimer>();
    }

    std::unique_ptr<FocusTimer> timer;
};

TEST_F(FocusTimerTest, ConfiguresCorrectly) {
    timer->configure(25, 5, 5);
    EXPECT_EQ(timer->getState(), TimerState::Idle);
}

TEST_F(FocusTimerTest, StartsFromIdle) {
    timer->configure(25, 5, 5);
    EXPECT_TRUE(timer->start());
    EXPECT_EQ(timer->getState(), TimerState::Working);
}
```

## Mocking

**Framework:**
- Not applicable (no testing framework)

**Patterns:**
- Not applicable

**What Would Need Mocking:**
- Hyprland API calls (`HyprlandAPI::*`, `g_pCompositor`, `g_pHyprRenderer`)
- `Desktop::focusState()` returns
- System time for timer tests
- Thread operations for animation tests

## Fixtures and Factories

**Test Data:**
- Not applicable

**If Implementing:**
```cpp
// Factory for test workspaces
std::vector<WORKSPACEID> createTestWorkspaceList(std::initializer_list<int> ids) {
    return std::vector<WORKSPACEID>(ids);
}

// Fixture for timer state
struct TimerFixture {
    static FocusTimer createConfigured() {
        FocusTimer t;
        t.configure(25, 5, 5);
        return t;
    }
};
```

## Coverage

**Requirements:**
- No coverage requirements
- No coverage tooling configured

**Configuration:**
- Not applicable

**View Coverage:**
```bash
# Not available - would need to add:
# cmake -DCMAKE_CXX_FLAGS="--coverage" -B build
# lcov / gcov for coverage reports
```

## Test Types

**Unit Tests:**
- Not implemented
- Candidates: `FocusTimer`, `WorkspaceEnforcer`, `ExitChallenge` classes

**Integration Tests:**
- Not implemented
- Would require Hyprland test harness or mocking

**E2E Tests:**
- Not implemented
- Would require running Hyprland instance

## Code Testability Assessment

**Testable Components (with mocking):**

1. **FocusTimer** (`src/FocusTimer.cpp/hpp`)
   - State machine transitions
   - Interval calculations
   - Pause/resume logic
   - Callback invocation
   - Thread safety

2. **WorkspaceEnforcer** (`src/WorkspaceEnforcer.cpp/hpp`)
   - ACL validation (`isWorkspaceAllowed()`)
   - Whitelist management
   - Exception class handling

3. **ExitChallenge** (`src/ExitChallenge.cpp/hpp`)
   - Answer validation for each challenge type
   - Math problem generation
   - Countdown state management

4. **Parser Functions** (`src/dispatchers.cpp`)
   - `parseWorkspaceList()` - workspace ID parsing
   - `formatTime()` - time formatting

**Hard to Test (Hyprland dependencies):**

- `src/eventhooks.cpp` - Hook functions require Hyprland runtime
- `src/WindowShake.cpp` - Requires compositor for window manipulation
- `src/main.cpp` - Plugin lifecycle tied to Hyprland

## Suggested Testing Approach

**If adding tests:**

1. **Choose framework:** Google Test (gtest) is common for C++ projects
2. **Add to CMakeLists.txt:**
   ```cmake
   enable_testing()
   find_package(GTest REQUIRED)
   add_executable(tests tests/FocusTimer.test.cpp tests/WorkspaceEnforcer.test.cpp)
   target_link_libraries(tests GTest::GTest GTest::Main)
   add_test(NAME unit_tests COMMAND tests)
   ```
3. **Start with pure logic:** Test `WorkspaceEnforcer`, `ExitChallenge` first
4. **Mock Hyprland:** Create interfaces for API calls to enable testing

---

*Testing analysis: 2026-01-08*
*Update when test infrastructure changes*
