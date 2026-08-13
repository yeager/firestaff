# Nexus V1 Moddability Audit — Source-Locked

## Summary
Nexus is a sealed-source, Saturn-exclusive title with no public disassembly,
no modding community, and a custom big-endian asset format. Modding potential
is extremely limited without the original disc image and disassembly.

## 1. Can Nexus Be Modded?

Short answer: Not practically, at this time.

Current boundaries:
- No source code -- not open-source, no ReDMCSB-style decompilation exists
- The authenticated retail corpus is available outside the repository on the
  external disk; Firestaff reads it without repacking the game files
- No source-locked event VM -- SLEV/SDDRVS profiling exists, but event and
  runtime ownership remain unproven
- Big-endian only -- all data is SH2 big-endian; no off-the-shelf tools apply
- Japanese text -- Shift-JIS encoded; glyph mapping and Saturn placement
  remain capture-gated
- No community -- no known Nexus modding forum, tool, or patch community

What Could Be Modified (Theoretically):
1. Replace creature model files (e.g. ANTMAN.MNS, CHAOS.MNS) with custom DMDF meshes
2. Replace level files with custom dungeon layouts (grid and bounded DGN
   structures are known; Saturn 3D consumer remains unbound)
3. Replace CD audio tracks with custom music (Red Book Audio tracks 2-9)

Unbrick Path: Disc image + disassembly + DGN 3D blob reverse-engineering + SDDRVS.TSK parser.

## 2. Data File Formats

Dungeon Levels (LEV files):
- 16 levels, 147-322 KB each, stored as DMWeb 2048-byte block containers
- Structure1B is a 64x64 grid with 8 bytes per cell; later structures hold
  collision, doors, floor objects, sensors and decorations

Creature Models (DMDF format):
- Magic: 0x444D4446, Big-endian SH2
- Vertex: 16 bytes (int16 x,y,z + nx,ny,nz + uint16 u,v)
- Face: uint16[3] triangle indices
- Textures: embedded VDP1 BITMAP (undocumented)
- nexus_v1_dmdf_load() admits bounded header/vertex/face/material receipts;
  Saturn texture/VDP1 ownership remains capture-gated

Audio/runtime dispatch remains source-gated for SNDLEV00-15.SAL and SLEV00-15.BIN.
SMAP00-15.BIN is a decoded LVMP tilemap/palette/tileset resource; its Saturn
VDP2 HUD placement and explored-state writes are not yet authenticated.

## 3. Editability by File Type

| Asset          | Parsed? | Editable? | Notes                        |
|----------------|---------|-----------|------------------------------|
| LEV DGN grid   | Yes     | Partially | Structure1B/2/3 receipts; VDP1 consumer unbound |
| DMDF MNS models| Partial | No        | Source mesh/material receipts; VDP1 consumer unbound |
| SNDLEV SAL     | Receipt | No       | Codec and playback ABI capture-gated |
| SLEV BIN       | Receipt | No       | Event semantics and dispatch unproven |
| CD audio tracks| No      | Yes       | Red Book Audio               |

## 4. Comparison to DM1 Moddability

| Aspect         | DM1                    | Nexus                  |
|----------------|------------------------|------------------------|
| Disassembly    | ReDMCSB (complete)     | None                   |
| Dungeon format | Fully documented       | Grid/Structure receipts; Saturn consumer unbound |
| Script VM      | Hardwired in C         | SLEV/SDDRVS profiling; event VM unproven |
| Model format   | 2D sprites (doc.)      | DMDF 3D (partial)      |
| Mod community  | Active (CSB, etc.)     | None known             |

## 5. Conclusion

Theoretically moddable but practically limited today. The path to source-
faithful runtime editing requires: (1) an authenticated Saturn execution
capture, (2) SH2/68K owner joins, (3) DGN material/VDP1 binding, and (4)
SLEV/SDDRVS event-ABI evidence. Existing receipts are not runtime parity.
