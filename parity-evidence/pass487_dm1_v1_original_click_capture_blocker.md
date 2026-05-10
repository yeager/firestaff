# Pass487 — original PC34 click-route capture blocker

Status: `PASS487_ORIGINAL_CLICK_ROUTE_REACHES_GAMEPLAY_STILL_LABEL_BLOCKED`

Fresh N2 DOSBox capture using source-locked PC34 click centers reached gameplay frames, but it is still not promotable overlay evidence.

## Classification
- labels: `party_ready_click_gate, turn_left_click, turn_right_click, move_forward_click, move_backward_click, turn_left_2_click`
- classes: `entrance_menu, dungeon_gameplay, dungeon_gameplay, dungeon_gameplay, dungeon_gameplay, dungeon_gameplay`
- duplicate hashes: `{'48ed3743ab6ac9de41689af6c1d3169a8fe00863b4552c1ed813e71c98286397': 5}`
- decision: route mechanism partially unblocked; source-state labeling remains blocked by repeated post-entry gameplay frames.

## Source references audited
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
- this verifier records the blocker instead of promoting stale/duplicate frames

## Non-claims
No original-vs-Firestaff pixel parity and no overlay promotion are claimed.
