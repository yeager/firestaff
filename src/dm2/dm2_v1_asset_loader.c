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

/* ── IMG3 decompression (4-bit nibble → pixel bytes) ─────────────── */
/*
 * dm2_img3_decode — decode IMG3 format to 8-bit pixel buffer.
 * IMG3 format: two pixels per byte (4 bits each), with escape sequences.
 * Escape: nibble 15 followed by:
 *   15 → repeat count (next nibble = count, next = value)
 *   0-14 → literal run of that many pixels with value from next nibble
 *
 * Source: ReDMCSB DUNGEON.C — IMG3 decoder (F0687, F0688, F0689)
 */
static DM2_MAYBE_UNUSED uint8_t *dm2_img3_decode(const uint8_t *src, size_t src_size,
                                                  int *out_width, int *out_height,
                                                  int alloc_w, int alloc_h) {
    /* IMG3 output is typically 64x64 or 128x128 for wall/floor tiles.
     * DM2 uses 64x64 as standard tile size for indoor levels.
     * Outdoor level tiles (sky/ground) are larger (256x128 or 640x200).
     * Source: SKULL.ASM T560 viewport rendering */
    int w = alloc_w > 0 ? alloc_w : 64;
    int h = alloc_h > 0 ? alloc_h : 64;
    uint8_t *pixels = malloc((size_t)w * (size_t)h);
    if (!pixels) return NULL;
    memset(pixels, 0, (size_t)w * (size_t)h);

    int src_idx = 0;
    int dst_idx = 0;
    int count_remaining = 0;
    int repeat_value = 0;

    while (dst_idx < w * h && src_idx < (int)src_size) {
        if (count_remaining > 0) {
            pixels[dst_idx++] = (uint8_t)repeat_value;
            count_remaining--;
            continue;
        }

        int nibble = (src_idx % 2 == 0)
                     ? (src[src_idx / 2] >> 4) & 0x0F
                     : src[src_idx / 2] & 0x0F;

        if (nibble == 15) {
            src_idx++;
            if (src_idx >= (int)src_size) break;
            int count_nibble = (src_idx % 2 == 0)
                               ? (src[src_idx / 2] >> 4) & 0x0F
                               : src[src_idx / 2] & 0x0F;
            src_idx++;
            if (src_idx >= (int)src_size) break;
            repeat_value = (src_idx % 2 == 0)
                           ? (src[src_idx / 2] >> 4) & 0x0F
                           : src[src_idx / 2] & 0x0F;
            src_idx++;
            count_remaining = count_nibble;
        } else {
            pixels[dst_idx++] = (uint8_t)nibble;
        }
    }

    if (out_width) *out_width = w;
    if (out_height) *out_height = h;
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
    (void)loader; (void)category; (void)index;
    if (out_width) *out_width = 0;
    if (out_height) *out_height = 0;
    if (out_format) *out_format = DM2_IMG_FMT_UNKNOWN;
    /* skproject separates GDAT indexing from image realization
     * (QUERY_GDAT_IMAGE_ENTRY_BUFF / QUERY_PICST_IT).  Firestaff now has the
     * index; IMG realization remains the next startup-rendering step. */
    return NULL;
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
