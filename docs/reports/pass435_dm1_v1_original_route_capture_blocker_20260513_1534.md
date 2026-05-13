# Pass435 DM1 V1 original route/capture blocker - 2026-05-13 15:34 lane

Worktree: /home/trv2/work/firestaff-worktrees/dm1v1-original-route-capture-20260513-1534-codex

Base: d9848e834ae53134173014d862f14b2b43b1f88d (push-main-20260513-openai-fix)

Verdict: blocked, not promotable. The route source audit is locked and the supporting readiness gates are green, but pass435 still blocks on the tracked pass376 capture set because the raw/cropped images are duplicate/non-semantic route evidence. A fresh capture attempt in this lane started DOSBox but produced no raw images, so there is still no replacement six-state original route capture to promote.

## Primary ReDMCSB route audit

- COMMAND.C:63-72 defines the entrance click box: C200_COMMAND_ENTRANCE_ENTER_DUNGEON at PC34 screen box left/right/top/bottom 244,298,45,58.
- ENTRANCE.C:739-747 installs the entrance primary mouse table and entrance keyboard table before the entrance loop.
- ENTRANCE.C:857-881 waits on the entrance, processes F1/F2/Return shortcuts, and calls F0380_COMMAND_ProcessQueue_CPSC() for queued entrance input.
- COMMAND.C:2438-2440 handles C200_COMMAND_ENTRANCE_ENTER_DUNGEON by setting G0298_B_NewGame = C001_MODE_LOAD_DUNGEON.
- ENTRANCE.C:939-944 opens the entrance doors when transitioning into dungeon load.
- LOADSAVE.C:1941-1944 decodes G0278_ps_DungeonHeader->InitialPartyLocation into party mapX, mapY, direction, and map index 0. For the locked DM1 PC34 DUNGEON.DAT this is the Hall start tuple carried by prior lane evidence: map0 x=1 y=3 dir=South.
- Greatstone atlas secondary check: raw/greatstone.free.fr__dm__d_articles_dungeon_html.html.html:155-160 documents extracted dungeon XML start metadata including start_x="1".

## Primary ReDMCSB command/capture boundary

- COMMAND.C:106-114 defines the PC34 movement/click boxes used after Hall entry: left/right/forward/back/strafe and C080_COMMAND_CLICK_IN_DUNGEON_VIEW.
- COMMAND.C:2045-2156 drains the command queue, pops L1160_i_Command, then dispatches turns to F0365_COMMAND_ProcessTypes1To2_TurnParty and movement commands to F0366_COMMAND_ProcessTypes3To6_MoveParty.
- CLIKMENU.C:142-174 is the turn processor boundary: it plays the turn sound, mutates party direction, sets disabled movement ticks, and sets G0321_B_StopWaitingForPlayerInput.
- CLIKMENU.C:180-347 is the movement processor boundary: it computes relative target squares, calls the move result path, commits cooldown, and sets G0321_B_StopWaitingForPlayerInput.
- GAMELOOP.C:90 redraws the dungeon with F0128_DUNGEONVIEW_Draw_CPSF(G0308_i_PartyDirection, G0306_i_PartyMapX, G0307_i_PartyMapY) after the command wait boundary.
- DUNVIEW.C:8318-8618 composes the dungeon view into the viewport buffers and calls the viewport present path.
- DRAWVIEW.C:709-858 is the viewport present/copy path that must be tied to a known route state before a capture can become parity evidence.

## Locked data checked

- DM1 canonical DUNGEON.DAT: d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85
- DM1 canonical GRAPHICS.DAT: 2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e
- DM1 PC34 DM.EXE: 4c79b43276f1eb3191d496ba71f8e4c03380d252193561bc6bba6017ef554db4

## Gate results in this lane

