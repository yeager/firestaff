# Nexus NBG0 capture plane

The same hash-pinned frame-239 Ymir capture has NBG0 enabled in 8bpp bitmap
mode, with the bitmap at VDP2 VRAM byte offset zero, zero scroll, unity
increments, palette bank zero, and priority four. The bounded raw snapshot
decoder returns its 320×224 index and CRAM-color planes as capture-only data.

The authentic capture produces 3,968 nonzero pixels and 67,712 transparent
pixels. Comparing CRAM-derived colors against the upper-left sample in the
hash-pinned 2× Ymir screenshot places 3,954 of 3,968 nonzero pixels within
eight channel levels. This layer check does not establish cross-layer order,
color calculation, retail bitmap ownership or production renderer
permission.

The real-data test uses the same three frame-239 snapshots documented in
[`nexus_nbg2_capture_plane.md`](nexus_nbg2_capture_plane.md). Set
`FIRESTAFF_NEXUS_YMIR_NBG0_CAPTURE_OUT` to retain the result in
`index,R,G,B` byte order, then verify it with:

```sh
python3 scripts/verify_nexus_bitmap_capture_plane.py \
  "$FIRESTAFF_NEXUS_YMIR_NBG0_CAPTURE_OUT" \
  "/path/to/DUNGEON MASTER NEXUS [T-9111G]-20261030T091953.253.png" \
  --layer nbg0
```
