# Pass624 - DM1 V1 pending-click overwrite queue gate

Status: PASS624_DM1_V1_PENDING_CLICK_OVERWRITE_LOCKED

This gate closes one narrow movement/input queue edge: while the command queue is locked, ReDMCSB has one pending-click slot. A second locked click overwrites the first saved click, and the unlock path replays exactly one saved click.

## ReDMCSB source audit

- PASS `COMMAND.C:6-16` - command queue storage declares `G0436_B_PendingClickPresent` plus one pending X/Y/buttons tuple (`G0437_i_PendingClickX`, `G0438_i_PendingClickY`, `G0439_i_PendingClickButtonsStatus`).
- PASS `COMMAND.C:F0359_COMMAND_ProcessClick_CPSC:1452-1661` - the locked-click path writes `G0436_B_PendingClickPresent = C1_TRUE` and overwrites the same pending tuple at `COMMAND.C:1490-1493` before returning.
- PASS `COMMAND.C:F0360_COMMAND_ProcessPendingClick:1692-1707` - unlock replay clears `G0436_B_PendingClickPresent` and calls `F0359_COMMAND_ProcessClick_CPSC(G0437_i_PendingClickX, G0438_i_PendingClickY, G0439_i_PendingClickButtonsStatus)`, so only the current tuple is replayed.
- PASS `COMMAND.C:F0380_COMMAND_ProcessQueue_CPSC:2075-2127` - the queue unlock paths call `F0360_COMMAND_ProcessPendingClick` after empty/gated/dequeued states before movement dispatch.
- PASS `COMMAND.C:F0359_COMMAND_ProcessClick_CPSC:2831-2928` / `F0360_COMMAND_ProcessPendingClick:2922-2928` - the alternate locked-click path has the same single-tuple overwrite and one-click replay shape.

## Firestaff coverage

- `tests/test_dm1_v1_input_command_queue_pc34_compat.c` now stores a locked movement-arrow click, overwrites it with a second locked movement-arrow click, unlocks, and verifies that exactly one pending replay queues only the latest command.
- `tools/verify_dm1_v1_input_command_queue_source_lock.py` now source-checks the overwrite assignments and requires the regression labels.

## Verification

- `cmake -S . -B build -DCMAKE_BUILD_TYPE=Release` - passed.
- `cmake --build build --target test_dm1_v1_input_command_queue_pc34_compat -j2` - passed.
- `./build/test_dm1_v1_input_command_queue_pc34_compat` - passed (`dm1V1InputCommandQueueInvariantOk=1`).
- `python3 tools/verify_dm1_v1_input_command_queue_source_lock.py` - passed (`dm1V1InputCommandQueueSourceLockOk=1`).
- `ctest --test-dir build --output-on-failure -R "dm1_v1_input_command_queue_pc34_compat|dm1_v1_input_command_queue_source_lock"` - passed, 2/2 tests.

## Non-claims

- No original DOS runtime capture is promoted.
- No pixel/overlay parity claim is made.
- No production movement behavior is changed in this pass.
- No push, tag, package, or release action.
