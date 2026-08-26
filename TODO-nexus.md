# Firestaff TODO — Nexus

Reviewed 2026-08-25. Only open work is listed here.

- Bind authentic Saturn CD reads to SH-2 consumers, VDP destinations and
  decoded source assets before admitting title/menu rendering. The title
  `TITLE.CG` producer-prefix is now captured at SH-2 PC `0x06041fa0`, but its
  display-list consumer remains unbound. In particular, the active NBG0
  bitmap is neither `TITLE.CG`/MAPD nor `LOGOBG.DG2`; a full-disc
  resident-member scan finds no other complete source file. Identify that
  producer (including any decode/transform), alongside the NBG1 map and
  title timing, before admitting composition.
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
