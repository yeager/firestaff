# Pass337b — DM1 V1 direct movement command injection

Status: **YES — source-locked at the compat command-queue seam; no OS keypad delivery is required.**

## Decision

Firestaff already has a narrow direct-command seam for DM1 V1 movement commands:

1. Build/use a `struct Dm1V1InputCommandQueuePc34Compat` (or `pipeline.commandQueue`).
2. Enqueue one of the command enum values directly:
   - `DM1_V1_COMMAND_TURN_LEFT = 1`
   - `DM1_V1_COMMAND_TURN_RIGHT = 2`
   - `DM1_V1_COMMAND_MOVE_FORWARD = 3`
   - `DM1_V1_COMMAND_MOVE_RIGHT = 4`
   - `DM1_V1_COMMAND_MOVE_BACKWARD = 5`
   - `DM1_V1_COMMAND_MOVE_LEFT = 6`
3. Use `DM1_V1_InputCommandQueue_EnqueueMouseCommandPc34Compat(queue, command, x, y, buttonMask)` as the current public queue entry point that accepts an already-resolved command id.
4. Consume it via either:
   - `DM1_V1_InputCommandQueue_ProcessOnePc34Compat(...)` for queue-only proof, or
   - `DM1_V1_MovementCommandCore_ProcessOnePc34Compat(...)` / `DM1_V1_MovementPipeline_ProcessOneTickPc34Compat(...)` for actual turn/step dispatch.

That bypasses OS keypad delivery because the command id is already resolved before the queue path. It does **not** depend on `RawKeyConvert`, PC-34 key normalization, SDL keypad/NumLock state, or the keyboard movement table.

## ReDMCSB source audit

Source root: `/home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source`

### `INPUT.C`

- `F0543_INPUT_DeviceInterruptHandler`: `INPUT.C:298-751`
  - Raw keyboard/mouse interrupt handler.
  - PC/Amiga key routes normalize key events before queue lookup; this is the route pass337b bypasses.
- Amiga raw-key normalization and FIFO store: `INPUT.C:425-493`
  - `RawKeyConvert(...)` normalizes raw key bytes into `L1740_i_NormalizedKeyCode` and stores them in `G1045_ai_InputBufferCharacters`.
- Later-platform/PC-34-style key normalization and movement keypad remap: `INPUT.C:527-569`
  - Numeric keypad raw codes are translated to arrow/delete/help key codes, then `F1097_StoreKeyInBuffer((int16_t)L2623_l_NormalizedKeyCode)` stores the normalized key.
- `F1097_StoreKeyInBuffer`: `INPUT.C:822-834`
  - Adds a normalized key to the key FIFO.
- `F1098_GetFirstKeyFromBuffer`: `INPUT.C:836-847`
  - Removes the next normalized key from the key FIFO.

### `COMMAND.C`

- Command queue globals: `COMMAND.C:6-12`
  - `G0432_as_CommandQueue`, queue count/indexes, and lock state.
- Movement mouse rows: `COMMAND.C:106-121` and zone-form rows `COMMAND.C:396-404`
  - Visible movement-arrow clicks resolve directly to `C001..C006` / `C080` / `C083` commands.
- PC-34 movement key rows: `COMMAND.C:636-685`
  - `MEDIA707_I34E_I34M` maps normalized PC-34 movement keys `0x004B..0x0051` to `C001..C006`.
- `F0357_COMMAND_DiscardAllInput`: `COMMAND.C:1304-1377`
  - Flushes keyboard input and clears command queue/pending input after blocked movement.
- `F0358_COMMAND_GetCommandFromMouseInput_CPSC`: `COMMAND.C:1379-1450`
  - Resolves a mouse click/table hit into a command id.
- `F0359_COMMAND_ProcessClick_CPSC`: `COMMAND.C:1452-1690`
  - Mouse command path: if not locked, stores the already-resolved command id into `G0432_as_CommandQueue`; if locked, records one pending click.
- `F0360_COMMAND_ProcessPendingClick`: `COMMAND.C:1692-1707`
  - Replays a deferred click after queue unlock.
- `F0361_COMMAND_ProcessKeyPress`: `COMMAND.C:1709-1813`
  - Keyboard command path: looks up normalized key code in primary/secondary `KEYBOARD_INPUT` tables, then writes the matching command into the queue.
