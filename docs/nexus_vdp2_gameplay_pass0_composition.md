# Nexus gameplay pass 0 VDP composition

This regression joins an authentic Ymir capture from the Japanese retail Nexus
edition to Firestaff's host-side VDP compositor. It covers the complete
320×224 RGB frame for one gameplay save-state frame and keeps the save state,
BIOS, disc image, VRAM dumps, and rendered pixels outside the repository.

## Evidence and result

- Ymir loaded save-state slot 3 from the authentic Japanese retail CUE. The CUE
  SHA-256 is `878b3bc223bb56e99626dbaa2946d17fb349a2a67a10cd4423742d8ab30b8699`;
  the Saturn IPL SHA-256 is
  `dcfef4b99605f872b6c3b6d05c045385cdea3d1b702906a0ed930df7bcb7deac`; the
  save-state SHA-256 is
  `156fa6982798ac7850ff21d12b7ce0b7dee278e0f501f55e18178fd96a2dce2e`.
- Captured VDP2 state has `BGON=0x0007`, `CHCTLA=0x1013`, and
  `CHCTLB=0x0002`. The active background mix includes NBG0 bitmap, NBG1 8bpp
  tilemap, and NBG2 tilemap, together with VDP1 sprites.
- Ymir's bounded NBG1 dot-fetch trace contains 71,680 samples. All PND, tile
  data, and dot reads are valid; the dot histogram is 71,606 zero dots and 74
  dots with value 3. Comparing each traced dot value with the same-pass raw
  VDP2 VRAM dump produced zero mismatches.
- Firestaff's Ymir priority compositor produced 64,804 VDP1 pixels, 3,297
  NBG0 pixels, 74 NBG1 pixels, and 3,505 NBG2 pixels, with no empty pixels,
  unsupported color-calculation cases, or unsupported shadow cases.
- Comparing the Firestaff result with Ymir's final composed RGBX framebuffer
  produced **zero RGB mismatches across all 71,680 pixels**. The reference
  framebuffer SHA-256 is
  `9dcb81a08e186aedf674ac72a89669715926f205e072322114c9ac5edc528ebb`.

The full emulator trace, per-pass register and memory snapshots, exact rendered
frame, Firestaff output, receipts, hashes, and replay log are stored outside Git
at `firestaff-evidence/nexus/ymir-vdp2-gameplay-pass0-20261101` on the external
disk. This proves parity for this captured frame and state. It does not establish
parity for other Nexus scenes, save states, video modes, or the complete game.
The compositor remains capture-only and is not admitted as the live renderer.

## Reproducing the capture comparison

Build `test_nexus_v1_vdp2_runtime_tilemap`, then set the following environment
variables to the authentic capture files. `PASS0_STACK_OUT` can point to any
writable output path. The exact source hashes and expected layer counts are
pinned in `tests/test_nexus_v1_vdp2_runtime_tilemap.c`.

```sh
FIRESTAFF_NEXUS_YMIR_PASS0_VDP1_FBS=/path/to/vdp1-fbs.bin \
FIRESTAFF_NEXUS_YMIR_PASS0_VDP2_VRAM=/path/to/vdp2-vram.bin \
FIRESTAFF_NEXUS_YMIR_PASS0_VDP2_CRAM=/path/to/vdp2-cram.bin \
FIRESTAFF_NEXUS_YMIR_PASS0_VDP2_REGS=/path/to/vdp2-registers.bin \
FIRESTAFF_NEXUS_YMIR_PASS0_RENDERED=/path/to/composed-frame.rgbx8888le \
FIRESTAFF_NEXUS_YMIR_PASS0_STACK_OUT=/path/to/firestaff-priority-stack.bin \
./build/test_nexus_v1_vdp2_runtime_tilemap
```

Without those environment variables, the test still runs its existing bounded
NBG decoder checks and skips the external-media composition comparison.

## Cross-state repeatability check

The same PR revision was also rebuilt on TRV2 and run three times against the
authentic frame-8037 title-menu capture. Each run matched both Ymir VDP2 pass
buffers exactly across all 71,680 RGB pixels. The output reports 70,185 direct
VDP1 pixels, 1,495 palette/special VDP1 pixels, and 18,513 NBG0 overlay pixels;
the compositor remains capture-only and does not claim DGN source ownership.
The verbose CTest logs and run details are retained externally under
`firestaff-evidence/nexus/ymir-vdp2-repeat-validation-20261101/frame8037/`.
The test implementation and frame-specific limitations are documented in
[`nexus_vdp1_frame8037_ymir_composition.md`](nexus_vdp1_frame8037_ymir_composition.md).
