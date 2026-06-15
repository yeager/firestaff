# Pass509 - DM1 V1 original overlay keyboard-buffer blocker

Status: PASS509_ORIGINAL_OVERLAY_KEYBOARD_BUFFER_BLOCKER_LOCKED

## Decision

Original DM1 V1 overlay/capture remains blocked at the keyboard-buffer and overlay-boundary seam: a replacement route must prove every keyboard-driven overlay token survives F0357 buffer discard, writes through F0361 into G0432/G2153, is popped by F0380, and reaches the matching post-command F0128/F0097 present boundary. Host route labels, repeated 48ed gameplay frames, and duplicate pass376/pass487 artifacts are blocker evidence only.

## ReDMCSB source audit

- GAMELOOP.C:164-219 / F0002_MAIN_GameLoop_CPSDF ok=True - route keys are drained into F0361 before F0380 and before the wait loop exits
- COMMAND.C:1304-1326 / F0357_COMMAND_DiscardAllInput ok=True - panel/rest/frozen transitions can flush pending keyboard input before it reaches the command queue
- COMMAND.C:1327-1375 / F0357_COMMAND_DiscardAllInput ok=True - discard keeps only release/eye-mouth queue entries, so packed keyboard tokens cannot be assumed to survive overlay boundaries
- COMMAND.C:1734-1812 / F0361_COMMAND_ProcessKeyPress ok=True - keyboard-driven overlay routes need evidence of F0361 queue write, not only host key labels
- COMMAND.C:2045-2127 / F0380_COMMAND_ProcessQueue_CPSC ok=True - capture must be after a real F0380 pop/decrement/unlock path for the relevant route token
- DUNVIEW.C:8318-8611 / F0128_DUNGEONVIEW_Draw_CPSF ok=True - a route frame is overlay-eligible only when command-state reaches viewport compose/present

## Current blocker state

- pass435 status: BLOCKED_PASS435_SEMANTIC_ORIGINAL_ROUTE_NOT_READY; blockers: ['pass376 original-route artifacts are quarantined as non-promotable duplicate/non-semantic evidence']
- pass497 status: PASS497_ORIGINAL_CAPTURE_NEXT_BLOCKER_LOCKED
- pass498 status: PASS498_ORIGINAL_CAPTURE_BLOCKER_NARROWED_TO_POST_COMMAND_STATE_DELTA
- repeated gameplay frame still present: True
- filename/route-label drift rows: 5

## Promotion predicate for the next route

- record a strict original-runtime stop or equivalent memory watch showing F0357 did not flush the route token before overlay capture
- for each keyboard token, show F0361 writes G0432_as_CommandQueue and increments G2153_i_QueuedCommandsCount
- show the same token is later popped by F0380 with G2153 decrement and first-index advance
- show the matching command reaches F0365/F0366 or the source-owned overlay handler before the shot label is captured
- show the following F0128/F0097 viewport present boundary or an explicit source-owned overlay present boundary for that same shot
- reject repeated raw/crop hashes unless source/runtime proves the command was intentionally blocked or no-op

## Non-claims

- no DOSBox launch by this verifier
- no original-vs-Firestaff pixel parity
- no promotion of pass376/pass487 screenshots
- no claim that Firestaff movement or viewport implementation is wrong

## Gate

- python3 tools/verify_pass509_dm1_v1_original_overlay_keyboard_buffer_blocker.py

Manifest: parity-evidence/verification/pass509_dm1_v1_original_overlay_keyboard_buffer_blocker/manifest.json
