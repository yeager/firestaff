# CSB V1 Amiga RGB4 original-capture comparison

Date: 2026-09-05

## Authenticated inputs

| Input | SHA-256 |
|---|---|
| Supplied CSB Amiga archive | `fa0296c7bf62d806e8b1c45542d8ac225b67f4080026fa7f6fb61c97905f24c0` |
| Disk 1 ADF (read by FS-UAE only from a RAM staging directory) | `addaaa51255affcd9c53cc40026470b880eaba337c54a62a48ecb5b3b31a5f4d` |
| Disk 2 ADF (read by FS-UAE only from a RAM staging directory) | `7fd59a061ab92a4f3380393e19ad294a6eef9adb31fcf71380850c0c9b91185e` |
| Supplied Kickstart 1.3 archive | `c9521c114900633c09317ca6ff979db7b9df34d3cb537de062f5d51811c42c04` |
| Kickstart 1.3 ROM used by FS-UAE | `ee05862d8102a08436ac4056da7d549db31625c7d47b24dfb7b3c9a5c113ca53` |

The product path continued to read the supplied ZIP/ADF data in memory. The
temporary ADF and ROM copies existed only under `/dev/shm` for the external
reference emulator and were deleted after this comparison. Firestaff has no
Kickstart or emulator dependency.

## Original observations

FS-UAE 3.2.35, A500 and Kickstart 1.3 reached these authentic surfaces from
disk 1: the FTL logo at approximately 30 seconds, `Presents` at 37 seconds,
the complete `Chaos Strikes Back` title at 42 and 47 seconds, and the APPB
language page from 52 seconds onward. The emulator's complete-title frame used
the expected Amiga RGB4 host values, including `(255,0,0)`, `(0,136,136)`,
`(136,136,0)`, `(0,255,0)` and `(255,255,0)`.

The first comparison exposed a real Firestaff defect. Its source-owned
`TITL.DAT` pixels were correct, but RGB4 registers had been shifted into the
VGA six-bit palette API. Consequently nibble `0xf` was presented as 243 and
nibble `0x8` as 130 instead of the Amiga values 255 and 136. Screenshot output
repeated the same quantization even after the live renderer was corrected.

## Fix and current proof

`M11_Render_SetIndexedPaletteRgb4` now expands each authentic Amiga register
component exactly as `(nibble << 4) | nibble`. CSB's Amiga title, entrance,
credits and dungeon/HUD presenters use that path. The screenshot writer copies
the renderer's actual RGB8 table, so capture evidence no longer re-quantizes
the fixed frame through VGA RGB6.

The same supplied CSB ZIP now produces these dominant Firestaff title colors:

| Pixel value | Count in the checked 320x200 frame |
|---|---:|
| `(0,0,0)` | 37296 |
| `(0,136,136)` | 10020 |
| `(255,0,0)` | 8734 |
| `(136,136,0)` | 4083 |
| `(0,136,0)` | 1040 |
| `(136,0,0)` | 1022 |
| `(255,255,0)` | 680 |
| `(0,255,0)` | 487 |

The focused real-media M12/M11 test also checks all 256 title palette entries
at the RGB8 presentation boundary and checks the dungeon and C005 credits
palettes there. It passed 55/55.

This promotes exact palette-register expansion, not full same-VBlank pixel
parity. The FS-UAE wall-clock capture and Firestaff source-VBlank test use
different capture clocks; an exact synchronized source-frame image pair is
still required before claiming animation pixel/timing parity.

## Atari diagnostic (not a parity capture)

Hatari 2.6.1 was also run with the supplied protected Atari STX
(`d9aed23f7916d60dfef61c7b79bc3eb1995f8afbb6a6c8b7b4160ee12ada1025`).
The repository firmware image
(`5393932066f3199a6a653dfd1f1524bb52375ae0ad0831720743c2e015360a2b`)
identifies itself as EmuTOS and reproducibly reaches the original program's
`Panic: Bus Error` screen on both ST and STE configurations. This is evidence
that this EmuTOS setup cannot authenticate a retail Atari title capture; it is
not evidence of an Atari Firestaff rendering discrepancy. No screenshot from
that failed route is retained or promoted.
