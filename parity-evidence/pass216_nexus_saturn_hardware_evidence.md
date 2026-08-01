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
