# Firestaff TODO — Nexus

Reviewed 2026-09-09. Only open work is listed here; completed evidence belongs
in the Nexus capture and reverse-engineering records.

## Available local retail media

The staged JP retail set supplies the CUE, nine BIN tracks, CDDA WAV tracks,
and the original ZIP package.  The disc itself does **not** contain a Saturn
VDP1/VDP2 VRAM dump, CRAM dump, register snapshot, or frame/timing capture.
Firestaff reads the CUE/ISO members directly in memory; it must not manufacture
either a capture or a presentation claim from those disc files.

The native title renderer consumes the retail `TITLE.BIN`/`TITLE.CG` MAPD
planes, selector sequence and BGR555 palettes directly from the selected CUE/
BIN media. `test_nexus_v1_title_mapd_real`, `test_m11_nexus_startup_gate` and
`test_m11_nexus_startup_runtime_handoff` cover this bounded path. The renderer
does not authorise the separate menu, face, HUD or dungeon compositors.

A same-revision, media-immutable title-session receipt now joins retail CD
FIFO records for LBA 6063--6089 to SH-2 RAM source writes for the same range,
cached SH-2 reads, and frame-stamped VDP2 writes/registers for frames
13294--13455. This closes title CD-to-RAM-to-VDP2 provenance only; it is not
menu, HUD, dungeon, audio or input evidence and does not widen the native
presentation boundary.

- Capture the actual interactive title/menu transition and its input contract,
  including the display consumer that places menu and face material.
  The JP window at frames 13000–13039 is bit-identical with and without
  Start/A pulses, so it is non-interactive animation rather than the native
  startup menu. A later same-session control/Start pair establishes that
  Start is read from the live controller register at frames 18020--18023 by
  SH-2 code, but its 64-frame VDP1/VDP2 output remains byte-identical to the
  no-input control. That reader is therefore not yet an admitted menu
  consumer; find the later transition that changes presentation and bind it
  to the retail menu asset consumer.  A separate post-title control/input
  pair at frames 30000--30119 exercised Start, A, B and C in four distinct
  20-frame pulses.  All eight captured video regions (VDP1 state/VRAM/frame
  buffers/draw selector and VDP2 registers/VRAM/CRAM) were byte-identical to
  the no-input control.  The observed active VDP1 state has two stable
  texture sources for frames 30000--30037, but remains an unbound hardware
  observation rather than proof of a menu, HUD or dungeon consumer.  Search
  beyond this window for the first input-correlated presentation change.
- Resolve the remaining Structure2/VDP1 material, texture, CLUT, raster,
  clipping, animation and composition ownership with real captures. Keep
  unbound bytes and generated fixtures out of production gameplay.
- Implement native Saturn runtime semantics only after each dispatcher,
  material, event, save or audio consumer has a captured, hash-verified
  contract.
