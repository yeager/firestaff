# DM1 all-graphics phase 76 — probe gates use the real DM1 viewport rectangle

## Problem

Some newer clipping gates used the correct DM1 viewport rectangle `(0,33,224,136)` inline, while older probe constants still carried the pre-source-bound prototype viewport `(12,24,196,118)`. One parity-flip sample (`INV_GV_351`) was still sampling rows using the old prototype origin, so it was not explicitly checking the same rectangle the runtime now draws.

## Change

Added named probe constants for the source DM1 viewport:

```c
PROBE_DM1_VIEWPORT_X = 0
PROBE_DM1_VIEWPORT_Y = 33
PROBE_DM1_VIEWPORT_W = 224
PROBE_DM1_VIEWPORT_H = 136
```

Kept the old `PROBE_VIEWPORT_*` constants as legacy/debug-probe geometry.

Updated:

- all recent `probe_count_diffs_outside_rect(...)` clipping gates to use the named DM1 constants
- `INV_GV_351` floor/ceiling parity-flip sampling to check rows inside the real DM1 viewport rectangle instead of the old prototype origin

## Gate

```text
PASS INV_GV_351 normal V1 viewport floor/ceiling obeys DM1 parity flip
# summary: 410/410 invariants passed
ctest --test-dir build --output-on-failure
5/5 PASS
```
