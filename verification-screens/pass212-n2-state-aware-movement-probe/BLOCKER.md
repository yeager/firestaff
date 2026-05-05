# Pass 212 BLOCKER: entrance/menu state is not movement-ready

Date: 2026-05-05
Worktree: `/home/trv2/work/firestaff-worktrees/n2-dm1v1-merge-readiness-20260505-1228`
Primary source reference: `/home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source`

## Verdict

Pass 212 does **not** produce movement-only parity evidence.

The blocker is not raw geometry. The raw captures classify as `dungeon_gameplay` at `320x200`, but the movement frames are frozen duplicates:

- `pass80_original_frame_classifier.md`: `pass: False`
- original classifier problems:
  - `expected 4 frame classes but found 6 raw captures`
  - `duplicate raw frames detected: 1 unique sha256 value(s) repeat`
- movement six-class gate problem:
  - `duplicate raw frames detected: 1 unique sha256 value(s) repeat`
- all six movement labels resolve to the same frame:
  - sha256 prefix: `48ed3743ab6a`
  - bytes: `8017`
  - class: `dungeon_gameplay`

So the route reaches a gameplay-looking screen, but it does not prove movement state progression. A `320x200` `dungeon_gameplay` screenshot is still not parity evidence when every intended movement snapshot is the same state.

## Exact hang / state boundary

`pass118_gate.tsv` shows the entrance/menu transition explicitly:

| index | class | sha12 | meaning |
|---:|---|---|---|
| 01 | `entrance_menu` | `ceb0c2eec633` | entrance controls still visible |
| 02 | `entrance_menu` | `ceb0c2eec633` | still entrance/menu |
| 03 | `entrance_menu` | `7f34445c6d04` | still entrance/menu |
| 04 | `graphics_320x200_unclassified` | `2908c8c701b8` | transition/animation frame, not stable gameplay |
| 05 | `graphics_320x200_unclassified` | `0c198ddafde3` | transition/animation frame, not stable gameplay |
| 06 | `entrance_menu` | `d3ee1a4f1a36` | entrance/menu still observable |
| 07 | `entrance_menu` | `17bd7e878157` | entrance/menu still observable |
| 08 | `entrance_menu` | `17bd7e878157` | duplicate entrance/menu |
| 09 | `dungeon_gameplay` | `48ed3743ab6a` | first gameplay-looking frame |

`original_viewport_shot_labels.tsv` then labels six intended movement-only captures:

1. `gate_confirmed_gameplay`
2. `move_up_after_gate`
3. `turn_right_after_gate`
4. `move_left_after_gate`
5. `spell_or_party_key_after_gate`
6. `inventory_or_party_key_after_gate`

But every one of those six labels has the same sha256 prefix `48ed3743ab6a`. The route is effectively stuck at the first gameplay-looking frame after the entrance gate. It is not demonstrating that subsequent movement/input commands were accepted and rendered.

## ReDMCSB source functions involved

Relevant primary-source files and functions:

- `ENTRANCE.C`
  - `F0441_STARTEND_ProcessEntrance` loads entrance graphics, installs entrance mouse/keyboard handlers, sets `G0298_B_NewGame = C099_MODE_WAITING_ON_ENTRANCE`, then loops on `M526_WaitVerticalBlank()` and `F0380_COMMAND_ProcessQueue_CPSC()` until an entrance command changes state.
  - `F0438_STARTEND_OpenEntranceDoors` performs the door-opening animation after `G0298_B_NewGame` leaves the waiting state.
  - `F0797_STARTEND_DrawEntranceMicroDungeon` / entrance micro-dungeon draw path means viewport-like dungeon content can appear before the real movement route is safe to sample.
  - `F0022_MAIN_Delay(20)` precedes door opening after the entrance command.
- `COMMAND.C`
  - `G0445_as_Graphic561_PrimaryMouseInput_Entrance` maps Enter Dungeon to the entrance button area. For the Atari/Amiga-era entrance table this is `244..298,45..58`; later zone builds use `C407_ZONE_ENTRANCE_ENTER`.
  - `G0463_aai_Graphic561_Box_MovementArrows` maps gameplay movement arrow boxes: forward `263..289,125..145`, right `291..318,147..167`, backward `263..289,147..167`, left `234..261,147..167`.
  - `F0380_COMMAND_ProcessQueue_CPSC` rejects movement while `G0310_i_DisabledMovementTicks` or matching projectile movement delay is non-zero, then dispatches turns to `F0365_COMMAND_ProcessTypes1To2_TurnParty` and moves to `F0366_COMMAND_ProcessTypes3To6_MoveParty`.
  - entrance commands set `G0298_B_NewGame = C001_MODE_LOAD_DUNGEON` / saved-game modes.
