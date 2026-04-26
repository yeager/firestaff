# DM1 all-graphics parity — phase 2137–2156: V1 status bar colors

## Scope

Expose and route the source V1 champion status bar colors and blank-bar color used by the party HUD bar graph renderer.

## Source anchors

- ReDMCSB `DATA.C` / `G0046_auc_Graphic562_ChampionColor` defines champion bar colors `{ 7, 11, 8, 14 }`.
- `CHAMDRAW.C:F0287_CHAMPION_DrawBarGraphs` blanks the bar containers with `C12_COLOR_DARKEST_GRAY` before drawing the colored fill.

## Implemented

- Promoted the bar-color table to `M11_GameView_GetV1ChampionBarColor()` for probe-visible source parity.
- Added `M11_GameView_GetV1StatusBarBlankColor()` for the `C12` blank-bar fill.
- Routed V1 status bar rendering through the new helpers.

## Updated invariant

- `INV_GV_300Y`: V1 champion bar colors use source `G0046` order with `C12` blank bars.

## Verification

- `cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2` — OK
- `FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"` — OK, `# summary: 510/510 invariants passed`
- `ctest --test-dir build --output-on-failure` — OK, `100% tests passed, 0 tests failed out of 5`

## Probe excerpt

```text
PASS INV_GV_300Y V1 champion bar colors use source G0046 order with C12 blank bars
```
