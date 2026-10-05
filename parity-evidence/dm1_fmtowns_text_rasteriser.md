# DM1 FM Towns — text rasteriser (`DO_DRAW_CTEXT`) disassembly evidence

Source: HMA-240 English disc `EDM.EXP`, MD5 `c27e7b984df9753912c3375dc121919f`,
310518 bytes. Load image begins at file offset `0x200`. SYM1 symbol table at
`0x46b41`. All vaddrs are load-image virtual addresses (file offset = `0x200 +
vaddr`). Disassembly is `capstone` i386 protected-mode, cross-references
resolved against the SYM1 name table (see parse recipe in
`parity-evidence/dm1_fmtowns_menu_p3_disassembly.md`).

Note: some SYM1 entries in the earlier menu evidence were labelled off by
one entry. The addresses below were re-verified by re-parsing the SYM1 table
and by inspecting the actual opcode bytes.

## Symbol coordinates used by this note

| Symbol / label       | Vaddr     | Kind     | Notes                                        |
|----------------------|-----------|----------|----------------------------------------------|
| DO_FDRAW_CTEXT shim  | `0x1a8c0` | function | space-pads string, tail-calls `DO_DRAW_CTEXT`|
| DO_DRAW_CTEXT        | `0x1a804` | function | the entry decoded here (89 bytes)            |
| `TEXT_SIZE` (`text_measure`) | `0x1a710` | function | walks the string, returns pixel extent |
| `text_place` helper  | `0x18df0` | function | resolves a picture-loc → destination rect    |
| DO_DRAW_TEXT (blit)  | `0x1a664` | function | per-glyph blit loop (168 bytes)              |
| `text_colourise`     | `0x1a5a4` | function | 1bpp → 4bpp coloured font cache              |
| PIX_BLOT             | `0x1fe7c` | function | EGB_PUTBLOCKCOLOR wrapper (from prior evid.) |
| CHAR_X_SIZE          | `0x26c8a` | data w   | glyph width — static default 5               |
| CHAR_Y_SIZE          | `0x26c8c` | data w   | glyph height — static default 6              |
| CHAR_X_SPC           | `0x26c8e` | data w   | horizontal gap — static default 1            |
| CHAR_Y_SPC           | `0x26c90` | data w   | vertical gap — static default 1              |
| CHAR_DESCENDER       | `0x26c92` | data w   | baseline offset — static default 1           |
| CHAR_X_WID           | `0x26c94` | data w   | horizontal pitch — static default 6          |
| CHAR_Y_HYT           | `0x26c96` | data w   | vertical pitch — static default 7            |
| TEXT_PIC             | `0x29344` | data d   | pointer to the 1bpp font raster              |
| SCROLL_BUF           | `0x29348` | data d   | scroll-buffer pointer                        |
| TEXT_BUF             | `0x2934c` | data d   | text-buffer pointer                          |
| CTEXT                | `0x293e4` | data d   | pointer to 4bpp colour cache (3072 bytes)    |
| TEMP_AVAIL           | `0x23c6c` | data d   | temporary-memory bound; unrelated to text    |
| cache-key fg         | `0x293e8` | data w   | last fg colour seen by `text_colourise`      |
| cache-key bg         | `0x293ea` | data w   | last bg colour seen by `text_colourise`      |

The CHAR_* values above are the initialized values in this EDM.EXP image.
`INIT_TEXT` at `0x1ae54` allocates a 768-byte font buffer at `TEXT_PIC`
(`0x29344`) and reads picture-library index 557 into it. `INIT_PC_TEXT` at
`0x1a58c` allocates the 3072-byte colour cache at `CTEXT` (`0x293e4`). Neither
routine changes the CHAR_* metrics. The symbol `TEXT_SIZE` names the helper
function at `0x1a710`, not the font-buffer pointer.

## `DO_DRAW_CTEXT` at 0x1a804 — argument layout

