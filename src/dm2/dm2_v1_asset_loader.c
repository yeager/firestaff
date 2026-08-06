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
#define DM2_FMTOWNS_GRAPHICS_MIN_SIZE (2U * 1024U * 1024U)
#define DM2_FMTOWNS_GRAPHICS_MAX_SIZE (4U * 1024U * 1024U)

/* PC-9821 JP: GDAT v5 but only ~2MB (smaller Japanese graphics set) */
#define DM2_PC9821_GRAPHICS_MIN_SIZE (1U * 1024U * 1024U)
#define DM2_PC9821_GRAPHICS_MAX_SIZE (4U * 1024U * 1024U)
#define DM2_FMTOWNS_GDAT_CONTAINER_WORD 0x8004u
#define DM2_PC_GDAT_ENT1_WORD 0x8001u
#define DM2_GDAT_ENTRY_TYPE_MAX 0x0e
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
static uint32_t rd32le(const uint8_t *p) {
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}
static uint32_t rd32be(const uint8_t *p) {
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
           ((uint32_t)p[2] << 8) | (uint32_t)p[3];
}
static uint16_t gdat_rd16(const DM2_V1_AssetLoader *l, const uint8_t *p) {
    return l->big_endian ? rd16be(p) : rd16le(p);
}
static uint32_t gdat_rd32(const DM2_V1_AssetLoader *l, const uint8_t *p) {
    return l->big_endian ? rd32be(p) : rd32le(p);
}

static uint32_t dm2_gdat_file_receipt_hash(uint32_t a,
                                           uint32_t b,
                                           uint32_t c,
                                           uint32_t d);

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

static uint32_t dm2_fnv1a_bytes(const uint8_t *data, size_t size)
{
    uint32_t hash = 2166136261u;
    size_t i;

    if (!data && size != 0u) return 0u;
    for (i = 0; i < size; ++i) {
        hash = (hash ^ data[i]) * 16777619u;
    }
    return hash ? hash : 1u;
}

