# Pass388 — DM1 V1 queue producer runtime

Status: `BLOCKED_PASS388_B_ORIGINAL_ROUTE_NEVER_HITS_ENQUEUE_BRANCH`

## Decision

runtime reached F0380 with G2153 byte sampled as zero and no G2153 write/watchpoint under producer-side arming

## Predicate summary

- `routesRanAfterArmingWithBreakpointsRetained`: `True`
- `clickProducerEntryHit`: `False`
- `pendingReplayHit`: `False`
- `keyboardProducerEntryHit`: `False`
- `queueCountWriteObserved`: `False`
- `pendingClickWriteObserved`: `False`
- `f0380Reached`: `True`
- `f0380QueueCountPositiveImmediatelyBefore`: `False`
- `f0380QueueCountSamples`: `[0, 0]`
- `dispatchReached`: `False`

## Evidence

- Manifest: `parity-evidence/verification/pass388_dm1_v1_queue_producer_runtime/manifest.json`
- Keyboard transcript: `parity-evidence/verification/pass388_dm1_v1_queue_producer_runtime/pass388_keyboard_runtime.clean.txt`
- Click transcript: `parity-evidence/verification/pass388_dm1_v1_queue_producer_runtime/pass388_click_runtime.clean.txt`
