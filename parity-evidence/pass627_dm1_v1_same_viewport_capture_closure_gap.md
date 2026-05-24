# Pass627 - DM1 V1 same-viewport capture closure — gap classification

Status: CLOSED_PASS627_DM1_V1_SAME_VIEWPORT_CAPTURE_CLOSURE_GAP_CLASSIFIED

## Classification

The remaining "wall/viewport parity gaps still need promotable original/Firestaff capture-backed closure" (TODO.md line 16) is classified as:

**Case (c) — Missing comparator evidence (original DOS runtime transcript).**

The Firestaff side of the same-viewport capture contract is fully implemented and source-contract backed. No Firestaff renderer change is required to close this gap.

## Firestaff-side closure (complete)

The following are all PASS and CTest-locked:

- **s_same_viewport_capture_contract** (dm1_v1_viewport_3d_pc34_compat.c:209-219):
  - `requires_original_command_transcript = true`
  - `requires_same_firestaff_view_tuple = true`
  - `duplicate_original_viewport_hashes_block_promotion = true`
  - Source lines: COMMAND.C:106-114 (mouse zones), COMMAND.C:2045-2156 (F0380 queue pop), CLIKMENU.C:142-174 (F0365 turn), CLIKMENU.C:180-347 (F0366 move), DUNVIEW.C:8318-8611 (F0128 tuple), DRAWVIEW.C:709-858 (F0097 present boundary)
  - CTest: test_same_viewport_capture_contract() — all 10 checks PASS

- **pass609 source audit** (59fffa57): All 5 source audit items PASS (game loop waits, F0380 dequeue, F0365/F0366 mutation, F0128 tuple, F0097 present boundary)

- **pass623 Firestaff input bridge**: 4 canonical rows with paired Firestaff viewport crops:
  - `01_start_south_1_3`: sha 66be5867..., crop 01_start_south_1_3_viewport_224x136.ppm
  - `02_turn_right_west_1_3`: sha 1e71ed87..., crop 02_turn_right_west_1_3_viewport_224x136.ppm (canonical promotable row per pass625/626)
  - `03_blocked_west_wall_1_3`: sha 1e71ed87... (intentionally same as 02 — proven same state per manifest "sameViewportHashAs")
  - `04_forward_south_1_4`: sha 5e9fb1e3..., crop 04_forward_south_1_4_viewport_224x136.ppm

- **pass625 row preflight** (a7de3704): Target row `02_turn_right_west_1_3` source-locked as C002 turn-right, dir 2→3, map 0 x=1 y=3

- **pass626 turn-redraw route** (446c7d07): All 6 route audit items PASS:
  - F0380 dequeues C002 and dispatches F0365
  - F0365 sets wait stop and new direction 3
  - F0284 commits party direction before redraw
  - Main loop dispatches then next iteration redraws
  - F0128 consumes direction=3 x=1 y=3 and calls present
  - Crop boundary is PC/I34E VIDRV_09_BlitViewPort

## Gap: original DOS runtime transcript

What is still missing is the **original DOSBox capture** with runtime transcript proving the same-viewport redraw route for the `02_turn_right_west_1_3` target row. Specifically:

Required for promotable original:
1. DOSBox runtime log showing F0380 dequeue of C002 (command id 2) from queue slot
2. F0365/F0366 state mutation proof (direction committed to 3 at map 0 x=1 y=3)
3. F0128 direction/mapX/mapY tuple consumption proof (direction=3 x=1 y=3)
4. F0097/VIDRV present boundary evidence for the captured viewport frame
5. One original viewport crop `.ppm` with same hash as Firestaff `1e71ed87...` (224x136, PC/I34E)
6. No duplicate viewport crop hash across different semantic labels

pass622 already identified this gap and correctly classified it as the missing original transcript.

## Non-claims

- No Firestaff renderer behavior change
- No original DOSBox capture was run
- No pixel parity is promoted
- No DANNESBURK use
- No V2/CSB/DM2 scope
- pass609 source audit remains valid; the gap is evidence-only, not logic-only

## Conclusion

The "remaining wall/viewport parity gaps" referenced in TODO.md line 16 are now fully classified: the Firestaff same-viewport capture combine/dedupe logic is source-contract complete and CTest-locked. The gap is exclusively on the original DOS capture side — a runtime transcript proving the command→mutation→redraw→present route for the `02_turn_right_west_1_3` labeled frame. No Firestaff code change can promote this; it requires a paired original DOS capture session.

This is the correct and only remaining gap. No further Firestaff pass is needed until an original DOS capture is available.
