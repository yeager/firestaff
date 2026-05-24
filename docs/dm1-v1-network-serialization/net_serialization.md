# DM1 V1 — Save File Serialization

## Source-locked
ReDMCSB: SAVEHEAD.C (lines 16–109), LOADSAVE.C (line 2192+), DEFS.H (lines 469–501)
Firestaff: src/dm1/dm1_v1_save_load.c, src/dm1/dm1_v1_save_load_system_pc34_compat.c

## Save File Binary Format
A DM1 save file is a 512-byte header + variable-length data sections.

### Header Layout — DM_SAVE_HEADER (512 bytes, DEFS.H:469)
Offset 0x000: Noise[149] (298 bytes) — random data; word[10] = decryption key
Offset 0x12A: Useless (1 byte, always 1 — dead code)
Offset 0x12B: FormatID (1 byte) — 1=DM Atari ST, 2=Amiga/PC98/X68k/CSB, 3=Apple IIGS, 5=Amiga36/PC
Offset 0x12C: aUnreferenced (4 bytes) — padding, never used
Offset 0x130: SaveAndPlayChoice (1 byte) — 0=Save+Quit, 1=Save+Play
Offset 0x134: GameID (4 bytes) — random unique per game; blocks reload after party death
Offset 0x138: Keys[16] (32 bytes) — per-section encryption keys
Offset 0x158: Checksums[16] (32 bytes) — per-section CRC/checksum
Offset 0x178: Platform (2 bytes)
Offset 0x17A: DungeonID (2 bytes)
Offset 0x17C: AdditionalData (134 bytes) — preserved, never used by engine

CSB variant (CSB_SAVE_HEADER, DEFS.H:483): Noise[150], AdditionalData[132]. Key index = 29.

## Game State Serialization (LOADSAVE.C:2192+)
F0435_STARTEND_LoadGame deserializes in order:
1. 512-byte header — validated via F0429_STARTEND_IsReadSaveHeaderSuccessful
2. Global state — GameTime, party position, dungeon level
3. Champion portraits — raw bitmap data via F0416_SAVEUTIL_IsWriteBytesSuccessful
4. Dungeon header (DUNGEON_HEADER struct)
5. Map array (MAP[]) — count from header
6. Column square thing counts (int16_t[])
7. Square first-thing indices (int16_t[])
8. Dungeon text data (int16_t[])
9. Thing data arrays (by type 0–11) — thing_count * bytes_per_thing
10. Raw map data — compressed dungeon layout
11. Dungeon checksum (int16_t)

## Header Generation on Write (SAVEHEAD.C:68–107)
F0430_STARTEND_IsWriteObfuscatedSaveHeaderSuccessful:
- Checksum = sum of all int16_t in words 128–255
- Words 0–127 filled with M006_RANDOM(65536) pseudo-random data
- Word 127 = LastWord XOR Checksum — integrity sentinel
- Words 128–255 obfuscated via F0417_SAVEUTIL_GetChecksumAndObfuscate
- Platform-specific key: C10_DM_SAVE_HEADER_DECRYPTION_KEY_INDEX (10) for DM, C29_CSB_SAVE_HEADER_DECRYPTION_KEY_INDEX (29) for CSB

## Header Validation on Read (SAVEHEAD.C:16–53)
F0429_STARTEND_IsReadSaveHeaderSuccessful:
- Compute expected checksum over words 0–127 (32 iterations of sum+=word; sum^=word; sum-=word; sum^=word)
- Deobfuscate words 128–255 with F0417_SAVEUTIL_GetChecksumAndObfuscate
- Compute actual checksum over words 128–255; return true only if match

## Firestaff Implementation
src/dm1/dm1_v1_save_load.c — cross-platform reimplementation
src/dm1/dm1_v1_save_load_system_pc34_compat.c — PC-34 compatibility shim
Key functions: DM1_SaveLoad_SerializeGameState(), DM1_SaveLoad_DeserializeGameState()
