# DM1 all-graphics phase 12 — wall-only gate for wall blits

Date: 2026-04-25 11:42 Europe/Stockholm
Scope: Firestaff V1 / DM1 viewport element-type correctness.

## Change

The new source-bound wall-zone blits now only draw for real wall-like squares:

- `DUNGEON_ELEMENT_WALL`
- `DUNGEON_ELEMENT_FAKEWALL`

They no longer draw for every non-open square. This matters because `m11_viewport_cell_is_open()` also returns false for closed doors, and treating closed doors as stone walls is source-wrong. Door, pit, stair, teleporter, and field rendering must be bound separately to their own `DUNVIEW.C` zones.

Added helper:

- `m11_viewport_cell_is_wall_like(...)`

Updated users:

- `m11_draw_dm1_front_walls(...)`
- `m11_draw_dm1_side_walls(...)`

## Verification

```sh
cmake --build build -j4
./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"
ctest --test-dir build --output-on-failure
```

Results:

- game-view probe: `362/362 invariants passed`
- CTest: `4/4 PASS`

## Next step

Implement element-specific source paths instead of using wall blits as fallback:

1. Door side/front frames and door panels.
2. Pit/stairs/teleporter/field zones.
3. Original `F0120..F0127_DUNGEONVIEW_DrawSquare*` call order and occlusion.
