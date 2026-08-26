# Firestaff TODO — Nexus

Reviewed 2026-08-25. Only open work is listed here.

- Bind authentic Saturn CD reads to SH-2 consumers, VDP destinations and
  decoded source assets before admitting title/menu rendering. The full title
  `TITLE.CG` SH-2→VDP2 copy plan is now captured at PC `0x06041fa0`; a
  source-LBA-filtered receipt now reconstructs the complete real `TITLE.CG`
  buffer at `0x0025daf0`. The terminal 32-byte CDB-register path remains
  unbound. The title VDP1 chain now byte-binds all 177 texture spans to
  `LEV00.DGN`; 173 also match their real Structure2 image and CLUT pair. The
  remaining four draws (`0x00e60`, `0x00780`, `0x00880`, `0x01380`) share the
  directly traced `LEV00.DGN` CLUT at word `0xca00`, though it has no exact
  Structure2 palette-record identity. The 175 distorted-sprite vertices and
  their two local-coordinate transforms are captured; raster, clipping and
  final compositor semantics remain unbound. In
  particular, the active NBG0
  bitmap is neither `TITLE.CG`/MAPD nor `LOGOBG.DG2`; a full-disc
  resident-member scan finds no other complete source file. Identify that
  producer (including any decode/transform), alongside the actual title
  display consumer and timing, before admitting composition. The measured
  title NBG0 span is `0x00000`–`0x1ffff` (SHA-256
  `ad10d99f00c3eecdf9577b15af1a7b86870a4ba83299dc50a09881dc569ad5e8`);
  retained traces after frames 11900 and 12501 have no writes in that range.
  Capture the later update with VDP2-range writes, writer registers and
  same-frame SH-2 RAM/CD provenance. The title-frame NBG1 route is captured
  and excludes `TITLE.CG` (it resolves to `0x20000`),
  so it must not be promoted as a title-map substitute.
- Calibrate the title/menu capture from post-composition frames after the
  Saturn BIOS and the unskippable opening movie. The verified JP title window
  at frames 13000–13039 is bit-identical with and without Start/A pulses, so
  it is still non-interactive animation—not the title-menu transition that
  native startup must implement.
- Implement native Saturn runtime semantics only where a real dispatcher,
  material, event, save or audio consumer has been captured and verified.
- Resolve Structure2/VDP1 material, texture, palette, animation and timing
  ownership with real captures; keep unbound bytes and generated fixtures
  out of production gameplay.
