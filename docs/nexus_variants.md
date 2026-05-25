# Nexus V1 — Platform Variants

## Sources
- `docs/nexus_platform.md`
- `docs/nexus_overview.md`
- `docs/NEXUS_FILE_CLASSIFICATION.md`
- `docs/NEXUS_PLAN.md`
- `src/nexus/nexus_v1_engine.c`, `nexus_v1_iso_reader.c`
- Sega Saturn CD image: `Dungeon Master Nexus_SEGA-Saturn_JA.zip`

---

## 1. Original Platform: Sega Saturn (1998)

Dungeon Master Nexus was **exclusive to the Sega Saturn**, released in Japan only.

| Detail | Value |
|--------|-------|
| Platform | Sega Saturn (SH-2 big-endian) |
| Release date | 1998-02-03 |
| Publisher | Victor Interactive Software / FTL Games / Athena |
| Product ID | T-9111G V1.003 |
| Media | CD-ROM (CUE/BIN, 133 MB data + 8 CD-DA audio tracks) |
| Territory | Japan only — no Western release |
| Language | Japanese only |

### Saturn Hardware Context
- **CPU:** 2× Hitachi SH-2 @ ~10.5 MHz (big-endian)
- **3D:** Sega VDP1 polygon rasterizer (software on SH-2)
- **Resolution:** 320×224 (NTSC), 16-bit framebuffer
- **Audio:** Red Book CD-DA (tracks 2–9), ADX/SEGA PCM sfx
- **Storage:** CD-ROM + Saturn SRAM cartridge (8 KB save)

### Disc Image Structure
```
Track 1 (133 MB):  MODE1/2352 — ISO 9660 filesystem, game data
Tracks 2–9:        Red Book Audio CD-DA — per-level music
Total:             9 tracks, ~228 MB disc
```

---

## 2. No Windows Version

Unlike Dungeon Master II (which had DOS + Windows 3.1 releases), **Nexus was never ported to PC**. No Windows, DOS, or Macintosh version exists.

- No DM Nexus for Windows
- No DM Nexus for DOS
- No DM Nexus for PlayStation (which existed alongside Saturn in 1998)
- The 1998 landscape: PlayStation had won the32-bit war; Saturn was niche in Japan

Evidence: `docs/NEXUS_PLAN.md` states explicitly "Exclusive to Sega Saturn, Japanese only" and lists platform constraints (SH-2 big-endian, VDP1/VDP2) as core challenges.

---

## 3. No Java Version

No Java version of Dungeon Master Nexus was ever produced. The Java mention sometimes arises from confusion with:
- Firestaff's web/Emscripten target (planned future work — not implemented)
- General 1998-era speculation about Java game applets (never materialized for commercial titles)

Evidence: `docs/nexus_platform.md` states explicitly: "Java mention in task: No Java version of Dungeon Master Nexus exists. Java was not used for Saturn development or any subsequent port."

---

## 4. Firestaff Cross-Platform Reimplementation

Firestaff implements Nexus as a **cross-platform C library** that compiles on Windows/macOS/Linux:

| Platform | Status | Notes |
|----------|--------|-------|
| Sega Saturn (original) | N/A | No dev tools, source-locked |
| Windows (x64) | Implemented | CMake + MSVC/GCC |
| macOS (ARM64/x64) | Implemented | CMake + Clang |
| Linux (x64/ARM) | Implemented | CMake + GCC |
| Web (Emscripten/WASM) | Future work | No implementation yet |

This is a **reimplementation** from reverse-engineering, not a binary port. The Firestaff `firestaff_nexus` library has zero external dependencies (no SDL, no libm) — pure standard C.

### Data Source Modes
The engine supports two data modes (from `nexus_v1_engine.c`):
1. **NEXUS_SRC_ISO** — direct CUE/BIN disc image access (ISO 9660 MODE1/2352)
2. **NEXUS_SRC_EXTRACTED** — extracted CD contents on filesystem

---

## 5. Platform Variant Summary

| Variant | Platform | Language | Territory | Notes |
|---------|----------|----------|-----------|-------|
| T-9111G V1.003 | Sega Saturn | Japanese | Japan only | Only official release |
| Firestaff PC | Win/Mac/Linux | EN | Worldwide | Reimplementation |
| Firestaff Web | Emscripten | EN | Worldwide | Future work, not implemented |

**Bottom line:** One official release. No ports, no localization, no Java. The entire modern Nexus availability comes from Firestaff's reverse-engineered reimplementation.
