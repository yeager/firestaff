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

## Raw Bitmap Consumer Boundary

`theron_v1_startup_media_consume_raw_bitmap_route()` consumes one of the
already hash-recognised raw 4 bpp startup-atlas routes only after the complete
Track 02 receipt validates its variant, route mask, dimensions, nonzero index
count, checksum, and MODE1 raw/user-data coordinates. It copies the observed
palette indices unchanged and records their source coordinates.

The resulting receipt has `palette_binding_verified=0` and
`rgba_output_allowed=0`. This is intentional: the authenticated runtime trace
currently proves the Track 02 controller/CD transaction, not a HuC6260 palette
write or a palette-to-menu binding. The consumer therefore cannot publish RGB
pixels and must not select a default palette. The corpus probe uses only an
operator-provided, hash-recognised Track 02 BIN plus System Card; it verifies
the raw-byte receipt and expects the palette/RGB gate to remain blocked.

## Live Trace Result

The locally available US E98A trace is provenance-marked but does not contain
the dynamic CD-read or controller-state receipts required by the Track 02
runtime gate, nor an observed write to the VCE palette-data ports. It is
therefore insufficient to authenticate a HuC6260 palette. A future capture
must contain the authenticated Track 02 transaction and record the original
hardware writes, including the selected colour-table indices and both bytes of
each 9-bit entry. Until that capture exists, no code may infer a palette window
from matching bytes in the disc image or open RGB output for a raw bitmap route.

The Mednafen instrumentation records only raw, pre-execution `STA abs` receipts
to `$0402..$0405` after the controller receipt: PC, physical PC, opcode,
address, and accumulator. These rows preserve the observed control/data store
sequence without claiming a completed write, selected palette, entry count, or
bitmap binding. Indirect and block-transfer instructions remain unrecognised;
their presence keeps the RGB gate closed until a complete, authenticated receipt
format is implemented.
