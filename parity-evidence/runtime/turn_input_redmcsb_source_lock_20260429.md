# Turn input ReDMCSB source-lock — 2026-04-29

## Source anchors
- `COMMAND.C:2150-2152`: `C001_COMMAND_TURN_LEFT` / `C002_COMMAND_TURN_RIGHT` dispatch to `F0365_COMMAND_ProcessTypes1To2_TurnParty`.
- `CLIKMENU.C:171-173`: turn commands process party leave/enter sensors around `F0284_CHAMPION_SetPartyDirection(M021_NORMALIZE(...))` and do not require a map-coordinate move.
- `CHAMPION.C:117-130`: `F0284_CHAMPION_SetPartyDirection` rotates every party champion's `Cell` and `Direction` by the normalized delta, then updates `G0308_i_PartyDirection`.

## Firestaff mismatch fixed
- `F0888_ORCH_ApplyPlayerInput_Compat` called `F0702_MOVEMENT_TryMove_Compat`, but only applied state changes in the `MOVE_OK` branch.
- Turn-only inputs (`CMD_TURN_LEFT`, `CMD_TURN_RIGHT`) return `MOVE_TURN_ONLY`, so real-dungeon turns were silently dropped; runtime clicks/keys could report input but the actual `world->party.direction` did not advance.

## Runtime fix
- Added `set_party_direction_redmcsb_compat()` to apply the ReDMCSB `F0284` party-direction contract to compat state.
- `MOVE_TURN_ONLY` now updates `world->party.direction`, rotates present champion directions, emits `EMIT_PARTY_MOVED`, and returns success.
- Post-move final direction updates now also use the same source-aligned helper.

## Verification
- `cmake --build build --target firestaff -j2`: PASS.
- `./run_firestaff_m10_tick_orchestrator_probe.sh $HOME/.firestaff/data/DUNGEON.DAT verification-turn-fix-2`: new functional checks PASS:
  - `17b: CMD_TURN_RIGHT updates party direction with real dungeon loaded`
  - `17c: CMD_TURN_RIGHT rotates champion direction per F0284`
  - Existing perf invariant `46: 10000-tick headless run completes < 5s` remains FAIL on N2 (`10.278s`), unrelated to turn correctness.
- `./run_firestaff_m11_game_view_probe.sh`: PASS, `592/592 invariants passed`, including `INV_GV_04 game view input turns the party through the real tick orchestrator` and `INV_GV_05 turning changes the rendered pseudo-viewport frame`.
