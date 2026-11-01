# Authentic Nexus VDP2 layer output at frame 7486

## Capture

An authentic Japanese retail *Dungeon Master Nexus* cold boot was captured with the Yeager Ymir fork on TRV2. The full-height target is VDP1 frame 7486 and VDP2 render serial 8199, rows 0 through 223.

Ymir's receipts report 224 of 224 NBG0 row samples and 1,568 layer records, with no layer-output truncation or write error. There are seven layer records for every row. A same-pass 320×224 RGBX framebuffer was captured. The full-height capture is retained outside Git at:

`/Volumes/Extern-disk/firestaff-evidence/nexus/nexus-vdp2-full-frame-7486-20261102`

A separate partial-height capture with the same framebuffer hash and additional SCU DMA payload traces remains at `/Volumes/Extern-disk/firestaff-evidence/nexus/nexus-vdp2-layer-output-20261102`.

The full layer stream uses Ymir format `YMLYR001`; SHA-256 `6a0f43fcd1effe7d5c5144fc435ad55de054c26f6cd484223cac93465dcb5946`. The paired framebuffer SHA-256 is `6d0742b5e7c96c523890c4f15b6fe0c51538c8330393b33eaeb66ce29cbd11a8`.

## NBG0 source-to-layer match

Ymir's row-state trace shows NBG0 enabled, bitmap mode active, priority 4, and color calculation enabled at ratio 15. RBG1 is disabled for this pass. Layer-output slot 2 is shared by NBG0/RBG1; with the sampled state it is the NBG0 output.

Using each row's captured scroll and increments, all 71,680 output positions map to the authentic 8bpp VRAM bitmap exactly. Each of the 21,455 nonzero indices selects an authentic big-endian CRAM word whose decoded color matches Ymir's slot-2 plane; all 50,225 zero indices produce matching transparent color and priority. The computed priority matches at every position, and every active NBG0 pixel has the special-color-calculation flag.

The VDP2 memory-write receipt covers frame range `7486:7664`, reports a complete trace with zero dropped rows, and includes 14,364 writes. It shows no writes to the NBG0 bitmap range `0x00000..0x1ffff` or CRAM through snapshot frame 7663, binding those source bytes from the target frame to the snapshot. The final register snapshot reports the base-zero BMPNA/CRAOFA values, but it is not a frame-bound register-write history.

## Same-pass full-frame composition

Reconstructing all 224 scanlines from the recorded layer planes and Ymir's ratio-15 color-calculation rule matches the same-pass RGB framebuffer at all 71,680 pixels. Active NBG0 pixels blend over 14,002 sprite pixels and 7,453 black back-screen pixels. Transparent NBG0 positions expose 41,341 sprite pixels and 8,884 black back-screen pixels. Layer slots 1, 3, 4, 5, and 6 have zero priority across the frame. Slot 3 is shared by NBG1/EXBG, so this does not establish NBG1's enable state.

This verifies exact bitmap-byte → CRAM color → NBG0 output → full Ymir RGB-frame composition for this startup pass. The analyzer checks retained Ymir layer and memory data; it does not invoke Firestaff's production renderer.

## Limits

This is one authentic Ymir cold-boot frame. It does not establish NBG1 output, interactive menu transitions, dungeon/HUD states, other frames, original-Saturn hardware parity, or production-renderer parity. The SH-2 memory-write trace retained 250,000 rows and dropped 1,660,912 master rows, so exclusive final-writer attribution remains unresolved.

## Reproduction

Run the source and composition checks against the full-height external capture:

```sh
python3 scripts/analyze_nexus_vdp2_layer_output.py \
  /Volumes/Extern-disk/firestaff-evidence/nexus/nexus-vdp2-full-frame-7486-20261102
```

The evidence bundle includes capture files, Ymir receipts, a screenshot from the same run, an English README, an analyzer, and a SHA-256 manifest. The IPL, emulator profile, and save-state files were excluded.
