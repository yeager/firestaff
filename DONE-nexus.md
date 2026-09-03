# Firestaff DONE — Nexus

## 2026-09-03 — Native CUE-media regression audit

- Re-ran the native production boundaries for combat, magic, light, rest,
  experience and world actions, plus CUE data-track and CDDA binding. The
  retail CUE route and native title mouse route passed without BIOS or
  emulator runtime dependencies. ISO-only probes remained skip-safe because
  the staged retail set is CUE/BIN/WAV media, not a standalone ISO.

Reviewed 2026-08-26. Completed work only.

- Native CUE-media corpus checks read the supplied Saturn title resources,
  SAL, MAP, SLEV and raw data without a runtime emulator dependency.
- The full `LEV00.DGN`–`LEV15.DGN` Structure3 mesh corpus is read directly
  from the supplied retail Track-1 CUE/BIN into RAM and checked against its
  canonical per-level MD5 before parsing.  This is source verification only:
  it does not authorize an unbound Saturn mesh renderer.
- The native sound owner now binds retail CUE-declared audio Tracks 02–09 to
  their original BIN payloads and rejects host WAV/OGG/MP3/FLAC substitutes.
  This is source selection only; playback remains closed without a verified
  Saturn decoder and dispatcher.
- An authenticated Saturn capture verifies NBG1 hardware state: enabled
  bitmap mode, 256-colour code, BMPNA palette bank 0 and scroll origin
  `(0,0)`. It does not identify the bitmap/CLUT source or authorize drawing.
- The same authentic frame's full 512×256 indexed NBG1 span and 256-entry
  CRAM decode in native code using the capture's recorded Saturn byte order.
  This is capture-only evidence: it does not identify an asset owner or
  authorize production presentation.
- The real Japanese CUE independently proves Track-1 `STABG.BIN` reaches the
  native STMP/DMWeb first-map consumer (320×168 with retained source palette);
  it remains `no_draw` without an exact VDP source join.
- Development-only VDP tracing has deterministic emulation-frame filters and
  retains fail-closed source correlation for unbound rendering writes.
- A same-session Japanese retail receipt verifies one SH-2 RAM-to-VDP1 copy:
  PC `0x060135e8` transfers 2 KiB from `0x06027874..0x06028074` to
  VDP1 `0x10a00..0x11200` with Saturn word byte order. It is a completed
  transport observation, not an asset, palette, command or title-rendering
  admission.
- A post-intro Japanese retail title receipt binds the full `TITLE.CG` payload
  to word-swapped VDP2 VRAM `0x24020` and the MAPD palette to word-swapped
  CRAM `0x400`. The raw MAPD planes, tilemap transform, layer placement and
  timing remain unbound, so this does not authorize title rendering.
- The post-intro input receipt is completed as a negative observation: title
  frames 13000–13039 are bit-identical with and without verified Start/A
  pulses, so that window is not an interactive start-menu transition.
