/*
 * dm2_v1_gfx_decode_receipt.c — DM2 V1 SKULLWIN/c_gfx_decode.cpp source-named
 * decode receipts.
 *
 * These functions are narrow, source-shaped C11 receipts over the SKULLWIN
 * image-decoder helpers used by DM2's GDAT loader.  Firestaff decodes 4-bit
 * nibble values directly to 8-bit palette indices, so the helpers below
 * preserve the source nibble semantics while operating on caller-owned
 * 8-bit pixel buffers instead of the original packed 4-bit surface.
 *
 * Source: /Users/bosse/Documents/skproject-codex-ref/SKULLWIN/c_gfx_decode.cpp
 * Source: docs/dm2_graphics.md — DM2 GDAT image formats (IMG3/IMG9)
 */

#include "dm2_v1_asset_loader.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define DM2_IMG3_HEADER_SIZE 10u
#define DM2_DECODE_IMG9_MEM 5003
#define DM2_DECODE_IMG9_STACK_SIZE DM2_DECODE_IMG9_MEM
#define DM2_DECODE_IMG9_TABLE_SIZE (3 * DM2_DECODE_IMG9_MEM)
#define DM2_E_COLX90 0x90u

/* ── IMG3 nibble helpers ───────────────────────────────────────────── */

/*
 * skproject: c_gfx_decode.cpp read_img3_nibble (line 87)
 * Reads one 4-bit nibble from a packed byte stream at *cursor.
 */
int dm2_v1_decode_img3_read_nibble(const uint8_t *raw,
                                   size_t raw_size,
                                   size_t *cursor,
                                   uint8_t *out) {
    size_t byte_pos;

    if (!raw || !cursor || !out) return 0;
    byte_pos = *cursor >> 1;
    if (byte_pos >= raw_size) return 0;
    *out = (uint8_t)(((*cursor & 1u) != 0u)
                     ? (raw[byte_pos] & 0x0fu)
                     : ((raw[byte_pos] >> 4) & 0x0fu));
    ++(*cursor);
    return 1;
}

/*
 * skproject: c_gfx_decode.cpp read_img3_duration (line 95)
 * Reads a variable-length run length from the nibble stream.
 */
int dm2_v1_decode_img3_read_duration(const uint8_t *raw,
                                     size_t raw_size,
                                     size_t *cursor,
                                     int *out_duration) {
    uint8_t n;

    if (!dm2_v1_decode_img3_read_nibble(raw, raw_size, cursor, &n)) return 0;
    if (n == 0x0fu) {
        uint8_t hi;
        uint8_t lo;
        int v;
        if (!dm2_v1_decode_img3_read_nibble(raw, raw_size, cursor, &hi) ||
            !dm2_v1_decode_img3_read_nibble(raw, raw_size, cursor, &lo)) {
            return 0;
        }
        v = ((int)hi << 4) | (int)lo;
        if (v == 0xff) {
            uint8_t a;
            uint8_t b;
            uint8_t c;
            uint8_t d;
            if (!dm2_v1_decode_img3_read_nibble(raw, raw_size, cursor, &a) ||
                !dm2_v1_decode_img3_read_nibble(raw, raw_size, cursor, &b) ||
                !dm2_v1_decode_img3_read_nibble(raw, raw_size, cursor, &c) ||
                !dm2_v1_decode_img3_read_nibble(raw, raw_size, cursor, &d)) {
                return 0;
            }
            v = ((int)a << 12) | ((int)b << 8) | ((int)c << 4) | (int)d;
            *out_duration = v;
            return v > 0;
        }
        *out_duration = v + 0x11;
        return 1;
    }
    *out_duration = (int)n + 2;
    return 1;
}

/*
 * skproject: c_gfx_decode.cpp func_44c8_1202 (line 52)
 * Writes a single 4-bit pixel value into an 8-bit-index pixel buffer.
 * The source writes one nibble into a packed 16-bit surface; here the
 * nibble value is stored as a full 8-bit palette index.
 */
void dm2_v1_decode_img3_func_44c8_1202(uint8_t *dest,
                                       size_t dest_pixels,
                                       size_t offset,
                                       uint8_t pixel4) {
    if (!dest || offset >= dest_pixels || pixel4 > 0x0fu) return;
    dest[offset] = pixel4;
}

