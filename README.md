# HyFocus

A Hyprland plugin that implements Pomodoro-style focus enforcement with workspace locking and app blocking.

## Overview

HyFocus helps you stay productive by restricting workspace access and blocking app launches during focus sessions. When a timer is running, attempting to switch to non-whitelisted workspaces or launching new apps triggers a visual "shake" animation and blocks the action.

## Features

- **Pomodoro Timer**: Configurable work/break intervals (default: 25/5 minutes)
- **Workspace Locking**: Whitelist specific workspaces during focus sessions
- **App Blocking**: Prevent launching new applications during focus sessions
- **Exit Challenge**: Optional minigame to discourage stopping sessions early
- **Visual Feedback**: Window shake animation when attempting restricted actions
- **Pause/Resume**: Pause your session without losing progress
- **Flexible Configuration**: All settings configurable via `hyprland.conf`
- **Exception Classes**: Floating widgets (EWW, rofi, etc.) remain accessible

## Installation

### Using hyprpm (Recommended)

```bash
hyprpm add https://github.com/ernestoCruz05/hyfocus
hyprpm enable hyfocus
```

### Manual Build

```bash
git clone https://github.com/ernestoCruz05/hyfocus
cd hyfocus

# Using CMake
cmake -DCMAKE_BUILD_TYPE=Release -B build
cmake --build build
sudo cp build/libhyfocus.so /usr/lib/hyprland/plugins/

# Or using Meson
meson setup build --buildtype=release
ninja -C build
sudo cp build/libhyfocus.so /usr/lib/hyprland/plugins/
```

## Configuration

Add to your `hyprland.conf`:

```bash
# Load the plugin
plugin = /path/to/libhyfocus.so

# Configure settings
plugin {
    hyfocus {
        # Timer settings (in minutes)
        total_duration = 120          # Total session length (2 hours)
        work_interval = 25            # Work block duration (Pomodoro standard)
        break_interval = 5            # Break duration

        # Enforcement behavior
        enforce_during_break = false  # Allow any workspace during breaks
        
        # App blocking
        block_spawn = true            # Block launching new apps during focus
        spawn_whitelist = kitty,alacritty  # Apps allowed to launch (comma-separated)

        # Visual feedback
        shake_intensity = 15          # Pixels to shake (1-100)
        shake_duration = 300          # Animation duration in ms
        shake_frequency = 50          # Oscillation speed in ms

        # Window classes exempt from enforcement
        exception_classes = eww,rofi,wofi,dmenu,ulauncher
        
        # Exit challenge - makes stopping annoying (optional)
        # 0 = disabled, 1 = type phrase, 2 = math problem, 3 = countdown
        exit_challenge_type = 0
        exit_challenge_phrase = "I want to stop focusing"
    }
}
```

### Exit Challenge Types

The exit challenge adds intentional friction to prevent impulsive session stops:

| Type | Value | Description |
|------|-------|-------------|
| None | `0` | Immediate stop (default) |
| Type Phrase | `1` | Must type a specific phrase |
| Math Problem | `2` | Must solve a random math problem |
| Countdown | `3` | Must confirm 3 times to stop |

### Keybinds

```bash
# Start focus session on workspaces 3 and 5
bind = SUPER, F, hyfocus:start, 3,5

# Toggle focus mode (start/stop)
bind = SUPER SHIFT, F, hyfocus:toggle, 3,5

# Stop current session (may require challenge if enabled)
bind = SUPER CTRL, F, hyfocus:stop,

# Confirm exit challenge answer
bind = SUPER, C, hyfocus:confirm, yes

# Pause/resume session
bind = SUPER, P, hyfocus:pause,
bind = SUPER SHIFT, P, hyfocus:resume,

# Show status
bind = SUPER, S, hyfocus:status,

# Dynamically add/remove workspaces
bind = SUPER, A, hyfocus:allow, 2
bind = SUPER, D, hyfocus:disallow, 2

# Add exception class
bind = SUPER, E, hyfocus:except, firefox

# App spawn whitelist management
bind = SUPER ALT, A, hyfocus:allowapp, firefox
bind = SUPER ALT, D, hyfocus:disallowapp, firefox
```

## Commands

