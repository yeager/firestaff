# Pass624 - DM1 V1 original transcript row preflight

Status: FAIL_PASS625_DM1_V1_ORIGINAL_TRANSCRIPT_ROW_PREFLIGHT

This gate narrows the pass622 blocker to one original runtime transcript row for `02_turn_right_west_1_3`. It does not run DOSBox and does not promote original-vs-Firestaff parity.

## Source evidence
- PASS COMMAND.C:341-352 i34e_entrance_enter_mouse_command - the original PC/I34E entrance click must resolve to C200 before gameplay capture labels are accepted
- PASS COMMAND.C:551-571 i34e_entrance_enter_keyboard_command - the original PC/I34E keyboard entrance route has a source command id distinct from later movement commands
- PASS COMMAND.C:396-405 i34e_movement_mouse_commands - post-entry movement labels must use the source movement command ids consumed by F0380
- PASS COMMAND.C:1452-1661 mouse_queue_write_with_coordinates - a mouse-driven transcript row must record the G0432 command/x/y write and queued-count delta
- PASS COMMAND.C:1709-1813 keyboard_queue_write - a keyboard-driven transcript row must record the same queue boundary before F0380 dispatch
- PASS COMMAND.C:2045-2156 queue_pop_dispatch_turn_or_step - the transcript row must bind the queue pop to the exact turn/step handler
- PASS GAMELOOP.C:90-219 game_loop_redraw_after_dispatch - a promotable crop must be sampled after the bounded command wait loop reaches the next F0128 draw
- PASS DUNVIEW.C:8318-8610 viewport_tuple_consumed_by_f0128 - the transcript row must include the direction/x/y tuple consumed by F0128
- PASS DRAWVIEW.C:709-858 pc_viewport_present_boundary - the original screenshot/crop must be taken at or after the viewport-present boundary

## N2 original asset locks
- PASS TITLE bytes=12002 sha256=adc7f1916eeef343849f23c047977d307495b29793b796a54aa427ba71dd3745
- PASS GRAPHICS.DAT bytes=363417 sha256=2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e
- PASS DUNGEON.DAT bytes=33357 sha256=d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85
- PASS DungeonMasterPC34/DM.EXE bytes=11471 sha256=4c79b43276f1eb3191d496ba71f8e4c03380d252193561bc6bba6017ef554db4
- FAIL DungeonMasterPC34/VGA bytes=None sha256=None
- FAIL DungeonMasterPC34/SELECTOR bytes=None sha256=None
- FAIL Dungeon-Master_DOS_EN.zip bytes=None sha256=None

## Target transcript row
- label=02_turn_right_west_1_3 input=M12_MENU_INPUT_RIGHT command=2 C002_COMMAND_TURN_RIGHT
- partyAfter={'map': 0, 'x': 1, 'y': 3, 'direction': 3} firestaffViewportSha256=1e71ed8799806ff0594943c52a0a99a12c3f6f441888a750f7f6be0f7c2c6d81
- pass623 row ok=True

## Consumed gates
- PASS pass608_same_viewport_blocker observed=BLOCKED_PASS608_DM1_V1_SAME_VIEWPORT_CAPTURE_NOT_PROMOTABLE
- PASS pass622_viewport_wall_capture_gap observed=BLOCKED_PASS622_DM1_V1_VIEWPORT_WALL_CAPTURE_CLOSURE_GAP_LOCKED
- FAIL pass623_input_capture_bridge observed=FAIL_PASS623_DM1_V1_INPUT_CAPTURE_READINESS_BRIDGE

## Required original transcript fields
- runId
- routeLabel
- originalAssetSet.sha256.GRAPHICS.DAT
- originalAssetSet.sha256.DUNGEON.DAT
- originalFrame.path
- originalFrame.rawSha256
- originalFrame.cropSha256
- originalFrame.width
- originalFrame.height
- input.source
- input.token
- input.sourceCommandId
- commandQueue.sourceFunction
- commandQueue.command
- commandQueue.countBefore
- commandQueue.countAfter
- commandQueue.firstIndexBefore
- commandQueue.firstIndexAfter
- dispatch.sourceFunction
- dispatch.handler
- partyBefore.mapIndex
- partyBefore.mapX
- partyBefore.mapY
- partyBefore.direction
- partyAfter.mapIndex
- partyAfter.mapX
- partyAfter.mapY
- partyAfter.direction
- redraw.sourceFunction
- redraw.mapX
- redraw.mapY
- redraw.direction
- present.sourceFunction
- present.viewportPresented
- present.boundary
- firestaffFrame.mapIndex
- firestaffFrame.mapX
- firestaffFrame.mapY
- firestaffFrame.direction
- firestaffFrame.viewportSha256

## Decision

The next original capture attempt has a machine-checked, source-backed target row: one original PC/I34E transcript for 02_turn_right_west_1_3 must prove queue write/pop, F0380->F0365 dispatch, party tuple 0/1/3/3, F0128 redraw, and F0097/VIDRV viewport present before its crop can be paired with the locked Firestaff viewport hash.

## Non-claims
- no original DOS runtime capture was run
- no original-vs-Firestaff pixel parity is promoted
- no renderer, movement, or input behavior is changed
- no non-N2 original asset path is used
- no push, tag, package, or release action

## Problems
- asset lock failed: DungeonMasterPC34/VGA
- asset lock failed: DungeonMasterPC34/SELECTOR
- asset lock failed: Dungeon-Master_DOS_EN.zip
- gate status drifted: pass623_input_capture_bridge
