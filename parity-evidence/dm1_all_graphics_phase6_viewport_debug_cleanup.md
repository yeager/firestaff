# DM1 all-graphics phase 6 — viewport debug-overlay cleanup

Date: 2026-04-25 10:41 Europe/Stockholm
Scope: Firestaff V1 / DM1 PC 3.4 normal viewport presentation.

## Change

Normal V1 viewport no longer draws Firestaff diagnostic overlays that made the dungeon view read as a debug/prototype screen.

Moved behind `state->showDebugHUD`:

- yellow/cyan viewport debug frame boxes
- lane scanner chips (`L`, `F`, `R`)
- depth chips
- focus brackets / reticle
- viewport feedback frame
- horizontal guide line across the viewport

The underlying source-bound/fixture rendering path remains active; this phase only removes invented diagnostic decoration from normal V1.

## Artifacts

Generated normal V1 screenshot set:

- `verification-m11/dm1-all-graphics/phase6-viewport-debug-clean-20260425-1041/normal/*.pgm`
- `verification-m11/dm1-all-graphics/phase6-viewport-debug-clean-20260425-1041/normal/*.png`

Quick viewport/top crop:

- `verification-m11/dm1-all-graphics/phase6-viewport-debug-clean-20260425-1041/normal/party_hud_viewport_top_crop.png`

Visual inspection of the top/viewport crop confirmed that the previous obvious debug overlays are gone: no yellow/cyan viewport frame, no L/F/R chips, no depth chips, no reticle/crosshair, no guide line.

## Verification

```sh
cmake --build build -j4
./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"
ctest --test-dir build --output-on-failure
```

Results:

- game-view probe: `362/362 invariants passed`
- CTest: `4/4 PASS`

## Remaining work

This is not viewport 1:1 parity yet. Obvious remaining work:

- replace/split the remaining procedural dungeon wall/floor/ceiling drawing with source-bound DM1 draw order
- resolve `0078`/`0079` floor/ceiling mapping with evidence before changing semantics
- build controlled original-vs-Firestaff viewport crop comparisons for fixed map/x/y/facing/light states
- remove or source-bind any remaining non-original effects (e.g. attack cue / decorative guide-like strips) during the sprite/effect pass
