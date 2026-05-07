# Pass277 — DM1 V2 real-state draw-list and pixel comparator scaffold

## Scope

This pass advances the pass274 blocker without claiming final pixel parity.

It adds a source-locked sparse fixture for the canonical DM1 PC34 entry state:

- `DUNGEON.DAT` SHA256: `d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85`
- Header offset 8 decodes to map 0, x=1, y=3, direction=2 (`0x0861`)
- The known front wall square is map 0, x=1, y=4 (`D1C` from the entry pose), sourced from the existing pass173/pass162 front-wall sensor audits

## Landed implementation

- `dm1_v2_vp_relative_coords()` mirrors ReDMCSB `DUNGEON.C:35-44` and `DUNGEON.C:1371-1391` relative movement math.
- `dm1_v2_vp_dm1_pc34_entry_state_fixture()` exposes the named fixture `dm1_pc34_entry_portrait_wall`.
- `dm1_v2_vp_build_composition_from_fixture()` now feeds `mapX/mapY/direction` through relative coordinates into the existing D0-D3 draw-list emitter.
- `dm1_v2_vp_compare_viewport_region()` is a framebuffer region comparator scaffold that reports compared pixels, mismatch count, and first mismatch coordinate.

## Proof added

`test_viewport_real_state_fixture_draw_list` proves the named entry fixture maps input `(1,3,2)` to a D1C wall draw command in source order:

1. floor/ceiling base
2. D1C wall, order 9

`test_viewport_region_comparator_scaffold` proves the region comparator detects exact match and first mismatch coordinates on synthetic buffers.

## Still missing

- Full `DUNGEON.DAT` square decoder; current fixture is intentionally sparse.
- Matched original/ReDMCSB and Firestaff viewport pixel captures for the same state.
- Real pixel parity claim. This pass only lands the plumbing and gate evidence.