Call signature verified from `DO_FDRAW_CTEXT` (0x1a8c0) — the padding shim
already decoded in the menu evidence file:

```
DO_DRAW_CTEXT(SCREEN,      // [ebp+0x08]  dword  destination descriptor
              stride,      // [ebp+0x0c]  word   destination byte pitch
              x,           // [ebp+0x10]  word   destination X (pixels)
              bg,          // [ebp+0x14]  word   background colour index
              fg,          // [ebp+0x18]  word   foreground colour index
              str_padded)  // [ebp+0x1c]  dword  NUL-terminated ASCII
```

## `DO_DRAW_CTEXT` disassembly (0x1a804 .. 0x1a85d, 89 bytes)

```
0001a804 55                push  ebp
0001a805 8bec              mov   ebp, esp
0001a807 83ec0c            sub   esp, 0xc            ; three local words
0001a80a 53 56 57          push  ebx esi edi
0001a80d 8b7d1c            mov   edi, [ebp+0x1c]     ; edi = str_padded
0001a810 8d5dfc            lea   ebx, [ebp-4]        ; ebx = &measured_h
0001a813 53                push  ebx
0001a814 8d75fe            lea   esi, [ebp-2]        ; esi = &measured_w
0001a817 56                push  esi
0001a818 57                push  edi
0001a819 e8f2feffff        call  TEXT_SIZE(0x1a710)        ; -> width,height
0001a81e 83c40c            add   esp, 0xc
0001a821 6623c0            and   ax, ax
0001a824 7433              je    .return             ; empty / degenerate string

0001a826 53                push  ebx                 ; &measured_h
0001a827 56                push  esi                 ; &measured_w
0001a828 ff7510            push  [ebp+0x10]          ; x
0001a82b 8d5df4            lea   ebx, [ebp-0xc]      ; ebx = &placed_rect (2 words)
0001a82e 53                push  ebx
0001a82f 6a00              push  0
0001a831 e8bae5ffff        call  text_place(0x18df0)       ; fills [ebp-0xc..-6]
0001a836 83c414            add   esp, 0x14
0001a839 23c0              and   eax, eax
0001a83b 741c              je    .return

0001a83d 57                push  edi                 ; str_padded
0001a83e ff7518            push  [ebp+0x18]          ; fg  (passed through unused
0001a841 ff7514            push  [ebp+0x14]          ;  by callee; see below)
0001a844 ff75fa            push  [ebp-6]             ; placed_rect[+6] (dst_y)
0001a847 668b03            mov   ax, [ebx]           ; word at [ebp-0xc] (dst_x)
0001a84a 50                push  eax
0001a84b ff750c            push  [ebp+0x0c]          ; stride
0001a84e ff7508            push  [ebp+0x08]          ; SCREEN
0001a851 e80efeffff        call  DO_DRAW_TEXT(0x1a664)
0001a856 83c41c            add   esp, 0x1c
.return:
0001a859 5f 5e 5b          pop   edi esi ebx
0001a85c c9                leave
0001a85d c3                ret
```

`TEXT_SIZE` returns width in `[ebp-2]` and height in `[ebp-4]` using the
CHAR_* metrics (see next section). `text_place` receives the caller-supplied
`x` and the two measured extents and fills a 6-byte struct at `[ebp-0xc]`:
the low word is the destination top-left X and `[ebp-6]` is the top-left Y
after any clip/centering adjustment. `DO_DRAW_TEXT` is then called with the
resolved destination coordinates.

Note: `bg` (arg+0x14) and `fg` (arg+0x18) are pushed onward but the callee
`DO_DRAW_TEXT (0x1a664)` does **not** consult them directly — colour handling
is done ahead of the blit inside that function via `text_colourise` (see
"Colour handling" below).

## `TEXT_SIZE` at 0x1a710 (verified)

