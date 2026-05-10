# Pass487 — original PC34 click-route capture blocker

Status: `PASS487_ORIGINAL_CLICK_ROUTE_REACHES_GAMEPLAY_STILL_LABEL_BLOCKED`

Fresh N2 DOSBox capture using source-locked PC34 click centers reached gameplay frames, but it is still not promotable overlay evidence.

## Classification
- labels: `party_ready_click_gate, turn_left_click, turn_right_click, move_forward_click, move_backward_click, turn_left_2_click`
- classes: `entrance_menu, dungeon_gameplay, dungeon_gameplay, dungeon_gameplay, dungeon_gameplay, dungeon_gameplay`
- first raw SHA: `17bd7e87815750b45e742964ffe93e0312d9bbdc45dd8e7358be0a069a6db1b8`
- duplicate hashes: `{'48ed3743ab6ac9de41689af6c1d3169a8fe00863b4552c1ed813e71c98286397': 5}`
- decision: route mechanism partially unblocked; source-state labeling remains blocked by entrance/menu first frame plus repeated post-entry static/no-state-delta gameplay frames.

## Blocker findings
- first frame is still entrance/menu: `True`
- post-entry gameplay frames repeat static hash: `True`
- true-stop classification: `static_no_state_delta_after_entrance_not_movement_processor_stop`
- static/no-state-delta provenance: `pass113/pass118 classify the same 48ed3743ab6a frame family as direct-start/no-party or party-control-not-ready gameplay; pass487 route labels after entry all collapse to this hash, so clicks are not yielding source-visible movement/control deltas`
- filename/route-label drift rows: `5`

## Source references audited
- `COMMAND.C:63-72,341-353` ok=True
- `ENTRANCE.C:739-747,850-883,939-944` ok=True
- `COMMAND.C:2428-2456` ok=True
- `COMMAND.C:106-114` ok=True
- `COMMAND.C:2045-2156` ok=True
- `CLIKMENU.C:142-174` ok=True
- `CLIKMENU.C:180-347` ok=True
- `DUNGEON.C:1371-1392` ok=True
- `MOVESENS.C:316-843` ok=True
- `DUNVIEW.C:2962-3000,8318-8618` ok=True
- `DRAWVIEW.C:709-858` ok=True

## Gates
- `scripts/dosbox_dm1_original_viewport_reference_capture.sh --run` on N2 with six labeled shots
- `python3 tools/pass80_original_frame_classifier.py verification-screens/pass487-n2-original-pc34-click-primitives-route --fail-on-duplicates` retained the expected duplicate-frame blocker
- `python3 tools/verify_pass487_dm1_v1_original_click_capture_blocker.py` records entrance/menu first-frame, repeated gameplay hash, true-stop classification, and filename/route-label drift blockers
- this verifier records the blocker instead of promoting stale/duplicate frames

## Non-claims
No original-vs-Firestaff pixel parity and no overlay promotion are claimed.
