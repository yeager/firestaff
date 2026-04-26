# DM1 all-graphics phase 3037-3056 — V1 damage indicator source zone ids

## Scope

Bind the V1 champion damage indicator helper to the ReDMCSB/layout source zone ids instead of leaving the centered C015 placement as anonymous status-box math.

## Source anchors

- ReDMCSB `CHAMPION.C F0291` selects `C167_ZONE_FIRST_DAMAGE_TO_CHAMPION_SMALL + championIndex` when drawing damage on champion status boxes.
- GRAPHICS.DAT `C015_GRAPHIC_DAMAGE_TO_CHAMPION_SMALL` is the 45x7 small damage banner.
- The V1 status-box footprint remains the source `C151..C154` / C007 geometry: 67x29 at 69 px stride.

## Implemented

- Added `M11_GameView_GetV1DamageIndicatorZoneId(slot)` returning source zones `C167..C170`.
- Routed `M11_GameView_GetV1DamageIndicatorZone(...)` through that zone-id mapping before resolving the existing centered C015 rectangle.
- Tightened the game-view probe so invalid slots return no zone and slots 0/3 verify `C167`/`C170` plus the same source-backed pixel rectangles.

## Verification

-  verifies /, invalid slot rejection, and the centered  C015 rectangles.
-  phase A, game-view, and launcher smoke passed on the worker before landing.