```
edi = str; if edi == 0 return 0
dx  = -CHAR_X_SPC
cx  =  CHAR_Y_SIZE - CHAR_DESCENDER      ; placement height
while (*edi++ != 0)  dx += CHAR_X_WID    ; horizontal pitch per char
if dx <= 0 return 0
*out_w = dx                              ; total drawn width
*out_h = cx                              ; row height
return 1
```

This yields `n * CHAR_X_WID - CHAR_X_SPC` for width and
`CHAR_Y_SIZE - CHAR_DESCENDER` for the placement-height value. The routine
reads the official SYM1 fields; `TEXT_SIZE` itself is the routine name.

## `DO_DRAW_TEXT` (per-glyph blit) at 0x1a664 (168 bytes)

Args as called from `DO_DRAW_CTEXT` (see push list above):

```
[ebp+0x08] SCREEN        [ebp+0x18] bg  (unused here — see note)
[ebp+0x0c] stride        [ebp+0x1c] fg  (unused here — see note)
[ebp+0x10] dst_x         [ebp+0x20] str
[ebp+0x14] dst_y
```

Verified disassembly:

```
0001a664 push ebp; mov ebp,esp; sub esp,8; push ebx esi edi
0001a66d mov  edi, [ebp+0x20]                ; edi = str
0001a670 cmp  byte [edi], 0
0001a673 je   .done                          ; empty string

; --- build destination rect (layout of a PIX_BLOT rect is
;     {x1@0, x2@2, y1@4, y2@6})
0001a679 mov  ax, [ebp+0x10];  mov [ebp-8], ax        ; rect.x1 = dst_x
0001a681 mov  ax, [ebp+0x14];  mov [ebp-2], ax        ; rect.y2 = dst_y
0001a689 mov  ax, CHAR_DESCENDER; add [ebp-2], ax    ; rect.y2 += descender
0001a693 mov  dx, CHAR_Y_SIZE; sub edx, eax           ; height = Y_SIZE - DESCENDER
0001a69c mov  ax, [ebp-2];    sub eax, edx
0001a6a2 mov  [ebp-4], ax                             ; rect.y1 = rect.y2 - edx

; --- ensure the fg/bg-baked font raster exists
0001a6a6 movsx eax, [ebp+0x1c]; push eax             ; fg
0001a6ab movsx eax, [ebp+0x18]; push eax             ; bg
0001a6b0 call  text_colourise(0x1a5a4)
0001a6b5 xchg  ebx, eax                              ; ebx = baked font base

; --- per-glyph loop
.next:
0001a6b9 movzx eax, byte [edi]                       ; eax = current char (0..255)
0001a6bc shl   eax, 3                                ; * 8  (source pitch is 8 pixels/char)
0001a6bf mov   si, 8
0001a6c3 sub   si, CHAR_X_SIZE
0001a6ca add   esi, eax                              ; esi = char*8 + (8 - CHAR_X_SIZE)
                                                     ;      = source X in coloured raster
0001a6cc mov   ax, [ebp-8]
0001a6d0 add   ax, CHAR_X_SIZE
0001a6d7 mov   [ebp-6], ax                           ; rect.x2 = rect.x1 + CHAR_X_SIZE

0001a6db push  -1                                    ; colour = -1 -> plain copy path
0001a6dd push  [ebp+0x0c]                            ; dst stride
0001a6e0 push  0x400                                 ; source stride (1024 bytes)
0001a6e5 push  0                                     ; source Y = 0
0001a6e7 push  esi                                   ; source X
0001a6e8 lea   eax, [ebp-8]; push eax                ; &rect
0001a6ec push  [ebp+0x08]                            ; SCREEN
0001a6ef push  ebx                                   ; baked font base
0001a6f0 call  PIX_BLOT(0x1fe7c)

0001a6f5 mov   ax, CHAR_X_WID
0001a6fb add   [ebp-8], ax                           ; rect.x1 += CHAR_X_WID
0001a6ff inc   edi
0001a700 add   esp, 0x20
0001a703 cmp   byte [edi], 0
0001a706 jne   .next
.done:
0001a708 pop edi esi ebx; leave; ret
```