static int dm2_gdat_parse_raw_table(DM2_V1_AssetLoader *loader,
                                    uint16_t raw_count) {
    uint32_t offset;
    uint32_t raw0_size;
    uint16_t i;

    if (!loader || !loader->data) return -1;
    if (raw_count == 0) return -1;
    if (loader->data_size < 8u + ((size_t)raw_count - 1u) * 2u) return -1;

    raw0_size = gdat_rd32(loader, loader->data + 4);
    offset = 6u + ((uint32_t)raw_count * 2u);
    if ((uint64_t)offset + raw0_size > loader->data_size) return -1;

    loader->raw_offsets = calloc(raw_count, sizeof(*loader->raw_offsets));
    loader->raw_sizes = calloc(raw_count, sizeof(*loader->raw_sizes));
    if (!loader->raw_offsets || !loader->raw_sizes) return -1;

    loader->raw_offsets[0] = offset;
    loader->raw_sizes[0] = raw0_size;
    offset += raw0_size;
    for (i = 1; i < raw_count; ++i) {
        uint32_t sz = gdat_rd16(loader, loader->data + 8u + ((uint32_t)(i - 1u) * 2u));
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
            loader->ent1_ep_present[ep] = 1u;
            loader->ent1_ep_lengths[ep] = len;
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
    loader->ent1_entry_stride = (uint16_t)stride;
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

static int dm2_gdat_entry_owns_raw_payload(const DM2_V1_GdatEntry *entry) {
    if (!entry) return 0;
    return entry->cls3 != DM2_GDAT_ENTRY_TYPE_WORD_VALUE &&
           entry->cls3 != DM2_GDAT_ENTRY_TYPE_IMAGE_OFFSET;
}

static int dm2_img3_bits_per_pixel(uint16_t cy, uint16_t bpp_word,
                                   uint16_t *out_bpp) {
    int offset_y;

    if (out_bpp) *out_bpp = 0u;
    offset_y = dm2_img3_signed_offset(cy);
    if (offset_y == 31) {
        if (out_bpp) *out_bpp = 8u;
        return 1;
    }
    if (bpp_word == 4u || bpp_word == 8u) {
        if (out_bpp) *out_bpp = bpp_word;
        return 1;
    }
    /* SKWINSPX c_gfx_decode.cpp::DECODE_IMG3 reads the compressed command
     * stream after the six-byte header; it does not require this word to be
     * the literal value 4.  Real PC DM2 IMG3 records such as Greatstone raw
     * 0003 (224x136, control word 0x578a) store their compression control
     * here.  They remain 4-bit local-palette images unless the source's
     * signed-height IMG9 marker above selected the 8-bit route. */
    if (out_bpp) *out_bpp = 4u;
    return 1;
}

static uint16_t img_rd16(const uint8_t *p, int be) {
    return be ? rd16be(p) : rd16le(p);
}

static int dm2_img3_raw_bits_per_pixel(const uint8_t *raw,
                                       size_t raw_size,
                                       uint16_t *out_bpp,
                                       int be) {
    uint16_t cy;

    if (out_bpp) *out_bpp = 0u;
    if (!raw || raw_size < DM2_IMG3_HEADER_SIZE) return 0;
    cy = img_rd16(raw + 2u, be);
    if (dm2_img3_bits_per_pixel(cy, img_rd16(raw + 4u, be), out_bpp)) return 1;
    if (dm2_img3_signed_offset(cy) == -32) {
        return dm2_img3_bits_per_pixel(cy, img_rd16(raw + 6u, be), out_bpp);
    }
    return 0;
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
    if (!dm2_gdat_entry_owns_raw_payload(entry)) return NULL;
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

static int dm2_v1_asset_load_image_metadata(
    const DM2_V1_AssetLoader *loader,
    int category,
    int index,
    int field,
    DM2_V1_GdatImageMetadata *out_metadata) {
    const DM2_V1_GdatEntry *entry;
    const uint8_t *raw;
    size_t raw_size = 0u;
    uint16_t cy;
    uint16_t bpp = 0u;

    if (!out_metadata) return 0;
    memset(out_metadata, 0, sizeof(*out_metadata));
    entry = dm2_gdat_find_entry(loader, category, index,
                                DM2_GDAT_ENTRY_TYPE_IMAGE, field);
    raw = dm2_gdat_raw_from_entry(loader, entry, &raw_size);
    if (!raw || raw_size < DM2_IMG3_HEADER_SIZE) return 0;
    cy = img_rd16(raw + 2u, loader->big_endian);
    if (!dm2_img3_raw_bits_per_pixel(raw, raw_size, &bpp, loader->big_endian)) return 0;
    out_metadata->width = (uint16_t)(img_rd16(raw, loader->big_endian) & 0x03ffu);
    out_metadata->height = (uint16_t)(cy & 0x03ffu);
    if (out_metadata->width == 0u || out_metadata->height == 0u) {
        memset(out_metadata, 0, sizeof(*out_metadata));
        return 0;
    }
    out_metadata->bits_per_pixel = (uint8_t)bpp;
    out_metadata->query_offset_y = (int16_t)dm2_img3_signed_offset(cy);
    out_metadata->graphicsset_offset_present = 0;
    out_metadata->image_offset_present = 0;
    out_metadata->metadata_hash = dm2_gdat_file_receipt_hash(
        entry ? entry->data_index : 0u,
        ((uint32_t)out_metadata->width << 16) | out_metadata->height,
        ((uint32_t)out_metadata->bits_per_pixel << 16) |
            (uint16_t)out_metadata->query_offset_y,
        dm2_fnv1a_bytes(raw, raw_size));
    return 1;
}

static int dm2_v1_asset_load_image_local_palette(
    const DM2_V1_AssetLoader *loader,
    int category,
    int index,
    int field,
    uint8_t out_palette16[16],
    uint32_t *out_hash) {
    const DM2_V1_GdatEntry *entry;
    const uint8_t *raw;
    size_t raw_size = 0u;
    uint16_t width;
    uint16_t height;
    uint16_t bpp = 0u;
    size_t palette_offset;

    if (out_hash) *out_hash = 0u;
    if (!out_palette16) return 0;
    memset(out_palette16, 0, 16u);
    entry = dm2_gdat_find_entry(loader, category, index,
                                DM2_GDAT_ENTRY_TYPE_IMAGE, field);
    raw = dm2_gdat_raw_from_entry(loader, entry, &raw_size);
    if (!raw || raw_size < DM2_IMG3_HEADER_SIZE + DM2_IMG_LOCAL_PALETTE_SIZE ||
        !dm2_img3_raw_bits_per_pixel(raw, raw_size, &bpp, loader->big_endian) ||
        bpp != 4u) {
        return 0;
    }
    width = (uint16_t)(img_rd16(raw, loader->big_endian) & 0x03ffu);
    height = (uint16_t)(img_rd16(raw + 2u, loader->big_endian) & 0x03ffu);
    if (width == 0u || height == 0u) return 0;
    /* SKProject's QUERY_GDAT_IMAGE_LOCALPAL returns
     * `raw + QUERY_GDAT_RAW_DATA_LENGTH(didx) - 0x10` for every accepted
     * 4-bpp image (SKULLWIN/c_querydb.cpp:228-253).  C4 command streams are
     * variable length, so their palette cannot be found from width*height:
     * that calculation can point into compressed pixels. */
    palette_offset = raw_size - DM2_IMG_LOCAL_PALETTE_SIZE;
    memcpy(out_palette16, raw + palette_offset, DM2_IMG_LOCAL_PALETTE_SIZE);
    if (out_hash) {
        *out_hash = dm2_fnv1a_bytes(out_palette16,
                                    DM2_IMG_LOCAL_PALETTE_SIZE);
    }
    return 1;
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
    /* SKProject c_gfx_decode.cpp::decode_img9 dispatches mode 1, 2, or 3.
     * The former local shortcut decoded every non-2 stream as mode 3, so a
     * real mode-1 GDAT image was silently interpreted as another format.
     * Keep the active loader on the complete source-locked decoder shared by
     * the focused decoder receipt instead of maintaining a partial copy. */
    return dm2_v1_decode_img9(raw, raw_size, width, height, out_format);
}

/* ── Public API ─────────────────────────────────────────────────── */

int dm2_v1_asset_loader_init(DM2_V1_AssetLoader *loader,
                              const uint8_t *data, size_t size) {
    if (!loader) return -1;
    memset(loader, 0, sizeof(*loader));

    if (!data || size < DM2_GDAT_HEADER_SIZE + 4) return -1;

    uint16_t first_word_le = rd16le(data + 0);
    uint16_t first_word_be = rd16be(data + 0);
    int be = 0;
    uint16_t first_word;
    uint16_t raw_count;

    /* Detect endianness: Mac/Amiga 68k store words in big-endian order */
    if ((first_word_le & 0x8000u) != 0) {
        first_word = first_word_le;
    } else if ((first_word_be & 0x8000u) != 0) {
        first_word = first_word_be;
        be = 1;
    } else {
        return -1;
    }

    if ((first_word & 0x7fffu) != 5u &&
        (first_word & 0x7fffu) != 4u &&
        (first_word & 0x7fffu) != 2u) {
        return -1;
    }
    int is_pc = (first_word == DM2_PC_GDAT_CONTAINER_WORD &&
                 size >= DM2_PC_GRAPHICS_MIN_SIZE &&
                 size <= DM2_PC_GRAPHICS_MAX_SIZE);
    int is_fmtowns = (first_word == DM2_FMTOWNS_GDAT_CONTAINER_WORD &&
                      size >= DM2_FMTOWNS_GRAPHICS_MIN_SIZE &&
                      size <= DM2_FMTOWNS_GRAPHICS_MAX_SIZE);
    int is_pc9821 = (!be && first_word == DM2_PC_GDAT_CONTAINER_WORD &&
                     size >= DM2_PC9821_GRAPHICS_MIN_SIZE &&
                     size < DM2_PC_GRAPHICS_MIN_SIZE);
    int is_be = (be && (first_word == DM2_PC_GDAT_CONTAINER_WORD) &&
                 size >= (1U * 1024U * 1024U));
    if (!is_pc && !is_fmtowns && !is_pc9821 && !is_be) return -1;

    loader->data = data;
    loader->data_size = size;
    loader->big_endian = be;
    loader->category_count = DM2_GDAT_CATEGORY_LIMIT + 1;
    loader->gdat_version = (uint16_t)(first_word & 0x7fffu);
    raw_count = be ? rd16be(data + 2) : rd16le(data + 2);
    loader->raw_data_count = raw_count;
    if (dm2_gdat_parse_raw_table(loader, raw_count) != 0 ||
        dm2_gdat_parse_ent1(loader) != 0) {
        dm2_v1_asset_loader_free(loader);
        return -1;
    }

    loader->loaded = 1;
    return 0;
}

int dm2_v1_asset_loader_validate_typed_graph(const DM2_V1_AssetLoader *loader) {
    uint16_t i;

    if (!loader || !loader->loaded || !loader->data || !loader->entries ||
        !loader->raw_offsets || !loader->raw_sizes ||
        loader->raw_data_count == 0 || loader->entry_count == 0) {
        return 0;
    }

    for (i = 0; i < loader->entry_count; ++i) {
        const DM2_V1_GdatEntry *entry = &loader->entries[i];
        uint16_t raw_index;

        if (!dm2_gdat_entry_owns_raw_payload(entry)) {
            continue;
        }
        raw_index = (uint16_t)(entry->data_index & 0x7fffu);
        if (raw_index >= loader->raw_data_count ||
            loader->raw_sizes[raw_index] == 0 ||
            (uint64_t)loader->raw_offsets[raw_index] +
                loader->raw_sizes[raw_index] > loader->data_size) {
            return 0;
        }
    }
    return 1;
}

const uint8_t *dm2_v1_asset_load(const DM2_V1_AssetLoader *loader,
                                   int category, int index, int field) {
    return dm2_v1_asset_load_sized(loader, category, index, field, NULL);
}

const uint8_t *dm2_v1_asset_load_sized(const DM2_V1_AssetLoader *loader,
                                        int category, int index, int field,
                                        size_t *out_size) {
    uint16_t i;

    if (out_size) *out_size = 0;
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
        if (!dm2_gdat_entry_owns_raw_payload(entry)) continue;
        if (raw_index >= loader->raw_data_count) return NULL;
        if (loader->raw_sizes[raw_index] == 0) return NULL;
        if ((uint64_t)loader->raw_offsets[raw_index] +
            loader->raw_sizes[raw_index] > loader->data_size) {
            return NULL;
        }
        if (out_size) *out_size = loader->raw_sizes[raw_index];
        return loader->data + loader->raw_offsets[raw_index];
    }
    return NULL;
}

const uint8_t *dm2_v1_asset_load_typed_sized(
    const DM2_V1_AssetLoader *loader,
    int category,
    int index,
    int type,
    int field,
    size_t *out_size)
{
    const DM2_V1_GdatEntry *entry;

    if (out_size) *out_size = 0;
    entry = dm2_gdat_find_entry(loader, category, index, type, field);
    return dm2_gdat_raw_from_entry(loader, entry, out_size);
}

const uint8_t *dm2_v1_asset_load_text_sized(
    const DM2_V1_AssetLoader *loader,
    int category,
    int index,
    int field,
    size_t *out_size)
{
    return dm2_v1_asset_load_typed_sized(loader,
                                         category,
                                         index,
                                         DM2_GDAT_ENTRY_TYPE_TEXT,
                                         field,
                                         out_size);
}

static int dm2_gdat_raw_bounds(const DM2_V1_AssetLoader *loader,
                               uint16_t raw_index,
                               uint32_t *out_offset,
                               uint32_t *out_size)
{
    uint32_t offset;
    uint32_t size;

    if (out_offset) *out_offset = 0u;
    if (out_size) *out_size = 0u;
    if (!loader || !loader->loaded || !loader->data || !loader->raw_offsets ||
        !loader->raw_sizes || raw_index >= loader->raw_data_count) {
        return 0;
    }
    offset = loader->raw_offsets[raw_index];
    size = loader->raw_sizes[raw_index];
    if (size == 0u || (uint64_t)offset + size > loader->data_size) {
        return 0;
    }
    if (out_offset) *out_offset = offset;
    if (out_size) *out_size = size;
    return 1;
}

static uint32_t dm2_gdat_entry_receipt_hash(
    int category, int index, int type, int field, uint16_t data_index,
    uint16_t raw_index, uint32_t raw_file_pos, uint32_t raw_length)
{
    uint32_t hash = 2166136261u;

    hash = (hash ^ (uint32_t)(uint8_t)category) * 16777619u;
    hash = (hash ^ (uint32_t)(uint8_t)index) * 16777619u;
    hash = (hash ^ (uint32_t)(uint8_t)type) * 16777619u;
    hash = (hash ^ (uint32_t)(uint8_t)field) * 16777619u;
    hash = (hash ^ data_index) * 16777619u;
    hash = (hash ^ raw_index) * 16777619u;
    hash = (hash ^ raw_file_pos) * 16777619u;
    hash = (hash ^ raw_length) * 16777619u;
    return hash ? hash : 1u;
}

static uint32_t dm2_gdat_pict_receipt_hash(
    uint32_t tag, uint16_t raw_index, uint16_t width, uint16_t height,
    uint8_t bpp, uint8_t pool, uint32_t allocation_bytes,
    uint32_t free_bytes)
{
    uint32_t hash = 2166136261u;

    hash = (hash ^ tag) * 16777619u;
    hash = (hash ^ raw_index) * 16777619u;
    hash = (hash ^ width) * 16777619u;
    hash = (hash ^ height) * 16777619u;
    hash = (hash ^ (uint32_t)bpp) * 16777619u;
    hash = (hash ^ (uint32_t)pool) * 16777619u;
    hash = (hash ^ allocation_bytes) * 16777619u;
    hash = (hash ^ free_bytes) * 16777619u;
    return hash ? hash : 1u;
}

static int dm2_gdat_pict_row_bytes(uint16_t width,
                                   uint16_t height,
                                   uint8_t bpp,
                                   uint16_t *out_row_bytes,
                                   uint32_t *out_payload_bytes)
{
    uint32_t row_bytes;
    uint32_t payload_bytes;

    if (out_row_bytes) *out_row_bytes = 0u;
    if (out_payload_bytes) *out_payload_bytes = 0u;
    if (width == 0u || height == 0u || (bpp != 4u && bpp != 8u)) {
        return 0;
    }
    row_bytes = bpp == 4u ? (uint32_t)(((width + 1u) & 0xfffeu) >> 1)
                          : (uint32_t)width;
    payload_bytes = row_bytes * (uint32_t)height;
    if (row_bytes > 0xffffu || payload_bytes > 0xffffu) {
        return 0;
    }
    if (out_row_bytes) *out_row_bytes = (uint16_t)row_bytes;
    if (out_payload_bytes) *out_payload_bytes = payload_bytes;
    return 1;
}

int dm2_v1_query_gdat_raw_data_file_pos(
    const DM2_V1_AssetLoader *loader,
    uint16_t raw_index,
    uint32_t *out_file_pos)
{
    uint32_t offset = 0u;

    if (out_file_pos) *out_file_pos = 0u;
    if (!out_file_pos ||
        !dm2_gdat_raw_bounds(loader, raw_index, &offset, NULL)) {
        return 0;
    }
    /* skproject c_gdatfile.cpp::QUERY_GDAT_RAW_DATA_FILE_POS resolves the
     * raw payload index through the loaded raw-data offset table. */
    *out_file_pos = offset;
    return 1;
}

int dm2_v1_query_gdat_raw_data_length(
    const DM2_V1_AssetLoader *loader,
    uint16_t raw_index,
    uint32_t *out_length)
{
    uint32_t size = 0u;

    if (out_length) *out_length = 0u;
    if (!out_length || !dm2_gdat_raw_bounds(loader, raw_index, NULL, &size)) {
        return 0;
    }
    /* skproject c_dballoc/c_gdatfile.cpp::QUERY_GDAT_RAW_DATA_LENGTH returns
     * the exact raw-table byte count; it is not an image dimension. */
    *out_length = size;
    return 1;
}

const uint8_t *dm2_v1_load_gdat_raw_data(
    const DM2_V1_AssetLoader *loader,
    uint16_t raw_index,
    size_t *out_size)
{
    uint32_t offset = 0u;
    uint32_t size = 0u;

    if (out_size) *out_size = 0u;
    if (!dm2_gdat_raw_bounds(loader, raw_index, &offset, &size)) {
        return NULL;
    }
    if (out_size) *out_size = size;
    return loader->data + offset;
}

int dm2_v1_query_gdat_entry(
    const DM2_V1_AssetLoader *loader,
    int category,
    int index,
    int type,
    int field,
    DM2_V1_GdatEntryQueryReceipt *out_receipt)
{
    const DM2_V1_GdatEntry *entry;
    uint16_t raw_index;
    uint32_t raw_file_pos = 0u;
    uint32_t raw_length = 0u;
    int loadable_raw = 0;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    entry = dm2_gdat_find_entry(loader, category, index, type, field);
    if (!entry) return 0;

    raw_index = (uint16_t)(entry->data_index & 0x7fffu);
    if (type != DM2_GDAT_ENTRY_TYPE_WORD_VALUE &&
        type != DM2_GDAT_ENTRY_TYPE_IMAGE_OFFSET) {
        loadable_raw = dm2_gdat_raw_bounds(loader, raw_index,
                                           &raw_file_pos, &raw_length);
        if (!loadable_raw) return 0;
    }

    out_receipt->present = 1u;
    out_receipt->loadable_raw = (uint8_t)loadable_raw;
    out_receipt->category = entry->cls1;
    out_receipt->index = entry->cls2;
    out_receipt->type = entry->cls3;
    out_receipt->field = entry->cls4;
    out_receipt->data_index = entry->data_index;
    out_receipt->raw_index = raw_index;
    out_receipt->raw_file_pos = raw_file_pos;
    out_receipt->raw_length = raw_length;
    out_receipt->receipt_hash = dm2_gdat_entry_receipt_hash(
        category, index, type, field, entry->data_index, raw_index,
        raw_file_pos, raw_length);
    return 1;
}

int dm2_v1_query_gdat_entry_value(
    const DM2_V1_AssetLoader *loader,
    uint16_t entry_ordinal,
    uint8_t group_index,
    uint32_t *out_value)
{
    const DM2_V1_GdatEntry *entry;

    if (out_value) *out_value = 0u;
    if (!loader || !loader->loaded || !loader->entries || !out_value ||
        entry_ordinal >= loader->entry_count ||
        group_index >= DM2_GDAT_EP_COUNT) {
        return 0;
    }
    if (loader->ent1_entry_stride != 0u &&
        !loader->ent1_ep_present[group_index] &&
        group_index > DM2_GDAT_EP_DATA) {
        return 0;
    }

    entry = &loader->entries[entry_ordinal];
    switch (group_index) {
    case DM2_GDAT_EP_CLS1:
        *out_value = entry->cls1;
        return 1;
    case DM2_GDAT_EP_CLS2:
        *out_value = entry->cls2;
        return 1;
    case DM2_GDAT_EP_CLS3:
        *out_value = entry->cls3;
        return 1;
    case DM2_GDAT_EP_CLS4:
        *out_value = entry->cls4;
        return 1;
    case DM2_GDAT_EP_DATA:
        *out_value = entry->data_index;
        return 1;
    case DM2_GDAT_EP_CLS5:
        *out_value = entry->cls5;
        return 1;
    case DM2_GDAT_EP_CLS6:
        *out_value = entry->cls6;
        return 1;
    default:
        return 0;
    }
}

int dm2_v1_query_gdat_entry_data_index(
    const DM2_V1_AssetLoader *loader,
    int category,
    int index,
    int type,
    int field,
    uint16_t *out_data_index)
{
    const DM2_V1_GdatEntry *entry;

    if (out_data_index) *out_data_index = 0u;
    if (!out_data_index) return 0;
    entry = dm2_gdat_find_entry(loader, category, index, type, field);
    if (!entry) return 0;
    *out_data_index = entry->data_index;
    return 1;
}

const uint8_t *dm2_v1_query_gdat_entry_data_ptr(
    const DM2_V1_AssetLoader *loader,
    int category,
    int index,
    int type,
    int field,
    size_t *out_size)
{
    const DM2_V1_GdatEntry *entry;

    if (out_size) *out_size = 0u;
    if (type == DM2_GDAT_ENTRY_TYPE_WORD_VALUE ||
        type == DM2_GDAT_ENTRY_TYPE_IMAGE_OFFSET) {
        return NULL;
    }
    entry = dm2_gdat_find_entry(loader, category, index, type, field);
    return dm2_gdat_raw_from_entry(loader, entry, out_size);
}

const uint8_t *dm2_v1_direct_query_gdat_entry_data_buff_receipt(
    const DM2_V1_AssetLoader *loader,
    int category,
    int index,
    int type,
    int field,
    size_t *out_size,
    DM2_V1_DirectGdatEntryDataBuffReceipt *out_receipt)
{
    DM2_V1_GdatEntryQueryReceipt query;
    DM2_V1_DirectGdatEntryDataBuffReceipt receipt;
    const uint8_t *ptr;
    size_t size = 0u;

    if (out_size) *out_size = 0u;
    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    memset(&receipt, 0, sizeof(receipt));
    if (!out_receipt) return NULL;
    if (!dm2_v1_query_gdat_entry_if_loadable(loader, category, index, type,
                                             field, &query)) {
        return NULL;
    }
    ptr = dm2_v1_query_gdat_entry_data_ptr(loader, category, index, type,
                                           field, &size);
    if (!ptr || size == 0u || size != (size_t)query.raw_length) {
        return NULL;
    }

    receipt.accepted = 1u;
    receipt.category = (uint8_t)category;
    receipt.index = (uint8_t)index;
    receipt.type = (uint8_t)type;
    receipt.field = (uint8_t)field;
    receipt.raw_index = query.raw_index;
    receipt.data_index = query.data_index;
    receipt.raw_length = query.raw_length;
    receipt.raw_hash = dm2_fnv1a_bytes(ptr, size);
    receipt.receipt_hash = dm2_gdat_file_receipt_hash(
        query.receipt_hash,
        ((uint32_t)receipt.category << 24) |
            ((uint32_t)receipt.index << 16) |
            ((uint32_t)receipt.type << 8) | receipt.field,
        receipt.raw_length,
        receipt.raw_hash);
    *out_receipt = receipt;
    if (out_size) *out_size = size;
    return ptr;
}

const uint8_t *dm2_v1_direct_query_gdat_text_receipt(
    const DM2_V1_AssetLoader *loader,
    int category,
    int index,
    int field,
    size_t *out_size,
    DM2_V1_DirectGdatTextReceipt *out_receipt)
{
    DM2_V1_DirectGdatEntryDataBuffReceipt data;
    DM2_V1_DirectGdatTextReceipt receipt;
    const uint8_t *ptr;
    size_t size = 0u;

    if (out_size) *out_size = 0u;
    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    memset(&receipt, 0, sizeof(receipt));
    if (!out_receipt) return NULL;
    ptr = dm2_v1_direct_query_gdat_entry_data_buff_receipt(
        loader, category, index, DM2_GDAT_ENTRY_TYPE_TEXT, field, &size,
        &data);
    if (!ptr || size == 0u) return NULL;

    receipt.accepted = 1u;
    receipt.category = data.category;
    receipt.index = data.index;
    receipt.field = data.field;
    receipt.raw_index = data.raw_index;
    receipt.data_index = data.data_index;
    receipt.text_length = data.raw_length;
    receipt.text_hash = data.raw_hash;
    receipt.receipt_hash = dm2_gdat_file_receipt_hash(
        data.receipt_hash, DM2_GDAT_ENTRY_TYPE_TEXT, receipt.text_length,
        receipt.text_hash);
    *out_receipt = receipt;
    if (out_size) *out_size = size;
    return ptr;
}

int dm2_v1_query_gdat_entry_data_length(
    const DM2_V1_AssetLoader *loader,
    int category,
    int index,
    int type,
    int field,
    uint32_t *out_length)
{
    DM2_V1_GdatEntryQueryReceipt receipt;

    if (out_length) *out_length = 0u;
    if (!out_length ||
        !dm2_v1_query_gdat_entry(loader, category, index, type, field,
                                 &receipt) ||
        !receipt.loadable_raw) {
        return 0;
    }
    *out_length = receipt.raw_length;
    return 1;
}

int dm2_v1_query_gdat_entry_if_loadable(
    const DM2_V1_AssetLoader *loader,
    int category,
    int index,
    int type,
    int field,
    DM2_V1_GdatEntryQueryReceipt *out_receipt)
{
    DM2_V1_GdatEntryQueryReceipt receipt;

    if (!dm2_v1_query_gdat_entry(loader, category, index, type, field,
                                 &receipt) ||
        !receipt.loadable_raw) {
        if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
        return 0;
    }
    if (out_receipt) *out_receipt = receipt;
    return 1;
}

int dm2_v1_load_gdat_entry_data_to(
    const DM2_V1_AssetLoader *loader,
    int category,
    int index,
    int type,
    int field,
    uint8_t *destination,
    size_t destination_capacity,
    DM2_V1_GdatEntryQueryReceipt *out_receipt)
{
    DM2_V1_GdatEntryQueryReceipt receipt;
    const uint8_t *payload;
    size_t payload_size = 0u;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!destination ||
        !dm2_v1_query_gdat_entry_if_loadable(loader, category, index,
                                             type, field, &receipt)) {
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    payload = dm2_v1_load_gdat_raw_data(loader, receipt.raw_index,
                                        &payload_size);
    if (!payload || payload_size != receipt.raw_length ||
        destination_capacity < payload_size) {
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    memcpy(destination, payload, payload_size);
    receipt.copied_to_destination = 1u;
    receipt.copied_length = (uint32_t)payload_size;
    receipt.receipt_hash = dm2_gdat_entry_receipt_hash(
        category, index, type, field, receipt.data_index, receipt.raw_index,
        receipt.raw_file_pos, receipt.copied_length);
    if (out_receipt) *out_receipt = receipt;
    return 1;
}

int dm2_v1_load_ent1_receipt(
    const DM2_V1_AssetLoader *loader,
    DM2_V1_GdatEnt1Receipt *out_receipt)
{
    const uint8_t *ent;
    uint16_t (*rd16)(const uint8_t *);
    uint16_t entry_count;
    uint16_t group_count;
    uint32_t stride = 0u;
    uint32_t pos = 6u;
    uint16_t g;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (!loader || !loader->loaded || !loader->data ||
        !loader->raw_offsets || !loader->raw_sizes ||
        loader->raw_data_count == 0u || loader->raw_sizes[0] < 6u) {
        return 0;
    }
    ent = loader->data + loader->raw_offsets[0];
    if (rd16le(ent) == DM2_PC_GDAT_ENT1_WORD) {
        rd16 = rd16le;
        out_receipt->endian_swapped = 0u;
    } else if (rd16be(ent) == DM2_PC_GDAT_ENT1_WORD) {
        rd16 = rd16be;
        out_receipt->endian_swapped = 1u;
    } else {
        return 0;
    }
    entry_count = rd16(ent + 2);
    group_count = rd16(ent + 4);
    if (entry_count == 0u || group_count == 0u ||
        6u + ((uint32_t)group_count * 2u) > loader->raw_sizes[0]) {
        return 0;
    }
    for (g = 0; g < group_count; ++g) {
        int ep = dm2_gdat_ent1_group_id_to_ep(ent[pos]);
        uint8_t len = ent[pos + 1u];
        if (ep >= 0 && ep < DM2_GDAT_EP_COUNT) {
            out_receipt->ep_present[ep] = 1u;
            out_receipt->ep_lengths[ep] = len;
            out_receipt->ep_offsets[ep] = (uint16_t)stride;
        }
        stride += len;
        pos += 2u;
    }
    if (stride == 0u ||
        pos + ((uint32_t)entry_count * stride) > loader->raw_sizes[0]) {
        memset(out_receipt, 0, sizeof(*out_receipt));
        return 0;
    }
    out_receipt->valid = 1u;
    out_receipt->entry_count = entry_count;
    out_receipt->group_count = (uint8_t)group_count;
    out_receipt->entry_stride = (uint16_t)stride;
    out_receipt->raw0_length = loader->raw_sizes[0];
    out_receipt->receipt_hash = dm2_fnv1a_bytes(ent, loader->raw_sizes[0]);
    return 1;
}

int dm2_v1_load_gdat_entries_receipt(
    const DM2_V1_AssetLoader *loader,
    DM2_V1_GdatLoadEntriesReceipt *out_receipt)
{
    uint16_t i;
    uint32_t hash = 2166136261u;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (!loader || !loader->loaded || !loader->entries ||
        !loader->raw_offsets || !loader->raw_sizes) {
        return 0;
    }

    for (i = 0; i < loader->entry_count; ++i) {
        const DM2_V1_GdatEntry *entry = &loader->entries[i];
        uint16_t raw_index = (uint16_t)(entry->data_index & 0x7fffu);
        uint32_t raw_length = 0u;

        if (entry->cls3 == DM2_GDAT_ENTRY_TYPE_WORD_VALUE ||
            entry->cls3 == DM2_GDAT_ENTRY_TYPE_IMAGE_OFFSET) {
            ++out_receipt->scalar_entry_count;
            continue;
        }
        if (!dm2_gdat_raw_bounds(loader, raw_index, NULL, &raw_length)) {
            ++out_receipt->rejected_raw_count;
            continue;
        }
        ++out_receipt->loadable_entry_count;
        out_receipt->payload_bytes += raw_length;
        out_receipt->allocated_bytes_with_length_words += raw_length + 2u;
        hash = (hash ^ entry->cls1) * 16777619u;
        hash = (hash ^ entry->cls2) * 16777619u;
        hash = (hash ^ entry->cls3) * 16777619u;
        hash = (hash ^ entry->cls4) * 16777619u;
        hash = (hash ^ raw_index) * 16777619u;
        hash = (hash ^ raw_length) * 16777619u;
    }
    out_receipt->valid = 1u;
    out_receipt->entry_count = loader->entry_count;
    out_receipt->receipt_hash = hash ? hash : 1u;
    return 1;
}

int dm2_v1_gdat_dyn4_selection_receipt(
    const DM2_V1_AssetLoader *loader,
    uint32_t resource_id,
    DM2_V1_GdatDyn4SelectionReceipt *out_receipt)
{
    uint16_t i;
    uint32_t hash = 2166136261u;
    uint8_t category = (uint8_t)(resource_id >> 24);
    uint8_t index = (uint8_t)(resource_id >> 16);
    uint8_t type = (uint8_t)(resource_id >> 8);
    uint8_t field = (uint8_t)resource_id;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (!loader || !loader->loaded || !loader->entries ||
        !loader->raw_offsets || !loader->raw_sizes) {
        return 0;
    }

    /* SKProject SKULLWIN/c_gdatfile.cpp::DM2_QUERY_NEXT_GDAT_ENTRY
     * (lines 41-75, 98-128, 202-217): each descriptor byte is exact unless
     * it is 0xff, in which case that dimension spans the original table.
     * DM2_LOAD_DYN4 (lines 1345-1351) omits scalar dtWordValue/
     * dtImageOffset rows and high-bit data indexes before it marks the
     * dynamic cache. */
    for (i = 0; i < loader->entry_count; ++i) {
        const DM2_V1_GdatEntry *entry = &loader->entries[i];
        uint16_t raw_index;
        uint32_t raw_length = 0u;

        if ((category != 0xffu && entry->cls1 != category) ||
            (index != 0xffu && entry->cls2 != index) ||
            (type != 0xffu && entry->cls3 != type) ||
            (field != 0xffu && entry->cls4 != field)) {
            continue;
        }
        ++out_receipt->matched_entry_count;
        if (entry->cls3 == DM2_GDAT_ENTRY_TYPE_WORD_VALUE ||
            entry->cls3 == DM2_GDAT_ENTRY_TYPE_IMAGE_OFFSET) {
            ++out_receipt->scalar_entry_count;
            continue;
        }
        if ((entry->data_index & 0x8000u) != 0u) {
            ++out_receipt->high_bit_data_index_count;
            continue;
        }
        raw_index = entry->data_index;
        if (!dm2_gdat_raw_bounds(loader, raw_index, NULL, &raw_length)) {
            ++out_receipt->rejected_raw_count;
            continue;
        }
        ++out_receipt->raw_loadable_entry_count;
        if (entry->cls3 == DM2_GDAT_ENTRY_TYPE_SOUND) {
            ++out_receipt->sound_entry_count;
        }
        out_receipt->payload_bytes += raw_length;
        hash = (hash ^ entry->cls1) * 16777619u;
        hash = (hash ^ entry->cls2) * 16777619u;
        hash = (hash ^ entry->cls3) * 16777619u;
        hash = (hash ^ entry->cls4) * 16777619u;
        hash = (hash ^ raw_index) * 16777619u;
        hash = (hash ^ raw_length) * 16777619u;
    }
    out_receipt->valid = 1u;
    out_receipt->category = category;
    out_receipt->index = index;
    out_receipt->type = type;
    out_receipt->field = field;
    out_receipt->receipt_hash = hash ? hash : 1u;
    return 1;
}

int dm2_v1_query_next_gdat_entry(
    const DM2_V1_AssetLoader *loader,
    DM2_V1_GdatEntryIterator *iterator,
    DM2_V1_GdatEntryQueryReceipt *out_receipt)
{
    uint16_t i;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!loader || !loader->loaded || !loader->entries || !iterator ||
        !out_receipt) {
        return 0;
    }
    if (iterator->category_first < 0) iterator->category_first = 0;
    if (iterator->category_last < 0 ||
        iterator->category_last > DM2_GDAT_CATEGORY_LIMIT) {
        iterator->category_last = DM2_GDAT_CATEGORY_LIMIT;
    }
    if (iterator->category_first > iterator->category_last) return 0;

    for (i = iterator->cursor; i < loader->entry_count; ++i) {
        const DM2_V1_GdatEntry *entry = &loader->entries[i];
        if ((int)entry->cls1 < iterator->category_first ||
            (int)entry->cls1 > iterator->category_last ||
            (iterator->index_filter >= 0 &&
             (int)entry->cls2 != iterator->index_filter) ||
            (iterator->type_filter >= 0 &&
             (int)entry->cls3 != iterator->type_filter) ||
            (iterator->field_filter >= 0 &&
             (int)entry->cls4 != iterator->field_filter)) {
            continue;
        }
        iterator->cursor = (uint16_t)(i + 1u);
        return dm2_v1_query_gdat_entry(loader, entry->cls1, entry->cls2,
                                       entry->cls3, entry->cls4,
                                       out_receipt);
    }
    iterator->cursor = loader->entry_count;
    return 0;
}

int dm2_v1_gdat_sound_toggle_payload(
    uint8_t *payload,
    uint16_t payload_length,
    uint16_t header_state,
    uint8_t header_flags,
    DM2_V1_GdatSoundToggleReceipt *out_receipt)
{
    uint16_t i;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    out_receipt->header_state_before = (uint8_t)header_state;
    out_receipt->header_state_after = (uint8_t)header_state;
    out_receipt->flags_before = header_flags;
    out_receipt->flags_after = header_flags;
    out_receipt->payload_hash_before = dm2_fnv1a_bytes(payload,
                                                       payload_length);
    out_receipt->payload_hash_after = out_receipt->payload_hash_before;
    if (!payload || payload_length == 0u || header_state == 1u) {
        return 0;
    }
    if (header_state != 0u) {
        out_receipt->header_state_after = 1u;
    } else {
        if ((header_flags & 1u) == 0u) return 0;
        out_receipt->flags_after = (uint8_t)(header_flags & (uint8_t)~1u);
    }
    for (i = 0; i < payload_length; ++i) {
        payload[i] ^= 0x80u;
    }
    out_receipt->accepted = 1u;
    out_receipt->toggled_bytes = payload_length;
    out_receipt->payload_hash_after = dm2_fnv1a_bytes(payload,
                                                      payload_length);
    return 1;
}

int dm2_v1_gdat_sound_entry_receipt(
    const DM2_V1_AssetLoader *loader,
    int category,
    int index,
    int field,
    int sound7_result,
    int extended_header,
    DM2_V1_GdatSoundEntryReceipt *out_receipt)
{
    DM2_V1_GdatEntryQueryReceipt entry_receipt;
    uint32_t skip;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (sound7_result != 0 ||
        !dm2_v1_query_gdat_entry(loader, category, index,
                                 DM2_GDAT_ENTRY_TYPE_SOUND, field,
                                 &entry_receipt) ||
        !entry_receipt.loadable_raw) {
        return 0;
    }
    skip = extended_header ? 6u : 2u;
    if (entry_receipt.raw_length <= skip) return 0;
    out_receipt->accepted = 1u;
    out_receipt->category = (uint8_t)category;
    out_receipt->index = (uint8_t)index;
    out_receipt->field = (uint8_t)field;
    out_receipt->header_skip_bytes = (uint8_t)skip;
    out_receipt->data_index = entry_receipt.data_index;
    out_receipt->raw_index = entry_receipt.raw_index;
    out_receipt->raw_length = entry_receipt.raw_length;
    out_receipt->payload_length = entry_receipt.raw_length - skip;
    out_receipt->payload_offset = entry_receipt.raw_file_pos + skip;
    out_receipt->receipt_hash = dm2_gdat_entry_receipt_hash(
        category, index, DM2_GDAT_ENTRY_TYPE_SOUND, field,
        entry_receipt.data_index, entry_receipt.raw_index,
        out_receipt->payload_offset, out_receipt->payload_length);
    return 1;
}

uint16_t dm2_v1_r_2bad4_swap_word(uint16_t value)
{
    /* skproject SKULLWIN/c_gdatfile.cpp R_2BAD4 byte-swaps a 16-bit word. */
    return (uint16_t)((value >> 8) | (uint16_t)(value << 8));
}

int dm2_v1_r_2d07d_max_raw_length_receipt(
    const DM2_V1_AssetLoader *loader,
    uint8_t type_filter,
    uint8_t field_filter,
    DM2_V1_GdatMaxRawLengthReceipt *out_receipt)
{
    DM2_V1_GdatEntryIterator iterator;
    DM2_V1_GdatEntryQueryReceipt entry;
    uint16_t max_raw = 0u;
    uint16_t scanned = 0u;
    uint32_t hash = 2166136261u;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (!loader || !loader->loaded || !loader->entries || !loader->raw_sizes) {
        return 0;
    }

    memset(&iterator, 0, sizeof(iterator));
    iterator.category_first = 0;
    iterator.category_last = DM2_GDAT_CATEGORY_LIMIT;
    iterator.index_filter = -1;
    iterator.type_filter = type_filter;
    iterator.field_filter = field_filter;
    while (dm2_v1_query_next_gdat_entry(loader, &iterator, &entry)) {
        ++scanned;
        if (entry.raw_length > 0xffffu) return 0;
        if ((uint16_t)entry.raw_length > max_raw) {
            max_raw = (uint16_t)entry.raw_length;
        }
        hash = (hash ^ entry.category) * 16777619u;
        hash = (hash ^ entry.index) * 16777619u;
        hash = (hash ^ entry.type) * 16777619u;
        hash = (hash ^ entry.field) * 16777619u;
        hash = (hash ^ entry.raw_index) * 16777619u;
        hash = (hash ^ entry.raw_length) * 16777619u;
    }

    out_receipt->accepted = 1u;
    out_receipt->type_filter = type_filter;
    out_receipt->field_filter = field_filter;
    out_receipt->scanned_entry_count = scanned;
    out_receipt->max_raw_length = max_raw;
    out_receipt->receipt_hash = hash ? hash : 1u;
    return 1;
}

static uint32_t dm2_gdat_file_receipt_hash(uint32_t a,
                                           uint32_t b,
                                           uint32_t c,
                                           uint32_t d)
{
    uint32_t words[4];

    words[0] = a;
    words[1] = b;
    words[2] = c;
    words[3] = d;
    return dm2_fnv1a_bytes((const uint8_t *)words, sizeof(words));
}

int dm2_v1_graphics_data_open_receipt(
    DM2_V1_GraphicsDataFileState *state,
    int primary_open_ok,
    int16_t primary_handle,
    int secondary_open_ok,
    int16_t secondary_handle,
    DM2_V1_GraphicsDataOpenReceipt *out_receipt)
{
    DM2_V1_GraphicsDataOpenReceipt receipt;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    memset(&receipt, 0, sizeof(receipt));
    if (!state) return 0;
    receipt.counter_before = state->file_open_counter;
    state->file_open_counter++;
    receipt.counter_after = state->file_open_counter;
    if (state->file_open_counter == 1) {
        if (!primary_open_ok) {
            receipt.blocked_primary_open = 1u;
            receipt.syserr_code = 0x29u;
            *out_receipt = receipt;
            return 0;
        }
        state->file_handle = primary_handle;
        receipt.opened_primary = 1u;
        if (!state->filetype1 && state->filetype2) {
            if (!secondary_open_ok) {
                receipt.blocked_secondary_open = 1u;
                receipt.syserr_code = 0x1fu;
                *out_receipt = receipt;
                return 0;
            }
            state->xfile_handle = secondary_handle;
            receipt.opened_secondary = 1u;
        }
    }
    receipt.valid = 1u;
    receipt.primary_handle = state->file_handle;
    receipt.secondary_handle = state->xfile_handle;
    receipt.receipt_hash = dm2_gdat_file_receipt_hash(
        (uint32_t)receipt.counter_before,
        (uint32_t)receipt.counter_after,
        (uint16_t)receipt.primary_handle,
        (uint16_t)receipt.secondary_handle);
    *out_receipt = receipt;
    return 1;
}

int dm2_v1_graphics_data_close_receipt(
    DM2_V1_GraphicsDataFileState *state,
    DM2_V1_GraphicsDataCloseReceipt *out_receipt)
{
    DM2_V1_GraphicsDataCloseReceipt receipt;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    memset(&receipt, 0, sizeof(receipt));
    if (!state) return 0;
    receipt.counter_before = state->file_open_counter;
    if (state->file_open_counter <= 0) {
        receipt.blocked_underflow = 1u;
        *out_receipt = receipt;
        return 0;
    }
    state->file_open_counter--;
    receipt.counter_after = state->file_open_counter;
    if (state->file_open_counter == 0) {
        receipt.closed_primary = 1u;
        if (!state->filetype1 && state->filetype2)
            receipt.closed_secondary = 1u;
    }
    receipt.primary_handle = state->file_handle;
    receipt.secondary_handle = state->xfile_handle;
    receipt.valid = 1u;
    receipt.receipt_hash = dm2_gdat_file_receipt_hash(
        (uint32_t)receipt.counter_before,
        (uint32_t)receipt.counter_after,
        (uint16_t)receipt.primary_handle,
        (uint16_t)receipt.secondary_handle);
    *out_receipt = receipt;
    return 1;
}

int dm2_v1_graphics_data_read_receipt(
    const DM2_V1_GraphicsDataFileState *state,
    uint32_t offset,
    uint32_t length,
    DM2_V1_GraphicsDataReadReceipt *out_receipt)
{
    DM2_V1_GraphicsDataReadReceipt receipt;
    uint32_t primary_len = length;
    uint32_t secondary_len = 0u;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    memset(&receipt, 0, sizeof(receipt));
    receipt.request_offset = offset;
    receipt.request_length = length;
    if (!state) {
        receipt.blocked_missing_state = 1u;
        *out_receipt = receipt;
        return 0;
    }
    receipt.primary_handle = state->file_handle;
    receipt.secondary_handle = state->xfile_handle;
    receipt.primary_offset = offset;
    if (state->filetype2) {
        int64_t secondary_offset = (int64_t)offset -
                                   (int64_t)state->primary_file_size;

        if (secondary_offset < 0) {
            int64_t overlap = secondary_offset + (int64_t)length;

            if (overlap > 0) {
                secondary_len = (uint32_t)overlap;
                receipt.secondary_offset = 0u;
            }
        } else {
            secondary_len = length;
            receipt.secondary_offset = (uint32_t)secondary_offset;
        }
        primary_len -= secondary_len;
    }
    receipt.primary_length = primary_len;
    receipt.secondary_length = secondary_len;
    receipt.uses_primary = primary_len != 0u;
    receipt.uses_secondary = secondary_len != 0u;
    receipt.crosses_secondary_split =
        (receipt.uses_primary && receipt.uses_secondary) ? 1u : 0u;
    receipt.valid = 1u;
    receipt.receipt_hash = dm2_gdat_file_receipt_hash(
        offset, length, primary_len, secondary_len);
    *out_receipt = receipt;
    return 1;
}

int dm2_v1_gdat_track_underlay(
    const DM2_V1_GdatUnderlayPair *pairs,
    size_t pair_count,
    uint16_t image_raw_index,
    int16_t *out_underlay_raw_index)
{
    size_t low = 0u;
    size_t high = pair_count;

    if (out_underlay_raw_index) *out_underlay_raw_index = -1;
    if (!pairs || !out_underlay_raw_index || pair_count == 0u) return 0;

    /* skproject c_gdatfile.cpp::DM2_TRACK_UNDERLAY performs a binary search
     * over sorted four-byte image->underlay pairs loaded from dtRaw8 0/0/0. */
    while (low < high) {
        size_t mid = low + ((high - low) >> 1);
        if (image_raw_index < pairs[mid].image_raw_index) {
            high = mid;
        } else if (image_raw_index > pairs[mid].image_raw_index) {
            low = mid + 1u;
        } else {
            *out_underlay_raw_index = pairs[mid].underlay_raw_index;
            return 1;
        }
    }
    return 0;
}

int dm2_v1_read_graphics_structure_receipt(
    const DM2_V1_AssetLoader *loader,
    DM2_V1_GraphicsStructureReceipt *out_receipt)
{
    DM2_V1_GraphicsStructureReceipt receipt;
    uint32_t max_end = 0u;
    uint32_t max_raw = 0u;
    uint32_t underlay_len = 0u;
    uint16_t i;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    memset(&receipt, 0, sizeof(receipt));
    if (!loader || !loader->loaded || !loader->raw_offsets ||
        !loader->raw_sizes || loader->raw_data_count == 0u) {
        return 0;
    }

    for (i = 0; i < loader->raw_data_count; ++i) {
        uint32_t end = loader->raw_offsets[i] + loader->raw_sizes[i];
        if (end > max_end) max_end = end;
        if (loader->raw_sizes[i] > max_raw) max_raw = loader->raw_sizes[i];
    }

    receipt.valid = 1u;
    receipt.versionlo = (uint8_t)loader->gdat_version;
    receipt.entries = loader->entry_count;
    receipt.raw_data_count = loader->raw_data_count;
    receipt.raw0_length = loader->raw_sizes[0];
    receipt.graphics_file_size = (uint32_t)loader->data_size;
    receipt.calculated_payload_end = max_end;
    receipt.max_raw_payload_length = max_raw;
    receipt.filetype1 = loader->data_size < max_end ? 1u : 0u;
    receipt.filetype2 = receipt.filetype1;
    if (dm2_v1_query_gdat_entry_data_length(
            loader, 0, 0, DM2_GDAT_ENTRY_TYPE_RAW8, 0, &underlay_len) &&
        underlay_len >= 4u && (underlay_len % 4u) == 0u) {
        receipt.has_underlay_table = 1u;
        receipt.underlay_pair_count = (uint16_t)(underlay_len / 4u);
    }
    receipt.receipt_hash = dm2_gdat_file_receipt_hash(
        receipt.raw0_length, receipt.graphics_file_size,
        receipt.calculated_payload_end, receipt.max_raw_payload_length);
    *out_receipt = receipt;
    return 1;
}

int dm2_v1_extract_gdat_image_receipt(
    const DM2_V1_AssetLoader *loader,
    uint16_t raw_index,
    int gfxalloc_done,
    int prefer_hi_pool,
    const DM2_V1_GdatUnderlayPair *underlays,
    size_t underlay_count,
    DM2_V1_GdatImageExtractReceipt *out_receipt)
{
    DM2_V1_GdatImageExtractReceipt receipt;
    const uint8_t *raw;
    uint8_t *pixels = NULL;
    size_t raw_size = 0u;
    uint16_t cy;
    uint16_t bpp = 0u;
    uint32_t row_bytes;
    int16_t underlay_raw = -1;
    DM2_ImageFormat fmt = DM2_IMG_FMT_UNKNOWN;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    memset(&receipt, 0, sizeof(receipt));
    raw = dm2_v1_load_gdat_raw_data(loader, raw_index, &raw_size);
    if (!raw || raw_size < DM2_IMG3_HEADER_SIZE) return 0;

    cy = img_rd16(raw + 2, loader->big_endian);
    receipt.width = (uint16_t)(img_rd16(raw, loader->big_endian) & 0x03ffu);
    receipt.height = (uint16_t)(cy & 0x03ffu);
    if (receipt.width == 0u || receipt.height == 0u ||
        !dm2_img3_raw_bits_per_pixel(raw, raw_size, &bpp, loader->big_endian)) {
        return 0;
    }

    receipt.valid = 1u;
    receipt.raw_index = raw_index;
    receipt.raw_length = (uint32_t)raw_size;
    receipt.bpp = (uint8_t)bpp;
    receipt.gfxalloc_done = gfxalloc_done ? 1u : 0u;
    receipt.prefer_hi_pool = prefer_hi_pool ? 1u : 0u;
    receipt.underlay_raw_index = 0xffffu;
    if (dm2_v1_gdat_track_underlay(underlays, underlay_count, raw_index,
                                   &underlay_raw)) {
        receipt.uses_underlay = 1u;
        receipt.decode_img3_overlay = 1u;
        receipt.underlay_raw_index = (uint16_t)underlay_raw;
    } else if (dm2_img3_signed_offset(cy) == -32) {
        /* DME.h IMG3::Getpf routes OffsetY == -32 through the native
         * uncompressed U4/U8 payload, before the IMG9 branch.  Greatstone's
         * four IMG11 records are the U8 form: their bpp word is 8 but their
         * pixels start directly at IMG3+10, not in an IMG9 command stream. */
        pixels = dm2_decode_uncompressed_image(raw, raw_size,
                                               receipt.width,
                                               receipt.height,
                                               bpp, &fmt);
    } else if (bpp == 8u) {
        receipt.decode_img9 = 1u;
        pixels = dm2_decode_img9_c8(raw, raw_size, receipt.width,
                                    receipt.height, &fmt);
    } else {
        receipt.decode_img3_underlay = 1u;
        pixels = dm2_decode_img3_c4(raw, raw_size, receipt.width,
                                    receipt.height, &fmt);
    }

    row_bytes = bpp == 4u ? (uint32_t)(((receipt.width + 1u) & 0xfffeu) >> 1)
                          : (uint32_t)receipt.width;
    receipt.pixel_payload_bytes = row_bytes * (uint32_t)receipt.height;
    if (!gfxalloc_done && bpp == 4u) receipt.pixel_payload_bytes += 16u;
    receipt.allocation_bytes = receipt.pixel_payload_bytes +
        (gfxalloc_done ? 0x16u : 0x0eu);
    if (pixels) {
        receipt.decoded_pixel_hash = dm2_fnv1a_bytes(
            pixels, (size_t)receipt.width * (size_t)receipt.height);
        free(pixels);
    }
    receipt.receipt_hash = dm2_gdat_file_receipt_hash(
        raw_index, receipt.raw_length, receipt.pixel_payload_bytes,
        receipt.decoded_pixel_hash ^ receipt.underlay_raw_index);
    *out_receipt = receipt;
    return 1;
}

static int dm2_v1_gdat_gfx_material_from_entry(
    const DM2_V1_AssetLoader *loader,
    const DM2_V1_GdatEntry *entry,
    int gfxalloc_done,
    int used_gfx16_default,
    DM2_V1_GdatGfxMaterialReceipt *out_receipt)
{
    DM2_V1_GdatGfxMaterialReceipt receipt;
    size_t source_byte_count = 0u;
    const uint8_t *source_bytes;
    uint16_t raw_index;

    if (!entry) return 0;
    memset(&receipt, 0, sizeof(receipt));
    raw_index = (uint16_t)(entry->data_index & 0x7fffu);
    source_bytes = dm2_v1_load_gdat_raw_data(loader, raw_index,
                                              &source_byte_count);
    if (!source_bytes || source_byte_count == 0u ||
        !dm2_v1_extract_gdat_image_receipt(loader, raw_index, gfxalloc_done,
                                           0, NULL, 0u, &receipt.image) ||
        !receipt.image.valid) {
        return 0;
    }
    receipt.accepted = 1u;
    receipt.used_gfx16_default = used_gfx16_default ? 1u : 0u;
    receipt.gfxalloc_done = gfxalloc_done ? 1u : 0u;
    receipt.selected_category = entry->cls1;
    receipt.selected_index = entry->cls2;
    receipt.selected_field = entry->cls4;
    receipt.raw_index = raw_index;
    receipt.source_bytes = source_bytes;
    receipt.source_byte_count = source_byte_count;
    receipt.receipt_hash = dm2_gdat_file_receipt_hash(
        receipt.image.receipt_hash,
        ((uint32_t)receipt.selected_category << 16) |
            ((uint32_t)receipt.selected_index << 8) |
            receipt.selected_field,
        raw_index,
        receipt.used_gfx16_default);
    *out_receipt = receipt;
    return 1;
}

int dm2_v1_gdat_allocate_gfx256_material_receipt(
    const DM2_V1_AssetLoader *loader,
    uint16_t raw_index,
    int gfxalloc_done,
    DM2_V1_GdatGfxMaterialReceipt *out_receipt)
{
    DM2_V1_GdatGfxMaterialReceipt receipt;
    size_t source_byte_count = 0u;
    const uint8_t *source_bytes;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    memset(&receipt, 0, sizeof(receipt));
    source_bytes = dm2_v1_load_gdat_raw_data(loader, raw_index,
                                              &source_byte_count);
    if (!source_bytes || source_byte_count == 0u ||
        !dm2_v1_extract_gdat_image_receipt(loader, raw_index, gfxalloc_done,
                                           0, NULL, 0u, &receipt.image) ||
        !receipt.image.valid) {
        return 0;
    }
    receipt.accepted = 1u;
    receipt.gfxalloc_done = gfxalloc_done ? 1u : 0u;
    receipt.raw_index = raw_index;
    receipt.source_bytes = source_bytes;
    receipt.source_byte_count = source_byte_count;
    receipt.receipt_hash = dm2_gdat_file_receipt_hash(
        receipt.image.receipt_hash, raw_index, source_byte_count, 0u);
    *out_receipt = receipt;
    return 1;
}

int dm2_v1_gdat_allocate_gfx256_raw_material_receipt(
    const DM2_V1_AssetLoader *loader,
    uint16_t raw_index,
    DM2_V1_GdatGfxRawMaterialReceipt *out_receipt)
{
    DM2_V1_GdatGfxRawMaterialReceipt receipt;
    size_t source_byte_count = 0u;
    const uint8_t *source_bytes;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    source_bytes = dm2_v1_load_gdat_raw_data(loader, raw_index,
                                              &source_byte_count);
    if (!source_bytes || source_byte_count == 0u) return 0;
    memset(&receipt, 0, sizeof(receipt));
    receipt.accepted = 1u;
    receipt.raw_index = raw_index;
    receipt.source_bytes = source_bytes;
    receipt.source_byte_count = source_byte_count;
    receipt.source_hash = dm2_fnv1a_bytes(source_bytes, source_byte_count);
    receipt.receipt_hash = dm2_gdat_file_receipt_hash(
        raw_index, source_byte_count, receipt.source_hash, 0x47584632u);
    *out_receipt = receipt;
    return 1;
}

int dm2_v1_gdat_image_raw_material_receipt(
    const DM2_V1_AssetLoader *loader, int category, int index, int field,
    DM2_V1_GdatGfxRawMaterialReceipt *out_receipt)
{
    const DM2_V1_GdatEntry *entry;
    DM2_V1_GdatGfxRawMaterialReceipt receipt;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    entry = dm2_gdat_find_entry(loader, category, index,
                                DM2_GDAT_ENTRY_TYPE_IMAGE, field);
    if (!entry || !dm2_v1_gdat_allocate_gfx256_raw_material_receipt(
                      loader, (uint16_t)(entry->data_index & 0x7fffu),
                      &receipt)) {
        return 0;
    }
    receipt.receipt_hash = dm2_gdat_file_receipt_hash(
        receipt.receipt_hash,
        ((uint32_t)(uint8_t)category << 16) |
            ((uint32_t)(uint8_t)index << 8) | (uint8_t)field,
        receipt.raw_index, receipt.source_hash);
    if (!receipt.receipt_hash) return 0;
    *out_receipt = receipt;
    return 1;
}

int dm2_v1_gdat_allocate_gfx16_material_receipt(
    const DM2_V1_AssetLoader *loader,
    int category,
    int index,
    int field,
    int gfxalloc_done,
    DM2_V1_GdatGfxMaterialReceipt *out_receipt)
{
    const DM2_V1_GdatEntry *entry = NULL;
    int used_default = 0;
    uint16_t i;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (loader && loader->entries) {
        for (i = 0u; i < loader->entry_count; ++i) {
            const DM2_V1_GdatEntry *candidate = &loader->entries[i];
            if ((int)candidate->cls1 == category &&
                (int)candidate->cls2 == index &&
                candidate->cls3 == DM2_GDAT_ENTRY_TYPE_IMAGE &&
                (int)candidate->cls4 == field &&
                dm2_v1_gdat_gfx_material_from_entry(
                    loader, candidate, gfxalloc_done, 0, out_receipt)) {
                return 1;
            }
        }
    }
    if (!entry) {
        entry = dm2_gdat_find_entry(loader, DM2_GDAT_CATEGORY_MISCELLANEOUS,
                                    0xfe, DM2_GDAT_ENTRY_TYPE_IMAGE, 0xfe);
        used_default = 1;
    }
    return dm2_v1_gdat_gfx_material_from_entry(loader, entry, gfxalloc_done,
                                                used_default, out_receipt);
}

int dm2_v1_query_gdat_image_entry_buff_receipt(
    const DM2_V1_AssetLoader *loader,
    int category,
    int index,
    int field,
    DM2_V1_GdatImageEntryBuffReceipt *out_receipt)
{
    DM2_V1_GdatImageEntryBuffReceipt receipt;
    const DM2_V1_GdatEntry *entry;
    const DM2_V1_GdatEntry *selected;
    const uint8_t *raw;
    size_t raw_size = 0u;
    uint16_t bpp = 0u;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    memset(&receipt, 0, sizeof(receipt));

    entry = dm2_gdat_find_entry(loader, category, index,
                                DM2_GDAT_ENTRY_TYPE_IMAGE, field);
    selected = entry;
    if (!selected) {
        /* skproject QUERY_GDAT_IMAGE_ENTRY_BUFF falls back to the real
         * MISCELLANEOUS/FE/FE GDAT image. This is source-owned data, not a
         * Firestaff generated visual. Missing default data still fails. */
        selected = dm2_gdat_find_entry(loader, DM2_GDAT_CATEGORY_MISCELLANEOUS,
                                       0xfe, DM2_GDAT_ENTRY_TYPE_IMAGE, 0xfe);
        receipt.used_default_image = 1u;
    }
    raw = dm2_gdat_raw_from_entry(loader, selected, &raw_size);
    if (!selected || !raw || raw_size < DM2_IMG3_HEADER_SIZE ||
        !dm2_img3_raw_bits_per_pixel(raw, raw_size, &bpp, loader->big_endian)) {
        return 0;
    }

    receipt.width = (uint16_t)(img_rd16(raw, loader->big_endian) & 0x03ffu);
    receipt.height = (uint16_t)(img_rd16(raw + 2u, loader->big_endian) & 0x03ffu);
    if (receipt.width == 0u || receipt.height == 0u ||
        (bpp != 4u && bpp != 8u)) {
        return 0;
    }

    receipt.accepted = 1u;
    receipt.category = (uint8_t)category;
    receipt.index = (uint8_t)index;
    receipt.field = (uint8_t)field;
    receipt.bits_per_pixel = (uint8_t)bpp;
    receipt.requested_data_index = entry ? entry->data_index : 0xffffu;
    receipt.selected_data_index = selected->data_index;
    receipt.selected_raw_index = (uint16_t)(selected->data_index & 0x7fffu);
    receipt.raw_length = (uint32_t)raw_size;
    receipt.raw_hash = dm2_fnv1a_bytes(raw, raw_size);
    receipt.receipt_hash = dm2_gdat_file_receipt_hash(
        ((uint32_t)(uint8_t)category << 16) |
            ((uint32_t)(uint8_t)index << 8) | (uint32_t)(uint8_t)field,
        receipt.selected_data_index,
        ((uint32_t)receipt.width << 16) | receipt.height,
        receipt.raw_hash ^ receipt.used_default_image);
    *out_receipt = receipt;
    return 1;
}

int dm2_v1_query_gdat_image_metrics_receipt(
    const DM2_V1_AssetLoader *loader,
    int category,
    int index,
    int field,
    uint16_t *out_width,
    uint16_t *out_height,
    DM2_V1_GdatImageEntryBuffReceipt *out_receipt)
{
    DM2_V1_GdatImageEntryBuffReceipt receipt;

    if (out_width) *out_width = 0u;
    if (out_height) *out_height = 0u;
    if (!out_width || !out_height ||
        !dm2_v1_query_gdat_image_entry_buff_receipt(loader, category, index,
                                                    field, &receipt)) {
        if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
        return 0;
    }
    *out_width = receipt.width;
    *out_height = receipt.height;
    if (out_receipt) *out_receipt = receipt;
    return 1;
}

int dm2_v1_query_pict_bits_receipt(
    const DM2_V1_AssetLoader *loader,
    uint8_t mode,
    int existing_bitmap_present,
    int cached_bitmap_present,
    int category,
    int index,
    int field,
    DM2_V1_QueryPictBitsReceipt *out_receipt)
{
    DM2_V1_GdatImageEntryBuffReceipt image;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    out_receipt->mode = mode;
    out_receipt->category = (uint8_t)category;
    out_receipt->index = (uint8_t)index;
    out_receipt->field = (uint8_t)field;

    /* skproject QUERY_PICT_BITS: mode bit 2 forces a GDAT image lookup,
     * otherwise bit 3 asks the existing bitmap cache, otherwise the current
     * bitmap pointer is reused. This receipt records only that ownership
     * route and never creates substitute pixels. */
    if ((mode & 0x04u) != 0u) {
        if (!dm2_v1_query_gdat_image_entry_buff_receipt(
                loader, category, index, field, &image)) {
            memset(out_receipt, 0, sizeof(*out_receipt));
            return 0;
        }
        out_receipt->accepted = 1u;
        out_receipt->queried_gdat_image = 1u;
        out_receipt->selected_raw_index = image.selected_raw_index;
        out_receipt->width = image.width;
        out_receipt->height = image.height;
        out_receipt->receipt_hash = dm2_gdat_file_receipt_hash(
            mode, image.selected_data_index,
            ((uint32_t)image.width << 16) | image.height,
            image.raw_hash);
        return 1;
    }

    if ((mode & 0x08u) != 0u) {
        if (!cached_bitmap_present) return 0;
        out_receipt->accepted = 1u;
        out_receipt->used_cached_bitmap = 1u;
        out_receipt->receipt_hash = dm2_gdat_file_receipt_hash(
            mode, 0x43414348u, (uint32_t)category, (uint32_t)field);
        return 1;
    }

    if (!existing_bitmap_present) return 0;
    out_receipt->accepted = 1u;
    out_receipt->used_existing_bitmap = 1u;
    out_receipt->receipt_hash = dm2_gdat_file_receipt_hash(
        mode, 0x45584953u, (uint32_t)category, (uint32_t)field);
    return 1;
}

int dm2_v1_query_4bpp_pict_buff_and_pal_receipt(
    const DM2_V1_AssetLoader *loader,
    int category,
    int index,
    uint16_t width_divisor,
    DM2_V1_Query4BppPictBuffAndPalReceipt *out_receipt)
{
    enum { MAP_CHIP_FIELD = DM2_GDAT_IMG_MAP_CHIP };
    DM2_V1_GdatEntryQueryReceipt loadable;
    DM2_V1_GdatImageEntryBuffReceipt image;
    uint8_t palette16[16];
    uint32_t palette_hash = 0u;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (width_divisor == 0u ||
        !dm2_v1_query_gdat_entry_if_loadable(
            loader, category, index, DM2_GDAT_ENTRY_TYPE_IMAGE,
            MAP_CHIP_FIELD, &loadable) ||
        !dm2_v1_query_gdat_image_entry_buff_receipt(
            loader, category, index, MAP_CHIP_FIELD, &image) ||
        image.used_default_image ||
        image.bits_per_pixel != 4u ||
        !dm2_v1_asset_load_image_local_palette(
            loader, category, index, MAP_CHIP_FIELD, palette16,
            &palette_hash)) {
        return 0;
    }

    out_receipt->accepted = 1u;
    out_receipt->category = (uint8_t)category;
    out_receipt->index = (uint8_t)index;
    out_receipt->field = MAP_CHIP_FIELD;
    memcpy(out_receipt->palette16, palette16, sizeof(out_receipt->palette16));
    out_receipt->selected_raw_index = image.selected_raw_index;
    out_receipt->width = image.width;
    out_receipt->height = image.height;
    out_receipt->width_units = (uint16_t)(image.width / width_divisor);
    out_receipt->image_hash = image.raw_hash;
    out_receipt->palette_hash = palette_hash;
    out_receipt->receipt_hash = dm2_gdat_file_receipt_hash(
        loadable.receipt_hash, image.raw_hash, palette_hash,
        ((uint32_t)out_receipt->width_units << 16) | image.height);
    return 1;
}

int dm2_v1_query_picst_image_receipt(
    const DM2_V1_AssetLoader *loader,
    int category,
    int index,
    int field,
    DM2_V1_QueryPicstImageReceipt *out_receipt)
{
    DM2_V1_GdatImageEntryBuffReceipt image;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (!dm2_v1_query_gdat_image_entry_buff_receipt(
            loader, category, index, field, &image)) {
        return 0;
    }

    /* skproject QUERY_PICST_IMAGE resolves the source GDAT image, resets
     * picture x/y to zero, stores mode 4, and copies the bitmap dimensions.
     * This receipt records those source fields without drawing or allocating
     * a replacement picture. */
    out_receipt->accepted = 1u;
    out_receipt->category = (uint8_t)category;
    out_receipt->index = (uint8_t)index;
    out_receipt->field = (uint8_t)field;
    out_receipt->mode = 4u;
    out_receipt->selected_raw_index = image.selected_raw_index;
    out_receipt->width = image.width;
    out_receipt->height = image.height;
    out_receipt->data_index = image.selected_data_index;
    out_receipt->image_hash = image.raw_hash;
    out_receipt->receipt_hash = dm2_gdat_file_receipt_hash(
        image.receipt_hash, 4u,
        ((uint32_t)image.width << 16) | image.height,
        image.raw_hash);
    return 1;
}

int dm2_v1_query_gdat_summary_image_receipt(
    const DM2_V1_AssetLoader *loader,
    int category,
    int index,
    int field,
    DM2_V1_QueryGdatSummaryImageReceipt *out_receipt)
{
    uint16_t data_index = 0u;
    uint16_t graphicsset_offset = 0u;
    uint16_t image_offset = 0u;
    uint32_t palette_hash = 0u;
    uint8_t palette16[16];

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    out_receipt->category = (uint8_t)category;
    out_receipt->index = (uint8_t)index;
    out_receipt->field = (uint8_t)field;

    /* QUERY_GDAT_SUMMARY_IMAGE initializes a blank picture when cls1 == FF.
     * It intentionally avoids GDAT lookup in that case. */
    if ((uint8_t)category == 0xffu) {
        out_receipt->accepted = 1u;
        out_receipt->gdat_bypassed_for_ff = 1u;
        out_receipt->colors = 0xffu;
        out_receipt->receipt_hash = dm2_gdat_file_receipt_hash(
            0xffu, (uint32_t)(uint8_t)index, (uint32_t)(uint8_t)field, 0u);
        return 1;
    }

    if (!dm2_v1_query_gdat_entry_data_index(
            loader, category, index, DM2_GDAT_ENTRY_TYPE_IMAGE, field,
            &data_index) ||
        !dm2_v1_asset_load_image_metadata(
            loader, category, index, field, &out_receipt->metadata)) {
        memset(out_receipt, 0, sizeof(*out_receipt));
        return 0;
    }
    /* SKWIN c_querydb.cpp:1781-1817: SUMMARY_IMAGE applies the category/
     * index dtImageOffset(0xfe) first, then the selected image-field offset.
     * Each nonzero word is an exact signed-byte x/y pair; absence remains the
     * source zero offset rather than a replacement placement. */
    if (dm2_v1_asset_load_image_offset(loader, category, index, 0xfe,
                                       &graphicsset_offset) &&
        graphicsset_offset != 0u) {
        out_receipt->metadata.query_offset_x =
            (int8_t)(graphicsset_offset >> 8);
        out_receipt->metadata.query_offset_y = (int8_t)graphicsset_offset;
        out_receipt->metadata.graphicsset_offset_present = 1;
    }
    if (dm2_v1_asset_load_image_offset(loader, category, index, field,
                                       &image_offset) && image_offset != 0u) {
        out_receipt->metadata.query_offset_x += (int8_t)(image_offset >> 8);
        out_receipt->metadata.query_offset_y += (int8_t)image_offset;
        out_receipt->metadata.image_offset_present = 1;
    }
    if (dm2_v1_asset_load_image_local_palette(
            loader, category, index, field, palette16, &palette_hash)) {
        out_receipt->colors = 16u;
        memcpy(out_receipt->palette16, palette16,
               sizeof(out_receipt->palette16));
        out_receipt->palette_hash = palette_hash;
    } else {
        /* skproject records colors=-1 when local palette lookup fails; it
         * still returns the source image descriptor, without substituting
         * another palette. */
        out_receipt->colors = 0xffu;
    }

    out_receipt->accepted = 1u;
    out_receipt->data_index = data_index;
    out_receipt->graphicsset_offset_word = graphicsset_offset;
    out_receipt->image_offset_word = image_offset;
    out_receipt->offset_receipt_hash = dm2_gdat_file_receipt_hash(
        graphicsset_offset, image_offset,
        ((uint32_t)(uint16_t)out_receipt->metadata.query_offset_x << 16) |
            (uint16_t)out_receipt->metadata.query_offset_y,
        ((uint32_t)out_receipt->metadata.graphicsset_offset_present << 1) |
            (uint32_t)out_receipt->metadata.image_offset_present);
    if (!out_receipt->offset_receipt_hash) {
        memset(out_receipt, 0, sizeof(*out_receipt));
        return 0;
    }
    out_receipt->receipt_hash = dm2_gdat_file_receipt_hash(
        data_index, out_receipt->metadata.metadata_hash,
        palette_hash ^ out_receipt->offset_receipt_hash,
        ((uint32_t)out_receipt->metadata.width << 16) |
            out_receipt->metadata.height);
    return 1;
}

static int dm2_base36_frame(uint8_t byte, uint16_t *out_frame)
{
    if (!out_frame) return 0;
    if (byte >= '0' && byte <= '9') {
        *out_frame = (uint16_t)(byte - '0');
        return 1;
    }
    if (byte >= 'A' && byte <= 'Z') {
        *out_frame = (uint16_t)(byte - 'A' + 10u);
        return 1;
    }
    return 0;
}

int dm2_v1_query_ornate_anim_frame_receipt(
    const DM2_V1_AssetLoader *loader,
    int category,
    int index,
    uint32_t tick,
    uint32_t delta,
    DM2_V1_QueryOrnateAnimFrameReceipt *out_receipt)
{
    enum { ANIMATION_FIELD = 0x0d };
    uint16_t word = 0u;
    uint16_t length = 0u;
    uint16_t frame_base = 0u;
    const uint8_t *sequence;
    size_t sequence_size = 0u;
    size_t sequence_length = 0u;
    uint16_t frame = 0u;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (!loader || category < 0 || index < 0 || index > 0xff) return 0;
    out_receipt->category = (uint8_t)category;
    out_receipt->index = (uint8_t)index;

    if (dm2_v1_asset_load_word_value(loader, category, index,
                                     ANIMATION_FIELD, &word)) {
        length = (uint16_t)(word & 0x7fffu);
        if (length == 0u) return 0;
        frame_base = (word & 0x8000u) ? 1u : 0u;
        frame = (uint16_t)(((tick + delta) % length) + frame_base);
        out_receipt->used_word_value = 1u;
    } else {
        sequence = dm2_v1_asset_load_text_sized(loader, category, index,
                                                ANIMATION_FIELD,
                                                &sequence_size);
        if (!sequence || sequence_size == 0u) return 0;
        while (sequence_length < sequence_size && sequence[sequence_length]) {
            if (!dm2_base36_frame(sequence[sequence_length], &frame)) return 0;
            ++sequence_length;
        }
        if (sequence_length == 0u || sequence_length == sequence_size)
            return 0;
        if (!dm2_base36_frame(
                sequence[(tick + delta) % sequence_length], &frame)) {
            return 0;
        }
        length = (uint16_t)sequence_length;
        out_receipt->used_text_sequence = 1u;
    }

    out_receipt->accepted = 1u;
    out_receipt->length = length;
    out_receipt->frame_base = frame_base;
    out_receipt->frame = frame;
    out_receipt->receipt_hash = dm2_gdat_file_receipt_hash(
        (uint32_t)category, (uint32_t)index,
        ((uint32_t)length << 16) | frame, tick + delta);
    return 1;
}

int dm2_v1_get_ornate_anim_len_receipt(
    const DM2_V1_AssetLoader *loader,
    int category,
    int index,
    int decoration_absent,
    DM2_V1_GetOrnateAnimLenReceipt *out_receipt)
{
    enum { ANIMATION_FIELD = 0x0d };
    uint16_t word = 0u;
    const uint8_t *sequence;
    size_t sequence_size = 0u;
    size_t sequence_length = 0u;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (category < 0 || index < 0 || index > 0xff) return 0;
    out_receipt->category = (uint8_t)category;
    out_receipt->index = (uint8_t)index;

    if (decoration_absent) {
        out_receipt->accepted = 1u;
        out_receipt->decoration_absent = 1u;
        out_receipt->length = 1u;
        out_receipt->receipt_hash = dm2_gdat_file_receipt_hash(
            0x4f414c45u, (uint32_t)category, (uint32_t)index, 1u);
        return 1;
    }

    if (dm2_v1_asset_load_word_value(loader, category, index,
                                     ANIMATION_FIELD, &word)) {
        out_receipt->length = (uint16_t)(word & 0x7fffu);
        if (out_receipt->length == 0u) return 0;
        out_receipt->used_word_value = 1u;
    } else {
        sequence = dm2_v1_asset_load_text_sized(loader, category, index,
                                                ANIMATION_FIELD,
                                                &sequence_size);
        if (!sequence || sequence_size == 0u) return 0;
        while (sequence_length < sequence_size && sequence[sequence_length]) {
            uint16_t ignored = 0u;
            if (!dm2_base36_frame(sequence[sequence_length], &ignored))
                return 0;
            ++sequence_length;
        }
        if (sequence_length == 0u || sequence_length == sequence_size)
            return 0;
        out_receipt->length = (uint16_t)sequence_length;
        out_receipt->used_text_sequence = 1u;
    }

    out_receipt->accepted = 1u;
    out_receipt->receipt_hash = dm2_gdat_file_receipt_hash(
        0x4f414c45u, (uint32_t)category, (uint32_t)index,
        out_receipt->length);
    return 1;
}

static int dm2_gdat_word_receipt(
    int category,
    int index,
    int field,
    uint16_t value,
    uint8_t used_cache_byte,
    DM2_V1_GdatWordQueryReceipt *out_receipt)
{
    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (category < 0 || index < 0 || field < 0 ||
        category > 0xff || index > 0xff || field > 0xff) {
        return 0;
    }
    out_receipt->accepted = 1u;
    out_receipt->used_cache_byte = used_cache_byte;
    out_receipt->category = (uint8_t)category;
    out_receipt->index = (uint8_t)index;
    out_receipt->field = (uint8_t)field;
    out_receipt->value = value;
    out_receipt->receipt_hash = dm2_gdat_file_receipt_hash(
        (uint32_t)category, (uint32_t)index,
        ((uint32_t)(uint8_t)field << 16) | value, used_cache_byte);
    return 1;
}

static const char *dm2_v1_cmdstr_key(uint8_t key_index)
{
    static const char *keys[DM2_V1_CMDSTR_KEY_COUNT] = {
        "SK", "LV", "CM", "BZ", "TR", "ST",
        "PA", "TA", "NC", "EX", "PB", "DM",
        "MS", "SD", "RP", "HN", "AT", "WH"
    };
    return key_index < DM2_V1_CMDSTR_KEY_COUNT ? keys[key_index] : NULL;
}

static int dm2_v1_text_has_nul(const uint8_t *text, size_t text_size,
                               size_t *out_len)
{
    size_t i;

    if (out_len) *out_len = 0u;
    if (!text) return 0;
    for (i = 0; i < text_size; ++i) {
        if (text[i] == 0u) {
            if (out_len) *out_len = i;
            return 1;
        }
    }
    return 0;
}

static int dm2_v1_query_cmdstr_text_value(
    const uint8_t *text,
    size_t text_size,
    const char *key,
    int *out_found,
    int32_t *out_value)
{
    size_t key_size;
    size_t cursor = 0u;
    int found = 0;
    int32_t value = 0;

    if (out_found) *out_found = 0;
    if (out_value) *out_value = 0;
    if (!text || !key || key[0] == '\0' ||
        !dm2_v1_text_has_nul(text, text_size, NULL)) {
        return 0;
    }
    key_size = strlen(key);
    while (cursor + key_size <= text_size) {
        size_t at = text_size;
        int negative = 0;

        for (size_t i = cursor; i + key_size <= text_size; ++i) {
            if (text[i] == '\0') break;
            if (memcmp(text + i, key, key_size) == 0) {
                at = i;
                break;
            }
        }
        if (at == text_size) break;
        found = 1;
        at += key_size;
        if (at < text_size && text[at] == '=') ++at;
        if (at < text_size && text[at] == '-') {
            negative = 1;
            ++at;
        }
        while (at < text_size && text[at] >= '0' && text[at] <= '9') {
            if (value > (INT32_MAX - (int32_t)(text[at] - '0')) / 10)
                return 0;
            value = value * 10 + (int32_t)(text[at] - '0');
            ++at;
        }
        if (negative) value = -value;
        cursor = at > cursor ? at : cursor + 1u;
    }
    if (out_found) *out_found = found;
    if (out_value) *out_value = value;
    return 1;
}

int dm2_v1_query_gdat_item_name_receipt(
    const DM2_V1_AssetLoader *loader,
    int category,
    int index,
    DM2_V1_GdatNameReceipt *out_receipt)
{
    enum { ITEM_NAME_FIELD = 0x18 };
    return dm2_v1_query_cmdstr_name_receipt(
        loader, category, index, ITEM_NAME_FIELD, out_receipt);
}

int dm2_v1_query_cmdstr_name_receipt(
    const DM2_V1_AssetLoader *loader,
    int category,
    int index,
    int field,
    DM2_V1_GdatNameReceipt *out_receipt)
{
    const uint8_t *text;
    size_t text_size = 0u;
    size_t text_len = 0u;
    size_t copy_len = 0u;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (!loader || category < 0 || category > 0xff ||
        index < 0 || index > 0xff || field < 0 || field > 0xff) {
        return 0;
    }
    out_receipt->category = (uint8_t)category;
    out_receipt->index = (uint8_t)index;
    out_receipt->field = (uint8_t)field;
    text = dm2_v1_asset_load_text_sized(loader, category, index, field,
                                        &text_size);
    if (!text || !dm2_v1_text_has_nul(text, text_size, &text_len) ||
        text_len == 0u) {
        return 0;
    }

    while (copy_len < text_len && text[copy_len] != ':') ++copy_len;
    if (copy_len >= sizeof(out_receipt->text)) {
        copy_len = sizeof(out_receipt->text) - 1u;
        out_receipt->truncated = 1u;
    }
    memcpy(out_receipt->text, text, copy_len);
    out_receipt->text[copy_len] = '\0';
    out_receipt->byte_count = (uint16_t)copy_len;
    out_receipt->text_hash = dm2_fnv1a_bytes(text, text_len);
    out_receipt->receipt_hash = dm2_gdat_file_receipt_hash(
        (uint32_t)category, (uint32_t)index,
        ((uint32_t)(uint8_t)field << 16) | (uint32_t)copy_len,
        out_receipt->text_hash);
    out_receipt->accepted = 1u;
    return 1;
}

int dm2_v1_query_cmdstr_entry_receipt(
    const DM2_V1_AssetLoader *loader,
    int category,
    int index,
    int field,
    int key_index,
    DM2_V1_CmdstrEntryReceipt *out_receipt)
{
    const char *key;
    const uint8_t *text;
    size_t text_size = 0u;
    size_t text_len = 0u;
    int found = 0;
    int32_t value = 0;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (!loader || category < 0 || category > 0xff ||
        index < 0 || index > 0xff || field < 0 || field > 0xff ||
        key_index < 0 || key_index >= (int)DM2_V1_CMDSTR_KEY_COUNT) {
        return 0;
    }
    key = dm2_v1_cmdstr_key((uint8_t)key_index);
    if (!key) return 0;
    out_receipt->key_index = (uint8_t)key_index;
    out_receipt->key[0] = key[0];
    out_receipt->key[1] = key[1];
    out_receipt->key[2] = '\0';

    text = dm2_v1_asset_load_text_sized(loader, category, index, field,
                                        &text_size);
    if (!text || !dm2_v1_text_has_nul(text, text_size, &text_len) ||
        text_len == 0u) {
        return 0;
    }
    if (!dm2_v1_query_cmdstr_text_value(text, text_size, key, &found, &value))
        return 0;

    out_receipt->accepted = 1u;
    out_receipt->found = (uint8_t)(found ? 1u : 0u);
    out_receipt->value = value;
    out_receipt->text_hash = dm2_fnv1a_bytes(text, text_len);
    out_receipt->receipt_hash = dm2_gdat_file_receipt_hash(
        (uint32_t)category, (uint32_t)index,
        ((uint32_t)(uint8_t)field << 16) | (uint32_t)(uint8_t)key_index,
        (uint32_t)value ^ out_receipt->text_hash);
    return 1;
}

int dm2_v1_query_cur_cmdstr_entry_receipt(
    const DM2_V1_AssetLoader *loader,
    const DM2_V1_CurCmdstrContext *context,
    int key_index,
    DM2_V1_CmdstrEntryReceipt *out_receipt)
{
    if (!context) {
        if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
        return 0;
    }
    return dm2_v1_query_cmdstr_entry_receipt(
        loader, context->category, context->index, context->field, key_index,
        out_receipt);
}

int dm2_v1_query_door_damage_resist_receipt(
    const DM2_V1_AssetLoader *loader,
    int door_index,
    DM2_V1_GdatWordQueryReceipt *out_receipt)
{
    uint16_t value = 0u;
    /* SKProject SkWinCore.cpp::QUERY_DOOR_DAMAGE_RESIST reads
     * GDAT_DOOR_DEFENSE (0x0e), not the neighbouring strength field. */
    if (!dm2_v1_asset_load_word_value(loader, DM2_GDAT_CATEGORY_DOORS,
                                      door_index, 0x0e, &value)) {
        if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
        return 0;
    }
    return dm2_gdat_word_receipt(DM2_GDAT_CATEGORY_DOORS, door_index, 0x0e,
                                 value, 0u, out_receipt);
}

static int dm2_door_word_field_receipt(
    const DM2_V1_AssetLoader *loader,
    int door_index,
    int field,
    DM2_V1_GdatWordQueryReceipt *out_receipt)
{
    uint16_t value = 0u;
    if (!dm2_v1_asset_load_word_value(loader, DM2_GDAT_CATEGORY_DOORS,
                                      door_index, field, &value)) {
        if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
        return 0;
    }
    return dm2_gdat_word_receipt(DM2_GDAT_CATEGORY_DOORS, door_index, field,
                                 value, 0u, out_receipt);
}

int dm2_v1_get_door_stat_0x10_receipt(
    const DM2_V1_AssetLoader *loader,
    int door_index,
    DM2_V1_GdatWordQueryReceipt *out_receipt)
{
    return dm2_door_word_field_receipt(loader, door_index, 0x10, out_receipt);
}

int dm2_v1_get_graphics_for_door_receipt(
    const DM2_V1_AssetLoader *loader,
    int door_index,
    DM2_V1_GdatWordQueryReceipt *out_receipt)
{
    return dm2_door_word_field_receipt(loader, door_index, 0x0d, out_receipt);
}

int dm2_v1_query_0cee_3275_receipt(
    const DM2_V1_AssetLoader *loader,
    int door_index,
    DM2_V1_GdatWordQueryReceipt *out_receipt)
{
    return dm2_door_word_field_receipt(loader, door_index, 0x0d, out_receipt);
}

int dm2_v1_query_gdat_creature_word_value_receipt(
    const DM2_V1_AssetLoader *loader,
    int creature_index,
    int field,
    const uint8_t *cache3,
    size_t cache_count,
    DM2_V1_GdatWordQueryReceipt *out_receipt)
{
    int cache_slot = -1;
    uint16_t value = 0u;

    if (field == 0) cache_slot = 1;
    else if (field == 1) cache_slot = 0;
    else if (field == 5) cache_slot = 2;
    if (cache3 && cache_slot >= 0 && cache_count > (size_t)cache_slot &&
        cache3[cache_slot] != 0xffu) {
        return dm2_gdat_word_receipt(DM2_GDAT_CATEGORY_CREATURES,
                                     creature_index, field, cache3[cache_slot],
                                     1u, out_receipt);
    }
    if (!dm2_v1_asset_load_word_value(loader, DM2_GDAT_CATEGORY_CREATURES,
                                      creature_index, field, &value)) {
        if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
        return 0;
    }
    return dm2_gdat_word_receipt(DM2_GDAT_CATEGORY_CREATURES,
                                 creature_index, field, value, 0u,
                                 out_receipt);
}

int dm2_v1_query_gdat_food_value_from_record_receipt(
    const DM2_V1_AssetLoader *loader,
    int category,
    int index,
    DM2_V1_GdatWordQueryReceipt *out_receipt)
{
    uint16_t value = 0u;
    if (!dm2_v1_asset_load_word_value(loader, category, index, 0x03,
                                      &value)) {
        if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
        return 0;
    }
    return dm2_gdat_word_receipt(category, index, 0x03, value, 0u,
                                 out_receipt);
}

static int dm2_item_dbspec_word_from_record_receipt(
    const DM2_V1_AssetLoader *loader,
    int category,
    int index,
    int field,
    DM2_V1_GdatWordQueryReceipt *out_receipt)
{
    uint16_t value = 0u;
    if (!dm2_v1_asset_load_word_value(loader, category, index, field,
                                      &value)) {
        if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
        return 0;
    }
    return dm2_gdat_word_receipt(category, index, field, value, 0u,
                                 out_receipt);
}

int dm2_v1_query_gdat_potion_spell_type_from_record_receipt(
    const DM2_V1_AssetLoader *loader,
    int category,
    int index,
    DM2_V1_GdatWordQueryReceipt *out_receipt)
{
    return dm2_item_dbspec_word_from_record_receipt(
        loader, category, index, 0x4d, out_receipt);
}

int dm2_v1_query_gdat_potion_behaviour_from_record_receipt(
    const DM2_V1_AssetLoader *loader,
    int category,
    int index,
    DM2_V1_GdatWordQueryReceipt *out_receipt)
{
    return dm2_item_dbspec_word_from_record_receipt(
        loader, category, index, 0x05, out_receipt);
}

int dm2_v1_query_gdat_water_value_from_record_receipt(
    const DM2_V1_AssetLoader *loader,
    int category,
    int index,
    DM2_V1_GdatWordQueryReceipt *out_receipt)
{
    return dm2_item_dbspec_word_from_record_receipt(
        loader, category, index, 0x43, out_receipt);
}

int dm2_v1_query_gdat_door_is_mirrored_receipt(
    const DM2_V1_AssetLoader *loader,
    int door_index,
    DM2_V1_GdatWordQueryReceipt *out_receipt)
{
    return dm2_door_word_field_receipt(loader, door_index, 0x20,
                                       out_receipt);
}

int dm2_v1_query_door_strength_receipt(
    const DM2_V1_AssetLoader *loader,
    int door_index,
    DM2_V1_DoorStrengthReceipt *out_receipt)
{
    uint16_t strength = 0u;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (door_index < 0 || door_index > 0xff) return 0;
    out_receipt->door_index = (uint8_t)door_index;

    /* SKProject: SkWinCore.cpp::QUERY_DOOR_STRENGTH is a direct GDAT
     * lookup: DOORS / dtWordValue / GDAT_DOOR_STRENGTH (0x0f).  Earlier
     * Firestaff code looked at unrelated 0x10/0x11 fields and manufactured
     * a 6-or-1 strength fallback.  Never infer gameplay values where the
     * original record has an exact field. */
    if (!dm2_v1_asset_load_word_value(loader, DM2_GDAT_CATEGORY_DOORS,
                                      door_index, 0x0f, &strength)) {
        return 0;
    }
    out_receipt->accepted = 1u;
    out_receipt->used_explicit_strength = 1u;
    out_receipt->strength = strength;

    out_receipt->receipt_hash = dm2_gdat_file_receipt_hash(
        0x44535452u, (uint32_t)door_index,
        ((uint32_t)out_receipt->strength << 16) | 0x000fu,
        out_receipt->used_explicit_strength);
    return 1;
}

static int dm2_creatures_item_mask_base(uint8_t token, int is_creature,
                                        int32_t *out_base)
{
    if (!out_base) return 0;
    switch (token) {
    case 'A': *out_base = 0x080; return 1;
    case 'J': *out_base = 0x100; return 1;
    case 'P': *out_base = 0x180; return 1;
    case 'S': *out_base = 0x1fc; return 1;
    case 'C': *out_base = is_creature ? 0 : 0x1e0; return 1;
    case 'W': *out_base = 0; return 1;
    default: return 0;
    }
}

int dm2_v1_query_creatures_item_mask_receipt(
    const DM2_V1_AssetLoader *loader,
    int creature_index,
    int text_field_base,
    int is_creature,
    DM2_V1_CreaturesItemMaskReceipt *out_receipt)
{
    const uint8_t *text;
    size_t text_size = 0u;
    size_t text_length = 0u;
    uint16_t number = 0u;
    int have_number = 0;
    int32_t range_start = -1;
    int32_t item_base = -1;
    uint16_t set_bits = 0u;
    uint32_t text_hash = 0u;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (!loader || creature_index < 0 || creature_index > 0xff ||
        text_field_base < 0 || text_field_base > 0xef) {
        return 0;
    }
    text = dm2_v1_asset_load_text_sized(loader, DM2_GDAT_CATEGORY_CREATURES,
                                        creature_index, text_field_base + 0x10,
                                        &text_size);
    if (!text || text_size == 0u || text[0] == 0u) return 0;
    while (text_length < text_size && text[text_length]) ++text_length;
    if (text_length == text_size) return 0;
    text_hash = dm2_fnv1a_bytes(text, text_length + 1u);

    for (size_t i = 0u; i <= text_length; ++i) {
        uint8_t ch = text[i];
        if (ch >= '0' && ch <= '9') {
            if (number > 6552u) return 0;
            number = (uint16_t)(number * 10u + (uint16_t)(ch - '0'));
            have_number = 1;
            continue;
        }
        if (ch == '-') {
            if (!have_number) return 0;
            range_start = number;
            number = 0u;
            have_number = 0;
            continue;
        }
        if (have_number) {
            int32_t start = range_start < 0 ? number : range_start;
            int32_t end = number;
            if (item_base < 0 || start > end) return 0;
            for (int32_t bit = start; bit <= end; ++bit) {
                int32_t absolute = bit + item_base;
                if (absolute < 0 || absolute >= 512) return 0;
                if ((out_receipt->mask[absolute / 8] &
                     (uint8_t)(1u << (absolute & 7))) == 0u) {
                    ++set_bits;
                }
                out_receipt->mask[absolute / 8] |=
                    (uint8_t)(1u << (absolute & 7));
            }
            number = 0u;
            have_number = 0;
            range_start = -1;
            item_base = -1;
        }
        if (ch == 0u) break;
        if (!dm2_creatures_item_mask_base(ch, is_creature, &item_base))
            return 0;
    }

    out_receipt->accepted = 1u;
    out_receipt->category = DM2_GDAT_CATEGORY_CREATURES;
    out_receipt->index = (uint8_t)creature_index;
    out_receipt->field = (uint8_t)(text_field_base + 0x10);
    out_receipt->creature_route = is_creature ? 1u : 0u;
    out_receipt->set_bits = set_bits;
    out_receipt->text_hash = text_hash;
    out_receipt->receipt_hash = dm2_gdat_file_receipt_hash(
        ((uint32_t)out_receipt->field << 16) | (uint32_t)creature_index,
        text_hash, set_bits, out_receipt->mask[0] ^ out_receipt->mask[63]);
    return 1;
}

static uint16_t dm2_equip_slot_mask(int slot)
{
    static const uint16_t table1d2670[13] = {
        0x0200u, 0x0100u, 0x0001u, 0x0004u, 0x0008u, 0x0010u, 0x0020u,
        0x0040u, 0x0040u, 0x0040u, 0x0002u, 0x0020u, 0x0080u
    };
    if (slot < 0) return 0x0400u;
    if (slot >= 13) return 0u;
    return table1d2670[slot];
}

int dm2_v1_is_item_fit_for_equip_receipt(
    const DM2_V1_AssetLoader *loader,
    int category,
    int index,
    int inventory_slot,
    int only_body_part,
    int active_hand_fit_result,
    DM2_V1_ItemFitForEquipReceipt *out_receipt)
{
    uint16_t flags = 0u;
    uint16_t mask = 0u;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (category < 0 || category > 0xff || index < 0 || index > 0xff ||
        inventory_slot < -32768 || inventory_slot > 32767) {
        return 0;
    }
    if (!dm2_v1_asset_load_word_value(loader, category, index, 0x04,
                                      &flags)) {
        return 0;
    }

    out_receipt->category = (uint8_t)category;
    out_receipt->index = (uint8_t)index;
    out_receipt->inventory_slot = (int16_t)inventory_slot;
    out_receipt->only_body_part = only_body_part ? 1u : 0u;
    out_receipt->equip_flags = flags;

    if (only_body_part) {
        if (inventory_slot >= 13) {
            out_receipt->result = 0u;
        } else {
            mask = dm2_equip_slot_mask(inventory_slot);
            out_receipt->result = (uint16_t)(flags & mask);
        }
    } else if (inventory_slot < 13 && inventory_slot > 1) {
        mask = dm2_equip_slot_mask(inventory_slot);
        out_receipt->result = (uint16_t)(flags & mask);
    } else if (inventory_slot < 30 || inventory_slot >= 38) {
        out_receipt->result = 1u;
    } else {
        if ((flags & 0x8000u) != 0u) {
            out_receipt->result = 0u;
        } else {
            if (active_hand_fit_result < 0) return 0;
            out_receipt->used_active_hand_result = 1u;
            out_receipt->result = active_hand_fit_result == 0
                                      ? 1u
                                      : (uint16_t)(flags & 0x0040u);
        }
    }

    out_receipt->accepted = 1u;
    out_receipt->tested_mask = mask;
    out_receipt->receipt_hash = dm2_gdat_file_receipt_hash(
        ((uint32_t)(uint8_t)category << 16) | (uint32_t)(uint8_t)index,
        flags, ((uint32_t)(uint16_t)inventory_slot << 16) |
                   (uint32_t)out_receipt->result,
        out_receipt->used_active_hand_result);
    return 1;
}

int dm2_v1_get_item_order_in_container_receipt(
    const DM2_V1_AssetLoader *loader,
    int container_index,
    int requested_order,
    const uint16_t *money_item_ids,
    size_t money_item_count,
    DM2_V1_ItemOrderInContainerReceipt *out_receipt)
{
    const uint8_t *text;
    size_t text_size = 0u;
    size_t text_len = 0u;
    size_t pos = 0u;
    int done = 0;
    int16_t range_start = -1;
    int16_t money_base = -1;
    uint16_t value = 0u;
    uint16_t enumerated = 0u;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (container_index < 0 || container_index > 0xff ||
        requested_order < 0 || requested_order > 0xffff ||
        !money_item_ids || money_item_count == 0u) {
        return 0;
    }

    text = dm2_v1_asset_load_text_sized(loader, DM2_GDAT_CATEGORY_CONTAINERS,
                                        container_index, 0x40, &text_size);
    if (!text || text_size == 0u || text[0] == 0u) return 0;
    while (text_len < text_size && text[text_len] != 0u) ++text_len;
    if (text_len == text_size) return 0;

    while (!done && pos <= text_len) {
        uint8_t ch = text[pos++];
        if (ch >= '0' && ch <= '9') {
            value = (uint16_t)(value * 10u + (uint16_t)(ch - '0'));
            continue;
        }
        switch (ch) {
        case 'J':
            if (value == 0u) {
                money_base = 0x100;
                continue;
            }
            --pos;
            break;
        case '-':
            range_start = (int16_t)value;
            value = 0u;
            continue;
        case 0:
            done = 1;
            break;
        default:
            break;
        }

        if (range_start < 0) range_start = (int16_t)value;
        while (range_start <= (int16_t)value) {
            if (enumerated == (uint16_t)requested_order) {
                uint16_t item_type =
                    (uint16_t)((int32_t)range_start + money_base);
                size_t i;
                for (i = 0u; i < money_item_count; ++i) {
                    if (money_item_ids[i] == item_type) {
                        out_receipt->accepted = 1u;
                        out_receipt->category = DM2_GDAT_CATEGORY_CONTAINERS;
                        out_receipt->index = (uint8_t)container_index;
                        out_receipt->field = 0x40u;
                        out_receipt->requested_order =
                            (uint16_t)requested_order;
                        out_receipt->enumerated_order = enumerated;
                        out_receipt->resolved_item_type = item_type;
                        out_receipt->money_item_index = (int16_t)i;
                        out_receipt->text_length = (uint16_t)text_len;
                        out_receipt->text_hash =
                            dm2_gdat_file_receipt_hash(
                                (uint32_t)container_index, 0x40u,
                                dm2_fnv1a_bytes(text, text_len),
                                (uint32_t)text_len);
                        out_receipt->receipt_hash =
                            dm2_gdat_file_receipt_hash(
                                out_receipt->text_hash, item_type,
                                (uint32_t)requested_order, (uint32_t)i);
                        return 1;
                    }
                }
                return 0;
            }
            ++enumerated;
            ++range_start;
        }
        value = 0u;
        range_start = -1;
        money_base = -1;
    }
    return 0;
}

int dm2_v1_gdat_alloc_pict_buff_receipt(
    uint16_t width,
    uint16_t height,
    uint8_t bpp,
    DM2_V1_GdatPictPool pool,
    DM2_V1_GdatPictAllocationReceipt *out_receipt)
{
    uint16_t row_bytes = 0u;
    uint32_t payload_bytes = 0u;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (pool != DM2_V1_GDAT_PICT_POOL_FREE &&
        pool != DM2_V1_GDAT_PICT_POOL_LOBIG) {
        return 0;
    }
    if (!dm2_gdat_pict_row_bytes(width, height, bpp,
                                 &row_bytes, &payload_bytes)) {
        return 0;
    }
    /* skproject c_gdatfile.cpp::DM2_ALLOC_PICT_BUFF asks
     * DM2_ALLOC_MEMORY_RAM for payload+6 bytes and returns the pointer six
     * bytes after the source bitmap header. */
    out_receipt->accepted = 1u;
    out_receipt->bpp = bpp;
    out_receipt->pool = (uint8_t)pool;
    out_receipt->width = width;
    out_receipt->height = height;
    out_receipt->row_bytes = row_bytes;
    out_receipt->payload_bytes = payload_bytes;
    out_receipt->header_bytes = 6u;
    out_receipt->allocation_bytes = payload_bytes + 6u;
    out_receipt->free_bytes = payload_bytes + 6u;
    out_receipt->receipt_hash = dm2_gdat_pict_receipt_hash(
        0x50425546u, 0u, width, height, bpp, (uint8_t)pool,
        out_receipt->allocation_bytes, out_receipt->free_bytes);
    return 1;
}

int dm2_v1_gdat_alloc_new_bmp_receipt(
    uint16_t raw_index,
    uint16_t width,
    uint16_t height,
    uint8_t bpp,
    DM2_V1_GdatPictAllocationReceipt *out_receipt)
{
    uint16_t row_bytes = 0u;
    uint32_t payload_bytes = 0u;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (!dm2_gdat_pict_row_bytes(width, height, bpp,
                                 &row_bytes, &payload_bytes)) {
        return 0;
    }
    /* skproject c_gdatfile.cpp::DM2_ALLOC_NEW_BMP computes the same 4bpp
     * rounded row bytes, allocates only the payload from CPX heap for the
     * GDAT raw index, then writes the six-byte bitmap header in front. */
    out_receipt->accepted = 1u;
    out_receipt->is_cpx_heap = 1u;
    out_receipt->bpp = bpp;
    out_receipt->pool = DM2_V1_GDAT_PICT_POOL_CPXHEAP;
    out_receipt->raw_index = raw_index;
    out_receipt->width = width;
    out_receipt->height = height;
    out_receipt->row_bytes = row_bytes;
    out_receipt->payload_bytes = payload_bytes;
    out_receipt->header_bytes = 6u;
    out_receipt->allocation_bytes = payload_bytes;
    out_receipt->free_bytes = payload_bytes + 14u + (bpp == 4u ? 16u : 0u);
    out_receipt->receipt_hash = dm2_gdat_pict_receipt_hash(
        0x4e424d50u, raw_index, width, height, bpp,
        DM2_V1_GDAT_PICT_POOL_CPXHEAP, out_receipt->allocation_bytes,
        out_receipt->free_bytes);
    return 1;
}

int dm2_v1_gdat_free_pict_buff_receipt(
    const DM2_V1_GdatPictAllocationReceipt *allocation,
    DM2_V1_GdatPictFreeReceipt *out_receipt)
{
    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (!allocation || !allocation->accepted || allocation->is_cpx_heap ||
        allocation->header_bytes != 6u || allocation->free_bytes == 0u) {
        return 0;
    }
    /* skproject c_gdatfile.cpp::DM2_FREE_PICT_BUFF recomputes row bytes from
     * the bitmap header and frees payload+6 through the low bigpool route. */
    out_receipt->accepted = 1u;
    out_receipt->freed_pool = DM2_V1_GDAT_PICT_POOL_LOBIG;
    out_receipt->width = allocation->width;
    out_receipt->height = allocation->height;
    out_receipt->row_bytes = allocation->row_bytes;
    out_receipt->free_bytes = allocation->free_bytes;
    out_receipt->receipt_hash = dm2_gdat_pict_receipt_hash(
        0x46505546u, allocation->raw_index, allocation->width,
        allocation->height, allocation->bpp, out_receipt->freed_pool, 0u,
        out_receipt->free_bytes);
    return 1;
}

int dm2_v1_gdat_free_pict_entry_receipt(
    const DM2_V1_GdatPictAllocationReceipt *allocation,
    int header_matches,
    int has_bigpool_struct_tail,
    int preserved_list_member,
    DM2_V1_GdatPictFreeReceipt *out_receipt)
{
    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (!allocation || !allocation->accepted || !allocation->is_cpx_heap ||
        !header_matches || allocation->free_bytes == 0u) {
        return 0;
    }
    /* skproject c_gdatfile.cpp::DM2_FREE_PICT_ENTRY first admits only a
     * matching 14-byte preserved-GFX header. A bigpool tail takes the
     * STRUCT_BEFORE route; otherwise the preserved list is unlinked before
     * high/low pool byte release. */
    out_receipt->accepted = 1u;
    out_receipt->used_bigpool_struct_before =
        has_bigpool_struct_tail ? 1u : 0u;
    out_receipt->removed_from_preserved_list =
        (!has_bigpool_struct_tail && preserved_list_member) ? 1u : 0u;
    out_receipt->freed_pool = has_bigpool_struct_tail
                                  ? DM2_V1_GDAT_PICT_POOL_CPXHEAP
                                  : DM2_V1_GDAT_PICT_POOL_LOBIG;
    out_receipt->width = allocation->width;
    out_receipt->height = allocation->height;
    out_receipt->row_bytes = allocation->row_bytes;
    out_receipt->free_bytes = allocation->free_bytes;
    out_receipt->receipt_hash = dm2_gdat_pict_receipt_hash(
        0x4650454eu, allocation->raw_index, allocation->width,
        allocation->height, allocation->bpp, out_receipt->freed_pool, 0u,
        out_receipt->free_bytes);
    return 1;
}

int dm2_v1_gdat_bigpool_memory_receipt(
    uint32_t requested_bytes,
    DM2_V1_GdatPictPool pool,
    int clean,
    int deallocate,
    DM2_V1_GdatBigpoolMemoryReceipt *out_receipt)
{
    uint32_t aligned = requested_bytes;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (requested_bytes == 0u ||
        (pool != DM2_V1_GDAT_PICT_POOL_LOBIG &&
         pool != DM2_V1_GDAT_PICT_POOL_HIBIG)) {
        return 0;
    }
    if ((aligned & 1u) != 0u) ++aligned;
    out_receipt->accepted = 1u;
    out_receipt->pool = (uint8_t)pool;
    out_receipt->clean = clean ? 1u : 0u;
    out_receipt->deallocate = deallocate ? 1u : 0u;
    out_receipt->requested_bytes = requested_bytes;
    out_receipt->aligned_bytes = aligned;
    out_receipt->receipt_hash = dm2_gdat_pict_receipt_hash(
        deallocate ? 0x44424f50u : 0x41424f50u, 0u,
        (uint16_t)(aligned & 0xffffu),
        (uint16_t)(aligned >> 16), 0u, (uint8_t)pool,
        requested_bytes, aligned);
    return 1;
}

static int dm2_gdat_cpx_block_words(uint32_t raw_length,
                                    uint16_t *out_words)
{
    uint32_t bytes = ((raw_length & 0xffffu) + 1u) & 0xfffffffeu;

    if (!out_words) return 0;
    bytes += 4u;
    if (bytes == 0u || bytes > 0xfffeu || (bytes & 1u) != 0u) return 0;
    *out_words = (uint16_t)(bytes / 2u);
    return 1;
}

int dm2_v1_gdat_cpx_reserve_receipt(
    uint16_t wp08_word,
    uint32_t byte_count,
    DM2_V1_GdatCpxReserveReceipt *out_receipt)
{
    uint32_t aligned = (byte_count + 1u) & 0xfffffffeu;
    uint32_t words = aligned / 2u;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (byte_count == 0u || aligned == 0u || words > wp08_word ||
        words > 0xffffu) {
        return 0;
    }
    /* skproject R_2D8AD subtracts the requested byte count from wp_08 and
     * returns the new top-down reservation pointer. */
    out_receipt->accepted = 1u;
    out_receipt->old_wp08_word = wp08_word;
    out_receipt->requested_bytes = (uint16_t)byte_count;
    out_receipt->reserved_words = (uint16_t)words;
    out_receipt->new_wp08_word = (uint16_t)(wp08_word - words);
    out_receipt->returned_word = out_receipt->new_wp08_word;
    out_receipt->receipt_hash = dm2_gdat_pict_receipt_hash(
        0x32443841u, 0u, wp08_word, out_receipt->new_wp08_word,
        0u, DM2_V1_GDAT_PICT_POOL_CPXHEAP, byte_count, aligned);
    return 1;
}

int dm2_v1_gdat_cpx_copy_receipt(
    uint16_t wp08_word,
    uint32_t byte_count,
    const uint8_t *source_with_header,
    uint32_t source_byte_count,
    DM2_V1_GdatCpxCopyReceipt *out_receipt)
{
    DM2_V1_GdatCpxReserveReceipt reserve;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (!source_with_header || source_byte_count < byte_count ||
        !dm2_v1_gdat_cpx_reserve_receipt(wp08_word, byte_count, &reserve)) {
        return 0;
    }
    /* skproject R_2D8BA copies from xedxp - 2 into the reservation, then
     * returns wptr + 1 so callers store the GDAT payload pointer after the
     * two-byte length word. */
    out_receipt->accepted = 1u;
    out_receipt->source_header_included = 1u;
    out_receipt->copied_bytes = (uint16_t)byte_count;
    out_receipt->returned_payload_word = (uint16_t)(reserve.returned_word + 1u);
    out_receipt->reserve = reserve;
    out_receipt->receipt_hash = dm2_gdat_pict_receipt_hash(
        0x32443842u, 0u, reserve.old_wp08_word, reserve.new_wp08_word,
        0u, DM2_V1_GDAT_PICT_POOL_CPXHEAP, byte_count,
        out_receipt->returned_payload_word);
    return 1;
}

int dm2_v1_gdat_cpx_compact_receipt(
    uint16_t pool_top_word,
    uint16_t wp08_word,
    const DM2_V1_GdatCpxBlockInput *blocks,
    uint16_t block_count,
    DM2_V1_GdatCpxCompactReceipt *out_receipt)
{
    uint16_t write_word = pool_top_word;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (block_count == 0u) {
        out_receipt->accepted = 1u;
        out_receipt->empty_pool = 1u;
        out_receipt->old_wp08_word = wp08_word;
        out_receipt->new_wp08_word = wp08_word;
        out_receipt->receipt_hash = dm2_gdat_pict_receipt_hash(
            0x32443830u, 0u, pool_top_word, wp08_word, 0u,
            DM2_V1_GDAT_PICT_POOL_CPXHEAP, 0u, wp08_word);
        return 1;
    }
    if (!blocks || block_count > DM2_V1_GDAT_CPX_COMPACT_MAX_BLOCKS ||
        wp08_word > pool_top_word) {
        return 0;
    }
    out_receipt->accepted = 1u;
    out_receipt->old_wp08_word = wp08_word;
    out_receipt->input_block_count = block_count;

    for (uint16_t i = 0u; i < block_count; ++i) {
        const DM2_V1_GdatCpxBlockInput *in = &blocks[i];
        DM2_V1_GdatCpxCompactBlockReceipt *block = &out_receipt->blocks[i];
        uint16_t words = 0u;

        if (!dm2_gdat_cpx_block_words(in->raw_length, &words) ||
            in->old_start_word < wp08_word ||
            in->old_start_word + words > pool_top_word ||
            words > write_word) {
            memset(out_receipt, 0, sizeof(*out_receipt));
            return 0;
        }
        block->raw_index = (uint16_t)(in->raw_index & 0x7fffu);
        block->raw_length = in->raw_length;
        block->word_count = words;
        block->old_start_word = in->old_start_word;
        if (in->marked_free || (in->raw_index & 0x8000u) != 0u) {
            block->skipped_free = 1u;
            ++out_receipt->skipped_free_block_count;
            continue;
        }
        write_word = (uint16_t)(write_word - words);
        block->preserved = 1u;
        block->new_start_word = write_word;
        if (block->new_start_word != block->old_start_word)
            ++out_receipt->moved_block_count;
        ++out_receipt->preserved_block_count;
        out_receipt->receipt_hash = dm2_gdat_pict_receipt_hash(
            out_receipt->receipt_hash ? out_receipt->receipt_hash
                                      : 0x32443830u,
            block->raw_index, block->old_start_word, block->new_start_word,
            0u, DM2_V1_GDAT_PICT_POOL_CPXHEAP,
            block->raw_length, block->word_count);
    }
    out_receipt->new_wp08_word = write_word;
    out_receipt->receipt_hash = dm2_gdat_pict_receipt_hash(
        out_receipt->receipt_hash ? out_receipt->receipt_hash : 0x32443830u,
        out_receipt->preserved_block_count, out_receipt->skipped_free_block_count,
        out_receipt->moved_block_count, 0u, DM2_V1_GDAT_PICT_POOL_CPXHEAP,
        wp08_word, write_word);
    return 1;
}

int dm2_v1_dballoc_3e74_24b8_receipt(
    const DM2_V1_AssetLoader *loader,
    DM2_V1_DballocSoundCensusReceipt *out_receipt)
{
    uint16_t sound_count = 0u;
    uint16_t unique_count = 0u;
    uint16_t max_raw = 0u;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (!loader || !loader->loaded || !loader->entries ||
        !loader->raw_sizes) {
        return 0;
    }
    for (uint16_t i = 0u; i < loader->entry_count; ++i) {
        const DM2_V1_GdatEntry *entry = &loader->entries[i];
        uint16_t raw_index;
        uint32_t raw_length;
        int seen_before = 0;

        if (entry->cls3 != DM2_GDAT_ENTRY_TYPE_SOUND) continue;
        raw_index = (uint16_t)(entry->data_index & 0x7fffu);
        if (raw_index >= loader->raw_data_count) return 0;
        raw_length = loader->raw_sizes[raw_index];
        if (raw_length > 0xffffu) return 0;
        ++sound_count;
        if ((uint16_t)raw_length > max_raw) max_raw = (uint16_t)raw_length;
        for (uint16_t j = 0u; j < i; ++j) {
            if (loader->entries[j].cls3 == DM2_GDAT_ENTRY_TYPE_SOUND &&
                (loader->entries[j].data_index & 0x7fffu) == raw_index) {
                seen_before = 1;
                break;
            }
        }
        if (!seen_before) {
            ++unique_count;
        }
    }
    out_receipt->accepted = 1u;
    out_receipt->sound_entry_count = sound_count;
    out_receipt->unique_raw_index_count = unique_count;
    out_receipt->max_raw_length = max_raw;
    out_receipt->scratch_allocation_bytes = (uint32_t)sound_count * 2u;
    out_receipt->receipt_hash = dm2_gdat_file_receipt_hash(
        0x32346238u, sound_count, unique_count,
        ((uint32_t)max_raw << 16) | (uint32_t)loader->entry_count);
    return 1;
}

int dm2_v1_dballoc_3e74_2162_receipt(
    const DM2_V1_AssetLoader *loader,
    uint16_t entry_ordinal,
    uint8_t active_mask,
    DM2_V1_DballocEntryFilterReceipt *out_receipt)
{
    uint32_t cls5 = 0u;
    uint8_t mask;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (!dm2_v1_query_gdat_entry_value(loader, entry_ordinal,
                                       DM2_GDAT_EP_CLS5, &cls5)) {
        return 0;
    }
    mask = (uint8_t)(cls5 & 0xf0u);
    out_receipt->accepted = 1u;
    out_receipt->entry_ordinal = entry_ordinal;
    out_receipt->cls5_mask = mask;
    out_receipt->active_mask = active_mask;
    out_receipt->allowed = (mask == 0u || mask == active_mask) ? 1u : 0u;
    out_receipt->receipt_hash = dm2_gdat_file_receipt_hash(
        0x32313632u, entry_ordinal, mask, active_mask);
    return 1;
}

int dm2_v1_load_dyn4_admission_receipt(
    const DM2_V1_AssetLoader *loader,
    uint16_t descriptor_count,
    int cache_locked,
    DM2_V1_LoadDyn4AdmissionReceipt *out_receipt)
{
    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (!loader || !loader->loaded || !loader->entries ||
        loader->entry_count == 0u || descriptor_count == 0u) {
        return 0;
    }
    out_receipt->accepted = 1u;
    out_receipt->entry_count = loader->entry_count;
    out_receipt->descriptor_count = descriptor_count;
    out_receipt->marker_allocation_bytes = loader->entry_count;
    out_receipt->requested_sound_cleanup = cache_locked ? 0u : 1u;
    out_receipt->early_dealloc_when_locked = cache_locked ? 1u : 0u;
    out_receipt->receipt_hash = dm2_gdat_file_receipt_hash(
        0x44594e34u, loader->entry_count, descriptor_count,
        ((uint32_t)out_receipt->requested_sound_cleanup << 16) |
            out_receipt->early_dealloc_when_locked);
    return 1;
}

int dm2_v1_asset_load_word_value(
    const DM2_V1_AssetLoader *loader,
    int category,
    int index,
    int field,
    uint16_t *out_value)
{
    const DM2_V1_GdatEntry *entry;

    if (out_value) *out_value = 0u;
    if (!out_value) return 0;
    entry = dm2_gdat_find_entry(loader,
                                category,
                                index,
                                DM2_GDAT_ENTRY_TYPE_WORD_VALUE,
                                field);
    if (!entry) return 0;

    /* skproject/SKWIN/SkWinCore.cpp QUERY_GDAT_ENTRY_DATA_INDEX reads
     * dtWordValue (DME.h: dtWordValue = 11) from the ENT1 data index, not
     * from a raw payload. QUERY_ITEM_VALUE uses fields 0x01/0x02 for item
     * weight and money semantics. */
    *out_value = entry->data_index;
    return 1;
}

int dm2_v1_asset_query_ornate_animation_frame(
    const DM2_V1_AssetLoader *loader, int category, int index,
    uint32_t tick, uint32_t delta, uint16_t *out_frame,
    uint32_t *out_receipt_hash)
{
    enum { ANIMATION_FIELD = 0x0d };
    uint16_t length = 0u;
    uint16_t frame_base = 0u;
    uint32_t hash = 2166136261u;
    const uint8_t *sequence = NULL;
    size_t sequence_size = 0u;
    size_t sequence_length = 0u;

    if (out_frame) *out_frame = 0u;
    if (out_receipt_hash) *out_receipt_hash = 0u;
    if (!loader || !out_frame || !out_receipt_hash || category < 0 ||
        index < 0 || index > 0xff) return 0;
    if (dm2_v1_asset_load_word_value(loader, category, index,
                                     ANIMATION_FIELD, &length)) {
        if (length & 0x8000u) frame_base = 1u;
        length &= 0x7fffu;
        if (length == 0u) return 0;
        *out_frame = (uint16_t)(((tick + delta) % length) + frame_base);
        hash ^= (uint32_t)category; hash *= 16777619u;
        hash ^= (uint32_t)index; hash *= 16777619u;
        hash ^= length; hash *= 16777619u;
        hash ^= frame_base; hash *= 16777619u;
    } else {
        sequence = dm2_v1_asset_load_text_sized(loader, category, index,
                                                 ANIMATION_FIELD,
                                                 &sequence_size);
        if (!sequence || sequence_size == 0u) return 0;
        while (sequence_length < sequence_size && sequence[sequence_length])
            ++sequence_length;
        if (sequence_length == 0u || sequence_length == sequence_size)
            return 0;
        for (size_t i = 0u; i < sequence_length; ++i) {
            uint8_t value = sequence[i];
            if (value >= '0' && value <= '9') value -= '0';
            else if (value >= 'A' && value <= 'Z') value -= 'A' - 10u;
            else return 0;
            if (i == (size_t)((tick + delta) % sequence_length))
                *out_frame = value;
            hash ^= sequence[i]; hash *= 16777619u;
        }
    }
    hash ^= tick; hash *= 16777619u;
    hash ^= delta; hash *= 16777619u;
    hash ^= *out_frame; hash *= 16777619u;
    *out_receipt_hash = hash ? hash : 1u;
    return 1;
}

int dm2_v1_asset_load_image_offset(
    const DM2_V1_AssetLoader *loader,
    int category,
    int index,
    int field,
    uint16_t *out_value)
{
    const DM2_V1_GdatEntry *entry;

    if (out_value) *out_value = 0u;
    if (!out_value) return 0;
    entry = dm2_gdat_find_entry(loader,
                                category,
                                index,
                                DM2_GDAT_ENTRY_TYPE_IMAGE_OFFSET,
                                field);
    if (!entry) return 0;

    /* skproject/SKWIN QUERY_GDAT_PICT_OFFSET reads dtImageOffset
     * (DME.h: dtImageOffset = 12) from the ENT1 data index. */
    *out_value = entry->data_index;
    return 1;
}

int dm2_v1_asset_load_interface_palette(
    const DM2_V1_AssetLoader *loader,
    int category,
    int index,
    int field,
    DM2_V1_InterfacePalette *out_palette)
{
    const uint8_t *irgb;
    const uint8_t *palette16;
    size_t irgb_size = 0u;
    size_t palette16_size = 0u;
    uint32_t hash = 2166136261u;
    int color;

    if (!out_palette) return 0;
    memset(out_palette, 0, sizeof(*out_palette));
    irgb = dm2_v1_asset_load_typed_sized(loader, category, index,
                                          DM2_GDAT_ENTRY_TYPE_PAL_IRGB,
                                          field, &irgb_size);
    palette16 = dm2_v1_asset_load_typed_sized(loader, category, index,
                                               DM2_GDAT_ENTRY_TYPE_PAL_16,
                                               field, &palette16_size);
    /* ReDMCSB-compatible SKWIN INIT copies 0x400 bytes of dtPalIRGB into a
     * 256x4 row buffer, then indexes dtPalette16 byte-wise. */
    if (!irgb || !palette16 || irgb_size < 256u * 4u ||
        palette16_size < sizeof(out_palette->palette16)) {
        return 0;
    }
    for (color = 0; color < 256; ++color) {
        const uint8_t *src = irgb + (size_t)color * 4u;
        out_palette->rgb6[color][0] = (uint8_t)(src[1] >> 2);
        out_palette->rgb6[color][1] = (uint8_t)(src[2] >> 2);
        out_palette->rgb6[color][2] = (uint8_t)(src[3] >> 2);
        hash = (hash ^ out_palette->rgb6[color][0]) * 16777619u;
        hash = (hash ^ out_palette->rgb6[color][1]) * 16777619u;
        hash = (hash ^ out_palette->rgb6[color][2]) * 16777619u;
    }
    memcpy(out_palette->palette16, palette16, sizeof(out_palette->palette16));
    for (color = 0; color < (int)sizeof(out_palette->palette16); ++color) {
        hash = (hash ^ out_palette->palette16[color]) * 16777619u;
    }
    out_palette->hash = hash ? hash : 1u;
    return 1;
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
    uint16_t raw_index;

    if (out_width) *out_width = 0;
    if (out_height) *out_height = 0;
    if (out_format) *out_format = DM2_IMG_FMT_UNKNOWN;
    entry = dm2_gdat_find_entry(loader,
                                category,
                                index,
                                DM2_GDAT_ENTRY_TYPE_IMAGE,
                                field);
    if (!entry) return NULL;
    raw_index = (uint16_t)(entry->data_index & 0x7fffu);
    return dm2_v1_asset_load_raw_image(loader, raw_index,
                                       out_width, out_height, out_format);
}

uint8_t *dm2_v1_asset_load_raw_image(const DM2_V1_AssetLoader *loader,
                                      uint16_t raw_index,
                                      int *out_width, int *out_height,
                                      DM2_ImageFormat *out_format) {
    const uint8_t *raw;
    size_t raw_size = 0u;
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
    raw = dm2_v1_load_gdat_raw_data(loader, raw_index, &raw_size);
    if (!raw || raw_size < DM2_IMG3_HEADER_SIZE) return NULL;

    cx = img_rd16(raw + 0, loader->big_endian);
    cy = img_rd16(raw + 2, loader->big_endian);
    bpp = img_rd16(raw + 4, loader->big_endian);
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
    uint16_t w0 = gdat_rd16(loader, loader->data);
    if (loader->data_size >= DM2_PC_GRAPHICS_MIN_SIZE &&
        loader->data_size <= DM2_PC_GRAPHICS_MAX_SIZE &&
        w0 == DM2_PC_GDAT_CONTAINER_WORD) {
        return 1;
    }
    if (loader->data_size >= DM2_FMTOWNS_GRAPHICS_MIN_SIZE &&
        loader->data_size <= DM2_FMTOWNS_GRAPHICS_MAX_SIZE &&
        w0 == DM2_FMTOWNS_GDAT_CONTAINER_WORD) {
        return 1;
    }
    if (loader->big_endian && w0 == DM2_PC_GDAT_CONTAINER_WORD &&
        loader->data_size >= DM2_FMTOWNS_GRAPHICS_MIN_SIZE) {
        return 1;
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

/* IMG3/IMG9 decode is part of the loader's source-owned raw-image boundary.
 * Keep its implementation in this translation unit: firestaff_m10 owns the
 * loader and several bounded real-data tests deliberately link M10 without
 * the higher-level DM2 archive.  Leaving the decoder solely in firestaff_dm2
 * made those callers fail at link time after IMG9 began using the complete
 * SKProject mode-1/2/3 dispatcher.  The included implementation remains the
 * separately named c_gfx_decode.cpp receipt surface; it is not a fallback or
 * a generated-image path. */
#include "dm2_v1_gfx_decode_receipt.c"
