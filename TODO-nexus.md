# Firestaff TODO — Nexus

Reviewed 2026-08-25. Only open work is listed here.

- Bind authentic Saturn CD reads to SH-2 consumers, VDP destinations and
  decoded source assets before admitting title/menu rendering. The full title
  `TITLE.CG` SH-2→VDP2 copy plan is now captured at PC `0x06041fa0`; a
  source-LBA-filtered receipt now reconstructs the complete real `TITLE.CG`
  buffer at `0x0025daf0`. The terminal 32-byte CDB-register path remains
  unbound. The title VDP1 chain now byte-binds all 177 texture spans to
  `LEV00.DGN`; 173 also match their real Structure2 image and CLUT pair. The
  four remaining draws (`0x00e60`, `0x00780`, `0x00880`, `0x01380`) share the
  unbound captured CLUT at word `0xca00`. Geometry and final compositor
  semantics remain unbound. In
  particular, the active NBG0
  bitmap is neither `TITLE.CG`/MAPD nor `LOGOBG.DG2`; a full-disc
  resident-member scan finds no other complete source file. Identify that
  producer (including any decode/transform), alongside the actual title
  display consumer and timing, before admitting composition. The title-frame
  NBG1 route is captured and excludes `TITLE.CG` (it resolves to `0x20000`),
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
