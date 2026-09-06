# DM1 PC3.4 scroll text coordinates

## Original-media evidence

The original English PC3.4 GRAPHICS.DAT was read directly from the user's
ZIP into memory, with entry size and CRC checked. Its SHA-256 is
`2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e`.
The C696 layout at byte 293487 has magic `0xfc0d`. These records agree
with `data/zones_h_reconstruction.json`:

| Zone | Type | Parent | Data1 | Data2 |
|---|---:|---:|---:|---:|
| 3 | 9 | 0 | 224 | 136 |
| 4 | 1 | 3 | 0 | 0 |
| 100 | 9 | 4 | 144 | 73 |
| 101 | 0 | 100 | 152 | 89 |
| 560 | 0 | 101 | 84 | 35 |

## Source calculation

ReDMCSB WIP20210206 `COORD.C` F0635 (2052–2412) and F0636
(2434–2448) resolve C101's panel origin to `(80,52)`. Resolving C560
with the one-pixel dimensions supplied by F0636 gives
`(80+84-1,52+35-1) = (163,86)`.

`PANEL.C` F0341 (971, 1009–1018) computes the PC3.4 first-line baseline
as `86 - floor((7*n-2)/2) + 6`, for logical line count `n`.
`TEXT2.C` F0644 (130–143) converts that baseline to a six-pixel raster
top by subtracting four. Thus:

- Horizontal text center: viewport X **163**.
- First raster row: viewport Y **89 - floor(7*n/2)**.
- Subsequent lines: seven pixels apart.
- Character advance: six pixels, black ink on opaque white cells.

The older Atari path in PANEL.C passes `92-floor(7*n/2)` to F0040;
that is a baseline, not a raster top. It must not be used as the PC3.4
raster position. Do not apply the PC3.4 layout to other media without
their own layout/source verification.

This is original-data and source-chain evidence, not a same-state emulator
capture. Japanese fonts, other platform layouts, localized line wrapping,
and complete visual parity remain separate requirements.
