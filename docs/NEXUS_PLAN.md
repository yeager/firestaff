# DM Nexus — Implementation Plan

> Status correction — 2026-08-13: This is a historical implementation plan,
> not the current completion state. ISO/Track-1 reading, real-file hashing,
> DGN/DMDF/MNS/S2D/FACE/SMAP parsing, PRS3 decoding, host mechanics, save
> round-trips, and bounded VDP1/VDP2 capture replay are implemented and
> externally verified. See [`NEXUS_COMPLETION.md`](NEXUS_COMPLETION.md) and
> [`NEXUS_RUNTIME_CAPTURE.md`](NEXUS_RUNTIME_CAPTURE.md) for authoritative
> status. Do not use the old phase estimates or release target below as a
> completion claim.

## Background

Dungeon Master Nexus (1998) is the fourth and final game in the DM series. It
is a Sega Saturn 3D polygon remake of DM1.

## Available Data

- Sega Saturn CD image (CUE/BIN), with authentic Japanese and English/Merged
  operator captures available outside the repository
- Track 1 (133 MB): MODE1/2352 — game data (Saturn filesystem, ISO 9660)
- Track 2-9 (~95 MB): AUDIO — CD music (Red Book Audio)
- No original source code or ReDMCSB equivalent. DMWeb, Greatstone, static
  disassembly, and instrumented Saturn captures are the reference evidence.

## Challenges

| # | Challenge | Difficulty | Description |
|---|---|---|---|
| 1 | Saturn CD filesystem | Medium | Extract files from Track 1 |
| 2 | Asset formats | High | Saturn VDP1/VDP2 textures, SH2 big-endian |
| 3 | No source code | Very high | Must reverse-engineer or infer from DM1 |
| 4 | 3D rendering | High | Polygons, not 2D sprites |
| 5 | Japanese text | Medium | Shift-JIS extraction + translation |
| 6 | CD audio | Low | Standard Red Book Audio |

## Phase 1: Data Extraction (1-2 days)

### 1.1 Saturn ISO parser — implemented
- Read Track 1 as ISO 9660 with Saturn header
- Tool: bchunk or custom MODE1/2352 parser
- Output: directory structure + file listing

### 1.2 File classification — implemented
- Identify types: graphics, models, textures, sound, dungeon, text
- Compare with DM1/DM2 naming conventions

### 1.3 CD audio extraction — capture-gated
- Track 2-9 AUDIO to WAV/FLAC
- Map tracks to game situations

### New files
- `include/nexus_v1_iso_reader.h`
- `src/nexus/nexus_v1_iso_reader.c`
- `tools/extract_nexus_iso.py`

## Phase 2: Asset Parsing (2-3 days)

### 2.1 Texture format — substantially implemented
- Saturn VDP1/VDP2: 4bpp/8bpp paletted, 15-bit RGB
- SH2 big-endian byte order
- CLUT identification

### 2.2 3D model format — parser/provenance implemented; runtime transform/culling remains capture-gated
- Saturn VDP1 quads: 4-vertex textured polygons
- Extract vertices, faces, texture references

### 2.3 Dungeon data — implemented for the verified real corpus
- Likely extended DM1 dungeon.dat format
- Square types, thing lists, creature placement

### 2.4 Text extraction — source parsing implemented; Saturn text consumer remains capture-gated
- Shift-JIS to UTF-8
- Champion names, monster names, spells, UI, inscriptions

### New files
- `include/nexus_v1_texture_format.h` + `.c`
- `include/nexus_v1_model_format.h` + `.c`
- `include/nexus_v1_dungeon_loader.h` + `.c`
- `include/nexus_v1_text_extractor.h` + `.c`

## Phase 3: Rendering (3-5 days)

### 3.1 Software 3D renderer — bounded host/capture adapters exist; retail production rasterization remains blocked
- VDP1-like quad renderer in software
- Perspective-correct texture mapping
- Z-buffer or painter's algorithm
- 320x224 native, then EPX upscale

### 3.2 V1: Saturn faithful — requires a source-owned Saturn scene witness
- Original 320x224 resolution
- Saturn 15-bit RGB palette
- Original frame rate

### 3.3 V2.1: Upscaled
- 320x224 rendered, EPX 2x to 640x448

### 3.4 V2.2: Enhanced
- Smooth camera rotation
- Texture filtering
- Dynamic lighting, particles, minimap via shared V2.2 modules

### New files
- `include/nexus_v1_renderer.h` + `.c`
- `include/nexus_v2_viewport.h` + `.c`

## Phase 4: Game Logic (2-3 days)

### 4.1 DM1-based logic — real Nexus mechanics and multi-level playability are implemented
- Nexus IS DM1 underneath — same dungeon, monsters, spells
- Reuse DM1 V1 modules: combat, spells, inventory, creature AI
- Test: load DM1 dungeon.dat, render with Nexus 3D renderer

### 4.2 Nexus-specific — real level corpus and source-bound state are implemented; Saturn pose/save consumer remains open
- Possible different creature stats
- Japanese champion names
- Saturn save format

### 4.3 CD music — disc layout is verified; level selector and playback remain capture-gated
- Map levels to audio tracks
- SDL_mixer for playback
- Fade on level change

### New files
- `include/nexus_v1_game_state.h` + `.c`
- `include/nexus_v1_cd_audio.h` + `.c`

## Phase 5: Japanese Text + Translation (1-2 days)

- Shift-JIS to UTF-8 conversion
- Japanese to English translation
- Japanese to Swedish + 18 other languages
- CJK font support in V2 renderer

### New files
- `include/nexus_v1_shift_jis.h` + `.c`
- `src/shared/firestaff_l10n_ja_nexus.c`

## Phase 6: Integration + Test (1-2 days)

- CLI and start-menu integration exist.
- The external-data Nexus CTest selection completed 304/304 tests; capture-only
  tests remain skip-safe when their authenticated witness is absent.
- Native Firestaff save/load round-trips exist; Saturn backup-RAM import is not
  authenticated.
- No release is implied by this document.

## Time Estimate

| Phase | Task | Time |
|---|---|---|
| 1 | Data extraction | 1-2 days |
| 2 | Asset parsing | 2-3 days |
| 3 | 3D rendering | 3-5 days |
| 4 | Game logic | 2-3 days |
| 5 | Japanese text + translation | 1-2 days |
| 6 | Integration + test | 1-2 days |
| **Total** | | **Historical estimate only; not a current schedule** |

## Risks

| Risk | Likelihood | Impact | Mitigation |
|---|---|---|---|
| Unknown Saturn formats | High | Blocks phase 2-3 | Reference Yabause/Kronos emulator source |
| Non-standard 3D models | Medium | Delays phase 3 | Fallback: DM1 sprites + perspective |
| Nexus logic differs from DM1 | Low | Extra work | DM1 covers 90%+ |
| CJK font support | Low | V2 needs Unicode | Start with ASCII transcription |

## Dependencies

- bchunk/cdrdao for ISO extraction
- Yabause/Kronos source as VDP format reference
- DM1 V1 modules for shared game logic
- V2 animation timing system

## File Structure

```
src/nexus/
  nexus_v1_iso_reader.c
  nexus_v1_texture_format.c
  nexus_v1_model_format.c
  nexus_v1_dungeon_loader.c
  nexus_v1_renderer.c
  nexus_v1_game_state.c
  nexus_v1_cd_audio.c
  nexus_v1_text_extractor.c
  nexus_v1_shift_jis.c
  nexus_v2_viewport.c
```

~15-20 new files, ~3000-5000 lines of C.
