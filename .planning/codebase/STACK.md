# Technology Stack

**Analysis Date:** 2026-01-08

## Languages

**Primary:**
- C++ (C++23) - All application code
  - `CMakeLists.txt` (line 8): `set(CMAKE_CXX_STANDARD 23)`
  - `meson.build` (line 4): `'cpp_std=c++23'`

**Secondary:**
- None (pure C++ project)

## Runtime

**Environment:**
- Linux/Unix with Hyprland window manager (Wayland compositor)
- Hyprland Plugin API - Required runtime dependency
- No Node.js/Python/interpreter (compiled native plugin)

**Package Manager:**
- None (native C++ project with no language-specific package manager)
- Dependencies managed via system libraries and Hyprland headers
- No lockfiles

## Frameworks

**Core:**
- Hyprland Plugin API - Window manager integration
  - `src/globals.hpp` (lines 27-38): Core Hyprland headers
  - `hyprland/src/Compositor.hpp`
  - `hyprland/src/plugins/PluginAPI.hpp`
  - `hyprland/src/managers/KeybindManager.hpp`
  - `hyprland/src/managers/EventManager.hpp`
  - `hyprland/src/managers/HookSystemManager.hpp`
- Hyprutils - String utilities
  - `src/globals.hpp` (line 38): `hyprutils/string/String.hpp`

**Testing:**
- Not detected - No test framework configured

**Build/Dev:**
- CMake 3.19+ - Primary build system (`CMakeLists.txt`)
- Meson - Alternative build system (`meson.build`)
- hyprpm - Hyprland Plugin Manager for distribution (`hyprpm.toml`)

## Key Dependencies

**Critical:**
- Hyprland development headers - Plugin API, compositor access
- C++ Standard Library - Threading, chrono, containers

**Standard Library Features Used:**
- `<atomic>` - Thread-safe state flags (`src/globals.hpp`)
- `<chrono>` - Timer and duration management (`src/FocusTimer.hpp`)
- `<thread>` - Background timer/animation threads (`src/FocusTimer.cpp`, `src/WindowShake.cpp`)
- `<mutex>`, `<condition_variable>` - Thread synchronization
- `<random>` - Math challenge generation (`src/ExitChallenge.hpp`)
- `<functional>` - Callbacks
- `<set>`, `<vector>`, `<string>` - Data structures

**Infrastructure:**
- No external package dependencies
- System C++ runtime only

## Configuration

**Environment:**
- No environment variables required
- No `.env` files (not applicable for system plugin)
- Configuration via Hyprland config file (`hyprland.conf`)

**Build:**
- `CMakeLists.txt` - CMake build configuration
- `meson.build` - Meson build configuration
- `hyprpm.toml` - Plugin manifest for hyprpm distribution

**Runtime Configuration:**
- `plugin:hyfocus:*` keys in Hyprland config
- Configuration loaded via `HyprlandAPI::getConfigValue()` in `src/main.cpp` (lines 72-154)
- Settings include: timer durations, shake animation params, exception classes

## Platform Requirements

**Development:**
- Linux (Wayland required for Hyprland)
- Hyprland development headers installed
- CMake 3.19+ or Meson
- C++23 capable compiler (GCC 13+, Clang 17+)

**Production:**
- Linux with Hyprland window manager running
- Distributed as shared library (`libhyfocus.so`)
- Installed to `/usr/lib/hyprland/plugins/` via hyprpm
- No runtime dependencies beyond system libraries

## Version Information

- HyFocus v0.1.0 - `CMakeLists.txt` (line 3), `meson.build` (line 2), `hyprpm.toml` (line 9)
- Author: faky (github.com/ernestoCruz05)
- License: MIT

---

*Stack analysis: 2026-01-08*
*Update after major dependency changes*
