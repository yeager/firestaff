# DM2 V1 Phase 8 — Save/Load Round-Trip Verification
**Pass:** H2312
**Date:** 2026-05-26
**Schema:** `firestaff.dm2_v1.save_load_round_trip.v1`

## Summary
- Total tests: 43
- Passed: 43  
- Failed: 0  
- Status: **PASS**

## Test Results

| # | Test | Status | Detail |
|---|------|--------|--------|
| T1 | T1_slot_header_size_is_42 | ✅ PASS | actual=42, expected=42 |
| T2 | T2_magic1_is_beef | ✅ PASS | magic1=0xBEEF, expected=0xBEEF |
| T3 | T2_magic2_is_dead | ✅ PASS | magic2=0xDEAD, expected=0xDEAD |
| T4 | T2_magic_valid | ✅ PASS | magic_valid=True |
| T5 | T3_version_flag_is_1 | ✅ PASS | version=1, expected=1 |
| T6 | T4_slot_number_encoded | ✅ PASS | decoded slot=0, expected=0 |
| T7 | T4_slot_plus30 | ✅ PASS | slot_plus30=0x30, expected=0x30 |
| T8 | T5_name_roundtrip_slot0 | ✅ PASS | name='Slot 0', expected='Slot 0' |
| T9 | T5_name_roundtrip_slot9 | ✅ PASS | name='Slot 9', expected='Slot 9' |
| T10 | T6_suppress_encode_roundtrip | ✅ PASS | decoded==expected: True, got=[1, 2, 3, 4, 5, 6, 7, 8] |
| T11 | T7_suppress_byte_0_match | ✅ PASS | byte[0]: 0x01 == 0x01 |
| T12 | T7_suppress_byte_1_match | ✅ PASS | byte[1]: 0x02 == 0x02 |
| T13 | T7_suppress_byte_2_match | ✅ PASS | byte[2]: 0x03 == 0x03 |
| T14 | T7_suppress_byte_3_match | ✅ PASS | byte[3]: 0x04 == 0x04 |
| T15 | T7_suppress_byte_4_match | ✅ PASS | byte[4]: 0x05 == 0x05 |
| T16 | T7_suppress_byte_5_match | ✅ PASS | byte[5]: 0x06 == 0x06 |
| T17 | T7_suppress_byte_6_match | ✅ PASS | byte[6]: 0x07 == 0x07 |
| T18 | T7_suppress_byte_7_match | ✅ PASS | byte[7]: 0x08 == 0x08 |
| T19 | T8_suppress_empty_mask_empty_output | ✅ PASS | empty_mask produces empty encoded and 0xFF fill |
| T20 | T9_suppress_nibble_8_is_skip | ✅ PASS | mask nibble 8 (value 0) skips: encoded=b'' |
| T21 | T10_suppress_nibble_encode_4bytes | ✅ PASS | 8 nibbles → 4 bytes, got 4 |
| T22 | T10_suppress_nibble_roundtrip | ✅ PASS | decoded lower nibbles: ['0x1', '0x2', '0x3', '0x4', '0x5', '0x6', '0x7', '0x8'] |
| T23 | T11_save_load_same_bytes | ✅ PASS | loaded 298 bytes == expected 298 |
| T24 | T11_loaded_header_magic_valid | ✅ PASS | magic_valid=True |
| T25 | T11_loaded_header_name_match | ✅ PASS | name='Skullkeep Level 1' == 'Skullkeep Level 1' |
| T26 | T11_loaded_header_slot_match | ✅ PASS | slot=3 == 3 |
| T27 | T11_loaded_body_matches | ✅ PASS | body matches: True |
| T28 | T11_backup_not_created_on_first_save | ✅ PASS | bak should not exist after first save: True |
| T29 | T12_identical_header_same_hash | ✅ PASS | SHA256(header1)=6a4ad1d2dfacdcd4..., SHA256(header2)=6a4ad1d2dfacdcd4... |
| T30 | T13_different_name_different_hash | ✅ PASS | Different names produce different hashes |
| T31 | T14_slot_0_decodes_correctly | ✅ PASS | slot 0: decoded=0 |
| T32 | T14_slot_1_decodes_correctly | ✅ PASS | slot 1: decoded=1 |
| T33 | T14_slot_2_decodes_correctly | ✅ PASS | slot 2: decoded=2 |
| T34 | T14_slot_3_decodes_correctly | ✅ PASS | slot 3: decoded=3 |
| T35 | T14_slot_4_decodes_correctly | ✅ PASS | slot 4: decoded=4 |
| T36 | T14_slot_5_decodes_correctly | ✅ PASS | slot 5: decoded=5 |
| T37 | T14_slot_6_decodes_correctly | ✅ PASS | slot 6: decoded=6 |
| T38 | T14_slot_7_decodes_correctly | ✅ PASS | slot 7: decoded=7 |
| T39 | T14_slot_8_decodes_correctly | ✅ PASS | slot 8: decoded=8 |
| T40 | T14_slot_9_decodes_correctly | ✅ PASS | slot 9: decoded=9 |
| T41 | T16_suppress_self_verification | ✅ PASS | self-verification round-trip: True |
| T42 | T17_suppress_fill_00_matches_original | ✅ PASS | fill=0x00 round-trip: True |
| T43 | T18_large_data_suppress_roundtrip | ✅ PASS | 256-byte large data SUPPRESS round-trip |

