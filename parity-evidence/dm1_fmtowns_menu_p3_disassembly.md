# DM1 FM Towns menu — P3 executable disassembly evidence

Source: HMA-240 English disc `EDM.EXP` (Phar Lap P3 level 1), extracted
from `~/.firestaff/data/dm1/Dungeon Master (Japan) (En,Ja) (Rev 1).7z`,
Track 01 (MODE1/2048 ISO 9660 volume "DUNGEON"), file
`/DUNGEON/EDM.EXP`, 310518 bytes.

Load image begins at file offset `0x200`; SYM1 symbol table at
`0x46b41` (1174 entries). All virtual addresses below are load-image
virtual addresses (as stored in the SYM1 table). File offset = `0x200 +
vaddr`.

Disassembly is `capstone` i386 protected-mode; cross-references
resolved against the SYM1 name table.

## Symbol coordinates

| Symbol         | Vaddr      | Kind     | Notes                          |
|----------------|------------|----------|--------------------------------|
| DRAW_DMENU     | `0x4620`   | function | 240 bytes                      |
| DRAW_ICN_BUTTON| `0x44f0`   | function | 304 bytes                      |
| GET_LABEL      | `0x43e4`   | function | 40 bytes                       |
| MOUSE_OFF      | `0xdd38`   | function | 32 bytes, reference-counted    |
| MOUSE_ON       | `0xdd18`   | function | 32 bytes, reference-counted    |
| SPC_BLOT       | `0x1ccec`  | function | 140 bytes                      |
| FILL_CSCREEN   | `0x1f448`  | function | 48 bytes                       |
| MENU_OWNER     | `0x24156`  | data     | word — active menu record id   |
| NUM_DYNABTNS   | `0x24158`  | data     | word                           |
| REDRAW_MENU    | `0x2415a`  | data     | word — cleared by DRAW_DMENU   |
| MENU_ICONS     | `0x2415c`  | data     | word — nonzero => icon mode    |
| ARMR_OPTS      | `0x24160`  | data     |                                |
| DYNAMENU       | `0x2418c`  | data     | 8-byte record                  |
| DYNA_BUTTONS   | `0x24194`  | data     | NUL-separated label pool       |
| PLAYER         | `0x26158`  | data     | player records (stride 319)    |
| SCR_X_SIZE     | `0x26c68`  | data     | word                           |
| ICON_SIZE      | `0x26c76`  | data     | word                           |
| ICON_X_SIZE    | `0x26c78`  | data     | word                           |
| ICON_Y_SIZE    | `0x26c7a`  | data     | word                           |
| PARTY_SIZE     | `0x29424`  | data     | word — number of icons drawn   |
| PICKING_CHARACTER | `0x29418` | data   | word                           |
| PARTY_RESTING  | `0x2941a`  | data     | word                           |
| SCREEN         | `0x31290`  | data     | dword — screen descriptor ptr  |
| MSE_STATE      | `0x25848`  | data     | word — mouse-hide depth        |

DYNAMENU is only 8 bytes and immediately followed by DYNA_BUTTONS.
Byte offsets used by DRAW_DMENU inside DYNAMENU: `+1..+3` are three
label indices; `+2` and `+3` may hold `0xFF` sentinels that override
the panel colour (see below).

## DYNA_BUTTONS content (first 96 bytes)

Verified NUL-separated label pool starting at `0x24194`:

```
"N", "BLOCK", "CHOP", "X", "BLOW HORN", "FLIP", "PUNCH", "KICK",
"WAR CRY", "STAB", "CLIMB DOWN", "FREEZE LIFE", "HIT", "SWING",
"STAB", "THRU", ...
```

Index 0 is the single-char `"N"` (a placeholder / empty-slot glyph).
GET_LABEL treats input index `0xFF` as a special sentinel and returns a
fixed pointer (`0x21d9c`, presumably the blank label).

## DRAW_DMENU behaviour (verified)

