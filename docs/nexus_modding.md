# Nexus V1 Moddability Audit — Source-Locked

## Summary
Nexus is a sealed-source, Saturn-exclusive title with no public disassembly,
no modding community, and a custom big-endian asset format. Modding potential
is extremely limited without the original disc image and disassembly.

## 1. Can Nexus Be Modded?

Short answer: Not practically, at this time.

Obstacles:
- No source code -- not open-source, no ReDMCSB-style decompilation exists
- No disc image -- the repository has no Saturn disc image (critical blocker)
- No script VM -- SDDRVS.TSK (event/AI script interpreter) is unimplemented
- Big-endian only -- all data is SH2 big-endian; no off-the-shelf tools apply
- Japanese text -- Shift-JIS encoded; no translation tooling in tree
- No community -- no known Nexus modding forum, tool, or patch community

What Could Be Modified (Theoretically):
1. Replace creature model files (e.g. ANTMAN.MNS, CHAOS.MNS) with custom DMDF meshes
2. Replace level files with custom dungeon layouts (grid known; 3D blob unknown)
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
- nexus_v1_dmdf_load() scaffolded for header/vertices/faces

Audio (NOT PARSED): SNDLEV00-15.SAL, SLEV00-15.BIN, SMAP00-15.BIN all unknown format.

## 3. Editability by File Type

| Asset          | Parsed? | Editable? | Notes                        |
|----------------|---------|-----------|------------------------------|
| LEV DGN grid   | Yes     | Partially | 3D blob unknown              |
| DMDF MNS models| Partial | No        | Textures not parsed          |
| SNDLEV SAL     | No      | No        | Audio bank format unknown    |
| SLEV BIN       | No      | No        | Script VM not implemented    |
| CD audio tracks| No      | Yes       | Red Book Audio               |

## 4. Comparison to DM1 Moddability

| Aspect         | DM1                    | Nexus                  |
|----------------|------------------------|------------------------|
| Disassembly    | ReDMCSB (complete)     | None                   |
| Dungeon format | Fully documented       | Grid known, 3D unknown |
| Script VM      | Hardwired in C         | SDDRVS.TSK (not impl.) |
| Model format   | 2D sprites (doc.)      | DMDF 3D (partial)      |
| Mod community  | Active (CSB, etc.)     | None known             |

## 5. Conclusion

Theoretically moddable but practically immoddedable today. The path to
modding requires: (1) disc image acquisition, (2) SH2 disassembly, (3)
DGN 3D blob reverse-engineering, (4) SDDRVS.TSK implementation.
