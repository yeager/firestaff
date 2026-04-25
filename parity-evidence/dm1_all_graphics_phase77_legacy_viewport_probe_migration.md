# DM1 all-graphics phase 77 — migrate legacy viewport probe checks to DM1 rect

## Problem

After phase 76 introduced explicit `PROBE_DM1_VIEWPORT_*` constants, several older visual sanity gates still sampled the historical prototype viewport rectangle `(12,24,196,118)`. Those gates predate the source-bound DM1 viewport move and could pass while ignoring pixels in the real DM1 viewport margins.

## Change

Migrated the remaining viewport-content sanity checks to use the source DM1 viewport constants `(0,33,224,136)`:

- `INV_GV_10` synthetic feature cues inside viewport
- `INV_GV_12` viewport/minimap coexistence
- `INV_GV_12B` item/effect cues inside viewport
- `INV_GV_16` layered face bands/edges
- `INV_GV_17` threat reticle + inspect readout
- `INV_GV_92` asset-backed viewport palette diversity

The old `PROBE_VIEWPORT_*` constants remain defined only for legacy/debug geometry reference, but there are no remaining probe uses of them.

## Gate

```text
PASS INV_GV_10 synthetic feature cells add door, stair, and occupancy cues inside the viewport
PASS INV_GV_12 viewport slice and minimap inset coexist in the same frame
PASS INV_GV_12B viewport item and effect cues appear when real thing chains include loot and projectiles
PASS INV_GV_16 viewport framing uses layered face bands and bright dungeon edges
PASS INV_GV_17 front-cell focus adds a threat-colored viewport reticle plus contextual inspect readout
PASS INV_GV_92 asset-backed viewport uses at least 6 distinct palette colors
# summary: 410/410 invariants passed
ctest --test-dir build --output-on-failure
5/5 PASS
```
