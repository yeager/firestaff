# Pass340 — DM1 V1 direct enqueue wrapper seam

Status: **CLOSED with minimal runtime API.**

## Result

Added a first-class direct command wrapper that injects already-resolved ReDMCSB command ids into the existing DM1 V1 command queue and pipeline:

- `DM1_V1_InputCommandQueue_EnqueueCommandPc34Compat(queue, command, x, y)`
- `DM1_V1_MovementPipeline_EnqueueCommandPc34Compat(pipeline, command, x, y)`

This bypasses OS/keypad delivery and keeps the source-locked path after command resolution: queue capacity, F0380 dequeue/gates, F0365 turn dispatch, F0366 step dispatch, movement cooldowns, post-move resolution, and viewport dirty provenance.

## ReDMCSB source audit first

Source root audited: `/home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source`.

### `COMMAND.C`

- `COMMAND.C:6-12` defines `G0432_as_CommandQueue`, queue count/indexes, lock, and pending-click state.
- `COMMAND.C:636-685` defines the I34E/I34M movement keyboard table; `COMMAND.C:677-684` maps normalized keys `0x004B..0x0051` to movement commands `C001..C006`.
- `COMMAND.C:1452-1690` (`F0359_COMMAND_ProcessClick_CPSC`) queues an already-resolved command id into `G0432_as_CommandQueue` or records one pending click while locked.
- `COMMAND.C:1709-1813` (`F0361_COMMAND_ProcessKeyPress`) resolves normalized keyboard input through primary/secondary tables, queues the matching command id, then replays pending click.
- `COMMAND.C:2045-2156` (`F0380_COMMAND_ProcessQueue_CPSC`) locks the queue, checks empty/movement-disabled gates, dequeues one command, unlocks/replays pending click, and dispatches turns/moves.
- `COMMAND.C:2151` dispatches `C001..C002` to `F0365_COMMAND_ProcessTypes1To2_TurnParty`.
- `COMMAND.C:2155` dispatches `C003..C006` to `F0366_COMMAND_ProcessTypes3To6_MoveParty`.

### `INPUT.C` and `GAMELOOP.C`

- `INPUT.C:298-751` (`F0543_INPUT_DeviceInterruptHandler`) is the raw keyboard/mouse interrupt route; direct wrapper bypasses this.
- `INPUT.C:527-569` normalizes later-platform/PC-34 keypad-like raw input and stores normalized key codes.
- `INPUT.C:822-847` (`F1097_StoreKeyInBuffer` / `F1098_GetFirstKeyFromBuffer`) maintains the normalized key FIFO; direct wrapper bypasses this.
- `GAMELOOP.C:167` polls a key and calls `F0361_COMMAND_ProcessKeyPress(...)`.
- `GAMELOOP.C:215` calls `F0380_COMMAND_ProcessQueue_CPSC()` once per command-processing loop.

## Firestaff seam closed

- `dm1_v1_input_command_queue_pc34_compat.h/c` now exposes a named direct queue wrapper. It calls the same private queue primitive used after mouse/key command resolution and does not synthesize key events.
- `dm1_v1_movement_pipeline_pc34_compat.h/c` now exposes a named pipeline wrapper that forwards to the queue wrapper on `pipeline->commandQueue`.
- `test_dm1_v1_movement_pipeline_pc34_compat.c` now proves the wrapper can enqueue `DM1_V1_COMMAND_MOVE_FORWARD`, peek the queued command, process one tick, dequeue it, and move the party north on an open corridor.

## Decision

- **Direct queue/core injection:** NOT_BLOCKED.
- **First-class pipeline command API:** CLOSED.
- **OS keypad/NumLock route:** BYPASSED by design.
- **Original runtime/pixel parity:** not claimed.

## Verification artifacts

Manifest/logs: `parity-evidence/verification/pass340_dm1_v1_direct_enqueue_wrapper/`.

Required gates run:

- `python3 -m py_compile tools/verify_pass340_dm1_v1_direct_enqueue_wrapper.py`
- `python3 tools/verify_pass340_dm1_v1_direct_enqueue_wrapper.py`
- `cmake --build build --target test_dm1_v1_movement_pipeline_pc34_compat -j2`
- `./build/test_dm1_v1_movement_pipeline_pc34_compat`
- `git diff --check`
