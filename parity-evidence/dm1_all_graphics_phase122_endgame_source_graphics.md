# DM1 all-graphics phase 122 — source-backed endgame graphics

## Problem

Dialog `F0427_DIALOG_Draw` is now source-backed, but the game-won/endgame overlay still used an invented Firestaff victory panel in default V1.

ReDMCSB `ENDGAME.C:F0444_STARTEND_Endgame` uses source endgame graphics and zones:

- loads `C006_GRAPHIC_THE_END`
- when `G0302_B_GameWon` is true, loads `C346_GRAPHIC_WALL_ORNAMENT_43_CHAMPION_MIRROR`
- blits champion mirrors into `C412_ZONE_ENDGAME_CHAMPION_MIRROR_0 + championIndex`
- blits champion portraits into `C416_ZONE_ENDGAME_CHAMPION_PORTRAIT_0 + championIndex`
- later draws restart/quit boxes using `G2019/G2020/G2021/G2022` from `DATA.C`

Local GRAPHICS.DAT mapping:

```text
C006_GRAPHIC_THE_END = graphic 6, 80×14
C346_GRAPHIC_WALL_ORNAMENT_43_CHAMPION_MIRROR = graphic 346, 48×43
C412..C415 champion mirror zones = x 19, y 7/55/103/151
restart outer/inner = {103,140,115,15} / {105,142,111,11}
quit outer/inner    = {127,165,67,15} / {129,167,63,11}
```

## Change

Default V1 game-won rendering now uses a source-backed endgame path when assets are available:

- fills the screen with source endgame dark-gray base
- blits champion mirror graphic 346 in the four C412..C415 zone positions
- blits `THE END` graphic 6 centered near the source endgame area
- draws source-coordinate restart/quit boxes and labels
- keeps the old invented panel as fallback for missing assets / non-V1/debug configurations

## Gates

Added invariants:

- `INV_GV_165C` — V1 endgame uses source C006 The End graphic
- `INV_GV_165D` — V1 endgame uses source champion mirror graphic zone

Updated the older debug-label gate:

- `INV_GV_165B` now asserts the default source path keeps invented tick/help text out of default V1

```text
PASS INV_GV_165 Endgame victory overlay renders differently from normal
PASS INV_GV_165B V1 endgame overlay keeps invented tick/help text out of default source path
PASS INV_GV_165C V1 endgame uses source C006 The End graphic
PASS INV_GV_165D V1 endgame uses source champion mirror graphic zone
# summary: 436/436 invariants passed
ctest --test-dir build --output-on-failure
5/5 PASS
```

## Remaining gaps

This narrows endgame substantially but is not full `F0444` parity yet:

- champion portrait/name/title/skill list draw remains incomplete
- endgame timing/music/restart loop is not source-exact
- original overlay comparison captures still needed
