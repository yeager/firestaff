# Nexus NBG1 capture plane

The retained authentic Ymir gameplay capture at frame 239 has NBG1 enabled
with `BGON=0x0007`. Hash-pinned register, VRAM and CRAM snapshots select the
8bpp, two-word-PND NBG1 tilemap. Both map registers are `0x17`, the name page
starts at byte offset `0x5c000`, and the NBG1 scroll registers are zero. The
visible page contains 40×28 cells from a 64×64 map.

`nexus_v1_vdp2_capture_decode_ymir_nbg1_tilemap_plane()` accepts only that
register tuple. It applies PND flips, Saturn's 32-byte character-number base,
8bpp pixel rows, and the captured CRAM palette. The plane output is marked
capture-only; it does not claim a retail asset owner, presentation crop, or
renderer permission. Full-stack layer priority and color selection are
separately verified for frame 239 in
[`nexus_vdp2_frame239_composition.md`](nexus_vdp2_frame239_composition.md).

The real-data test decodes 1,120 cells, with 148 nonzero and 71,532
transparent pixels. All 148 nontransparent pixel colors match the upper-left
sample of the authentic 2× Ymir screenshot within eight RGB channel levels.
The measured maximum RGB channel error is seven. This verifies the captured
NBG1 fetch plane. The separately documented frame-239 stack comparison checks
all four screenshot samples per native pixel and matches exactly.

Set `FIRESTAFF_NEXUS_YMIR_NBG2_VRAM`, `FIRESTAFF_NEXUS_YMIR_NBG2_CRAM`, and
`FIRESTAFF_NEXUS_YMIR_NBG2_REGS` to the hash-pinned frame-239 snapshots, and
optionally set `FIRESTAFF_NEXUS_YMIR_NBG1_CAPTURE_OUT` to write the NBG1 plane
as `index,R,G,B` bytes. Run `test_nexus_v1_vdp2_runtime_tilemap`, then compare
the plane with the retained Ymir screenshot:

```sh
python3 scripts/verify_nexus_bitmap_capture_plane.py \
  "$FIRESTAFF_NEXUS_YMIR_NBG1_CAPTURE_OUT" \
  "/path/to/DUNGEON MASTER NEXUS [T-9111G]-20261030T091953.253.png" \
  --layer nbg1
```
