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

The captured `SCRCTL` value is zero. The bounded decoder now rejects all four
NBG1 line-state controls in `SCRCTL` bits 8–11 (vertical cell scroll, line
scroll X/Y, and line zoom), because this capture lane does not implement those
modes. The authentic MAPD regression asserts each bit independently and
requires the decoder to reject the altered register state.

## Reproduction

From the Firestaff repository root, build the focused test, then point it at
the external raw snapshot files:

```sh
cmake --build .codex-scratch/build-fast-dev --target test_nexus_v1_vdp2_runtime_tilemap -j1
capture=/Volumes/Extern-disk/firestaff-evidence/nexus/nexus_mapd_ymir_vdp_snapshots_20261031/vdp-snapshots
FIRESTAFF_NEXUS_YMIR_MAPD_NBG1_VRAM="$capture/vdp1-891-vdp2-1280-vdp2-vram.bin" \
FIRESTAFF_NEXUS_YMIR_MAPD_NBG1_CRAM="$capture/vdp1-891-vdp2-1280-vdp2-cram.bin" \
FIRESTAFF_NEXUS_YMIR_MAPD_NBG1_REGS="$capture/vdp1-891-vdp2-1280-vdp2-registers.bin" \
FIRESTAFF_NEXUS_YMIR_MAPD_NBG1_OUT=/Volumes/Extern-disk/nexus-nbg1-frame891-1280.rgba \
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

## Row-effective VRAM source replay

The complete pixel-fetch trace resolves the earlier upper-row discrepancy:
Ymir fetches each NBG1 PND and 4bpp character byte from exactly one of the
adjacent authentic VRAM snapshots. Firestaff now runs the same C tile decoder
against both snapshots, then selects the decoded pixel only when both its PND
and fetched nibble uniquely match that pixel's Ymir trace record. It does not
choose a source by a fixed row number.

For all 71,680 target-pass pixels, the selected Firestaff dot matches Ymir.
The selected output is transparent at all 26,880 zero-priority pixels; its
RGB matches Ymir at all 44,800 visible NBG1 pixels. The source distribution is
derived from the fetch records: 26,880 pixels use the preceding VRAM snapshot
and 44,800 use the target snapshot. The resulting NBG1-only plane has
SHA-256 `359f0ce1a93298f3117cf9c9b6c27550ed3b68ed0c3f65fd4d3e31aa7a42cf9d`.
The standalone current-snapshot plane remains available for comparison at
`a7d55feabe7ec09beb568808cb7674274f04b7e90be3758f8d91ed8aad2a0dea`.
The current plane, previous plane, row-source-composed plane, verification
JSON, and checksums are retained at
`/Volumes/Extern-disk/firestaff-evidence/nexus/nexus_mapd_ymir_vdp_snapshots_20261031/pr252-row-source-replay/`.

This repairs the capture-only NBG1 layer replay for this authentic pass. It
does not yet feed the row-effective layer into Firestaff's whole-frame VDP
priority stack, and it does not enable production rendering or establish
hardware parity. Do not replace the observed per-pixel source map with a fixed
84-row clip.

## Whole-frame priority stack audit

An existing Firestaff stack output for this authentic pass is retained outside
Git at
`nexus_mapd_ymir_vdp_snapshots_20261031/pr239-8f3b5841-validation/frame891-display-stack.raw`.
The output hash is
`cc6c926860104797078645bb3fd784c78fc6ad0f768aa2556cfa4fecdc68cddc`; the
matching Ymir render has hash
`24cc53361cf3b6e0456bc9b48fddc362d6865ac491376fb203181f3108469f17`.
Comparing their RGB bytes finds exact equality on all 44,800 pixels where the
authentic per-pixel Ymir trace reports active NBG1 priority 5. The other 26,880
pixels have NBG1 priority 0 and dot 0, while the saved Firestaff NBG1-only
stack still marks every pixel as NBG1. The row-effective NBG1 plane can be
reproduced from the authenticated current/previous VRAM snapshots, and matches
the active NBG1 RGB at all 44,800 pixels. This is a single-layer result; it
does not establish whole-frame VDP1+VDP2 composition. The stack artifact was
generated at code head `8f3b5841` and is retained only as a negative audit.

Reproduce the pinned negative audit with:

```sh
evidence=/Volumes/Extern-disk/firestaff-evidence/nexus
python3 scripts/audit_nexus_mapd_frame891_priority_stack.py \
  --stack "$evidence/nexus_mapd_ymir_vdp_snapshots_20261031/pr239-8f3b5841-validation/frame891-display-stack.raw" \
  --reference "$evidence/nexus_mapd_ymir_vdp_snapshots_20261031/layer-trace-frame891-start60-20261101/vdp1-891-vdp2-1280-320x224.rgbx8888le" \
  --trace "$evidence/ymir-vdp2-pixel-fetch-trace-20261101/mapd-frame891-frame1280/nexus-mapd-frame891-vdp2-pixels.csv" \
  --receipt "$evidence/ymir-vdp2-pixel-fetch-trace-20261101/mapd-frame891-frame1280/nexus-mapd-frame891-vdp2-pixels.csv.receipt.json" \
  --row-source-plane "$evidence/nexus_mapd_ymir_vdp_snapshots_20261031/pr252-row-source-replay/firestaff-nbg1-row-source-composed-plane.rgba"