Key facts locked here:

- **ASCII → glyph-index mapping is direct**. The source column is
  `char_byte * 8 + (8 - CHAR_X_SIZE)`. There is **no** subtraction of `0x20`
  and **no** lookup table — the byte value is the raster index, times an
  8-pixel source pitch.
- **Glyph width drawn per blit** is `CHAR_X_SIZE` (5 in the initialized image).
- **Glyph height drawn per blit** is `CHAR_Y_SIZE`: the inclusive rectangle
  spans `CHAR_Y_SIZE - CHAR_DESCENDER + 1` rows.
- **Source pitch between glyphs** is 8 pixels — the source raster reserves
  an 8-pixel-wide cell per glyph, of which the rightmost `CHAR_X_SIZE`
  source columns are copied to the destination.
- **Per-character advance in the destination** is `CHAR_X_WID` pixels,
  matching the width formula in `TEXT_SIZE`.
- The blit itself is `PIX_BLOT` (0x1fe7c) with colour argument `-1`. From
  the previously-decoded PIX_BLOT: `colour < 0` selects `EGB_WRITEPAGE(0)`
  + `EGB_PUTBLOCKCOLOR` in plain-copy mode — no EGB-side masking, no
  EGB-side colour substitution. **The colour is baked into the source
  raster** by `text_colourise` before the blit loop begins.

## Colour handling — `text_colourise` at 0x1a5a4

`text_colourise(fg_word, bg_word)` returns a dword pointer to a 4bpp raster
in which every source-font pixel has already been resolved to `fg` or `bg`
according to its 1bpp value. The result pointer is stored at `[0x293e4]`
and cached — the function returns immediately if `(fg, bg)` match the last
call.

Verified disassembly (essentials, edited for readability):

```
di = arg_fg;  si = arg_bg
if di == cached_fg AND si == cached_bg: return cached_out_ptr

ecx = [TEXT_PIC]                      ; source ptr (1bpp font raster)
eax = out_ptr = [0x293e4]              ; destination cursor
[ebp-8]  = fg << 4                     ; fg copy shifted into high nibble
[ebp-0xa] = bg << 4                    ; bg copy shifted into high nibble

for outer = 0x300 iterations:          ; 768 source bytes
  byte = *ecx++
  for inner = 0..3:                    ; four 2-bit groups per byte
    lo = (byte & 0x80) ? fg : bg       ; bit 7 -> low-nibble pixel colour
    hi = (byte & 0x40) ? fg : bg       ; bit 6 -> high-nibble pixel colour
    *out++ = ((hi << 4) | lo) & 0xff   ; one output byte = two 4bpp pixels
    byte <<= 2
cached_fg = di; cached_bg = si
return out_ptr
```

Colour semantics:

- Input font raster at `[TEXT_PIC]` is **1 bit per pixel**, 768 bytes total
  (`0x300 × 8` = 6144 pixels).
- Output raster at `[0x293e4]` is **4 bits per pixel packed two pixels per
  byte** (FM Towns EGB `WPAGE=6`/`WMODE=6` layout), 3072 bytes total.
- Every set source bit becomes an `fg`-coloured 4bpp pixel; every cleared
  source bit becomes a `bg`-coloured 4bpp pixel. The lookup is per-bit
  (both are pushed into every output byte).
- The blit path is therefore an **EGB_PUTBLOCKCOLOR plain copy** of the
  baked buffer with no additional masking or blending — colours are chosen
  entirely by `text_colourise`.
- The (fg, bg) pair is cached at `[0x293e8]` / `[0x293ea]`; a subsequent
  call with the same pair skips re-colourising.

## Font raster identification

