# Firestaff TODO — DM2

Reviewed 2026-08-29. Only open work is listed here.

- Complete source-owned record-pool, relocation and `SKSAVE` ownership using
  authentic saves; do not promote reduced state layouts as retail parity.
- Extend real-media gameplay evidence across DOS, Amiga, FM Towns and Mac for
  dialog/input ordering, creature AI/drop routes, audio and save/resume.
- Replace the optional FFmpeg-only Macintosh QuickTime/MooV pixel decoder
  with bounded native Cinepak (`cvid`) decoders for `Title.MooV`,
  `Credits.MooV` and `Ending.MooV`, plus `twos` PCM for `Title.MooV`.
  `Swoosh.MooV` is already decoded natively as 16-bit QuickTime Animation
  (`rle `) with its source `raw ` PCM. The native atom/sample-table reader
  proves the exact spans in the retained original view; the normal build must
  decode every remaining span in memory with no host codec or runtime
  dependency before the Macintosh title route is promoted as verified.
- Complete the native PC-DOS WIELD command owner before promoting the retail
  `SKSAVE1` creature-drop regression. Capture an original input-to-CD/RAM
  trace with its valid encounter, weapon choice, command arguments and RNG
  timing, then bind the remaining WIELD fallback/luck path to that trace. Do
  not replace it with a generated weapon, creature, save or combat result.
- Bind renderer/HUD V2.2 material, clipping and outdoor routes to original
  GDAT/capture evidence; synthetic V2.2 art is allowed only as a fixture.
