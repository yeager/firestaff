# Theron's Quest Track 02 -- HuC6260 Palette Word Format

## Scope

This note locks only the decoder for a palette payload whose location has
already been supplied by independent loader/media evidence. It does not locate
a Track 02 palette, assign a palette to any title or dungeon surface, or make a
render-promotion claim.

## Independent Hardware Evidence

The HuC6260 CMOS Video Color Encoder manual describes its color-table RAM as
512 entries of 9 bits and documents the CTW/CTR word fields as:

| Bits | Component |
| --- | --- |
| 0..2 | Blue |
| 3..5 | Red |
| 6..8 | Green |
| 9..15 | Unused/reserved |

The same register mapping is independently transcribed in the TurboGrafx-16
VDC programmer reference:

- https://datacrystal.tcrf.net/wiki/TurboGrafx-16/VDC_Programmers_Reference
- https://www.scribd.com/document/942023441/HuC6260-CMOS-Video-Color-Encoder-Manual

This is hardware evidence, not evidence that any particular 32-byte Track 02
span is a palette payload. The payload reader therefore remains caller-offset
driven and hash-gated.

## Firestaff Boundary

`theron_v1_track02_decode_4bpp_palette()` reads sixteen little-endian words,
rejects bits 9..15, expands each 3-bit component to 8-bit RGB, and returns no
semantic role. `theron_v1_track02_inspect_4bpp_palette_window()` retains the
existing raw/MODE1 provenance and explicitly leaves both
`semantic_binding_verified` and `promotion_allowed` unset.

No Track 02 palette offset, object table, non-startup level record, or runtime
route is established by this note.
