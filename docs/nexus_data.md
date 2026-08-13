# Nexus V1 Data File Formats Audit — Source-Locked

> Äldre formatanteckningar kan beskriva historiska host-stubbar som
> “loading” eller “implemented”. De är inte runtime-bevis. Se
> [`NEXUS_STALE_CLAIM_AUDIT.md`](NEXUS_STALE_CLAIM_AUDIT.md) och den strikta
> inventeringen innan en route öppnas.

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
| 0 | LEV00.DGN | 147,456 | 64×64 Structure1B | Title-sequence entrance image; not playable |
| 1 | LEV01.DGN | 280,576 | 64×64 Structure1B | |
| 2 | LEV02.DGN | 272,384 | 64×64 Structure1B | |
| 3 | LEV03.DGN | 290,816 | 64×64 Structure1B | |
| 4 | LEV04.DGN | 245,760 | 64×64 Structure1B | |
| 5 | LEV05.DGN | 266,240 | 64×64 Structure1B | |
| 6 | LEV06.DGN | 239,616 | 64×64 Structure1B | |
| 7 | LEV07.DGN | 258,048 | 64×64 Structure1B | |
| 8 | LEV08.DGN | 303,104 | 64×64 Structure1B | |
| 9 | LEV09.DGN | 288,768 | 64×64 Structure1B | |
| 10 | LEV10.DGN | 290,816 | 64×64 Structure1B | |
| 11 | LEV11.DGN | 278,528 | 64×64 Structure1B | |
| 12 | LEV12.DGN | 321,536 | 64×64 Structure1B | Largest checked file |
| 13 | LEV13.DGN | 256,000 | 64×64 Structure1B | |
| 14 | LEV14.DGN | 253,952 | 64×64 Structure1B | |
| 15 | LEV15.DGN | 270,336 | 64×64 Structure1B | Final-level file; presentation remains gated |

**Total: the checked European corpus is retained as source data; do not infer
runtime presentation from file size alone.**

### Format Structure
Each DGN file is a 2048-byte-block container with a Structure1 header,
Structure1B collision/grid data and additional bounded structures:

1. **Structure1B**:
   - 64×64 = 4096 cells
   - Each cell occupies 8 bytes (`0x8000` bytes total)
   - Firestaff retains the bounded cell/square and collision-reference fields
     identified by the DMWeb decoder; unknown fields are not assigned DM1
     semantics

2. **Post-Structure1 data**:
   - Structure1C collision records and bounded Structure1A/1F/2/3 spans
   - Structure2 descriptor/payload bytes and Structure3 mesh evidence are
     retained only as source receipts until Saturn transform, palette, VDP1
     command order and runtime ownership are captured

### Grid Parsing (Firestaff)
`nexus_v1_level_get_square()` in `nexus_v1_dungeon.c` reads the already
validated 64×64 Structure1B cell view. It does not reinterpret the DGN as a
64×64 Nexus Structure1B grid. Do not apply the 32×32 DM1 `DUNGEON.DAT`
interpretation to these cells.

### 3D Geometry Parsing
**Status: bounded real-data intake implemented.**
`nexus_v1_dungeon.c` parses the 64×64 Structure1B grid and retains bounded
post-grid/Structure3 source receipts across LEV00–LEV15. Transform, material,
palette, VDP1 command ownership and final Saturn presentation remain
capture-gated; no host fallback geometry is permitted.

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

### Loading (historisk formatbeskrivning; presentation är spärrad)
`nexus_v1_dmdf_load()` in `nexus_v1_dmdf_model.c`:
1. Retail-byteidentiteten och DMDF-envelope kan verifieras.
2. Bounded vertex-/face-/texture-receipts får behållas som källproveniens.
3. Hostens transform, materialbindning, palette och VDP1-command-order är
   inte autentiserade och får därför inte presenteras som en färdig modell.

---

## 3. Sound Files

### Per-Level Sound Banks: SNDLEV00-15.SAL
- 16 sound banks, one per level
- Each: 290–460 KB
- Loaded on level entry
- CD audio is present in the retail disc layout; SAL/SDDRVS SFX transport
  and level-to-track selection remain capture-gated

