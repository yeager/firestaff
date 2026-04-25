# DM1 all-graphics phase 73 — side creature clipping guard

## Problem

Phase 62 bound side-cell creatures to layout-696 `C3200_ZONE_` points. Some source points are intentionally outside the viewport, e.g. D1 left can anchor at negative X (`-21`). That is source-correct, but it makes clipping behavior important: an extreme side creature must not bleed into UI or framebuffer regions outside the DM1 viewport rectangle.

## Change

Added probe helper:

```c
probe_count_diffs_outside_rect(...)
```

Added invariant:

- `INV_GV_38S` — focused D1L side-cell Trolin changes the viewport, but has zero pixel diffs outside the DM1 viewport rectangle `(0,33,224,136)` compared with the empty-corridor baseline.

This is not a new rendering feature; it locks down the clipping expectation for the already source-bound C3200 side placement.

## Gate

```text
PASS INV_GV_38S focused viewport: extreme C3200 side creature clips inside the DM1 viewport rectangle
# summary: 401/401 invariants passed
ctest --test-dir build --output-on-failure
5/5 PASS
```