- Base pointer symbol: `TEXT_PIC` at vaddr `0x29344` (dword). In the
  hash-verified file this dword is zero — it is populated at init.
- Raster kind: **1 bit per pixel**, 768 bytes = 6144 pixels total.
- Baked (fg/bg-resolved) copy: `[0x293e4]` (dword) — 3072 bytes, 4bpp,
  EGB source-stride `0x400 = 1024` bytes as passed to `PIX_BLOT`.
- Per-glyph source pitch: `8` pixels (`char_byte × 8` addressing).
- Per-glyph horizontal draw width: `CHAR_X_SIZE` pixels.
- Per-glyph vertical draw height: `CHAR_Y_SIZE` pixels.
- Per-character horizontal advance: `CHAR_X_WID` pixels.
- End-of-string horizontal trim: `CHAR_X_SPC` pixels.

The initialized CHAR_* values and the loaded asset determine the glyph
dimensions: this retail image has a 5-pixel-wide × 6-pixel-tall cell and a
6-pixel horizontal pitch. The 768-byte raster is loaded from asset 557 into
`TEXT_PIC`; its source cells are 8 pixels wide by 6 rows high.

## What is NOT recoverable from `DO_DRAW_CTEXT` alone

Static disassembly of `DO_DRAW_CTEXT` and its callees fixes the code path,
the ASCII mapping, the colour formula, and every EGB primitive used. It
does **not** recover:

- The concrete glyph bitmaps — they are not in the executable; `INIT_TEXT`
  (`0x1ae54`) reads asset 557 into `TEXT_PIC` (`0x29344`).
- The EGB palette entries selected by the `fg` / `bg` indices are installed
  outside this rasteriser subtree.

## Full rasteriser subtree (call graph)

```
DO_FDRAW_CTEXT (0x1a8c0)   -- space-pads string to arg[+0x20], tail-calls:
  DO_DRAW_CTEXT (0x1a804)  -- measures + places + blits
    TEXT_SIZE     (0x1a710) -- string extents using CHAR_* globals
    text_place   (0x18df0) -- resolves picture-loc -> destination rect
    DO_DRAW_TEXT (0x1a664) -- per-glyph blit loop
      text_colourise (0x1a5a4)  -- 1bpp->4bpp fg/bg-baked cache
        reads : [TEXT_PIC]   (0x29344)
        writes: [0x293e4] output raster
        keys  : [0x293e8] / [0x293ea]  (cached fg,bg)
      PIX_BLOT     (0x1fe7c)  -- EGB_PUTBLOCKCOLOR wrapper
        EGB_WRITEPAGE       (0x408a5)
        EGB_COLORIGRB       (0x40836)   ; only when colour >= 0 (not this path)
        EGBPARA-fill helper (0x407ec)
        EGB_WRITEMODE       (0x407a0)
        EGB_RESOLVE         (0x40739)   ; RAM-destination path only
        EGB_PUTBLOCKCOLOR   (0x40bec)
```

Nothing else needs to be lifted from `EDM.EXP` to bind the rasteriser to
the M11 framebuffer:

1. Provide the loaded font raster at the `TEXT_PIC` pointer
   (768 bytes, 1bpp, source pitch 8 pixels per glyph, glyph height
   `CHAR_Y_SIZE`).
2. Reimplement `text_colourise` verbatim (768-iter outer loop, 4-inner,
   `bit7 → low_nibble`, `bit6 → high_nibble`) — no EGB call needed.
3. Reimplement the per-glyph loop: for each byte in the padded string,
   copy a `CHAR_X_SIZE × CHAR_Y_SIZE` slab from source-X
   `char*8 + (8 - CHAR_X_SIZE)` to `(dst_x, dst_y)` in the destination
   framebuffer, then `dst_x += CHAR_X_WID`.

The colour path is **plain copy** — no masking, no per-pixel test at blit
time. All colour intelligence lives in `text_colourise`.
