# Nexus V1 Known Bugs and Quirks — Source-Locked

> **Historical snapshot — not current status.** This page contains legacy
> issue assumptions from before the real European retail corpus and the
> current source-bound Nexus runtime gates. It must not be used to claim that
> the present tree lacks tests, disc data, DGN/PRS3 parsing or launcher
> integration. See [`docs/NEXUS_STRICT_FIDELITY_INVENTORY.md`](NEXUS_STRICT_FIDELITY_INVENTORY.md)
> and [`docs/NEXUS_STALE_CLAIM_AUDIT.md`](NEXUS_STALE_CLAIM_AUDIT.md).

## Summary
This page separates historical DM1 bug references from current Nexus gates.
Nexus has an authenticated external corpus, a launcher/runtime boundary and a
304/304 passing external-data Nexus CTest selection (with capture-gated tests
reported as skips). The original Saturn executable is
partly disassembled, but start pose, VDP1/VDP2 consumers, SLEV/SAL runtime
ownership and Saturn saves remain unproven. No DM1 bug is automatically
promoted to a Nexus bug.

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

Historical blockers B1-B4 are obsolete. Current Firestaff-specific gates are
the authentic Saturn LEV01 start pose, VDP1/VDP2 presentation consumers,
SLEV/SAL event/audio ownership and Saturn memory-card save compatibility.

High-priority remaining production gates:
- M1: VDP1/VDP2 source formats and bounded capture replay are implemented;
  authenticated Saturn scene ownership, transform/culling and production
  pixel consumers remain open.
- M2: DMDF/MNS model structures and real material payloads are parsed;
  face selection, transform/culling and production raster ownership remain
  capture-gated.
- M3: ISO/CUE boundary and sector reads are verified; Saturn startup-to-LEV01
  identity and the real start pose remain unbound.
- M4: FONT256/S2D glyph data is decoded; the Saturn text/page consumer and
  menu composition remain capture-gated.

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
1. Retail extracted/ISO data is now available for the verified Nexus corpus;
   runtime presentation still requires source-owned Saturn capture.
2. DGN Structure1B/2/3 source records have bounded parsers and receipts, but
   the VDP1 material/command consumer is not yet bound, so full rendering is
   correctly blocked.
3. SDDRVS.TSK and SLEV banks have static/corridor receipts; event ownership
   and door/trap dispatch remain unproven.
4. SAL/MAP bytes are profiled, but codec, SCSP voice ABI and host playback
   remain capture-gated.