```
push edi
call MOUSE_OFF                    ; hide mouse cursor
push 0 ; push 0xB                 ; region 11 (main menu area)
call FILL_CSCREEN                 ; clear region to colour 0

if MENU_ICONS != 0:               ; ICON mode
    for i = 0; i < PARTY_SIZE; i++:
        push i
        call DRAW_ICN_BUTTON      ; draw one champion icon
    goto done

if MENU_OWNER == 0: goto done     ; DYNAMIC mode
di = 0x0B                          ; default panel colour
if DYNAMENU[+3] != 0xFF: di = 0x4D
if DYNAMENU[+2] != 0xFF: di = 0x4F
push -1 ; push di ; push 0xA      ; region 10 = panel
call SPC_BLOT                     ; draw coloured panel

; Draw main label:
;   record ptr = 0x26019 + MENU_OWNER * 319
;   (5*MENU_OWNER shifted <<6 minus MENU_OWNER = MENU_OWNER * 319)
push 7                             ; text row height
lea edx, [MENU_OWNER * 319 + 0x26019]
push edx                           ; text pointer
push 4                             ; foreground colour
push 0                             ; background colour
push 0x50                          ; text x = 80
push SCR_X_SIZE                    ; screen stride
push SCREEN                        ; screen descriptor
call DO_FDRAW_CTEXT

; Draw three button labels:
for i = 0; i < 3; i++:
    push 0xC                       ; row height 12
    push DYNAMENU[+1 + i]          ; label index
    call GET_LABEL                 ; -> string ptr
    push eax                       ; string
    push 0                         ; bg
    push 4                         ; fg
    push (i + 0x55)                ; y = 85 + i (rows 85, 86, 87 in
                                   ; whatever unit — see notes)
    push SCR_X_SIZE
    push SCREEN
    call DO_FDRAW_CTEXT

done:
call MOUSE_ON
REDRAW_MENU = 0
pop edi ; ret
```

The three-slot dynamic menu therefore corresponds exactly to the
original DM1 action-menu layout (title + three verb buttons).

## GET_LABEL (0x43e4) verified

Walks `DYNA_BUTTONS` as a NUL-terminated string table:

```
cl = arg[0]
if cl == 0xFF: return 0x21d9c         ; blank/sentinel label
edx = 0x24194                         ; DYNA_BUTTONS base
while cl > 0:
    while *edx++ != 0: ;              ; skip current label
    cl--
return edx                            ; ptr to cl-th string
```

Index 0 returns the first label ("N"), index 1 returns "BLOCK", etc.

## MOUSE_OFF / MOUSE_ON (0xdd38 / 0xdd18) verified

Both routines are `cli`-guarded reference-counted hide/show wrappers
around `MOS_DISP` at `0x21a40`:

```
MOUSE_OFF:  cli; ax = MSE_STATE; MSE_STATE++; if ax == 0: MOS_DISP(0); sti; ret
MOUSE_ON:   cli; ax = MSE_STATE; MSE_STATE--; if ax-1 == 0: MOS_DISP(1); sti; ret
```

## Helper functions to decode next

DRAW_DMENU's remaining unresolved dependencies, in call order:

- `FILL_CSCREEN` (0x1f448) — resolves region id to screen coords via
  `GET_RGN_COORD` (0x194fc) and calls `FILL_RECT` (0x1fccc). The region
  table itself lives inside GET_RGN_COORD's data references.
- `SPC_BLOT` (0x1ccec) — resolves a bitmap location via `GET_PICLOC`
  (0x18ca4), then calls `GET_COORD` (0x18df0) and `PIX_BLOT` (0x1fe7c).
- `DO_FDRAW_CTEXT` (0x1a8c0) — text rasteriser. Not yet decoded.
- `DRAW_ICN_BUTTON` (0x44f0) — decoded; drives `PUSH_MEM`/`POP_MEM`,
  `OICON_INDEX` (0x2a50), `THING_ICON` (0xb9ac), `LOAD_ICON` (0xbb74),
  `COLOR_BUF` (0x1f3cc), `CHANGE_COLORS` (0x1ce2c), `SC_BLOT` (0x1cc48),
  and `HATCH_CSCREEN` (0x1f478) using PLAYER records at stride 319.
- Region ids `0xA` (SPC_BLOT panel) and `0xB` (main menu area) still
  need their layout captured from `GET_RGN_COORD`'s region table.

Both `SPC_BLOT` and `FILL_CSCREEN` route through `SCREEN` (0x31290),
which is the FM Towns TBIOS screen descriptor.

## Chain-through primitives (also decoded)

### FILL_RECT (0x1fccc, 432 bytes)

Reads a rect descriptor at `[ebp+0xc]` (four words: x1, y1, x2, y2),
snaps width to a 32-pixel row alignment (`(w+0x1f) & ~0x1f`), then
routes to two paths depending on `[ebp+8]` (a write-target buffer, or
zero for the on-screen frame):

- Screen path (`[ebp+8] == 0`):
    - `EGB_WRITEPAGE(WORK, WRITE_PAGE)` sets destination page
    - `EGB_WRITEMODE(WORK, 0)` = plain write
    - `EGB_COLOR(WORK, 2, colour)` = solid-fill colour
    - `EGB_PAINTMODE(WORK, 0x20)` = filled-rectangle mode
    - Populate `EGBPARA[0..3]` with the rect and call `EGB_VIEWPORT`,
      then again for `EGB_RECTANGLE`.
