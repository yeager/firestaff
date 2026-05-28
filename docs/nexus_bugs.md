# Nexus V1 Known Bugs and Quirks — Source-Locked

## Summary
Nexus carries forward DM1 bugs (BUG0 series) since it re-implements DM1
game logic. There are also Nexus-specific gaps and known issues in the
Firestaff codebase. The original Saturn binary has not been disassembled,
so some "bugs" here may be Firestaff implementation gaps rather than
bugs in the original game.

## 1. BUG0 Issues (DM1 Bugs Inherited by Nexus)

From nexus_regression.md, these DM1 bugs are NOT FIXED in Nexus:

Critical:
| ID          | Bug                        | Severity  |
|-------------|----------------------------|-----------|
| BUG0_02     | Timeline 24-bit overflow   | CRITICAL  |

Minor/Low:
| ID          | Bug                         | Severity  |
|-------------|-----------------------------|-----------|
| BUG0_03     | VBlank timing glitch        | MINOR     |
| BUG0_04     | Creature colors             | LOW       |
| BUG0_05     | Portrait sensor z-order     | LOW       |
| BUG0_06     | Projectile blit left edge    | LOW       |
| BUG0_07     | Explosion blit left edge     | LOW       |
| BUG0_64     | Floor ornaments over pits   | LOW       |
| BUG0_83     | Thieves Eye hole animation   | MEDIUM    |
| BUG0_86     | Champion portrait graphics   | MEDIUM    |

BUG0_02 is the most severe: game hangs after approximately 850 hours
of continuous play due to a 24-bit timeline counter overflow.

## 2. Firestaff Implementation Gaps (Not Original Bugs)

Critical blockers (Firestaff-specific):
- B1: No Sega Saturn disc image present in repository
- B2: Nexus static library not linked into any binary (no --profile nexus)
- B3: Zero tests for Nexus (387 DM1 tests exist, 0 for Nexus)
- B4: No game loop integration (nexus_v1_engine.c isolated dead code)

High-priority missing implementations:
- M1: VDP1/VDP2 texture format not implemented
- M2: DMDF model format not fully documented
- M3: ISO 9660 + Saturn header parser incomplete/untested
- M4: Shift-JIS text decoding not implemented

## 3. Design Quirks (Not Bugs)

Japanese Language Only:
- All champion names are Japanese (Syra, Leyla, Nabi, etc.)
- All UI text in Japanese (Shift-JIS encoding)
- No language option -- cannot play in English

16 Levels (vs DM1 10):
- Nexus extends the dungeon to 16 levels (LEV00-LEV15)
- This is an intentional expansion, not a bug

First-Person 3D (vs DM1 2D):
- DM1 rendered as 2D sprite dungeon
- Nexus renders as full 3D polygon first-person view
- This is a deliberate redesign, not a bug

CD Audio Requirement:
- DM1 had PC speaker/AdLib audio
- Nexus streams CD audio (Red Book Audio tracks 2-9)
- Requires disc in drive or proper disc image mount

DMDF 3D Models (vs DM1 Sprites):
- DM1 creatures were 2D sprite images
- Nexus renders 3D polygon models from .MNS files
- Models are entirely new assets, not derived from DM1 graphics

## 4. Technical Quirks in Source

Big-Endian On-Disc Data:
- All Nexus data files are big-endian (SH2 Saturn byte order)
- Multi-byte values read via rb16()/rb32() byte-swapping
- All PC builds (x86/ARM) are little-endian

No Quaternion Support:
- Math subsystem uses mat4 for all rotations, no quaternions
- Fine for 90-degree dungeon turns
- Would need extension for smooth free-look or arbitrary camera angles

320x224 vs 320x200:
- Saturn native resolution: 320x224 (NTSC) or 320x240 (PAL)
- Firestaff software rasterizer targets 320x200
- Mild aspect-ratio quirk compared to original

Fixed 64x64 grid:
- DMWeb describes Nexus DGN Structure1B as a fixed 64x64 cell grid
- Each cell is 8 bytes; collision and door presence are packed into the cell data

## 5. Bug Comparison: Nexus vs DM1

| Category             | DM1              | Nexus                     |
|----------------------|------------------|---------------------------|
| Known game bugs      | BUG0_02-86 doc.  | Same bugs (inherited)    |
| Disassembly          | ReDMCSB (complete)| None                      |
| Bug-fixable          | Yes (patch C)    | No (no disassembly)       |
| Implementation gaps  | N/A              | Many (Firestaff)          |
| Binary analysis needed| No            | Yes (for original bugs)   |

## 6. Most Significant Issues

For Original Nexus Game:
1. BUG0_02 (Timeline overflow): Game hang after ~850 hours
2. BUG0_83 (Thieves Eye animation): Visual glitch
3. BUG0_86 (Champion portrait graphics): Memory issue

For Firestaff Implementation:
1. No disc image: Cannot verify anything without it
2. DGN 3D blob unparsed: Cannot render full dungeon
3. SDDRVS.TSK not implemented: No event/door/trap scripts
4. Audio not parsed: No sound or music in Firestaff
