/*
 * dm2_v1_asset_loader.c — DM2 V1 Graphics Asset Loader
 *
 * DM2 Phase 2: DM2-specific GRAPHICS.DAT loading.
 *
 * DM2 GRAPHICS.DAT format:
 *   - GDAT container with 240 category IDs addressed by category/index/field.
 *   - DOS PC English starts with a container word (0x8005), not a raw
 *     category-count word; the category limit comes from SkGlobal.h.
 *   - Entry size varies: IMG3 (4-bit nibble), IMG9 (9-bit), raw bytes.
 *
 * GDAT2 field codes (SKWin.GDAT2.InternalCodes.txt):
 *   - 06 00 00: Animation (e.g. 0x0504 = 4-frame animation)
 *   - 0F 00 00: Door strength
 *   - 04 00 00: Color key 1 (cyan — see-through effect)
 *   - 0C 00 00: Color key 2 (dark green — secondary transparency)
 *   - 20 00 00: Animated mirrored door flag
 *   - 09 00 00 / 0D 00 00: Missile strength
 *   - 85 00 00: Default ambient light
 *   - 86 00 00: Lowest acceptable light level
 *   - 87 00 00: Ambient darkness / sight distance
 *
 * Source: docs/dm2_v1_phase2_data_formats_H2254.md §3
 * Source: docs/dm2_graphics.md — GDAT categories, image formats, palette system
 * Source: docs/dm2_platform_data.md — DM2 GRAPHICS.DAT size (~8.6 MB)
 * Source: SKULL.ASM T560 — dungeon viewport rendering
 * Source: SKULL.ASM T600 — outdoor viewport rendering
 * Source: SKULL.ASM — GDAT image decoding (decode_img3_underlay/overlay, decode_img9)
 */

#include "dm2_v1_asset_loader.h"
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#if defined(__GNUC__)
#define DM2_MAYBE_UNUSED __attribute__((unused))
#else
#define DM2_MAYBE_UNUSED
#endif

/* ── Known-good DM2 hashes ─────────────────────────────────────────── */
static const uint8_t DM2_PC_EN_GRAPHICS_MD5[16] DM2_MAYBE_UNUSED = {
    0x25, 0x24, 0x7e, 0xde, 0x4d, 0xab, 0xb6, 0xa7,
    0x1e, 0x5d, 0xab, 0xdf, 0xbc, 0xd5, 0x90, 0x7d
};

/* ── GDAT header structures ───────────────────────────────────────── */

/*
 * GDAT header (DM2 PC format):
 *   DM2 PC English/French/JewelCase are hash-gated elsewhere and expose
 *   240 logical category IDs (SkGlobal.h GDAT_CATEGORY_LIMIT). The first
 *   file word is a GDAT container marker, not the category count.
 *
 * Category entry:
 *   uint16_t entry_count
 *   uint16_t field_count
 *   uint32_t field_offsets[field_count] — per-field offsets within category
 *
 * Each entry stores image data in one of:
 *   - IMG3 (4-bit nibble): wall/floor textures
 *   - IMG9 (9-bit): complex walls
 *   - Raw bytes: UI elements, fonts
 *
 * Source: docs/dm2_graphics.md §4 — image compression formats
 */
#define DM2_GDAT_HEADER_SIZE     4
#define DM2_GDAT_CATEGORY_OFFSET_TABLE_SIZE(category_count) ((category_count) * 4)
#define DM2_PC_GRAPHICS_MIN_SIZE (8U * 1024U * 1024U)
#define DM2_PC_GRAPHICS_MAX_SIZE (10U * 1024U * 1024U)
#define DM2_PC_GDAT_CONTAINER_WORD 0x8005u
#define DM2_PC_GDAT_ENT1_WORD 0x8001u
#define DM2_GDAT_ENTRY_TYPE_MAX 0x0e
#define DM2_GDAT_TYPE_IMAGE 0x01u
#define DM2_IMG3_HEADER_SIZE 10u
#define DM2_IMG_LOCAL_PALETTE_SIZE 16u

enum {
    DM2_GDAT_EP_CLS1 = 0,
    DM2_GDAT_EP_CLS2 = 1,
    DM2_GDAT_EP_CLS3 = 2,
    DM2_GDAT_EP_CLS4 = 3,
    DM2_GDAT_EP_DATA = 4,
    DM2_GDAT_EP_CLS5 = 5,
    DM2_GDAT_EP_CLS6 = 6,
    DM2_GDAT_EP_COUNT = 7
};