- `F0380_COMMAND_ProcessQueue_CPSC`: `COMMAND.C:2045-2156`
  - Locks the queue, checks empty/movement-disabled gates, dequeues one command, unlocks/replays pending clicks, then dispatches turns to `F0365_COMMAND_ProcessTypes1To2_TurnParty` and movement commands to `F0366_COMMAND_ProcessTypes3To6_MoveParty`.

## Firestaff files inspected

- `dm1_v1_input_command_queue_pc34_compat.h:8-18` exposes movement command enum ids matching ReDMCSB `C001..C006`.
- `dm1_v1_input_command_queue_pc34_compat.h:66-86` exposes queue init/discard/event enqueue/direct command enqueue/process/peek functions.
- `dm1_v1_input_command_queue_pc34_compat.c:75-88` has the private `enqueue_command(...)` primitive that stores an already-resolved command in the queue.
- `dm1_v1_input_command_queue_pc34_compat.c:135-150` exposes `DM1_V1_InputCommandQueue_EnqueueMouseCommandPc34Compat(...)`, which currently accepts an explicit `command` argument and calls `enqueue_command(...)` without OS key delivery.
- `dm1_v1_input_command_queue_pc34_compat.c:175-215` consumes queued commands through movement-disabled/projectile gates and reports dispatched turn/move flags.
- `dm1_v1_movement_command_core_pc34_compat.c:39-187` consumes the queue through `DM1_V1_MovementCommandCore_ProcessOnePc34Compat(...)` and applies turn/step semantics.
- `dm1_v1_movement_pipeline_pc34_compat.h:97-132` exposes pipeline init, input enqueue, and one-tick process functions; `dm1_v1_movement_pipeline_pc34_compat.h:49` exposes `commandQueue` inside the pipeline struct.
- `dm1_v1_movement_pipeline_pc34_compat.c:40-57` initializes the pipeline and only exposes SDL/input-event enqueue as the named pipeline API.
- `dm1_v1_movement_pipeline_pc34_compat.c:57-184` processes one tick by calling the movement command core.
- Probe/evidence context checked:
  - `firestaff_memory_graphics_dat_original_input_command_queue_probe.c`
  - `firestaff_memory_graphics_dat_original_main_loop_command_loop_probe.c`
  - `firestaff_memory_graphics_dat_original_main_loop_command_probe.c`
  - `firestaff_memory_graphics_dat_original_main_loop_command_queue_probe.c`
  - `firestaff_memory_graphics_dat_original_typed_command_queue_probe.c`
  - `parity-evidence/pass333_dm1_v1_keypad_mode_command_queue_probe.md`
  - `parity-evidence/pass335_dm1_v1_keyboard_table_route_readiness.md`

## Exact blocker status

- **Direct queue/core injection:** **NOT BLOCKED.** Use `DM1_V1_InputCommandQueue_EnqueueMouseCommandPc34Compat(queue, DM1_V1_COMMAND_MOVE_FORWARD, x, y, DM1_V1_BUTTON_LEFT)` or any other `DM1_V1_COMMAND_*` movement id, then process the queue/core/pipeline tick.
- **First-class pipeline command API:** **MINOR API GAP, not a parity blocker.** There is no named `DM1_V1_MovementPipeline_EnqueueCommandPc34Compat(...)` wrapper. A product-facing caller either reaches `pipeline.commandQueue` and calls the queue API, or adds a tiny wrapper later. Pass337b intentionally does not modify product runtime.
- **OS keypad/NumLock route:** **BYPASSED.** This path avoids the failed pass337 route entirely; it does not prove SDL/OS keypad delivery.

## Minimal direct-injection sketch

```c
struct Dm1V1MovementPipelinePc34Compat pipe;
DM1_V1_MovementPipeline_InitPc34Compat(&pipe);
DM1_V1_InputCommandQueue_EnqueueMouseCommandPc34Compat(
    &pipe.commandQueue,
    DM1_V1_COMMAND_MOVE_FORWARD,
    0,
    0,
    DM1_V1_BUTTON_LEFT);
DM1_V1_MovementPipeline_ProcessOneTickPc34Compat(
    &pipe,
    dungeon,
    things,
    party,
    footwearIcons,
    &result);
```

This is direct command injection into the DM1 V1 compat movement queue. The coordinates/button are inert for `C001..C006` movement semantics after the command id is queued; they are retained only because the ReDMCSB queue carries command `X/Y` fields for mouse-derived commands.

## Verification

Run:

```sh
python3 tools/verify_pass337b_dm1_v1_direct_command_injection.py
python3 -m py_compile tools/verify_pass337b_dm1_v1_direct_command_injection.py
```
