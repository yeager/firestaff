# pass787plus CSB V1 D1L/D1R F0108 Floor/Ceiling/Ornament Source Lock

Contract-only source-lock gate for the CSB V1 D1L/D1R F0108 floor+ceiling+ornament path. It does not load game data and does not claim original-DOS pixel parity.

## Anchors

- ReDMCSB `DUNVIEW.C F0108:3940-4011`: floor-ornament dispatch, `MASK0x8000` footprint recursion, `C10_COLOR_FLESH` transparent blit, and `C1500_ZONE_FLOOR_ORNAMENT + CoordinateSet * 11 + ViewFloor` zone math.
- ReDMCSB `DUNVIEW.C F0124:7873-7957`: D1C center path kept as an explicit exclusion so this lane remains D1L/D1R, not the integrated D1C gate.
- ReDMCSB `DUNVIEW.C F0107:3502-3938`: wall-ornament path excluded from this F0108 floor lane.
- ReDMCSB `DUNVIEW.C F0115:4547-4581, 4923, 5180-5188, 5211-5214, 5668-5671`: thing-pass row guards and object/creature/projectile/explosion ordering after floor+ceiling and wall update.
- ReDMCSB `DUNVIEW.C F0127/F0128:8318-8486, 8536-8541`: compose/dispatch path with D1L/D1R side pair before D1C, and D0L/D0R after D1C.
- ReDMCSB `DUNGEON.C F0163:1769-1838`, `F0164:1840-1905`, `F0172:2466-2523`: thing-list mutation keepout and square-aspect source.
- ReDMCSB `DEFS.H:2088, 2596-2611, 2662, 2668-2677, 4045-4046, 4139-4153, 4223`: C10 transparency, D1 view-square constants, cell-order constants, wall/stair keepouts, and floor-ornament zone base.
- CSB-lineage `Viewport.cpp:1192-1209, 1865-1879, 1903-1915, 1930-1944, 6507-6548, 6924-6927`: near-side open row, door-facing side/front contrast, masked decoration merge, and CustomBackgrounds ordering.

## Contract

- Pins the D1L then D1R side pair, not D0L2/D0R2, D1L2/D1R2, or D1C.
- Uses the 320x200 screen and 224x136 viewport contract through the same C10 transparent-blit and mask-order model as the existing CSB gates.
- Requires F0108 floor+ceiling before the F0115 thing pass and keeps F0107 wall ornament plus F0111 door-front paths disjoint.
- Rejects mutation attempts against the F0163/F0164 cell list route.
- Keeps CustomBackgrounds masks after floor+ceiling and before the C10 thing-pass row guard.
- Maintains at least three palette keepouts, at least three CustomBackgrounds mask checks, at least five mutation/route rejections, and a deterministic model hash.

## Verification

Local verification for this branch:

```text
cmake --build build --target test_csb_v1_viewport_d1l_d1r_f0108_floor_ceiling_ornament_pc34_compat --parallel
[100%] Built target test_csb_v1_viewport_d1l_d1r_f0108_floor_ceiling_ornament_pc34_compat

ctest -R pass_csb_v1_viewport_d1l_d1r_f0108_floor_ceiling_ornament -V
PASS test_csb_v1_viewport_d1l_d1r_f0108_floor_ceiling_ornament_pc34_compat assertions=600 failures=0 d1l_floor=5 d1r_floor=5 ceiling_copies=10 custom_bg_masks=10 thing_passes=10 palette_keepouts=10 mutation_rejections=6 hash=0x2e322a54

SDL_VIDEODRIVER=dummy ./build/firestaff_m11_phase_a_probe
# summary: 23/23 invariants passed

git diff --check HEAD
# clean
```

Note: the first fresh build of broad shared targets surfaced existing warnings in unrelated DM1/Nexus/DM1V2/Theron/M11 files. The incremental lane target rebuilt cleanly after this gate's source changes.
