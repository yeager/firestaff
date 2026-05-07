# Pass233 — DM1 PC34 movement runtime-hit blocker

Status: `BLOCKED_RUNTIME_HITS_REQUIRED`.

## Scope

Runtime-hit pass for the movement chain after `command_accepted`, specifically:

- `turn_or_step_state_applied`
- `party_coordinates_committed`

No symbol-map entries were promoted. `data/original_runtime/dm1_pc34_i34e_symbol_map.v1.json` still correctly requires `verified_runtime_hit` evidence before runtime CS:IP/global use.

## ReDMCSB source audit

### `turn_or_step_state_applied` — `CLIKMENU.C`

Source: `<redmcsb-source>/ReDMCSB_WIP20210206/Toolchains/Common/Source/CLIKMENU.C`

Exact anchors:

- `CLIKMENU.C:156` sets `G0321_B_StopWaitingForPlayerInput = C1_TRUE` for turn commands.
- `CLIKMENU.C:171` calls `F0276_SENSOR_ProcessThingAdditionOrRemoval(...)` before direction change.
- `CLIKMENU.C:172` calls `F0284_CHAMPION_SetPartyDirection(M021_NORMALIZE(G0308_i_PartyDirection + ...))`.
- `CLIKMENU.C:173` calls `F0276_SENSOR_ProcessThingAdditionOrRemoval(...)` after direction change.
- `CLIKMENU.C:237` sets `G0321_B_StopWaitingForPlayerInput = C1_TRUE` for movement commands.
- `CLIKMENU.C:256` computes `AL1118_ui_MovementArrowIndex = P0735_ui_Command - C003_COMMAND_MOVE_FORWARD`.
- `CLIKMENU.C:264` reads source party X/Y from `G0306_i_PartyMapX` / `G0307_i_PartyMapY`.
- `CLIKMENU.C:269` calls `F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement(...)` using `G0308_i_PartyDirection` and movement-arrow deltas.
- `CLIKMENU.C:272` calls `F0267_MOVE_GetMoveResult_CPSCE(...)` for the stairs-destination path.
- `CLIKMENU.C:325` calls `F0267_MOVE_GetMoveResult_CPSCE(C0xFFFF_THING_PARTY, G0306_i_PartyMapX, G0307_i_PartyMapY, L1121_i_MapX, L1122_i_MapY)`.
- `CLIKMENU.C:328` processes source-square sensor removal after successful non-stairs move.

Required runtime observable: debugger code breakpoint after line `173` for a turn route or after `325`/`328` for a step route, with observed CS:IP and party direction/X/Y context.

### `party_coordinates_committed` — `MOVESENS.C`

Source: `<redmcsb-source>/ReDMCSB_WIP20210206/Toolchains/Common/Source/MOVESENS.C`

Exact anchors:

- `MOVESENS.C:316` enters `F0267_MOVE_GetMoveResult_CPSCE`.
- `MOVESENS.C:438` tests `P0560_i_DestinationMapX >= 0` before destination processing.
- `MOVESENS.C:441` checks `P0557_T_Thing == C0xFFFF_THING_PARTY`.
- `MOVESENS.C:442` writes `G0306_i_PartyMapX = P0560_i_DestinationMapX`.
- `MOVESENS.C:443` writes `G0307_i_PartyMapY = P0561_i_DestinationMapY`.
- `MOVESENS.C:493` rechecks party thing after teleporter retarget.
- `MOVESENS.C:494` rewrites `G0306_i_PartyMapX` for a teleporter destination.
- `MOVESENS.C:495` rewrites `G0307_i_PartyMapY` for a teleporter destination.
- `MOVESENS.C:556` calls `F0128_DUNGEONVIEW_Draw_CPSF(G0308_i_PartyDirection, P0560_i_DestinationMapX, P0561_i_DestinationMapY)` during the falling path.

Required runtime observable: memory-write watchpoint on resolved `G0306_i_PartyMapX` / `G0307_i_PartyMapY` globals, with source-bound CS:IP at the `MOVESENS.C` write site.

## Runtime attempts on N2

- `dosbox-x` exists at `/usr/bin/dosbox-x`; a stock DM1 directory-context smoke run started under `xvfb-run`.
- `dosbox-debug` exists at `/usr/bin/dosbox-debug`; the debugger UI starts on N2.

These attempts did not produce promotable hits because the current repository has no N2-local `FIRES.MAP`/public-symbol table or equivalent source-to-loaded-image binding for `F0365`, `F0366`, `F0267`, `G0306_i_PartyMapX`, or `G0307_i_PartyMapY`.

## Blocker

Exact remaining blocker: need a `FIRES.MAP`/public-symbol table or another reproducible debugger binding that maps ReDMCSB `CLIKMENU.C`/`MOVESENS.C` source seams and globals to loaded original `FIRES` CS:IP/data addresses.

Required formula remains:

- `runtime_cs = (PSP + 0x10) + map_segment`
- `runtime_ip = map_offset`

## Tool improvement recommendation

Extend `tools/dm1_original_runtime_trace.py` with a DOSBox debugger driver that can ingest `FIRES.MAP`/global symbols, compute runtime CS:IP from the actual PSP, and emit code-breakpoint/data-watchpoint scripts. Keep the current guardrail: `--check-only --expect-blocked` should pass until every trace event has `verified_runtime_hit` evidence.
