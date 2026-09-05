# Pass 581 — DM1 V1 D3/D2 wall-ornament source order

## Result

The admitted PC 3.4 F0107 wall-ornament projections are owned by the live
F0128 per-square callback. `m11_draw_viewport` contains no direct call to the
wall-ornament rasterizer and therefore cannot replay those pixels.

## Source order

ReDMCSB `DUNVIEW.C` establishes:

- F0116/F0117: D3L and D3R wall, side projection, front projection, optional
  front-alcove F0115 handoff;
- F0119/F0120: the equivalent D2L and D2R order;
- F0118/F0121: one centre-front F0107 call;
- F0122/F0123: one D1 side-facing call whose return is ignored;
- F0676/F0677: one D3L2/D3R2 side-facing call whose return is ignored;
- F0128: complete D3 routes before D2 and D1 routes.

The scheduler now emits two F0107 steps only for normal D3/D2 side squares
and one for outer, centre, and D1 squares. The callback maps each admitted
occurrence to one G0205 projection; outer MEDIA720 projections remain
fail-closed because the checked-in ReDMCSB table exposes 13 rows while its
active view constants describe 15 positions.

## Verification

- `pass581_dm1_v1_d3_d2_wall_ornament_order_source_lock`
- `dm1_v1_f0128_per_square_scheduler_pc34_compat`
- `m11_dm1_hoc_orientation_runtime_pc34`

The real-media test reads the original PC 3.4 ZIP in memory and reported ten
wall-material and ten F0107 callback steps in its HoC route. No game-data
member was extracted or replaced.
