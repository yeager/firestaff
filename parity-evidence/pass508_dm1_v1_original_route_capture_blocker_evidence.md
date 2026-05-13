# Pass508 - DM1 V1 original route/capture blocker evidence

Status: PASS508_ORIGINAL_ROUTE_CAPTURE_BLOCKER_EVIDENCE_TIGHTENED

Original DM1 V1 overlay/crop promotion remains blocked only at source-visible post-command state-delta proof. The route reaches gameplay from the hash-locked N2 PC34 asset set, but current post-entry frames repeat the same static hash/region fingerprint and are not bound to F0380 -> F0365/F0366 -> subsequent F0128 -> F0097/VIDRV for each route label.

## ReDMCSB source anchors

- COMMAND.C:63-72,2428-2456 - ok=True; the entrance click route is real source input, not a synthetic label
- ENTRANCE.C:739-747,850-883,939-944 - ok=True; entrance waits on F0380 and opens doors only after G0298 changes
- COMMAND.C:106-114 - ok=True; movement/viewport click coordinates must map to these PC34 zones
- COMMAND.C:2045-2156 - ok=True; a promotable shot must prove the dequeued command, not only host labels
- CLIKMENU.C:142-174 - ok=True; turn shots need F0365 direction mutation before the redraw
- CLIKMENU.C:180-347 - ok=True; move shots need F0366 destination/move-result evidence before the redraw
- GAMELOOP.C:90,164,215-219 - ok=True; the next promotable frame is after command wait exits and the next F0128 consumes the tuple
- DUNVIEW.C:8318-8611 - ok=True; F0128 must compose G0296 for the same direction/X/Y tuple
- DRAWVIEW.C:709-858 - ok=True; the crop/pixel seam must be the PC34 viewport present path

## N2 original asset locks

- pc34_executable: ok=True; bytes=11471; sha256=4c79b43276f1eb3191d496ba71f8e4c03380d252193561bc6bba6017ef554db4; capture route must launch the N2-local PC34 executable variant
- pc34_dungeon_dat: ok=True; bytes=33357; sha256=d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85; route state and map tuple evidence must bind to this exact dungeon.dat
- pc34_graphics_dat: ok=True; bytes=363417; sha256=2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e; viewport/crop evidence must bind to this exact graphics.dat
- pc34_title: ok=True; bytes=12002; sha256=adc7f1916eeef343849f23c047977d307495b29793b796a54aa427ba71dd3745; startup/entrance handoff must bind to this exact TITLE asset

## Greatstone local reference

- atlasRoot: /home/trv2/.openclaw/data/firestaff-greatstone-atlas
- pagesIndexExists=True; filesIndexExists=True; pc34DiffManifestExists=True; pc34DiffManifestSha256=506c65d3a1aad453c3040c9c0031fb7419d6ec62d5b97f621d6494906afd9494
- Greatstone remains a local secondary atlas/provenance reference; ReDMCSB source and N2-local PC34 asset hashes are the promotion boundary.

## Current evidence

- pass304: BLOCKED_ORIGINAL_PC34_VIEWPORT_CAPTURE_NOT_ROUTE_PROVEN; routeLabelCoverage=None
- pass308: PASS_CAPTURE_EXECUTED_STATE_ORACLE_PENDING; coverage={'requiredLabelCoverage': True, 'requiredPromotionRowsGameplayOrWallCloseup': True}
- pass435: BLOCKED_PASS435_SEMANTIC_ORIGINAL_ROUTE_NOT_READY
- pass487: PASS487_ORIGINAL_CLICK_ROUTE_REACHES_GAMEPLAY_STILL_LABEL_BLOCKED; classes=['entrance_menu', 'dungeon_gameplay', 'dungeon_gameplay', 'dungeon_gameplay', 'dungeon_gameplay', 'dungeon_gameplay']
- pass487 duplicate hashes: {'48ed3743ab6ac9de41689af6c1d3169a8fe00863b4552c1ed813e71c98286397': 5}
- post-entry static hash: 48ed3743ab6ac9de41689af6c1d3169a8fe00863b4552c1ed813e71c98286397; repeatedRows=True; repeatedRegions=True
- filename/route-label drift rows: 5
- pass497 next blocker: Capture/replay must prove a source-visible post-command state delta before overlay promotion: after entering gameplay, each movement/turn click needs a capture tied to the exact command completion/redraw boundary with either a new raw hash/region fingerprint or debugger/runtime proof that F0365/F0366 and the F0128/F0097 present path ran for that shot. The current pass487/pass492 evidence reaches gameplay but repeats the 48ed static no-state-delta frame five times and has filename/route-label drift, so those frames remain blocker evidence only.
- pass498 narrowed blocker: Original DM1 V1 capture is blocked specifically at the post-gameplay, post-command state-delta boundary: the next evidence must connect a real F0380-dequeued movement/turn command through F0365/F0366 to the subsequent F0128 composition and F0097/VIDRV present, and must not collapse to the repeated static 48ed gameplay frame.

## Route rows

- 01 party_ready_click_gate entrance_menu 17bd7e87815750b45e742964ffe93e0312d9bbdc45dd8e7358be0a069a6db1b8 filenameMatchesRouteLabel=True repeatedStatic=False
- 02 turn_left_click dungeon_gameplay 48ed3743ab6ac9de41689af6c1d3169a8fe00863b4552c1ed813e71c98286397 filenameMatchesRouteLabel=False repeatedStatic=True
- 03 turn_right_click dungeon_gameplay 48ed3743ab6ac9de41689af6c1d3169a8fe00863b4552c1ed813e71c98286397 filenameMatchesRouteLabel=False repeatedStatic=True
- 04 move_forward_click dungeon_gameplay 48ed3743ab6ac9de41689af6c1d3169a8fe00863b4552c1ed813e71c98286397 filenameMatchesRouteLabel=False repeatedStatic=True
- 05 move_backward_click dungeon_gameplay 48ed3743ab6ac9de41689af6c1d3169a8fe00863b4552c1ed813e71c98286397 filenameMatchesRouteLabel=False repeatedStatic=True
- 06 turn_left_2_click dungeon_gameplay 48ed3743ab6ac9de41689af6c1d3169a8fe00863b4552c1ed813e71c98286397 filenameMatchesRouteLabel=False repeatedStatic=True

## Next evidence required

- capture each route shot at or after the F0097/VIDRV present boundary following the matching command
- record the F0380 command id/X/Y and the F0365 or F0366 handler reached for that shot
- record the later F0128 direction/X/Y tuple consumed for the same shot
- reject repeated 48ed static gameplay hashes unless source state proves the command was intentionally blocked/no-op

## Non-claims

- no DOSBox run launched
- no original-vs-Firestaff pixel parity
- no promotion of pass487 static frames
- no movement or viewport implementation change
