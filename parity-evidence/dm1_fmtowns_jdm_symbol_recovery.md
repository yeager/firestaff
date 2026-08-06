# DM1 FM Towns Japanese `JDM.EXP` — symbol recovery by byte fingerprint

Companion to `parity-evidence/dm1_fmtowns_jdm_structural_map.md`. Where
that document established that JDM.EXP is the *same* Phar Lap-linked
build as EDM.EXP but stripped of the SYM1 name table, this document
delivers the recovered vaddr for each menu-chain symbol we could
positively locate in JDM by matching EDM's byte body.

Executables (extracted, not shipped):

- EDM.EXP: `310 518` bytes, load image at file+0x200, SYM1 with 1174
  entries at 0x46b41..0x4bcf6.
- JDM.EXP: `290 221` bytes, load image at file+0x200, **no SYM1**.

Both files contain complete P3 load images verified with
`validate_p3_header` in `src/dm1/dm1_v1_fmtowns_startup.c`.

## Matching algorithm

For every EDM symbol we want to locate in JDM:

1. Parse EDM's SYM1 table (`name_size:1 | name:name_size | value:4 |
   flags:2`, starting at SYM1+0x22, exactly as `parse_sym1_table`
   reads it). Record every `(name, vaddr)` pair.
2. Sort the recorded vaddrs and take each symbol's body as
   `edm_load[vaddr .. next_vaddr]`; for long code we cap the window
   at a size sufficient to disambiguate (240–592 bytes).
3. Build a per-byte match mask over the body:
   - Bytes 1..4 after any `0xE8` (near-call rel32) or `0xE9` (near-jmp
     rel32) opcode are **wildcarded** — the relative displacement
     changes wherever the target function has moved.
   - Any 4-byte little-endian window whose value falls in the
     data / BSS load-image range `[0x20000, 0x80000]` is **wildcarded**
     — the Japanese build's data segment has different absolute
     addresses (the DYNA_BUTTONS pool alone shifted by +0x228).
4. Choose the **longest solid run** in the mask as the search anchor
   (must be ≥ 6 bytes). Locate every occurrence of that anchor in
   JDM's load image; at each hit, re-check the full masked template.
5. If exactly one JDM offset satisfies the full masked template, that
   is the recovered JDM vaddr; compute
   `jdm_shift = jdm_vaddr − edm_vaddr`. If two or more offsets match,
   the symbol is *not* recovered (we do not guess).

Everything below is the output of that pipeline running against the
two real files. No addresses are inferred or fabricated.

## Recovered symbols

### Menu / drawing / text code

| Symbol            | EDM vaddr | JDM vaddr | Shift    | Body length used |
|-------------------|-----------|-----------|----------|------------------|
| `DRAW_DMENU`      | 0x04620   | 0x046e0   | +0x00c0  | 240              |
| `DRAW_ICN_BUTTON` | 0x044f0   | 0x045b0   | +0x00c0  | 304              |
| `GET_LABEL`       | 0x043e4   | 0x04498   | +0x00b4  | 40               |
| `MOUSE_OFF`       | 0x0dd38   | 0x0ddb0   | +0x0078  | 32               |
| `MOUSE_ON`        | 0x0dd18   | 0x0dd90   | +0x0078  | 32               |
| `GET_SCL_COORD`   | 0x1942c   | 0x194a4   | +0x0078  | 208              |
| `GET_RGN_COORD`   | 0x194fc   | 0x19574   | +0x0078  | 32               |
| `DO_DRAW_CTEXT`   | 0x1a804   | 0x1aacc   | +0x02c8  | 92               |
| `FILL_RECT`       | 0x1fccc   | 0x1febc   | +0x01f0  | 432              |
| `PIX_BLOT`        | 0x1fe7c   | 0x2006c   | +0x01f0  | 592              |

Each row is uniquely determined (exactly one JDM offset satisfies the
masked EDM template).

### TownsOS EGB library trampolines

Every EGB trampoline symbol recovered — the entire library was
relocated as a block. All eight matches use identical shift
`+0x26c`, which independently confirms the "library moved wholesale"
observation from section 5 of the structural map.

| Symbol              | EDM vaddr | JDM vaddr | Shift   |
|---------------------|-----------|-----------|---------|
| `EGB_RESOLUTIONRAM` | 0x40739   | 0x409a5   | +0x026c |
| `EGB_VIEWPORT`      | 0x407a0   | 0x40a0c   | +0x026c |
| `EGB_WRITEPAGE`     | 0x407ec   | 0x40a58   | +0x026c |
| `EGB_COLOR`         | 0x40836   | 0x40aa2   | +0x026c |
| `EGB_WRITEMODE`     | 0x408a5   | 0x40b11   | +0x026c |
| `EGB_PAINTMODE`     | 0x408ed   | 0x40b59   | +0x026c |
| `EGB_PUTBLOCK`      | 0x40bec   | 0x40e58   | +0x026c |
| `EGB_RECTANGLE`     | 0x40ee5   | 0x41151   | +0x026c |

### DYNA_BUTTONS label pool

| Symbol         | EDM vaddr | JDM vaddr | Shift   |
|----------------|-----------|-----------|---------|
| `DYNA_BUTTONS` | 0x24194   | 0x243bc   | +0x0228 |

The JDM vaddr was originally identified in
`parity-evidence/dm1_fmtowns_jdm_structural_map.md` §3. This document
re-verifies it: the byte pattern at JDM 0x243bc is
`4e 00 82 b3 82 a6 82 ac 82 e9 00 92 40 82 ab 90 d8 82 e9 00 58 00 …`
— i.e. the ASCII `"N"` sentinel followed by the NUL-terminated
Shift-JIS labels `さえぎる`, `叩き切る`, `X`, …, in exact 1:1 order
with EDM's English pool. Independently, DRAW_DMENU at JDM 0x046e0
contains seven LE32 absolute references into the 0x24380..0x243b7
window — the same shape of pool-relative accesses that EDM's
DRAW_DMENU makes into 0x24194..0x24194+300.

## Uniform-shift verdict

The +0x228 data-segment shift asserted in the structural map holds
for the DYNA_BUTTONS pool only. Across the recovered code symbols,
the shift is **not uniform**:

| Region                     | Observed JDM shift |
|----------------------------|--------------------|
| DYNA_BUTTONS data pool     | +0x0228            |
| GET_LABEL (0x04398 area)   | +0x00b4            |
| DRAW_ICN_BUTTON / DRAW_DMENU (0x044xx–0x046xx) | +0x00c0 |
| MOUSE_ON / MOUSE_OFF       | +0x0078            |
| GET_SCL_COORD / GET_RGN_COORD | +0x0078         |
| DO_DRAW_CTEXT              | +0x02c8            |
| FILL_RECT / PIX_BLOT       | +0x01f0            |
| EGB_* trampoline library   | +0x026c (uniform)  |

The pattern is consistent with the Phar Lap linker inserting extra
data bytes (Shift-JIS strings) into localised object files while
leaving other objects untouched, then re-linking. Each object file's
code moves by the cumulative growth of every prior object; the EGB
library, linked as a single block from a Metaware-supplied `.OBJ`,
moves as one unit and therefore has one shift. Menu-drawing objects
that link before/after Japanese-text objects pick up smaller,
locale-specific offsets.

## Symbols we could NOT recover (and why)

- **`INIT_TEXT`**: the EDM body at 0x1ae54 contains multiple
  absolute pointers into the localised text data segment plus two
  E8 rel32 calls that resolve to different-shift targets. After
  masking, no solid run of ≥ 6 bytes has a unique JDM occurrence.
- **`SPC_BLOT`**: the masked template matches at two distinct JDM
  offsets (0x1ce54 and 0x1cfb4). Neighbouring code shifts are
  inconsistent (+0x2c8 for the CTEXT block, +0x1f0 for the FILL/PIX
  block), so we cannot pick between the two candidates by
  proximity. Omitted rather than guessed.
- **`DYNAMENU`, `MENU_ICONS`, `MENU_OWNER`, `REDRAW_MENU`,
  `PARTY_SIZE`, `TEXT_PIC`, `SCR_X_SIZE`, `CHAR_X_SIZE`,
  `CHAR_X_WID`, `CHAR_Y_HYT`, `ICON_X_SIZE`, `ICON_Y_SIZE`**:
  these are 2- or 4-byte scalars / word arrays in BSS. Their bodies
  are too short and their contents are runtime-initialised, so
  there is no distinctive fingerprint to match against. Recovery of
  these requires XREF triangulation from the recovered code symbols
  above (out of scope for this pass).

## Notes on method

- Header/SYM1 parse: `struct.unpack_from` at the exact offsets used
  by `validate_p3_header` and `parse_sym1_table` in
  `src/dm1/dm1_v1_fmtowns_startup.c`.
- Byte scan uses only Python `bytes.find`; no disassembler is on the
  path. Wildcard categories are limited to (a) rel32 follower bytes
  after `E8`/`E9` and (b) LE32 windows in the data range.
- All recovered vaddrs above have been re-checked by reading the
  exact 32–592 bytes at each JDM offset and confirming that every
  non-wildcard byte matches the EDM template.
