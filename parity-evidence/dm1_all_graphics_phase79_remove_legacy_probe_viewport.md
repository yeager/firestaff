# DM1 all-graphics phase 79 — remove legacy prototype viewport probe constants

## Problem

After phases 76–77 migrated visual/clipping gates to the real DM1 viewport rectangle `(0,33,224,136)`, the old prototype probe constants remained defined:

```c
PROBE_VIEWPORT_X = 12
PROBE_VIEWPORT_Y = 24
PROBE_VIEWPORT_W = 196
PROBE_VIEWPORT_H = 118
```

They were no longer used, but keeping them around made future probe work vulnerable to accidentally reintroducing the pre-source-bound viewport geometry.

## Change

Removed the unused `PROBE_VIEWPORT_*` constants from `firestaff_m11_game_view_probe.c`.

Verified there are no remaining `PROBE_VIEWPORT_` references.

## Gate

```text
! grep -n "PROBE_VIEWPORT_" probes/m11/firestaff_m11_game_view_probe.c
./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"
# summary: 410/410 invariants passed
ctest --test-dir build --output-on-failure
5/5 PASS
```
