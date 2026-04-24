# Pass 45 — GRAPHICS.DAT font-bank wiring (V1)

Date: 2026-04-24
Scope: `V1_BLOCKERS.md` §9 only

## Goal

Retire the remaining V1 typography blocker after pass 44 by proving that
Firestaff's generic `m11_draw_text(...)` path now defaults to the
original DM1 font atlas from `GRAPHICS.DAT` whenever the original asset
set is present.

This pass is intentionally narrow:

- no palette work (still pass 46)
- no viewport / side-panel relocation
- no M10 semantic changes
- no save-format changes

## Landed gain

The repo already had the core runtime pieces in place:

- `font_m11.[ch]` resolves the DM1 interface-font entry from
  `GRAPHICS.DAT` into `M11_FontState` via source-backed candidate slots
  instead of assuming one hard-coded media index
- `M11_GameView_StartGame` populates `state->originalFont` and marks
  `originalFontAvailable`
- `M11_GameView_Draw` activates `g_activeOriginalFont` for the render
  pass
- `m11_draw_text(...)` delegates to `m11_draw_text_original(...)`
  whenever that font is active and loaded

Pass 45 makes this blocker honest and probe-backed by adding a bounded
verification step dedicated to the font-bank path.  The probe deliberately
exercises font loading/rendering directly instead of invoking the whole game
view with a synthetic or uninitialized world; full view stability remains
covered by the normal M11 probes.

## Probe

`./run_firestaff_m11_pass45_font_bank_probe.sh`

The probe verifies 8/8 invariants:

1. the resolved interface-font entry loads and `originalFontAvailable`
   becomes true
2. DM1 font width is still 6 pixels per character (`"FONT" == 24 px`)
3. the title-strip glyphs match a direct render from the loaded font
4. disabling `originalFontAvailable` changes the title-strip output
5. selected-rune abbreviations render non-empty glyph pixels from the DM1 font atlas at the pass-44 placement
6. available-rune abbreviations render non-empty glyph pixels from the DM1 font atlas at the pass-44 placement
7. pass-44 spell-label placements stay intact (`83/43`, `67/74`)
8. a reproducible evidence screenshot is emitted

Output artifacts:

- `verification-m11/pass45-font-bank/pass45_font_bank_probe.log`
- `verification-m11/pass45-font-bank/pass45_font_bank_view.pgm`

## What this pass does not claim

- It does **not** claim full `TEXT.C` / runtime-string parity.
  Firestaff still has separate text-content questions tracked elsewhere
  (for example, over-labeling / invented strings).
- It does **not** change palette behavior.  The next remaining medium
  V1 blocker after this pass is the VGA palette swap in pass 46.
- It does **not** touch M10 semantics.