```

The report returns `KNOWN_CAPTURE_ONLY_MISMATCH` on success: success means the
hash-pinned limitation was reproduced, not that frame parity passed. It reads
priority, dot and coordinates from every target-pass trace row; it does not
encode a fixed vertical cutoff.

The complete Ymir candidate-layer trace now independently reconstructs this
pass at all 71,680 RGB pixels. The audit decodes the trace's low-to-high color
bytes, sorts active candidates by descending priority and ascending layer id,
and applies NBG0's per-pixel special color calculation using the captured
ratio. Its equation follows Ymir's software compositor:
`lower + (((upper - lower) * ratio) >> 5)`. The reconstructed output hash is
identical to Ymir's final pass hash. The audit now requires all 224 target-pass
render-state rows to report `nbg0_enabled=1`, `layer_enabled=1`, and
`rbg1_enabled=0` before it uses layer 2 as NBG0. This verifies a candidate-layer
reconstruction consistent with the row-local NBG0 state; it does not verify
the active-layer set for every Ymir layer or show that Firestaff can decode the
same candidates from raw VDP1/VDP2 snapshots.

The separately retained VDP2 register snapshot carrying the same frame/serial
label has big-endian `BGON=0x0002` at offset `0x20` (snapshot SHA-256
`774fa7f2e815b14770b83feb83e8c15406041a931e65ed465b1062944a811dd1`), while
the render-state rows say NBG0 was enabled before each row was drawn. The
snapshot is therefore not used as render-time NBG0 enable evidence. The
differing state has no row-time timestamp in the snapshot index. A cross-check
of all 163 render-state passes against their indexed VDP2 register snapshots
found nine NBG0-enable disagreements at MAPD transitions (891/1280, 901/1290,
931/1320, 938/1330, 960/1360, 970/1370, 999/1400, 1008/1409, and 1036/1440).
The pattern means the snapshots cannot serve as row-time enable evidence; it
does not establish which capture point is stale. A same-event register and
layer-enable trace remains necessary to resolve the discrepancy.

Reproduce the authentic Ymir layer-to-final comparison with:

```sh
evidence=/Volumes/Extern-disk/firestaff-evidence/nexus/nexus_mapd_ymir_vdp_snapshots_20261031/layer-trace-frame891-start60-20261101
python3 scripts/audit_nexus_ymir_mapd_layer_composition.py \
  --layers "$evidence/layers.csv" \
  --render-state "$evidence/render-state.csv" \
  --render-state-receipt "$evidence/render-state.csv.receipt.txt" \
  --reference "$evidence/vdp1-891-vdp2-1280-320x224.rgbx8888le" \
  --output-rgbx /Volumes/Extern-disk/firestaff-evidence/nexus/nexus_mapd_ymir_vdp_snapshots_20261031/pr252-row-source-replay/firestaff-independent-ymir-layer-reconstruction.rgbx
```

The pass uses Ymir layer id 0 for sprites, 2 for NBG0/RBG1, and 3 for NBG1.
There are 16,179 top NBG0 pixels with special color calculation. The pixel
ownership and composition order match the current [Ymir compositor source](https://github.com/ymir-emu/Ymir/blob/main/libs/ymir-core/res/shaders/src/vdp/cs_vdp2_compose.hlsl).

This result narrows the Firestaff gap to integrating verified candidate
decoders into raw snapshot composition: the effective NBG1 row-time fetch
source, VDP1 RGB/palette pixels, and their priority/color-calculation stack
must still be produced by Firestaff and compared against this authentic oracle.
The legacy stack remains a known mismatch and is not admitted for rendering.

The NBG0 raw bitmap source is now decoded by Firestaff's capture-only C path
and compared directly against the same authentic layer trace. For each row,
the decoder consumes Ymir's captured 11.8 fixed-point scroll state, reads an
8-bit index from VDP2 VRAM base `0x00000` with a 512-byte pitch, and resolves
the same index in CRAM. All 20,302 nonzero dots match Ymir's NBG0 candidate
color, priority 4, and special-color-calculation flag; all 51,378 index-zero
dots match Ymir transparency. The output plane hash is
`7a1e9053ab5cf14795828ad646c148cbd288a4b188599e8558b628c11255592a`. This
verifies the C raw byte-to-layer decoder for the captured MAPD profile. The
per-row state remains an explicit external capture input; extracting that
state from Firestaff's own runtime remains open.

Reproduce the raw NBG0 source join with:

```sh
evidence=/Volumes/Extern-disk/firestaff-evidence/nexus/nexus_mapd_ymir_vdp_snapshots_20261031
python3 scripts/audit_nexus_ymir_mapd_nbg0_bitmap_source.py \
  --vram "$evidence/vdp-snapshots/vdp1-891-vdp2-1280-vdp2-vram.bin" \
  --cram "$evidence/vdp-snapshots/vdp1-891-vdp2-1280-vdp2-cram.bin" \
  --registers "$evidence/vdp-snapshots/vdp1-891-vdp2-1280-vdp2-registers.bin" \
  --layers "$evidence/layer-trace-frame891-start60-20261101/layers.csv" \
  --render-state "$evidence/layer-trace-frame891-start60-20261101/render-state.csv"
```

Run Firestaff's C decoder and compare all output pixels against the authentic
Ymir layer candidate with:

```sh
FIRESTAFF_NEXUS_YMIR_MAPD_NBG0_EVIDENCE_DIR=/Volumes/Extern-disk/firestaff-evidence/nexus/nexus_mapd_ymir_vdp_snapshots_20261031 \
ctest --test-dir build -R '^nexus_v1_vdp2_mapd_nbg0_bitmap_source_real$' --output-on-failure
```

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
