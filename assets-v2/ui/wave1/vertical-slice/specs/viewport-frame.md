# Vertical Slice Production Brief: Viewport Frame

## Asset IDs
- `fs.v2.slice.viewport-frame.base`
- `fs.v2.slice.viewport-frame.inner-mask`

## Purpose
This spec remains as legacy slice context only. The viewport-frame family is currently blocked for trusted DM1-faithful rebuild work until `0000` is semantically re-locked.

## Source anchor
- Current provisional source candidate: `GRAPHICS.DAT` graphic `0000`
- `GRAPHICS_DAT_EXPORT_MAPPING_AUDIT.md` still marks `0000` as mapping-suspicious, so do not treat the current files as authoritative viewport-frame truth

## Deliverables

### `fs.v2.slice.viewport-frame.base`
- **4K master:** 2240×1360 PNG
- **1080p export:** 1120×680 PNG
- Painted frame only
- Keep the viewport opening integration-safe

### `fs.v2.slice.viewport-frame.inner-mask`
- **4K master:** 2240×1360 PNG
- **1080p export:** 1120×680 PNG
- Clean aperture mask aligned to the frame opening
- No painterly noise on the edge line

## Visual direction
- Dark-fantasy UI frame, painterly but controlled
- Material mix can be carved stone, oxidized bronze, aged iron, or lacquered wood accents
- The center opening must feel deeper and more deliberate than the outer border
- Edge treatment should imply craft and age, not random damage

## Hard constraints
- Do not widen or redesign the opening shape away from the DM1 role
- Do not add scene art, lighting rays, characters, or fake screenshot content inside the viewport
- Avoid faux-pixel-upscale texture and avoid glossy modern UI plastic
- Keep ornament concentrated on the outer frame, not crowding the aperture edge

## Layer guidance
- Treat the painted border and the integration mask as separate production outputs
- The mask should support later engine compositing without hand-cleanup

## Artist checklist
- Frame silhouette reads clearly at full size and at 1120×680
- Aperture border remains crisp after downscale
- Material palette matches the action area and status-box family
- Outer corners and bevels feel hand-authored rather than mirrored by default

## Acceptance gate
- [blocked] No new acceptance claim should be made until `0000` is verified against Greatstone/SCK and ReDMCSB
- Existing files may still be used for temporary slice integration only
