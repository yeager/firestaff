# Pass609 - DM1 V1 same-viewport capture contract

Status: PASS609_DM1_V1_SAME_VIEWPORT_CAPTURE_CONTRACT_LOCKED

Decision: The current pass505 attempt remains non-promotable: labels and crop filenames are not one-to-one semantic pairs, hashes collapse to duplicate states, source-bound original stops are absent, and no paired Firestaff frame exists.

Primary source locks:
- GAMELOOP.C:90,164,215-219 game_loop_waits_for_command_then_next_f0128 ok=True - A labeled original crop must be taken after the command wait loop allows the next F0128 draw.
- COMMAND.C:2045-2156 f0380_dequeues_real_turn_or_move_command ok=True - The route label must be tied to an actual dequeued command, not only to a host-side screenshot name.
- CLIKMENU.C:142-347 turn_and_move_handlers_mutate_state_before_viewport ok=True - The compared tuple must reflect the original turn/move state mutation for that label.
- DUNVIEW.C:8318-8610 f0128_uses_tuple_then_calls_present ok=True - The viewport crop must bind to the same direction/X/Y tuple used for composition.
- DRAWVIEW.C:709-858 f0097_presents_g0296_viewport ok=True - The crop must be at or after the source present boundary for G0296.

Current attempt audit:
- routeLabelCount: 6
- cropFilenameCount: 6
- labelsAndCropsPairedByName: False
- duplicateRawHashes: True
- duplicateViewportCropHashes: True
- sourceBoundStopsPresent: False
- pairedFirestaffCapturePresent: False
- promotable: False

Future capture contract:
- one route label maps to exactly one original viewport crop filename and one Firestaff crop filename
- each route label records map, X, Y, direction, wall/door state, light/palette, and viewport crop hash
- each original crop has source-bound stops: F0380 pop -> F0365/F0366 mutation -> later F0128 tuple -> F0097/VIDRV present
- each paired Firestaff crop records the same map/X/Y/direction/wall-door tuple and reaches the local F0128-to-present path
- duplicate raw or viewport-crop hashes across semantic labels remain blockers unless the manifest proves the labels intentionally share the same tuple

Non-claims:
- no new original capture was performed
- no original-vs-Firestaff pixel parity is promoted
- no renderer behavior change
- no TODO.md update
- no DANNESBURK use
