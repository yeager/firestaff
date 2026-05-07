# DM1 V1 movement + viewport integration sweep — N2 2026-05-01

Scope: N2 only (`firestaff-worker`, repo `<firestaff-repo>`). This is a source-first audit of DM1 V1 movement-to-viewport side effects after `ed83929` / `ceceeda`, focused on stepping/turning, side-lane occlusion, and object visibility after movement. No <private-host>/<private-host-ip> work was used.

Primary source: `<redmcsb-source>/ReDMCSB_WIP20210206/Toolchains/Common/Source/`.

## ReDMCSB source locks audited first

- `COMMAND.C:108-114` maps the visible movement arrows and viewport click route to `C001_COMMAND_TURN_LEFT`, `C003_COMMAND_MOVE_FORWARD`, `C002_COMMAND_TURN_RIGHT`, `C006_COMMAND_MOVE_LEFT`, `C005_COMMAND_MOVE_BACKWARD`, `C004_COMMAND_MOVE_RIGHT`, and `C080_COMMAND_CLICK_IN_DUNGEON_VIEW`.
- `COMMAND.C:397-403` maps the same six movement commands and `C080` through PC screen-relative zones (`C068`..`C073`, `C007`).
- `COMMAND.C:2150-2156` dispatches turn commands to `F0365_COMMAND_ProcessTypes1To2_TurnParty()` and step/strafe commands to `F0366_COMMAND_ProcessTypes3To6_MoveParty()`.
- `CLIKMENU.C:156-173` marks turn input as consumed, handles stairs, processes sensor remove/add around the party square, then updates party direction.
- `CLIKMENU.C:180-347` maps relative movement arrows through forward/right deltas, blocks walls/closed doors/fakewalls/groups, calls `F0267_MOVE_GetMoveResult_CPSCE()` for successful moves, and sets disabled movement/projectile ticks.
- `DUNGEON.C:1371-1391` applies relative forward/right movement to map coordinates from the current party direction.
- `MOVESENS.C:316-587` contains `F0267_MOVE_GetMoveResult_CPSCE()`, which updates party map coordinates, applies teleporter rotation/level transitions, and may redraw during pit falls.
- `DUNVIEW.C:8318-8618` draws/presents the dungeon view from the supplied direction/map coordinate; `DUNVIEW.C:8468-8542` traverses relative viewport squares D4/D3/D2/D1/D0 from far to near.
- `DUNVIEW.C:7391-7540` (`F0122_DUNGEONVIEW_DrawSquareD1L`) and `DUNVIEW.C:7559-7708` (`F0123_DUNGEONVIEW_DrawSquareD1R`) draw opaque D1 side walls and `return` before the open-cell `F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF()` branch, proving near side walls occlude farther same-lane side contents.

## Firestaff gates run

Commands:

```sh
cd <firestaff-repo>
cmake --build build --target \
  firestaff_m11_game_view_probe \
  firestaff_m11_viewport_state_probe \
  firestaff_m11_turn_viewport_orientation_probe \
  test_dm1_v1_movement_core_pc34_compat
./build/firestaff_m11_game_view_probe
./build/test_dm1_v1_movement_core_pc34_compat
./build/firestaff_m11_viewport_state_probe \
  ~/.openclaw/data/firestaff-original-games/DM/_canonical/dm1 \
  build/viewport-state
./build/firestaff_m11_turn_viewport_orientation_probe \
  ~/.openclaw/data/firestaff-original-games/DM/_canonical/dm1 \
  build/turn-viewport-orientation
python3 tools/verify_v1_viewport_side_wall_occlusion_gate.py
```

Results:

- `firestaff_m11_game_view_probe`: `603/603 invariants passed`.
- `test_dm1_v1_movement_core_pc34_compat`: `dm1V1MovementCoreInvariantOk=1`, source evidence `COMMAND.C:2045-2156; CLIKMENU.C:180-347,224-233,278-288,291-318; DUNGEON.C:1371-1391`.
- `firestaff_m11_viewport_state_probe`: PASS; wrote `build/viewport-state/dm1_viewport_state_probe.md` and `.json` from canonical DM1 `DUNGEON.DAT`/`GRAPHICS.DAT`.
- `firestaff_m11_turn_viewport_orientation_probe`: PASS; wrote `build/turn-viewport-orientation/pass127_turn_viewport_orientation_probe.md` and `.json`. It confirms start/turn/move/blocked-move snapshots resample left/center/right viewport cells from the updated party direction/map coordinate.
- `verify_v1_viewport_side_wall_occlusion_gate.py`: PASS; verifies Firestaff side contents/walls/door/ornament paths are guarded by same-lane non-open occlusion and match ReDMCSB far-to-near side draw shape.

## Notes / blocker

The movement+viewport gates are green. One pre-existing dirty file remains outside this sweep: `main_loop_m11.c` has uncommitted TITLE-intro fallback edits and emits a build warning about a preserved null character literal. I did not include that unrelated change in this evidence sweep.