- python3 -m py_compile tools/verify_pass434_dm1_v1_original_viewport_crop_readiness_gate.py passed.
- python3 -m py_compile tools/verify_pass435_dm1_v1_semantic_original_route_readiness_gate.py passed.
- python3 -m py_compile tools/verify_pass466_dm1_v1_initial_hall_c080_source_stop_capture_path.py passed.
- python3 -m py_compile tools/verify_pass497_dm1_v1_original_capture_next_blocker.py passed.
- python3 -m py_compile tools/verify_pass304_dm1_v1_original_viewport_capture_blocker_manifest.py passed.
- python3 tools/verify_pass434_dm1_v1_original_viewport_crop_readiness_gate.py returned PASS_PASS434_ORIGINAL_VIEWPORT_CROP_READINESS.
- python3 tools/verify_pass466_dm1_v1_initial_hall_c080_source_stop_capture_path.py returned PASS466_SOURCE_STOP_CAPTURE_PATH_LOCKED_TERMINAL_HUD_ROWS_READY_FOR_RECAPTURE.
- python3 tools/verify_pass497_dm1_v1_original_capture_next_blocker.py returned PASS497_ORIGINAL_CAPTURE_NEXT_BLOCKER_LOCKED.
- python3 tools/verify_pass304_dm1_v1_original_viewport_capture_blocker_manifest.py returned BLOCKED_ORIGINAL_PC34_VIEWPORT_CAPTURE_NOT_ROUTE_PROVEN with route label coverage False.
- python3 tools/verify_pass435_dm1_v1_semantic_original_route_readiness_gate.py returned BLOCKED_PASS435_SEMANTIC_ORIGINAL_ROUTE_NOT_READY.

## Capture attempt result

I ran the pass435 next-unblock capture contract in this isolated worktree after removing the local pass376 capture/crop directories. Observed output before cleanup:

- route shape accepted: 30 tokens, 6 shots, 6 labels.
- generated dosbox-original-viewports.conf, route Swift file, and route xdotool script.
- dosbox-original-viewports.log contained only: ALSA lib seq_hw.c:528:(snd_seq_hw_open) open /dev/snd/seq failed: Permission denied.
- original-viewpoint-route-keys.log stopped at: route-token wait:9000.
- no image000*-raw.png, raw_manifest.tsv, crop manifest, or pass80 classifier was produced.
- a lingering DOSBox process from this attempt was killed and the tracked pass376 files were restored, leaving the worktree clean before adding this report.

Capture command contract:

    OUT_DIR=$PWD/verification-screens/pass376-original-route DM1_ORIGINAL_STAGE_DIR=$HOME/.openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34 DOSBOX=/usr/bin/dosbox DM1_ORIGINAL_PROGRAM="DM -vv -sn -pk" DM1_ROUTE_SKIP_STARTUP_SELECTOR=1 WAIT_BEFORE_INPUT_MS=3000 NEW_FILE_TIMEOUT_MS=6000 DM1_ORIGINAL_ROUTE_EVENTS="wait:9000 enter wait:1800 one wait:1800 click:276,140 wait:2200 one wait:2500 shot:readiness_preflight wait:700 kp4 wait:900 shot:turn_left_after_vblank wait:700 kp6 wait:900 shot:turn_right_after_vblank wait:700 kp8 wait:1200 shot:forward_after_vblank wait:700 kp4 wait:900 shot:turn_left_2_after_vblank wait:700 kp6 wait:1200 shot:post_redraw_after_vblank" timeout 180 xvfb-run -a scripts/dosbox_dm1_original_viewport_reference_capture.sh --run

## Precise blocker

Pass435 cannot be unblocked by source evidence alone. The route is source-locked, and runtime dispatch evidence is already carried by pass391/pass435, but the current promotable-artifact requirement is still missing: a fresh six-shot original PC34 route capture whose raw frames and 224x136 crops are non-duplicate and match the semantic route sequence expected by pass435.

The next lane should fix the capture executor handoff first: prove the route key driver continues beyond the initial wait:9000, writes all six raw frames, then rerun tools/pass86_original_viewport_crop_manifest.py and tools/verify_pass435_dm1_v1_semantic_original_route_readiness_gate.py. Until then, pass376 remains quarantined and no original-vs-Firestaff pixel parity claim is valid.
