# Nexus frame 8037 capture composition

The authentic Japanese retail Ymir capture at VDP1 frame 8037 is replayed by
Firestaff's bounded, capture-only VDP compositor. The test joins the Ymir VDP1
display framebuffer to VDP2 VRAM, CRAM, and registers from the same pinned
capture bundle. It verifies the complete VDP1 command-trace receipt, including
178 rows for frame 8037 and zero dropped rows.

The compositor output matches every RGB pixel in both Ymir VDP2 pass buffers
(serials 9118 and 9119): 71,680 of 71,680 pixels, with zero channel
differences. The two pass buffers have identical bytes. Independently, the
retained 640×448 Ymir frontend screenshot is an exact 2× nearest-neighbor
match to those pass buffers.

The observed VDP2 state includes `TVMD=0x8000`, `BGON=0x0001`,
`CHCTLA=0x0013`, `SPCTL=0x1536`, and `CCCTL=0x0043`. The captured palette
pixels use priority 5 and ratio index 0, and the 8bpp NBG0 bitmap overlays the
same frame. The compositor resolves these samples from the captured FBS,
CRAM, bitmap, and register bytes. This does not join each displayed pixel to a
retail command or source asset.

## Reproduce with authentic external data

Extract the hash-pinned raw dumps from
`firestaff-evidence/nexus/nexus_vdp2_frame_join_20261030/capture-artifacts.tar.gz`
to a local external directory. Keep the game data and capture dumps outside
Git. The pass references are in
`firestaff-evidence/nexus/ymir_nbg0_frame_serial_diagnostic_20261030/rendered_frame_capture/frame8037-authentic-target/vdp2-frames/`.

```sh
cmake --build build --target test_nexus_v1_vdp2_ymir_frame8037_real
FIRESTAFF_NEXUS_YMIR_FRAME8037_CAPTURE_DIR=/path/to/external/dumps \
FIRESTAFF_NEXUS_YMIR_FRAME8037_REFERENCE_DIR=/path/to/external/vdp2-frames \
ctest --test-dir build -R '^nexus_v1_vdp2_ymir_frame8037_real$' --output-on-failure
```

The test pins SHA-256 for the FBS, VDP2 VRAM, CRAM, registers, command trace,
capture receipts, and both Ymir pass files. It skips when the external capture
paths are not provided and fails on missing or changed files.

This is emulator-internal same-run composition evidence. It does not establish
original-Saturn parity, complete retail source ownership, interactive menu
behavior, later startup transitions, or dungeon/HUD rendering. The result is
capture-only and `renderer_permitted=false`; the production renderer gate
remains closed.
