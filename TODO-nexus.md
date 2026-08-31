# Firestaff TODO — Nexus

Reviewed 2026-08-31. Only open work is listed here; completed evidence belongs
in the Nexus capture and reverse-engineering records.

## Available local retail media

The staged JP retail set supplies the CUE, nine BIN tracks, CDDA WAV tracks,
and the original ZIP package.  It does **not** contain a Saturn VDP1/VDP2 VRAM
dump, CRAM dump, register snapshot, or frame/timing capture.  Firestaff reads
the CUE/ISO members directly in memory; it must not manufacture either the
missing capture or a presentation claim from these disc files.

- Capture one same-revision, post-composition title/menu state that jointly
  binds the active NBG0 source, CRAM palette, VDP1/VDP2 layers, priorities and
  timing. The observed NBG0 span remains unowned; do not admit a native title
  renderer or substitute inferred assets until this consumer is identified.
- Capture the actual interactive title/menu transition and its input contract.
  The JP window at frames 13000–13039 is bit-identical with and without
  Start/A pulses, so it is non-interactive animation rather than the native
  startup menu.
- Resolve the remaining Structure2/VDP1 material, texture, CLUT, raster,
  clipping, animation and composition ownership with real captures. Keep
  unbound bytes and generated fixtures out of production gameplay.
- Implement native Saturn runtime semantics only after each dispatcher,
  material, event, save or audio consumer has a captured, hash-verified
  contract.
