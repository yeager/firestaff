# Nexus NBG1 8bpp character addressing

The generic source-bound NBG1 tilemap compositor now uses Saturn's character
number unit for both 4bpp and 8bpp: the tile byte base is
`(charno << 5) & 0x7ffff`. An 8bpp character occupies 64 pixel bytes, but its
PND character number still advances in 32-byte units. The 8bpp tile can
therefore overlap the address range selected by the next character number.
Full 512 KiB VDP2 VRAM inputs wrap pixel reads at the Saturn VRAM boundary.

## Authentic gameplay evidence

The real-data regression uses Ymir's Japanese retail gameplay capture from
frame 239. Its pinned VDP2 register, VRAM, and CRAM hashes are:

- Registers: `308b3c23bb1db102d3833801a836f0141540fe0870069abb37d2041e57f9b555`
- VRAM: `148eebc95b80465a278b7f6d9c1025ee378ec6d288563de3e52ba8af986d94cc`
- CRAM: `f97f5ebf7bcfab263514895097ec40e6c5dca6dd91fe878f1691e9f204d8515d`

The captured state has `BGON=0x0007`, `CHCTLA=0x1013`, `PLSZ=0`,
`MPOFN=0`, `MAP1A/B=0x1717`, and zero NBG1 scroll. The active NBG1 8bpp map
starts at VDP2 VRAM byte offset `0x5c000`. Its visible map uses character
numbers `0x1100`, `0x1110`, `0x112c`, and `0x1146`. On the top-left 320x200
area, the corrected fetch produces 148 nonzero index-3 pixels and 63,852
transparent index-zero pixels. A 64-byte character-number stride incorrectly
made all 64,000 pixels opaque and read unrelated bytes.

The test consumes the authentic memory snapshots directly and checks the
hashes before setting capture provenance. It compares the resulting tilemap
indices and pixel counts; it does not claim that the complete VDP2 display,
layer priority, HUD, or gameplay screen is at parity.

Set the test input to the `dumps` directory of the retained authentic
`gameplay-mouse-control` Ymir capture, then run:

```sh
cmake --build build --target test_nexus_v1_vdp2_tilemap_capture_compositor -j1
FIRESTAFF_NEXUS_VDP2_REAL_DUMPS=/path/to/gameplay-mouse-control/dumps \
  ctest --test-dir build -R '^nexus_v1_vdp2_tilemap_capture_compositor$' \
  --output-on-failure
```

The game capture itself stays outside Git. Its manifest is
`parity-evidence/verification/nexus_v1_ymir_scheduled_shuttle_mouse_20261030/SHA256SUMS.txt`.
