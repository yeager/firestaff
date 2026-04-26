# DM1/V1 visual parity — top HUD champion icons

## Scope

Restore the visible top-right V1 champion-position icon cells in the in-game HUD.

## Source anchors

- ReDMCSB `CHAMDRAW.C` `F0288` clears `C113_ZONE_CHAMPION_ICON_TOP_LEFT` through `C116_ZONE_CHAMPION_ICON_BOTTOM_LEFT` before drawing each icon.
- The source fills a temporary `19x14` champion-icon bitmap with `G0046_auc_Graphic562_ChampionColor[championIndex]`, overlays `C028_GRAPHIC_CHAMPION_ICONS`, and blits it into the clipped layout-696 icon zones.
- `M026_CHAMPION_ICON_INDEX(Direction, PartyDirection)` selects the C028 strip cell by rotating the champion direction relative to the party direction.
- ReDMCSB `G2362_auc_PaletteChanges_Invisibility` remaps invisible-party champion icon colours with bytes `{0,1,2,0,4,0,6,7,8,9,0,11,12,13,14,15}`.
- `GRAPHICS.DAT` metadata confirms graphic 28 is `76x14`, i.e. four `19x14` cells.

## Implemented

- Added source-index helper for C028 champion icon strip selection.
- Draws V1 champion icons in `C113..C116` after the status boxes, with source champion colours and black empty slots.
- Applies the source invisibility palette remap when the party invisibility event counter is active.
- Updated the top-chrome diagnostic invariant so it excludes both source status boxes and source champion icon zones.

## Verification added

- `INV_GV_15AF`: source-index selection follows the M026 direction-relative calculation.
- `INV_GV_15AG`: recruited champion icon cells render in source colours and absent icon slots remain black.
- `INV_GV_300AR`: invisibility palette remap bytes match `G2362_auc_PaletteChanges_Invisibility`.

## Local verification

```text
./run_firestaff_m11_game_view_probe.sh
# summary: 575/575 invariants passed
```