/*
 * skproject: c_gfx_decode.cpp spill_img3_pixels (line 63)
 * Copies num pixels from a previous-line source offset to the current
 * destination offset.  Bounds are checked against the caller-owned buffer.
 */
int dm2_v1_decode_img3_spill_pixels(uint8_t *dest,
                                    size_t dest_pixels,
                                    size_t dofs,
                                    size_t sofs,
                                    int num) {
    if (!dest || num < 0) return 0;
    if ((size_t)num > dest_pixels || dofs > dest_pixels - (size_t)num) return 0;
    if (sofs > dest_pixels - (size_t)num) return 0;
    memmove(dest + dofs, dest + sofs, (size_t)num);
    return 1;
}

/*
 * skproject: c_gfx_decode.cpp transparent_img3_pixels (line 111)
 * Copies num pixels from the underlay buffer into the destination buffer,
 * preserving the bounds of both caller-owned buffers.
 */
int dm2_v1_decode_img3_transparent_pixels(uint8_t *dest,
                                          const uint8_t *underlay,
                                          size_t underlay_pixels,
                                          size_t ofs,
                                          int num) {
    if (!dest || !underlay || num < 0) return 0;
    if ((size_t)num > underlay_pixels || ofs > underlay_pixels - (size_t)num) {
        return 0;
    }
    memcpy(dest + ofs, underlay + ofs, (size_t)num);
    return 1;
}

/* ── IMG3 overlay decoder ──────────────────────────────────────────── */

/*
 * skproject: c_gfx_decode.cpp decode_img3_overlay (line 276)
 * Decodes a door/panel IMG3 overlay over an existing underlay image.
 * Returns a newly allocated 8-bit pixel buffer, or NULL on error.
 */
uint8_t *dm2_v1_decode_img3_overlay(const uint8_t *raw,
                                    size_t raw_size,
                                    const uint8_t *underlay,
                                    size_t underlay_pixels,
                                    int width,
                                    int height,
                                    DM2_ImageFormat *out_format) {
    uint8_t palette[5];
    size_t cursor = 8u;
    size_t maxofs;
    size_t dofs = 0u;
    uint8_t *dest;

    if (!raw || raw_size < DM2_IMG3_HEADER_SIZE ||
        !underlay || width <= 0 || height <= 0) {
        return NULL;
    }
    maxofs = (size_t)width * (size_t)height;
    if (maxofs == 0 || maxofs > (size_t)1024u * 1024u ||
        underlay_pixels < maxofs) {
        return NULL;
    }
    dest = (uint8_t *)malloc(maxofs);
    if (!dest) return NULL;

    for (int i = 0; i < 5; ++i) {
        if (!dm2_v1_decode_img3_read_nibble(raw, raw_size, &cursor,
                                            &palette[i])) {
            free(dest);
            return NULL;
        }
    }

    while (dofs < maxofs) {
        uint8_t command;
        int idx;
        int umask;
        uint8_t fpix;
        int run;

        if (!dm2_v1_decode_img3_read_nibble(raw, raw_size, &cursor, &command)) {
            free(dest);
            return NULL;
        }
        idx = (int)(command & 0x07u);
        umask = (int)(command & 0x08u);

        if (idx == 6) {
            /* spill from previous line */
            run = (umask != 0)
                      ? 0
                      : 1;
            if (umask != 0 &&
                !dm2_v1_decode_img3_read_duration(raw, raw_size, &cursor,
                                                  &run)) {
                free(dest);
                return NULL;
            }
            if (dofs < (size_t)width ||
                !dm2_v1_decode_img3_spill_pixels(dest, maxofs, dofs,
                                                 dofs - (size_t)width, run)) {
                free(dest);
                return NULL;
            }
            dofs += (size_t)run;
        } else if (idx == 5) {
            /* transparent: copy from underlay */
            run = (umask != 0)
                      ? 0
                      : 1;
            if (umask != 0 &&
                !dm2_v1_decode_img3_read_duration(raw, raw_size, &cursor,
                                                  &run)) {
                free(dest);
                return NULL;
            }
            if (!dm2_v1_decode_img3_transparent_pixels(dest, underlay,
                                                       underlay_pixels,
                                                       dofs, run)) {
                free(dest);
                return NULL;
            }
            dofs += (size_t)run;
        } else {
            if (idx >= 5) {
                if (!dm2_v1_decode_img3_read_nibble(raw, raw_size, &cursor,
                                                    &fpix)) {
                    free(dest);
                    return NULL;
                }
            } else {
                fpix = palette[idx];
            }
            if (umask == 0) {
                dm2_v1_decode_img3_func_44c8_1202(dest, maxofs, dofs, fpix);
                dofs++;
            } else {
                if (!dm2_v1_decode_img3_read_duration(raw, raw_size, &cursor,
                                                      &run)) {
                    free(dest);
                    return NULL;
                }
                if (dofs + (size_t)run > maxofs) {
                    free(dest);
                    return NULL;
                }
                memset(dest + dofs, fpix, (size_t)run);
                dofs += (size_t)run;
            }
        }
    }

    if (out_format) *out_format = DM2_IMG_FMT_IMG3;
    return dest;
}

