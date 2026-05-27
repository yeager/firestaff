# Pass511 - DM1 V1 movement original route contract

Status: PASS511_DM1_V1_MOVEMENT_ORIGINAL_ROUTE_CONTRACT_LOCKED

## Decision

The next landable DM1 V1 movement step is a fresh original-runtime route transcript, not another Firestaff movement implementation patch: prove keyboard-buffer token -> F0361 queue write -> F0380 pop -> F0365/F0366 state delta -> F0267 tuple/timing for steps -> F0128/F0097 post-command viewport boundary, then attach route-labeled original captures.

## ReDMCSB source audit

- PASS IO2.C:5-61 / F0540_INPUT_Crawcin - PC/I34E route evidence must start with original keyboard-buffer tokens that map arrow scancodes to DM command chars, not host-only labels.
- PASS GAMELOOP.C:164-219 / F0002_MAIN_GameLoop_CPSDF - the original route transcript has to cross keyboard-buffer drain, F0361 queue write, F0380 processing, and wait-loop boundary.
- PASS COMMAND.C:1734-1812 / F0361_COMMAND_ProcessKeyPress - a semantic route label needs a real original queue write to G0432/G2153.
- PASS COMMAND.C:2045-2156 / F0380_COMMAND_ProcessQueue_CPSC - movement proof must bind the queued token to original F0380 pop and F0365/F0366 dispatch.
- PASS CLIKMENU.C:142-347 / F0365_COMMAND_ProcessTypes1To2_TurnParty / F0366_COMMAND_ProcessTypes3To6_MoveParty - turn/step evidence must show source-owned state delta, collision/stairs outcome, and movement cooldown boundary.
- PASS MOVESENS.C:738-818 / F0267_MOVE_GetMoveResult_CPSCE - successful step proof must include committed tuple and timing/scent side effects.
- PASS DUNVIEW.C:8318-8611 / F0128_DUNGEONVIEW_Draw_CPSF - overlay capture is movement-meaningful only after F0128 draws from the changed direction/X/Y tuple.
- PASS DRAWVIEW.C:709-858 / F0097_DUNGEONVIEW_DrawViewport - the route transcript must land at viewport present seam before screenshots can become overlay evidence.

## Required prior gates

- PASS pass504_keyboard_buffer_state_delta_blocker -> PASS504_KEYBOARD_BUFFER_STATE_DELTA_BLOCKER_LOCKED
- PASS pass505_blocked_collision_timing -> PASS505_DM1_V1_BLOCKED_MOVEMENT_COLLISION_TIMING_SOURCE_LOCKED
- PASS pass506_stairs_side_effects -> PASS506_DM1_V1_STAIRS_MOVEMENT_SIDE_EFFECT_SOURCE_LOCK_PROVEN
- PASS pass508_key_route_state_delta -> PASS508_DM1_V1_KEY_ROUTE_STATE_DELTA_GATE_LOCKED
- PASS pass509_keyboard_buffer_blocker -> PASS509_ORIGINAL_OVERLAY_KEYBOARD_BUFFER_BLOCKER_LOCKED
- PASS pass510_route_label_filename_fixture -> PASS510_ORIGINAL_CAPTURE_ROUTE_LABEL_FILENAME_FIXTURE

## Original/Greatstone anchors

- PASS canonicalDm1DungeonDat /Users/bosse/.openclaw/data/firestaff-original-games/DM/_canonical/dm1/DUNGEON.DAT sha256=d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85
- PASS canonicalDm1GraphicsDat /Users/bosse/.openclaw/data/firestaff-original-games/DM/_canonical/dm1/GRAPHICS.DAT sha256=2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e
- PASS canonicalDm1Title /Users/bosse/.openclaw/data/firestaff-original-games/DM/_canonical/dm1/TITLE sha256=adc7f1916eeef343849f23c047977d307495b29793b796a54aa427ba71dd3745
- PASS canonicalDm1Readme /Users/bosse/.openclaw/data/firestaff-original-games/DM/_canonical/dm1/README.md sha256=6bffffe6a147d34c9273975fa9f3e6888c3c690466ba1157cbfa17300baee056
- PASS greatstoneOverview /Users/bosse/.openclaw/data/firestaff-greatstone-atlas/raw/greatstone.free.fr__dm__g_dm.html.html sha256=7fdd2c8daef24250d58bc35632e245def338c0e63cf3832ce9af6534da54896c
- PASS greatstonePc34DiffManifest /Users/bosse/.openclaw/data/firestaff-original-games/DM/_manifests/dm_pc34_greatstone_item_by_item_diff_20260510.json sha256=506c65d3a1aad453c3040c9c0031fb7419d6ec62d5b97f621d6494906afd9494

## Artifact contract for the next landing step

- per-token original PC/I34E keyboard-buffer value from IO2/F0540 or equivalent memory watch
- F0361 queue write record: command id, G0432 slot, G0434 last index, G2153 increment
- F0380 pop record: same command id, G0433 first index, G2153 decrement
- handler record: F0365 direction mutation or F0366 target/collision/stairs outcome
- for successful steps: F0267 committed map index, X, Y, direction/cell, and last-movement-time side effect
- post-command F0128/F0097 boundary record tied to the same tuple
- route-labeled original viewport/HUD captures whose filenames match route labels and whose hashes are not repeated unless source trace proves a no-op/block

## Non-claims

- no new DOSBox/FIRES capture was launched
- no original-vs-Firestaff pixel parity is claimed
- no completion percentage change is claimed
- no viewport/wall implementation is modified

Manifest: parity-evidence/verification/pass511_dm1_v1_movement_original_route_contract/manifest.json
