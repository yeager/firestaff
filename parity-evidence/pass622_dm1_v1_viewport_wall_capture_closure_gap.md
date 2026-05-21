# Pass622 - DM1 V1 viewport wall capture-closure gap

Status: BLOCKED_PASS622_DM1_V1_VIEWPORT_WALL_CAPTURE_CLOSURE_GAP_LOCKED

Decision: The next viewport/wall closure gap after pass609 is still capture-backed, not renderer-backed: Firestaff has deterministic same-viewport fixture rows, but the original side has no source-bound command/state/redraw/present transcript and the latest crops are duplicate/non-promotable.

Primary source locks:
- DUNVIEW.C:3048-3107 wall_and_door_blits_target_g0296_viewport ok=True - wall and door evidence must be sampled from the composed viewport bitmap, not isolated wall assets
- DUNVIEW.C:8533-8542 current_square_draws_after_d1_and_d0_side_squares ok=True - D0C/current-square closure must be paired to the same F0128 order that already source-locks side and center walls
- DUNVIEW.C:8318-8610 f0128_consumes_tuple_then_requests_present ok=True - the accepted crop must bind to the same direction/X/Y tuple consumed by F0128
- DRAWVIEW.C:709-858 f0097_presents_g0296_viewport ok=True - original captures are promotable only at or after the source viewport-present boundary

Pass608 same-viewport blocker:
- status: BLOCKED_PASS608_DM1_V1_SAME_VIEWPORT_CAPTURE_NOT_PROMOTABLE
- runtimeTranscriptProvided: False
- runtimeTranscriptOk: False
- firestaffStateCount: 6
- firestaffViewportHashCount: 6
- duplicateOriginalCropGroups: 1

Pass609 contract:
- statusLocked: True
- currentAttemptNonPromotable: True
- sourceLockRowCount: 5

Promotion requires:
- one original runtime transcript row with F0380 dequeue/count delta for the sampled route label
- matching F0365/F0366 state mutation or source-visible blocked/no-op proof for that row
- later F0128 direction/X/Y tuple and F0097/VIDRV present boundary for the same original frame
- one Firestaff viewport frame with the same map/X/Y/direction/wall-door tuple and reproducible viewport hash
- no duplicate viewport crop hash across labels unless the manifest proves the labels share the same semantic tuple

Blocker:
- missing original runtime transcript row paired to a Firestaff viewport frame for the same map/X/Y/direction/wall-door tuple

Non-claims:
- no new original DOSBox capture was run
- no original-vs-Firestaff pixel parity is promoted
- no renderer behavior change
- no V2/CSB/DM2 scope
- no DANNESBURK use

Manifest: parity-evidence/verification/pass622_dm1_v1_viewport_wall_capture_closure_gap/manifest.json
