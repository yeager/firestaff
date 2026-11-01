# Authentic Nexus VDP2 layer output at frame 7486

## Capture

An authentic Japanese retail *Dungeon Master Nexus* cold boot was captured with the Yeager Ymir fork on TRV2. The target is VDP1 frame 7486 and VDP2 render serial 8199, rows 4 through 116 inclusive.

Ymir's receipts report 113 of 113 NBG0 row samples and 791 layer records, with no layer-output truncation or write error. There are seven layer records for each sampled row. A same-pass 320×224 RGBX framebuffer was also captured. The receipts and raw captures are retained outside Git at:

`/Volumes/Extern-disk/firestaff-evidence/nexus/nexus-vdp2-layer-output-20261102`

The layer stream uses Ymir format `YMLYR001`; its SHA-256 is `6c761ae823976939d8044a31fe4f4b4f0713619dc0e3c8751459d38205d7d0b6`. The paired framebuffer SHA-256 is `6d0742b5e7c96c523890c4f15b6fe0c51538c8330393b33eaeb66ce29cbd11a8`.

## Observations

The render-state trace says NBG0 is enabled, its layer is enabled, bitmap mode is active, and RBG1 is disabled during the sampled rows. In the layer-output stream, slot 2 (shared by NBG0/RBG1) contains nonzero color words at 21,209 of 36,160 samples (58.653%) with priority 4. Given the paired state, this slot is attributable to NBG0 for this pass.

Slot 3 (shared by NBG1/EXBG) and slots 1, 4, 5, and 6 contain zero color words in all 36,160 samples each. The slot identifiers do not distinguish NBG1 from EXBG, so this observation does not establish NBG1's enable state.

The same-pass composed framebuffer has 62,796 nonblack RGB pixels out of 71,680. This is a capture statistic; it is not a pixel-parity result or a source-data join.

## Limits

This capture verifies that Ymir produced complete layer-output records for the sampled render pass and gives Firestaff a reproducible layer-level reference. It does not establish retail pixel parity, identify source bytes for the layer-2 colors, correlate SCU descriptors to this exact frame, or prove NBG1 behavior in other frames. The final memory/register snapshots in the capture bundle are from VDP1 frame 7663, so they are not used as frame-7486 state.

The SH-2 memory-write trace retained 250,000 rows and dropped 1,660,912 master rows. Exclusive final-writer attribution is unresolved.

## Reproduction

Run the analyzer against the external capture:

```sh
python3 scripts/analyze_nexus_vdp2_layer_output.py \
  /Volumes/Extern-disk/firestaff-evidence/nexus/nexus-vdp2-layer-output-20261102/dumps/nexus-vdp2-layer-outputs.bin
```

The evidence bundle includes all capture files, Ymir receipts, a screenshot from the same run, an English README, the analyzer, and a SHA-256 manifest. The IPL, emulator profile, and save-state files were excluded.
