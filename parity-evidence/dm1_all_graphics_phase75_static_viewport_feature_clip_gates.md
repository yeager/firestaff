# DM1 all-graphics phase 75 — static viewport feature clipping guards

## Problem

Phases 73–74 locked clipping for side creatures, projectiles, and objects. Static viewport features also use source GRAPHICS.DAT/layout-zone blits and can be large or near viewport edges: pits, invisible pits, stairs, teleporter fields, and center creatures. These must remain confined to the DM1 viewport rectangle before further source-zone tuning.

## Change

Added focused clipping invariants comparing each feature frame against the empty-corridor baseline and requiring zero diffs outside the DM1 viewport rectangle `(0,33,224,136)`:

- `INV_GV_38X` — D1C normal pit clips inside viewport
- `INV_GV_38Y` — D1C invisible pit clips inside viewport
- `INV_GV_38Z` — D1C stairs clips inside viewport
- `INV_GV_38AA` — D1C teleporter field clips inside viewport
- `INV_GV_38AB` — D1C center Trolin creature clips inside viewport

## Gate

```text
PASS INV_GV_38X focused viewport: D1C normal pit clips inside the DM1 viewport rectangle
PASS INV_GV_38Y focused viewport: D1C invisible pit clips inside the DM1 viewport rectangle
PASS INV_GV_38Z focused viewport: D1C stairs clips inside the DM1 viewport rectangle
PASS INV_GV_38AA focused viewport: D1C teleporter field clips inside the DM1 viewport rectangle
PASS INV_GV_38AB focused viewport: D1C Trolin creature clips inside the DM1 viewport rectangle
# summary: 410/410 invariants passed
ctest --test-dir build --output-on-failure
5/5 PASS
```
