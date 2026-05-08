# Pass339b — DM1 V1 runtime movement gate artifact

Status: **MOVEMENT_PROVED** for the narrow runtime gate. This is not a full launcher-route proof and not pixel parity; it proves the post-table-ready DM1 V1 game view accepts the route-token-equivalent inputs `up,left,right`, mutates party facing/position, and requests redraws.

## ReDMCSB source audit

Source root: `/home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source/`.

- `COMMAND.C:106-121` defines the movement mouse input table: turn-left, forward, turn-right, left, back, right.
- `COMMAND.C:678-683` is the PC I34E/I34M keyboard movement table from pass336: left turn `0x004B`, forward `0x004C`, right turn `0x004D`, left `0x004F`, back `0x0050`, right `0x0051`.
- `COMMAND.C:1709-1813` `F0361_COMMAND_ProcessKeyPress` locks the queue, scans primary/secondary keyboard tables, stores matching commands in `G0432_as_CommandQueue`, unlocks, and replays pending clicks.
- `COMMAND.C:2029-2042` `F1053_Pre_F0380_COMMAND_ProcessQueue_CPSC` is the pre-wrapper anchor required for this pass.
- `COMMAND.C:2045-2158` `F0380_COMMAND_ProcessQueue_CPSC` locks/empties/dequeues one command, applies the movement-disabled gate at `2095-2100`, and dispatches turns at `2150-2152` and movement at `2154-2156`.
- `CLIKMENU.C:223-233` defines relative movement arrow deltas; `CLIKMENU.C:264-328` resolves stairs, wall/door/fakewall/group blockers, discards blocked input, and calls `F0267_MOVE_GetMoveResult_CPSCE` for successful steps.
- `MOVESENS.C:315-545` `F0267_MOVE_GetMoveResult_CPSCE` is the movement-result core; the party-coordinate write is at `441-443`.
- `INPUT.C:531-568` normalizes Amiga movement/raw keypad variants before `F1097_StoreKeyInBuffer`; relevant as an input-buffer contrast, not the PC I34E route used here.
- `IO2.C:47-59` maps extended PC arrow scancodes into the I34E movement key codes (`K/L/M/P`) before the command table sees them.

## Firestaff route/runtime audit

- `main_loop_m11.c:740-768` maps script route tokens `up`, `left`, and `right` to `M12_MENU_INPUT_UP`, `M12_MENU_INPUT_LEFT`, and `M12_MENU_INPUT_RIGHT`. No keypad numeric route token is used.
- `m11_game_view.c:5228-5242` maps those inputs to forward, turn-left, and turn-right commands for the active DM1 V1 game view.
- `m11_game_view.c:4880-4936` advances one runtime tick and sets action/outcome; turn/move commands produce redraw through `M11_GAME_INPUT_REDRAW` after `m11_apply_tick`.
- Existing live probe source `probes/m11/firestaff_m11_hall_walkaround_runtime_probe.c:241-255` directly calls `M11_GameView_HandleInput` with `LEFT,UP,LEFT,UP,LEFT,UP,RIGHT,RIGHT`, i.e. route-token-equivalent `left,up,left,up,left,up,right,right` after the DM1 game view has opened.

## Runtime gate result

Command run:

```sh
cmake -S . -B build-pass339b
cmake --build build-pass339b --target firestaff_m11_hall_walkaround_runtime_probe -j2
./build-pass339b/firestaff_m11_hall_walkaround_runtime_probe /home/trv2/.openclaw/data/firestaff-original-games/DM/_canonical/dm1 /tmp/pass339b-hall-out
```

Result: `PASS dm1 v1 hall walkaround runtime probe`.

Machine artifacts:

- `parity-evidence/verification/pass339b_dm1_v1_runtime_movement_gate/runtime_hall_walkaround_route.json`
- `parity-evidence/verification/pass339b_dm1_v1_runtime_movement_gate/runtime_hall_walkaround_route.md`
- `parity-evidence/verification/pass339b_dm1_v1_runtime_movement_gate/runtime_probe_command.log`
- `parity-evidence/verification/pass339b_dm1_v1_runtime_movement_gate/pipeline_test.log`
- `parity-evidence/verification/pass339b_dm1_v1_runtime_movement_gate/manifest.json`

Key proof points from the runtime JSON:

- `turn_left_east_view_changes`: result `1`, direction changes south→east, outcome `FACING UPDATED`.
- `step_west_moves_in_hall`: result `1`, position changes `0,1,3`→`0,0,3`, outcome `PARTY MOVED`.
- All nine runtime steps return `M11_GAME_INPUT_REDRAW` (`result: 1`), and the probe explicitly verifies the front cell changes after turn and step.

## Full launcher route note

I attempted full binary launcher scripting with `--script enter` and with longer `enter/down/.../enter` variants under `SDL_VIDEODRIVER=dummy`, but the current launcher smoke did not reach launch before exit (`firestaff: launch smoke failed: no launch reached before exit`). That is recorded only as a launcher-script blocker, not as a movement blocker, because `M11_GameView_OpenSelectedMenuEntry` plus the hall runtime probe reaches the post-table-ready game view and proves movement/redraw there.

Final status: **MOVEMENT_PROVED** for post-table-ready runtime movement/redraw; **BLOCKED_FULL_LAUNCHER_SCRIPT_HANDOFF** for the optional outer launcher route.
