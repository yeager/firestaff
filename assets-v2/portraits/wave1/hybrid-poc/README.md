# Firestaff V2 hybrid hero portrait PoC — champion r2/c9

## Source

- Source sheet: `reference-artifacts/anchors/0026_champion_portraits.png`
- Selected cell: row `2`, column `9`
- Source crop: `../source/champion_r2_c9_source.png`
- Nearest reference upscale: `../source/champion_r2_c9_nearest64x.png`

## Intent

First bounded proof-of-concept for the V2 "original + upscale hybrid" champion portrait style.

The goal is **not** generic fantasy splash art. The portrait must preserve the original champion's small-pixel identity and remain readable as a compact HUD portrait.

## Generated candidates

- `fs-v2-hybrid-hero-r2c9-poc-a.png`
- `fs-v2-hybrid-hero-r2c9-poc-b.png`
- `fs-v2-hybrid-hero-r2c9-poc-c.png`

Each generated candidate is `1024×1536`.

Upscaled working masters:

- `masters-4k/fs-v2-hybrid-hero-r2c9-poc-a.4k.png`
- `masters-4k/fs-v2-hybrid-hero-r2c9-poc-b.4k.png`
- `masters-4k/fs-v2-hybrid-hero-r2c9-poc-c.4k.png`

These are upscaled from the generated candidates to `2160×3240` as provisional 4K-ish portrait masters. They are **not final art**.

## Quick review

Current best candidate: **A**.

Why:

- strongest compact bust framing
- most readable face/eyes at HUD scale
- best overall hybrid quality

Known risks:

- identity still drifts from the source in skin/hair/clothing interpretation
- needs a stricter source-lock pass before batch production
- next generation should keep the pixel crop visible during prompt construction and explicitly preserve exact palette blocks more aggressively

## Recommendation

Do **one more stricter source-lock generation pass** on candidate A before scaling to all heroes.
