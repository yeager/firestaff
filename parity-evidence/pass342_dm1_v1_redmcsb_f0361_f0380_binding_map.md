# Pass342 — DM1 V1 ReDMCSB F0361/F0380 movement binding map

Status: **SOURCE-LOCKED BINDING MAP COMPLETE.** This pass is source-first only: it maps the ReDMCSB keyboard-buffer → command enqueue → queue dispatch → movement/turn → redraw path onto current Firestaff modules. It does not claim a new DOSBox/runtime hook or pixel parity.

Source root audited: `/home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source`.

## ReDMCSB movement input anchors

- Keyboard movement table: `COMMAND.C:252-260` plus later shared rows at `COMMAND.C:272-305` bind movement/turn key codes to `C001..C006`.
- `F0361_COMMAND_ProcessKeyPress`: `COMMAND.C:1709-1813`.
  - `COMMAND.C:1734-1737`: requires `G0443_ps_PrimaryKeyboardInput`, then locks `G0435_B_CommandQueueLocked`.
  - `COMMAND.C:1746-1751`: computes/checks the queue slot against `G0434_i_CommandQueueLastIndex` and `G0433_i_CommandQueueFirstIndex`.
  - `COMMAND.C:1754-1769`: primary keyboard table lookup and queue write into `G0432_as_CommandQueue`.
  - `COMMAND.C:1779-1793`: secondary keyboard table lookup and queue write into `G0432_as_CommandQueue`.
  - `COMMAND.C:1810-1812`: unlocks the queue and replays one pending click via `F0360_COMMAND_ProcessPendingClick`.
- `F0380_COMMAND_ProcessQueue_CPSC`: `COMMAND.C:2045-2156`.
  - `COMMAND.C:2075-2089`: locks queue, tests empty queue, unlocks/replays if empty.
  - `COMMAND.C:2095-2100`: reads command id and blocks movement commands while `G0310`/`G0311` gates apply.
  - `COMMAND.C:2118-2127`: reads command X/Y, advances `G0433_i_CommandQueueFirstIndex`, unlocks, replays pending click.
  - `COMMAND.C:2150-2156`: dispatches turn commands to `F0365` and movement commands to `F0366`.
- `F0365_COMMAND_ProcessTypes1To2_TurnParty`: `CLIKMENU.C:142-174`.
  - `CLIKMENU.C:156`: sets `G0321_B_StopWaitingForPlayerInput`.
  - `CLIKMENU.C:167-173`: stairs shortcut or sensor leave, direction mutation, sensor enter.
- `F0366_COMMAND_ProcessTypes3To6_MoveParty`: `CLIKMENU.C:180-346`.
  - `CLIKMENU.C:224-233`: movement arrow index to forward/right deltas.
  - `CLIKMENU.C:237`: sets `G0321_B_StopWaitingForPlayerInput`.
  - `CLIKMENU.C:264-276`: stairs-square special cases.
  - `CLIKMENU.C:269-323`: relative target, wall/door/fakewall/group block checks, discard-on-block, wait flag reset.
  - `CLIKMENU.C:325-346`: calls `F0267_MOVE_GetMoveResult_CPSCE` and writes movement cooldowns.
- `F0267_MOVE_GetMoveResult_CPSCE`: `MOVESENS.C:316-328` signature/source-destination contract and `MOVESENS.C:799-818` party sensor leave/enter side effects.
- Redraw/cooldown loop: `GAMELOOP.C:86-92` calls `F0128_DUNGEONVIEW_Draw_CPSF(G0308,G0306,G0307)` and `GAMELOOP.C:150-155` decrements movement cooldowns.

## Firestaff binding map

Key Firestaff functions: `DM1_V1_InputCommandQueue_ProcessOnePc34Compat`, `DM1_V1_MovementCommandCore_ProcessOnePc34Compat`, and `DM1_V1_MovementPipeline_ProcessOneTickPc34Compat`.

