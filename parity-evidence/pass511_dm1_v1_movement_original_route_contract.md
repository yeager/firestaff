# Pass511 - DM1 V1 movement original route contract

Status: PASS511_DM1_V1_MOVEMENT_ORIGINAL_ROUTE_CONTRACT_LOCKED

## Decision

The remaining blocker is still a fresh original-runtime route transcript: prove keyboard-buffer token -> F0361 queue write -> F0380 pop -> F0365/F0366 state delta -> F0267 tuple/timing for steps -> F0128/F0097 post-command viewport boundary, then attach route-labeled original captures. Source and media admission alone do not promote runtime or pixel parity.

## ReDMCSB source audit

- PASS IO2.C:5-61 / F0540_INPUT_Crawcin - PC/I34E route evidence must start with original keyboard-buffer tokens that map arrow scancodes to DM command chars, not host-only labels.
- PASS GAMELOOP.C:164-219 / F0002_MAIN_GameLoop_CPSDF - the original route transcript has to cross keyboard-buffer drain, F0361 queue write, F0380 processing, and wait-loop boundary.
- PASS COMMAND.C:1734-1812 / F0361_COMMAND_ProcessKeyPress - a semantic route label needs a real original queue write to G0432/G2153.
- PASS COMMAND.C:2045-2156 / F0380_COMMAND_ProcessQueue_CPSC - movement proof must bind the queued token to original F0380 pop and F0365/F0366 dispatch.
- PASS CLIKMENU.C:142-347 / F0365_COMMAND_ProcessTypes1To2_TurnParty / F0366_COMMAND_ProcessTypes3To6_MoveParty - turn/step evidence must show source-owned state delta, collision/stairs outcome, and movement cooldown boundary.
- PASS MOVESENS.C:738-818 / F0267_MOVE_GetMoveResult_CPSCE - successful step proof must include committed tuple and timing/scent side effects.
- PASS DUNVIEW.C:8318-8611 / F0128_DUNGEONVIEW_Draw_CPSF - overlay capture is movement-meaningful only after F0128 draws from the changed direction/X/Y tuple.
- PASS DRAWVIEW.C:709-858 / F0097_DUNGEONVIEW_DrawViewport - the route transcript must land at viewport present seam before screenshots can become overlay evidence.

## Original PC 3.4 archive members

- PASS /home/yeager/.firestaff/data/dm1/Dungeon-Master_DOS_EN_Version-34.zip::DATA/DUNGEON.DAT size=33357 sha256=d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85 (in-memory/no-extraction)
- PASS /home/yeager/.firestaff/data/dm1/Dungeon-Master_DOS_EN_Version-34.zip::DATA/GRAPHICS.DAT size=363417 sha256=2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e (in-memory/no-extraction)
- PASS /home/yeager/.firestaff/data/dm1/Dungeon-Master_DOS_EN_Version-34.zip::TITLE size=12002 sha256=adc7f1916eeef343849f23c047977d307495b29793b796a54aa427ba71dd3745 (in-memory/no-extraction)
- PASS /home/yeager/.firestaff/data/dm1/Dungeon-Master_DOS_EN_Version-34.zip::DM.EXE size=11471 sha256=4c79b43276f1eb3191d496ba71f8e4c03380d252193561bc6bba6017ef554db4 (in-memory/no-extraction)

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
- no generated prior-gate or completion-matrix artifact is used as authority
- no viewport/wall implementation is modified

Manifest: parity-evidence/verification/pass511_dm1_v1_movement_original_route_contract/manifest.json
