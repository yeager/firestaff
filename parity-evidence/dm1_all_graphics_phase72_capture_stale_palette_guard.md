# DM1 all-graphics phase 72 — stale palette capture guard

## Problem

The deterministic in-game capture pipeline previously produced a false rainbow/static visual regression when a stale binary interpreted packed framebuffer bytes through the old EGA-like path. The smoke test from phase 71 proved files existed, but did not yet prove the viewport was free of the obsolete saturated colours that exposed the stale path.

## Change

Extended `run_firestaff_m11_ingame_capture_smoke.sh` with a viewport colour sanity check over `01_ingame_start_latest.ppm`:

- samples the DM1 viewport rectangle `(x=0..223, y=33..168)`
- rejects captures where stale EGA/CGA-like colours dominate:
  - `(0,0,170)`
  - `(85,255,85)`
  - `(255,85,85)`
  - `(255,255,85)`
  - `(85,85,255)`
- threshold: stale-colour ratio must stay below 1% of sampled viewport pixels

This is intentionally not pixel-perfect art parity. It is a regression guard for the exact stale-capture failure mode that misled visual inspection.

## Gate

```text
./run_firestaff_m11_ingame_capture_smoke.sh
In-game capture smoke PASS: 6 screenshots

./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"
# summary: 400/400 invariants passed

ctest --test-dir build --output-on-failure
5/5 PASS
```
