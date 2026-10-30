# Nexus VDP1 and VDP2 composition at Ymir frame 239

This capture-only lane composes one hash-pinned Nexus gameplay frame from raw
Ymir VDP1 framebuffer, VDP2 VRAM, CRAM, and register snapshots. It provides a
reproducible test of Saturn layer priority and color selection. The lane does
not grant production-renderer permission or claim general Nexus frame support.

## Evidence

The retained capture is under
`parity-evidence/verification/nexus_v1_ymir_scheduled_shuttle_mouse_20261030/gameplay-mouse-control/`.
The VDP1 FBS, VDP2 VRAM, CRAM, and register files are hash-pinned by
`tests/test_nexus_v1_vdp2_runtime_tilemap.c`; the comparison screenshot is
also hash-pinned by `scripts/verify_nexus_ymir_frame239_composition.py`. The
raw captures and screenshot stay outside the source changes.

The verified display tuple has `TVMD=0x8000`, `BGON=0x0007`,
`CHCTLA=0x1013`, `CHCTLB=0x0002`, `MAP1A/B=0x1717`, `MAP2A/B=0x1818`,
`PRINA=0x0404`, `PRINB=0x0004`, `SFPRMD=0x0015`, `CCCTL=0x0142`,
`SFCCMD=0x0004`, and `SPCTL=0x1536`. NBG0 is an 8bpp bitmap at VRAM byte
offset `0`; NBG1 is an 8bpp tilemap whose visible name page starts at
`0x5c000`; NBG2 is an 8bpp tilemap at `0x60000`.

The compositor follows Ymir's software VDP2 renderer priority rule: higher
priority wins, then the lower internal layer index wins ties. In this frame,
NBG0 and NBG2 use priority 4. NBG1's PND special-priority bit selects priority
5 for its visible pixels. Direct RGB555 VDP1 FBS pixels use sprite priority 2;
the 491 visible palette-form VDP1 pixels use priorities 6 or 7. VDP1 FBS
pixels are read from its actual 512 KiB raw snapshot, using the first
256 KiB display half verified equal to the second half. RGB555 components are
expanded by shifting each five-bit channel left three bits, matching Ymir's
pixel output on this capture.

The resulting display has 59,884 VDP1 RGB pixels, 491 VDP1 CRAM pixels,
3,954 NBG0 pixels, 148 NBG1 pixels, and 7,203 NBG2 pixels. The independent
verifier compares each native 320x224 result pixel against all four samples
of the 640x448 Ymir screenshot: 286,720 samples match exactly (maximum channel
error 0).

## Reproduction

Build `test_nexus_v1_vdp2_runtime_tilemap`, then set these paths to the
retained raw files:

```sh
export FIRESTAFF_NEXUS_YMIR_VDP1_FBS=/path/to/dumps/vdp1-fbs.bin
export FIRESTAFF_NEXUS_YMIR_NBG2_VRAM=/path/to/dumps/vdp2-vram.bin
export FIRESTAFF_NEXUS_YMIR_NBG2_CRAM=/path/to/dumps/vdp2-cram.bin
export FIRESTAFF_NEXUS_YMIR_NBG2_REGS=/path/to/dumps/vdp2-registers.bin
export FIRESTAFF_NEXUS_YMIR_VDP2_STACK_CAPTURE_OUT=/tmp/frame239-stack.raw
./test_nexus_v1_vdp2_runtime_tilemap
python3 scripts/verify_nexus_ymir_frame239_composition.py \
  /tmp/frame239-stack.raw /path/to/Ymir-frame-239.png
```

The output is four bytes per native pixel: winning layer ID followed by
red, green, and blue. Unsupported shadow and color-calculation cases reject
the frame. The checked frame has neither. This exact match validates only the
captured frame and the implemented priority/color cases; it does not establish
arbitrary scroll, windows, line effects, rotation, interlace, or other Nexus
display states.
