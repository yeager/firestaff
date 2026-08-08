# Theron Track 02 item properties: US/JP source binding

The property rows are a real 66-entry table of six bytes per item, not a
host-authored stat table. The existing row catalog is sourced from the US
Track 02 BIN and is accepted only after the complete 396-byte sequence is
found in the selected normalized Track 02 image.

The US table is present at user-data offset `0x099825` (raw BIN offset
`0x0B04C5`). The JP image uses a different bank layout. Its first verified
copy is at user-data offset `0x0990A2` (raw BIN offset `0x0AFC12`); additional
identical bank copies occur at user-data offsets `0x119D4D`, `0x15955D`,
`0x1D91D9` and `0x219B13`.

The source match is byte-for-byte over all 396 bytes and is checked separately
for US and JP before `theron_v1_track02_dungeon_loader.c` copies a row into a
ground-object or carried-object provenance record. A matching table proves
the payload identity only. It does not by itself prove the meaning of the six
fields, equipment formulas, item use, stacking, save ownership or the T900
consumer; those remain gated until the original consumer is joined.

Evidence:

- US Track 02 BIN MD5 `f23601102138f87c33025877767ebf76`.
- JP Track 02 BIN MD5 `b7afb338ad31be1025b53f9aff12d73a`.
- `include/theron_v1_track02_item_properties.h` and
  `src/theron/theron_v1_track02_item_properties.c` perform the complete
  source-image match.
- `tests/test_theron_v1_track02_dungeon_loader.c` verifies all seven real US
  and JP dungeons.
