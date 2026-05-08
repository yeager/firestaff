# Pass345 — DM1 V1 route-to-live-viewport bridge

Status: `BRIDGE_CLOSED`

## Scope

Closed the product seam `product_runtime_route_to_compat_pipeline_live_viewport_bridge` for M11 live route movement/turn inputs. The bridge is intentionally narrow: it does not claim DOSBox/original FIRES debugger proof, pixel parity, or full launcher handoff closure.

## Source lock

ReDMCSB first, using the local ReDMCSB source tree:

- `COMMAND.C:252-260` and `COMMAND.C:272-305`: movement keyboard tables map turn/step commands (`C001..C006`) to keypad/arrow input forms.
- `GAMELOOP.C:164-219`: each loop drains keyboard input through `F0361_COMMAND_ProcessKeyPress`, then dispatches the command queue through `F0380_COMMAND_ProcessQueue_CPSC` until player input stops waiting.
- `CLIKMENU.C:142-174`: `F0365_COMMAND_ProcessTypes1To2_TurnParty` applies turn commands through sensor leave/enter and `F0284_CHAMPION_SetPartyDirection`.
- `CLIKMENU.C:180-347`: `F0366_COMMAND_ProcessTypes3To6_MoveParty` computes relative movement, checks wall/door/fakewall/group blockers, and calls `F0267_MOVE_GetMoveResult_CPSCE` for accepted movement.
- `GAMELOOP.C:150-155`: movement cooldown counters decrement at loop end.
- `MOVESENS.C:316-326`: `F0267_MOVE_GetMoveResult_CPSCE` is the movement/sensor application entry used by the compat path.

## Product bridge landed

- `m11_game_view.h:12` includes `dm1_v1_movement_pipeline_pc34_compat.h`.
- `m11_game_view.h:100-105` stores `Dm1V1MovementPipelinePc34Compat` and the last pipeline result on `M11_GameViewState`.
- `m11_game_view.c:4549-4550` initializes the pipeline during game-view initialization.
- `m11_game_view.c:4864-4881` maps live M12 route inputs (`left/right/up/down/strafe`) to DM1 V1 compat commands (`TURN_LEFT`, `TURN_RIGHT`, `MOVE_FORWARD`, `MOVE_RIGHT`, `MOVE_BACKWARD`, `MOVE_LEFT`).
- `m11_game_view.c:4883-4958` enqueues the command into `DM1_V1_MovementPipeline_EnqueueCommandPc34Compat`, processes `DM1_V1_MovementPipeline_ProcessOneTickPc34Compat` against the live M11 world party, updates live tick side effects, decrements pipeline cooldowns, and returns the pipeline dirty/dequeued result.
- `m11_game_view.c:5483-5487` routes live movement inputs through the compat pipeline and returns `M11_GAME_INPUT_REDRAW` for the viewport path.

No keyboard synthesis or NumLock workaround is used.

## Verification artifacts

- Verifier: `tools/verify_pass345_dm1_v1_route_to_live_viewport_bridge.py`
- Manifest: `parity-evidence/verification/pass345_dm1_v1_route_to_live_viewport_bridge/manifest.json`
- Runtime probe output: `parity-evidence/verification/pass345_dm1_v1_route_to_live_viewport_bridge/hall_probe/dm1_v1_hall_walkaround_runtime_probe.json`

## Gates run

- `python3 -m py_compile tools/verify_pass345_dm1_v1_route_to_live_viewport_bridge.py`
- `tools/verify_pass345_dm1_v1_route_to_live_viewport_bridge.py` -> `BRIDGE_CLOSED`
- `cmake --build build-pass345 --target firestaff_m11_hall_walkaround_runtime_probe test_dm1_v1_movement_pipeline_pc34_compat -j2`
- `/tmp/firestaff-pass345-build/test_dm1_v1_movement_pipeline_pc34_compat` -> 138 passed, 0 failed
- `/tmp/firestaff-pass345-build/firestaff_m11_hall_walkaround_runtime_probe ~/.openclaw/data/firestaff-original-games/DM/_canonical/dm1 parity-evidence/verification/pass345_dm1_v1_route_to_live_viewport_bridge/hall_probe` -> PASS

## Decision

`BRIDGE_CLOSED`: live M11 route tokens now flow into the source-locked DM1 V1 compat command pipeline and propagate a redraw result back to the live viewport path.
