# Pass 126 — V1 viewport/world source-zone gates in CTest

Date: 2026-04-29
Lane: DM1 V1 viewport/world visuals — walls/items/ornaments/creatures/draw-order/evidence

## Scope

Promoted the existing source-shape viewport/world evidence gates from ad-hoc scripts into CTest so normal local/CI test runs now catch regressions in both:

- viewport draw order: wall/door ornaments before open-cell contents; floor ornaments → floor items → creatures → projectiles/effects inside open cells;
- source-zone anchor tables: C2500 object/item, C2900 projectile, and C3200 creature placement points against reconstructed layout-696 `GRAPHICS.DAT` records.

This is intentionally not a pixel-perfect original-runtime claim. It hardens the source/data seam while the semantically matched original DM1 route remains unresolved.

## Change

`CMakeLists.txt` now discovers a Python interpreter and, when available, registers two tests:

- `v1_viewport_draw_order_gate` → `tools/verify_v1_viewport_draw_order_gate.py`
- `v1_viewport_source_zone_tables` → `tools/verify_v1_viewport_source_zone_tables.py`

The gates are optional only with respect to Python discovery; when Python is present they run under `ctest` alongside the existing M11 probes.

## Verification

```sh
cmake -S . -B build
cmake --build build --target firestaff_m11_game_view_probe -- -j4
ctest --test-dir build -R "v1_viewport_(draw_order_gate|source_zone_tables)|m11_game_view" --output-on-failure
```

Result:

```text
100% tests passed, 0 tests failed out of 3
```

Direct gate output was also captured in:

- `parity-evidence/runs/pass126_viewport_zone_ctest_20260429T0650Z/output.log`

Key source anchors observed:

```text
V1 viewport draw-order source-shape verification passed
- wall/door ornaments before open-cell contents
- open-cell layer order: floor ornaments -> floor items -> creatures -> projectiles/effects

V1 viewport content zone table verification passed
- GRAPHICS.DAT sha256=2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e
- C2500 object/item anchors: 68 records match layout-696 C2500..C2567
- C2900 projectile anchors: 48 records match layout-696 C2900..C2947
- C3200 center/side creature anchors are present in the source C3200-family records
```

## Remaining blocker

The exact F0115 `viewSquareTo`/side/deep row mapping for the non-rendered C2500/C2900 rows is still a source-mapping task. This pass makes regressions visible in routine test runs but does not yet wire those additional rows into the normal renderer.
