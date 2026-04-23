# V2 UI 4K rebuild note — 2026-04-23

## Scope redone

Redone from verified DM1 reference:
- spell area family (`0009`)
- action area family (`0010`)
- status-box family aliases using trusted semantics:
  - `0007` = alive/normal status box
  - `0008` = dead-champion status box
- party HUD cell family:
  - `0033` = normal slot box
  - `0035` = acting-hand slot box used for the bounded active emphasis asset
- party HUD four-slot family rebuilt as a derived grouping strip from the trusted status-box and slot-cell families

## Intentionally blocked / suspicious

Not rebuilt or re-approved as trusted DM1-faithful reference:
- viewport-frame family (`0000`)

Reason:
- `GRAPHICS_DAT_EXPORT_MAPPING_AUDIT.md` still marks `0000` as mapping-suspicious.
- Greatstone/SCK and ReDMCSB do not yet justify calling it a safely locked viewport-frame base.
- Existing first-pass viewport files remain in repo only as provisional legacy slice material.

## Rebuild method

For trusted families, the old ornate V2 shells were superseded with restrained DM-like rebuilds that keep the verified original raster structure at exact preserve-scale enlargement.

Production method used:
- start from verified `extracted-graphics-v1/pgm/graphic_XXXX.pgm`
- preserve original silhouette and internal partitioning exactly at 10x scale
- recolor into a restrained DM-like dark bronze / stone value range
- regenerate 4K PNG masters and exact 50% 1080p exports
- keep SVG masters as thin wrappers around the rebuilt canonical PNGs so the current pipeline still has SVG companions

## Reference source used per family

- spell area: Greatstone/SCK + ReDMCSB-confirmed `0009`
- action area: Greatstone/SCK + ReDMCSB-confirmed `0010`
- status boxes: Greatstone/SCK + ReDMCSB-confirmed `0007`, `0008`
- party HUD cell family: Greatstone/SCK + ReDMCSB-confirmed `0033`, `0035`
- party HUD four-slot family: derived current V2 grouping asset built only from the trusted status-box and slot-cell subset above

## Honest semantic corrections applied

- old `left-frame` / `right-frame` file names were preserved only as legacy pipeline aliases
- trusted meanings are now `alive/normal status box` and `dead-champion status box`
- old generic `party HUD highlight` wording was removed from the `0035`-based asset
- `0034` wounded slot-box semantics remain intentionally unclaimed in the current bounded V2 set

## Verification performed

- checked source family trust against `GRAPHICS_DAT_EXPORT_MAPPING_AUDIT.md`
- regenerated every rebuilt family at expected 4K and 1080p sizes
- verified PNG dimensions match manifest contracts
- visually spot-checked rebuilt outputs against the local DM1 originals to confirm preserved silhouette and calmer DM-like palette/material direction
