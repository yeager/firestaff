# DM1 V1 — Asset Loading (GRAPHICS.DAT / DUNGEON.DAT)

## Source-locked
ReDMCSB: MEMORY.C (lines 1230+, 1484), DUNVIEW.C, GRF1.C, GRF1BASE.C
Firestaff: src/engine/firestaff_graphics_dat_reader.c, src/dm1/dm1_v1_graphics_loader_pc34_compat.c

## GRAPHICS.DAT Loading (MEMORY.C:1230–1283)

Initialization sequence (FIRESTART/boot):
1. Open GRAPHICS.DAT (try 8 paths in order until one succeeds)
2. Read GRAPHICS_DAT_HEADER from file (error 41 if cannot open)
3. Read header into memory (error 42 if cannot read header)
4. Pre-allocate memory for all graphics based on header bitmap count

Header format includes: bitmap count, width/height per bitmap, compressed byte counts.
G0630_i_GraphicsDatFileHandle — file handle stored globally.

## Lazy Loading Strategy (DUNVIEW.C:2045–2941)

DM1 does NOT preload all graphics at startup. Instead:

**Demand loading per viewport render:**
F0490_MEMORY_LoadDecompressAndExpandGraphic() called:
- Per tile: floor graphic (index), ceiling graphic (index+1)
- Per creature: creature sprite graphics
- Per object: object sprite graphics
- Per wall: wall set graphics

**Conditional preload for large heaps (DUNVIEW.C:2897):**
If G0661_B_LargeHeapMemory is true AND no memory limit:
- Load ALL graphics at dungeon load time
Else:
- Load only mandatory graphics (G0018_ai_Graphic562_MandatoryGraphicIndices)
- Load additional graphics on demand as viewport renders

**Champion portrait loading issue (DUNVIEW.C:2463–2464):**
BUG0_86 — Champion portraits may be missing/garbage on maps with creature types
allowed on the same map as champion mirrors. Only occurs when heap is not large
and G0661_B_LargeHeapMemory is false.

## DUNGEON.DAT Loading (LOADSAVE.C:2192+, DUNGEON.C)

Loaded when player starts or resumes a game:
1. Open DUNGEON.DAT (try 7 path variants)
2. Read DUNGEON_HEADER — contains map count, text data size, thing counts
3. Read MAP[] array — per-map metadata
4. Read column square thing counts
5. Read square first-thing indices
6. Read dungeon text data
7. Read thing data by type (0–11, each separate array)
8. Read raw map data (compressed)
9. Decompress if compressed (G0530_B_LoadingCompressedDungeon flag)

## Firestaff Implementation
firestaff_graphics_dat_reader.c — reads GRAPHICS.DAT headers, manages bitmap cache
dm1_v1_graphics_loader_pc34_compat.c — PC-34 pixel format adaptation
firestaff_asset_pipeline.c — asset pipeline and extraction
