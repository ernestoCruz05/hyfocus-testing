# Phase 6: EWW Widget UI - Research

**Researched:** 2026-01-09
**Domain:** EWW (Elkowar's Wacky Widgets) for Hyprland with glass/blur styling
**Confidence:** HIGH

<research_summary>
## Summary

Researched the EWW ecosystem for building a minimal, glass-style widget UI for HyFocus on Hyprland. EWW is a Rust+GTK widget system using the "yuck" configuration language and GTK CSS for styling. It's the standard choice for custom widgets on Hyprland.

Key finding: Blur effects cannot be achieved through EWW/GTK CSS alone - they require compositor-level support via Hyprland's `layerrule` configuration. The glass effect is achieved by: (1) setting transparent backgrounds in EWW CSS, and (2) enabling blur for the EWW namespace in Hyprland config.

**Primary recommendation:** Use EWW with multiple `defwindow` definitions (start panel, status indicator) so each can be independently styled and blurred. Communicate with HyFocus via `hyprctl` commands and listen to Hyprland IPC socket for state changes.
</research_summary>

<standard_stack>
## Standard Stack

### Core
| Library | Version | Purpose | Why Standard |
|---------|---------|---------|--------------|
| eww | 0.6.x | Widget system | Only practical widget toolkit for Hyprland that supports layer shell |
| gtk-layer-shell | latest | Wayland layer protocol | Required for EWW on Wayland - positions widgets as overlay/dock layers |
| socat | latest | Socket communication | Standard tool for listening to Hyprland IPC socket |
| jq | latest | JSON parsing | Parse hyprctl JSON output for workspace/window state |

### Supporting
| Library | Version | Purpose | When to Use |
|---------|---------|---------|-------------|
| playerctl | latest | Media control | If adding media controls to widgets |
| brightnessctl | latest | Brightness | If adding brightness controls |
| bash/zsh | system | Script runner | For `deflisten` and `defpoll` commands |

### Alternatives Considered
| Instead of | Could Use | Tradeoff |
|------------|-----------|----------|
| EWW | AGS (Aylur's GTK Shell) | AGS uses JavaScript, more complex but more powerful. EWW is simpler, better documented. |
| EWW | Waybar | Waybar is status-bar only, not general widgets. EWW allows custom panels/popups. |
| Shell scripts | Rust/Python IPC lib | Shell scripts simpler for prototyping, can optimize later if needed |

**Installation:**
```bash
# Arch Linux
pacman -S eww-wayland gtk-layer-shell socat jq

# From source (Wayland)
cargo build --release --no-default-features --features=wayland
```
</standard_stack>

<architecture_patterns>
## Architecture Patterns

### Recommended Project Structure
```
eww/
├── eww.yuck              # Main config - includes all .yuck files
├── eww.scss              # Main styles - imports all .scss files
├── windows/
│   ├── start-panel.yuck  # Start panel window definition
│   ├── status.yuck       # Minimal status indicator
│   └── warning.yuck      # Block warning popup
├── widgets/
│   ├── workspace-select.yuck  # Workspace checkbox list
│   ├── timer-display.yuck     # Focus timer display
│   └── common.yuck            # Shared widget components
├── styles/
│   ├── _variables.scss   # Color palette, sizing
│   ├── _glass.scss       # Glass effect styles
│   ├── start-panel.scss  # Start panel styles
│   └── status.scss       # Status indicator styles
└── scripts/
    ├── hyfocus-status    # Get HyFocus state via hyprctl
    ├── hyfocus-start     # Start focus session
    ├── hyfocus-stop      # Stop focus session
    └── listen-state      # Listen to HyFocus state changes
```

### Pattern 1: Multiple Windows for Blur Control
**What:** Define each UI component as a separate `defwindow` with unique namespace
**When to use:** Always - Hyprland can only blur entire layer surfaces, not parts
**Example:**
```yuck
; Start panel - larger, more complex
(defwindow start-panel
  :monitor 0
  :namespace "hyfocus-start"
  :geometry (geometry :x "50%" :y "50%" :width "400px" :height "300px" :anchor "center")
  :stacking "overlay"
  :exclusive false
  :focusable true
  (start-panel-content))

; Status indicator - minimal, corner
(defwindow status
  :monitor 0
  :namespace "hyfocus-status"
  :geometry (geometry :x "10px" :y "10px" :width "120px" :height "40px" :anchor "top left")
  :stacking "overlay"
  :exclusive false
  :focusable false
  (status-content))
```

### Pattern 2: Hyprland IPC Integration
**What:** Use `deflisten` with socat to react to Hyprland events in real-time
**When to use:** For workspace changes, window events, HyFocus state changes
**Example:**
```yuck
; Listen to Hyprland socket for workspace changes
(deflisten active-workspace :initial "1"
  `socat -u UNIX-CONNECT:$XDG_RUNTIME_DIR/hypr/$HYPRLAND_INSTANCE_SIGNATURE/.socket2.sock - |
   stdbuf -oL grep '^workspace>>' |
   stdbuf -oL awk -F'>>' '{print $2}'`)

; Poll HyFocus status (plugin exposes via hyprctl)
(defpoll hyfocus-state :interval "1s"
  `hyprctl plugin hyfocus status 2>/dev/null || echo 'inactive'`)
```

### Pattern 3: Glass Effect via Compositor + CSS
**What:** Combine transparent CSS backgrounds with Hyprland blur rules
**When to use:** For the "glass" aesthetic on any EWW window
**Example:**
```scss
// eww.scss - transparent background with subtle border
.glass-panel {
  background-color: rgba(30, 30, 46, 0.7);  // Semi-transparent
  border-radius: 12px;
  border: 1px solid rgba(255, 255, 255, 0.1);
  padding: 16px;
}

// hyprland.conf - enable blur for EWW namespaces
layerrule = blur, hyfocus-start
layerrule = blur, hyfocus-status
layerrule = ignorezero, hyfocus-start
layerrule = ignorezero, hyfocus-status
```

### Anti-Patterns to Avoid
- **Single defwindow with multiple "panels":** Can't blur selectively, harder to position
- **Polling hyprctl every 100ms:** Performance killer, use socket2 deflisten instead
- **CSS blur attempts:** GTK CSS doesn't support backdrop-filter, must use compositor
- **Hardcoded monitor indices:** Use named monitors or dynamic detection
</architecture_patterns>

<dont_hand_roll>
## Don't Hand-Roll

| Problem | Don't Build | Use Instead | Why |
|---------|-------------|-------------|-----|
| Blur effects | CSS backdrop-filter attempts | Hyprland `layerrule = blur` | GTK doesn't support backdrop-filter; compositor owns blur |
| Window positioning | Manual coordinate calculation | EWW anchor + geometry | EWW handles multi-monitor, scaling, etc. |
| IPC event handling | Custom socket code | `deflisten` + socat | socat handles reconnection, buffering |
| Workspace state | Parsing hyprctl output manually | `jq` with hyprctl -j | JSON output is stable, text output changes |
| Animation/transitions | Manual GTK animation | EWW `revealer`, `stack` transitions | Built-in widgets handle GTK animation properly |
| Configuration system | Custom config files | EWW variables + Hyprland config | Users expect ~/.config/eww/ and hyprland.conf |

**Key insight:** EWW + Hyprland together provide a complete system. EWW handles widget rendering and user interaction. Hyprland handles window management, blur, positioning. Don't try to do compositor work in EWW or widget work in Hyprland config.
</dont_hand_roll>

<common_pitfalls>
## Common Pitfalls

### Pitfall 1: Blur Not Working
**What goes wrong:** EWW window has transparent background but no blur effect
**Why it happens:** Missing Hyprland layerrule or wrong namespace
**How to avoid:**
1. Set unique `:namespace` in defwindow
2. Add `layerrule = blur, <namespace>` to hyprland.conf
3. Add `layerrule = ignorezero, <namespace>` for rounded corners
4. Verify with `hyprctl layers` that namespace appears
**Warning signs:** Window is just dark/transparent, no frosted effect

### Pitfall 2: Widgets Disappear After Monitor Sleep
**What goes wrong:** EWW widgets vanish when monitor wakes from sleep
**Why it happens:** GTK layer-shell reconnection issue
**How to avoid:**
- Add `exec-once = sleep 1 && eww daemon` to hyprland.conf (delay startup)
- Use a script that monitors and restarts EWW if windows disappear
- Consider `eww reload` on monitor wake event
**Warning signs:** Widgets work initially, gone after lock/sleep

### Pitfall 3: Focusable Windows Steal All Input
**What goes wrong:** Setting `:focusable true` causes EWW to capture all keyboard input
**Why it happens:** GTK layer-shell keyboard_interactivity modes behave unexpectedly
**How to avoid:**
- Only use `:focusable true` for windows that need text input
- Use `:focusable "ondemand"` instead of `true` when possible
- Keep status indicators as `:focusable false`
**Warning signs:** Can't type in other windows while EWW panel is visible

### Pitfall 4: Performance Issues from Polling
**What goes wrong:** High CPU usage, laggy system
**Why it happens:** defpoll running expensive commands at short intervals
**How to avoid:**
- Use `deflisten` with socket2 for event-driven updates
- Poll no faster than 1s for non-critical data
- Cache script results where possible
**Warning signs:** High CPU in `eww` or script processes

### Pitfall 5: Rounded Corners Show Background Through Blur
**What goes wrong:** Blur visible in corners where widget has border-radius
**Why it happens:** Blur applies to entire layer surface rectangle
**How to avoid:**
- Use `layerrule = ignorezero, <namespace>` - ignores fully transparent pixels
- Alternatively: `layerrule = ignorealpha 0.5, <namespace>` for semi-transparent
**Warning signs:** Rectangle blur shape visible around rounded widget
</common_pitfalls>

<code_examples>
## Code Examples

Verified patterns from official sources and community best practices:

### Basic Window with Glass Effect
```yuck
; eww.yuck
(defwindow hyfocus-status
  :monitor 0
  :namespace "hyfocus-status"
  :geometry (geometry
    :x "10px"
    :y "10px"
    :width "140px"
    :height "50px"
    :anchor "top right")
  :stacking "overlay"
  :exclusive false
  :focusable false
  (status-widget))

(defwidget status-widget []
  (box :class "glass-panel status"
       :orientation "h"
       :space-evenly false
    (label :class "icon" :text "")
    (label :class "time" :text "${focus-time}")))

(defpoll focus-time :interval "1s"
  `hyprctl plugin hyfocus status | grep -oP 'remaining: \\K[0-9:]+' || echo "--:--"`)
```

```scss
// eww.scss
* { all: unset; }

.glass-panel {
  background-color: rgba(30, 30, 46, 0.75);
  border-radius: 12px;
  border: 1px solid rgba(255, 255, 255, 0.08);
  padding: 8px 12px;
}

.status {
  .icon {
    font-family: "Symbols Nerd Font";
    font-size: 18px;
    margin-right: 8px;
    color: #89b4fa;
  }
  .time {
    font-family: "JetBrains Mono";
    font-size: 14px;
    color: #cdd6f4;
  }
}
```

### Start Panel with Workspace Selection
```yuck
; windows/start-panel.yuck
(defwindow start-panel
  :monitor 0
  :namespace "hyfocus-start"
  :geometry (geometry :x "50%" :y "50%" :width "420px" :height "auto" :anchor "center")
  :stacking "overlay"
  :exclusive false
  :focusable true
  (start-panel-content))

(defwidget start-panel-content []
  (box :class "glass-panel start-panel" :orientation "v" :space-evenly false
    (box :class "header" :orientation "h"
      (label :text "Start Focus Session")
      (button :class "close-btn" :onclick "eww close start-panel" ""))

    (box :class "workspace-section" :orientation "v"
      (label :class "section-title" :text "Allowed Workspaces")
      (workspace-list))

    (box :class "warning" :orientation "h"
      (label :class "warning-icon" :text "")
      (label :class "warning-text"
        :text "App launching and workspace switching will be blocked"))

    (box :class "actions" :orientation "h" :halign "end"
      (button :class "cancel-btn" :onclick "eww close start-panel" "Cancel")
      (button :class "start-btn"
        :onclick "hyprctl dispatch hyfocus:start ${allowed-workspaces} && eww close start-panel"
        "Start Focus"))))

(defvar allowed-workspaces "1,2")

(defwidget workspace-list []
  (box :class "workspace-grid" :orientation "h" :space-evenly true
    (for ws in "[1, 2, 3, 4, 5, 6, 7, 8, 9, 10]"
      (checkbox
        :class "ws-checkbox"
        :onchecked "scripts/toggle-workspace ${ws} add"
        :onunchecked "scripts/toggle-workspace ${ws} remove"
        :checked {matches(allowed-workspaces, "${ws}")}
        (label :text "${ws}")))))
```

### Hyprland Config for Glass Effect
```conf
# hyprland.conf

# EWW blur configuration
layerrule = blur, hyfocus-start
layerrule = blur, hyfocus-status
layerrule = blur, hyfocus-warning

# Ignore transparent pixels (for rounded corners)
layerrule = ignorezero, hyfocus-start
layerrule = ignorezero, hyfocus-status
layerrule = ignorezero, hyfocus-warning

# Optional: adjust blur strength for these layers
# decoration:blur:size = 8
# decoration:blur:passes = 2
```

### Script: Get HyFocus Status
```bash
#!/bin/bash
# scripts/hyfocus-status

# Query HyFocus plugin status
status=$(hyprctl plugin hyfocus status 2>/dev/null)

if [ -z "$status" ]; then
  echo '{"active": false, "remaining": "--:--", "workspaces": []}'
  exit 0
fi

# Parse and output JSON for EWW
active=$(echo "$status" | grep -q "active: true" && echo "true" || echo "false")
remaining=$(echo "$status" | grep -oP 'remaining: \K[0-9:]+' || echo "--:--")
workspaces=$(echo "$status" | grep -oP 'workspaces: \[\K[^\]]+' || echo "")

echo "{\"active\": $active, \"remaining\": \"$remaining\", \"workspaces\": [$workspaces]}"
```
</code_examples>

<sota_updates>
## State of the Art (2025-2026)

| Old Approach | Current Approach | When Changed | Impact |
|--------------|------------------|--------------|--------|
| blurls config | layerrule = blur | Hyprland 0.40+ | Old blurls deprecated, use layerrule instead |
| X11 EWW | Wayland-native EWW | 2023+ | Must build with `--features=wayland`, no X11 fallback |
| Manual eww.xml | eww.yuck (Lisp syntax) | 2022 | XML deprecated, yuck is current syntax |
| Single config file | Modular includes | Current | Use `(include "path.yuck")` for organization |

**New tools/patterns to consider:**
- **AGS (Aylur's GTK Shell):** JavaScript-based alternative, more powerful but steeper learning curve
- **Hyprpanel:** Pre-built panel solution, but less customizable than EWW
- **eww-wayland in AUR:** Pre-built package, easier than compiling

**Deprecated/outdated:**
- **eww.xml format:** Completely removed, use .yuck
- **blurls config option:** Use layerrule instead
- **X11-specific features:** Don't use on Wayland (reserve, wm-ignore only partial support)
</sota_updates>

<open_questions>
## Open Questions

Things that need validation during implementation:

1. **HyFocus IPC interface**
   - What we know: HyFocus exposes commands via Hyprland dispatchers
   - What's unclear: Exact command format for getting status, starting/stopping sessions
   - Recommendation: Document current HyFocus dispatcher API before building widgets

2. **Multi-monitor behavior**
   - What we know: EWW supports per-monitor windows via `:monitor` property
   - What's unclear: How HyFocus handles multi-monitor focus sessions
   - Recommendation: Test on multi-monitor setup, may need per-monitor status widgets

3. **Widget persistence across Hyprland reload**
   - What we know: `hyprctl reload` can disrupt layer surfaces
   - What's unclear: Whether EWW needs restart after hyprland config reload
   - Recommendation: Test behavior, possibly add restart hook
</open_questions>

<sources>
## Sources

### Primary (HIGH confidence)
- [EWW Official Documentation](https://elkowar.github.io/eww/) - Configuration, widgets, GTK theming
- [EWW GitHub](https://github.com/elkowar/eww) - Source, examples, issues
- [Hyprland Wiki - Status Bars](https://wiki.hypr.land/Useful-Utilities/Status-Bars/) - EWW integration guide
- [Hyprland Wiki - IPC](https://wiki.hypr.land/IPC/) - Socket events, hyprctl commands
- [Hyprland Wiki - Window Rules](https://wiki.hypr.land/Configuring/Window-Rules/) - layerrule syntax

### Secondary (MEDIUM confidence)
- [GitHub Discussion: Blur specific widget](https://github.com/elkowar/eww/discussions/586) - Confirmed blur is compositor-level
- [GitHub Discussion: Hyprland layer blur](https://github.com/hyprwm/Hyprland/discussions/748) - layerrule solution
- [EWW Troubleshooting](https://elkowar.github.io/eww/troubleshooting.html) - Common issues

### Tertiary (LOW confidence - needs validation)
- Community dotfile repos (husseinhareb/hyprland-eww, adi1090x/widgets) - Pattern examples
</sources>

<metadata>
## Metadata

**Research scope:**
- Core technology: EWW (Elkowar's Wacky Widgets) on Wayland
- Ecosystem: Hyprland IPC, GTK layer-shell, socat, jq
- Patterns: Multi-window architecture, glass styling, event-driven updates
- Pitfalls: Blur configuration, focus handling, performance

**Confidence breakdown:**
- Standard stack: HIGH - EWW is the established choice for Hyprland widgets
- Architecture: HIGH - Multi-window pattern verified in documentation and discussions
- Pitfalls: HIGH - Documented in GitHub issues and official troubleshooting
- Code examples: MEDIUM - Adapted from official examples, needs HyFocus-specific testing

**Research date:** 2026-01-09
**Valid until:** 2026-02-09 (30 days - EWW/Hyprland ecosystem relatively stable)
</metadata>

---

*Phase: 06-eww-widget-ui*
*Research completed: 2026-01-09*
*Ready for planning: yes*
