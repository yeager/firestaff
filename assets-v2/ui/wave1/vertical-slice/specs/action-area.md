# Vertical Slice Production Brief: Action Area

## Asset IDs
- `fs.v2.slice.action-area.base`
- `fs.v2.slice.action-area.recess-bed`
- `fs.v2.slice.action-area.highlight-overlay`
- `fs.v2.slice.action-area.active-overlay`

## Purpose
Create the right-column action panel shell as a layered V2 UI asset that can later receive icons, labels, and action-state logic without repainting the whole panel.

## Source anchor
- DM1 action area role from `GRAPHICS.DAT` graphic `0010`
- Preserve the original silhouette and internal hierarchy

## Size contract
- **4K master:** 870×450
- **1080p export:** 435×225
- PNG with alpha where needed for overlays

## Deliverables

### `fs.v2.slice.action-area.base`
- Structural shell, bevels, frame, and main material treatment

### `fs.v2.slice.action-area.recess-bed`
- Clean interior bed for later action content placement
- Keep readable separation from the outer shell

### `fs.v2.slice.action-area.highlight-overlay`
- Hover / affordance emphasis
- Must remain readable when composited over the base, not replace it

### `fs.v2.slice.action-area.active-overlay`
- Armed / selected emphasis
- Stronger than highlight, but still non-destructive

## Visual direction
- DM-faithful preserve-scale repaint rooted in verified `0010`
- Strongly readable shape language, not fussy decoration
- Wear should stay restrained and subordinate to the original partitioning
- Interior subdivisions should stay calm enough for future glyphs and labels

## Hard constraints
- No final action icons, hands, text, or gameplay labels in this pass
- Do not over-detail tiny sub-panels so hard that 1080p becomes noisy
- Do not flatten state overlays into the base render
- Do not introduce a new layout that breaks DM1 placement logic

## Layer guidance
- `base` carries structure
- `recess-bed` isolates usable interior surfaces
- `highlight-overlay` handles soft focus/hover treatment
- `active-overlay` handles committed state emphasis

## Artist checklist
- Panel still reads cleanly at 435×225
- Overlays remain useful when toggled on and off
- Interior spaces can accept future icon passes without repainting the shell
- Materials stay consistent with the viewport frame and status-box family

## Acceptance gate
- A UI mock can place placeholder icons over the recess bed immediately
- Highlight and active overlays remain visually distinct after downscale
- The rebuilt panel still reads as the original DM1 action-area structure before it reads as new ornament
