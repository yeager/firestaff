# Nexus NBG1 4bpp MAPD capture

## Evidence

This capture is an authentic Japanese retail run in Ymir. The pass is VDP1
frame 891 / VDP2 serial 1280. The capture bundle is retained outside Git at
`firestaff-evidence/nexus/nexus_mapd_ymir_vdp_snapshots_20261031/`.

The hash-pinned VDP2 inputs are:

| Input | SHA-256 |
| --- | --- |
| VDP2 VRAM | `7367347e6adcb72ac09c946d03a15e1486e12f32593cee7bf3992e58da3b3d9f` |
| VDP2 CRAM | `db282593790dec4a2bf8f76293c0458a1a140adc3351650bd56a417f0359e0de` |
| VDP2 registers | `774fa7f2e815b14770b83feb83e8c15406041a931e65ed465b1062944a811dd1` |

The pass has `BGON=0x0002` (NBG1 enabled), `CHCTLA=0x0013` (NBG1 4bpp
tilemap), `PNCN1=0`, `PLSZ=0`, `MPOFN=0`, and `MAP1A/B=0x1717`. The
NBG1 name table begins at byte offset `0x5c000`. Its 64×64 page exposes a
40×28 visible region. The decoder records 1,120 unique cells, 71,680
nontransparent pixels, and zero transparent pixels for this pass.

## Reproduction

From the Firestaff repository root, build the focused test, then point it at
the external raw snapshot files:

```sh
cmake --build .codex-scratch/build-fast-dev --target test_nexus_v1_vdp2_runtime_tilemap -j1
capture=/Volumes/Extern-disk/firestaff-evidence/nexus/nexus_mapd_ymir_vdp_snapshots_20261031/vdp-snapshots
FIRESTAFF_NEXUS_YMIR_MAPD_NBG1_VRAM="$capture/vdp1-891-vdp2-1280-vdp2-vram.bin" \
FIRESTAFF_NEXUS_YMIR_MAPD_NBG1_CRAM="$capture/vdp1-891-vdp2-1280-vdp2-cram.bin" \
FIRESTAFF_NEXUS_YMIR_MAPD_NBG1_REGS="$capture/vdp1-891-vdp2-1280-vdp2-registers.bin" \
FIRESTAFF_NEXUS_YMIR_MAPD_NBG1_OUT=/tmp/nexus-nbg1-frame891-1280.rgba \
  .codex-scratch/build-fast-dev/test_nexus_v1_vdp2_runtime_tilemap
```

The test verifies each input hash and the layer receipt. Its output plane SHA-256
is `a7d55feabe7ec09beb568808cb7674274f04b7e90be3758f8d91ed8aad2a0dea`.
The same-pass Ymir RGBX render is retained externally as
`rendered-frames/vdp1-891-vdp2-1280-320x224.rgbx8888le` with SHA-256
`24cc53361cf3b6e0456bc9b48fddc362d6865ac491376fb203181f3108469f17`.

This verifies the NBG1 fetch and isolated layer decode for one authentic
register state. The output is not the final VDP2 composition: it does not
account for VDP1 overlays, other enabled layers, full color-calculation cases,
or source ownership and placement. The decoder keeps production renderer
permission disabled. No game data or raw emulator capture is tracked in Git.