/* ── LE read helpers ─────────────────────────────────────────────── */
static uint16_t rd16le(const uint8_t *p) {
    return (uint16_t)p[0] | ((uint16_t)p[1] << 8);
}
static uint16_t rd16be(const uint8_t *p) {
    return ((uint16_t)p[0] << 8) | (uint16_t)p[1];
}
static DM2_MAYBE_UNUSED uint32_t rd32le(const uint8_t *p) {
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

static uint32_t dm2_gdat_read_be_bytes(const uint8_t *p, int len) {
    uint32_t v = 0;
    int i;
    for (i = 0; i < len; ++i) {
        v = (v << 8) | p[i];
    }
    return v;
}

static int dm2_gdat_ent1_group_id_to_ep(uint8_t id) {
    /* skproject SKWIN/SkWinCore.cpp LOAD_ENT1 uses _4976_4813 to map
     * the ENT1 field tags T/I/D/S/F/G/P to EPcls1..EPcls6/EPdata. */
    switch (id) {
        case 'T': return DM2_GDAT_EP_CLS1;
        case 'I': return DM2_GDAT_EP_CLS2;
        case 'D': return DM2_GDAT_EP_CLS3;
        case 'S': return DM2_GDAT_EP_CLS4;
        case 'P': return DM2_GDAT_EP_DATA;
        case 'F': return DM2_GDAT_EP_CLS5;
        case 'G': return DM2_GDAT_EP_CLS6;
        default: return -1;
    }
}

static int dm2_gdat_parse_raw_table(DM2_V1_AssetLoader *loader,
                                    uint16_t raw_count) {
    uint32_t offset;
    uint32_t raw0_size;
    uint16_t i;

    if (!loader || !loader->data) return -1;
    if (raw_count == 0) return -1;
    if (loader->data_size < 8u + ((size_t)raw_count - 1u) * 2u) return -1;

    raw0_size = rd32le(loader->data + 4);
    offset = 6u + ((uint32_t)raw_count * 2u);
    if ((uint64_t)offset + raw0_size > loader->data_size) return -1;

    loader->raw_offsets = calloc(raw_count, sizeof(*loader->raw_offsets));
    loader->raw_sizes = calloc(raw_count, sizeof(*loader->raw_sizes));
    if (!loader->raw_offsets || !loader->raw_sizes) return -1;

    loader->raw_offsets[0] = offset;
    loader->raw_sizes[0] = raw0_size;
    offset += raw0_size;
    for (i = 1; i < raw_count; ++i) {
        uint32_t sz = rd16le(loader->data + 8u + ((uint32_t)(i - 1u) * 2u));
        loader->raw_offsets[i] = offset;
        loader->raw_sizes[i] = sz;
        if ((uint64_t)offset + sz > loader->data_size) return -1;
        offset += sz;
    }
    return 0;
}

static int dm2_gdat_parse_ent1(DM2_V1_AssetLoader *loader) {
    const uint8_t *ent;
    const uint8_t *entry_base;
    uint32_t entry_offsets[DM2_GDAT_EP_COUNT];
    uint8_t entry_lengths[DM2_GDAT_EP_COUNT];
    uint16_t (*rd16)(const uint8_t *);
    uint16_t entry_count;
    uint16_t group_count;
    uint32_t stride = 0;
    uint32_t pos;
    uint16_t e;
    uint16_t g;

    if (!loader || !loader->raw_offsets || !loader->raw_sizes) return -1;
    if (loader->raw_sizes[0] < 6) return -1;
    ent = loader->data + loader->raw_offsets[0];
    if (rd16le(ent) == DM2_PC_GDAT_ENT1_WORD) {
        rd16 = rd16le;
    } else if (rd16be(ent) == DM2_PC_GDAT_ENT1_WORD) {
        rd16 = rd16be;
    } else {
        return -1;
    }

    entry_count = rd16(ent + 2);
    group_count = rd16(ent + 4);
    if (entry_count == 0 || group_count == 0) return -1;
    if (6u + ((uint32_t)group_count * 2u) > loader->raw_sizes[0]) return -1;

    for (g = 0; g < DM2_GDAT_EP_COUNT; ++g) {
        entry_offsets[g] = UINT32_MAX;
        entry_lengths[g] = 0;
    }

    pos = 6;
    for (g = 0; g < group_count; ++g) {
        int ep = dm2_gdat_ent1_group_id_to_ep(ent[pos]);
        uint8_t len = ent[pos + 1];
        if (ep >= 0 && ep < DM2_GDAT_EP_COUNT) {
            entry_offsets[ep] = stride;
            entry_lengths[ep] = len;
        }
        stride += len;
        pos += 2;
    }
    if (stride == 0) return -1;
    if (entry_offsets[DM2_GDAT_EP_CLS1] == UINT32_MAX ||
        entry_offsets[DM2_GDAT_EP_CLS2] == UINT32_MAX ||
        entry_offsets[DM2_GDAT_EP_CLS3] == UINT32_MAX ||
        entry_offsets[DM2_GDAT_EP_CLS4] == UINT32_MAX ||
        entry_offsets[DM2_GDAT_EP_DATA] == UINT32_MAX) {
        return -1;
    }
    if (pos + ((uint32_t)entry_count * stride) > loader->raw_sizes[0]) {
        return -1;
    }

    loader->entries = calloc(entry_count, sizeof(*loader->entries));
    if (!loader->entries) return -1;
    loader->entry_count = entry_count;
    entry_base = ent + pos;

    for (e = 0; e < entry_count; ++e) {
        const uint8_t *row = entry_base + ((uint32_t)e * stride);
        uint32_t cls1 = dm2_gdat_read_be_bytes(
            row + entry_offsets[DM2_GDAT_EP_CLS1],
            entry_lengths[DM2_GDAT_EP_CLS1]);
        uint32_t cls2 = dm2_gdat_read_be_bytes(
            row + entry_offsets[DM2_GDAT_EP_CLS2],
            entry_lengths[DM2_GDAT_EP_CLS2]);
        uint32_t cls3 = dm2_gdat_read_be_bytes(
            row + entry_offsets[DM2_GDAT_EP_CLS3],
            entry_lengths[DM2_GDAT_EP_CLS3]);
        uint32_t cls4 = dm2_gdat_read_be_bytes(
            row + entry_offsets[DM2_GDAT_EP_CLS4],
            entry_lengths[DM2_GDAT_EP_CLS4]);
        uint32_t data = dm2_gdat_read_be_bytes(
            row + entry_offsets[DM2_GDAT_EP_DATA],
            entry_lengths[DM2_GDAT_EP_DATA]);
        uint32_t cls5 = 0;
        uint32_t cls6 = 0;

        if (entry_offsets[DM2_GDAT_EP_CLS5] != UINT32_MAX) {
            cls5 = dm2_gdat_read_be_bytes(row + entry_offsets[DM2_GDAT_EP_CLS5],
                                          entry_lengths[DM2_GDAT_EP_CLS5]);
        }
        if (entry_offsets[DM2_GDAT_EP_CLS6] != UINT32_MAX) {
            cls6 = dm2_gdat_read_be_bytes(row + entry_offsets[DM2_GDAT_EP_CLS6],
                                          entry_lengths[DM2_GDAT_EP_CLS6]);
        }

        loader->entries[e].cls1 = (uint8_t)cls1;
        loader->entries[e].cls2 = (uint8_t)cls2;
        loader->entries[e].cls3 = (uint8_t)cls3;
        loader->entries[e].cls4 = (uint8_t)cls4;
        loader->entries[e].cls5 = (uint8_t)cls5;
        loader->entries[e].cls6 = (uint8_t)cls6;
        loader->entries[e].data_index = (uint16_t)data;
        if (cls1 <= DM2_GDAT_CATEGORY_LIMIT && cls3 <= DM2_GDAT_ENTRY_TYPE_MAX) {
            loader->category_entry_counts[cls1]++;
        }
    }
    return 0;
}

static int dm2_img3_signed_offset(uint16_t value) {
    return (int)((int16_t)value >> 10);
}

static const DM2_V1_GdatEntry *dm2_gdat_find_entry(
    const DM2_V1_AssetLoader *loader,
    int category,
    int index,
    int type,
    int field) {
    uint16_t i;

    if (!loader || !loader->loaded || !loader->entries ||
        !loader->raw_offsets || !loader->raw_sizes) {
        return NULL;
    }
    if (category < 0 || category > DM2_GDAT_CATEGORY_LIMIT ||
        index < 0 || index > 0xff ||
        field < 0 || field > 0xff ||
        type < 0 || type > 0xff) {
        return NULL;
    }
    for (i = 0; i < loader->entry_count; ++i) {
        const DM2_V1_GdatEntry *entry = &loader->entries[i];
        if ((int)entry->cls1 == category &&
            (int)entry->cls2 == index &&
            (int)entry->cls3 == type &&
            (int)entry->cls4 == field) {
            return entry;
        }
    }
    return NULL;
}

static const uint8_t *dm2_gdat_raw_from_entry(
    const DM2_V1_AssetLoader *loader,
    const DM2_V1_GdatEntry *entry,
    size_t *out_size) {
    uint16_t raw_index;

    if (out_size) *out_size = 0;
    if (!loader || !entry || !loader->raw_offsets || !loader->raw_sizes) {
        return NULL;
    }
    raw_index = (uint16_t)(entry->data_index & 0x7fffu);
    if (raw_index >= loader->raw_data_count) return NULL;
    if (loader->raw_sizes[raw_index] == 0) return NULL;
    if ((uint64_t)loader->raw_offsets[raw_index] +
        loader->raw_sizes[raw_index] > loader->data_size) {
        return NULL;
    }
    if (out_size) *out_size = loader->raw_sizes[raw_index];
    return loader->data + loader->raw_offsets[raw_index];
}

static uint8_t *dm2_decode_uncompressed_image(const uint8_t *raw,
                                              size_t raw_size,
                                              int width,
                                              int height,
                                              int bpp,
                                              DM2_ImageFormat *out_format) {
    uint8_t *pixels;
    size_t pixel_count;
    size_t src_size;
    size_t i;

    if (!raw || width <= 0 || height <= 0) return NULL;
    pixel_count = (size_t)width * (size_t)height;
    if (pixel_count == 0 || pixel_count > (size_t)1024u * 1024u) return NULL;
    if (raw_size < DM2_IMG3_HEADER_SIZE + DM2_IMG_LOCAL_PALETTE_SIZE) {
        return NULL;
    }

    if (bpp == 4) {
        src_size = (((size_t)width + 1u) & ~(size_t)1u) / 2u * (size_t)height;
        if (raw_size < DM2_IMG3_HEADER_SIZE + src_size) return NULL;
        pixels = (uint8_t *)malloc(pixel_count);
        if (!pixels) return NULL;
        for (i = 0; i < pixel_count; ++i) {
            uint8_t byte = raw[DM2_IMG3_HEADER_SIZE + (i >> 1)];
            pixels[i] = (uint8_t)((i & 1u) ? (byte & 0x0fu)
                                           : ((byte >> 4) & 0x0fu));
        }
        if (out_format) *out_format = DM2_IMG_FMT_U4;
        return pixels;
    }

    if (bpp == 8) {
        src_size = pixel_count;
        if (raw_size < DM2_IMG3_HEADER_SIZE + src_size) return NULL;
        pixels = (uint8_t *)malloc(pixel_count);
        if (!pixels) return NULL;
        memcpy(pixels, raw + DM2_IMG3_HEADER_SIZE, pixel_count);
        if (out_format) *out_format = DM2_IMG_FMT_U8;
        return pixels;
    }

    return NULL;
}

static int dm2_img3_read_nibble(const uint8_t *raw,
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

static int dm2_img3_read_duration(const uint8_t *raw,
                                  size_t raw_size,
                                  size_t *cursor,
                                  int *out_duration) {
    uint8_t n;

    if (!dm2_img3_read_nibble(raw, raw_size, cursor, &n)) return 0;
    if (n == 0x0f) {
        uint8_t hi;
        uint8_t lo;
        int v;
        if (!dm2_img3_read_nibble(raw, raw_size, cursor, &hi) ||
            !dm2_img3_read_nibble(raw, raw_size, cursor, &lo)) {
            return 0;
        }
        v = ((int)hi << 4) | (int)lo;
        if (v == 0xff) {
            uint8_t a;
            uint8_t b;
            uint8_t c;
            uint8_t d;
            if (!dm2_img3_read_nibble(raw, raw_size, cursor, &a) ||
                !dm2_img3_read_nibble(raw, raw_size, cursor, &b) ||
                !dm2_img3_read_nibble(raw, raw_size, cursor, &c) ||
                !dm2_img3_read_nibble(raw, raw_size, cursor, &d)) {
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

static int dm2_img3_emit_run(uint8_t *padded,
                             size_t total,
                             int width,
                             int even_width,
                             size_t *pos,
                             int *line_left,
                             int count,
                             uint8_t color,
                             int copy_previous_line) {
    while (count > 0) {
        int n;
        int i;
        if (!padded || !pos || !line_left || width <= 0 ||
            even_width < width || *line_left <= 0) {
            return 0;
        }
        n = count < *line_left ? count : *line_left;
        if (*pos + (size_t)n > total) return 0;
        if (copy_previous_line) {
            /* skproject SKWIN/SkWinCore.cpp DECODE_IMG3_UNDERLAY lines
             * ~37980-37983 copies the remaining row span for odd-width
             * partial previous-line runs, then advances only by run length. */
            int copy_n = n < *line_left ? *line_left : n;
            if (*pos < (size_t)even_width) return 0;
            if (*pos + (size_t)copy_n > total) return 0;
            for (i = 0; i < copy_n; ++i) {
                padded[*pos + (size_t)i] =
                    padded[*pos - (size_t)even_width + (size_t)i];
            }
        } else {
            memset(padded + *pos, color, (size_t)n);
        }
        *pos += (size_t)n;
        *line_left -= n;
        count -= n;
        if (*line_left == 0 && *pos < total) {
            *pos += (size_t)(even_width - width);
            *line_left = width;
        }
    }
    return 1;
}

static uint8_t *dm2_decode_img3_c4(const uint8_t *raw,
                                   size_t raw_size,
                                   int width,
                                   int height,
                                   DM2_ImageFormat *out_format) {
    int even_width;
    size_t padded_total;
    size_t pixel_total;
    uint8_t *padded;
    uint8_t *pixels;
    uint8_t palette[6];
    size_t cursor = 8u;
    size_t pos = 0u;
    int line_left;
    int i;
    int y;

    if (!raw || raw_size < DM2_IMG3_HEADER_SIZE ||
        width <= 0 || height <= 0) {
        return NULL;
    }
    even_width = (width + 1) & ~1;
    padded_total = (size_t)even_width * (size_t)height;
    pixel_total = (size_t)width * (size_t)height;
    if (padded_total == 0 || padded_total > (size_t)1024u * 1024u) {
        return NULL;
    }

    padded = (uint8_t *)calloc(padded_total, 1u);
    pixels = (uint8_t *)malloc(pixel_total);
    if (!padded || !pixels) {
        free(padded);
        free(pixels);
        return NULL;
    }

    for (i = 0; i < 6; ++i) {
        if (!dm2_img3_read_nibble(raw, raw_size, &cursor, &palette[i])) {
            free(padded);
            free(pixels);
            return NULL;
        }
    }

    line_left = width;
    while (pos < padded_total) {
        uint8_t command;
        int code;
        int run;
        uint8_t color;

        if (!dm2_img3_read_nibble(raw, raw_size, &cursor, &command)) {
            free(padded);
            free(pixels);
            return NULL;
        }
        code = command & 7;
        if (code == 6) {
            run = ((command & 8) != 0)
                      ? 0
                      : 1;
            if ((command & 8) != 0 &&
                !dm2_img3_read_duration(raw, raw_size, &cursor, &run)) {
                free(padded);
                free(pixels);
                return NULL;
            }
            if (!dm2_img3_emit_run(padded,
                                   padded_total,
                                   width,
                                   even_width,
                                   &pos,
                                   &line_left,
                                   run,
                                   0,
                                   1)) {
                free(padded);
                free(pixels);
                return NULL;
            }
            continue;
        }

        if (code < 6) {
            color = palette[code];
        } else if (!dm2_img3_read_nibble(raw, raw_size, &cursor, &color)) {
            free(padded);
            free(pixels);
            return NULL;
        }

        if ((command & 8) != 0) {
            if (!dm2_img3_read_duration(raw, raw_size, &cursor, &run)) {
                free(padded);
                free(pixels);
                return NULL;
            }
        } else {
            run = 1;
        }
        if (!dm2_img3_emit_run(padded,
                               padded_total,
                               width,
                               even_width,
                               &pos,
                               &line_left,
                               run,
                               color,
                               0)) {
            free(padded);
            free(pixels);
            return NULL;
        }
    }

    for (y = 0; y < height; ++y) {
        memcpy(pixels + ((size_t)y * (size_t)width),
               padded + ((size_t)y * (size_t)even_width),
               (size_t)width);
    }
    free(padded);
    if (out_format) *out_format = DM2_IMG_FMT_IMG3;
    return pixels;
}

static uint8_t *dm2_decode_img9_c8(const uint8_t *raw,
                                   size_t raw_size,
                                   int width,
                                   int height,
                                   DM2_ImageFormat *out_format) {
    size_t pixel_total;
    size_t in_pos = 8u;
    size_t out_pos = 0u;
    uint8_t *pixels;
    uint8_t typex;

    if (!raw || raw_size < 9u || width <= 0 || height <= 0) return NULL;
    pixel_total = (size_t)width * (size_t)height;
    if (pixel_total == 0 || pixel_total > (size_t)1024u * 1024u) return NULL;
    pixels = (uint8_t *)malloc(pixel_total);
    if (!pixels) return NULL;
    typex = raw[6];

    while (out_pos < pixel_total) {
        uint8_t command;
        int bit;
        if (in_pos >= raw_size) {
            free(pixels);
            return NULL;
        }
        command = raw[in_pos++];
        for (bit = 0; bit < 8 && out_pos < pixel_total; ++bit, command >>= 1) {
            if ((command & 1u) != 0u) {
                if (in_pos >= raw_size) {
                    free(pixels);
                    return NULL;
                }
                pixels[out_pos++] = raw[in_pos++];
            } else {
                int a;
                int b;
                int negative_offset;
                int copy_length;
                int i;
                if (in_pos + 1u >= raw_size) {
                    free(pixels);
                    return NULL;
                }
                a = raw[in_pos++];
                b = raw[in_pos++];
                if (typex == 2u) {
                    negative_offset = (a >> 4) + (16 * b);
                    copy_length = (a & 0x0f) + 3;
                } else {
                    negative_offset = (a >> 5) + (8 * b);
                    copy_length = (a & 0x1f) + 3;
                }
                if (negative_offset <= 0 ||
                    (size_t)negative_offset > out_pos) {
                    free(pixels);
                    return NULL;
                }
                for (i = 0; i < copy_length && out_pos < pixel_total; ++i) {
                    pixels[out_pos] = pixels[out_pos - (size_t)negative_offset];
                    ++out_pos;
                }
            }
        }
    }
    if (out_format) *out_format = DM2_IMG_FMT_IMG9;
    return pixels;
}

/* ── Public API ─────────────────────────────────────────────────── */

int dm2_v1_asset_loader_init(DM2_V1_AssetLoader *loader,
                              const uint8_t *data, size_t size) {
    if (!loader) return -1;
    memset(loader, 0, sizeof(*loader));

    if (!data || size < DM2_GDAT_HEADER_SIZE + 4) return -1;

    uint16_t first_word = rd16le(data + 0);
    uint16_t raw_count = rd16le(data + 2);

    if ((first_word & 0x8000u) == 0) return -1;
    if ((first_word & 0x7fffu) != 5u &&
        (first_word & 0x7fffu) != 4u &&
        (first_word & 0x7fffu) != 2u) {
        return -1;
    }
    if (first_word != DM2_PC_GDAT_CONTAINER_WORD ||
        size < DM2_PC_GRAPHICS_MIN_SIZE ||
        size > DM2_PC_GRAPHICS_MAX_SIZE) return -1;

    loader->data = data;
    loader->data_size = size;
    loader->category_count = DM2_GDAT_CATEGORY_LIMIT + 1;
    loader->gdat_version = (uint16_t)(first_word & 0x7fffu);
    loader->raw_data_count = raw_count;
    if (dm2_gdat_parse_raw_table(loader, raw_count) != 0 ||
        dm2_gdat_parse_ent1(loader) != 0) {
        dm2_v1_asset_loader_free(loader);
        return -1;
    }

    loader->loaded = 1;
    return 0;
}

const uint8_t *dm2_v1_asset_load(const DM2_V1_AssetLoader *loader,
                                   int category, int index, int field) {
    uint16_t i;

    if (!loader || !loader->loaded || !loader->entries ||
        !loader->raw_offsets || !loader->raw_sizes) {
        return NULL;
    }
    if (category < 0 || category > DM2_GDAT_CATEGORY_LIMIT ||
        index < 0 || index > 0xff ||
        field < 0 || field > 0xff) {
        return NULL;
    }
    for (i = 0; i < loader->entry_count; ++i) {
        const DM2_V1_GdatEntry *entry = &loader->entries[i];
        uint16_t raw_index = (uint16_t)(entry->data_index & 0x7fffu);
        if ((int)entry->cls1 != category ||
            (int)entry->cls2 != index ||
            (int)entry->cls4 != field) {
            continue;
        }
        if (raw_index >= loader->raw_data_count) return NULL;
        if (loader->raw_sizes[raw_index] == 0) return NULL;
        if ((uint64_t)loader->raw_offsets[raw_index] +
            loader->raw_sizes[raw_index] > loader->data_size) {
            return NULL;
        }
        return loader->data + loader->raw_offsets[raw_index];
    }
    return NULL;
}

uint8_t *dm2_v1_asset_load_image(const DM2_V1_AssetLoader *loader,
                                   int category, int index,
                                   int *out_width, int *out_height,
                                   DM2_ImageFormat *out_format) {
    return dm2_v1_asset_load_image_field(loader,
                                         category,
                                         index,
                                         0,
                                         out_width,
                                         out_height,
                                         out_format);
}

uint8_t *dm2_v1_asset_load_image_field(const DM2_V1_AssetLoader *loader,
                                        int category, int index, int field,
                                        int *out_width, int *out_height,
                                        DM2_ImageFormat *out_format) {
    const DM2_V1_GdatEntry *entry;
    const uint8_t *raw;
    size_t raw_size = 0;
    uint16_t cx;
    uint16_t cy;
    uint16_t bpp;
    int width;
    int height;
    int offset_y;
    uint8_t *pixels;

    if (out_width) *out_width = 0;
    if (out_height) *out_height = 0;
    if (out_format) *out_format = DM2_IMG_FMT_UNKNOWN;
    entry = dm2_gdat_find_entry(loader,
                                category,
                                index,
                                DM2_GDAT_TYPE_IMAGE,
                                field);
    raw = dm2_gdat_raw_from_entry(loader, entry, &raw_size);
    if (!raw || raw_size < DM2_IMG3_HEADER_SIZE) return NULL;

    cx = rd16le(raw + 0);
    cy = rd16le(raw + 2);
    bpp = rd16le(raw + 4);
    width = (int)(cx & 0x03ffu);
    height = (int)(cy & 0x03ffu);
    offset_y = dm2_img3_signed_offset(cy);
    if (width <= 0 || height <= 0) return NULL;

    /* skproject: SKWIN/DME.h IMG3::Getpf lines 1114-1120 classifies
     * OffsetY == -32 with w4 == 4/8 as uncompressed U4/U8.  EXTRACT_GDAT_IMAGE
     * lines 38157-38170 returns the payload at IMG3+10 for those images. */
    if (offset_y == -32) {
        pixels = dm2_decode_uncompressed_image(raw,
                                               raw_size,
                                               width,
                                               height,
                                               (int)bpp,
                                               out_format);
        if (!pixels) return NULL;
        if (out_width) *out_width = width;
        if (out_height) *out_height = height;
        return pixels;
    }

    if (offset_y == 31) {
        pixels = dm2_decode_img9_c8(raw,
                                    raw_size,
                                    width,
                                    height,
                                    out_format);
        if (!pixels) return NULL;
        if (out_width) *out_width = width;
        if (out_height) *out_height = height;
        return pixels;
    }

    /* skproject routes compressed C4 through DECODE_IMG3_UNDERLAY_LOCAL in
     * EXTRACT_GDAT_IMAGE lines 38273-38286 and DME.h IMG3::Getpf line ~1122. */
    pixels = dm2_decode_img3_c4(raw,
                                raw_size,
                                width,
                                height,
                                out_format);
    if (!pixels) return NULL;
    if (out_width) *out_width = width;
    if (out_height) *out_height = height;
    return pixels;
}

int dm2_v1_asset_category_entry_count(const DM2_V1_AssetLoader *loader,
                                       int category) {
    if (!loader || !loader->loaded) return 0;
    if (category < 0 || category > DM2_GDAT_CATEGORY_LIMIT) return 0;
    return loader->category_entry_counts[category];
}

void dm2_v1_asset_free_pixels(uint8_t *pixels) {
    free(pixels);
}

const char *dm2_v1_asset_gdat2_field_name(int field_code) {
    switch (field_code) {
        case 0x060000: return "Animation";
        case 0x0F0000: return "DoorStrength";
        case 0x040000: return "ColorKey1_Cyan";
        case 0x0C0000: return "ColorKey2_DarkGreen";
        case 0x200000: return "AnimatedMirroredDoor";
        case 0x090000: return "MissileStrength1";
        case 0x0D0000: return "MissileStrength2";
        case 0x850000: return "DefaultAmbientLight";
        case 0x860000: return "LowestAcceptableLight";
        case 0x870000: return "AmbientDarkness_SightDistance";
        default: return "UnknownField";
    }
}

int dm2_v1_asset_loader_verify(const DM2_V1_AssetLoader *loader) {
    if (!loader || !loader->data) return 0;
    /* DM2 PC English MD5: 25247ede4dabb6a71e5dabdfbcd5907d.
     * This loader is initialized from memory and does not own a filename, so
     * the scanner performs exact hash gating before launch; here we enforce
     * the real DOS GDAT marker plus the locked PC size window. */
    if (loader->data_size >= DM2_PC_GRAPHICS_MIN_SIZE &&
        loader->data_size <= DM2_PC_GRAPHICS_MAX_SIZE &&
        rd16le(loader->data) == DM2_PC_GDAT_CONTAINER_WORD) {
        return 1; /* plausible DM2 GRAPHICS.DAT size */
    }
    return 0;
}

void dm2_v1_asset_loader_free(DM2_V1_AssetLoader *loader) {
    if (!loader) return;
    free(loader->raw_offsets);
    free(loader->raw_sizes);
    free(loader->entries);
    /* data is not owned by loader (referenced), so don't free it */
    loader->data = NULL;
    loader->data_size = 0;
    loader->category_count = 0;
    loader->loaded = 0;
    loader->gdat_version = 0;
    loader->raw_data_count = 0;
    loader->raw_offsets = NULL;
    loader->raw_sizes = NULL;
    loader->entries = NULL;
    loader->entry_count = 0;
    memset(loader->category_entry_counts, 0, sizeof(loader->category_entry_counts));
}

const char *dm2_v1_asset_loader_source_evidence(void) {
    return
        "DM2 V1 Asset Loader — Phase 2 Graphics Ingestion\n"
        "Source: skproject SKWIN/SkWinCore.cpp READ_GRAPHICS_STRUCTURE/LOAD_ENT1/BUILD_GDAT_ENTRY_DATA\n"
        "Source: skproject SKWIN/SkWinCore.cpp QUERY_GDAT_IMAGE_ENTRY_BUFF/EXTRACT_GDAT_IMAGE lines ~38081-38407\n"
        "Source: skproject SKWIN/SkWinCore.cpp READ_IMG3_DURATION/DECODE_IMG3_UNDERLAY/DECODE_IMG9 lines ~37543-38066\n"
        "Source: skproject SKWIN/DME.h IMG3 lines ~1099-1143\n"
        "Source: skproject SKWIN/defines.h GDAT_CATEGORY_* and dtImage/dtWordValue codes\n"
        "Source: docs/dm2_v1_phase2_data_formats_H2254.md §3 — GRAPHICS.DAT structure\n"
        "Source: docs/dm2_graphics.md — GDAT categories (240 vs 29), image formats (IMG3/IMG9)\n"
        "Source: docs/dm2_platform_data.md — DM2 GRAPHICS.DAT size (~8.6 MB)\n"
        "Source: SKULL.ASM T560 — dungeon viewport rendering (indoor)\n"
        "Source: SKULL.ASM T600 — outdoor viewport rendering (sky/terrain)\n"
        "Source: ReDMCSB DUNGEON.C:1371-1421 — wall frame table, map coordinate resolution\n"
        "Source: ReDMCSB DUNGEON.C:148-165 — wall set indices\n"
        "Source: ReDMCSB DUNVIEW.C:2962-3047 — F0098 DrawFloorAndCeiling (IMG3 decoding)\n"
        "Source: SKWIN/knowledge/SKWin.GDAT2.InternalCodes.txt — GDAT2 field codes\n"
        "Source: SKULL.ASM — decode_img3_underlay/overlay, decode_img9\n"
        "Asset: DM2 PC English GRAPHICS.DAT 25247ede4dabb6a71e5dabdfbcd5907d (~8.6 MB)\n";
}
