# Firestaff Gamepad Support Plan

Created: 2026-04-25

## Goal

Add first-class gamepad/controller support for Firestaff without weakening keyboard/mouse behavior.

Gamepad must work in two places:

1. **Startup launcher / Museum / Settings** — navigation, selection, back, launch.
2. **Runtime gameplay** — movement, turning, strafing, champion/action shortcuts, inventory/spell affordances where practical.

The first implementation should be boring and stable: SDL controller events mapped into the existing `M12_MenuInput` abstraction, with deterministic probes and script injection before any fancy UI polish.

## Non-goals for first pass

- No rebinding UI in the first slice unless the core mapping is already stable.
- No platform-specific controller database shipping unless SDL default mappings are insufficient.
- No haptics until gameplay semantics are solid.
- No V1 gameplay behavior changes except input source plumbing.

## Current input architecture

Relevant files:

- `main_loop_m11.c`
  - maps keyboard, mouse, and script tokens into `M12_MenuInput`
  - already has runtime script injection for `key:*`, `click:x:y`, `move:x:y`
- `menu_startup_m12.[ch]`
  - owns menu state and `M12_MenuInput`
- `firestaff_main_m11.c`
  - CLI flags including `--script`
- `run_firestaff_m12_startup_runtime_fuzz.sh`
  - real runtime launcher fuzz path
- M12 probes:
  - `run_firestaff_m12_startup_menu_probe.sh`
  - `run_firestaff_m12_menu_mouse_probe.sh`
  - `run_firestaff_m12_settings_smoke.sh`

This is good: gamepad should feed the same `M12_MenuInput` seam instead of inventing a parallel path.

## Phase 1 — SDL controller detection and normalized mapping

### Implementation

Add a small controller input layer, probably:

- `input_gamepad_m12.h`
- `input_gamepad_m12.c`

Responsibilities:

- initialize/close SDL GameController or SDL3 Gamepad support depending on current SDL version in use;
- detect first available controller;
- normalize controller events into Firestaff actions;
- expose a pure function that can be probed without hardware where possible.

Suggested logical actions:

| Gamepad input | Launcher/Menu action | Gameplay action |
|---|---|---|
| D-pad up/down | `M12_MENU_INPUT_UP/DOWN` | move forward/back |
| D-pad left/right | `M12_MENU_INPUT_LEFT/RIGHT` | turn left/right |
| Left stick digital threshold | same as D-pad | movement/turning |
| A / Cross | `ACCEPT` | primary action / click equivalent |
| B / Circle | `BACK` | back/cancel |
| X / Square | `ACTION` | secondary/action-hand |
| Y / Triangle | `CYCLE_CHAMPION` | cycle champion |
| LB/RB | left/right section/page or strafe | strafe/cycle hand/action page |
| Start/Menu | accept/pause/options | pause/menu |
| Back/View | back | inventory/map toggle candidate |

Rules:

- Unknown/unmapped controller events are no-op.
- Axis noise uses deadzone and edge-triggering, not repeated floods.
- Button repeat should be explicit and rate-limited for menu navigation.

### Verification

Add pure unit/probe coverage:

- button A -> accept
- button B -> back
- D-pad directions -> arrows
- axis below deadzone -> no-op
- axis crossing deadzone -> one navigation event
- axis staying held -> controlled repeat only if enabled
- invalid controller event -> no-op

## Phase 2 — runtime event integration

Wire SDL controller events in `main_loop_m11.c` near the existing keyboard/mouse mapping.

Requirements:

- launcher receives gamepad events while in startup menu, settings, Museum, game-options, message views;
- gameplay receives gamepad events after launch;
- gamepad path must not break keyboard/mouse/script paths;
- controller open/close hotplug events should be safe no-ops if unsupported.

Add script injection support for deterministic tests, e.g.

- `pad:a`
- `pad:b`
- `pad:x`
- `pad:y`
- `pad:up/down/left/right`
- `pad:lb/rb/start/back`
- `pad:axis:leftx:+1`

This avoids requiring physical controller hardware in CI.

## Phase 3 — launcher/menu UX

Update startup menu footer/help text to include controller hints when a controller is present or gamepad mode is enabled.

Examples:

- `D-PAD MOVE    A SELECT    B BACK`
- In Museum: `D-PAD SECTIONS    LB/RB PAGE    B BACK`
- In game options: `D-PAD MOVE    LEFT/RIGHT CYCLE    A LAUNCH    B BACK`

Do not clutter V1 original gameplay UI; launcher can be modern.

## Phase 4 — gameplay mapping

First gameplay slice should be conservative:

- D-pad/stick up/down/left/right -> existing movement/turn input path
- LB/RB -> strafe left/right if currently supported
- Y -> cycle champion
- A/X -> existing `ACTION`/use paths only where already safe
- B -> back/cancel/inventory-close if stateful screens exist

Do not bind complex spell/inventory workflows until basic dungeon movement is stable.

Add a runtime fuzz script with gamepad tokens:

- startup menu navigation
- open Museum and back
- settings cycle and back
- launch DM1 with assets present
- gameplay movement tokens after launch
- invalid spam does not crash

## Phase 5 — settings and remapping

Only after phases 1–4 are green:

- add settings row: `GAMEPAD: AUTO / OFF`
- later: mapping presets
  - `CLASSIC D-PAD`
  - `MODERN STICK`
  - `ACCESSIBILITY SIMPLE`
- persist setting through existing M12 config.

## Phase 6 — controller database and platform polish

If SDL mapping coverage is poor:

- load SDL gamecontroller database from user config path;
- document where to put mappings;
- add fallback joystick mode if GameController API does not recognize device.

Later polish:

- rumble/haptics for damage/door/thud, optional and disabled by default;
- controller glyphs in launcher help text;
- accessibility options: hold-to-repeat speed, invert axes, left-handed mapping.

## Regression gates

Before claiming gamepad support:

- existing startup menu probe still green;
- menu mouse probe still green;
- settings smoke still green;
- startup runtime fuzz still green;
- new gamepad mapping probe green;
- new gamepad runtime fuzz green;
- launcher smoke green;
- if gameplay path touched: M11 game-view probe and M10 verify green if feasible;
- `git diff --check` clean;
- staged secret scan clean.

## Suggested pass breakdown

### Pass GP1 — pure mapper + probe

- Add controller-event -> `M12_MenuInput` mapper.
- Add script token parser for `pad:*` without needing real hardware.
- Probe mapping/deadzone/no-op behavior.

### Pass GP2 — launcher integration

- Wire gamepad events into startup menu runtime.
- Add runtime fuzz for Museum/settings/launch via `pad:*` script tokens.
- Add controller help text.

### Pass GP3 — gameplay movement integration

- Wire gamepad movement after launch.
- Prove movement/turn/strafe/cycle champion through runtime fuzz.

### Pass GP4 — settings toggle

- Add `GAMEPAD AUTO/OFF` setting and persistence.
- Ensure keyboard/mouse unaffected.

### Pass GP5 — hardware smoke

- Optional/manual when a physical controller is available.
- Record controller name, SDL mapping, and supported buttons.

## Definition of done

Gamepad support is done for the first release when:

- launcher can be operated fully with a common gamepad;
- DM1 can be launched with gamepad only;
- basic dungeon movement works with gamepad;
- invalid/hotplug/no-controller states never crash;
- keyboard and mouse remain fully functional;
- all automated probes and fuzz scripts pass.
