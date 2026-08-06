# Theron's Quest Track 02 bitmap-route provenance audit

**Date:** 2026-08-06

**Scope:** production startup-media capture for the real Track 02 files under
`/Users/bosse/.firestaff/data/theron/`
**Status:** diagnostic byte evidence; semantic runtime promotion remains open

## Finding

`src/theron/theron_v1_track02.c` samples 4bpp-looking 8×8 tiles from bounded
raw/ISO offsets and assigns them to four route bits: title, stage, Soul Room,
and forcefield. The bytes are real and the hash-recognised media identity is
real, but the route assignment is currently a layout-catalog classification.
The static bank and stage-2 disassembly do not join those offsets to a
screen-owned VDC destination, HuC6260 palette write, or executing post-startup
consumer.

`src/theron/theron_v1_startup_media.c` therefore retains the indexed samples
and atlas as diagnostic receipts. `theron_v1_startup_media_consume_raw_bitmap_route()`
explicitly leaves `palette_binding_verified` and `rgba_output_allowed` clear;
this is the correct boundary for the evidence currently available.

## Evidence boundary

- Real media: `TQUS02.bin` (US) and `TQJP02.bin` (JP), with their authenticated
  Track 02 identities.
- Static consumer fragment: `docs/source-lock/theron-disassembly/theron-us-bank1f-consumer.asm`.
- Stage-2 loader fragment: `docs/source-lock/theron-disassembly/theron-us-stage2-huc6280.asm`.
- Layout-only bitmap evidence: `docs/source-lock/tqr_v1_track02_bank_signal_2026-06-03.md`.
- Graphics-format boundary: `docs/source-lock/tqr_v1_track02_graphics_format_real_media_2026-07-11.md`.
- Palette ownership boundary: `docs/source-lock/tqr_v1_track02_palette_offset_receipt_2026-08-06.md`.

Those sources establish authentic bytes, not semantic screen ownership. In
particular, a complete four-route receipt must not be described as proof of a
decoded title, stage, Soul Room, or forcefield image.

## Required next proof

The production route can be promoted only after one real capture supplies all
of the following for the same media identity: the CD read/LBA and byte span,
the executing HuC6280 consumer PC, the VDC VRAM destination/transfer length,
and the matching HuC6260 palette write. Until then the atlas is retained for
inspection and tests, while color and viewport rendering remain fail-closed.

## Capture-backed screen-space binding

The authenticated US capture now supplies a complete raw VDC VRAM snapshot
(65,536 bytes) and VCE snapshot (1,024 bytes), alongside the expected US
Track 02 MD5 `ceb02343868f80cec899e9b239aff2da` and System Card MD5
`ff1a674273fe3540ccef576376407d1d`. The production viewport binds those
snapshots only when both paths are explicitly supplied. The capture-side
regression observed 154 real tile/palette pairs and presented 9,954
non-zero indexed pixels to M11; the VCE snapshot contributes 512 palette
entries.

This closes the old production no-op for a captured screen-space frame. It
does not identify a dungeon square, object, level, or post-startup consumer:
`theron_vp_render_dungeon()` replays the captured BAT window without assigning
its cells to the world model, and the no-snapshot path remains no-draw. The
consumer/handoff gate therefore stays closed for forcefield-to-dungeon entry.
The operator replay used the instrumented Mednafen build with SDL 2.32.70
through `sdl2-compat` and a dummy video driver; it is authentic emulator
memory state, but not native Quartz/SDL2 capture evidence.
