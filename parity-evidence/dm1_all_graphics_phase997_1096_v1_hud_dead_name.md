# DM1 all-graphics parity — phase 997–1096: V1 HUD dead champion name parity

## Scope

Continue compact champion HUD parity by correcting dead champion status-box text: source prints the champion name in the same compact name zone, not an invented `DEAD` label.

## Source anchors

- `firestaff_pc34_core_amalgam.c:11483-11486` (`F0292_CHAMPION_DrawState`): dead champions blit `C008_GRAPHIC_STATUS_BOX_DEAD_CHAMPION`, then call `F0650_PrintCenteredTextToScreenZone(P0615_ui_ChampionIndex + C163_ZONE_FIRST_CHAMPION_NAME, C13_COLOR_LIGHTEST_GRAY, C01_COLOR_DARK_GRAY, Champion.Name)`.
- `zones_h_reconstruction.json` / GRAPHICS.DAT entry 696: `C163..C166` are the compact champion-name text zones inside the 67×29 status boxes.
- `dm7z-extract/Toolchains/Common/Source/DEFS.H:3787-3794`: `C159..C166` status-name zone constants.

## Implemented

- `M11_GameView_GetV1StatusNameColor(...)` now models both source branches:
  - dead champion: `C13` lightest gray / silver,
  - living leader: `C11` yellow,
  - other living champion: `C09` gold.
- V1 dead champion boxes now use the shared compact centered name rendering path after graphic 8 is blitted.
- The invented red `DEAD` text is no longer drawn in V1 mode; it remains only in the non-V1/V2 fallback path.

## New invariant

- `INV_GV_15E5`: dead champion compact HUD name color resolves to C13 and renders silver pixels inside the source name zone.

## Verification

- `cmake --build build --target firestaff_m11_game_view_probe -j2`
- `ctest --test-dir build --output-on-failure`
- `./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"`
- Probe result: `470/470 invariants passed`

## Probe excerpt

```text
PASS INV_GV_15E2 V1 champion HUD name clear zones match layout-696 C159..C162 geometry
PASS INV_GV_15E3 V1 champion HUD name colors follow F0292 leader yellow / non-leader gold
PASS INV_GV_15E4 V1 champion HUD renders source-colored names inside the compact status name zones
PASS INV_GV_15E5 V1 dead champion HUD prints source centered name in C13 lightest gray
PASS INV_GV_15F V1 champion HUD ready/action hands use normal slot-box graphic when idle
# summary: 470/470 invariants passed
```