/* ── IMG9 mode-1 LZW-like decoder ──────────────────────────────────── */

typedef struct {
    const uint8_t *decodeptr2;
    const uint8_t *decodeptr3;
    int decodew0;
    int decodew1;
    int decodew2;
    int decodew3;
    int decodew4;
    int decodew5;
    int decodew6;
    int decode7;
    const uint8_t *decodemask;
} DM2_Dec9State;

static const uint8_t dm2_decode_img9_decodemask[9] = {
    0x00, 0x01, 0x03, 0x07, 0x0f, 0x1f, 0x3f, 0x7f, 0xff
};

/*
 * skproject: c_gfx_decode.cpp dec9_1sub (line 465)
 * Reads the next variable-length code word from the mode-1 bit stream.
 * Returns -1 when the input is exhausted.
 */
static int dm2_decode_img9_dec9_1sub(DM2_Dec9State *s) {
    int pass = 0;
    int pass2 = 0;

    if (s->decodew3 <= s->decodew1) {
        if (!s->decodew2) {
            if (s->decodew5 != 0) pass = 1;
        } else {
            s->decodew4 = 9;
            s->decodew1 = 0x1ff;
            s->decodew2 = 0;
        }
    } else {
        s->decodew4++;
        if (s->decodew4 != 12) {
            s->decodew1 = (1 << s->decodew4) - 1;
        } else {
            s->decodew1 = 0x1000;
        }
    }

    if (!pass) {
        int wordrg21;
        if (s->decodew4 >= s->decodew0) {
            wordrg21 = s->decodew0;
            if (wordrg21 == 0) return -1;
            s->decodew5 = ((long)wordrg21 << 3) / s->decodew4;
        } else {
            wordrg21 = s->decodew4;
            s->decodew5 = 8;
        }
        s->decodeptr3 = s->decodeptr2;
        s->decodeptr2 += wordrg21;
        s->decodew0 -= wordrg21;
        s->decodew6 = 0;
    }

    {
        int wordrg11 = s->decodew6;
        int wordrg2 = s->decodew4;
        const uint8_t *xptrrg4 = s->decodeptr3;
        int wordrg4;
        int wordrg39;
        int wordrg12;

        s->decodeptr3 = xptrrg4 + 1;
        wordrg4 = (*xptrrg4) >> wordrg11;
        wordrg39 = 8 - wordrg11;
        wordrg12 = wordrg39;
        wordrg2 -= wordrg39;

        if (wordrg2 >= 8) {
            s->decodeptr3++;
            {
                int wordrg52 = *(s->decodeptr3);
                wordrg52 <<= wordrg12;
                wordrg4 |= wordrg52;
            }
            wordrg2 -= 8;
            if (wordrg2 == 0) pass2 = 1;
            else wordrg12 += 8;
        }

        if (!pass2) {
            int wordrg51 = (int)(s->decodemask[wordrg2] & *(s->decodeptr3));
            wordrg4 |= wordrg51 << wordrg12;
        }

        s->decodew6 = wordrg2;
        s->decodew5--;
        return wordrg4;
    }
}

/*
 * skproject: c_gfx_decode.cpp dec9_1 (line 545)
 * Decompresses a mode-1 (variable LZW-like) IMG9 payload into an 8-bit
 * pixel buffer.  The E_COLX90 escape byte marks transparent repeat runs.
 */
