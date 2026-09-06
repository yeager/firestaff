# CSB FM Towns action pointer regions

## Original-media evidence

Read on 2026-09-06 from the user's original
`Dungeon-Master-Chaos-Strikes-Back-Expansion-Set-1_FM-Towns_JA-EN.zip`.
The CUE declares track 1 as MODE1/2352. Only its first 4,500 sectors were
read into RAM; the ISO9660 payload starts 16 bytes into each sector.
No archive member was extracted to disk.

Selected GRAPHICS.DAT SHA-256:

- CDATA: `08cceb0c7003da3f286dc5805748f7e391a65f2ad85099c3d9adb49afc4cc723`
- CJDATA: `f8029e3d75f0d1ee931b0b6de41c7bd3dc795ad8971855a0219683cf93546431`

Both containers contain a raw 9,160-byte C696 record, marker `0xfc0d`.
Its little-endian range table and eight-byte `(type,parent,data1,data2)`
records establish the following graph. Coordinates below are inclusive.

| Region | English | Japanese |
| --- | --- | --- |
| C010 size | 87x45 | 87x72 |
| C011 top-right anchor | (319,77) | (319,85) |
| C081 row size | 85x11 | 85x20 |
| C082 offset | (1,9) | (1,9) |
| C083 offset | (1,21) | (1,30) |
| C084 offset | (1,33) | (1,51) |
| C097 Pass size | 35x7 | 25x7 |
| C098 top-right offset | (86,0) | (86,0) |
| First row screen rectangle | x234..318, y86..96 | x234..318, y94..113 |
| Second row screen rectangle | x234..318, y98..108 | x234..318, y115..134 |
| Third row screen rectangle | x234..318, y110..120 | x234..318, y136..155 |
| Pass screen rectangle | x285..319, y77..83 | x295..319, y85..91 |

ReDMCSB `COMMAND.C:461–465` binds Pass and action rows to C098 and
C082..C084. `COORD.C` F0635 resolves their parent rectangles. C085..C087
are text anchors, not click rectangles. The previous x232..318/y77+7*n
approximation dispatched actions in the name band and ignored Japanese
row geometry.

## Scope

The native pointer route now selects the English/Japanese F31 profile and
uses these rectangles without requiring a DM1 startup receipt. Original
MINI.DAT handoff tests exercise name-band rejection, both inclusive Pass
corners without stamina/leader changes, and first-row dispatch.
This is source-data/input evidence, not full menu pixel or emulator parity.
Japanese glyphs, all row boundaries, and complete visual composition still
require independent verification.

## Additional HUD blockers confirmed from the same media

The original graphic dimension table gives C009=87x25 and C011=14x39 in
both languages. C010 is EN87x45/JP96x72; C013 is EN87x45/JP96x41. The
generic CSB HUD admission requires C011=14x26 and PC-sized Japanese panels,
so it rejects these authentic records before rendering its action overlay.

C696 C003 is 224x136, C004 is its (0,0) anchor, and C007 has offset
(0,33) in English or (0,31) in Japanese. The live composition currently
saves/restores x48..271,y33..168 after HUD painting. That restore intersects
the action menu's x233..319 region. Correcting the whole viewport requires
auditing the raster, sprite and input consumers together, not just changing
one save/restore rectangle. A final active-menu source pass can independently
preserve the menu's verified region while that broader gap remains open.
