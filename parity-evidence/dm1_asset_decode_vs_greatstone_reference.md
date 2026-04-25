# DM1 asset decode vs Greatstone/SCK-style extracted reference

Date: 2026-04-25 10:58 Europe/Stockholm
Scope: Firestaff M11 `GRAPHICS.DAT` asset loader output for key DM1/V1 graphics.

## Question

Daniel asked whether Firestaff is extracting the right assets from `GRAPHICS.DAT` and specifically asked to compare with Greatstone.

## Reference

Local reference set:

- `extracted-graphics-v1/pgm/graphic_NNNN.pgm`
- source SHA-256: `2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e`

This reference set is the local Greatstone/SCK-style extracted `GRAPHICS.DAT` output used throughout the V1 parity work.

## Method

A small one-off loader dump compared M11 `M11_AssetLoader_Load()` output against `extracted-graphics-v1/pgm` for selected critical assets. Comparison was byte/pixel exact on indexed PGM data.

Artifacts:

- `verification-m11/dm1-all-graphics/greatstone-compare-20260425-1058/current/*.pgm`
- `verification-m11/dm1-all-graphics/greatstone-compare-20260425-1058/compare_report.md`
- `verification-m11/dm1-all-graphics/greatstone-compare-20260425-1058/asset_compare_sheet.png`

## Result

| index | role | result |
|---:|---|---|
| 0000 | viewport background | exact match |
| 0007 | status box | exact match |
| 0008 | dead status box | exact match |
| 0009 | spell area background | exact match |
| 0010 | action/PASS area | exact match |
| 0020 | panel/inventory background | exact match |
| 0033 | slot box normal | exact match |
| 0034 | slot box wounded | exact match |
| 0035 | slot box acting hand | exact match |
| 0042 | wall set | exact match |
| 0076 | floor tile | exact match |
| 0078 | viewport ceiling/floor strip candidate | exact match |
| 0079 | viewport ceiling/floor strip candidate | exact match |
| 0246 | creature sprite | exact match |
| 0248 | creature sprite | exact match |
| 0303 | wall ornament | exact match |
| 0304 | wall ornament | exact match |
| 0344 | item/potion sprite | exact match |
| 0420 | projectile/effect sprite | exact match |
| 0437 | projectile/effect sprite | exact match |

All compared assets: `max delta = 0`, `differing pixels = 0`.

## Conclusion

The current problem is not asset extraction/decode for these key graphics. `M11_AssetLoader_Load()` produces pixel-exact output vs the local Greatstone/SCK-style extracted reference for the tested set, including the previously distorted odd-width `0010` PASS/action area.

The remaining visible ugliness is runtime use/placement/draw order:

- viewport is not yet source-faithful `DRAWVIEW` output
- right action/spell panel zones still need exact source-zone placement/use
- bottom champion HUD/inventory panel still needs source-zone rebuild
- full-screen comparison against original runtime capture remains required before any 1:1 claim
