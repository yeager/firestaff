# Pass497 — DM1 V1 original capture next blocker

Status: `PASS497_ORIGINAL_CAPTURE_NEXT_BLOCKER_LOCKED`

This pass does not unblock original overlay parity. It locks the next blocker after the pass487/pass492 click evidence so static frames cannot be promoted by accident.

## Observed pass487/pass492 state
- classes: `entrance_menu, dungeon_gameplay, dungeon_gameplay, dungeon_gameplay, dungeon_gameplay, dungeon_gameplay`
- duplicate hashes: `{'48ed3743ab6ac9de41689af6c1d3169a8fe00863b4552c1ed813e71c98286397': 5}`
- first frame SHA: `17bd7e87815750b45e742964ffe93e0312d9bbdc45dd8e7358be0a069a6db1b8`
- post-entry static SHA: `48ed3743ab6ac9de41689af6c1d3169a8fe00863b4552c1ed813e71c98286397`
- filename/route-label drift rows: `5`
- interpretation: source-locked click primitives reach gameplay, but the post-entry captures are still static/no-state-delta blocker frames.

## ReDMCSB source audit
- `entrance-click-enters-load-dungeon` ok=True refs=COMMAND.C:63-72, ENTRANCE.C:739-747, ENTRANCE.C:850-883, COMMAND.C:2428-2456, ENTRANCE.C:939-944: entrance primary mouse table maps the enter box to C200; the entrance loop processes the command queue; C200 sets G0298_B_NewGame=C001_MODE_LOAD_DUNGEON and the doors open
- `movement-click-primitives-are-real-pc34-zones` ok=True refs=COMMAND.C:106-114: PC34 movement mouse zones are fixed: turn left/right, forward/back/strafe, and dungeon-view click
- `queued-commands-must-hit-f0365-or-f0366-before-state-delta` ok=True refs=COMMAND.C:2045-2156, CLIKMENU.C:142-174, CLIKMENU.C:180-347: F0380 drains the command queue and dispatches turns to F0365 and moves to F0366; F0365 changes party direction and F0366 computes movement and calls F0267
- `movement-state-delta-is-source-visible` ok=True refs=DUNGEON.C:1371-1392, MOVESENS.C:316-843: relative movement updates map coordinates; F0267 writes G0306/G0307 on successful party movement/teleporter/pit transitions and can draw intermediate viewport frames while falling
- `viewport-redraw-is-presentable-after-state-delta` ok=True refs=DUNVIEW.C:2962-3000, DUNVIEW.C:8318-8618, DRAWVIEW.C:709-858: viewport draw rebuilds floor/ceiling and wall layers into G0296, then F0097 requests/presents the viewport; repeated identical 48ed frames are capture/route-state failure, not overlay evidence

## Precise next blocker
Capture/replay must prove a source-visible post-command state delta before overlay promotion: after entering gameplay, each movement/turn click needs a capture tied to the exact command completion/redraw boundary with either a new raw hash/region fingerprint or debugger/runtime proof that F0365/F0366 and the F0128/F0097 present path ran for that shot. The current pass487/pass492 evidence reaches gameplay but repeats the 48ed static no-state-delta frame five times and has filename/route-label drift, so those frames remain blocker evidence only.

## Gate
- `python3 tools/verify_pass497_dm1_v1_original_capture_next_blocker.py`

## Non-claims
No original-vs-Firestaff pixel parity, no overlay-reference promotion, and no claim that F0365/F0366 are broken.
