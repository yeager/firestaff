# DM1 V1 D3C F0111 Door-Front + F0107 Wall-Ornament Pair

Contract-only source-lock for the centered D3C door-front route. It does
not claim real game-data pixels.

ReDMCSB anchors:

- `DUNVIEW.C F0118:6721-6747`: D3C door-front composition.
- `DUNVIEW.C F0111:4218-4337`: native door bitmap copy, ornament pass,
  and final `F0791` transparent blit.
- `DUNVIEW.C F0107:3502-3938`: wall-ornament palette and alcove return,
  kept out of the D3C door-front frame.
- `DUNVIEW.C F0104:3113-3156` and `F0105:3185-3247`: native and flipped
  C10 blit helpers used for the D3C door-frame halves.
- `DUNVIEW.C F0115:4547-4581`: door pass cell ordering.
- `DUNVIEW.C F0128:8478-8508,8534-8542`: D3C dispatch position.
- `DEFS.H:2088,2668-2677,2698-2702,4044-4046,4239-4254`: C10,
  cell orders, D3 wall views, wall zones, and the door-zone family.

The PC34 ReDMCSB source names centered D3C door zone as
`M625_ZONE_DOOR_D3C=3730`; `M628_ZONE_DOOR_D2C` and
`M629_ZONE_DOOR_D2R` are asserted as disjoint neighbors in the same family.
