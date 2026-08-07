# Firestaff V2 Wave 1 item source inventory

Source audit anchors:
- ReDMCSB `DUNVIEW.C` `G0209_as_Graphic558_ObjectAspects` and `G2030_auc_ObjectScales` define object aspect/scaling data.
- ReDMCSB `DUNVIEW.C` `G0219` coordinate sets define per-cell floor object placement.
- ReDMCSB `DUNVIEW.C` `F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF` lines 4820-5193 gate visible floor/alcove objects, pile shifts, grabbable zones, and projectile-as-object fallback.

Verified binding:
- `C201_ICON_ACTION_ICON_EMPTY_HAND` is PC34 `GRAPHICS.DAT` record 48,
  icon 201 at `(144, 0, 16×16)`. The local archive gives the canonical
  `GRAPHICS.DAT` SHA-256
  `2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e`.

Contract:
- No generated or hand-authored V2 item pixels are admitted.
- A generic floor item has no single source bitmap: F0115 resolves every live
  Thing through `G0209` and `G0219`. Until that live source route is exposed,
  V2 returns no floor-item binding and V1 remains the pixel owner.
