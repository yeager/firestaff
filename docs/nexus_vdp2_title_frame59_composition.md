# Nexus title-prompt VDP composition, frame 59

Firestaff now replays the authentic Japanese Ymir frame-59 title prompt by
compositing the captured NBG0 8bpp bitmap over the captured VDP1 framebuffer.
The replay is limited to this observed zero-scroll, zero-palette-bank
register profile and remains capture-only.

## Authentic capture and replay

The raw files live outside Git in
`/Volumes/Extern-disk/firestaff/parity-evidence/verification/nexus_v1_ymir_scheduled_shuttle_mouse_20261030/baseline/`.
The real-data test pins SHA-256 for VDP1 FBS, VDP2 VRAM, CRAM, VDP2
registers, the VDP1 command bundle and trace, register snapshot, native RGB
reference, and Ymir screenshot. The two 512x256 VDP1 RGB555 buffers are
byte-identical in the capture.

The observed profile has `TVMD=0x8000`, `RAMCTL=0x0300`, `BGON=0x0001`,
`CHCTLA=0x0013`, zero `BMPNA`, `MPOFN`, `CRAOFA`, `CRAOFB`, and NBG0 scroll,
`CCCTL=0x0043`, and `CCR=0x1000`. The hash-pinned raw register bytes at offset
`0xf8` contain `0x0004`; this is the value admitted by the C replay. One older
external report labels the register `PRINA=0x01f4`, which does not match those
raw bytes.

For output coordinates `(x,y)` in 320x224, a nonzero NBG0 index selects its
big-endian CRAM RGB555 word. Index zero selects the same coordinate in the
first VDP1 framebuffer. The replay writes 18,513 NBG0 pixels and 53,167 VDP1
pixels. Its RGB triplets match the authentic native-size Ymir reference
exactly (71,680/71,680). A separate 2x nearest-neighbor comparison against the
hash-pinned 640x448 screenshot accepts all 286,720 samples within eight RGB
levels; maximum channel error is eight.

Run the regression with the retained real capture:

```sh
CAPTURE=/Volumes/Extern-disk/firestaff/parity-evidence/verification/nexus_v1_ymir_scheduled_shuttle_mouse_20261030/baseline
FIRESTAFF_NEXUS_YMIR_FRAME59_CAPTURE_DIR="$CAPTURE" \
  ctest --test-dir /path/to/firestaff-build --output-on-failure \
  -R '^nexus_v1_vdp2_title_frame59_real$'
```

The test requires Python Pillow for the screenshot comparison. It creates and
removes its replay output inside the build directory. No game data or emulator
is needed at runtime.

## Limits

This proves one captured title-prompt state and its exact captured inputs. It
does not prove the bitmap's complete retail asset owner, general VDP1/VDP2
priority or color-calculation behavior, other title/menu states, original
Saturn parity, or production renderer admission. A 921-byte visible TITL
source span is independently bound; the full NBG0 source and transform remain
unresolved. See [the title evidence](nexus/nexus_title.md).
