# Pass391 — DM1 V1 queued command dispatch

Status: `PASS391_KEYBOARD_QUEUE_TO_F0380_DISPATCH_PROVEN`

## Decision

Keyboard queued-command dispatch chain proven.

## Runtime predicates

- `sourceAuditOk`: `True`
- `postGameplayRuntimeRan`: `True`
- `routeInputAfterArming`: `True`
- `initialBreakpointsRetainedAtArm`: `True`
- `queueCountAtArmZero`: `True`
- `f0361HitAfterArm`: `True`
- `g2153IncrementObserved`: `True`
- `consumerBreakpointsArmedAfterIncrement`: `True`
- `f0380EntryBreakpointObservedAfterQueueWrite`: `False`
- `f0380PopLoadAfterQueueWriteObserved`: `True`
- `g2153DecrementPopLoadObserved`: `True`
- `f0365OrF0366DispatchObserved`: `True`

## Evidence

- Manifest: `parity-evidence/verification/pass391_dm1_v1_queued_command_dispatch/manifest.json`
- Transcript: `parity-evidence/verification/pass391_dm1_v1_queued_command_dispatch/pass391_runtime.clean.txt`
- Route keylog: `parity-evidence/verification/pass391_dm1_v1_queued_command_dispatch/pass391_route_keylog.json`
- Command log: `parity-evidence/verification/pass391_dm1_v1_queued_command_dispatch/pass391_command_log.json`
