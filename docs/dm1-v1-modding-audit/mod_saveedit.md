# DM1 V1 Save File Editing — Source Audit

## Save File Format Overview

### Two Header Formats (DEFS.H:469–494)
DM1 V1 uses two save header formats, auto-detected by F7061_F0429_STARTEND_IsReadSaveHeaderSuccessful:

1. DM_SAVE_HEADER (512 bytes, Dungeon Master primary):
   - 150 uint16_t Noise[] — decryption key (index C10_DM_SAVE_HEADER_DECRYPTION_KEY_INDEX=10)
   - uint16_t DungeonID — dungeon identifier
   - uint16_t Checksums[5], uint16_t Keys[5] — per-section obfuscation

2. CSB_SAVE_HEADER (512 bytes, Chaos Strikes Back):
   - 150 uint16_t Noise[] — decryption key (index C29_CSB_SAVE_HEADER_DECRYPTION_KEY_INDEX=29)
   - uint16_t DungeonID
   - uint16_t Checksums[5], uint16_t Keys[5]

Header is obfuscated with a rolling noise/key: F7061 validates the decrypted header
(DungeonID=10 for DM, DungeonID=12/13 for CSB).

### Save File Sections (CEDTINC8.C, CEDTINCD.C)
5 sections written with F7058_WriteSavePartWithChecksum:
1. GLOBAL_DATA (sizeof(GLOBAL_DATA))
2. ACTIVE_GROUP[GlobalData.MaximumActiveGroupCount * 16]
3. PARTY_INFO + CHAMPION data (ChampionDataByteCount, variable size)
4. EVENT[GlobalData.EventMaximumCount * 10]
5. int16_t TimeLine[GlobalData.EventMaximumCount]

Plus portrait data (if PortraitCount > 0).

### Checksum Obfuscation (F7056_F0418_SAVEUTIL_GetChecksum)
Each section is obfuscated with a per-session random key:
  Keys[i] = M006_RANDOM(65536) per section
  Checksum = GetChecksum(sectionData, Keys[i], wordCount)
On load, checksum mismatch causes error exit with code 23004 (CEDTINC8.C).

### Editing Difficulty: HIGH
Save files are not straightforward to edit because:
1. Checksum validation — each section must have matching Checksums[i]
2. Rolling decryption — Noise array at specific index is used as decryption key
3. Variable-size champion data — ChampionDataByteCount varies with champion count and CSB additions
4. GLOBAL_DATA contains pointers — some offsets embedded as pointers, not raw indices

### What Would Be Needed to Edit a Save
1. Read header, determine key index (C10 for DM, C29 for CSB)
2. Read Noise[keyIndex] to get decryption key
3. Decrypt header
4. For each section: compute Checksum(sectionData, Keys[i], byteCount/2), write Checksum + section
5. Write portrait data last

## Firestaff Save Architecture
Firestaff implements its own save system using the same structural layout:
- GAME struct mirrors ReDMCSB GAME (DEFS.H:4449–4483)
- Save/load uses Firestaff own serialization (sessions handling)
- No direct binary save editing support

## References
- ReDMCSB/DEFS.H:469–494 — DM_SAVE_HEADER and CSB_SAVE_HEADER structs
- ReDMCSB/CEDTINC8.C — F7052_SaveGame, checksum computation
- ReDMCSB/CEDTINCD.C — F7051_LoadGame, header decryption
- ReDMCSB/DEFS.H:500–501 — Decryption key index constants
- ReDMCSB/DEFS.H:4449–4483 — GAME struct (in-memory save game)

STATUS: DOCUMENTED — Save files use checksum-obfuscated binary format. Editing requires
full header/key/checksum duplication. Not practical for casual save editing.
