# DM1 all-graphics phase 74 — viewport contents clipping guards

## Problem

Phase 73 locked C3200 side-creature clipping. The same leak risk exists for the other source-bound dynamic viewport contents: projectiles and floor objects. Those sprites are drawn from source GRAPHICS.DAT assets and increasingly from source zone tables, so the important invariant is that focused sprite changes stay confined to the DM1 viewport rectangle and never dirty the UI/HUD area.

## Change

Reused the `probe_count_diffs_outside_rect(...)` helper from phase 73 and added focused clipping invariants for dynamic contents:

- `INV_GV_38T` — D1C fireball projectile clips inside `(0,33,224,136)`
- `INV_GV_38U` — D1C lightning projectile clips inside `(0,33,224,136)`
- `INV_GV_38V` — D1C dagger object clips inside `(0,33,224,136)`
- `INV_GV_38W` — D1C multi-object pile clips inside `(0,33,224,136)`

These compare each focused frame against the empty-corridor baseline and require zero diffs outside the DM1 viewport rectangle.

## Gate

```text
PASS INV_GV_38T focused viewport: D1C fireball projectile clips inside the DM1 viewport rectangle
PASS INV_GV_38U focused viewport: D1C lightning projectile clips inside the DM1 viewport rectangle
PASS INV_GV_38V focused viewport: D1C dagger object clips inside the DM1 viewport rectangle
PASS INV_GV_38W focused viewport: D1C multi-object pile clips inside the DM1 viewport rectangle
# summary: 405/405 invariants passed
ctest --test-dir build --output-on-failure
5/5 PASS
```
