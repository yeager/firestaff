# DM1 all-graphics phase 64 — probe warning cleanup

## Change

Removed dead local variables from the M11 game-view probe:

- unused `fb2`
- unused `multiPixels`
- unused `startMap`

This keeps the probe build clean after the C2500/C2900/C3200 passes and prevents warning noise from hiding real regressions.

## Verification

```text
cmake --build build -j4
# no compiler warnings emitted in the captured build log

# summary: 399/399 invariants passed
ctest: 4/4 PASS
```
