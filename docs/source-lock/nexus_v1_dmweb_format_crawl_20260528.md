# Nexus V1 DMWeb format crawl, 2026-05-28

Source: http://dmweb.free.fr/community/documentation/dungeon-master-nexus/dgn-files/

DMWeb contradicts Firestaff's early Nexus assumption that `LEV*.DGN` starts with a raw 32x32 square grid at offset 0. The live source-lock for DGN parsing is now the DMWeb block-container description.

## DGN container contract

- `LEV00.DGN` is used for the entrance/title sequence and is not a playable dungeon level.
- `LEV01.DGN` through `LEV15.DGN` are the playable dungeon levels. `LEV01.DGN` is the Hall of Champions.
- DGN files are organized in `0x800` byte blocks.
- The first block is a header. It points to Structure1, Structure2 and Structure3 using block offsets and block counts.
- Header fields used by Firestaff:
  - `0x0C`: Structure1 offset in `0x800` byte blocks.
  - `0x0E`: Structure1 size in `0x800` byte blocks.
  - `0x10`: useful Structure1 size in bytes.

## Structure1B grid

Structure1 contains the dungeon grid as Structure1B.

- Structure1 offset `0x14` gives the relative offset of Structure1B.
- Structure1B is always `0x8000` bytes.
- The grid is 64x64 cells.
- Each cell is 8 bytes.
- The first word contains floor, ceiling and door-presence flags.
- Bytes 5, 6 and 7 pack two 12-bit values. Firestaff currently uses the low 12-bit collision value:
  - `0x000`: open corridor/floor.
  - `0xFFF`: cannot enter/wall.
  - other values: collision descriptor index in Structure1C.

## Implementation impact

Firestaff must not treat real Nexus DGN files as raw 32x32 data at offset 0. `nexus_v1_level_load()` first parses the DMWeb block header and Structure1B. The old 32x32 reader remains only as a legacy synthetic-fixture fallback while old probes and one-off tests are migrated.

Do not describe `geometry_offset = 2048` as a real Nexus layout rule. After Structure1B, the remaining data is still being separated into Structure1C through Structure1F and render payloads.
