# Theron JP Roster to Startup Party (2026-08-10)

## Result

The hash-verified JP Track 02 roster cluster can now be used during the
startup forcefield handoff. `theron_v1_party_refresh_jp_source_records()`
reads the eight real records through `theron_v1_track02_jp_roster_read()` and
updates only the already selected party state when every selected champion
matches a source record.

The following fields are bound from the bytes in `TQJP02.bin`:

- name
- HP, stamina, and mana
- the seven attribute bytes
- the 16 skill bytes and highest skill per class
- primary class derived from those source skills
- startup names and titles are copied from verified raw offsets after byte
  matching; they are not passed through host constants for search strings

The JP records are located at the verified raw roster cluster `0xB3D98` and
may only be used with JP BIN MD5 `b7afb338ad31be1025b53f9aff12d73a`. The first
Theron record is locally verified as HP `175`, stamina `1500`, and mana `35`.

## Boundary

This is a source-bound roster and text-payload binding, not evidence for the
original portrait, palette, font/VDC-text, or T900 consumer. `portrait_index`
therefore remains `THERON_PORTRAIT_UNAVAILABLE`. Equipment, inventory,
use/equip/stack, RNG, AI, T700, and sound are not enabled by this receipt.

## Verification

```text
./build/test_theron_v1_track02_champion_roster                 PASS
./build/test_theron_v1_startup_media_palette_bind              PASS
FIRESTAFF_THERON_DATA=~/.firestaff/data/theron \
  ./build/test_theron_v1_track02_dungeon_loader                 PASS
./build/test_theron_v1_combat_mechanics                         PASS (121/121)
cmake --build build --target firestaff --parallel 1             PASS
```

No BIN, BIOS, or capture files are added to the repository.