static int dm2_decode_img9_mode1_into(const uint8_t *data,
                                      size_t data_size,
                                      uint8_t *gfx,
                                      size_t gfx_capacity) {
    int16_t wptrrg6[DM2_DECODE_IMG9_TABLE_SIZE];
    uint8_t pb_04[DM2_DECODE_IMG9_MEM];
    uint8_t stack[DM2_DECODE_IMG9_STACK_SIZE];
    uint8_t *xptrrg2 = stack;
    const uint8_t *pb_08 = stack;
    DM2_Dec9State s;
    int wordrg11;
    unsigned vw_10;
    uint8_t vb_18;
    uint8_t vb_1c = 0;
    size_t out_pos = 0u;

    if (!data || !gfx || data_size == 0) return 0;

    memset(&s, 0, sizeof(s));
    s.decodemask = dm2_decode_img9_decodemask;
    s.decodeptr2 = data;
    s.decodew0 = (int)data_size;
    s.decodew4 = 9;
    s.decodew1 = 0x1ff;

    memset(wptrrg6, 0, 0x200);
    for (int vw_14 = 0x100; vw_14 >= 0; --vw_14) {
        pb_04[vw_14] = (uint8_t)vw_14;
    }
    s.decodew3 = 0x101;

    wordrg11 = dm2_decode_img9_dec9_1sub(&s);
    if (wordrg11 < 0) return 1; /* empty stream: success with zero pixels */

    vw_10 = (unsigned)wordrg11;
    vb_18 = (uint8_t)wordrg11;
    if ((wordrg11 & 0xff) == DM2_E_COLX90 && (wordrg11 & 0xff00) == 0) {
        s.decode7 = 1;
    } else {
        vb_1c = (uint8_t)wordrg11;
        if (out_pos >= gfx_capacity) return 0;
        gfx[out_pos++] = vb_1c;
    }

    for (;;) {
        int wordrg1 = dm2_decode_img9_dec9_1sub(&s);
        int vw_14 = wordrg1;
        unsigned vw_0c;

        if (wordrg1 < 0) return 1; /* input exhausted */

        if (wordrg1 != 0x100) {
            vw_0c = (unsigned)wordrg1;
            if (wordrg1 >= s.decodew3) {
                *xptrrg2++ = vb_18;
                vw_14 = (int)vw_10;
            }

            for (;;) {
                long longrg12 = (long)vw_14;
                uint8_t *ebppb = xptrrg2 + 1;
                const uint8_t *xptrrg51 = pb_04 + longrg12;
                if (longrg12 < 0x100) {
                    vb_18 = *xptrrg51;
                    *xptrrg2 = vb_18;
                    xptrrg2 = ebppb;
                    break;
                }
                *xptrrg2 = *xptrrg51;
                xptrrg2 = ebppb;
                vw_14 = (unsigned)wptrrg6[longrg12];
            }

            while (xptrrg2 > pb_08) {
                uint8_t *xptrrg52 = gfx + out_pos + 1;
                uint8_t *xptrrg1 = xptrrg2 - 1;
                uint8_t b;
                if (s.decode7) {
                    xptrrg2 = xptrrg1;
                    b = *xptrrg1;
                    if (b == 0) {
                        if (out_pos >= gfx_capacity) return 0;
                        gfx[out_pos] = DM2_E_COLX90;
                        out_pos++;
                    } else {
                        uint8_t pix = vb_1c;
                        while (--b != 0) {
                            if (out_pos >= gfx_capacity) return 0;
                            gfx[out_pos++] = pix;
                        }
                    }
                    s.decode7 = 0;
                } else {
                    xptrrg2 = xptrrg1;
                    b = *xptrrg1;
                    if (b != DM2_E_COLX90) {
                        vb_1c = b;
                        if (out_pos >= gfx_capacity) return 0;
                        gfx[out_pos] = b;
                        out_pos++;
                    } else {
                        s.decode7 = 1;
                    }
                }
                (void)xptrrg52;
            }

            {
                int rg1 = s.decodew3;
                if (rg1 < 0x1000) {
                    wptrrg6[rg1] = (int16_t)vw_10;
                    pb_04[rg1] = vb_18;
                    s.decodew3++;
                }
            }
            vw_10 = vw_0c;
        } else {
            memset(wptrrg6, 0, 0x200);
            s.decodew2 = 1;
            s.decodew3 = 0x100;
        }
    }
}

