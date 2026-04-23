# Vertical Slice Production Brief: Party HUD Four-Slot Family

## Asset IDs
- `fs.v2.slice.party-hud-four-slot.base`
- `fs.v2.slice.party-hud-four-slot.active-slot-overlay`

## Purpose
Add a bounded next-pass expansion on top of the existing party HUD cell family so the champion row reads as a clearer four-slot party HUD instead of four loose repeated fragments.

## Source anchor
- DM1 four-champion party row layout
- Existing Firestaff V2 status-box family and party HUD cell family spacing
- Preserve the compact DM1 bottom-row footprint while improving shared structure

## Size contract
- `fs.v2.slice.party-hud-four-slot.base`
  - **4K master:** 3020×280
  - **1080p export:** 1510×140
- `fs.v2.slice.party-hud-four-slot.active-slot-overlay`
  - **4K master:** 710×280
  - **1080p export:** 355×140

## Deliverables

### `fs.v2.slice.party-hud-four-slot.base`
- Shared four-slot strip behind the party row
- Must group all four champion slots into one readable module
- Must leave existing name/bar/icon payload zones usable

### `fs.v2.slice.party-hud-four-slot.active-slot-overlay`
- Warm active-slot emphasis layer sized for one champion slot
- Must strengthen focus without forcing a typography or portrait redesign

## Visual direction
- Same dark bronze / muted gold language as the viewport, status boxes, and party HUD cell family
- Shared strip should feel structural, not ornamental clutter
- Repeated slot pods should be clear at gameplay size
- Focus treatment should stay warm and restrained

## Hard constraints
- No portraits in this pass
- No full stat typography redesign in this pass
- No attempt to solve every future HUD state
- Keep the result compatible with the existing bounded V2 vertical-slice integration

## Acceptance gate
- Four champion slots read as one coherent party HUD component
- Existing per-champion payloads still fit without a new layout system
- Active champion is easier to pick out at a glance
- The added strip still reads cleanly after the exact 50% downscale
