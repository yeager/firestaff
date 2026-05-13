# Pass511 - DM1 V1 original transcript blocker evidence

Status: BLOCKED_PASS511_ORIGINAL_TRANSCRIPT_STATE_DELTA_REQUIRED

DM1 V1 original route/capture promotion is now blocked specifically on an external original transcript or equivalent debugger trace. Existing route/crop artifacts and label normalization are present, but no artifact binds each captured shot to the source-visible command, stop-wait, redraw, and PC34 present boundary.

## ReDMCSB Source Audit

- COMMAND.C:55-75,2428-2456 entrance_click_is_source_command ok=True: the route enters the dungeon through the source entrance command
- COMMAND.C:106-121 pc34_movement_and_viewport_click_boxes ok=True: route input must resolve to PC34 movement or dungeon-view command boxes
- COMMAND.C:2029-2058,2138-2162 command_queue_dispatch_boundary ok=True: each shot needs dequeued command and dispatched turn/move handler proof
- CLIKMENU.C:142-174 turn_handler_sets_stop_wait_and_direction ok=True: turn captures must follow source direction mutation
- CLIKMENU.C:180-347 move_handler_sets_stop_wait_and_move_result ok=True: movement captures must follow target square and move-result handling
- GAMELOOP.C:80-98,156-220 game_loop_redraw_after_stop_wait ok=True: the redraw must consume the post-command party tuple after wait exit
- DUNVIEW.C:8318-8620 viewport_composition_for_tuple ok=True: viewport bitmap must be composed for a known direction/X/Y tuple
- DRAWVIEW.C:709-858 pc34_present_boundary ok=True: the crop seam must be the PC34 viewport present/blit boundary
- LOADSAVE.C:1936-1946 dm1_initial_hall_tuple ok=True: DM1 V1 starts from the DUNGEON.DAT initial Hall tuple

## N2 Local References

- /home/trv2/.openclaw/data/firestaff-greatstone-atlas/index/SUMMARY.md exists=True sha256=b8ee685a2b60a49f305d0f1423e329d5e1019382b53598510833a46840bc3e2d
- /home/trv2/.openclaw/data/firestaff-greatstone-atlas/raw/greatstone.free.fr__dm__d_articles_dungeon_html.html.html exists=True sha256=c24aa9436cf8ea06041add1a93ba88f00f69677584396bf6c579bfe06b621f8e
- /home/trv2/.openclaw/data/firestaff-original-games/DM/_manifests/dm_pc34_greatstone_item_by_item_diff_20260510.json exists=True sha256=506c65d3a1aad453c3040c9c0031fb7419d6ec62d5b97f621d6494906afd9494
- /home/trv2/.openclaw/data/firestaff-original-games/DM/_canonical/dm1/DUNGEON.DAT exists=True sha256=d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85
- /home/trv2/.openclaw/data/firestaff-original-games/DM/_canonical/dm1/GRAPHICS.DAT exists=True sha256=2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e
- /home/trv2/.openclaw/data/firestaff-original-games/DM/_canonical/dm1/TITLE exists=True sha256=adc7f1916eeef343849f23c047977d307495b29793b796a54aa427ba71dd3745
- /home/trv2/.openclaw/data/firestaff-original-games/DM/_canonical/dm1/Dungeon-Master_DOS_EN.zip exists=True sha256=aeb5a47f3b753206e474185f2c08b5e884dc8ddf4bd5cb82e2f28f9b7617f275

## Current Route/Capture Artifacts

- verification-screens/pass376-original-route/raw_manifest.tsv exists=True sha256=0577489da1e9fb9fa6aeba890c248dd0aa40d83206badaa5eaf7c9537f3b5924
- verification-screens/pass376-original-route/original_viewport_shot_labels.tsv exists=True sha256=e8c2c58587c92f0e5daca764cfa475caf97cb9755381dce9fd1d219b24a9d313
- verification-screens/pass376-original-route/pass80_original_frame_classifier.json exists=True sha256=fc7b8bca3be798559e2e514ab1fb280f4358c6b5d520664c00626abb9a103a46
- verification-screens/pass487-n2-original-pc34-click-primitives-route/raw_manifest.tsv exists=True sha256=c90500461761cb06258a0b7810c9397ddd2f815a6edcfdbb2c71aad54f676db6
- verification-screens/pass487-n2-original-pc34-click-primitives-route/original_viewport_shot_labels.tsv exists=True sha256=b59bd5edfbd6e572da45001e8714c98dd7af1ae323f25db8c71f782381175909
- verification-screens/pass505-original-overlay-mouse-route-recapture/raw_manifest.tsv exists=True sha256=c5486926a9e647e95a7430fd8670e67c97af289b721dc7366c4f9811f4fae8ad
- verification-screens/pass505-original-overlay-mouse-route-recapture/original_viewport_shot_labels.tsv exists=True sha256=499342336d641ac15f28c991b924fbb24582d23febdb484551635a0c1c120bcb

## Existing Gate Boundary

- pass435 status=BLOCKED_PASS435_SEMANTIC_ORIGINAL_ROUTE_NOT_READY ok=None;
- pass497 status=PASS497_ORIGINAL_CAPTURE_NEXT_BLOCKER_LOCKED ok=True; Capture/replay must prove a source-visible post-command state delta before overlay promotion: after entering gameplay, each movement/turn click needs a capture tied to the exact command completion/redraw boundary with either a new raw hash/region fingerprint or debugger/runtime proof that F0365/F0366 and the F0128/F0097 present path ran for that shot. The current pass487/pass492 evidence reaches gameplay but repeats the 48ed static no-state-delta frame five times and has filename/route-label drift, so those frames remain blocker evidence only.
- pass504 status=BLOCKED_PASS504_ORIGINAL_ROUTE_STATE_DELTA_DIVERSITY_NOT_PROVEN ok=True;
- pass508 status=PASS508_ORIGINAL_ROUTE_CAPTURE_BLOCKER_EVIDENCE_TIGHTENED ok=True; Original DM1 V1 overlay/crop promotion remains blocked only at source-visible post-command state-delta proof. The route reaches gameplay, but current post-entry frames repeat the same static hash/region fingerprint and are not bound to F0380 -> F0365/F0366 -> subsequent F0128 -> F0097/VIDRV for each route label.
- pass510 status=PASS510_ORIGINAL_CAPTURE_ROUTE_LABEL_FILENAME_FIXTURE ok=True;

## Transcript Search

- No external original transcript/state-delta transcript artifact found under this worktree, the N2 DM originals, or the N2 Greatstone atlas mirror.

## Next Evidence Required

- record each shot command id plus X/Y from F0380
- record matching F0365 or F0366 handler hit and G0321 write
- record later F0128 direction/X/Y/map tuple
- record F0097/VIDRV present before screenshot acceptance
- rerun pass435/pass504/pass508 after transcript capture

## Non-Claims

- no DOSBox/emulator behavior guessed from screen images
- no original-vs-Firestaff pixel parity claim
- no movement or viewport implementation change
- no promotion of duplicate/static captures
