## Firestaff v0.5.1

### New Features
- **20 languages**: EN, SV, DE, FR, ES, IT, PT, NL, PL, CS, RU, JA, KO, ZH, DA, NO, FI, HU, TR
  - Auto-detects system language from LC_ALL/LANG
  - Full UI string tables for all 20 languages
- **CLI**: `firestaff dm1 --v21 --fullscreen --lang sv` — all games, versions, settings
- **Bestiary**: 24 creatures from DM1/CSB/DM2 with full stats
- **Spell Reference**: 14 spells with symbol sequences and mana costs
- **Map Viewer**: Dungeon level renderer with fog-of-war
- **Item Encyclopedia**: 33 items across 7 categories
- **Screenshot Gallery**: Browse captured screenshots
- **GRAPHICS.DAT reader**: Bitmap decompression (RLE + 4bpp expand)
- **Dungeon text extraction**: Foundation for translated inscriptions/scrolls

### Improvements
- Viewport rendering pipeline: indexed FB → palette RGBA → SDL present
- SDL3 event loop: keyboard, mouse, quit
- Settings persistence (INI save/load)
- Menu navigation: 5-level hierarchy with localized labels
- Swedish game text: all DM1 in-game strings with correct ÅÄÖ

### Technical
- 920 source files, 187K lines of C
- 304 tests (278 pass, 26 known failures)
- 4 games × 3 versions, repo restructured (28 root items)
- GitHub Actions: release + verify workflows
