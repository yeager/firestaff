# Vertical Slice Production Brief: Party HUD Cell Family

## Asset IDs
- `fs.v2.slice.party-hud-cell.standard-base`
- `fs.v2.slice.party-hud-cell.highlight-overlay`

## Purpose
Create one reusable V2 HUD cell family for hand/inventory-style slot usage, with enough detail headroom to survive downscale and repeated on-screen use.

## Source anchor
- DM1 slot-cell family from `GRAPHICS.DAT` graphics `0033`, `0034`, and `0035`
- Preserve the role and compact framed-cell logic, not literal low-res pixel texture

## Size contract
- **4K master:** 216×216
- **1080p export:** 108×108

## Deliverables

### `fs.v2.slice.party-hud-cell.standard-base`
- Reusable base framed cell
- Clean enough for repeated placement in a party HUD strip

### `fs.v2.slice.party-hud-cell.highlight-overlay`
- Selection / hover emphasis for the same cell
- Must not obscure later icon content

## Visual direction
- Compact framed cell with visible border depth
- Slight wear is welcome, but it must not become visual dirt
- Interior should stay open and neutral so later icons remain legible

## Hard constraints
- No item art, hands, glyphs, counters, or text in this pass
- Do not make the border so ornate that repeated use becomes noisy
- Keep the interior flatter than the outer ring for content readability

## Artist checklist
- Border depth still reads at 108×108
- Interior remains uncluttered for future icon placement
- Highlight overlay improves state clarity without covering contents
- Family palette matches the larger UI surfaces

## Acceptance gate
- One base cell and one highlight overlay are enough to test repeated HUD placement immediately
