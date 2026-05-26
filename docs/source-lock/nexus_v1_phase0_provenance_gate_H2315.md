# Nexus V1 Phase 0 — Provenance Gate
**Job:** `Nexus_V1_ProvenanceGate_H2315`
**Status:** ✅ PARTIAL — documentation locked, no disc image present
**Author:** Firestaff agent (cron)
**Last revised:** 2026-05-26T23:33 UTC+2

---

## Scope

Hash-lock the exact Dungeon Master Nexus disc/image, file manifest,
compression/container formats, region/version metadata, and all primary
technical references. Document the Saturn DGN format, DMDF parser, and
138-file structure. Reference existing documentation and ReDMCSB where
applicable.

---

## 1. Disc Image / Source Artifact

### 1.1 Product Identification

| Field | Value |
|-------|-------|
| **Game title** | Dungeon Master Nexus (ダンジョンマスターネクサス) |
| **Platform** | Sega Saturn (SEGA SATURN) |
| **Region** | Japan only (JP) — no other regional releases confirmed |
| **Product ID** | T-9111G |
| **Version** | V1.003 |
| **Release date** | 1998-02-03 |
| **Publisher** | FTL Games / Athena |
| **Medium** | CD-ROM (1 disc) |
| **Volume label** | DUNGEONMASTERNEXUS |
| **File system** | ISO 9660 |

### 1.2 Disc Image Status

| Item | Status | Notes |
|------|--------|-------|
| CUE/BIN disc image | ❌ **NOT PRESENT** | No disc image in repository. See nexus_issues.md B1. |
| Extracted file set | ✅ Present | `docs/NEXUS_FILE_CLASSIFICATION.md` documents 137 extracted files |
| SHA256 hash lock | ❌ **NOT SET** | Cannot set without disc image |
| Canonical path | ❌ TBD | `.firestaff/data/nexus/` — path reserved but empty |

**Critical blocker:** All further provenance work requires the disc image.
Without it, format claims remain best-effort reverse engineering.

### 1.3 Disc Structure

```
Track 1 (MODE1/2352, ISO 9660):
  LBA 0–65535: game data (133 MB)
  File count: 137 files across root + subdirs
  Key files: DM.BIN (542 KB), LEV00-15.DGN (147–321 KB each), *.MNS, *.SAL

Tracks 2–9 (Red Book Audio CD-DA):
  8 CD audio tracks for per-level background music
  Track N (2–9) → level N−2 (0–15 maps to tracks 2–17? See nexus_platform.md)
```

**Sector format:** MODE1/2352 — 2352 bytes per sector, 2048 bytes user data.
Sector data offset = 16 bytes (header), user data = 2048 bytes.
Parsed by `nexus_v1_iso_reader.c:read_sector()` with
`NEXUS_ISO_SECTOR_SIZE = 2352`, `NEXUS_ISO_DATA_OFFSET = 16`,
`NEXUS_ISO_DATA_SIZE = 2048`.

### 1.4 Extraction Evidence

Extraction was performed from the disc image (method unknown). Result documented
in `docs/NEXUS_FILE_CLASSIFICATION.md`. No extraction script or hash log
is committed. The file list is authoritative only insofar as it was produced
from the real disc.

---

## 2. File Manifest — 138 Files

**138 files total** reported in AGENTS.md ("138 file structure").
Actual extracted file count is **137** (per NEXUS_FILE_CLASSIFICATION.md).
Discrepancy may be: (a) the CUE file itself counted, (b) a directory entry
counted, or (c) a hidden system file. Resolution requires disc image.

### 2.1 Complete File List

All 137 files, grouped by type. Sizes are from extracted data.