### Script Files: SLEV00-15.BIN
- Real per-level SH-2 task candidates (the complete 16-file corpus)
- Bounded 36-byte entry spine, literal and RTS/call-shape receipts
- Task body, event ownership and SDDRVS dispatch remain opaque; no
  declarative event rules are inferred

### Minimap Data: SMAP00-15.BIN
- Per-level minimap images (17–30 KB each)
- 2D overhead map for in-game map display

### Sound File Format
- **CD-DA tracks** (tracks 2–9): Red Book Audio layout receipt; level
  selection is not source-bound
- **SFX format**: real SAL DataID-0 directory and bounded PCM metadata
- **MAP**: real eight-byte records from offset zero, retained as opaque
  selector/attribute/SAL windows
- **Streaming/playback**: not promoted without SDDRVS/event-dispatch evidence

---

## 4. Other Asset Files

### FONT256.S2D
- `FONT256.S2D` är 25,012 byte i den monterade retailrevisionen och har 242
  verifierade CG-tiles.
- Loadern behåller bytes/glyph-åtkomst som receipt.
- Saturns textkonsument, page/attribute mapping och synlig textplacering är
  inte verifierade; den används inte som bevis för all in-game text.

### FACE.BIN
The European corpus contains the real FACE.BIN resource. The bounded loader
admits its authenticated 20-record layout and retains 20 indexed 56×56
portrait surfaces with their 64-entry BGR555 palettes. Saturn VDP1 command
order, destination, scale and flip remain capture-gated; portraits are not
drawn from guessed host rectangles.

### SDDRVS.TSK
The real 26,610-byte SDDRVS.TSK is admitted as a Saturn sound-driver task
identity. It is not treated as a script VM or event-table parser. SLEV task
profiles and SAL/MAP metadata remain receipts only until an authenticated
execution trace proves the owner and ABI.

---

## 5. Data File Format Summary Table

| File | Format | Size | Purpose | Status |
|------|--------|------|---------|--------|
| LEV00-15.DGN | DMWeb DGN (Structure1B + bounded payload) | 147-322 KB each | Dungeon levels | Real grid/source receipts; Saturn mesh presentation gated |
| *.MNS | DMDF (big-endian) | 46-88 KB each | 3D creature models | Header + vertices + faces parsed |
| SNDLEV*.SAL | SAL DataID 0 + bounded payload | 290-460 KB each | Per-level audio bank | Real directory/metadata parsed; playback gated |
| SLEV*.BIN | SH-2 task candidate | 2-12 KB each | Per-level task payload | 16-file entry profile; dispatch opaque |
| SMAP*.BIN | LVMP binary | 17-30 KB each | 80×76 tilemap + 256-colour BGR555 palette + 8×8 indexed tiles | Parsed as source data; VDP2 placement remains gated |
| FONT256.S2D | Saturn font binary | 25,012 bytes in mounted retail revision | Page words plus 242 actual CG tiles; mapping is unproven | Section/CG receipt only |
| DM.BIN | Saturn executable/resource binary | 555,144 bytes in mounted retail revision | Startup/menu/VDP/audio code and resource anchors | Hash-verified source receipt; not an ISO image |
| FACE.BIN | Authenticated indexed portrait records | retail resource | Champion portraits | 20 records admitted; VDP1 placement gated |

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
| Grid format | Column-major, 5-bit types | 64×64 Structure1B cells; do not assume DM1 field semantics |
| 3D geometry | None (2D sprite) | Baked polygon meshes in DGN |
| Creature models | 2D sprites in GRAPHICS.DAT | DMDF .MNS 3D models |
| Texture format | PC CGA/EGA BITMAP | VDP1 BITMAP (big-endian) |
| Audio | PC speaker/AdLib | CD-DA + ADX |
| Sound banks | None | Per-level .SAL files |
| Scripts | Hardwired in game loop | SDDRVS.TSK + SLEV*.BIN |
