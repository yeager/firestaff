# Nexus V1 Open Issues

## Summary

Nexus V1 implementation is in scaffolding phase. All Phase 0-7 items are NOT DONE in TODO.md. No tests, no linked executable, no disc image. The open issues below span blocking infrastructure problems, missing implementations, and unknown risks.

---

## Blocker Issues

### B1: No Sega Saturn Disc Image

**Severity:** CRITICAL  
**Status:** No disc image present in repository  

No parity work can begin without the Sega Saturn disc image. The disc image is the single source of truth for all Nexus assets, formats, and behavior. Without it:

- No ISO parsing tests (tools/extract_nexus_iso.py untested)
- No dungeon data to parse
- No textures/models to decode
- No champion/monster data to verify
- No audio tracks to map

**Action required:** Obtain Dungeon Master Nexus (1998) Sega Saturn disc image, SHA256-lock it, place in secure canonical path.

---

### B2: Nexus Static Library Not Linked Into Any Binary

**Severity:** CRITICAL  
**Status:** libfirestaff_nexus.a builds but nothing links against it  

The CMake target `firestaff_nexus` produces a static library, but no `firestaff` or `firestaff_*` binary links to it. There is no `--profile nexus` entry point.

**Action required:** In TODO.md Phase 1, wire Nexus into the game binary as a named profile. Add a smoke test that verifies --profile nexus starts without loading DM1 assets.

---

### B3: No Tests for Nexus

**Severity:** CRITICAL  
**Status:** 0 tests, 0 CTest entries, 0% test coverage  

The test suite has 387 tests (DM1/M11/memory), none for Nexus. Cannot measure progress, detect regressions, or verify parity.

**Action required:** Write first Nexus test once binary is available. Add to CMake test target and verify.yml CI.

---

### B4: No Game Loop Integration

**Severity:** CRITICAL  
**Status:** nexus_v1_engine.c is isolated dead code  

firestaff_game_loop.c runs DM1 game loop. nexus_v1_engine.c is not invoked. Nexus game tick never runs.

**Action required:** Phase 1 must wire nexus_v1_engine into main loop as profile-switchable path.

---

## Missing Implementation Issues

### M1: VDP1/VDP2 Texture Format Not Implemented

**Severity:** HIGH  
**Status:** nexus_v1_rasterizer.c scaffolding only  

Saturn VDP1 textures (4bpp/8bpp paletted, 15-bit RGB, SH2 big-endian) are not parsed. No texture data loads. No renderer output possible without textures.

**Action required:** Document VDP1 format from disc inspection. Implement decoder in nexus_v1_rasterizer.c. Add fixture test.

---

### M2: DMDF Model Format Not Documented

**Severity:** HIGH  
**Status:** nexus_v1_dmdf_model.c scaffolding only  

DMDF format (3D model, quad list, texture references) is not documented. No model data loads. 3D creatures and geometry not rendered.

**Action required:** Reverse-engineer DMDF from disc. Document header, vertex layout, face format. Implement loader.

---

### M3: ISO 9660 + Saturn Header Parser Incomplete

**Severity:** MEDIUM  
**Status:** tools/extract_nexus_iso.py exists but not tested  

The extract_nexus_iso.py script exists but has not been run against a real disc image. Its correctness is unverified.

**Action required:** Test against real disc image. Verify file count, directory structure, track boundaries.

---

### M4: Shift-JIS Text Decoding Not Implemented

**Severity:** MEDIUM  
**Status:** nexus_v1_text.c scaffolding only  

Japanese text extraction (dungeon names, monster names, inscriptions, UI) from Shift-JIS to UTF-8 not implemented. Text will be garbled or missing.

**Action required:** Implement Shift-JIS decoder. Wire to text rendering in nexus_v1_saturn_font.c. Verify against known Japanese text strings from disc.

---

### M5: CD Audio Not Integrated

**Severity:** MEDIUM  
**Status:** nexus_v1_game.c has stub nexus_v1_cd_track_for_level() returning 2+(level/2)  

Red Book Audio tracks (Tracks 2-9 on the disc) are not played. No music in game. Sound effects also not wired (unknown format).

**Action required:** Integrate SDL_mixer for CD audio playback. Map level to track. Fade on level transition.

---

### M6: No Save/Load Implementation