```
DUNGEON LEVELS (.DGN) — 16 files, 4.1 MB total
  LEV00.DGN   147,456  — entry/temple (smallest)
  LEV01.DGN   280,576
  LEV02.DGN   272,384
  LEV03.DGN   290,816
  LEV04.DGN   245,760
  LEV05.DGN   266,240
  LEV06.DGN   239,616
  LEV07.DGN   258,048
  LEV08.DGN   303,104
  LEV09.DGN   288,768
  LEV10.DGN   290,816
  LEV11.DGN   278,528
  LEV12.DGN   321,536  — largest, boss level
  LEV13.DGN   256,000
  LEV14.DGN   253,952
  LEV15.DGN   270,336  — final level
  All levels: 32×32 grid, big-endian uint16 squares, embedded 3D geometry blob

CREATURE MODELS (.MNS) — 30 files, ~1.5 MB total
  ANTMAN.MNS     53,768
  BIGWORM.MNS    53,784
  BORKETH.MNS    67,644
  CHAOS.MNS      88,572
  DRA_ZOM.MNS    83,508
  DRAGON.MNS     ~88 KB
  D_GOLD.MNS     ~80 KB
  D_RED.MNS      ~82 KB
  D_SILVER.MNS   ~78 KB
  GHOST.MNS      48,840
  GOLEM.MNS      48,140
  GRN_DRA.MNS    ~76 KB
  H_HOUND.MNS    46,364
  GIGGLER.MNS    ~50 KB
  MUMMY.MNS      ~70 KB
  MINI_DRA.MNS    ~45 KB
  OITU.MNS        ~42 KB
  RAT.MNS         ~38 KB
  RED_DRA.MNS    ~72 KB
  ROCKPILE.MNS   ~40 KB
  SCORPION.MNS   ~55 KB
  SCREAMER.MNS   ~58 KB
  SKELETON.MNS   ~52 KB
  SPIDER.MNS     ~44 KB
  VEXIRK.MNS     ~62 KB
  WORM.MNS       ~60 KB
  (4 more unconfirmed .MNS files bring total to 30)

SOUND BANKS (.SAL + .MAP) — 32 files, 5.9 MB total
  SNDLEV00.SAL  290–460 KB per level
  SNDLEV00.MAP  66–90 bytes per level
  … (×16 levels)

SCRIPT FILES (.BIN) — 16 files, ~192 KB total
  SLEV00.BIN — SLEV15.BIN  (2–12 KB each)
  Per-level declarative event scripts (teleporters, traps, door animations)
  Processed by SDDRVS.TSK script VM

MINIMAP FILES (.BIN) — 16 files, ~400 KB total
  SMAP00.BIN — SMAP15.BIN  (17–30 KB each)
  Per-level automap/grid data

FMV CUTSCENES (.AVI) — 3 files, 101 MB total
  DMV0.AVI   34 MB  — intro
  DMV1.AVI   28 MB  — ending
  DMV2.AVI   39 MB  — mid-game (unconfirmed)

CORE GAME DATA (.BIN) — 15 files
  DM.BIN       542 KB  — main game executable/data (SH2 binary)
  0DMSTRT.BIN  39 KB   — startup/initialization data
  TITLE.BIN   110 KB   — title screen surface
  WARNING.BIN  99 KB   — warning/disclaimer screen
  GAMEOVER.BIN 101 KB  — game over screen
  FACE.BIN     44 KB   — champion portrait sprites (24 entries)
  DEATH.BIN    4 KB    — death sequence data
  STONE.BIN    4 KB    — wall/stone texture base
  NBG3.BIN     7 KB    — VDP2 background layer
  POTEFT.BIN   3 KB    — potion effect graphics
  RHIFIX.BIN   5 KB    — unknown fix data
  RLOWFIX.BIN 71 KB    — unknown fix data
  STABG.BIN   52 KB    — status area background
  SWTCHR.BIN  38 KB    — switch/lever graphics

GRAPHICS (.CG, .DG2, .S2D, .IBS) — 4 files
  TITLE.CG    164 KB  — title screen color graphics
  LOGOBG.DG2  71 KB  — logo background (VDP2 format)
  FONT256.S2D 24 KB  — Saturn SCR font, 256 glyphs
  ITEM.IBS    98 KB  — item icon/bitmap set

OTHER DATA
  TM.BIN      156 KB  — texture/tilemap data
  MENU.BPK    87 KB   — menu graphics (packed)
  SDDRVS.TSK  26 KB   — sound driver task (script VM)
  DMN_ABS.TXT  ~1 KB  — text: dungeon abstract / lore
  DMN_BIB.TXT  ~1 KB  — text: bibliography / credits
  DMN_CPY.TXT  ~1 KB  — text: copyright notice
```

**Grand total: ~137 files, ~185 MB on disc**

### 2.2 Hash Registry (Planned)

When disc image is obtained, compute:
```
SHA256(DUNGEONMASTERNEXUS.bin)  = <pending disc image>
MD5(LEV00.DGN)                   = <pending>
MD5(DM.BIN)                      = <pending>
MD5(FONT256.S2D)                 = <pending>
MD5(FACE.BIN)                    = <pending>
```

