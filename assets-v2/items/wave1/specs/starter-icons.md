# Firestaff V2 Wave 1 Starter Item Icons

Source audit anchors:
- ReDMCSB `DUNVIEW.C` `G0209_as_Graphic558_ObjectAspects` and `G2030_auc_ObjectScales` define object aspect/scaling data.
- ReDMCSB `DUNVIEW.C` `G0219` coordinate sets define per-cell floor object placement.
- ReDMCSB `DUNVIEW.C` `F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF` lines 4820-5193 gate visible floor/alcove objects, pile shifts, grabbable zones, and projectile-as-object fallback.

Contract:
- Master PNGs are authored at exactly 1024x1024, with exact 512x512 derived exports.
- Silhouette must remain readable at HUD scale; no glossy or tiny-detail-first rendering.
- This pack is manifest-only until approved art exists; it must not alter V1 object behavior.
