# Pass623 - DM1 V1 input capture readiness bridge

Status: FAIL_PASS623_DM1_V1_INPUT_CAPTURE_READINESS_BRIDGE

This gate binds the Firestaff input script to movement queue dispatch and viewport crop rows, while keeping the original-side blocker explicit.

## ReDMCSB source audit
- PASS COMMAND.C:106-121 pc34_movement_mouse_boxes_map_to_c001_c006 - movement-arrow input resolves to the same C001..C006 command ids used by the Firestaff script rows
- PASS COMMAND.C:1452-1661 f0359_queues_mouse_command_with_coordinates - accepted mouse input enters G0432 with command and coordinates before dispatch
- PASS COMMAND.C:1709-1813 f0361_queues_keyboard_command - keyboard input has the same queue/count boundary that a promotable original transcript must show
- PASS COMMAND.C:2045-2156 f0380_pops_and_dispatches_turn_or_step - the queue pop must be paired to F0365/F0366 before any post-command viewport row is promotable
- PASS GAMELOOP.C:90-219 game_loop_draws_tuple_then_waits_for_command_dispatch - an original transcript must tie the command pop to the later F0128 tuple draw in the same bounded route
- PASS DUNVIEW.C:8318-8610 f0128_consumes_tuple_and_calls_present - the viewport crop row must bind to the exact direction/X/Y consumed by F0128
- PASS DRAWVIEW.C:709-858 f0097_presents_the_composed_viewport - promotable original crops must be sampled at or after the viewport-present boundary

## Firestaff route audit
- FAIL src/engine/m11_game_view.c:6162-6225 m11_input_maps_to_dm1_v1_commands - Firestaff route tokens enter the DM1 V1 queue as source command ids and process one compat tick
- FAIL src/engine/m11_game_view.c:6229-6297 m11_records_movement_pipeline_capture_state - capture rows can distinguish turn, step, blocked no-op, dirty viewport, and dequeued command
- PASS probes/m11/firestaff_m11_wall_collision_capture_probe.c:20-174,210-236 wall_collision_probe_emits_input_script_and_viewport_crops - the Firestaff-side probe is an input-script capture manifest, not only a screenshot dumper

## Canonical input/crop rows
- PASS 01_start_south_1_3 inputs=[] commands=[] tuple={'map': 0, 'x': 1, 'y': 3, 'direction': 2} crop=01_start_south_1_3_viewport_224x136.ppm sha256=210fa5eedd9c37172c59dd451bffa7f942c5402358ae535d841d3a8614711371
- PASS 02_turn_right_west_1_3 inputs=['M12_MENU_INPUT_RIGHT'] commands=[2] tuple={'map': 0, 'x': 1, 'y': 3, 'direction': 3} crop=02_turn_right_west_1_3_viewport_224x136.ppm sha256=1e71ed8799806ff0594943c52a0a99a12c3f6f441888a750f7f6be0f7c2c6d81
- PASS 03_blocked_west_wall_1_3 inputs=['M12_MENU_INPUT_UP'] commands=[3] tuple={'map': 0, 'x': 1, 'y': 3, 'direction': 3} crop=03_blocked_west_wall_1_3_viewport_224x136.ppm sha256=1e71ed8799806ff0594943c52a0a99a12c3f6f441888a750f7f6be0f7c2c6d81
- PASS 04_forward_south_1_4 inputs=['M12_MENU_INPUT_LEFT', 'M12_MENU_INPUT_UP'] commands=[1, 3] tuple={'map': 0, 'x': 1, 'y': 4, 'direction': 2} crop=04_forward_south_1_4_viewport_224x136.ppm sha256=25bcc97ae93881a39e4bdeffadf07f6fc7b1ac695adbfcc07b585113a8ad4b2e

## Required original transcript columns
- routeLabel
- inputToken
- sourceCommandId
- G0432WriteIndex
- G2153BeforeAfter
- F0380DequeuedCommand
- F0365OrF0366Dispatch
- postCommandMapXMapYDirection
- F0128DrawTuple
- F0097PresentFrameId
- viewportCropSha256

## Consumed gates
- PASS movement_queue_capture_closure observed=PASS_DM1_V1_MOVEMENT_QUEUE_CAPTURE_CLOSURE_LOCKED
- PASS pass564_original_transcript_gate observed=BLOCKED_PASS564_RUNTIME_TRANSCRIPT_CHAIN_MISSING
- PASS pass608_same_viewport_blocker observed=BLOCKED_PASS608_DM1_V1_SAME_VIEWPORT_CAPTURE_NOT_PROMOTABLE
- PASS pass622_viewport_wall_capture_closure_gap observed=BLOCKED_PASS622_DM1_V1_VIEWPORT_WALL_CAPTURE_CLOSURE_GAP_LOCKED

## Decision

Firestaff now has an audited bridge from M12 input tokens to DM1 V1 command ids, movement/blocked state, and 224x136 viewport crop labels. The remaining blocker is specifically original-side: capture one transcript row with the required queue/dispatch/F0128/F0097 fields for one of these route labels before promoting original-vs-Firestaff parity.

## Non-claims
- no original runtime capture was produced
- no original-vs-Firestaff pixel parity is promoted
- no movement, renderer, or input behavior is changed
- no non-N2 source path is used
- no push or release action

## Problems
- firestaff route audit failed: m11_input_maps_to_dm1_v1_commands
- firestaff route audit failed: m11_records_movement_pipeline_capture_state