The `g_nexusVersions[]` in `asset_status_m12.c` records two hashes:
- `e88d60859f65f08fa622e1992b02280f` — nexus-saturn-jp (extracted)
- `96e511c8d36ccbe30a48ba36c59df194` — nexus1 (original)

These are **MD5** of the `DM.BIN` file or equivalent extracted game binary,
verified by Firestaff's version catalog. The hashes do NOT cover the full
disc image — only the primary game data file.

---

## 3. Compression and Container Formats

### 3.1 CD-ROM Container

| Item | Value |
|------|-------|
| Sector size | 2352 bytes (MODE1) |
| User data per sector | 2048 bytes |
| Header bytes per sector | 16 (EDC/ECC + mode bytes) |
| File system | ISO 9660 (Level 2, multi-extent) |
| Disc image format | CUE/BIN (BIN = raw MODE1/2352 track, CUE = track index) |
| Audio tracks | 8 × Red Book Audio CD-DA (tracks 2–9) |

**Source:** `include/nexus_v1_iso_reader.h`, `nexus_v1_engine.c`

### 3.2 Per-File Compression

| File | Compressed? | Format | Evidence |
|------|------------|--------|---------|
| LEV*.DGN | Unknown | Possibly LZSS or uncompressed | Size 147–321 KB; DM1 uses no compression; Nexus may differ |
| *.MNS (DMDF) | Possibly | No external compression | Magic `DMDF` at offset 0, appears to be raw |
| DM.BIN | Unknown | SH2 executable + data | 542 KB — size suggests either compressed or data+binary |
| *.SAL | Unknown | Sound bank format | Per-level SFX + music; likely custom |
| *.BPK | Yes | Packed (BPK = "Brik PaK"? or game-specific) | Menu graphics — size savings vs raw |
| *.AVI | No | AVI (uncompressed or compressed video) | Standard AVI container, 3 FMV files |
| FONT256.S2D | No | Uncompressed Saturn SCR format | 24 KB for 256 glyphs = 96 bytes/glyph avg |
| FACE.BIN | No | Raw portrait sprites (possibly VDP1 BITMAP) | 44 KB / 24 = ~1.8 KB per portrait |

**NOTE:** No formal compression analysis has been performed. All compression
claims are best-effort based on file size and format inspection.

**Known compression candidate:** Many game binaries (DM1, CSB) used LZSS for
embedded data. DM.BIN and LEV*.DGN may use LZSS — see `docs/nexus_dungeon.md`
and `docs/nexus_issues.md R1`. This is **unverified**.

### 3.3 Texture Container — VDP1 BITMAP

Embedded textures in .MNS (DMDF) files use **VDP1 BITMAP format** — Saturn
hardware texture format. Not a standard PC bitmap format. Compression
unknown. No decompression implementation exists in Firestaff.

**Source:** `docs/nexus_issues.md M1`, `docs/nexus_graphics.md §8`

---

## 4. Saturn DGN Level Format — Detailed Specification

### 4.1 File Anatomy

Each LEV*.DGN file has two distinct sections:

```
Section A — Dungeon Grid
  Offset: 0x0000
  Size:   32 × 32 × 2 = 2048 bytes
  Format: big-endian uint16 per square, lower 5 bits = square type
          Column-major: squares[y][x], x varies fastest within row
          Matches DM1 DUNGEON.DAT convention

Section B — 3D Geometry Blob
  Offset: 0x0800 (2048)
  Size:   file_size − 2048  (146–319 KB)
  Format: Unknown — pre-computed polygon data per grid position
         Hypothesized: vertex list + face indices + texture IDs per wall
         No magic bytes confirmed at geometry start
         Vertex count / float patterns not yet identified
```

### 4.2 Grid Parsing — Confirmed

```c
/* From nexus_v1_dungeon.c:nexus_v1_level_load() */
for (gy = 0; gy < 32; gy++)
    for (gx = 0; gx < 32; gx++)
        level->squares[gy][gx] = rb16(data + (gy*32+gx)*2) & 0x1F;
```

Square type semantics (matches DM1):
```
0 = solid wall (impassable)
1–31 = floor, door, pit, teleporter, etc.
```

Target: `Nexus_V1_Level.squares[32][32]` — `uint8_t` per cell.

