# Nexus Ymir gameplay VDP priority composition

This capture-only lane composes two hash-pinned Nexus gameplay frames from raw
Ymir VDP1 framebuffer, VDP2 VRAM, CRAM, and register snapshots. It verifies
Saturn layer priority and color selection across gameplay frames 239, 397, and
the four-champion Continue frame 2173. It does
not grant production-renderer permission or claim general Nexus frame support.

## Authentic evidence

Both captures are from Ymir `0.4.2-dev` runs of the authentic Japanese retail
disc, using the corpus-retained four-champion backup-memory image. The
operator-managed capture bundle is on the external disk at
`/Volumes/Extern-disk/firestaff/parity-evidence/verification/nexus_v1_ymir_scheduled_shuttle_mouse_20261030/`.
Its `SHA256SUMS.txt` covers the raw captures and screenshots.

| Ymir frame | Capture | VDP1 FBS SHA-256 | VDP2 VRAM SHA-256 | CRAM SHA-256 | VDP2 registers SHA-256 | Screenshot SHA-256 |
| ---: | --- | --- | --- | --- | --- | --- |
| 239 | `gameplay-mouse-control` | `82caef1162741e5baca99e3a8747abb62bd0568df84c75cc0a781176b6636f04` | `148eebc95b80465a278b7f6d9c1025ee378ec6d288563de3e52ba8af986d94cc` | `f97f5ebf7bcfab263514895097ec40e6c5dca6dd91fe878f1691e9f204d8515d` | `308b3c23bb1db102d3833801a836f0141540fe0870069abb37d2041e57f9b555` | `ffd622503bafd437e4f20513e13b7ea3983f1240c930b755ae4b24986e5c90bf` |
| 397 | `gameplay-controlpad` | `82caef1162741e5baca99e3a8747abb62bd0568df84c75cc0a781176b6636f04` | `148eebc95b80465a278b7f6d9c1025ee378ec6d288563de3e52ba8af986d94cc` | `76c0f491634b7ddcb86f0fe688e35e1e08e0c594640b068bb175abecec669c77` | `e853e43bfc7d73d3425c56bf9c5258217c6db63d12859d4cbf587662be0799cd` | `b03843d943bfade7b68d19c8f195896b59f8aa254c7a8bc549abe3b702d510cc` |
| 2173 | `authentic-bkr-after-accept` | `82caef1162741e5baca99e3a8747abb62bd0568df84c75cc0a781176b6636f04` | `972bf6209ec8543a0665b5077a1c8f069ec213cea100be623b591268e1a03e31` | `244e7d84c910f5555f12250dde6ccf698171682588ba88253fa73fb2478006de` | `e853e43bfc7d73d3425c56bf9c5258217c6db63d12859d4cbf587662be0799cd` | `742ea48a955f07c8490169c6b444b53fb947385ed97a3f1812b684b65b2a2872` |

Frames 239 and 397 share VDP1 FBS and VDP2 VRAM. Sixteen CRAM bytes differ. The
VDP2 register dumps differ only at status register `TVSTAT` (`0x008`), whose
captured word changes from `0x0042` to `0x0086`; the registers that select
display mode, planes, palettes, priorities, scroll, windows, and color
calculation agree. The compositor reads the full captured CRAM and checks the
render-relevant register set for each call.

Both outputs contain 59,884 VDP1 RGB pixels, 491 VDP1 CRAM pixels, 3,954
NBG0 pixels, 148 NBG1 pixels, and 7,203 NBG2 pixels, with no empty pixels.
The compositor follows Ymir's software VDP2 priority rule: higher priority
wins, then the lower internal layer index wins ties. NBG0 and NBG2 use
priority 4; NBG1's PND special-priority bit selects priority 5 for its visible
pixels. Direct RGB555 VDP1 pixels use sprite priority 2; visible palette-form
VDP1 pixels use priorities 6 or 7.

For each capture, the independent verifier compares the native 320×224
composition against all four samples of the 640×448 Ymir screenshot. Frames
239, 397, and 2173 each match all 286,720 samples exactly, with maximum
channel error 0. Frame 2173 is the authentic accepted four-champion BKR dungeon
view; its stack hash is
`116b9ca3a19a6ce2642224d42efbf45a22f0d2481022ae7ba19b6bc100336fdf`. Its
registers include `CCCTL=0x0142` and `SFCCMD=0x0004`. In the Saturn register
layout, CCCTL bit 1 enables NBG1 color calculation (bit 0 is NBG0), and SFCCMD
selects per-character color calculation for NBG1. No visible NBG1 pixel in the
authenticated replay comes from a PND with its color-calculation enable bit
set; the compositor rejects such a pixel. Therefore this frame verifies the
NBG1 calculation-disabled case and its priority composition, but does not
exercise active NBG1 color calculation. Exact screenshot agreement must not be
read as general color-calculation coverage. The earlier stack hashes are
`1fcf190a75bc137f312160e92dfb4d5d0d0b2e1fa84144a035bac15ba974199b` (frame
239) and `402ca6d5d295214f68d574097753ee04c6194f42d82bb832bd902052bbbe97e3`
(frame 397).

## Reproduction