/* ── IMG9 mode-2/mode-3 LZ77 decoders ──────────────────────────────── */

/*
 * skproject: c_gfx_decode.cpp dec9_2 (line 672)
 * Mode-2 LZ77: 1-bit flags; literal byte or 12-bit offset/4-bit length.
 */
static int dm2_decode_img9_mode2_into(const uint8_t *data,
                                      size_t data_size,
                                      uint8_t *gfx,
                                      size_t gfx_capacity) {
    size_t in_pos = 0u;
    size_t out_pos = 0u;
    unsigned wmask = 0u;

    if (!data || !gfx) return 0;

    for (;;) {
        wmask >>= 1;
        if ((wmask & 0x100u) == 0u) {
            if (in_pos >= data_size) return 1;
            wmask = (unsigned)data[in_pos++] | 0xff00u;
        }

        if ((wmask & 1u) != 0u) {
            if (in_pos >= data_size) return 1;
            if (out_pos >= gfx_capacity) return 0;
            gfx[out_pos++] = data[in_pos++];
        } else {
            unsigned a;
            unsigned b;
            size_t negative_offset;
            size_t copy_length;
            size_t i;
            if (in_pos + 1u >= data_size) return 1;
            a = data[in_pos++];
            b = data[in_pos++];
            negative_offset = (a >> 4) + (16u * b);
            copy_length = (size_t)((a & 0x0fu) + 3);
            if (negative_offset == 0 || negative_offset > out_pos) return 0;
            for (i = 0; i < copy_length && out_pos < gfx_capacity; ++i) {
                gfx[out_pos] = gfx[out_pos - negative_offset];
                ++out_pos;
            }
        }
    }
}

/*
 * skproject: c_gfx_decode.cpp dec9_3 (line 708)
 * Mode-3 LZ77: 1-bit flags; literal byte or 11-bit offset/5-bit length.
 */
static int dm2_decode_img9_mode3_into(const uint8_t *data,
                                      size_t data_size,
                                      uint8_t *gfx,
                                      size_t gfx_capacity) {
    size_t in_pos = 0u;
    size_t out_pos = 0u;
    unsigned wmask = 0u;

    if (!data || !gfx) return 0;

    for (;;) {
        wmask >>= 1;
        if ((wmask & 0x100u) == 0u) {
            if (in_pos >= data_size) return 1;
            wmask = (unsigned)data[in_pos++] | 0xff00u;
        }

        if ((wmask & 1u) != 0u) {
            if (in_pos >= data_size) return 1;
            if (out_pos >= gfx_capacity) return 0;
            gfx[out_pos++] = data[in_pos++];
        } else {
            unsigned a;
            unsigned b;
            size_t negative_offset;
            size_t copy_length;
            size_t i;
            if (in_pos + 1u >= data_size) return 1;
            a = data[in_pos++];
            b = data[in_pos++];
            negative_offset = (a >> 5) + (8u * b);
            copy_length = (size_t)((a & 0x1fu) + 3);
            if (negative_offset == 0 || negative_offset > out_pos) return 0;
            for (i = 0; i < copy_length && out_pos < gfx_capacity; ++i) {
                gfx[out_pos] = gfx[out_pos - negative_offset];
                ++out_pos;
            }
        }
    }
}

/*
 * skproject: c_gfx_decode.cpp decode_img9 (line 744)
 * Dispatches to mode 1/2/3 IMG9 decoders.
 */
uint8_t *dm2_v1_decode_img9(const uint8_t *raw,
                            size_t raw_size,
                            int width,
                            int height,
                            DM2_ImageFormat *out_format) {
    uint8_t mode;
    size_t pixel_total;
    uint8_t *pixels;

    if (!raw || raw_size < 9u || width <= 0 || height <= 0) return NULL;
    mode = raw[6];
    pixel_total = (size_t)width * (size_t)height;
    if (pixel_total == 0 || pixel_total > (size_t)1024u * 1024u) return NULL;
    pixels = (uint8_t *)malloc(pixel_total);
    if (!pixels) return NULL;

    if (mode == 1u) {
        if (!dm2_decode_img9_mode1_into(raw + 8, raw_size - 8u,
                                        pixels, pixel_total)) {
            free(pixels);
            return NULL;
        }
    } else if (mode == 2u) {
        if (!dm2_decode_img9_mode2_into(raw + 8, raw_size - 8u,
                                        pixels, pixel_total)) {
            free(pixels);
            return NULL;
        }
    } else if (mode == 3u) {
        if (!dm2_decode_img9_mode3_into(raw + 8, raw_size - 8u,
                                        pixels, pixel_total)) {
            free(pixels);
            return NULL;
        }
    } else {
        free(pixels);
        return NULL;
    }

    if (out_format) *out_format = DM2_IMG_FMT_IMG9;
    return pixels;
}

