# Nexus V1 Data File Formats Audit — Source-Locked

## Sources
- `src/nexus/nexus_v1_dungeon.c`, `nexus_v1_dmdf_model.c`, `nexus_v1_iso_reader.c`
- `include/nexus_v1_engine.h`, `nexus_v1_dmdf_model.h`
- `docs/nexus_overview.md`, `docs/nexus_dungeon.md`, `docs/nexus_squares.md`
- `docs/nexus_graphics.md`
- Saturn CD image structure

---

## 1. Level Files: LEV00.DGN – LEV15.DGN

### File Overview
| Level | File | Size (bytes) | Grid | Notes |
|-------|------|-------------|------|-------|
| 0 | LEV00.DGN | 147,456 | 32×32 | Entry/temple level |
| 1 | LEV01.DGN | 280,576 | 32×32 | |
| 2 | LEV02.DGN | 272,384 | 32×32 | |
| 3 | LEV03.DGN | 290,816 | 32×32 | |
| 4 | LEV04.DGN | 245,760 | 32×32 | |
| 5 | LEV05.DGN | 266,240 | 32×32 | |
| 6 | LEV06.DGN | 239,616 | 32×32 | |
| 7 | LEV07.DGN | 258,048 | 32×32 | |
| 8 | LEV08.DGN | 303,104 | 32×32 | |
| 9 | LEV09.DGN | 288,768 | 32×32 | |
| 10 | LEV10.DGN | 290,816 | 32×32 | |
| 11 | LEV11.DGN | 278,528 | 32×32 | |
| 12 | LEV12.DGN | 321,536 | 32×32 | Largest (boss level?) |
| 13 | LEV13.DGN | 256,000 | 32×32 | |
| 14 | LEV14.DGN | 253,952 | 32×32 | |
| 15 | LEV15.DGN | 270,336 | 32×32 | Final level |

**Total: ~4.3 MB** — vs. DM1's ~33 KB. Ratio: ~130× larger.

### Format Structure
Each DGN file contains two sections:

1. **Grid section** (beginning of file):
   - 32×32 = 1024 square entries
   - Each square: lower 5 bits = square type (matching DM1 convention)
   - Square type 0 = solid wall; types 1-31 = various floor/passable
   - Grid uses same column-major format as DM1's DUNGEON.DAT

2. **3D geometry blob** (remaining bytes after grid):
   - Pre-computed wall polygon data per grid position
   - Floor/ceiling mesh vertices per open square
   - Per-square mesh identifiers for wall type, door state, stairs variant
   - No procedural geometry — all geometry is baked into the file

### Grid Parsing (Firestaff)
`nexus_v1_level_get_square()` in `nexus_v1_dungeon.c`:
```c
return (int)((uint8_t *)level->grid)[y * width + x] & 0x1F;
```
Lower 5 bits extracted, matching DM1 square type semantics.

### 3D Geometry Parsing
**Status: NOT YET IMPLEMENTED in Firestaff.**
The geometry blob parser is TODO — `nexus_v1_dungeon.c` only parses grid,
not the post-grid 3D geometry section. This is a known gap.

---

## 2. Creature Models: DMDF / .MNS Files

### DMDF Format (Dungeon Master Data Format)
Defined in `include/nexus_v1_dmdf_model.h`.

Header structure (all values big-endian / SH2 byte order):
```
Offset  Size  Field
0x00    4     Magic = 0x444D4446 ('DMDF')
0x04    4     file_size (uint32)
0x08    4     section_count (uint32)
0x0C    4     flags (uint32)
0x10    4     data_offset (uint32)
0x14    4     vertex_offset (uint32)
0x18    4     vertex_count (uint32)
0x1C    4     face_count (uint32)
...     ...   (data sections follow)
```

### Nexus_DMDFVertex Structure
```c
int16_t  x, y, z;      // Position
int16_t  nx, ny, nz;   // Normal vector
uint16_t u, v;          // Texture coordinates
```

### Face Format
Faces are uint16_t index arrays (3 per triangle, 4 per quad).
Face byte size = face_count × 6 bytes (3 uint16_t × 2 bytes each).

### DMDF Sections
The file contains multiple sections after the header:
- Vertex data section at vertex_offset
- Face index section
- Embedded BITMAP texture data (compressed)

### Example Models (from extracted data)
| Model | Size (bytes) |
|-------|-------------|
| ANTMAN.MNS | 53,768 |
| BIGWORM.MNS | 53,784 |
| BORKETH.MNS | 67,644 |
| CHAOS.MNS | 88,572 |
| DRA_ZOM.MNS | 83,508 |
| GHOST.MNS | 48,840 |
| GOLEM.MNS | 48,140 |
| H_HOUND.MNS | 46,364 |