Build `test_nexus_v1_vdp2_runtime_tilemap`. Set the common variables to frame
239. Set the additional `FRAME397` variables to exercise both authentic
captures in the same test invocation:

```sh
CAPTURE_ROOT=/Volumes/Extern-disk/firestaff/parity-evidence/verification/nexus_v1_ymir_scheduled_shuttle_mouse_20261030
OUTPUT_ROOT=/Volumes/Extern-disk/firestaff-evidence/nexus/hud-snapshot-investigation-20261030
FRAME239_DUMPS="$CAPTURE_ROOT/gameplay-mouse-control/dumps"
FRAME397_DUMPS="$CAPTURE_ROOT/gameplay-controlpad/dumps"
export FIRESTAFF_NEXUS_YMIR_VDP1_FBS="$FRAME239_DUMPS/vdp1-fbs.bin"
export FIRESTAFF_NEXUS_YMIR_NBG2_VRAM="$FRAME239_DUMPS/vdp2-vram.bin"
export FIRESTAFF_NEXUS_YMIR_NBG2_CRAM="$FRAME239_DUMPS/vdp2-cram.bin"
export FIRESTAFF_NEXUS_YMIR_NBG2_REGS="$FRAME239_DUMPS/vdp2-registers.bin"
export FIRESTAFF_NEXUS_YMIR_VDP2_STACK_CAPTURE_OUT="$OUTPUT_ROOT/frame239-stack.raw"
export FIRESTAFF_NEXUS_YMIR_FRAME397_VDP1_FBS="$FRAME397_DUMPS/vdp1-fbs.bin"
export FIRESTAFF_NEXUS_YMIR_FRAME397_VDP2_VRAM="$FRAME397_DUMPS/vdp2-vram.bin"
export FIRESTAFF_NEXUS_YMIR_FRAME397_VDP2_CRAM="$FRAME397_DUMPS/vdp2-cram.bin"
export FIRESTAFF_NEXUS_YMIR_FRAME397_VDP2_REGS="$FRAME397_DUMPS/vdp2-registers.bin"
export FIRESTAFF_NEXUS_YMIR_FRAME397_STACK_CAPTURE_OUT="$OUTPUT_ROOT/frame397-stack.raw"
ctest --test-dir /path/to/firestaff-build --output-on-failure \
  -R '^nexus_v1_vdp2_runtime_tilemap$'
python3 scripts/verify_nexus_ymir_priority_composition.py \
  "$OUTPUT_ROOT/frame239-stack.raw" \
  "$CAPTURE_ROOT/gameplay-mouse-control/screenshots/DUNGEON MASTER NEXUS [T-9111G]-20261030T091953.253.png" \
  --capture-frame 239
python3 scripts/verify_nexus_ymir_priority_composition.py \
  "$OUTPUT_ROOT/frame397-stack.raw" \
  "$CAPTURE_ROOT/gameplay-controlpad/screenshots/DUNGEON MASTER NEXUS [T-9111G]-20261030T091814.957.png" \
  --capture-frame 397

FRAME2173_DUMPS=/Volumes/Extern-disk/firestaff-evidence/nexus/nexus_authentic_bkr_continue_20261028/dumps
FRAME2173_ROOT=/Volumes/Extern-disk/firestaff-evidence/nexus/nexus_authentic_bkr_continue_20261028
export FIRESTAFF_NEXUS_YMIR_FRAME2173_VDP1_FBS="$FRAME2173_DUMPS/vdp1-fbs.bin"
export FIRESTAFF_NEXUS_YMIR_FRAME2173_VDP2_VRAM="$FRAME2173_DUMPS/vdp2-vram.bin"
export FIRESTAFF_NEXUS_YMIR_FRAME2173_VDP2_CRAM="$FRAME2173_DUMPS/vdp2-cram.bin"
export FIRESTAFF_NEXUS_YMIR_FRAME2173_VDP2_REGS="$FRAME2173_DUMPS/vdp2-registers.bin"
export FIRESTAFF_NEXUS_YMIR_FRAME2173_STACK_OUT="$OUTPUT_ROOT/frame2173-stack.raw"
ctest --test-dir /path/to/firestaff-build --output-on-failure \
  -R '^nexus_v1_vdp2_runtime_tilemap$'
python3 scripts/verify_nexus_ymir_priority_composition.py \
  "$OUTPUT_ROOT/frame2173-stack.raw" \
  "$FRAME2173_ROOT/authentic-bkr-after-accept.png" \
  --capture-frame 2173
```

Each C replay verifies the raw input hashes before composition. The independent
Python verifier pins the matching screenshot and generated stack hashes, then
checks every doubled RGB sample and the layer counts. Unsupported shadow and
color-calculation cases reject the capture. The current frames contain neither.
This exact match validates only these two captures and the implemented
priority/color cases; it does not establish arbitrary scroll, windows, line
effects, rotation, interlace, other Nexus scenes, or production renderer
admission. Raw game captures remain external to the source repository.

Ymir's raw `vdp1-fbs.bin` dump stores the draw framebuffer first and the display
framebuffer second. The real-capture regression verifies the hash of the whole
dump, then passes the display half to the compositor. Re-running frames 239,
397, 2173, and 300 with this selection produced the same hash-pinned priority
stacks and same-pass RGB results recorded above.
