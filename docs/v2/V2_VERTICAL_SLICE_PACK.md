# Firestaff V2 Vertical Slice Pack

## Purpose

This pack narrows Wave 1 to the smallest useful V2 UI production slice that can drive real art creation and later render integration.

It covers exactly four core UI families, plus one bounded follow-up expansion:
1. viewport frame
2. action area
3. one status box family
4. one party HUD cell family
5. one shared four-slot party HUD strip expansion layered on top of that cell family

It does **not** claim full Wave 1 completion.
It does **not** cover portraits or a full text/stat typography system in this slice.

## Slice definition

The chosen vertical slice is the minimum reusable gameplay shell needed to stage one believable V2 combat/exploration screen:
- the framed world viewport
- the right-column action container
- a matched left/right status-box pair as one family
- one reusable party HUD slot cell family

This gives production enough surface area to validate:
- V2 material language
- small-frame readability after downscale
- layer separation for UI states
- size contracts for later engine-side composition
- whether the party row can read as one bounded four-slot HUD without committing to the full final HUD system

## Canonical layout contract

The DM1 gameplay field remains the base logic field:
- original logic: **320×200**
- 4K gameplay field: **3200×2000**
- 1080p gameplay field: **1600×1000**

Centered presentation envelopes:

| Target | Gameplay field | Screen | Margins |
|---|---:|---:|---|
| 1080p | 1600×1000 | 1920×1080 | 320 left/right, 40 top/bottom |
| 4K | 3200×2000 | 3840×2160 | 320 left/right, 80 top/bottom |

## Asset set

| Asset ID | Family | Role | 4K master | 1080p derived |
|---|---|---|---:|---:|
| `fs.v2.slice.viewport-frame.base` | `viewport_frame` | Main world viewport frame | 2240×1360 | 1120×680 |
| `fs.v2.slice.viewport-frame.inner-mask` | `viewport_frame` | Clean viewport opening / alpha mask | 2240×1360 | 1120×680 |
| `fs.v2.slice.action-area.base` | `action_area` | Painted action panel shell | 870×450 | 435×225 |
| `fs.v2.slice.action-area.recess-bed` | `action_area` | Interior content bed | 870×450 | 435×225 |
| `fs.v2.slice.action-area.highlight-overlay` | `action_area` | Hover / affordance highlight | 870×450 | 435×225 |
| `fs.v2.slice.action-area.active-overlay` | `action_area` | Armed / selected state overlay | 870×450 | 435×225 |
| `fs.v2.slice.status-box.left-frame` | `status_box_family` | Left status shell | 670×290 | 335×145 |
| `fs.v2.slice.status-box.right-frame` | `status_box_family` | Right status shell | 670×290 | 335×145 |
| `fs.v2.slice.party-hud-cell.standard-base` | `party_hud_cell_family` | Reusable HUD cell shell | 216×216 | 108×108 |
| `fs.v2.slice.party-hud-cell.highlight-overlay` | `party_hud_cell_family` | Hover / active highlight pass | 216×216 | 108×108 |
| `fs.v2.slice.party-hud-four-slot.base` | `party_hud_four_slot_family` | Shared four-slot party HUD strip | 3020×280 | 1510×140 |
| `fs.v2.slice.party-hud-four-slot.active-slot-overlay` | `party_hud_four_slot_family` | Active-slot focus overlay | 710×280 | 355×140 |

## Layering model

### 1. Viewport frame

Required deliverables:
- `base`: painted outer frame with transparent-safe viewport opening treatment
- `inner-mask`: clean opening mask aligned to the exact viewport aperture

Layer intent:
- outer frame paint remains separate from dynamic world rendering
- inner mask gives render integration a clean crop target

### 2. Action area

Required deliverables:
- `base`
- `recess-bed`
- `highlight-overlay`
- `active-overlay`

Layer intent:
- `base` carries structure, material, bevels, and silhouette
- `recess-bed` isolates icon/content placement zone
- `highlight-overlay` adds non-destructive feedback states
- `active-overlay` handles selected/armed emphasis without repainting the base

### 3. Status box family

Required deliverables:
- `left-frame`
- `right-frame`

Layer intent:
- this slice only requires painted shells
- portrait fill, text, damage flash, shield state, and stat overlays remain future work

### 4. Party HUD cell family

Required deliverables:
- `standard-base`
- `highlight-overlay`

Layer intent:
- `standard-base` supports repeated reuse across hand/inventory slots
- `highlight-overlay` enables selection/focus without duplicating base paint

### 5. Party HUD four-slot expansion

Required deliverables:
- `base`
- `active-slot-overlay`

Layer intent:
- `base` groups the four champion slots into one readable strip while preserving the DM1 bottom-row footprint
- `active-slot-overlay` adds a bounded warm focus state for the selected champion without committing to a full future HUD-state matrix

## Acceptance criteria

### Global
- Every 1080p asset is derived by exact **50%** downscale from its approved 4K master.
- DM1 proportions stay intact; no widening, squashing, or faux-widescreen redraw.
- Assets read as hand-authored V2 UI art, not nearest-neighbor enlargements.
- Materials and edge treatment feel consistent across all four families.
- Alpha edges are clean enough for direct engine compositing.

### Viewport frame
- The silhouette clearly matches the DM1 viewport frame role.
- The inner opening is clean, centered, and safe for dynamic scene rendering.
- The 1080p downscale preserves edge readability and does not muddy the aperture border.

### Action area
- The panel remains legible at 435×225.
- Overlays read clearly without crushing the base values.
- Interior zones remain open enough for later icon/text integration.

### Status box family
- Left and right frames feel intentionally paired.
- Portrait and stat payload regions remain visually calm and readable.
- Downscaled frame edges remain crisp at 335×145.

### Party HUD cell family
- The border depth still reads at 108×108.
- The interior remains uncluttered enough for later item/icon placement.
- The highlight overlay improves state clarity without obscuring contents.

### Party HUD four-slot expansion
- The four champion row reads as one bounded module instead of four loose repeats.
- The shared strip still leaves room for the existing champion icon/name/bar payloads.
- The active-slot emphasis is clearer without introducing a full typography redesign.

## Production scaffolding

```text
assets-v2/ui/wave1/vertical-slice/
  prompts/
  specs/
  viewport-frame/
    masters/4k/
    exports/1080p/
  action-area/
    masters/4k/
    exports/1080p/
  status-box-family/
    masters/4k/
    exports/1080p/
  party-hud-cell-family/
    masters/4k/
    exports/1080p/
  party-hud-four-slot/
    masters/4k/
    exports/1080p/
```

## What this pack enables now

This slice is enough to:
- brief an artist or image-generation workflow against exact production targets
- test a shared V2 material/style language on both large and tiny UI assets
- stage a first real composite using one viewport, one action area, one status-box family, one HUD-cell family, and a shared four-slot party strip
- validate whether the 4K-to-1080p downscale policy holds up on practical UI assets

## Still missing before art generation and render integration

This pack still needs, in later passes:
- final source reference captures or extraction sheets beside each brief if the art team wants them locally
- additional authored masters and exports beyond the current bounded Wave 1 set
- portraits, icons, text, and runtime state overlays beyond the layers defined here
- broader engine-side composition wiring beyond the current opt-in vertical-slice path

## Assumptions

- Existing Wave 1 dimensions and DM1 source indices are already the approved baseline.
- One status-box family means shipping the left/right pair together, because they only make sense as a designed set.
- One party HUD cell family means a single standard framed cell plus highlight treatment, not a full inventory-state matrix.
