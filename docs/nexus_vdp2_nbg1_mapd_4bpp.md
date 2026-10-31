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
40×28 decoded region. The isolated Firestaff plane decoder records 1,120
unique cells and nonzero palette indices for all 71,680 output pixels. This is
a plane-decode result; it does not establish that every pixel is visible in
Ymir's rendered NBG1 layer.

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

The test verifies each input hash, the layer receipt, and the decoded plane bytes
(index, red, green, and blue for each pixel). The decoded plane SHA-256 is
`a7d55feabe7ec09beb568808cb7674274f04b7e90be3758f8d91ed8aad2a0dea`.
The opt-in command above was rerun against PR head `c1d2664`; it passed with this
hash. The raw inputs and decoded output are retained outside the repository in
`/Volumes/Extern-disk/firestaff-evidence/nexus/nexus_mapd_ymir_vdp_snapshots_20261031/pr239-c1d2664-validation/`.
The same-pass Ymir RGBX render is retained externally as
`rendered-frames/vdp1-891-vdp2-1280-320x224.rgbx8888le` with SHA-256
`24cc53361cf3b6e0456bc9b48fddc362d6865ac491376fb203181f3108469f17`.

This verifies the NBG1 fetch and isolated layer decode for one authentic
register state. The output is not the final VDP2 composition: it does not
account for VDP1 overlays, other enabled layers, full color-calculation cases,
or source ownership and placement. The decoder keeps production renderer
permission disabled. No game data or raw emulator capture is tracked in Git.

## Same-pass layer coverage

Ymir's complete pre-composition layer trace is retained at
`/Volumes/Extern-disk/firestaff-evidence/nexus/nexus_mapd_ymir_vdp_snapshots_20261031/layer-trace-frame891-start60-20261101/`.
The trace SHA-256 is
`086d660dce00d0df4f38a33fe1591043676d0e72482cd0e867540d215a494526`; its
receipt reports 3,136 complete layer-row records across VDP2 serials 1279 and
1280. The selected frame 891 / serial 1280 RGBX pass has SHA-256
`24cc53361cf3b6e0456bc9b48fddc362d6865ac491376fb203181f3108469f17`.

The layer-output analyzer confirms 44,800 nonzero-priority pixels for layer 3
(NBG1/EXBG). They occupy rows 84–223 at priority 5. Rows 0–83 have priority 0
and RGB `(0,0,0)` throughout. Comparing Ymir's layer RGB with the isolated
Firestaff plane yields exact RGB equality for all 44,800 active pixels;
249 of the 26,880 upper-row pixels match. The earlier comparison against the
full Ymir composition is not a valid isolated-layer parity measure and is
removed here.

Reproduce the layer-to-plane comparison with the hash-pinned authentic
artifacts:

```sh
evidence=/Volumes/Extern-disk/firestaff-evidence/nexus/nexus_mapd_ymir_vdp_snapshots_20261031
trace="$evidence/layer-trace-frame891-start60-20261101"
python3 scripts/analyze_nexus_ymir_vdp2_layer_outputs.py \
  "$trace/layers.csv" "$trace/layers.csv.receipt.txt" \
  "$trace/vdp1-891-vdp2-1280-320x224.rgbx8888le" \
  "$evidence/rendered-frames/index.csv" \
  --frame 891 --serial 1280 --layer 3 --width 320 --height 224 \
  --decoded-plane "$evidence/firestaff-nbg1-4bpp-plane-frame891-1280.rgba"
```

The JSON reports 44,800/44,800 exact RGB matches where Ymir gives the layer
nonzero priority, and separately reports the 249/26,880 matches in zero-
priority pixels. Its layer-to-final RGB count compares against the full frame
and must not be used as an isolated-layer parity score.

The upper-row discrepancy remains unresolved: Firestaff's isolated plane has
nonzero palette indices in those rows while Ymir's traced NBG1 layer is fully
transparent. The captured WCTLA–WCTLD window controls are zero, and the PND and
character bytes checked at the map origin are nonzero. Do not apply a fixed
84-row clip or enable this decoder as a full-screen compositor until a
same-frame source or per-scanline state trace explains the difference. VDP1,
other layer ownership, and full color-calculation composition remain separate
parity work.

## SH-2 register-write trace

A separate Ymir run traced writes across the full VDP2 register range in frame
891. Its complete receipt records 127 CPU writes, no DMA writes, and no dropped
rows. The frame-wide register copy set `BGON=0x0002`, `CHCTLA=0x0013`,
`MPABN1=MPCDN1=0x1717`, and zero NBG1 scroll inputs at offsets `0x80`–
`0x86`. The NBG1 coordinate increments are `ZMXIN1=1`, `ZMXDN1=0`,
`ZMYIN1=1`, and `ZMYDN1=0` at offsets `0x88`–`0x8e`. All writes came from
the SH-2 word-copy loop at PC `0x060856ec`.

This independently confirms the active NBG1 setup, but it does not resolve the
upper-row transparency: the trace contains a frame-wide copy, not scanline
state transitions. Window registers are not written in this trace and their
values must be checked in the same-frame register snapshot.

The raw trace, receipt, same-run register snapshot, VDP2 memory, and high work
RAM are retained at
`/Volumes/Extern-disk/firestaff-evidence/nexus/nexus_mapd_ymir_vdp_snapshots_20261031/register-write-frame891-full-20261101/`.
The CSV SHA-256 is
`c8dadb7ec6f89e0dafb83301a75add3ebf2e0b1d59b8df87700886bf2cb3bcc9` and the
complete-receipt SHA-256 is
`9495034ec43c3149b02a0c0519cf8d81911bf6a1da2b80b0bbad36a3b4668daf`.