### Big-Endian Handling
All multi-byte values read via `rb16()` / `rb32()` byte-swapping functions.
SH2 is big-endian; x86/ARM (PC builds) are little-endian.

### Loading (Firestaff)
`nexus_v1_dmdf_load()` in `nexus_v1_dmdf_model.c`:
1. Validates magic (0x444D4446)
2. Reads vertex_count, face_count from header
3. Allocates vertex array and face index array
4. Loads embedded BITMAP texture data

---

## 3. Sound Files

### Per-Level Sound Banks: SNDLEV00-15.SAL
- 16 sound banks, one per level
- Each: 290–460 KB
- Loaded on level entry
- CD audio track for music + ADX/SEGA PCM for SFX

### Script Files: SLEV00-15.BIN
- Per-level event scripts (2–12 KB each)
- Processed by SDDRVS.TSK script VM
- Declarative event rules for teleporters, traps, door animations

### Minimap Data: SMAP00-15.BIN
- Per-level minimap images (17–30 KB each)
- 2D overhead map for in-game map display

### Sound File Format (from nexus_overview.md)
- **CD-DA tracks** (tracks 2-9): Red Book Audio, per-level music
- **SFX format**: ADX (Sega ADPCM) or SEGA PCM
- **Streaming**: Audio streamed from CD in real-time via DMA (Saturn)

---

## 4. Other Asset Files

### FONT256.S2D
- 256-character font including Japanese Shift-JIS characters
- Loaded by `nexus_v1_font_load()` from `nexus_v1_saturn_font.c`
- Used for all in-game text rendering

### FACE.BIN (hypothetical)
No FACE.BIN file found in Nexus source code or documentation.
The task mentions FACE.BIN — possible candidates:
- Champion face portrait data (for UI panels)
- Stored as indexed bitmap (8bpp with palette)
- Big-endian encoding on disc

**Status: Not confirmed in codebase.** No references found in source/docs.

### SDDRVS.TSK (Script/Task File)
No SDDRVS.TSK implementation exists in Firestaff.
The task file is mentioned in dungeon/squares docs as:
- Declarative script processed by a virtual machine
- Handles teleporters, trap triggers, door animations
- Per-level scripts in SLEV00-15.BIN format
- SDDRVS.TSK is the main task/script VM file

**Status: SDDRVS.TSK parser NOT IMPLEMENTED in Firestaff.**

---

## 5. Data File Format Summary Table

| File | Format | Size | Purpose | Status |
|------|--------|------|---------|--------|
| LEV00-15.DGN | Binary (grid + 3D blob) | 147-322 KB each | Dungeon levels | Grid parsed; 3D geometry TODO |
| *.MNS | DMDF (big-endian) | 46-88 KB each | 3D creature models | Header + vertices + faces parsed |
| SNDLEV*.SAL | Binary | 290-460 KB each | Per-level audio bank | NOT PARSED |
| SLEV*.BIN | Binary | 2-12 KB each | Per-level script | NOT PARSED |
| SMAP*.BIN | Binary | 17-30 KB each | Minimap data | NOT PARSED |
| FONT256.S2D | Saturn font binary | ~64 KB | 256-char font (incl. JP) | Parsed (Saturn font loader) |
| DM.BIN | Binary | ~133 MB | Full disc image data | ISO reader exists |
| FACE.BIN | Unknown | Unknown | Champion faces? | NOT FOUND in source |

---

## 6. Big-Endian vs Little-Endian

Nexus uses **big-endian** encoding (SH2 Saturn processor).
All multi-byte values on disc are big-endian.

Firestaff PC builds (x86/ARM) are little-endian.
Byte-swapping functions handle conversion:
- `rb16()` — read uint16_t big-endian → host byte order
- `rb32()` — read uint32_t big-endian → host byte order

---

## 7. DM1 Data Format Comparison

| Aspect | DM1 | Nexus V1 |
|--------|-----|----------|
| Dungeon file | DUNGEON.DAT (~33 KB total) | LEV*.DGN (130× larger) |
| Grid format | Column-major, 5-bit types | Same 5-bit types, 32×32 fixed |
| 3D geometry | None (2D sprite) | Baked polygon meshes in DGN |
| Creature models | 2D sprites in GRAPHICS.DAT | DMDF .MNS 3D models |
| Texture format | PC CGA/EGA BITMAP | VDP1 BITMAP (big-endian) |
| Audio | PC speaker/AdLib | CD-DA + ADX |
| Sound banks | None | Per-level .SAL files |
| Scripts | Hardwired in game loop | SDDRVS.TSK + SLEV*.BIN |