- RAM path (`[ebp+8] != 0`):
    - Same sequence prefixed by `EGB_RESOLUTIONRAM(WORK, 0x80, w, 4,
      h+1, buffer)` which retargets the EGB library at a caller-owned
      RAM raster before drawing.

The whole primitive is therefore a routine wrapper over the
FM Towns TownsOS **EGB** graphics library (published API), not custom
rasterisation — the fill is `EGB_RECTANGLE` after establishing colour,
mode and viewport.

`WORK` at `0x318d8` is the persistent EGB work area (the standard
1500-byte block every EGB caller passes); `EGBPARA` at `0x360d8` is
the 4-word rect parameter buffer; `WRITE_PAGE` at `0x36170` is the
runtime-selected VRAM page number.

### PIX_BLOT (0x1fe7c, 512 bytes)

Same envelope structure as FILL_RECT but the final call is
`EGB_PUTBLOCK` — a source-block copy that reads pixels from the caller's
bitmap at `EGBPARA[0]` = base + row*(width>>1) + (x>>1). Handles both
on-screen and off-screen destinations symmetrically. Colour argument
`>= 0` picks colour and `EGB_WRITEMODE 6` (masked); `< 0` uses
`EGB_WRITEMODE 0` (plain copy).

### GET_RGN_COORD (0x194fc)

Tail-call wrapper: `GET_SCL_COORD(region_id, out, 0x2710, 0x2710)`
(constant 10000 for both scale axes = 1:1 unscaled resolution). The
region table itself lives inside `GET_SCL_COORD` at 0x1942c and is the
next artefact to lift.

### DO_FDRAW_CTEXT (0x1a8c0, 92 bytes)

Formatted-centered-text shim:
- Copies the C-string at arg[0x1c] into a stack buffer up to its NUL,
  padding with `0x20` up to the requested column count arg[0x20], then
  NUL-terminates.
- Tail-calls `DO_DRAW_CTEXT` (0x1a804) with the padded buffer and the
  remaining args (`SCREEN`, `SCR_X_SIZE`, `y`, `x`, `colour`).

`DO_DRAW_CTEXT` itself is the next unresolved rasteriser to decode.

### GET_PICLOC (0x18ca4)

Picture-location descriptor lookup at table base `0x26cac`, stride
14 bytes: `{ byte kind, byte pad, word field04, word field06, word
width, word height, dword data_or_index }`. `kind == 1` decodes the
graphic via `GET_DECODED` (0x9df8); `kind == 2` is a raw pointer
already in memory. Width/height default to the graphic's own
`[data-4]/[data-2]` prefix if the descriptor holds zero.

## TBIOS boundary

Everything below `SPC_BLOT` / `FILL_CSCREEN` funnels into the FM Towns
TownsOS **EGB** library. The concrete primitives observed so far:

| Primitive          | Vaddr    | Purpose                          |
|--------------------|----------|----------------------------------|
| EGB_RESOLUTIONRAM  | 0x40739  | Retarget EGB at a RAM raster     |
| EGB_VIEWPORT       | 0x407a0  | Clip rectangle                   |
| EGB_WRITEPAGE      | 0x407ec  | Select destination VRAM page     |
| EGB_COLOR          | 0x40836  | Set foreground/background colour |
| EGB_WRITEMODE      | 0x408a5  | Set write op (0=copy, 6=masked)  |
| EGB_PAINTMODE      | 0x408ed  | Set fill mode (0x20=solid)       |
| EGB_PUTBLOCK       | 0x40bec  | Copy source raster to viewport   |
| EGB_RECTANGLE      | 0x40ee5  | Filled/outlined rectangle        |

Because these are all documented TownsOS calls with published
semantics, the menu draw does not require reverse-engineering custom
pixel code — it requires wiring a bounded EGB shim (WORK area, EGBPARA
buffer, WRITE_PAGE, VRAM page selection) into the M11 framebuffer.

## Consumption plan

Firestaff still cannot present a real FM Towns menu frame from this
evidence alone. It unblocks the following bounded next steps, in order:

1. Lift `GET_SCL_COORD` (0x1942c) to recover the region table and the
   pixel geometry of regions 10 (SPC_BLOT panel) and 11 (menu area).
2. Provide an EGB shim: WORK area, EGBPARA staging, `EGB_RECTANGLE`
   and `EGB_PUTBLOCK` in software against the M11 framebuffer.
3. Decode `DO_DRAW_CTEXT` at 0x1a804 to recover the font raster and
   the glyph blit chain, then bind the DYNA_BUTTONS label pool to it.

None of these steps invent pixels. Do not use the disassembly here to
synthesise a placeholder menu — only decoded EGB primitives against
real, hash-verified EDM.EXP-owned data may drive the M11 framebuffer.
