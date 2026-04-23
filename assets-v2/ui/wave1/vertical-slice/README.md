# Firestaff V2 Wave 1 Vertical Slice

This directory now contains the first **real first-pass V2 asset pack** for the bounded Wave 1 slice.

Included families:
- viewport-frame
- action-area
- spell-area
- status-box-family
- party-hud-cell-family

## What is real in-repo now

Each shipped slice asset exists as:
- editable **4K SVG master**
- rendered **4K PNG master**
- derived **1080p PNG export** from exact 50% downscale

These assets are usable for an early visual integration pass, but they are still **first-pass art**, not final paintovers.

## Current quality bar

What these files are:
- coherent dark-fantasy V2 UI shells
- production-sized assets that respect the approved DM1 geometry role
- practical integration targets for compositing and layout tests

What these files are not:
- final art approval
- portrait, icon, text, or runtime-state system completion
- full Wave 1 coverage beyond this bounded slice

## Still pending outside this pass

Not produced here:
- portrait payloads
- stat text systems
- action glyph/icon sets
- broader state matrices beyond the included overlays

## Spell-area note for this pass

The spell-area expansion uses the canonical family directory at `assets-v2/ui/wave1/spell-area/` and is now referenced by the bounded vertical-slice manifest and engine path.

Integrated in this pass:
- spell-area base shell
- spell-area rune-bed layer
- spell-area highlight overlay
- spell-area active overlay

## Rules
- 4K masters are canonical.
- 1080p exports are exact 50% derivatives.
- English only for tracked repo content.
