# DM1 all-graphics phase 83 — viewport rect API gate

## Problem

After the viewport migration, probes and docs agreed on the source DM1 viewport `(0,33,224,136)`, but there was no small public helper that directly exposed the runtime viewport rectangle. Future tests or tools could rederive or hardcode the rectangle and drift again.

## Change

Added:

```c
int M11_GameView_GetViewportRect(int* outX, int* outY, int* outW, int* outH);
```

The helper returns the actual runtime `M11_VIEWPORT_*` constants.

Added invariant:

- `INV_GV_12C` — runtime viewport rect API returns source DM1 viewport geometry `(0,33,224,136)`.

## Gate

```text
PASS INV_GV_12C runtime viewport rect API returns source DM1 viewport geometry
# summary: 411/411 invariants passed
ctest --test-dir build --output-on-failure
5/5 PASS
```
