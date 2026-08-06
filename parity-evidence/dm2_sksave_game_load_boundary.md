# DM2 original SKSave — `GAME_LOAD` boundary

This records the exact boundary between the portions of a PC-DOS DM2 save
which Firestaff has verified from original bytes and the later portions that
still need a live source-owned runtime allocator. It follows the method used
by the `dm1_fmtowns*` evidence: establish a bounded container layout from the
original program, prove it against supplied retail media, and leave unknown
semantics unavailable. It does **not** import DM1's file layout, palette,
geometry, or menu behaviour into DM2.

## Source order

The owner is SKProject `SKULLWIN/c_savegame.cpp`:

1. `DM2_GAME_LOAD` accepts the original 42-byte `c_hex2a` save header and
   invokes `DM2_READ_DUNGEON_STRUCTURE` for the raw dungeon prefix.
2. Its one continuous MSB-first `DM2_SUPPRESS_READER` then reads
   `s_savegamebuffer` (0x3c), `v1e0104`, `globalb`, `globalw`, every 263-byte
   `c_hero`, `c_wbbb`, and every 12-byte `c_tim`.
3. Without flushing or realigning that reader, `DM2_READ_SKSAVE_DUNGEON`
   reads the 30 item roots for each party hero, the party-hand root, and then
   every map tile's record chain. It requires the live map, DB allocator,
   `DM2_APPEND_RECORD_TO`, possession-index and timer owners.

`DM2_READ_SKSAVE_DUNGEON` is therefore not an independent byte stream and
cannot be reconstructed from a convenient session structure. Its map walk
also depends on the loaded tile map and teleporter detail, so an inventory-only
decoder is not a resumable game state.

## Firestaff receipt boundary

`dm2_v1_original_raw_sksave_dungeon_receipt()` verifies the original raw
dungeon prefix: map descriptors and geometry, column indices, ground stacks,
text words, all sixteen source-sized DB pools, and saved map bytes. The
receipt deliberately preserves offsets and hashes rather than publishing
decoded object links.

`dm2_v1_original_raw_sksave_fixed_state_receipt()` continues the same
SUPPRESS reader through the complete fixed `GAME_LOAD` section. It carries
the precise byte/bit boundary at which `DM2_READ_SKSAVE_DUNGEON` starts and a
separate source hash for every present `c_hero`; it does not claim that a hero
inventory, portrait, stat projection, timer target, or tile chain is loaded.

The isolated `READ_RECORD_CHECKCODE` and map-walk helpers are test-only ABI
transcripts. They are intentionally excluded from the production M10 and DM2
archives because their callback arguments do not supply the original runtime
DB/map/possession owners. Making those callbacks a production loader would
replace the source graph with caller-authored data.

## Real-media regression

`tests/test_dm2_v1_save_load_real_data.c`, with
`FIRESTAFF_DM2_SKSAVE_CORPUS` set to the supplied PC-DOS `data` directory,
checks all four `sksaveN.dat` files and their four `.bak` companions. It
requires the mounted original `GRAPHICS.DAT` to resolve the original creature
AI mask selection before the direct roots are consumed.

The regression verifies all eight authenticated headers, raw-dungeon
prefixes, fixed-state receipts, per-hero identities, source-sized DB-pool
edges, and direct hero-item/party roots. It also proves that every candidate
leaves the live runtime unchanged: Resume remains blocked until the complete
`GAME_LOAD` allocation and publication chain exists. Missing real media
causes the test to skip; no save bytes are generated, unpacked, or written.

## Verified command

```
FIRESTAFF_DM2_SKSAVE_CORPUS='/Users/bosse/.firestaff/data/dm2/dos_extract/data' \
SDL_AUDIODRIVER=dummy \
ctest --test-dir build-dm2-main-verify \
  -R '^(dm2_v1_save_load_extra_dungeon_data|dm2_v1_save_load_real_data)$' \
  --output-on-failure
```

Result on 2026-08-06: both tests passed. The test-only map reader's empty
round-trip is an ABI check only; it is not accepted as source data and does
not grant runtime Resume permission.
