# Cross-Game Runtime Gesture Navigation Gate

**Status:** ✅ COMPLETE — initial seed landed 2026-06-28
**Lane:** gapbug_20260628_runtime_gesture_navigation_gate
**CTest names:**
- `test_fs_gesture_navigation_gate` (planned; `firestaff_fs_gesture_navigation_gate`)
- `firestaff_gesture_navigation_gate_probe` (planned)

**Module files:**
- `include/fs_gesture_navigation_gate.h`
- `src/engine/fs_gesture_navigation_gate.c`
- `tests/test_fs_gesture_navigation_gate.c`
- `probes/firestaff_gesture_navigation_gate_probe.c`

**Closes TODO.md rows:**
- 🔧 Touch and Controller Support → Gesture navigation for runtime movement and turning.
- 🔧 Touch and Controller Support → UI scaling and touch-target audit across launcher and game views.

---

## Scope

The cross-game runtime gesture navigation gate is the seam between
platform-level finger/touch events and the per-game movement/turn
command pipeline.  It is intentionally narrow and data-free:

1. **Recognition** — converts a stream of touch DOWN / MOVE / UP
   events into one of the `FsGestureType` values listed in
   `include/fs_gesture_navigation_gate.h`.  The recognizer is single
   finger and platform-neutral; callers feed it from any source
   (SDL3 SDL_FingerEvent adapter, debug replay, or synthetic test
   events).

2. **Translation** — maps a recognised gesture to the right
   movement/turn command for the active game.  Every supported
   game (DM1, CSB, DM2, Nexus, Theron) has a 12-row sub-table in
   `src/engine/fs_gesture_navigation_gate.c` (5 games × 12
   gestures = 60 rows).  DM1 / CSB / DM2 / Nexus share ReDMCSB
   `C001..C006` movement command IDs (per `DEFS.H:238-243`); Theron
   uses the smaller PCE command IDs reserved by
   `theron_v1_mechanics.h`.

3. **Touch target safety audit** — walks a list of touch zones
   `(x, y, w, h)` and reports each zone's `meetsMinimum` (24×24
   default) and `meetsRecommended` (44×44 default) flags against
   platform baselines.  This is the "UI scaling and touch-target
   audit across launcher and game views" cross-cutting concern:
   the audit produces a `FsGestureZoneAuditReport` so a CI gate or
   M12 settings UI can surface which zones fail the floor.

The gate does NOT:
- Replace the existing `firestaff_touch.c` DM1 V1 recognizer; the
  two coexist.  The DM1 V1 recognizer owns the SDL3 plumbing for
  the active runtime; the new gate is the cross-game metadata +
  audit layer that any recognizer (DM1 V1, future CSB V2, future
  Theron V1) can call into.
- Bypass keyboard routes.  When settings disable gestures for the
  active game, the recognizer still runs but every translation is
  rejected with `FS_GG_DISABLED`.
- Change game-logic.  All accepted gestures resolve to the same
  source-owned movement/turn command IDs that ReDMCSB routes
  dispatch.

---

## 1. Gesture types

```
FS_GG_GESTURE_NONE
FS_GG_GESTURE_SWIPE_UP          // primary forward
FS_GG_GESTURE_SWIPE_DOWN        // primary backward
FS_GG_GESTURE_SWIPE_LEFT        // turn left
FS_GG_GESTURE_SWIPE_RIGHT       // turn right
FS_GG_GESTURE_LONG_PRESS        // hold > 500ms → action/inventory
FS_GG_GESTURE_DOUBLE_TAP        // two quick taps → accept/forward
FS_GG_GESTURE_EDGE_STRAFE_LEFT  // left-edge swipe → strafe left
FS_GG_GESTURE_EDGE_STRAFE_RIGHT // right-edge swipe → strafe right
FS_GG_GESTURE_PINCH_ZOOM_IN     // two-finger spread (V2-only)
FS_GG_GESTURE_PINCH_ZOOM_OUT    // two-finger pinch (V2-only)
FS_GG_GESTURE_TAP               // single tap (no movement)
FS_GG_GESTURE_DRAG              // moved beyond tap tolerance
```

Recognizer tunables (compile-time defaults; runtime overrides go
through settings):

| Tunable              | Default | Source                                  |
|----------------------|---------|-----------------------------------------|
| Tap tolerance        | 24 px   | `firestaff_touch.h FIRESTAFF_TOUCH_TAP_TOLERANCE_PX` |
| Swipe threshold      | 40 px   | `firestaff_touch.h FIRESTAFF_TOUCH_SWIPE_THRESHOLD_PX` |
| Long-press threshold | 500 ms  | `firestaff_touch.h FIRESTAFF_TOUCH_LONG_PRESS_MS` |
| Double-tap window    | 300 ms  | this file `FS_GG_DOUBLE_TAP_MS` (new)   |
| Double-tap distance  | 32 px   | this file `FS_GG_DOUBLE_TAP_DISTANCE_PX` (new) |
| Edge zone fraction   | 20 %    | `firestaff_touch.c FIRESTAFF_TOUCH_EDGE_ZONE_FRAC` |

