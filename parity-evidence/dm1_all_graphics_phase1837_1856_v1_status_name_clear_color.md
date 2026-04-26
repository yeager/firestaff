# DM1 all-graphics parity — phase 1837–1856: V1 status-name clear color

## Scope

Make the champion status-name clear color explicit and probe-visible instead of leaving it as an inline palette constant in the draw path.

## Source anchors

- ReDMCSB `CHAMDRAW.C` / `F0292_CHAMPION_DrawChampionState` clears the status-name zone `C159+n` before drawing the centered champion name into `C163+n`.
- The source clear color is `C01_COLOR_DARK_GRAY`; in the DM PC VGA palette wired into M11 this is palette slot `1` (`gray`).
- Existing text-color parity remains unchanged: leader `C11` yellow, other living champions `C09` gold, dead champions `C13` silver.

## Implemented

- Added `M11_GameView_GetV1StatusNameClearColor()`.
- Routed V1 status-name-zone clearing through the helper.
- Added probe coverage that validates both the helper value and that the rendered name-clear zone contains the source `C01` clear color.

## New invariant

- `INV_GV_15Z`: V1 champion HUD name clear uses source `C01` gray before centered name text.

## Verification

- `cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2`
- `FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"`
- `ctest --test-dir build --output-on-failure`

## Probe excerpt

```text
PASS INV_GV_15Z V1 champion HUD name clear uses source C01 gray before centered name text
```
