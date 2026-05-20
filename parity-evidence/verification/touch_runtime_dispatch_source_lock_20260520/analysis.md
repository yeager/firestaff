# Touch Runtime Dispatch Source-Lock, 2026-05-20

Scope: wire DM1 V1 entrance/menu mouse and touch coordinates through the already source-locked entrance route helper, then into the existing M11 entrance command path. This pass deliberately does not add dungeon movement touch zones, item interaction, gestures, or UI scaling.

## ReDMCSB Evidence Anchors

- `ENTRANCE.C:739-747`: `F0441_STARTEND_ProcessEntrance` installs `G0445_as_Graphic561_PrimaryMouseInput_Entrance`, clears secondary mouse input, installs the later-media entrance keyboard table, and clears secondary keyboard input.
- `ENTRANCE.C:850-883`: the entrance loop draws the entrance, shows the pointer, discards prior input, sets `G0298_B_NewGame = C099_MODE_WAITING_ON_ENTRANCE`, then waits through VBlank/key processing and `F0380_COMMAND_ProcessQueue_CPSC` until a fresh command leaves waiting mode.
- `COMMAND.C:340-353`: `G0445_as_Graphic561_PrimaryMouseInput_Entrance` source order: enter `C200`/`C407`, bonus `C201`/`C407` mask `0x0010`, resume `M566`/`C409`, quit `C216`/`C434`, credits `M567`/`C411`, sentinel.
- `COMMAND.C:1379-1449`: `F0358_COMMAND_GetCommandFromMouseInput_CPSC` walks the active mouse table in source order, checks the row button mask, expands negative zone-backed rows, and uses inclusive hit tests.
- `COMMAND.C:1641-1660`: `F0359_COMMAND_ProcessClick_CPSC` routes primary mouse input first, secondary second, then queues command/x/y when non-zero.
- `COORD.C:1903-1920`: zone margin/point helpers preserve inclusive left/right/top/bottom tests.
- `COORD.C:2490-2495`: `F0638_GetZone` resolves a layout zone through `F0637_GetProportionalZone`.
- `DEFS.H:375-384`: later I34E/I34M entrance command IDs are `C200`, `C201`, `M566=202`, `M567=203`.
- `DEFS.H:3824-3826,3845`: entrance zone IDs are `C407`, `C409`, `C411`, and `C434`.

## Implementation

- `src/engine/main_loop_m11.c` now includes `entrance_mouse_routes_pc34_compat.h` and exposes deterministic dispatch helpers:
  - `M11_Entrance_DispatchSourceLockedPointerCommand`
  - `M11_Entrance_DispatchSourceLockedKeyCommand`
- The entrance wait loop routes SDL mouse button down and SDL finger down events through the source-locked entrance route helper before mapping source command IDs to the existing M11 entrance command path.
- Return, Space, and keypad Enter remain non-activation keys for the current click-only entrance semantics. Escape/Q remain quit.
- Credits are recognized as a source command and return to the entrance wait loop instead of accidentally opening the doors.
- The later-media quit zone is now distinct from credits: quit is `C434` and credits is `C411`.

## Verification Intent

`tests/test_entrance_runtime_dispatch_source_lock.c` proves enter, bonus, resume, quit, credits, outside misses, zero-mask misses, and keyboard non-activation. Existing entrance route and wait-policy probes remain targeted guardrails.
