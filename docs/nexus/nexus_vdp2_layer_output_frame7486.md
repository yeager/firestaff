# Authentic Nexus VDP2 layer output at frame 7486

## Capture

An authentic Japanese retail *Dungeon Master Nexus* cold boot was captured with the Yeager Ymir fork on TRV2. The target is VDP1 frame 7486 and VDP2 render serial 8199, rows 4 through 116 inclusive.

Ymir's receipts report 113 of 113 NBG0 row samples and 791 layer records, with no layer-output truncation or write error. There are seven layer records for each sampled row. A same-pass 320×224 RGBX framebuffer was also captured. The receipts and raw captures are retained outside Git at:

`/Volumes/Extern-disk/firestaff-evidence/nexus/nexus-vdp2-layer-output-20261102`

The layer stream uses Ymir format `YMLYR001`; its SHA-256 is `6c761ae823976939d8044a31fe4f4b4f0713619dc0e3c8751459d38205d7d0b6`. The paired framebuffer SHA-256 is `6d0742b5e7c96c523890c4f15b6fe0c51538c8330393b33eaeb66ce29cbd11a8`.

## NBG0 source-to-layer match

Ymir's render-state rows show NBG0 enabled, bitmap mode active, priority 4, color calculation enabled at ratio 15, and RBG1 disabled. The layer-output receipt assigns slot 2 to NBG0/RBG1; with this state, it is the NBG0 slot. The final memory snapshot reports an 8bpp base-zero bitmap lane. Its VDP2 write receipt covers frame range `7486:7664`, reports a complete trace with zero dropped rows, and shows no writes to the NBG0 bitmap range `0x00000..0x1ffff` or CRAM through snapshot frame 7663.

Using each sampled row's captured scroll and increments, all 36,160 source pixels map from the retained 8bpp VRAM plane and big-endian CRAM words to Ymir's slot-2 color plane exactly. The calculated priority also matches at all 36,160 positions: 21,209 nontransparent pixels have priority 4 and 14,951 transparent pixels have priority 0. All 21,209 active pixels carry the special-color-calculation flag.

The final snapshot's register image is from frame 7663, not frame 7486. Its base-zero BMPNA/CRAOFA values corroborate the source mapping but do not provide a frame-bound register history. The exact layer-color match across every sampled position is the direct pixel evidence.

## Same-pass composition

Reconstructing the 113 captured scanlines from the recorded layer planes and Ymir's ratio-15 color-calculation rule matches the same-pass RGB framebuffer at all 36,160 pixels. NBG0's active pixels blend over the sprite layer where sprite priority is present (13,764 pixels) and over the black back screen otherwise (7,445 pixels). Transparent NBG0 positions expose the sprite layer (10,101 pixels) or the black back screen (4,850 pixels). The other captured background and mesh layer slots have zero priority throughout.

This establishes exact NBG0 bitmap-byte → CRAM color → layer output → framebuffer RGB for these 113 scanlines. The analysis tool reproduces the checks from the retained Ymir data; it does not invoke Firestaff's production renderer.

## Limits

The capture covers 113 of 224 output rows, not a complete frame. It does not establish NBG1 contribution or enablement, another frame or gameplay state, interactive parity, or original-Saturn hardware parity. The layer slot for NBG1 is shared with EXBG, and that slot is zero in this pass.

The complete SH-2 memory-write trace retained 250,000 rows and dropped 1,660,912 master rows, so exclusive final-writer attribution remains unresolved. The source-memory write trace proves bitmap and CRAM stability for the captured interval; it does not supply a complete register-write history.

## Reproduction

Run the source and composition checks against the retained external capture:

```sh
python3 scripts/analyze_nexus_vdp2_layer_output.py \
  /Volumes/Extern-disk/firestaff-evidence/nexus/nexus-vdp2-layer-output-20261102
```

The evidence bundle includes capture files, Ymir receipts, a screenshot from the same run, an English README, both analyzers, and a SHA-256 manifest. The IPL, emulator profile, and save-state files were excluded.
