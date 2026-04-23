# Vertical Slice Production Brief: Viewport Frame

## Asset IDs
- `fs.v2.slice.viewport-frame.base`
- `fs.v2.slice.viewport-frame.inner-mask`

## Purpose
Create the V2 world viewport frame that anchors the whole screen composition while leaving a clean opening for dynamic world rendering.

## Source anchor
- DM1 viewport frame role from `GRAPHICS.DAT` graphic `0000`
- Preserve original frame proportions and aperture hierarchy

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
- Viewport opening is clean enough to drop a render plate behind it immediately
- No edge chatter, alpha fringing, or muddy inner lip at 1080p
