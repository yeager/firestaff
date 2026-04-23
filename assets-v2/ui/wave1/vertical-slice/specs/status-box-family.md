# Vertical Slice Production Brief: Status Box Family

## Asset IDs
- `fs.v2.slice.status-box.left-frame`
- `fs.v2.slice.status-box.right-frame`

## Purpose
Create one usable status-box family for the slice: a designed left/right pair that can later receive portraits, names, bars, and status overlays.

## Source anchor
- DM1 status box shells from verified `GRAPHICS.DAT` graphics `0007` and `0008`
- Trusted semantics are `0007` alive/normal status box and `0008` dead-champion status box
- Legacy left/right filenames may remain only as pipeline aliases

## Size contract
- **4K master:** 670×290 each
- **1080p export:** 335×145 each

## Deliverables

### `fs.v2.slice.status-box.left-frame`
- Legacy filename preserved for the `0007` alive/normal shell alias
- Framed portrait/stat container with calm payload areas

### `fs.v2.slice.status-box.right-frame`
- Legacy filename preserved for the `0008` dead-champion shell alias
- Must feel paired with the left version without pretending the original semantics were left/right

## Visual direction
- Painterly dark-fantasy shell with controlled detail
- Enough bevel and material depth to feel premium at 4K
- Enough restraint to remain readable when portrait and stat systems are added later
- The pair can be asymmetrical, but should clearly belong together

## Hard constraints
- No portraits, labels, numbers, bars, or damage-state effects in this pass
- Do not crowd the payload zones with ornate trim
- Do not rely on a trivial mirror if asymmetry is needed to feel correct

## Payload-safe areas
- Reserve a calm portrait zone
- Reserve a calm text/stat zone
- Keep these zones visually flatter than the outer frame edges

## Artist checklist
- Left/right frames feel like a coherent pair
- Portrait and stat areas remain easy to read against later content
- Downscaled edges stay crisp at 335×145
- Surface language matches the viewport frame and action area

## Acceptance gate
- A mock portrait and mock text block can be composited into both variants without additional cleanup
- The rebuilt pair is documented with the correct alive/dead semantics rather than the earlier incorrect left/right mapping claim
