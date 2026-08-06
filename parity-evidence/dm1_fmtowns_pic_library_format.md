# DM1 FM Towns picture-library container format

Every FM Towns DM1 graphic (viewport tiles, menu font, champion
portraits, item glyphs, splash screens) lives in the single file
`DATA/GRAPHICS.DAT` on the disc. The runtime opens it via
`INIT_PIC_LIB` (EDM.EXP `0x92f8`), keeps a refcounted handle with
`OPEN_PIC_LIB` (`0x9290`) / `CLOSE_PIC_LIB` (`0x92d0`), and reads
asset spans through the helper at `0x901c` (`READ_ASSET`). The
byte-verified formula in `INDEX_TO_FILE_OFFSET` (`0x8d04`) pins the
on-disk header down to three fields.

## Byte layout

```
+0000  u16  asset_count                                (little-endian)
+0002  u16  size_table_primary  [asset_count entries]
+0002+2N  u16  size_table_secondary[asset_count entries] (mirror)
+0002+4N  raw payload: asset_count spans back-to-back
```

`INDEX_TO_FILE_OFFSET(idx) = 2 + count*4 + Σ size_table_primary[0..idx-1]`
— straight from the accumulator initialised at `lea ecx, [ecx*4 + 2]`
and the loop starting at `0x8d1c`.

## Verification against the shipped English disc

`DATA/GRAPHICS.DAT` (Track 01 of the English Rev 1 disc):

| Field                | Value    |
|----------------------|----------|
| File size            | 396 970  |
| `asset_count`        | 575      |
| header bytes         | 2 302    |
| `Σ primary sizes`    | 394 668  |
| header + Σ sizes     | 396 970 ✓|
| Tables identical?    | yes (575/575 entries match) |

Asset 557 (the menu font) has `primary[557] = 768` bytes, which is
exactly the `0x300`-byte buffer `INIT_TEXT` allocates for
`TEXT_PIC`. The first 32 bytes at that span decode as glyph rows
(`00 1e 16 0f 1e 0f 1f 0e …`), each byte a five-bit-wide column
pattern — the raw font raster. Because `GET_MY_DECODED` uses the
DIRECT+NO_HDR path for the font (`di & 0x8000` and `di & 0x4000`
both set), `_DECODE` is never called on this asset: the 768 bytes
from the picture-library payload become the font raster directly.

## Runtime path

`GET_MY_DECODED` (EDM.EXP `0x9f04`):
```
di = arg0 & 0xffff                     ; encoded asset id
call CD_DATA                           ; ensure disc is mounted
call OPEN_PIC_LIB                      ; refcounted open
bx  = di & 0x8000                      ; DIRECT flag
ax  = di & 0x4000                      ; NO_HDR flag
di &= 0x3fff                           ; picture-library index
eax = (bx ? esi : DUNBUF)              ; destination
call READ_ASSET(index=di, dst=eax)     ; helper at 0x901c
if bx == 0:
    if !NO_HDR: [esi-4] = *(u32*)DUNBUF
    call _DECODE(DUNBUF, esi, extra1, extra2)  ; 0x1f940
call CLOSE_PIC_LIB
```

`READ_ASSET` (`0x901c`) walks the sector map, seeks into
`GRAPHICS.DAT` at `INDEX_TO_FILE_OFFSET(index)` (via `0x8d04`) and
reads `size_table_primary[index]` bytes in 0x400-byte chunks.

## Per-asset DECODEGRAPHIC header

Every asset span begins with a 4-byte header:
```
+0  u16 width_pixels
+2  u16 height_pixels
```
`DECODEGRAPHIC` (`0x1f63c`) computes the padded width and row
bytes:
```
padded_width = (width + 0x1f) & ~0x1f     ; 0x1f665..0x1f66e
row_bytes    = padded_width / 2            ; 0x1f66e..0x1f672 (4 bpp)
total_bytes  = row_bytes * height          ; 0x1f676
```
If `width == padded_width` the fast-copy branch at `0x1f85f` runs
(the on-disk bytes are the pixel matrix verbatim). Otherwise the
RLE loop starting at `0x1f6a0` runs — control byte layout:

| bit 7 | bit 6 | bits 4-5 | count encoding                     |
|-------|-------|----------|------------------------------------|
| 0     | -     | -        | `(ctrl >> 4) + 1` pixels literal   |
| 1     | 0     | see 4-5  | next byte + 1                      |
| 1     | 1     | see 4-5  | next two bytes (big-endian) + 1    |

Bits 4-5 select the operation applied to that count:

| 4-5 | branch      | operation                                            |
|-----|-------------|------------------------------------------------------|
| 00  | `0x1f734`   | copy `count` pixels from the stream                  |
| 01  | `0x1f775`   | write single pixel `ctrl & 0x0f` `count` times       |
| 11  | `0x1f7f3`   | mode-3 (row copy with pixel nibble); unfinished here |

The container + header parse is enough to reach any asset. The
inner RLE decoder is deferred to a follow-up job so we never emit
guessed pixels.

## Firestaff-side deliverable

- `include/dm1_v1_fmtowns_pic_library.h`
- `src/dm1/dm1_v1_fmtowns_pic_library.c`
- `tests/test_dm1_v1_fmtowns_pic_library.c`

The decoder is self-contained: no SDL, no M11, no `~/.firestaff`
runtime dependency, no `stdio` I/O. Callers hand it a buffer that
already contains a `GRAPHICS.DAT` file and the module returns
zero-copy views onto each asset span. `load_raw_asset_pc34`
implements the DIRECT+NO_HDR path `INIT_TEXT` uses for the menu
font — the raw span is memcpy'd into a caller-owned buffer.

## Next step

Decode the RLE cases at `0x1f775` and `0x1f7f3` and land a full
`DECODEGRAPHIC` port. Once that ships, every non-font asset in
`GRAPHICS.DAT` becomes reachable and the viewport / portrait /
splash bindings can move off placeholder pixels.