/*
 * skproject: c_gfx_decode.cpp decode_img9 (line 744) — mode-2 entry
 */
uint8_t *dm2_v1_decode_img9_mode2(const uint8_t *raw,
                                  size_t raw_size,
                                  int width,
                                  int height,
                                  DM2_ImageFormat *out_format) {
    size_t pixel_total;
    uint8_t *pixels;

    if (!raw || raw_size < 9u || width <= 0 || height <= 0) return NULL;
    pixel_total = (size_t)width * (size_t)height;
    if (pixel_total == 0 || pixel_total > (size_t)1024u * 1024u) return NULL;
    pixels = (uint8_t *)malloc(pixel_total);
    if (!pixels) return NULL;
    if (!dm2_decode_img9_mode2_into(raw + 8, raw_size - 8u,
                                    pixels, pixel_total)) {
        free(pixels);
        return NULL;
    }
    if (out_format) *out_format = DM2_IMG_FMT_IMG9;
    return pixels;
}

/*
 * skproject: c_gfx_decode.cpp decode_img9 (line 744) — mode-3 entry
 */
uint8_t *dm2_v1_decode_img9_mode3(const uint8_t *raw,
                                  size_t raw_size,
                                  int width,
                                  int height,
                                  DM2_ImageFormat *out_format) {
    size_t pixel_total;
    uint8_t *pixels;

    if (!raw || raw_size < 9u || width <= 0 || height <= 0) return NULL;
    pixel_total = (size_t)width * (size_t)height;
    if (pixel_total == 0 || pixel_total > (size_t)1024u * 1024u) return NULL;
    pixels = (uint8_t *)malloc(pixel_total);
    if (!pixels) return NULL;
    if (!dm2_decode_img9_mode3_into(raw + 8, raw_size - 8u,
                                    pixels, pixel_total)) {
        free(pixels);
        return NULL;
    }
    if (out_format) *out_format = DM2_IMG_FMT_IMG9;
    return pixels;
}

/*
 * skproject: c_gfx_decode.cpp decode_img9 (line 744) — mode-1 entry
 */
uint8_t *dm2_v1_decode_img9_mode1(const uint8_t *raw,
                                  size_t raw_size,
                                  int width,
                                  int height,
                                  DM2_ImageFormat *out_format) {
    size_t pixel_total;
    uint8_t *pixels;

    if (!raw || raw_size < 9u || width <= 0 || height <= 0) return NULL;
    pixel_total = (size_t)width * (size_t)height;
    if (pixel_total == 0 || pixel_total > (size_t)1024u * 1024u) return NULL;
    pixels = (uint8_t *)malloc(pixel_total);
    if (!pixels) return NULL;
    if (!dm2_decode_img9_mode1_into(raw + 8, raw_size - 8u,
                                    pixels, pixel_total)) {
        free(pixels);
        return NULL;
    }
    if (out_format) *out_format = DM2_IMG_FMT_IMG9;
    return pixels;
}

/*
 * skproject: c_gfx_decode.cpp init (line 19)
 * Object lifecycle boundary: Firestaff does not use the SKULLWIN blitter
 * decode singleton, so this receipt is a no-op that documents the boundary.
 */
void dm2_v1_decode_img3_init(void) {
    /* No singleton state is required; the loader owns its own buffers. */
}

/*
 * skproject: c_gfx_decode.cpp alloc (line 41)
 * Object lifecycle boundary: Firestaff does not use the SKULLWIN free-pool
 * scratch arrays, so this receipt is a no-op that documents the boundary.
 */
void dm2_v1_decode_img3_alloc(void) {
    /* Scratch allocation is local to each decode call. */
}