## Source Anchors
### `skull_save_load_entry`
- Source: SKULL.ASM
- File: `SKULL.ASM`  
- Lines: T4000-T4010
- Claim: SKULL.ASM T4000-T4010 are the save/load entry points for DM2 V1.
- Needles: `SAVE_GAME`, `LOAD_GAME`, `slot_number`, `SKSave`

### `dm2_save_header_format`
- Source: dm2_v1_save_load.c
- File: `src/dm2/dm2_v1_save_load.c`  
- Lines: 165-200
- Claim: dm2_v1_save_load.c defines the 42-byte slot header: version flag, name (33 chars), slot+0x30, magic 0xBEEF/0xDEAD.
- Needles: `sksave_header_asc`, `version_flag`, `DM2_SLOT_MAGIC_1`, `DM2_SLOT_MAGIC_2`

### `dm2_suppress_codec`
- Source: dm2_v1_save_load.c
- File: `src/dm2/dm2_v1_save_load.c`  
- Lines: 25-90
- Claim: SUPPRESS is a bit-plane RLE codec: mask low nibble 0→skip, 1..7→store that many LSBs of data[i]. LSB-first packing.
- Needles: `dm2_suppress_encode`, `dm2_suppress_decode`, `mask_nibble`, `bit_plane_RLE`

### `skull_slot_namespace`
- Source: SKULL.ASM
- File: `SKULL.ASM`  
- Lines: T4020-T4030
- Claim: SKULL.ASM T4020 defines the save slot namespace: SKSave00.dat through SKSave09.dat plus SKSave.bak.
- Needles: `SKSave`, `SKSave.dat`, `SKSave%02d.dat`, `slot_0`

### `dm2_slot_scan`
- Source: dm2_v1_save_load.c
- File: `src/dm2/dm2_v1_save_load.c`  
- Lines: 191-220
- Claim: dm2_sl_scan_slots identifies occupied slots by magic markers at offsets 38-41 of the 42-byte header.
- Needles: `dm2_sl_scan_slots`, `DM2_SLOT_MAGIC_1`, `DM2_SLOT_MAGIC_2`, `occupied`

### `dm2_suppress_self_verification`
- Source: dm2_v1_save_load.c
- File: `src/dm2/dm2_v1_save_load.c`  
- Lines: 109-125
- Claim: dm2_suppress_self_verification() tests encode→decode round-trip on a known vector.
- Needles: `dm2_suppress_self_verification`, `data[i]`, `mask[i]`, `encode_decode_round_trip`

