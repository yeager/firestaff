# Firestaff v3.0.337

## DM2

### Fixed

- `FM Towns IMG2/IMG6 decoder`: correct source-header, flagged-dimension,
  transparent-run, and packed-nibble handling against the documented FM Towns
  data layout, preventing decoder stream desynchronisation in native DM2
  rendering.

## Developer changes

- `FM Towns original-capture tool`: add DM2 as an explicit supported original
  startup-capture target without adding an emulator dependency to Firestaff.

- `CSB Atari original-capture tool`: validate recorded PCM signal, record the
  selected headless audio backend, and route injected input through the focused
  emulator window for reproducible private capture sessions.
