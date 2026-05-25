# Nexus V1 Platform Audit — Source-Locked

## Sources
- `src/nexus/nexus_v1_engine.c`, `nexus_v1_iso_reader.c`
- `include/nexus_v1_engine.h`
- `docs/nexus_overview.md`, `docs/nexus_graphics.md`
- Saturn CD image (CUE/BIN), Japanese version T-9111G V1.003 (1998-02-03)

---

## 1. Primary Platform: Sega Saturn

### Why Saturn?
Dungeon Master Nexus was an **exclusive Sega Saturn title, Japanese only** (1998).
FTL Games / Athena chose Saturn for its:
- **VDP1 polygon rendering** — hardware-accelerated 3D triangles (critical for a 3D remake)
- **VDP2 for backgrounds** — layer support for UI overlays
- **CD-ROM + Red Book Audio** — 8 CD audio tracks for per-level music
- **SH2 CPU architecture** — 32-bit dual-processor (master/slave)

### Saturn Hardware Specs
| Component | Detail |
|-----------|--------|
| CPU | 2× Hitachi SH-2 @ ~10.5 MHz (big-endian) |
| RAM | 2 MB main + 1.5 MB video + 0.5 MB sound |
| 3D Engine | Sega VDP1 (polygon rasterizer, 20M polygons/s theoretical) |
| Resolution | 320×224 (NTSC) or 320×240 (PAL), 16-bit framebuffer |
| Display | VDP2 for backgrounds, layers, transparency |
| Storage | CD-ROM (MODE1/2352 sectors), 8 tracks CD-DA audio |
| Controller | Saturn 8-button pad (same input semantics as keyboard/mouse) |

### Saturn-Specific Code in Firestaff
The Firestaff Nexus implementation handles Saturn-specific formats:
- **Big-endian SH2** data encoding — all multi-byte values byte-swapped
- **VDP1/VDP2 textures** — 4bpp/8bpp paletted, 15-bit RGB
- **ISO 9660 CD-ROM** parsing — MODE1/2352 sector reading
- **CUE/BIN disc image** support — for extracted disc access
- **Dual SH2 memory model** — VDP1 framebuffer at fixed address ranges

### Sega Tools
Original development used Sega's development tools for SH2:
- Sega C compiler for SH2 (Sega-provided toolchain)
- Custom linker scripts for Saturn memory layout
- No GNU toolchain — proprietary embedded development

---

## 2. Other Platforms (Firestaff Cross-Platform Build)

Firestaff implements Nexus V1 as a **cross-platform C library** that compiles
on Windows/macOS/Linux via standard toolchains (GCC/Clang/MSVC).

### Platform Support Matrix
| Platform | Status | Build |
|----------|--------|-------|
| Sega Saturn (original) | N/A — no modern dev | N/A |
| Windows (x64) | Implemented | CMake + MSVC/GCC |
| macOS (ARM64/x64) | Implemented | CMake + Clang |
| Linux (x64/ARM) | Implemented | CMake + GCC |
| Web (Emscripten) | Not yet | Future work |

### Cross-Platform Audio
- **Original Saturn**: Red Book CD audio (tracks 2-9), ADX/SEGA PCM sfx
- **Firestaff PC**: SDL3 audio, Ogg/MP3 fallback, optional CD audio via platform APIs

### Cross-Platform Rendering
- **Original Saturn**: VDP1 software rasterizer on custom hardware
- **Firestaff**: CPU software rasterizer (nexus_v1_rasterizer.c) → SDL framebuffer
- No GPU dependencies — pure software 3D pipeline

### Platform Detection
Platform is detected at compile time via standard preprocessor macros:
```c
#ifdef _WIN32   /* Windows (MSVC or MinGW) */
#elif defined(__APPLE__)  /* macOS */
#else  /* Linux / generic POSIX */
#endif
```

---

## 3. Platform Differences from DM1/CSB

| Aspect | DM1/CSB | Nexus V1 |
|--------|---------|----------|
| Platform | Amiga/Atari ST/DOS | Sega Saturn (original); Win/Mac/Linux (Firestaff) |
| Endianness | Little (x86/68k) | Big (SH2); little on PC builds |
| Graphics API | None (bare metal) | VDP1/VDP2 on Saturn; SDL on PC |
| Storage | Floppy/CD | CD-ROM (original); filesystem (Firestaff) |
| Audio | PC speaker/AdLib | CD-DA + ADX (Saturn); SDL (Firestaff) |
| Memory model | 64 KB RAM assumed | 2 MB RAM (Saturn); modern PC (Firestaff) |

---

## 4. Data Source Architecture

Nexus V1 supports two data source modes (nexus_v1_engine.c):
- **NEXUS_SRC_ISO**: Read directly from CUE/BIN disc image (ISO 9660, MODE1/2352)
- **NEXUS_SRC_EXTRACTED**: Read from extracted CD contents on disk

Discovery priority:
1. Search for .cue file in data directory → open .bin as ISO
2. Otherwise check for extracted files (DM.BIN, LEV00.DGN presence)

Saturn disc image structure:
- **Track 1**: MODE1/2352, ISO 9660 filesystem, game data (133 MB)
- **Tracks 2-9**: Red Book Audio CD-DA, per-level music tracks

---

## 5. Original Platform Constraints (Saturn)

### Memory Layout
- SH-2 address space: 4 GB linear
- VDP1 framebuffer: fixed address range (~0x25C00000)
- Work RAM: 0x06020000–0x062FFFFF (2 MB)
- CD-ROM DMA: transfers directly to work RAM

### Rendering Pipeline (Saturn Native)
1. SH-2 CPU computes 3D geometry transforms
2. VDP1 rasterizes triangles (polygon engine)
3. VDP2 composites background layers
4. Output to composite/S-Video/RGB

### CD Audio Architecture
- 8 CD-DA tracks (tracks 2-9 of disc)
- Per-level music: track N plays on level N-1
- Audio streamed from CD in real-time via DMA

---

## 6. Porting Notes

**Original Nexus (Saturn) → Firestaff (PC):**
- 3D math: identical, floating-point instead of fixed-point SH2
- Dungeon format: same LEV*.DGN files
- DMDF models: same .MNS files
- Game logic: DM1 mechanics ported from ReDMCSB
- Audio: CD-DA replaced with SDL audio stream

**Java mention in task:** No Java version of Dungeon Master Nexus exists.
Java was not used for Saturn development or any subsequent port. This may
refer to a hypothetical Firestaff web build using Emscripten/WASM.
