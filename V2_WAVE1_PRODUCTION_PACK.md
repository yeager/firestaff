# Firestaff V2 Wave 1 Production Pack

## Purpose

This document starts the real Firestaff V2 asset-production track for the first high-value UI families.

It assumes the following product decisions are already locked:
- V2 only; no V1 parity edits in this pack
- DM1 geometry and proportions remain the layout skeleton
- 4K masters are canonical
- 1080p assets are derived from the 4K masters at exactly 50% scale

## Wave 1 families

1. Core gameplay frame / viewport base
2. Action area
3. Spell area
4. Status boxes
5. Slot / party HUD cells

## Canonical layout rule

The DM1 logical gameplay field is preserved at:
- **320×200** original logic
- **3200×2000** canonical 4K layout field
- **1600×1000** derived 1080p layout field

The gameplay field stays centered inside 16:9 presentation space.

### Centering envelopes

| Target | Gameplay field | Screen | Remaining margin |
|---|---:|---:|---:|
| 1080p | 1600×1000 | 1920×1080 | 320 px left + right, 40 px top + bottom |
| 4K | 3200×2000 | 3840×2160 | 320 px left + right, 80 px top + bottom |

This keeps the DM screen composition intact without stretching the playfield to full 16:9.

## Wave 1 master sizes

All 1080p deliverables are exact half-scale derivatives of the 4K masters.

| Family | Stable family key | DM1 source ref | Original size | 4K master | 1080p derived | Production class |
|---|---|---|---:|---:|---:|---|
| Core gameplay frame | `core_gameplay_frame` | whole DM screen | 320×200 | 3200×2000 | 1600×1000 | preserve-scale-repaint |
| Viewport base | `viewport_base` | graphic 0000 | 224×136 | 2240×1360 | 1120×680 | preserve-scale-repaint |
| Action area | `action_area` | graphic 0010 | 87×45 | 870×450 | 435×225 | preserve-scale-repaint + layered overlays |
| Spell area | `spell_area` | graphic 0009 | 87×25 | 870×250 | 435×125 | preserve-scale-repaint + layered overlays |
| Status box frame | `status_box_frame` | graphics 0007, 0008 | 67×29 | 670×290 | 335×145 | preserve-scale-repaint |
| Party HUD slot cell | `party_hud_slot_cell` | graphics 0033, 0034, 0035 | 18×18 | 216×216 | 108×108 | preserve-scale-repaint |

## Wave 1 production notes by family

### 1. Core gameplay frame / viewport base
- The 4K gameplay frame is the canonical composition field for V2 gameplay scenes.
- The viewport base remains the main recognizability anchor.
- Treat the surrounding frame as modular painted UI, not a screenshot plate.
- The viewport base should be produced as a reusable framed surface with a transparent-safe integration plan for dynamic world rendering.

### 2. Action area
- Build as a layered UI family, not as a single flattened bitmap.
- Minimum layers:
  - base panel
  - recess/content bed
  - interaction highlight layer
  - selected/armed state layer
- Action icon art is out of scope for this pack; only the container family is defined here.

### 3. Spell area
- Match DM1 proportions exactly.
- Build with the same layering logic as the action area.
- Reserve clean interior space for runes, text, and active spell-state overlays.

### 4. Status boxes
- Treat left/right status frame variants as separate assets under one family.
- Keep the original silhouette and read order.
- Portrait and text systems are not part of this Wave 1 pack, but the frame must leave for them.

### 5. Slot / party HUD cells
- Use **216×216** as the working 4K master, not 180×180.
- Reason: small framed cells need extra detail headroom for bevels, wear, highlight edges, and clean downsampling.
- The 1080p shipping derivative is **108×108**.

## Production directory contract

```text
assets-v2/
  manifests/
    firestaff-v2-asset.schema.json
    firestaff-v2-wave1-ui.manifest.json
  ui/
    wave1/
      viewport-frame/
        masters/4k/
        exports/1080p/
      action-area/
        masters/4k/
        exports/1080p/
      spell-area/
        masters/4k/
        exports/1080p/
      status-boxes/
        masters/4k/
        exports/1080p/
      party-hud-cells/
        masters/4k/
        exports/1080p/
      specs/
```

## Production status for this pack

This pack creates:
- the exact Wave 1 size contract
- stable asset IDs
- a manifest and schema for integration work
- directory scaffolding for master and derived outputs
- family prompt/spec stubs for art production

This pack does **not** claim finished art.

## Export rules

- Paint and review from the 4K master first.
- Derive 1080p by exact 50% downscale from approved 4K masters.
- Do not independently repaint 1080p unless a specific readability issue is found after downsampling.
- Keep alpha-bearing UI assets in PNG.
- Keep source prompts, paintover notes, and provenance beside each family spec or in later source metadata.

## Explicit out-of-scope items

This production pack does not yet include:
- portraits
- icons/action glyphs
- text/font system
- creature art
- environment modules
- animation timing sheets
- engine integration code

Those broader classes are now tracked separately by:
- `V2_ALL_ASSETS_STYLE_GUIDE.md`
- `assets-v2/ALL_ASSETS_V2_COVERAGE.md`
- `assets-v2/manifests/firestaff-v2-all-assets-foundation.manifest.json`

## Assumptions used here

- DM1 graphic references from the extracted V1 manifest are correct.
- Wave 1 should optimize for production readiness, not art completion.
- Small HUD cell families benefit from a slightly richer master than strict 10× parity, so **216×216** is the canonical exception in this pack.