| ReDMCSB seam | Firestaff file/function | Status |
| --- | --- | --- |
| Keyboard code → command id (`COMMAND.C:252-305`) | `dm1_v1_input_command_queue_pc34_compat.c:33-51` `command_for_key`; command ids in `dm1_v1_input_command_queue_pc34_compat.h:8-18` | Bound for shared compat key codes and direct command ids. |
| `F0361` queue write/lock/replay (`COMMAND.C:1709-1813`) | `dm1_v1_input_command_queue_pc34_compat.c:75-111`, `153-173` | Bound at compat queue level: event enqueue resolves key/mouse command and stores it; pending click replay is modeled. |
| `F0380` empty/movement-disabled gate, dequeue, dispatch flags (`COMMAND.C:2045-2156`) | `dm1_v1_input_command_queue_pc34_compat.c:175-216` | Bound for queue gate/dequeue and turn/move dispatch classification. |
| `F0380` → `F0365/F0366` actual call chain | `dm1_v1_movement_command_core_pc34_compat.c:39-187` | Bound: queue result feeds turn or step semantics, mutating party state and stop/redraw flags. |
| `F0365` turn, stairs shortcut, sensor leave/enter | `dm1_v1_movement_command_core_pc34_compat.c:74-108`; `memory_movement_pc34_compat.c:654-704`; `memory_sensor_execution_pc34_compat.c:271-352` | Bound at compat layer. |
| `F0366` relative move/block/stairs/group/cooldown | `dm1_v1_movement_command_core_pc34_compat.c:115-187`; `memory_movement_pc34_compat.c:175-238`, `405-488`, `654-704`, `820-848`; `dm1_v1_movement_timing_pc34_compat.c:85-101` | Bound at compat layer. |
| `F0267` move result and source/destination sensor consequences | `memory_movement_pc34_compat.c:706-818`; `memory_sensor_execution_pc34_compat.c:331-352` | Bound for deterministic compat move-result/sensor processing. |
| `G0321` stop-wait and viewport redraw trigger | `dm1_v1_movement_command_core_pc34_compat.c:35-36`, `105-107`, `184-186`; `dm1_v1_movement_pipeline_pc34_compat.c:162-189` | Bound as `stopWaitingForPlayerInput` and `viewportDirty`/redraw provenance. |
| Main-loop draw and cooldown tick | `dm1_v1_movement_pipeline_pc34_compat.c:57-199`; `dm1_v1_game_loop_pc34_compat.c:67-139`; viewport source metadata in `dm1_v1_viewport_3d_pc34_compat.c:424-544` | Partially bound: compat pipeline exposes redraw/cooldown semantics; product runtime drawing remains separate from this pass. |

## Pass336/pass337b/pass338b context

- Pass336 found the table was ready but the keypad-code route was still wrong; do not rely on numeric keypad tokens as the source of truth.
- Pass337b proved direct command injection into the compat queue/core is not blocked and bypasses OS keypad/NumLock delivery.
- Pass338b audited route tokens: use plain high-level `up/down/left/right` or `key:up/key:left/key:right`; native `kp4/kp5/kp6/numlock` route tokens are absent/wrong-family.

## Exact remaining seam

The remaining unbound seam is **product/runtime integration from Firestaff scripted/SDL route input into the DM1 V1 compat movement pipeline and the live viewport redraw**. The source-equivalent compat path exists from command id through queue/core/pipeline and marks redraw dirty, but there is still no first-class product-facing `DM1_V1_MovementPipeline_EnqueueCommandPc34Compat(...)`/live game-loop bridge proving that high-level `main_loop_m11.c` route tokens drive this compat queue and present the resulting viewport in the running game view. OS keypad/NumLock is explicitly not required for that next step.

## Verification

- Verifier: `tools/verify_pass342_dm1_v1_redmcsb_f0361_f0380_binding_map.py`
- Manifest: `parity-evidence/verification/pass342_dm1_v1_redmcsb_f0361_f0380_binding_map/manifest.json`
