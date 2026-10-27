# Firestaff v3.0.334

## User-facing changes

- `Theron CloneCD CUE loader`: fixes shared-BIN CloneCD CUE startup without
  extracting or duplicating user-owned game data.

- `DM1 Original input mapping`: corrects pointer conversion through scaled host
  presentation targets, so Entrance and Hall of Champions controls retain their
  source-owned hit areas.

- `DM2 platform-media selection`: prevents an explicitly selected Macintosh
  media route from falling back to another platform's data.

## Developer changes

- `SDL dependency discovery`: rejects SDL2 as an SDL3 substitute during CMake
  configuration, producing an immediate actionable dependency error instead of
  a later compiler or ABI failure.
