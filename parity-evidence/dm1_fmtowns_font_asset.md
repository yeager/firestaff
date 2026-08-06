# DM1 FM Towns font-asset identity — INIT_TEXT + GET_MY_DECODED decode

Recovers the exact picture-library identity of the FM Towns DM1 menu
font. The font raster itself is not inline in EDM.EXP; it is decoded
on demand from the FM Towns picture library into a 768-byte buffer
that TEXT_PIC points at. Every value below is byte-verified against
the hash-verified HMA-240 English EDM.EXP.

## Trace

### INIT_TEXT (EDM.EXP 0x1ae54)

```
push edi
push 3 ; push 0 ; call GOTO_X_Y     ; unrelated cursor init
push 1
movzx eax, word ptr [SCRBUF_SIZE]   ; ~0x400
push eax ; call PUSH_MEM             ; alloc scroll buffer
mov  [SCROLL_BUF], eax               ; @ 0x29348
push 1
push 0x300                           ; 768 bytes, the font raster size
call PUSH_MEM                        ; alloc raster buffer
mov  [TEXT_PIC], eax                 ; @ 0x29344 (byte-verified vaddr)
push 0 ; push 0 ; push eax           ; extra1=0, extra2=0, buffer=eax
push 0xffffc22d                      ; picture-library asset id
call GET_MY_DECODED                  ; load + decode into buffer
```

### GET_MY_DECODED (EDM.EXP 0x9f04)

```
di   = arg0 & 0xFFFF                 ; low 16 bits of the id argument
esi  = arg1                          ; caller's buffer
call CD_DATA                          ; load CD (source disc)
call OPEN_PIC_LIB                     ; open the FM Towns picture lib
ebx  = di & 0x8000                   ; direct-to-caller-buffer flag
ax   = di & 0x4000                   ; skip-size-header flag
di  &= 0x3FFF                        ; picture-library index (14 bits)

if ebx != 0:                          ; direct path
  eax = esi                           ; use caller's buffer as-is
else:
  eax = [DUNBUF]                      ; stage through DUNBUF
push eax ; push di ; call 0x901c     ; read compressed asset into buffer

if ebx == 0:                          ; DUNBUF-staged path
  if (di & 0x4000) == 0:              ; write a size header
    [esi - 4] = [DUNBUF][0]
  _DECODE([DUNBUF], esi, extra1, extra2)
  DIRTY_DUNBUF = 1                    ; @ 0x29414

call CLOSE_PIC_LIB
```

### Decoding INIT_TEXT's specific argument

`0xffffc22d` is Metaware High-C's sign-extension of the signed
16-bit constant `-0x3dd3` into the 32-bit argument slot; only the
low 16 bits `0xc22d` reach the loader.

| Extract                       | Value  | Meaning                                     |
|-------------------------------|--------|---------------------------------------------|
| `di & 0x8000` (DIRECT)        | 0x8000 | 1 — decode into caller's buffer directly    |
| `di & 0x4000` (SKIP_HDR)      | 0x4000 | 1 — do not decorate `[esi - 4]` with size   |
| `di & 0x3fff` (INDEX)         | 0x022d | picture-library index **557** (decimal)     |

So the FM Towns DM1 menu font is picture-library asset 557 of the
FM Towns DM1 picture library, and INIT_TEXT decodes it in
"direct-to-caller-buffer, no-header" mode into the 768-byte TEXT_PIC
allocation.

## Consumption

A firestaff-side font-loader can now walk this identity end-to-end
without opening EDM.EXP: request picture-library index 557 from the
FM Towns DM1 picture library, decode into a caller-owned 768-byte
buffer, and hand the resulting pointer to the text rasteriser
subtree.

Encoded byte-for-byte as source-locked C in
`include/dm1_v1_fmtowns_font_asset.h` +
`src/dm1/dm1_v1_fmtowns_font_asset.c`; verified by
`tests/test_dm1_v1_fmtowns_font_asset.c`.

## Remaining scope

- The FM Towns DM1 picture library format (CD-DATA-backed rather
  than the DM1 PC 3.4 `GRAPHICS.DAT` LZW format) still needs its
  own decoder before index 557 can actually be loaded into RAM.
  The English PC 3.4 GRAPHICS.DAT only ships 543 items; index 557
  belongs to the FM Towns picture library specifically.
- Once the picture-library decoder lands, this module supplies the
  identity and the text-geometry module supplies the surrounding
  glyph metrics — no synthesis is needed for the menu draw itself.
