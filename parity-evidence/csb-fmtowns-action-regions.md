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
(0,33) in English or (0,31) in Japanese. ReDMCSB COORD.C:1693–1698 also
defines G2067/G2068 with these offsets. The previous live composition
saved/restored x48..271,y33..168 after HUD painting, intersecting the menu.

The F31 raster pointer, completed-frame hash and HUD save/restore now share
the source origin. Sprite callbacks already receive the viewport-local
pointer and therefore need no second translation. Full-page M648 text uses
the same origin. F31 C080 admission and screen-to-local conversion use its
224x136 rectangle, including Japanese y31–32 and excluding y167–168.
Other platform origins are unchanged.

Original-media tests compare the entire final 224x136 aperture hash with
the runtime receipt, independently using the C696 coordinates in both
languages and all three presentation modes. This proves final composition
and receipt consistency, not emulator raster parity. Authentic C080
edge/pickup/throw/sensor sequences remain a separate verification task.

## Spell-area source layout

An in-memory C696 walk of the same original media confirms C012=(9,2,87,33)
in both languages; C013=(3,12,319,74) EN or (3,12,319,82) JP. All 45
records 220..264 are identical between languages. The 87x8 control strip,
45x8 selected tab, 12x7 other tabs, 13x11 available-symbol cells and
type-10/type-18 text anchors match the late DM1 layout. The Japanese parent
therefore shifts the whole spell surface and its clicks down eight pixels.

C696 SHA-256: EN `2e14e0ff83bf72ecb1290db2976d24c571615822fccb489d003dcae08c674f9f`,
JP `27c24ac5696f070fb4fcd44609f695e70b215d3f3bf1ea6167e4dd7242b7af8c`.
F31 C009 is 87x25 and requires no F20 nine-pixel crop. CASTER.C:75-98
uses C009, controls and glyphs; it does not copy the legacy C011 strips.
SPELDRAW.C:85-95 selects living tabs; MENUDRAW.C F0397/F0398 consumes
SymbolStep and the champion's four-symbol incantation. TEXT.C F0818:93-109
routes names beginning with ESC or containing high bytes to the Japanese
renderer; ASCII rune cells remain on M653. Japanese-name glyph parity and
the CSB cast execution owner remain separate open requirements.

The final-frame tests compare original C009 borders and packed M653 glyph
bits for six available and four selected cells in all three modes/languages.
Public clicks open the panel, enter a rune into the original champion and
recant it. A RAM-only perturbation sets the host caster mirror to -1 and
checks that the entire 87x33 panel remains unchanged; the source party is
not modified by that perturbation. These are raster/input integration
checks, not successful casts or original-emulator timing comparisons.

## Idle action cells

ACTIDRAW.C F0386:247-288 selects the original object icon or C201 for an
empty hand, remaps color 12 to cyan 4 for F31, and places the icon in
C093..C096. C089..C092 are 20x35 EN / 20x62 JP, at x233+22*slot and
y86 EN / y94 JP. The 16x16 icons start at x235+22*slot, y96 EN / y117 JP.
The standalone idle compositor loads native atlases C042..C048 on the
owning cache before taking a fresh source-party mirror. It clears only
C011 and draws after viewport restoration, mutually exclusive with the
active menu. It does not admit the still-incomplete generic HUD.

Original-media tests independently decode the required atlas and compare
every pixel of each eligible unhatched cell in all three modes. The current
original-party route supplies one eligible cell per language/mode (six
cases), not coverage of every possible party configuration. Source
object/action-set APIs supply identity; bitmap decoding, placement and
palette comparison are independent. Public lower-cell-edge clicks select
the original actor and Pass closes without changing the leader. No party
or equipment is fabricated. Hatched cells, all click edges/gaps and emulator
pixel/timing comparisons remain outside this evidence.

## Movement controls

The same original C696 records define C009 at bottom-right (319,168) in
English and (319,199) in Japanese. C008 is 87x45 / 87x41 respectively.
MENUDRAW.C F0395:16 places C013 into C009 without transparency: English
87x45 at (233,124), Japanese 96x41 clipped by nine source pixels at
(233,159). The native package-bound IMG2 cache supplies these pixels.

C065/C067 are 28x21 EN or 28x19 JP; C066 is 27 pixels wide. C068/C069
offsets are (1,1)/(58,1), C070 is (30,1). Bottom-row offsets are y23 EN
and y21 JP. Therefore the top click rows start at y125 EN / y160 JP and
the bottom rows at y147 EN / y180 JP. COMMAND.C:397–402 maps them to the
original six movement commands. Japanese input bypasses the earlier PC
geometry resolver, and turn clicks use explicit TURN_LEFT/TURN_RIGHT
tokens accepted by the CSB command bridge.

Original-media tests compare every final C013 pixel and exercise left/right
rotation with unchanged source party position in both languages/all three
modes. Traversal, exact edge/gap coverage and inventory-open controls remain
separate verification work.
