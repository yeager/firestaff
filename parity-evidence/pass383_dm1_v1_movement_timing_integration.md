# Pass383 — DM1 V1 movement timing integration

Status: `PASS_DM1_V1_MOVEMENT_TIMING_GATES_SOURCE_LOCKED_RUNTIME_BOUNDARY_CLASSIFIED`

## Scope

Traced the DM1 V1 input → command queue → movement → sensor/timing pipeline in ReDMCSB and bound it to the current Firestaff compatibility seams. This is a timing-gate proof, not an original pixel-parity promotion.

## Timing gate findings

- `COMMAND.C:2075-2100` locks the command queue and checks the queued movement command before dequeue. If `G0310_i_DisabledMovementTicks` is non-zero, or if `G0311_i_ProjectileDisabledMovementTicks` matches the absolute movement direction, the command remains queued; the queue unlocks and pending clicks replay.
- `COMMAND.C:2118-2156` dequeues exactly one ungated command, unlocks, replays pending clicks, then dispatches turns to `F0365` and movement commands `C003..C006` to `F0366`.
- `CLIKMENU.C:317-323` blocked movement discards all buffered input, waits one vblank on PC-family ports, sets `G0321_B_StopWaitingForPlayerInput = C0_FALSE`, and returns before `F0267` side effects.
- `CLIKMENU.C:325-347` accepted movement calls `F0267_MOVE_GetMoveResult_CPSCE`, computes max living-champion movement ticks with `F0310_CHAMPION_GetMovementTicks`, stores `G0310_i_DisabledMovementTicks`, and clears `G0311_i_ProjectileDisabledMovementTicks`.
- `MOVESENS.C:764-818` updates `G0362_l_LastPartyMovementTime`/scents on true party square changes, then runs source-leave and destination-enter sensor processing.
- `GAMELOOP.C:46-50,150-155,164-219` sets the per-platform vblank threshold (`10` for PC34 lineage), redraws from party state before the wait loop, advances game time/cooldowns once per loop, then drains keyboard and calls `F0380` until stop-wait/ticking gates allow the next frame.
- `VBLANK.C:60-62` and `IO.C:772-778` prove the same vblank counter drives `G0321_B_StopWaitingForPlayerInput`.
- `ENTRANCE.C:322-357` uses `G0317_i_WaitForInputVerticalBlankCount` only for entrance-door animation pacing; it is not a separate party movement acceptance gate.

## Firestaff/runtime boundary

- Firestaff seams checked: `dm1_v1_movement_pipeline_pc34_compat.c` exposes the one-tick process and cooldown decrement seam; `dm1_v1_movement_timing_pc34_compat.c` models successful-step timing and per-loop cooldown decrement.
- Runtime-supported Firestaff route remains covered by pass339b/pass349 (`MOVEMENT_PROVED`, `FULL_LAUNCHER_KEYPAD_RUNTIME_ROUTE_PROVED`).
- Original DOSBox/Dunview route remains explicitly blocked for this proof: pass211 is `BLOCKED_SEMANTIC_ROUTE_NOT_PROMOTABLE`; pass243 is `BLOCKED_DUNVIEW_TCPP101_DOSBOX_VARIANTS_EXHAUSTED`.

## Verification

```sh
python3 tools/verify_pass383_dm1_v1_movement_timing_integration.py
ctest --test-dir ~/.openclaw/data/firestaff-builds/pass383-verify --output-on-failure -R 'dm1_v1_command_movement_sensor_timing_pc34_compat|dm1_v1_movement_pipeline_pc34_compat'
git diff --check
```

Artifacts:

- `tools/verify_pass383_dm1_v1_movement_timing_integration.py`
- `parity-evidence/verification/pass383_dm1_v1_movement_timing_integration/manifest.json`