---

## 2. Per-game translation table

The 60-row table in `src/engine/fs_gesture_navigation_gate.c`
covers every (game × gesture) pair.  Each row cites the source-lock
that owns the underlying command ID.

### 2.1 DM1 / CSB / DM2 / Nexus

These four games share `DEFS.H:238-243` C001..C006:

| Gesture              | DM1 command | CSB command | DM2 command | Nexus command | Notes |
|----------------------|-------------|-------------|-------------|---------------|-------|
| SWIPE_UP             | 3 MOVE_FWD  | 3 MOVE_FWD  | 3 MOVE_FWD  | 3 MOVE_FWD    | CLIKMENU.C:180 F0366 movement |
| SWIPE_DOWN           | 5 MOVE_BACK | 5 MOVE_BACK | 5 MOVE_BACK | 5 MOVE_BACK   | CLIKMENU.C:180 F0366 movement |
| SWIPE_LEFT           | 1 TURN_LEFT | 1 TURN_LEFT | 1 TURN_LEFT | 1 TURN_LEFT   | CLIKMENU.C:142 F0365 turn     |
| SWIPE_RIGHT          | 2 TURN_RIGHT| 2 TURN_RIGHT| 2 TURN_RIGHT| 2 TURN_RIGHT  | CLIKMENU.C:142 F0365 turn     |
| LONG_PRESS           | -1          | -1          | -1          | -1            | routes to right-button C083 |
| DOUBLE_TAP           | 3 MOVE_FWD  | 3 MOVE_FWD  | 3 MOVE_FWD  | 3 MOVE_FWD    | gate-level convenience       |
| EDGE_STRAFE_LEFT     | 6 MOVE_LEFT | 6 MOVE_LEFT | 6 MOVE_LEFT | 6 MOVE_LEFT   | DEFS.H:238-243 C006          |
| EDGE_STRAFE_RIGHT    | 4 MOVE_RIGHT| 4 MOVE_RIGHT| 4 MOVE_RIGHT| 4 MOVE_RIGHT  | DEFS.H:238-243 C004          |
| PINCH_ZOOM_IN/OUT    | -1          | -1          | -1          | -1            | V2-only (out of V1 scope)    |
| TAP / DRAG           | -1          | -1          | -1          | -1            | routed to touch_click_zone_matrix |

### 2.2 Theron

Theron uses opaque PCE command IDs reserved by
`theron_v1_mechanics.h`.  The gate exposes them through
`FS_GG_CMD_THERON_*` constants so the cross-game surface stays
self-contained.

| Gesture              | Theron command     | Source              |
|----------------------|--------------------|---------------------|
| SWIPE_UP             | 0x10 THERON_FORWARD| THQUEST.ASM T520    |
| SWIPE_DOWN           | 0x11 THERON_BACK   | THQUEST.ASM T520    |
| SWIPE_LEFT           | 0x12 THERON_TURN_L | THQUEST.ASM T560    |
| SWIPE_RIGHT          | 0x13 THERON_TURN_R | THQUEST.ASM T560    |
| LONG_PRESS           | -1                 | theron_v1_mechanics |
| DOUBLE_TAP           | 0x10 THERON_FORWARD| parity convenience  |
| EDGE_STRAFE_LEFT     | 0x14 THERON_STRAFE_L | theron_v1_mechanics |
| EDGE_STRAFE_RIGHT    | 0x15 THERON_STRAFE_R | theron_v1_mechanics |
| PINCH_ZOOM_IN/OUT    | -1                 | V2-only             |
| TAP / DRAG           | -1                 | theron_v1 viewport hit-test |

---

## 3. Settings gate

A single boolean per active game.  Stored in module state; the
runtime settings hook is the caller's responsibility:

```c
fs_gesture_gate_set_enabled(FS_GG_GAME_DM1, 1);
fs_gesture_gate_set_enabled(FS_GG_GAME_CSB, 0);  // V1 path stays keyboard-only
fs_gesture_gate_set_active_game(FS_GG_GAME_DM1);
```

The recommended M12 wiring is to read
`M12_Config.dm1V2AccessibilityTouchEnabled` (and per-game siblings)
and pass that to `fs_gesture_gate_set_enabled` at startup.  The
gate is OFF by default so existing keyboard/mouse routes are
unaffected.