**Severity:** MEDIUM  
**Status:** No save/load code exists for Nexus  

Saturn save format unknown. No save/load round-trip possible.

**Action required:** Identify save format from disc or documentation. Implement save/load. Write round-trip test.

---

### M7: V2 Renderer Not Wired to Game Loop

**Severity:** MEDIUM  
**Status:** nexus_v2_*.c files exist (lighting, atmosphere, particles, render_pipeline, upscaler) but are isolated  

V2 enhanced renderer exists but is not connected to game loop or viewport. V2.0 (filtered original), V2.1 (upscale), V2.2 (modern) all need wiring.

**Action required:** Phase 1.5+ V2.0/V2.1/V2.2 wiring after V1 gameplay is proven.

---

## Unknown/Risk Issues

### R1: No Source Disassembly Available

**Severity:** HIGH  
**Status:** No ReDMCSB equivalent for Nexus  

DM1 has ReDMCSB (WIP20210206) disassembly providing exact source-lock reference. Nexus has no equivalent. All format reverse-engineering must be done from disc inspection alone.

**Risk:** Some formats may be impossible to reverse-engineer accurately without disassembly. Unknown dungeon data structure, champion data format, save file format may have opaque binary layouts.

---

### R2: Region/Version Variants Unknown

**Severity:** MEDIUM  
**Status:** Unknown if other Nexus releases exist  

It is unknown if Dungeon Master Nexus was released in other regions (only Japanese known). No other versions confirmed. Disc used for implementation may not represent all releases.

**Risk:** Implementation may be specific to Japanese Sega Saturn release only.

---

### R3: No Equivalent to DM1 PC34 Compatibility Layer

**Severity:** HIGH  
**Status:** DM1 has a full compatibility layer (pc34_compat) proving file format compatibility  

DM1 V1 tests prove binary compatibility with original GRAPHICS.DAT and DUNGEON.DAT. Nexus has no such layer -- there is no confirmed reference for binary formats.

**Risk:** All Nexus format interpretations are best-effort reverse engineering. No verification path exists except actual gameplay.

---

### R4: Compatibility with DM1 Dungeon Data Is Assumed, Not Proven

**Severity:** MEDIUM  
**Status:** TODO.md says Nexus is DM1 underneath, but no evidence  

TODO.md claims Nexus dungeon format is an extended DM1 dungeon.dat format. This is unverified. If Nexus uses a different dungeon format, Phase 3 (world model) may be fundamentally wrong.

**Risk:** Dungeon loading implementation may fail on real disc data if format assumption is wrong.

---

## Documentation Issues

### D1: NEXUS_PLAN.md Incomplete

**Severity:** LOW  
**Status:** Plan exists but lacks time estimate for later phases and detailed risk mitigation  

Phase 1-6 time estimates present. Phase 7 (verification suite) time estimate missing. Several "TBD" entries in risk table.

---

### D2: No Nexus-Specific README

**Severity:** LOW  
**Status:** README.md covers firestaff broadly, no Nexus section  

README.md does not mention Nexus at all. No user-facing documentation for how to run Nexus mode.

---

## GitHub Issues (From Code Search)

No GitHub issues labeled "nexus" found in the repository. All open issues are DM1/CSB/DM2 related.

---

## Priority Order for Resolution

1. **B1 (Disc image)** -- nothing works without this
2. **B2 (Link library into binary)** -- enables running anything
3. **B4 (Game loop integration)** -- enables tick-based testing
4. **M3 (ISO parser test)** -- first test after disc image
5. **M1 (VDP1 texture format)** -- enables rendering
6. **M2 (DMDF model format)** -- enables 3D geometry
7. **R1 (No disassembly)** -- ongoing risk, must document unknowns
8. **M4 (Shift-JIS text)** -- enables Japanese text display
9. **M5 (CD audio)** -- enables music
10. **M6 (Save/load)** -- enables persistence
11. **M7 (V2 renderer)** -- Phase 1.5+, after V1 works
12. **B3 (Tests)** -- write as each phase completes

---

## Note: Parity-Evidence Nexus Directory

parity-evidence/nexus/ does not exist. When Phase 0 provenance gate is passed, this directory should be created with:
- disc_image_sha256.txt
- file_manifest.json
- iso_parse_output.txt
- dungeon_levels_summary.md

No such evidence exists yet.