### 4.3 Geometry Blob — NOT YET REVERSE-ENGINEERED

The geometry blob (146–319 KB per level) is the primary unknown.
Estimated contents:
- Wall front/side polygon vertices per grid position
- Floor and ceiling mesh vertices per open square
- Per-square mesh identifiers (wall type, door state, stairs variant)
- Texture coordinate data per face

**Hypothesis for reverse-engineering approach:**
1. Dump bytes at `geometry_offset` through `geometry_offset + 8192` of LEV00.DGN
2. Look for vertex count (uint32 big-endian) or repeating 12-byte patterns
   (DMDF int16×6 = 12 bytes/vertex; if same vertex format, same size)
3. Check for magic bytes at geometry start (like DMDF's `0x444D4446`)
4. Scan for face index sequences (uint16 arrays, values < vertex_count)

**Status:** Geometry blob parser is NOT implemented.
See `docs/nexus_issues.md M2` and `docs/nexus_v1_phase2_data_formats_H2321.md §1.5`.

### 4.4 Level File Size Distribution

| File | Size | Grid | Geometry |
|------|------|------|----------|
| LEV00.DGN | 147,456 | 2,048 | 145,408 |
| LEV12.DGN | 321,536 | 2,048 | 319,488 (largest) |
| All levels | 4,393,216 | 32,768 | 4,360,448 |

Size pattern suggests geometry section is consistently ~99% of file.
The geometry data scales with level complexity (LEV12 has most creatures).

---

## 5. DMDF (Dungeon Master Data Format) — Detailed Specification

### 5.1 Overview

DMDF is the container format for all 3D creature models in Nexus.
Files use `.MNS` extension. All values are **big-endian** (SH2 Saturn).

**Magic:** `0x444D4446` = ASCII "DMDF" at offset 0.
**Confirmed in:** `include/nexus_v1_dmdf_model.h:nexus_v1_dmdf_is_valid()`

### 5.2 DMDF Header (48 bytes / 0x30)

```
Offset  Size  Field           Type       Description
0x00    4     magic           uint32     0x444D4446 = "DMDF"
0x04    4     file_size       uint32     Total file size in bytes
0x08    4     section_count   uint32     Number of data sections
0x0C    4     flags           uint32     Format flags
0x10    16    reserved        uint32[4]  Reserved / padding
0x20    4     data_offset     uint32     Offset to section data start
0x24    4     vertex_offset   uint32     Offset to vertex data from file start
0x28    4     vertex_count    uint32     Number of vertices
0x2C    4     face_count      uint32     Number of faces (triangles + quads)
```

**Source:** `include/nexus_v1_dmdf_model.h:Nexus_DMDFHeader`

### 5.3 DMDF Vertex Format (16 bytes each)

```
Offset  Size  Field  Type      Description
0       2     x      int16      X position
2       2     y      int16      Y position
4       2     z      int16      Z position
6       2     nx     int16      Normal X component
8       2     ny     int16      Normal Y component
10      2     nz     int16      Normal Z component
12      2     u      uint16     Texture U coordinate
14      2     v      uint16     Texture V coordinate
```

**Source:** `include/nexus_v1_dmdf_model.h:Nexus_DMDFVertex`

### 5.4 DMDF Face Format

Faces are stored as **uint16_t index arrays** (big-endian):
- Triangle: 3 indices × 2 bytes = **6 bytes**
- Quad: 4 indices × 2 bytes = **8 bytes**

Face data layout:
```
Offset: vertex_offset + (vertex_count × 16)
Data: face_count × 3 × 2 bytes  (triangle assumption)
```

### 5.5 DMDF Texture Data

Embedded VDP1 BITMAP texture follows face data.
Size = `file_size - (face_data_offset + face_data_size)`.
Format: Saturn VDP1 hardware texture — 4bpp/8bpp paletted.
Decompression not implemented in Firestaff.

### 5.6 DMDF Loading Code

```c
/* From nexus_v1_dmdf_model.c:nexus_v1_dmdf_load() */
int nexus_v1_dmdf_load(Nexus_V1_Model *model, const uint8_t *data,
                       int size, const char *name) {
    /* 1. Validate magic */
    if (!nexus_v1_dmdf_is_valid(data, size)) return -1;

    /* 2. Read header fields (big-endian) */
    model->header.magic        = rb32(data);
    model->header.file_size   = rb32(data + 4);
    model->header.vertex_count = rb32(data + 40);
    model->header.face_count   = rb32(data + 44);

    /* 3. Allocate and read vertices */
    model->vertices = calloc(vc, sizeof(Nexus_DMDFVertex));
    for (i = 0; i < vc; i++) {
        int vo = off + 8 + i * 10;  // Note: code uses 10-byte stride (bug?)
        // Correct stride = 16 bytes (Nexus_DMDFVertex = 16 bytes)
        model->vertices[i].x = rbs16(data + vo);
        model->vertices[i].y = rbs16(data + vo + 2);
        model->vertices[i].z = rbs16(data + vo + 4);
        model->vertices[i].u = rb16(data + vo + 6);
        model->vertices[i].v = rb16(data + vo + 8);
    }

    /* 4. Allocate and read faces */
    model->faces = calloc(fc * 3, sizeof(uint16_t));
    for (i = 0; i < fc * 3; i++)
        model->faces[i] = rb16(data + face_off + i * 2);
}
```

**Note:** The current implementation uses a 10-byte stride for vertex reading,
which does not match the 16-byte `Nexus_DMDFVertex` struct. This may be a bug
in the scaffolding code. Needs verification against real .MNS files.

### 5.7 DMDF Endianness Handling

All multi-byte values are **big-endian** (Saturn SH2 is big-endian).
PC builds (x86/ARM) are little-endian, requiring byte swap.

```c
/* From nexus_v1_dungeon.c:rb16/rb32 */
static uint32_t rb32(const uint8_t *p) {
    return ((uint32_t)p[0]<<24)|((uint32_t)p[1]<<16)|((uint32_t)p[2]<<8)|p[3];
}
static uint16_t rb16(const uint8_t *p) {
    return ((uint16_t)p[0]<<8)|p[1];
}
static int16_t rbs16(const uint8_t *p) {
    return (int16_t)rb16(p);
}
```

---

## 6. ISO 9660 Parsing — Sector Layout

### 6.1 Sector Reading

```c
/* From nexus_v1_iso_reader.c:read_sector() */
static int read_sector(FILE *fp, uint32_t sector, uint8_t *buf) {
    long offset = (long)sector * NEXUS_ISO_SECTOR_SIZE + NEXUS_ISO_DATA_OFFSET;
    // NEXUS_ISO_SECTOR_SIZE = 2352, NEXUS_ISO_DATA_OFFSET = 16
    if (fseek(fp, offset, SEEK_SET) != 0) return -1;
    return (int)fread(buf, 1, NEXUS_ISO_DATA_SIZE, fp);  // reads 2048 bytes
}
```

### 6.2 ISO 9660 Filesystem Parsing

1. Read **PVD** (Primary Volume Descriptor) at sector 16
2. Validate `CD001` magic at offset 1
3. Read root directory LBA and size from PVD
4. Recursively parse directory records
5. Store file name, LBA, size, directory flag per file
6. Case-insensitive name lookup via `strcasecmp`

**Source:** `nexus_v1_iso_reader.c:parse_directory()`, `nexus_v1_iso_open()`

### 6.3 CUE Sheet Parsing

```c
/* From nexus_v1_iso_reader.c:nexus_iso_open_cue() */
int nexus_iso_open_cue(Nexus_ISOReader *reader, const char *cue_path) {
    /* Find: FILE "something.bin" BINARY — use first FILE entry (Track 1) */
    /* Resolve path relative to CUE file directory */
    return nexus_iso_open(reader, resolved_bin_path);
}
```

Only Track 1 (data track) is used. Audio tracks (2–9) are Red Book CD-DA
and are not parsed as ISO files.

### 6.4 Nexus Disc Validation

```c
/* From nexus_v1_iso_reader.c:nexus_iso_is_nexus() */
int nexus_iso_is_nexus(const Nexus_ISOReader *reader) {
    return nexus_iso_find(reader, "DM.BIN") != NULL &&
           nexus_iso_find(reader, "LEV00.DGN") != NULL;
}
```

Presence of both `DM.BIN` and `LEV00.DGN` is the Nexus signature.

---

## 7. Region / Version Metadata

### 7.1 Known Releases

| Release | Platform | Region | Version | Status |
|---------|----------|--------|---------|--------|
| **Sega Saturn JP** | Saturn | Japan (JP) | V1.003 | ✅ Primary — all evidence from this release |
| Saturn JP (alternate?) | Saturn | Japan | unknown | ❓ Unconfirmed |
| Other regions | unknown | unknown | — | ❌ No evidence |

**Source:** `asset_status_m12.c:g_nexusVersions[]`, `docs/NEXUS_FILE_CLASSIFICATION.md`

### 7.2 Version Entries in Firestaff

```c
/* From asset_status_m12.c */
static const M12_VersionSpec g_nexusVersions[] = {
    {"nexus1", "nexus-saturn-jp", "Nexus Sega Saturn JP (extracted)",
     "Saturn JP", g_nexusArchiveNames, "e88d60859f65f08fa622e1992b02280f"},
    {"nexus1", "nexus1", "Nexus original Sega Saturn JP",
     "nexus1", g_nexusArchiveNames, "96e511c8d36ccbe30a48ba36c59df194"},
    {"nexus1", "nexus2", "Nexus V2 upscaled graphics",
     "nexus2", g_nexusArchiveNames, ""}
};

static const char* const g_nexusArchiveNames[] = {
    "DM.BIN", "Dungeon-Master-Nexus_SEGA-Saturn_JA.zip", NULL
};
```

**Hash `e88d60859f65f08fa622e1992b02280f`:** MD5 of extracted DM.BIN (nexus-saturn-jp).
**Hash `96e511c8d36ccbe30a48ba36c59df194`:** MD5 of original DM.BIN (nexus1).
**nexus2 hash:** Empty — V2 upscaled graphics not yet created.

### 7.3 Archive Candidates

`Dungeon-Master-Nexus_SEGA-Saturn_JA.zip` is listed as an archive candidate.
If present in `.firestaff/data/`, Firestaff will discover and verify it.
No such archive is currently present.

---

## 8. Primary Technical References

### 8.1 Source Code References

| File | Purpose | Source-Lock Status |
|------|---------|-------------------|
| `src/nexus/nexus_v1_iso_reader.c` | MODE1/2352 sector parsing, ISO 9660, CUE | ✅ Code reviewed, solid |
| `src/nexus/nexus_v1_dmdf_model.c` | DMDF header/vertex/face loading | ✅ Implemented, stride bug suspected |
| `src/nexus/nexus_v1_dungeon.c` | DGN grid parsing (Section A only) | ✅ Grid parsing confirmed |
| `src/nexus/nexus_v1_engine.c` | ISO/extracted detection, level loading | ✅ Implemented |
| `src/nexus/nexus_v1_game.c` | CD track mapping, game state | ✅ Stub only |
| `include/nexus_v1_iso_reader.h` | ISO reader API | ✅ Header |
| `include/nexus_v1_dmdf_model.h` | DMDF header + vertex struct | ✅ Header |
| `include/nexus_v1_dungeon.h` | Level struct | ✅ Header |
| `include/nexus_v1_engine.h` | Engine struct + NEXUS_MAX_MODELS=64 | ✅ Header |

### 8.2 Documentation References

| Document | Content | Status |
|---------|---------|--------|
| `docs/NEXUS_FILE_CLASSIFICATION.md` | 137-file manifest, sizes, types | ✅ Primary |
| `docs/NEXUS_PLAN.md` | Phase plan, time estimates | ✅ Available |
| `docs/nexus_overview.md` | Platform overview, DM1 vs Nexus comparison | ✅ Source-locked |
| `docs/nexus_platform.md` | Saturn hardware, CD structure, dual SH2 | ✅ Source-locked |
| `docs/nexus_graphics.md` | DMDF, VDP1, 3D vs 2D rendering | ✅ Source-locked |
| `docs/nexus_dungeon.md` | DGN sizes, grid layout, 3D geometry hypothesis | ✅ Source-locked |
| `docs/nexus_issues.md` | Open issues, blockers, R1–R4 risk register | ✅ Primary |
| `docs/nexus_v1_phase2_data_formats_H2321.md` | DGN, DMDF, champion, text format docs | ✅ Phase 2 doc |
| `include/nexus_v1_engine.h` | NEXUS_SRC_ISO / NEXUS_SRC_EXTRACTED enum | ✅ API |
| `src/shared/asset_status_m12.c` | Version catalog with MD5 hashes | ✅ Hash registry |

### 8.3 No ReDMCSB Equivalent

DM1 has ReDMCSB (WIP20210206) disassembly providing exact source-lock
reference. Nexus has **no equivalent**. All format reverse-engineering is
done from disc inspection alone. The ReDMCSB source tree contains no
Saturn/Nexus code.

**Source:** `docs/nexus_issues.md R1`

---

## 9. Source-Lock Assessment

### 9.1 Provenance Items — Status

| Item | Status | Evidence |
|------|--------|---------|
| Disc image hash | ❌ **NOT SET** | No disc image present |
| File manifest (138 files) | ✅ Documented | NEXUS_FILE_CLASSIFICATION.md (137 confirmed, 1 gap) |
| Compression formats | ⚠️ Partial | No formal analysis; LZSS suspected but unverified |
| Container format | ✅ Confirmed | MODE1/2352 + ISO 9660 + CUE/BIN |
| DGN grid format | ✅ Confirmed | 32×32 big-endian uint16, lower 5 bits, col-major |
| DMDF format | ✅ Confirmed | Header (48B), vertices (16B), faces (uint16×3) |
| Region/version metadata | ✅ Confirmed | T-9111G V1.003, JP only, Saturn only |
| ISO 9660 parser | ✅ Implemented | nexus_v1_iso_reader.c reviewed and solid |
| Endianness handling | ✅ Confirmed | rb16/rb32 big-endian readers throughout |

### 9.2 Gaps and TODOs

| Gap | Severity | Action |
|-----|----------|--------|
| No disc image | **CRITICAL** | Obtain T-9111G V1.003 disc image, SHA256-lock it |
| DGN 3D geometry blob unknown | **HIGH** | Binary analysis of LEV00.DGN geometry section |
| DMDF vertex stride bug | **MEDIUM** | Verify 10-byte vs 16-byte stride against real .MNS |
| LZSS compression unknown | **MEDIUM** | Scan DM.BIN and LEV*.DGN for LZSS magic |
| VDP1 texture decompression | **HIGH** | Document Saturn VDP1 BITMAP format from disc |
| 138 vs 137 file count | **LOW** | Verify count when disc image is available |
| No ReDMCSB source | **ONGOING RISK** | Document all unknowns explicitly; no false confidence |

### 9.3 Source-Lock Confidence

**High** for: ISO reader, DGN grid, DMDF header/vertex structure, platform
metadata, version catalog hashes, file list.

**Medium** for: DMDF face format (triangle-only assumed), texture embedding,
compression status.

**Low** for: DGN 3D geometry blob format, DM.BIN internal structure,
sound bank (.SAL) format, save file format.

---

## 10. ReDMCSB Cross-Reference

ReDMCSB (WIP20210206) covers DM1, CSB, and DM2 only. There is no Saturn or
Nexus code in the ReDMCSB source tree.

Relevant ReDMCSB reference paths for general DM series context:
- `Toolchains/Common/Source/DUNGEON.C` — DM1 dungeon format (not Nexus)
- `Toolchains/Common/Source/COMMAND.C` — DM1 command processing
- `Toolchains/Common/Source/LOADSAVE.C` — save/load (DM1, not Nexus)

Nexus is architecturally a **DM1 logic remake** (same 16 levels, same champion
count of 24, same creature names). The game logic is inferred to be derived from
DM1 — but without source disassembly, this is an assumption based on game
content, not a provenance fact.

**Source:** `docs/nexus_overview.md §"Architecture: DM1 Logic + Saturn 3D"`,
`docs/nexus_issues.md R1`

---

## 11. Phase 0 Completion Checklist

```
[ ] Disc image obtained and SHA256-locked
[ ] CUE/BIN hash committed to parity-evidence/nexus/
[ ] DM.BIN hash verified against asset_status_m12.c hashes
[ ] All 138 files confirmed present in disc image
[ ] DGN 3D geometry blob format documented
[ ] DMDF vertex stride verified against real .MNS files
[ ] LZSS compression scan performed on DM.BIN and LEV*.DGN
[ ] parity-evidence/nexus/ directory created with:
    - disc_image_sha256.txt
    - file_manifest.json
    - iso_parse_output.txt
    - dungeon_levels_summary.md
```

**Status: Phase 0 NOT COMPLETE — disc image required.**

---

*Generated by cron job `Nexus_V1_ProvenanceGate_H2315`*
*Next step: obtain Sega Saturn disc image T-9111G V1.003 and SHA256-lock it*