# DM1 V1 Graphics Decompression — Source Lock

File: /tmp/gfx_decompression.md
Audit: DM1 V1 GRAPHICS.DAT RLE/LZW decompression
Sources: ReDMCSB LZW.C, IMAGE3.C, EXPAND.C, GRF1.C; Firestaff dm1_v1_graphics_loader_pc34_compat.c

---

## 1. Two-Stage Pipeline

DM1 V1 uses a two-stage decompression for all graphics:

  GRAPHICS.DAT compressed data
    -> [Stage 1: LZW decompress] -> planar pixel data (4 bitplanes)
         -> [Stage 2: IMG3 expand] -> full-resolution 4bpp bitmap

Stage 1: LZW (LZW.C / dm1_v1_graphics_loader_pc34_compat.c)
Stage 2: IMG3 run-length decoding (IMAGE3.C / EXPAND.C)

---

## 2. Stage 1 — LZW Decompression

### 2.1 Algorithm: LZW with 12-bit codes

ReDMCSB source: LZW.C:8 -- G0666_i_LZW_AbsoluteMaximumCode = 4096
ReDMCSB source: LZW.C:6 -- G0664_i_LZW_CodeBitCount (starts at 9)

### 2.2 Code table

| Code    | Value | Meaning                    |
|---------|-------|---------------------------|
| CLEAR   | 256   | Flush dictionary, reset 9 bits |
| END     | 257   | End of compressed stream   |
| 258+    |       | Dictionary entries         |

### 2.3 Code width growth

Starts at 9 bits, grows to 10 at 512 entries, 11 at 1024, caps at 12/4096.

ReDMCSB source: LZW.C:36-42

### 2.4 Dictionary format

Prefix table: int16_t[4096] -- code to previous code
Append table: uint8_t[4096] -- code to appended character
Initial: entries 0-255 map to themselves

ReDMCSB source: LZW.C:130-136

### 2.5 Firestaff implementation

src/dm1/dm1_v1_graphics_loader_pc34_compat.c -- m11_gfx_lzw_decompress()

Verified against ReDMCSB:
  Clear code = 256, End code = 257, Start bits = 9, Max code = 4096
  Dictionary reset on CLEAR, KwKwK case, code width growth

---

## 3. Stage 2 — IMG3 RLE Expansion

### 3.1 Purpose

Converts LZW output (4-bitplane planar) to a 4bpp chunky bitmap.

### 3.2 IMG3 Command Stream

Bytecode stream read by F0687_IMG3_GetNibble() and F0688_IMG3_GetPixelCount().

ReDMCSB source: IMAGE3.C:1100-1160 (I34E/MEDIA707 block)

Each command byte B encodes:
  B >> 4 = operation type
  B & 0x0F = color index or sub-command
  B & 8 = run-length flag (high count if set, else 1)
  B & 7 = palette lookup index

### 3.3 IMG3 opcodes (I34E PC34)

| High nibble | Operation                              |
|-------------|---------------------------------------|
| 0x0         | Fill run: count pixels of color low_nibble |
| 0x1-0x4     | Color from palette slot N              |
| 0x5         | Literal: read color from next nibble   |
| 0x6         | Line advance without color (blank row) |
| 0x7         | Line advance with single-pixel color   |

ReDMCSB source: IMAGE3.C:1140-1160

---

## 4. Output Format: 4-Bitplane Planar

After LZW, before IMG3 expand:

  Pixel data format: 4-bitplane interleaved
  Byte width: (width + 7) / 8 per bitplane
  Total bytes: byte_width * height * 4
  Plane order: Plane0, Plane1, Plane2, Plane3

Each pixel is a 4-bit palette index (0-15).

ReDMCSB source: GRF1.C:44-52

---

## 5. GRAPHICS.DAT File Format

Header: 2 bytes little-endian bitmap count

Index entries (10 bytes each):
  2 bytes: width
  2 bytes: height
  4 bytes: compressed size
  8+: compressed data at current offset

Source: dm1_v1_graphics_loader_pc34_compat.c -- m11_gfx_open_dat()

---

## 6. Compatibility Status

| Component             | ReDMCSB source    | Firestaff status |
|-----------------------|------------------|-----------------|
| LZW max code=4096     | LZW.C:8          | implemented     |
| Start bits=9           | LZW.C:46         | implemented     |
| Clear/End codes 256/257| LZW.C:143,157   | implemented     |
| Code width growth      | LZW.C:36-42      | implemented     |
| KwKwK case             | LZW.C:162        | implemented     |
| IMG3 RLE bytecode      | IMAGE3.C:1100-1200 | partial       |
| 4-bitplane output      | GRF1.C:44-52     | implemented     |
| GRAPHICS.DAT format    | MEMORY.C:707     | implemented     |

Gap: IMG3 expand (F0689_IMG_ExpandGraphicToBitmap) -- image_expand_pc34_compat.c
calls IMG3_Compat_ExpandFromSource() but implementation needs verification.
