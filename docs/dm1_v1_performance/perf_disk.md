# DM1 V1 — Disk I/O

## ReDMCSB source

**Disk access points**

1. **Initial dungeon load — DUNGEON.DAT + GRAPHICS.DAT**
   - At boot or new-game start, the entire DUNGEON.DAT (all 30 maps) is loaded into `G0279_pppuc_DungeonMapData` and `G0277_ps_DungeonMaps` via `F0020_*` map-load functions.
   - GRAPHICS.DAT is opened as a random-access file handle (`G0630_i_GraphicsDatFileHandle` in `MEMORY.C:30`). Graphics are decompressed into the cache on demand, not pre-loaded entirely.

2. **On-demand graphic loading** (`MEMORY.C:1748-2167`)
   - `F0133_VIDEO_BlitBoxFilledWithMaskedBitmap` triggers loading of missing graphics from GRAPHICS.DAT.
   - A three-tier cache system (`G0641_puc_CacheMemoryBottom`, `G0642_puc_GraphicMemoryBottom`, `G0649_puc_GraphicMemoryTop`) handles allocation.
   - If a needed graphic is not in the cache, `F0139_GRAPHICS_LoadGraphicFromDat_CPSD` reads the compressed data from GRAPHICS.DAT using the file handle and decompresses it.
   - Cache eviction removes unused graphics to make room.

3. **Save game reads/writes** (`LOADSAVE.C:809-1920`)
   - `F0774_FILE_Seek` / `Fseek` for positioned writes
   - Uses standard C file I/O (`fread`, `fwrite`) or DOS file handles depending on platform (`MEDIA009` vs. `MEDIA413`)
   - Copy protection sector reads use direct DMA sector commands on ST (`LOADSAVE.C:949-952`)

4. **Copy protection check** (`LOADSAVE.C:949-952`)
   - Direct DMA floppy sector read on ST, programmed via `0xFFFF8604` DMA registers
   - Read of sector 7 as anti-piracy measure

**Streaming vs. pre-load**
DM1 uses a **demand-loaded, cache-backed** approach:
- Dungeon map data is fully loaded at game start (all maps resident)
- Graphics are loaded from GRAPHICS.DAT on demand with LRU-style cache eviction
- No streaming of dungeon data during play
- Save game files are read/written in one atomic operation

**Floppy DMA buffer**
- `MEMORY.C:43`: `G0643_puc_FloppyDiskReadBuffer_CPSDF` — single DMA-sector buffer (512 bytes)
- Used for copy protection sector reads and save-game sector operations
- All floppy I/O goes through this single buffer, not scatter-gather

**Loading performance notes**
- GRAPHICS.DAT is a random-access file (indexed by compressed size table `G0634_pui_GraphicCompressedByteCount`)
- Each graphic load requires: file seek → read compressed bytes → decompress → copy to cache
- Cache misses are noticeable during first viewport render of a new area

## Firestaff coverage
- `dm1_v1_graphics_load.c` — implements demand loading with cache from GRAPHICS.DAT on original data
- `dm1_v1_save.c` — save/load via Firestaff's own format, not original floppy format
- Original DM1 disk I/O (floppy DMA sector reads) is not emulated in Firestaff

## Status
✅ SOURCE-LOCKED — disk I/O points documented with file:line citations.
