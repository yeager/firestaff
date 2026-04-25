# DM1 all-graphics phase 123 — endgame champion names

## Problem

Pass 122 replaced the invented game-won panel with source-backed endgame graphics, but it still did not draw any champion identity text.

ReDMCSB `ENDGAME.C:F0444_STARTEND_Endgame` draws, for each party champion:

```c
AL1410_i_Y = championIndex * 48;
F0021_MAIN_BlitToScreen(championMirror, C412_ZONE_ENDGAME_CHAMPION_MIRROR_0 + championIndex, C10_COLOR_FLESH);
F0021_MAIN_BlitToScreen(championPortrait, C416_ZONE_ENDGAME_CHAMPION_PORTRAIT_0 + championIndex, C01_COLOR_DARK_GRAY);
F0443_STARTEND_EndgamePrintString(87, AL1410_i_Y += 14, C09_COLOR_GOLD, champion->Name);
```

Pass 122 had the mirror zones but not the source-coordinate champion name text.

## Change

When the source endgame path is active, Firestaff now prints each present champion's name at the source coordinate:

```text
x = 87
y = 14 + championIndex * 48
color = C09 gold/orange
```

This is intentionally limited to the name line for now. Title, portraits, and skill-list rendering remain separate work.

## Gate

Added invariant:

- `INV_GV_165E` — V1 endgame prints champion name at source coordinate

```text
PASS INV_GV_165 Endgame victory overlay renders differently from normal
PASS INV_GV_165B V1 endgame overlay keeps invented tick/help text out of default source path
PASS INV_GV_165C V1 endgame uses source C006 The End graphic
PASS INV_GV_165D V1 endgame uses source champion mirror graphic zone
PASS INV_GV_165E V1 endgame prints champion name at source coordinate
# summary: 437/437 invariants passed
ctest --test-dir build --output-on-failure
5/5 PASS
```

## Remaining gaps

Still open for fuller `F0444_STARTEND_Endgame` parity:

- champion portrait blits into C416–C419
- champion title text
- skill-title/level list
- endgame timing/music/restart loop
- original overlay comparison captures
