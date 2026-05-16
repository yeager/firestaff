## Firestaff v0.6.0 — Complete Collection

### Rendering Pipeline
- GRAPHICS.DAT bitmap extraction: 713 graphics parsed from original PC-34 data
- 4bpp decompression: 2 pixels per byte, high nibble first
- Real DM1 VGA 16-color palette from ReDMCSB
- Viewport: graphic #0 (224x136) blitted from original data
- HUD panel: graphic #1 (320x200) bottom 64 rows from original data
- Wall texture mapping: D0-D3 distance, L/C/R position per wall set
- Dungeon view cone: square type query for 4 distances x 3 positions
- Full pipeline: GRAPHICS.DAT → 4bpp → palette RGBA → EPX upscale → SDL present

### Games
- Dungeon Master (DM1): V1 + V2.1 + V2.2, 62 V1 modules, 36 V2 modules
- Chaos Strikes Back (CSB): V1 + V2, 12 modules, DSA bytecode interpreter
- Dungeon Master II (DM2): V1 + V2, 10 modules, outdoor renderer

### 20 Languages
EN, SV, DE, FR, ES, IT, PT, NL, PL, CS, RU, JA, KO, ZH, DA, NO, FI, HU, TR
- Auto-detects system language
- Swedish game text with correct ÅÄÖ (Firestaff original translation)
- Dungeon inscriptions translated for all 20 languages
- Language-specific DUNGEON.DAT loading (EN/FR/DE)

### Features
- CLI: firestaff dm1 --v21 --fullscreen --lang sv
- Hierarchical start menu with 5 navigation levels
- 54 settings across 5 tabbed categories
- Settings persistence (INI save/load)
- Save/load system (10 slots per game)
- Bestiary (24 creatures), Spell Reference (14 spells), Map Viewer, Item Encyclopedia (33 items), Screenshot Gallery

### Technical
- 927 source files, 188K lines of C
- 100% ReDMCSB function parity (40/40 source files)
- Unified V2 animation timing (V1 tick sync, sub-tick interpolation)
- Repo: 28 root items (restructured from 1040+)
- 304 tests

### Downloads
macOS (Apple Silicon + Intel), Windows, Linux (DEB + RPM)
