# Pass 216 — Nexus Saturn Hardware Evidence (SH-2 Binary Analysis)

## Method

Static analysis of DM.BIN (555,144 bytes) SH-2 code. All addresses are
big-endian SH-2 instructions decoded from the binary at load address
0x06004000. Register values are resolved through MOV.L @(disp,PC),Rn
literal pool references.

## VDP1 Register Initialization

Code at 0x060813B8-0x060813E8 sets all VDP1 control registers:

| Register | Address | Value | Meaning |
|----------|---------|-------|---------|
| PTMR | 0x25D00004 | 0x0000 | Manual plot trigger |
| FBCR | 0x25D00002 | 0x0002 | Manual frame buffer change |
| EWDR | 0x25D00006 | 0x8000 | Erase to black (priority=1) |
| EWLR | 0x25D00008 | 0x0000 | Erase from (0,0) |
| EWRR | 0x25D0000A | 0xFFFF | Erase full frame |
| TVMR | 0x25D00000 | R4 (dynamic) | TV mode flags |
| MODR | 0x25D00010 | (set at 0x0607B0B6) | Mode register |

VDP1 VRAM base (0x25C00000) stored to state struct at 0x060813E8.
Command table resides at VDP1 VRAM offset 0, standard 32-byte entries.

## VDP1 Command Table Format

Standard Saturn VDP1 32-byte command structure confirmed by code at
0x0608141C which writes CMDCTRL to @R1 (0x25D00000 shadow) and reads
CMDSRCA, CMDSIZE, CMDCOLR from the state struct at offsets derived
from the command index.

| Word | Name | Bits | Description |
|------|------|------|-------------|
| 0 | CMDCTRL | 15-13=type, 5-3=color mode | Command control |
| 1 | CMDLINK | 15-0 | Link/jump address |
| 2 | CMDPMOD | 15-0 | Draw mode |
| 3 | CMDCOLR | 15-0 | Color bank/LUT address |
| 4 | CMDSRCA | 15-0 | Texture address (÷8) |
| 5 | CMDSIZE | 13-8=W÷8, 7-0=H | Character size |
| 6-7 | CMDXA/YA | signed 16-bit | Vertex A |

Color modes (bits 5-3 of CMDCTRL):
0=16-color LUT, 1=16-color bank, 2=64-color bank,
3=128-color bank, 4=256-color bank, 5=32768-color RGB.

## VDP2 Register Evidence

| Register | Address | Code Location | Purpose |
|----------|---------|---------------|---------|
| TVMD | 0x25F00000 | 0x0607994C | Display enable, resolution |
| CHCTLA | 0x25F00006 | 0x0602CEF0 | Character format (dynamic) |
| MPOFN | 0x25F00018 | 0x0602CE78 | Map offset |
| CLOFEN | 0x25F000A0 | 0x06032BD0 | Color offset enable |
| COAR | 0x25F00100 | 0x0602CE98 | Color offset A red |
| COAG | 0x25F00102 | 0x0602CEAA | Color offset A green |
| COAB | 0x25F00104 | 0x0602CEB8 | Color offset A blue |
| COBR | 0x25F00106 | 0x0602CED8 | Color offset B red |

## VDP2 CRAM Palette Upload

Code at 0x06079060: R3=0x25F80000 (CRAM base), R7=#8, loop writes 8
palette entries (16 bytes) per iteration from the game's palette table
at 0x06095274. Palette format: 16-bit BGR555 per entry.

## SCSP Sound Interface

Code at 0x0608752A: R2=0x25B00400 (SCSP register base). The SH-2 CPU
communicates with the MC68EC000 sound CPU through SCSP mailbox registers.
SDDRVS.TSK (26,610 bytes) runs on the sound CPU.

Error strings from sndlib2.c confirm the ABI:
- `submitPCMP` — primary PCM submission function
- `pcmtype = %x` — PCM format diagnostic
- `playing %s` — SAL file playback with filename argument
- `cannot open sound file %s` — file-based SAL loading

Sound bank filenames: SNDLEV01-SNDLEV15 at DM.BIN offset 0x038F34,
12 bytes per entry (8-char name + 4 bytes padding/ext).

## FONT012 Resource Evidence

RLOWFIX.BIN RES* archive contains three FONT resources:

| Resource | Glyphs | Size | BPP | Palette |
|----------|--------|------|-----|---------|
| FONT0 | 291 (0x123) | 6×12 | 2 | FFFF/DEF7/B9CE/8000 |
| FONT1 | 250 (0x0FA) | 12×12 | 2 | FFFF/DEF7/B9CE/8000 |
| FONT2 | 710 (0x2C6) | 12×12 | 2 | FFFF/DEF7/B9CE/8000 |

2bpp format: each row is (width*2/8) bytes, MSB-first, 2 bits per
pixel. Glyph data follows immediately after the 36-byte FONT header
(4+4+4+4+2+2+2+2+2+2+8).

DM.BIN resource pointers at 0x012880: three RAM addresses
(0x060795B4, 0x06046A94, 0x06024084) receive the parsed font data
at runtime.

## TEXT/TABL Character Encoding

RLOWFIX.BIN TEXT resources use TABL-indexed encoding:
- Bytes 0x00-0xDF: direct character codes
- Bytes 0xE0-0xE4: multi-byte TABL reference prefix (followed by index)
- TABL has 216 entries mapping to Shift-JIS character codes
- 0x03: newline separator within strings

## Original Source File Names (IWA Module)

