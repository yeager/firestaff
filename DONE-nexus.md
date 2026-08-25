# Firestaff DONE — Nexus

Reviewed 2026-08-25. Completed work only.

- Native CUE-media corpus checks read the supplied Saturn title resources,
  SAL, MAP, SLEV and raw data without a runtime emulator dependency.
- The native sound owner now binds retail CUE-declared audio Tracks 02–09 to
  their original BIN payloads and rejects host WAV/OGG/MP3/FLAC substitutes.
  This is source selection only; playback remains closed without a verified
  Saturn decoder and dispatcher.
- Development-only VDP tracing has deterministic emulation-frame filters and
  retains fail-closed source correlation for unbound rendering writes.
