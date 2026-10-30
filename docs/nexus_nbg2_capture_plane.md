# Nexus NBG2 capture plane

The retained authentic Ymir gameplay capture at frame 239 has NBG2 enabled
with `BGON=0x0007`. The hash-pinned register, VRAM and CRAM snapshots select
an 8bpp NBG2 tilemap with two-word PNDs, a 64×64 name page, map A/B index
`0x18`, zero scroll and zero NBG2 CRAM offset. The observed name-page byte
base is `0x60000`.

`nexus_v1_vdp2_capture_decode_ymir_nbg2_tilemap_plane()` admits only that
exact register tuple. It fetches the visible 40×28 cells, applies captured
PND flips, uses Saturn's 32-byte character-number base and 8bpp pixel rows,
and returns an index plane plus CRAM colors. The result is marked
capture-only; the plane decoder does not assign it a named retail asset, host
presentation crop, or production renderer permission. Frame-239 layer priority
and color selection are verified by the separate full-stack capture check in
[`nexus_vdp2_frame239_composition.md`](nexus_vdp2_frame239_composition.md).

The hash-pinned real-data test decodes 1,120 tiles with 11,232 nonzero and
60,448 transparent pixels. Comparing each nontransparent native pixel's
CRAM color with the upper-left sample of the 2× Ymir screenshot gives 9,258
of 11,232 samples within eight channel levels. Other samples differ because
this per-layer plane omits cross-layer priority. The complete capture-only
stack instead matches all four samples for every native pixel exactly.

Set `FIRESTAFF_NEXUS_YMIR_NBG2_VRAM`, `FIRESTAFF_NEXUS_YMIR_NBG2_CRAM`, and
`FIRESTAFF_NEXUS_YMIR_NBG2_REGS` to the authentic frame-239 snapshots, and
optionally set `FIRESTAFF_NEXUS_YMIR_NBG2_CAPTURE_OUT` to write the decoded
plane as four bytes per pixel (`index,R,G,B`). Then run
`test_nexus_v1_vdp2_runtime_tilemap`. Compare the output against the retained
Ymir screenshot with:

```sh
python3 scripts/verify_nexus_bitmap_capture_plane.py \
  "$FIRESTAFF_NEXUS_YMIR_NBG2_CAPTURE_OUT" \
  "/path/to/DUNGEON MASTER NEXUS [T-9111G]-20261030T091953.253.png"
```

The same run also decodes the NBG0 8bpp bitmap at byte offset zero and the NBG1
tilemap at byte offset `0x5c000`. Set the NBG0 or NBG1 capture output
environment variables to retain those index/RGB planes, then run the verifier
with `--layer nbg0` or `--layer nbg1`; NBG0 has 3,954/3,968 matching visible
samples (maximum error 232), and NBG1 has 148/148 (maximum error 7). These
remain per-layer capture checks. Set
`FIRESTAFF_NEXUS_YMIR_NBG0_CAPTURE_OUT` for NBG0 or
`FIRESTAFF_NEXUS_YMIR_NBG1_CAPTURE_OUT` for NBG1.
