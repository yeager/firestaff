# DM1 all-graphics phase 127 — endgame champion portrait blits

## Problem

The V1 source-backed endgame path drew the source champion mirror graphic and source name/skill text, but it still did not blit each champion portrait into the portrait sub-zone.

Source anchor in `ENDGAME.C:F0444_STARTEND_Endgame`:

```c
F0021_MAIN_BlitToScreen(L1418_puc_Bitmap_ChampionMirror,
                        C412_ZONE_ENDGAME_CHAMPION_MIRROR_0 + championIndex,
                        C10_COLOR_FLESH);
F0021_MAIN_BlitToScreen(champion->Portrait,
                        C416_ZONE_ENDGAME_CHAMPION_PORTRAIT_0 + championIndex,
                        C01_COLOR_DARK_GRAY);
```

Source layout anchor from `zones_h_reconstruction.json`:

- `C412`: `(19, 7)`
- `C416`: parent `C412`, offset `(8, 6)` → absolute `(27, 13)`
- next champions repeat every 48 px vertically via `C413–C415` / `C417–C419`

## Change

The V1 source endgame path now blits champion portraits from `C026_GRAPHIC_CHAMPION_PORTRAITS` into the source portrait zones:

- champion 0: `(27, 13)`
- champion 1: `(27, 61)`
- champion 2: `(27, 109)`
- champion 3: `(27, 157)`

The blit uses `M11_COLOR_DARK_GRAY` as transparent color, matching the source `C01_COLOR_DARK_GRAY` intent through the current V1 palette mapping.

## Gate

Added invariant:

- `INV_GV_165H` — V1 endgame blits source champion portrait into C416

Updated mirror invariant `INV_GV_165D` to skip the portrait overlay sub-zone, because source draws the portrait over the mirror.

```text
PASS INV_GV_165D V1 endgame uses source champion mirror graphic zone
PASS INV_GV_165H V1 endgame blits source champion portrait into C416
PASS INV_GV_165E V1 endgame prints champion name at source coordinate
PASS INV_GV_165F V1 endgame prints source fighter skill-title line
PASS INV_GV_165G V1 endgame prints source ninja/priest/wizard skill-title lines
# summary: 440/440 invariants passed
ctest --test-dir build --output-on-failure
5/5 PASS
```

## Remaining gaps

Still open for fuller `F0444_STARTEND_Endgame` parity:

- champion title text after name
- source-derived hidden/lifecycle skill-level computation rather than only stored base skill levels
- endgame timing/music/restart loop
- original overlay comparison captures
