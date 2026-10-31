# Ordered Nexus VDP1 capture draws

The Ymir title-menu capture at frame 119 records five final VDP1 draws in
order: two selection markers followed by `NEW GAME`, `CONTINUE`, and
`CONFIGURATION`. The retained source-join report binds their texture bytes to
authentic Japanese retail `TITLE.BIN` DGT2/PP records, their palettes to the
active captured CRAM banks, and their visible pixels to the Ymir screenshot.
Across all five draws, 5,780 screenshot samples are within eight RGB levels
per channel.

`Nexus_V1_Vdp2YmirMenuCaptureInput` now accepts a bounded ordered list of
source-joined VDP1 command offsets. The capture compositor validates every
offset and join flag, rejects duplicate command records, and applies draws in
the supplied trace order. The previous single-command input remains supported.
The receipt reports the number of applied commands and the sum of their
written pixels.

The C integration test hash-checks the retained frame-119 FBS, VDP1 VRAM,
VDP2 VRAM, CRAM, registers and reference RGB. It fetches the Japanese retail
`TITLE.BIN` from the user's `.firestaff/data/nexus`, verifies its known SHA-256,
normalizes only the five traced command records, and matches each command's
texture span against its source-file span. The compositor overlays all five
draws in trace order and compares the native 320×224 image against the
retained RGB reference within eight levels per channel. It reports 1,445
native written pixels; the separate 2× screenshot evidence counts 5,780
samples.

To run this external-data case, set these variables to the indicated inputs
before running `test_nexus_v1_vdp2_capture_compositor`:

| Variable | Input |
|---|---|
| `FIRESTAFF_NEXUS_YMIR_MENU119_FBS` | `dumps/vdp1-fbs.bin` |
| `FIRESTAFF_NEXUS_YMIR_MENU119_VDP2_VRAM` | `dumps/vdp2-vram.bin` |
| `FIRESTAFF_NEXUS_YMIR_MENU119_CRAM` | `dumps/vdp2-cram.bin` |
| `FIRESTAFF_NEXUS_YMIR_MENU119_REGS` | `dumps/vdp2-registers.bin` |
| `FIRESTAFF_NEXUS_YMIR_MENU119_VDP1_VRAM` | `dumps/vdp1-vram.bin` |
| `FIRESTAFF_NEXUS_YMIR_MENU119_TITLE` | `.firestaff/data/nexus/TITLE.BIN` |
| `FIRESTAFF_NEXUS_YMIR_MENU119_EXPECTED_RGB` | `screenshots/vdp12-reconstruction-frame119-native.rgb` |

The test also retains the source-joined frame-13162 command case through the
ordered-list interface. Both menu replays remain capture-only and do not
permit the production renderer or establish complete menu input behavior.