| Command | Arguments | Description |
|---------|-----------|-------------|
| `hyfocus:start` | `<workspace_ids>` | Start a focus session with allowed workspaces (comma-separated) |
| `hyfocus:stop` | - | Stop the current session (may require challenge) |
| `hyfocus:confirm` | `<answer>` | Submit answer for exit challenge |
| `hyfocus:pause` | - | Pause the timer |
| `hyfocus:resume` | - | Resume a paused session |
| `hyfocus:toggle` | `<workspace_ids>` | Toggle session on/off |
| `hyfocus:allow` | `<workspace_id>` | Add workspace to allowed list |
| `hyfocus:disallow` | `<workspace_id>` | Remove workspace from allowed list |
| `hyfocus:except` | `<window_class>` | Add window class to exception list |
| `hyfocus:allowapp` | `<app_name>` | Add app to spawn whitelist |
| `hyfocus:disallowapp` | `<app_name>` | Remove app from spawn whitelist |
| `hyfocus:status` | - | Display current session status |

### Using hyprctl

```bash
# Start a session
hyprctl dispatch hyfocus:start 3,5

# Check status
hyprctl dispatch hyfocus:status

# Stop session
hyprctl dispatch hyfocus:stop
```

## How It Works

### Timer System

The `FocusTimer` class manages Pomodoro-style intervals:

1. **Work Phase**: Full workspace enforcement active
2. **Break Phase**: Optionally relaxed enforcement
3. **Repeat**: Cycles through work/break until total duration reached

### Workspace Enforcement

The `WorkspaceEnforcer` intercepts workspace change attempts:

```
User Action → changeworkspace hook → Check allowed list → Allow/Block
                                                              ↓
                                               Block? → Shake window
```

### Window Shake Animation

When a switch is blocked, the `WindowShake` class provides visual feedback:

```
                    Window Position
                          |
    Original ─────────────┼─────────────
                         /│\
                        / │ \
                       /  │  \    ← Sinusoidal oscillation
                      /   │   \      with decay
                     /    │    \
    ───────────────•─────────────•───────────── Time
                   0ms          300ms
```

The animation uses a decaying sinusoid: `offset = intensity × sin(2π × t/period) × (1 - t/duration)`

## Architecture

```
src/
├── main.cpp              # Plugin entry point, config loading
├── globals.hpp           # Shared state and declarations
├── log.hpp               # Logging utilities
├── dispatchers.cpp/hpp   # User-facing commands
├── eventhooks.cpp/hpp    # Workspace/spawn change interception
├── FocusTimer.cpp/hpp    # Timer logic with work/break cycles
├── WorkspaceEnforcer.cpp/hpp  # Workspace validation
├── WindowShake.cpp/hpp   # Visual feedback animation
└── ExitChallenge.cpp/hpp # Exit minigame system
```

## Thread Safety

The plugin uses atomic operations and mutexes for thread safety:

- Timer runs on a background thread
- Shake animation runs on a background thread  
- All shared state is protected by atomics or mutexes

## Troubleshooting

### Plugin not loading
- Check Hyprland logs: `hyprctl log | grep hyfocus`
- Ensure the `.so` file path is correct
- Verify Hyprland version compatibility

### Workspace switches not being blocked
- Check if a session is active: `hyprctl dispatch hyfocus:status`
- Verify allowed workspaces are set correctly
- Check logs for "Blocked switch" messages

### Shake animation not working
- Ensure shake_intensity > 0
- Check if the focused window exists

### Apps still launching during focus
- Verify `block_spawn = true` in config
- Check if the app is in `spawn_whitelist`
- Look for "Blocked spawn" in logs

### Can't stop session
- If exit challenge is enabled, use `hyfocus:confirm <answer>`
- For phrase challenge: type the exact phrase (case-insensitive)
- For math: submit just the number
- For countdown: keep confirming with "yes"

## Contributing

Contributions welcome! Please:

1. Fork the repository
2. Create a feature branch
3. Submit a pull request

## License

MIT License - see [LICENSE](LICENSE) file.

## Author

**faky** - [github.com/ernestoCruz05](https://github.com/ernestoCruz05)

## Acknowledgments

- Inspired by [hycov](https://github.com/DreamMaoMao/hycov) plugin architecture
- Pomodoro Technique by Francesco Cirillo
