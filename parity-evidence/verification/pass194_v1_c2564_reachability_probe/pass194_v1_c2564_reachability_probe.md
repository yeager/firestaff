# Pass 194 — V1 C2564 reachability probe

## Source citations audited first
- ReDMCSB DUNVIEW.C:371-374 defines G2028 normal-object rows and G2029 alcove slots; G2028 never maps any view square to row 16.
- ReDMCSB DEFS.H:2600-2614 defines the media-720 view-square indices including D3C=11, D3L=12, D3R=13, D4C=16, D4L=17, D4R=18.
- ReDMCSB DUNVIEW.C:4922-4927 gates object drawing on thing type, L2476_i_ >= 0, matching thing cell, and depth/cell visibility.
- ReDMCSB DUNVIEW.C:5071-5078 selects C2548 + coordinateSet*7 + G2029[viewSquare] for alcove objects, otherwise C2500 + G2028[viewSquare]*4 + viewCell for normal floor/open-cell objects, then applies object pile shifts.
- ReDMCSB DUNVIEW.C:8468-8477 still calls F0115 for D4L/D4R/D4C, but G2028 entries for view-square indices 16-18 are -1, so those calls cannot emit normal C2500 object zones.

## Finding

- Target: `ingame_move_forward`, `C2564`, anchor `[181, 62]`, window `[177, 58, 185, 66]`.
- Normal/open-cell floor-item path reachable? `False`.
- Alcove-object path reachable? `True`; hits: `[{'zone': 2564, 'coordinate_set': 2, 'alcove_slot': 2, 'view_square_index': 13}]`.
- Classification: `C2564` is part of the `C2548` alcove-object subfamily, not the normal `C2500 + G2028*4 + viewCell` open-cell/floor-item path.

## Pixel window

- Changed pixels: `81/81`; mean luma current/original/abs: `0.0` / `163.765` / `163.765`.
- Current dark pixels (luma <= 8): `81/81`; original bright pixels (luma >= 160): `38/81`.
- Max delta: `{'x': 177, 'y': 59, 'current_rgb': [0, 0, 0], 'original_rgb': [255, 255, 255], 'luma_abs': 255}`.

## Interpretation

Do not spend the next lane on normal floor-item sprite placement for this target. The evidence points to wall/open-cell/base-geometry or alcove-adjacent rendering around the right side of the viewport; no low-risk code fix was made.

JSON: `parity-evidence/verification/pass194_v1_c2564_reachability_probe/pass194_v1_c2564_reachability_probe.json`
