# DM1 V1 Mod Support — Source Audit

## ReDMCSB Source Evidence

### No Plugin/Mod Loader Architecture
ReDMCSB Common/Source contains no plugin, mod-loader, or user-content discovery system.
- No file-extension or path scanning for user-content directories
- No INI/config parsing for mod settings
- No content-merging logic in the dungeon load path (LOADSAVE.C:1803–2070)

### Dungeon ID Hardcoding (DEFS.H:521–523)
  #define C10_DUNGEON_DM         10
  #define C12_DUNGEON_CSB_PRISON 12
  #define C13_DUNGEON_CSB_GAME   13
DM1 V1 recognizes only dungeon ID 10 as valid (CEDTINCD.C:117–124, F7272_IsDungeonValid).
Custom dungeon builds require recompilation with modified DEFS.H constants.

### DUNGEON.DAT Loading is Flat File Swap (LOADSAVE.C:2337–2373)
Loader tries 8 hardcoded paths in priority order:
  \DUNGEON.DAT
  \JDATA\DUNGEON.DAT
  Q:\DATA\DUNGEON.DAT
  Q:\JDATA\DUNGEON.DAT
  \CDATA\DUNGEON.DAT
  \CJDATA\DUNGEON.DAT
  (+ Amiga/ST equivalents)
No manifest, no subdirectory scanning, no version detection.

### DUNGEON_HEADER DungeonID Check (LOADSAVE.C:2826)
  if (!F0434_STARTEND_IsLoadDungeonSuccessful_CPSC())
Loader fails silently if DUNGEON.DAT is absent or corrupt.

### FTL Compression Support (DEFS.H:975–984)
Signature 0x8104 triggers FTL decompression on load.
COMPRESSED_DUNGEON_HEADER: Signature(2) + DecompressedByteCount(4, BE) + DungeonID(2)
Custom dungeons could theoretically be hand-crafted with this format.

## Conclusion
DM1 has **no mod support**. The only modding path is full DUNGEON.DAT binary replacement
with a file passing the F0434 load sequence. The format is documented in ReDMCSB, so a
custom dungeon could be hand-crafted, but no editor or user-facing tools exist.

## Firestaff Implementation
Firestaff replicates the flat file-swap: ~/.firestaff/data/dm1/DUNGEON.DAT (single file).
No mod loader, no content discovery beyond the single DUNGEON.DAT swap.

## References
- ReDMCSB/DEFS.H:521–523 — DungeonID constants
- ReDMCSB/LOADSAVE.C:1803–2070 — Full dungeon load sequence
- ReDMCSB/LOADSAVE.C:2337–2373 — Hardcoded DUNGEON.DAT paths
- ReDMCSB/CEDTINCD.C:117–124 — F7272_IsDungeonValid (DungeonID=10 gate)
- ReDMCSB/DEFS.H:975–984 — FTL compressed dungeon format

STATUS: DOCUMENTED — No mod support exists in DM1 V1 source or Firestaff.
