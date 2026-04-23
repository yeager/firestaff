# V2 Wave 1 Spec: Spell Area

## Scope
- Spell panel shell only
- No rune set, no typography system, no final spell FX

## Canonical sizes
- 4K master: **870×250**
- 1080p derived: **435×125**

## Required layers
- Base panel
- Rune/text bed
- Highlight overlay
- Active spell-state overlay

## Art direction stub
- DM-faithful preserve-scale repaint rooted in verified `0009`
- Must pair visually with the action area without looking identical
- Keep enough calm negative space for rune readability
- Prefer restrained dark bronze / stone values over ornate fantasy polish

## Guardrails
- Preserve DM1 proportions
- Avoid ornamental clutter that collapses at 1080p
- Keep final export compatible with later UI compositing

## Current status
- 2026-04-23 rebuild assets now exist for `base`, `rune-bed`, `highlight-overlay`, and `active-overlay`.
- The rebuild supersedes the earlier ornate shell pass with a verified-geometry preserve-scale repaint.
- The current pass deliberately stops short of full rune glyph production, typography, or final spell FX.
