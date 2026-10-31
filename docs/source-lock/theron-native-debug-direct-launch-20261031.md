# Theron native Debug direct-launch boundary (2026-10-31)

## Verified route

An isolated macOS Debug build of Firestaff was launched against authentic US
Track 02 data (MD5 `f23601102138f87c33025877767ebf76`) with native Theron
selection and the direct-launch boot probe. The isolated executable was
passed these arguments:

```text
--debug --game theron --theron-native us --boot-probe
```

The application entered Theron's native title/startup path without showing
Firestaff's Run menu or booting a PC Engine System Card. The System Card menu
and its long RUN hold belong to reference captures only.

## Limits

The boot-probe receipt remained in the startup phase and reported
`levelLoaded=0`. This run does not prove a loaded dungeon, playable gameplay,
dynamic rendering, original-emulator parity, or complete Theron support. The
existing authenticated JP fresh-start and US Continue routes are separate
evidence for reaching loaded native dungeons.

The Debug executable and per-run console output were kept outside the
repository. No game media, firmware, save, emulator capture, or generated
screen was added to Git.
