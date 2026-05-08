# Pass387 — DM1 V1 keyboard F0361 queue write

Status: `PASS387_KEYBOARD_F0361_QUEUE_WRITE_PROVEN`

## Decision

Keyboard queue write proven after arming at the post-gameplay party-control point.

Pass386 armed before the saved-game/load suffix. This pass drives that suffix unarmed, pauses at the live gameplay point, arms `F0361_COMMAND_ProcessKeyPress` and `G2153_i_QueuedCommandsCount`, then sends movement keys. Runtime observes `F0361` followed by a `G2153` `00 -> 01` write, so the earlier no-write result was a probe timing/arming artifact, not an F0361 queue gate.

## Source audit conclusion

- ReDMCSB source locks the keyboard-buffer path into `F0361_COMMAND_ProcessKeyPress`.
- `F0361` requires a non-null primary keyboard table, checks queue capacity, scans primary/secondary keyboard tables, writes `G0432_as_CommandQueue[...]`, and increments `G2153_i_QueuedCommandsCount`.

## Runtime predicates

- `sourceAuditOk`: `True`
- `postGameplayArm`: `True`
- `routeRanAfterArm`: `True`
- `breakpointsRetainedAtArm`: `True`
- `queueCountAtArmZero`: `True`
- `f0361HitAfterGameplayArm`: `True`
- `queuedCommandCountWriteObserved`: `True`
- `anyQueuedCommandCountWriteObserved`: `True`

## Evidence

- Manifest: `parity-evidence/verification/pass387_keyboard_f0361_queue_write/manifest.json`
- Bounded runtime transcript: `parity-evidence/verification/pass387_keyboard_f0361_queue_write/pass387_keyboard_runtime.clean.txt`
- Route keylog: `parity-evidence/verification/pass387_keyboard_f0361_queue_write/pass387_keyboard_route_keylog.json`
- Debug command log: `parity-evidence/verification/pass387_keyboard_f0361_queue_write/pass387_keyboard_command_log.json`