- `STARTUP1.C` / `STARTUP2.C`
  - startup calls `F0441_STARTEND_ProcessEntrance()`, then `F0435_STARTEND_LoadGame()`, then `F0462_START_StartGame_CPSEF()`.
  - only after that does a new game run `F0267_MOVE_GetMoveResult_CPSCE(...)`; this is the first real party-placement/movement-state boundary after entrance.
- `CLIKMENU.C`
  - `F0365_COMMAND_ProcessTypes1To2_TurnParty` mutates party direction.
  - `F0366_COMMAND_ProcessTypes3To6_MoveParty` mutates party position and sets `G0310_i_DisabledMovementTicks` from champion movement speed.
- `GAMELOOP.C`
  - game loop decrements `G0310_i_DisabledMovementTicks` / `G0311_i_ProjectileDisabledMovementTicks`; captures taken before this clears can show accepted-looking input with no rendered movement progression.
- vertical blank / timing
  - `M526_WaitVerticalBlank` is the timing primitive used in the entrance wait loop and animation path. Capture routing must wait for state, not only wall-clock sleeps.

## Why fixed sleeps are not enough

The capture harness currently routes by `wait:<ms>`, `click:<x>,<y>`, key inputs, and `shot:<label>` capture markers. Fixed sleeps can land on:

1. entrance/menu screen,
2. entrance door animation / micro-dungeon frames,
3. first gameplay-looking frame before movement input has been processed,
4. a movement-disabled tick window after a move.

Pass 118 proves (1)-(3) happen in this route. Pass 212 proves (4) or an equivalent input-not-accepted condition: all intended movement frames are duplicates.

## Minimal state-aware movement route candidate

Not run in this pass, because this task forbids creating PNG/PPM files and the pass212 raw PNGs are not present in the worktree. This is a candidate route shape for the next run on a capture host:

```sh
OUT_DIR="$PWD/verification-screens/pass213-n2-state-aware-movement-only" \
DM1_ORIGINAL_STAGE_DIR="$PWD/verification-screens/dm1-dosbox-capture/DungeonMasterPC34" \
DM1_ORIGINAL_PROGRAM="DM VGA" \
DM1_ROUTE_SKIP_STARTUP_SELECTOR=1 \
WAIT_BEFORE_INPUT_MS=5000 \
NEW_FILE_TIMEOUT_MS=6000 \
DM1_ORIGINAL_ROUTE_EVENTS="wait:7000 enter wait:1500 click:260,50 wait:6500 shot:gate_ready_gameplay click:276,135 wait:1200 shot:move_forward click:304,157 wait:1200 shot:turn_right click:247,157 wait:1200 shot:turn_left click:276,157 wait:1200 shot:move_backward click:276,135 wait:1200 shot:move_forward_2" \
scripts/dosbox_dm1_original_viewport_reference_capture.sh --run
```

The important difference is not the exact sleep values. The required fix is an entrance-ready signal before the first movement command and a movement-ready wait after each movement command.

## Required fixes before this can become parity evidence

1. Add a state-aware gate to `scripts/dosbox_dm1_original_viewport_reference_capture.sh` instead of relying on fixed `wait:<ms>` after the entrance click.
   - Wait until classifier/layout says `dungeon_gameplay` and the right column is no longer `entrance_menu`.
   - Require at least one stable post-entrance gameplay frame before sending movement input.
2. Add a movement-ready wait between movement commands.
   - Either wait conservatively for enough game-loop ticks after movement, or add a harness-level predicate that detects frame change / non-duplicate sha before the next `shot:<label>`.
3. Keep `--fail-on-duplicates` mandatory for movement-only evidence.
   - Duplicate `dungeon_gameplay` frames are blocker evidence, not parity evidence.
4. Keep the pass118 gate as a preflight diagnostic.
   - It identifies whether the route is still in entrance/menu, transition, or gameplay-looking state.
5. Do not write `manifest.json` for pass212 movement frames.
   - The movement classifier did not pass, so a manifest would falsely promote duplicate frames to evidence.

## Classifier rerun status

A new classifier run was **not** performed from raw images in this pass because `verification-screens/pass212-n2-state-aware-movement-probe/image0001-raw.png` and the other raw PNG files are not present in the worktree, and creating new PNG/PPM files was explicitly disallowed for this task.

Existing committed classifier artifacts already show the failure:

- `pass80_original_frame_classifier.json` / `.md`: `pass: False`
- `pass80_movement_six_class_gate.json` / `.md`: `pass: False`
- duplicate sha256 prefix for all movement labels: `48ed3743ab6a`
