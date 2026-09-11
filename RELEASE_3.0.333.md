# Firestaff v3.0.333

## Fixed

- `Theron CloneCD CUE loader`: fixes shared-BIN CloneCD CUE startup without
  extracting or duplicating user-owned game data.

- `DM2 platform-media selection`: fixes an explicitly selected Macintosh media
  route so it cannot fall back to a different platform's data.

- `DM1 original DOS capture route`: fixes the bounded HoC render and input
  verification timeout so its complete live matrix finishes before a release
  gate evaluates it.
