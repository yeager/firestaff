# Pass386 — DM1 V1 keyboard vs click command dispatch

Status: `BLOCKED_PASS386_F0365_F0366_NOT_REACHED_AFTER_KEYBOARD_AND_CLICK_PROBES`

## Decision

Blocked/narrowed: keyboard route reaches F0361 but no queued-command-count write is observed; panel-click route reaches F0380/G0321/F0128 but still no strict F0365/F0366 stop

This preserves `/F0365/F0366/F0128/G0321` as runtime anchors and keeps `/CLIKMENU/data`-style symbol handling inherited from pass385.

## Source audit conclusion

- ReDMCSB maps both movement-panel clicks and movement keys to the same command values consumed by `F0380`: `C001_COMMAND_TURN_LEFT`, `C002_COMMAND_TURN_RIGHT`, and `C003_COMMAND_MOVE_FORWARD`.
- `F0380` only dispatches to `F0365/F0366` after the queue is non-empty, it loads `G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].Command`, and it decrements `G2153_i_QueuedCommandsCount`.
- Runtime did not observe the queued-command-count write/decrement, so the remaining blocker is the queue-pop eligibility path before semantic handler dispatch, not the `F0365/F0366` anchors themselves.

## Runtime predicates

- `keyboardRouteRanAfterArm`: `True`
- `keyboardF0361Hit`: `True`
- `keyboardQueueCountChanged`: `False`
- `keyboardDispatchReached`: `False`
- `clickRouteRanAfterArm`: `True`
- `clickF0380Hit`: `True`
- `clickDispatchReached`: `False`
- `clickStopWaitWriteObserved`: `True`
- `clickF0128Observed`: `True`

## Evidence

- Manifest: `parity-evidence/verification/pass386_dm1_v1_keyboard_vs_click_command_dispatch/manifest.json`
- Keyboard transcript: `parity-evidence/verification/pass386_dm1_v1_keyboard_vs_click_command_dispatch/pass386_keyboard_runtime.clean.txt`
- Click transcript: `parity-evidence/verification/pass386_dm1_v1_keyboard_vs_click_command_dispatch/pass386_click_runtime.clean.txt`
