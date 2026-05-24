# DM1 V1 — Memory Usage

## ReDMCSB source

**Memory layout (Amiga/Atari ST)**
- `MEMORY.C:1-65`: Defines separate chip/fast memory zones and cache vs. graphics vs. heap regions.
- `MEMORY.C:7-20` (A20ED/A20E/A20F/A20G/A21E/A22E/A22G):  
  `G1018_l_AvailableChipMemoryByteCount`, `G1019_puc_AvailableGEMMemoryBottom`, `G1020_l_AvailableGEMMemoryByteCount` — tracks chip and GEM memory separately.

**Graphics memory architecture**
- `MEMORY.C:23-43`:
  - `G0633_l_GraphicsDatFileSize` — total compressed GRAPHICS.DAT size
  - `G0634_pui_GraphicCompressedByteCount[]` — per-graphic compressed sizes
  - `G0635_pui_GraphicDecompressedByteCount[]` — per-graphic decompressed sizes
  - `G0636_ppuc_Graphics[]` — loaded graphic bitmap pointers
  - `G0637_pui_NativeBitmapBlockIndices[]`, `G0638_pui_DerivedBitmapBlockIndices[]` — bitmap derivation tables

**Cache memory allocation**
- `MEMORY.C:1675-1683`: Three-tier fallback for cache allocation — cache bottom vs. graphic bottom, then graphic top vs. cache top.
- `MEMORY.C:1748`: Defragmentation loop removes unused graphics from cache when pressure is high.
- `MEMORY.C:1926`: `G0348_Bitmap_Screen[8000]` stores available cache memory after loading graphics (written but never read — BUG0_00).

**Floppy read buffer**
- `MEMORY.C:41-44`: `G0643_puc_FloppyDiskReadBuffer_CPSDF` — single 512-byte (or larger) sector buffer for DMA floppy reads.

**Dungeon map data**
- `DUNGEON.C:1010-1014`:  
  `G0277_ps_DungeonMaps[]` — map header structs  
  `G0279_pppuc_DungeonMapData[][][]` — per-map square data (char per square element + height + creature type)  
  `G0280_pui_DungeonColumnsCumulativeSquareFirstThingCount[]` — per-column thing index tables

**RAM footprint estimate (Atari ST, 512KB base + extended)**
| Region | Approximate size |
|---|---|
| Screen bitmap (320×200×2 planes) | 32 000 bytes |
| Viewport offscreen bitmap (G0296) | ~32 000 bytes |
| Graphics cache (compressed + decompressed windows) | 100–200 KB typical |
| Dungeon map data (all 30 maps, ~50×50 max) | ~50–80 KB |
| Party/champion state | ~4 KB |
| Command/timeline/event queues | ~4 KB |
| Sound buffers (4 channels × 2) | ~16 KB |
| Floppy DMA buffer | 512 bytes |
| **Estimated working set** | **~250–400 KB** |

**Memory fragmentation handling**
- `MEMORY.C:2057-2113`: Eviction loop removes graphics not in the current load list, from highest index downward, until enough contiguous cache space is freed.

**Amiga chip vs. fast RAM**
- `MEMORY.C:7-20`: Tracks chip memory separately from fast/GEM memory. Graphics that need DMA access must live in chip RAM on Amiga.

## Firestaff coverage
- `m11_memory.c` / `m11_alloc` — mirrors the three-tier cache allocation fallback
- `dm1_v1_graphics_load.c` — maps `G0634/G0635` compressed/decompressed size tables to Firestaff `GraphicAsset` loading
- No runtime memory probe was built for this audit; the source documentation is the lock.

## Status
✅ SOURCE-LOCKED — memory architecture documented with file:line citations.