DM.BIN contains debug/assert strings identifying the original Nexus source
files. The `iwa\` prefix is the game logic module (likely "Iwamoto" or
similar developer namespace):

| Offset | Address | Source File | Purpose |
|--------|---------|-------------|---------|
| 0x0365C4 | 0x0603A5C4 | iwa\..\yam\dminc.c | DM include/common |
| 0x03694C | 0x0603A94C | iwa\map3d3.c | 3D map rendering |
| 0x03695C | 0x0603A95C | iwa\map2d.c | 2D automap |
| 0x036968 | 0x0603A968 | iwa\tlist.c | Thing list (items) |
| 0x036974 | 0x0603A974 | iwa\clist.c | Creature list |
| 0x036980 | 0x0603A980 | iwa\loader.c | File loader (ITEM.IBS) |
| 0x0369A8 | 0x0603A9A8 | iwa\movbody.c | Party movement |
| 0x0369B8 | 0x0603A9B8 | iwa\movobj.c | Object/projectile movement |
| 0x0369C8 | 0x0603A9C8 | iwa\mapcalc.c | Map calculations |
| 0x0369D8 | 0x0603A9D8 | iwa\swctrl.c | Switch/sensor control |
| 0x0369E8 | 0x0603A9E8 | iwa\animtask.c | Animation task scheduler |
| 0x0369F8 | 0x0603A9F8 | iwa\projctl.c | Projectile control |
| 0x036A08 | 0x0603AA08 | iwa\dmsound.c | Sound event dispatch |
| 0x036A28 | 0x0603AA28 | iwa\overlay.c | Screen overlay |
| 0x036A44 | 0x0603AA44 | iwa\camera.c | Viewport camera |
| 0x036A54 | 0x0603AA54 | iwa\itline.c | Item info line |
| 0x036A64 | 0x0603AA64 | iwa\scripter.c | Event scripting engine |
| 0x036A74 | 0x0603AA74 | iwa\mapinit.c | Map initialization |
| 0x036A84 | 0x0603AA84 | iwa\bkupctrl.c | Save/backup control |

## Sound System Architecture

The sndlib2 code block spans 0x06087500-0x06088466 (~4 KB). Key findings:

- Sound state struct at RAM 0x06097368 (25 code references across 18 functions)
- SCSP communication via registers at 0x25B00400 (mailbox) and 0x25A00400 (SCSP)
- Sound CPU RAM base at 0x25A00000
- Two BSR calls at 0x060883B4 and 0x0608844A (sound submission subroutines)
- File extensions ".MAP" at 0x038EA0 and ".SAL" at 0x038F00 used to construct
  level filenames by appending to SNDLEV## base names
- GFS_SBL Version 2.10 (1996-02-01) CD filesystem library at 0x087754
- Game-level sound dispatch is in `iwa\dmsound.c`; the event→selector mapping
  is distributed across game logic call sites, not a single lookup table

## ITEM.IBS Field Analysis (243 Items, 40-byte Records)

Data-pattern analysis of all 243 items from ITEM.IBS. File loaded by
`iwa\loader.c`. Item access code uses `iwa\tlist.c` and `iwa\itline.c`.

| Byte | Proven Name | Range | Nonzero | Pattern |
|------|-------------|-------|---------|---------|
| 0 | item_index | 0-242 | 243/243 | Sequential ID |
| 1 | category | 0-6 | 184/243 | Item type enum |
| 2 | carry_locations | 0x00-0x3F | 129/243 | Bitmask |
| 3 | flags | 0x00-0x82 | 213/243 | Bit7=floor_image |
| 4 | (unproven) | 10-64 | 243/243 | Always nonzero, 22 unique values |
| 5 | (unproven) | 5-60 | 243/243 | Always nonzero, 22 unique values |
| 6 | skill1 | 0-19 | 175/243 | Skill index |
| 7 | skill2 | 0-18 | 175/243 | Skill index |
| 8 | weight | 0-110 | 198/243 | Item weight |
| 9 | (unproven) | 0-40 | 49/243 | Weapons only → combat stat |
| 10 | (unproven) | 0-45 | 64/243 | Mostly weapons → combat stat |
| 11 | (unproven) | 0-75 | 27/243 | Weapons only → combat stat |
| 12 | (unproven) | 0-60 | 62/243 | Mostly weapons → combat stat |
| 13 | (unproven) | 0-252 | 60/243 | Signed delta, high vals 236-251 |
| 14 | (unproven) | 0-135 | 44/243 | Bit7=flag, lower=value |
| 15 | (unproven) | 0-134 | 34/243 | Bit7=flag, lower=value |
| 16 | action1 | 0-41 | 197/243 | Action ID |
| 17 | action2 | 0-40 | 116/243 | Action ID |
| 18 | action3 | 0-42 | 84/243 | Action ID |
| 19 | (unproven) | 0-83 | 229/243 | 13 unique, broadly used |
| 20-21 | inventory_image_index | — | — | BE16 |
| 22-23 | floor_image_index | — | — | BE16 |
| 24-25 | name_string | — | — | BE16 |
| 26-27 | desc_string | — | — | BE16 |
| 28-29 | action1_string | — | — | BE16 |
| 30-31 | action2_string | — | — | BE16 |
| 32-33 | action3_string | — | — | BE16 |
| 34-35 | word34 | 0x0000 | 0/243 | Always zero (reserved) |
| 36-37 | attribute | 0x0000-0x6400 | 124/243 | Item attribute word |
| 38 | (unproven) | 0-192 | 83/243 | 20 unique values |
| 39 | (unproven) | 0-194 | 44/243 | 8 unique, bit7 common |

Bytes 4-5: b4==b5 in 151/243 items (all potions/misc); weapons differ.
Bytes 9-15: nonzero almost exclusively for cat=0 (Weapon) and cat=6 (Armor).
These are combat stats but their exact DM-engine semantics (attack strength,
defense, damage factor, etc.) cannot be proven without tracing the SH-2 code
that reads individual byte offsets from the item record — no such code
references were found via literal pool analysis.
