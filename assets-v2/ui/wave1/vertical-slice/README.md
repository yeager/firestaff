# Firestaff V2 Wave 1 Vertical Slice

This directory now contains the current bounded Wave 1 slice asset pack, including the 2026-04-23 trusted-family rebuild.

Included families:
- viewport-frame (still provisional / blocked for trusted DM1-faithful use)
- action-area
- spell-area
- status-box-family
- party-hud-cell-family
- party-hud-four-slot

## What is real in-repo now

Each rebuilt shipped slice asset exists as:
- companion **4K SVG master**
- rendered **4K PNG master**
- derived **1080p PNG export** from exact 50% downscale

The 2026-04-23 rebuild intentionally replaced the earlier ornate first-pass shells for the trusted families with more DM-faithful preserve-scale repaint assets.

## Current quality bar

What these files are:
- restrained DM-like V2 UI rebuilds for the trusted audited families
- production-sized assets that respect verified DM1 geometry roles where mapping is locked
- practical integration targets for compositing and layout tests

What these files are not:
- final art approval
- proof that every existing semantic mapping is settled
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
- Treat `viewport-frame` as blocked/provisional until `0000` is re-locked against Greatstone/SCK and ReDMCSB.