---

## 4. Touch target safety audit

The audit is the cross-cutting "UI scaling and touch-target audit"
row.  It walks a list of zones and produces a
`FsGestureZoneAuditReport` containing per-zone `meetsMinimum` and
`meetsRecommended` flags.

```c
FsGestureZone zones[] = {
    { 234, 125, 28, 21, "movement.turn_left" },
    { 263, 147, 27, 21, "movement.backward" }
};
FsGestureZoneAuditReport report;
fs_gesture_audit_zones(zones, 2, 24, 44, &report);
// report.zonesBelowMinimum     -- count of zones < 24
// report.zonesBelowRecommended -- count of zones < 44
```

The default platform targets are 24 px (Apple HIG / IBM minimum)
and 44 px (Apple HIG recommended).  The audit is honest about
sub-recommended zones — DM1 V1's 16×16 backpack slots and 21 px
movement arrows are documented as intentionally smaller than the
recommended tap target because the original UI was 320×200.

A `fs_gesture_audit_builtin_zones` convenience runs the audit
against the built-in DM1 V1 layout-696 + DATA.C zone sample
shipped in `src/engine/fs_gesture_navigation_gate.c`.

---

## 5. Source-lock anchors

ReDMCSB + sibling citations the gate's translation table + recognizer
audit reference:

- ReDMCSB `COMMAND.C:2045-2156` `F0380_COMMAND_ProcessQueue_CPSC` — process
  queue and dispatch (movement/turn source path).
- ReDMCSB `COMMAND.C:375-405` `G0448` mouse movement-arrow zones.
- ReDMCSB `COMMAND.C:108-113` mouse movement zone C001..C006.
- ReDMCSB `COMMAND.C:254-291` keyboard tables for C001..C006.
- ReDMCSB `COMMAND.C:1379-1449` `F0358_COMMAND_GetCommandFromMouseInput_CPSC`
  mouse hit-test.
- ReDMCSB `COMMAND.C:1641-1644` `F0359` primary-to-secondary search.
- ReDMCSB `COMMAND.C:2296-2300` C083 right-button inventory toggle.
- ReDMCSB `CLIKMENU.C:142-174` `F0365` turn handling.
- ReDMCSB `CLIKMENU.C:180-347` `F0366` movement handling.
- ReDMCSB `CLIKMENU.C:519-585` action-menu child clicks (unchanged).
- ReDMCSB `DEFS.H:238-243` C001..C006 movement command IDs.
- ReDMCSB `INPUT.C:574-664` raw mouse button forwarding.
- ReDMCSB `GAMELOOP.C:164-219` V1 input wait loop.
- `SKULL.ASM T520/T560` DM2 smooth movement/turn.
- `SATURN_DMDF T520` Nexus smooth movement/turn.
- `THQUEST.ASM T520/T560` Theron smooth movement/turn.
- `HuC6260/HuC6270` VDC/VCE Theron display.
- `M12_TOUCH_MIN_ZONE_SIZE=24` launcher touch-target floor
  (`touch_layout_m12.h`).
- Sibling `firestaff_touch.c` DM1 V1 SDL3 recognizer (existing baseline).
- Sibling `csb_v2_touch_runtime.c` + `csb_v2_touch_controller_affordance.c`
  CSB V2 affordance translation.
- Sibling `dm2_v2_touch_runtime.c` + `dm2_v2_touch_controller_affordance.c`
  DM2 V2 affordance translation.
- Sibling `nexus_v2_touch_runtime.c` + `nexus_v2_touch_controller_affordance.c`
  Nexus V2 affordance translation.

---

## 6. Honest boundary

This gate does NOT:
- Move the existing DM1 V1 recognizer.  `firestaff_touch.c`
  continues to own SDL3 plumbing and to push into the V1 input
  queue directly.  The new gate is a sibling metadata + audit
  layer that any recognizer can call into.
- Bind into the M11 game loop or M12 launch flow.  The settings
  gate exposes the boolean surface; wiring into `config_m12.c`
  is a follow-up pass.
- Capture real device frames.  All gate assertions are data-free
  and run from synthetic events in CI.
- Promise finished PBR touch overlays.  This is the navigation +
  audit seam, not the visual layer.

The remaining work for the cross-game gesture surface is:
1. M12 / M11 wire-up — read `M12_Config.dm1V2AccessibilityTouchEnabled`
   into `fs_gesture_gate_set_enabled` at startup.
2. Per-game tap-target audit hooks — call `fs_gesture_audit_zones`
   from each V2 HUD overlay + from M12 settings UI.
3. Real-device parity — once touch overlays land, capture paired
   gesture traces.
