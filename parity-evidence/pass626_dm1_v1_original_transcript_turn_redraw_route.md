# Pass626 - DM1 V1 original transcript turn/redraw route

Status: PASS626_DM1_V1_ORIGINAL_TRANSCRIPT_TURN_REDRAW_ROUTE_LOCKED

This gate source-locks the pass625 target row from queue pop through C002 turn-right state mutation, next redraw, and PC/I34E viewport present. It does not run DOSBox and does not promote original-vs-Firestaff pixel parity.

## ReDMCSB source evidence
- PASS COMMAND.C:2045-2156 f0380_dequeues_c002_and_dispatches_turn_handler - the target row's C002 queue pop must dispatch through F0380 into F0365, not directly to a viewport crop
- PASS CLIKMENU.C:142-173 f0365_turn_right_sets_wait_stop_and_new_direction - C002 turn-right advances direction by +1, stops the wait loop, and brackets the direction change with party sensor removal/addition
- PASS CHAMPION.C:117-130 f0284_commits_party_direction_before_redraw - F0365's computed direction is committed to G0308 before the next game-loop F0128 draw consumes it
- PASS GAMELOOP.C:90-219 main_loop_dispatches_then_next_iteration_redraws_current_tuple - after F0365 sets the wait-stop flag, the bounded wait loop exits and the next iteration draws the updated G0308/G0306/G0307 tuple
- PASS DUNVIEW.C:8318-8610 f0128_consumes_target_tuple_and_calls_present - the post-turn tuple is the tuple consumed by F0128 before viewport present
- PASS DRAWVIEW.C:709-858 i34e_f0097_blits_composed_viewport_to_screen - the crop boundary remains the PC/I34E VIDRV_09_BlitViewPort present call, not a pre-present buffer snapshot

## Locked target row
- label=02_turn_right_west_1_3 input=M12_MENU_INPUT_RIGHT command=2 C002_COMMAND_TURN_RIGHT
- partyBefore={'direction': 2, 'map': 0, 'x': 1, 'y': 3} partyAfter={'direction': 3, 'map': 0, 'x': 1, 'y': 3}
- pass625 status=PASS625_DM1_V1_ORIGINAL_TRANSCRIPT_ROW_PREFLIGHT_LOCKED ok=True

## Required original runtime transcript events
- queued C002 with queue index/count evidence
- F0380 dequeued C002 from the same queue slot
- F0365 received C002 and called F0284 with normalized direction 3
- post-dispatch party tuple is map=0 x=1 y=3 direction=3
- next F0128 consumed direction=3 x=1 y=3
- F0097 reached the PC/I34E VIDRV_09_BlitViewPort boundary before crop hash capture

## Decision

The pass625 target row is now narrowed to a source-backed C002 turn-right route: original capture still must provide runtime proof for queue write/pop, F0380->F0365, F0284 direction commit to 3 at map 0 x 1 y 3, the following F0128 tuple, and the F0097/VIDRV present boundary before any original-vs-Firestaff pixel claim is valid.

## Non-claims
- no original DOS runtime capture was run
- no original-vs-Firestaff pixel parity is promoted
- no renderer, movement, or input behavior is changed
- no non-N2 original asset path is used
- no push, tag, package, or release action
