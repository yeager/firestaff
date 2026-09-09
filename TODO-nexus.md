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

- Capture the actual interactive title/menu transition and its input contract,
  including the display consumer that places menu and face material.
  The JP window at frames 13000–13039 is bit-identical with and without
  Start/A pulses, so it is non-interactive animation rather than the native
  startup menu.
- Resolve the remaining Structure2/VDP1 material, texture, CLUT, raster,
  clipping, animation and composition ownership with real captures. Keep
  unbound bytes and generated fixtures out of production gameplay.
- Implement native Saturn runtime semantics only after each dispatcher,
  material, event, save or audio consumer has a captured, hash-verified
  contract.
