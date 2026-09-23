# Theron Track 02 item properties: US/JP source binding

Each dungeon bank contains a real 66-entry property table of six bytes per
item slot, not a host-authored stat table. The table follows that dungeon's
variable-length type-code and name spans. The authenticated name counts for
dungeons 1 through 7 are 80, 65, 69, 69, 67, 63 and 66 in both editions.
Treating every name span as 66 entries consumed property-table zero bytes as
fabricated empty names and is no longer accepted.

The US property-table user-data offsets are `0x099825`, `0x0D9DC5`,
`0x11A4D4`, `0x159D1D`, `0x19A64E`, `0x1D999F` and `0x21A32D`. Their
396-byte FNV-1a is `b97787ef`. The JP offsets are `0x0990A2`, `0x0D9616`,
`0x119D4D`, `0x15955D`, `0x199EB1`, `0x1D91D9` and `0x219B13`. JP Drator
is a distinct real table with FNV-1a `6c4d1386`; the other six JP tables use
`b97787ef`.

The source match is byte-for-byte over the exact type-code span, exact name
span and all 396 property bytes for the owning dungeon before
`theron_v1_track02_dungeon_loader.c` copies a row into a ground-object or
carried-object provenance record. Runtime lookup uses the object's source
dungeon and item type; it does not search a global first match. A match proves
the payload identity only. It does not by itself prove the meaning of the six
fields, equipment formulas, item use, stacking, save ownership or the T900
consumer; those remain gated until the original consumer is joined.

Evidence:

- US Track 02 BIN MD5 `f23601102138f87c33025877767ebf76`.
- JP Track 02 BIN MD5 `b7afb338ad31be1025b53f9aff12d73a`.
- `include/theron_v1_track02_item_name_source.h` and
  `src/theron/theron_v1_track02_item_name_source.c` authenticate and retain
  all seven regional name/type/property banks.
- `tests/test_theron_v1_track02_dungeon_loader.c` verifies all seven real US
  and JP dungeons.
- Production does not link the historical checked-in 66-row catalog. Track 19
  validates its complete real 396-byte table in place with FNV-1a `b97787ef`;
  the probe mutates a media byte and requires rejection.
