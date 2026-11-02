# Authentic Nexus VDP2 composition at frame 7486

## Same-frame capture

An authentic Japanese retail *Dungeon Master Nexus* cold boot was captured with the Yeager Ymir fork on TRV2. The retained full-height capture is:

`/Volumes/Extern-disk/firestaff-evidence/nexus/nexus-vdp2-full-frame7486-sameframe-20261102`

It targets VDP1 frame 7486 and Ymir VDP2 render serial 8199, rows 0 through 223. The 512-byte VDP2 register snapshot is from frame 7486 itself. It records `BGON=0x0003`, `CHCTLA=0x0013`, `BMPNA=0`, `MPOFN=0`, `PLSZ=0`, and `MAP1A/B=0x1717`. This is not the separate older capture whose final snapshot was from frame 7663.

Ymir receipts report 224 unique NBG0 row samples, 1,568 layer-output records, one same-pass 320×224 RGBX framebuffer, and no output truncation or write errors. The frame-scoped VDP2 memory receipt is complete and has zero VRAM/CRAM writes and zero dropped rows. `README.md` and `SHA256SUMS.txt` in the evidence directory record the capture scope and file hashes. Firmware, profile, backup RAM, and save states are excluded.

## Ymir NBG0 source and composition verification

Run the analyzer:

```sh
python3 scripts/analyze_nexus_vdp2_layer_output.py \
  /Volumes/Extern-disk/firestaff-evidence/nexus/nexus-vdp2-full-frame7486-sameframe-20261102
```

For this capture it reports:

- NBG0 bitmap/CRAM colors and priority match Ymir's layer-output plane at all 71,680 pixels.
- NBG0 has 21,455 active pixels and 50,225 transparent pixels. Every active pixel has special color calculation enabled.
- Reconstructed composition from the captured layer planes matches the same-pass Ymir RGB frame at all 71,680 pixels.
- Transparent NBG0 pixels expose 41,341 sprite-only and 8,884 back-screen-only pixels. Active NBG0 blends over 14,002 sprite pixels and 7,453 back-screen pixels.

This analyzer validates Ymir's captured source-to-layer and layer-to-frame data. It does not execute Firestaff's compositor.

## Firestaff C capture replay

`tests/test_nexus_v1_vdp2_nbg0_live_rows_real.c` replays all 224 captured rows with the Firestaff NBG0 capture compositor. The Python wrapper validates capture hashes, same-frame receipts and row state, confirms the 4,096 NBG1 map entries/transparent character bytes, and compares every Firestaff RGB pixel against Ymir's same-pass RGBX output. This is capture-only verification; it does not enable production-renderer admission.

On 2026-11-02 the test was built and run on TRV2 against this frame-matched
capture. CTest executed the test (not a skip): all 71,680 Firestaff RGB pixels
matched the same-pass Ymir RGBX pixels exactly, and the wrapper verified all
4,096 NBG1 PND entries plus the transparent character bytes before the C
replay. The C receipt reports 21,455 NBG0 pixels and 50,225 VDP1 underlay
pixels. This confirms the complete captured NBG0 row span in the capture-only
compositor for this one frame.

To reproduce it with the same external evidence bundle:

```sh
export FIRESTAFF_NEXUS_NBG0_LIVE_CAPTURE_DIR=/Volumes/Extern-disk/firestaff-evidence/nexus/nexus-vdp2-full-frame7486-sameframe-20261102
export FIRESTAFF_NEXUS_NBG0_LIVE_ROW_TRACE="$FIRESTAFF_NEXUS_NBG0_LIVE_CAPTURE_DIR/dumps/nexus-nbg0-vdp2-render-state.csv"
ctest --test-dir <build-dir> -R '^nexus_v1_vdp2_nbg0_live_rows_real$' --output-on-failure
```

CTest returns a skip when either evidence path is absent. A CI pass where the external capture is unavailable is not proof that this real-data replay ran. The run above used the SHA-256-pinned bundle and retained its inputs outside Git.

## Scope limits

The capture proves one authentic startup frame's NBG0 bitmap/CRAM output and full Ymir-frame composition. It does not prove NBG1 output, interactive menu transitions, dungeon/HUD states, other frames, original-Saturn hardware parity, or parity through Firestaff's production renderer. The retained SH-2 memory-write trace is truncated and does not establish exclusive final-writer attribution.
