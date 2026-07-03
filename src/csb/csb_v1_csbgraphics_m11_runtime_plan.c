#include "csb_v1_csbgraphics_m11_runtime_plan.h"
#include "csb_v1_viewport_custom_backgrounds_room_slot_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
    CSB_GRAPHIC_CUSTOM_BACKGROUND_SKIN_DEF = 1,
    CSB_GRAPHIC_CUSTOM_BACKGROUND_SKIN_DEF_GRAPHIC_ID = 1,
    CSB_GRAPHIC_CUSTOM_BACKGROUND_SKIN_DEF_MIN_BYTES = 18,
    CSB_GRAPHIC_INVENTORY = 17,
    CSB_GRAPHIC_PANEL_RESURRECT_REINCARNATE = 40,
    CSB_GRAPHIC_FIELD_MIN = 73,
    CSB_GRAPHIC_FIELD_MAX = 74,
    CSB_GRAPHIC_EXPLOSION_MIN = 351,
    CSB_GRAPHIC_EXPLOSION_MAX = 359
};

static const char s_source_evidence[] =
    "CSBgraphics.dat M11 runtime plan: CSBWin/Graphics.cpp:1838 "
    "OpenCSBgraphicsFile; Graphics.cpp:1918 ReadGraphicsIndex; "
    "Graphics.cpp:1643 LocateNthGraphic; Graphics.cpp:1717 ReadGraphic; "
    "ReDMCSB DEFS.H C017/C040/C000_DERIVED_BITMAP_VIEWPORT; "
    "ReDMCSB PANEL.C F0346/F0370 panel and viewport blit lanes; "
    "CSB-lineage Viewport.cpp:6451-6505 ApplyBackground masked composite; "
    "Viewport.cpp:6599-6619 CustomBackgrounds skin-def bitmap/mask pairs.";

static void copy_text(char *dst, size_t dst_size, const char *src)
{
    if (!dst || dst_size == 0u) {
        return;
    }
    snprintf(dst, dst_size, "%s", src ? src : "");
}

void csb_v1_csbgraphics_m11_runtime_plan_init(
    CSB_V1_CSBGraphicsM11RuntimePlan *plan)
{
    if (!plan) {
        return;
    }
    memset(plan, 0, sizeof(*plan));
}

static int route_is_viewport(uint32_t entry_index)
{
    return (entry_index >= CSB_GRAPHIC_FIELD_MIN &&
            entry_index <= CSB_GRAPHIC_FIELD_MAX) ||
           (entry_index >= CSB_GRAPHIC_EXPLOSION_MIN &&
            entry_index <= CSB_GRAPHIC_EXPLOSION_MAX);
}

static int find_pair(const CSB_V1_CSBGraphicsM11RuntimePlan *plan,
                     uint32_t bitmap_entry_index,
                     uint32_t mask_entry_index)
{
    uint32_t i;
    if (!plan) {
        return 0;
    }
    for (i = 0u; i < plan->planned_count; ++i) {
        if (plan->entries[i].entry_index == bitmap_entry_index &&
            plan->entries[i].mask_entry_index == mask_entry_index) {
            return 1;
        }
    }
    return 0;
}

static const CSB_V1_CSBGraphicsM11RuntimePlanEntry *
find_custom_background_entry_by_pair(
    const CSB_V1_CSBGraphicsM11RuntimePlan *plan,
    uint32_t bitmap_entry_index,
    uint32_t mask_entry_index,
    CSB_V1_CSBGraphicsM11CustomBackgroundLayer layer)
{
    uint32_t i;
    if (!plan) {
        return NULL;
    }
    for (i = 0u; i < plan->planned_count; ++i) {
        const CSB_V1_CSBGraphicsM11RuntimePlanEntry *entry = &plan->entries[i];
        if (entry->entry_index == bitmap_entry_index &&
            entry->mask_entry_index == mask_entry_index &&
            entry->route == CSB_V1_CSBGRAPHICS_M11_ROUTE_VIEWPORT_CUSTOM_BACKGROUND &&
            entry->custom_background_layer == layer) {
            return entry;
        }
    }
    return NULL;
}

static int custom_background_layer_skin_def_indices(
    int room_num,
    CSB_V1_CSBGraphicsM11CustomBackgroundLayer layer,
    int *out_bitmap_index,
    int *out_mask_index)
{
    const CSB_V1_CustomBackgroundsRoomSlotContract *contract =
        csb_v1_viewport_custom_backgrounds_room_slot_contract_pc34();

    if (!contract || !out_bitmap_index || !out_mask_index ||
        room_num < 0 || room_num >= contract->room_slot_count) {
        return 0;
    }

    switch (layer) {
    case CSB_V1_CSBGRAPHICS_M11_CUSTOM_BACKGROUND_LAYER_LARGE:
        *out_bitmap_index = contract->large_bitmap_skin_def_index;
        *out_mask_index = contract->large_mask_skin_def_index;
        return 1;
    case CSB_V1_CSBGRAPHICS_M11_CUSTOM_BACKGROUND_LAYER_MIDDLE:
        *out_bitmap_index = contract->middle_bitmap_skin_def_index;
        *out_mask_index = contract->middle_mask_skin_def_index;
        return 1;
    case CSB_V1_CSBGRAPHICS_M11_CUSTOM_BACKGROUND_LAYER_NEAR:
        if (room_num >= contract->near_layer_room_num_limit) {
            return 0;
        }
        *out_bitmap_index = contract->near_bitmap_skin_def_index;
        *out_mask_index = contract->near_mask_skin_def_index;
        return 1;
    default:
        return 0;
    }
}

static int known_geometry_for_entry(uint32_t entry_index,
                                    uint16_t decompressed_size,
                                    uint16_t *out_w,
                                    uint16_t *out_h,
                                    CSB_V1_CSBGraphicsM11Route *out_route)
{
    if (!out_w || !out_h || !out_route) {
        return 0;
    }
    if (entry_index == CSB_GRAPHIC_INVENTORY &&
        decompressed_size ==
            (uint16_t)(CSB_V1_CSBGRAPHICS_M11_VIEWPORT_W *
                       CSB_V1_CSBGRAPHICS_M11_VIEWPORT_H)) {
        *out_w = CSB_V1_CSBGRAPHICS_M11_VIEWPORT_W;
        *out_h = CSB_V1_CSBGRAPHICS_M11_VIEWPORT_H;
        *out_route = CSB_V1_CSBGRAPHICS_M11_ROUTE_HUD_INVENTORY;
        return 1;
    }
    if (entry_index == CSB_GRAPHIC_PANEL_RESURRECT_REINCARNATE &&
        decompressed_size ==
            (uint16_t)(CSB_V1_CSBGRAPHICS_M11_C040_PANEL_W *
                       CSB_V1_CSBGRAPHICS_M11_C040_PANEL_H)) {
        *out_w = CSB_V1_CSBGRAPHICS_M11_C040_PANEL_W;
        *out_h = CSB_V1_CSBGRAPHICS_M11_C040_PANEL_H;
        *out_route = CSB_V1_CSBGRAPHICS_M11_ROUTE_HUD_RESURRECT_PANEL;
        return 1;
    }
    if (route_is_viewport(entry_index) &&
        decompressed_size ==
            (uint16_t)(CSB_V1_CSBGRAPHICS_M11_VIEWPORT_W *
                       CSB_V1_CSBGRAPHICS_M11_VIEWPORT_H)) {
        *out_w = CSB_V1_CSBGRAPHICS_M11_VIEWPORT_W;
        *out_h = CSB_V1_CSBGRAPHICS_M11_VIEWPORT_H;
        *out_route = CSB_V1_CSBGRAPHICS_M11_ROUTE_VIEWPORT_DERIVED;
        return 1;
    }
    return 0;
}

static int append_entry(CSB_V1_CSBGraphicsM11RuntimePlan *plan,
                        const CSB_V1_CSBGraphicsEntrySpan *span,
                        uint16_t width,
                        uint16_t height,
                        CSB_V1_CSBGraphicsM11Route route,
                        int explicit_dimensions)
{
    CSB_V1_CSBGraphicsM11RuntimePlanEntry *entry;

    if (!plan || !span || width == 0u || height == 0u) {
        return CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_ERR_ARGUMENT;
    }
    if (plan->planned_count >=
        CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_MAX_ENTRIES) {
        return CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_ERR_FULL;
    }

    entry = &plan->entries[plan->planned_count++];
    memset(entry, 0, sizeof(*entry));
    entry->entry_index = span->entry_index;
    entry->expected_width = width;
    entry->expected_height = height;
    entry->decompressed_size = span->decompressed_size;
    entry->route = route;
    entry->explicit_dimensions = explicit_dimensions ? 1 : 0;
    entry->needs_viewport_redraw =
        (route == CSB_V1_CSBGRAPHICS_M11_ROUTE_VIEWPORT_DERIVED) ? 1 : 0;
    entry->needs_hud_redraw =
        (route == CSB_V1_CSBGRAPHICS_M11_ROUTE_HUD_INVENTORY ||
         route == CSB_V1_CSBGRAPHICS_M11_ROUTE_HUD_RESURRECT_PANEL) ? 1 : 0;
    plan->ready = 1;
    return CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_OK;
}

static int append_custom_background_pair(
    CSB_V1_CSBGraphicsM11RuntimePlan *plan,
    const CSB_V1_CSBGraphicsEntrySpan *bitmap_span,
    const CSB_V1_CSBGraphicsEntrySpan *mask_span,
    CSB_V1_CSBGraphicsM11CustomBackgroundLayer layer)
{
    CSB_V1_CSBGraphicsM11RuntimePlanEntry *entry;

    if (!plan || !bitmap_span || !mask_span ||
        bitmap_span->entry_index == 0u ||
        mask_span->entry_index == 0u ||
        bitmap_span->decompressed_size == 0u ||
        mask_span->decompressed_size == 0u) {
        return CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_ERR_ARGUMENT;
    }
    if (find_pair(plan, bitmap_span->entry_index, mask_span->entry_index)) {
        return CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_OK;
    }
    if (plan->planned_count >=
        CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_MAX_ENTRIES) {
        return CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_ERR_FULL;
    }

    entry = &plan->entries[plan->planned_count++];
    memset(entry, 0, sizeof(*entry));
    entry->entry_index = bitmap_span->entry_index;
    entry->mask_entry_index = mask_span->entry_index;
    entry->decompressed_size = bitmap_span->decompressed_size;
    entry->mask_decompressed_size = mask_span->decompressed_size;
    entry->route = CSB_V1_CSBGRAPHICS_M11_ROUTE_VIEWPORT_CUSTOM_BACKGROUND;
    entry->needs_viewport_redraw = 1;
    entry->needs_custom_background_composite = 1;
    entry->deferred_masked_composite = 1;
    entry->custom_background_layer = layer;
    ++plan->custom_background_pair_count;
    plan->ready = 1;
    return CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_OK;
}

static int cache_ready(const CSB_V1_CSBGraphicsDatRealCache *cache)
{
    return cache && cache->loaded && cache->file_buffer && cache->file_size > 0u;
}

static void copy_cache_metadata(const CSB_V1_CSBGraphicsDatRealCache *cache,
                                CSB_V1_CSBGraphicsM11RuntimePlan *plan)
{
    plan->cache_loaded = cache && cache->loaded ? 1 : 0;
    if (!cache) {
        return;
    }
    plan->source_entry_count = cache->index.count;
    copy_text(plan->source_path, sizeof(plan->source_path),
              cache->resolved_path);
    copy_text(plan->source_md5, sizeof(plan->source_md5),
              cache->matched_md5);
    copy_text(plan->source_label, sizeof(plan->source_label),
              cache->matched_label);
}

static uint16_t read_le16_bytes(const uint8_t *bytes)
{
    return (uint16_t)((uint16_t)bytes[0] | ((uint16_t)bytes[1] << 8));
}

static uint32_t read_le32_bytes(const uint8_t *bytes)
{
    return (uint32_t)bytes[0] |
           ((uint32_t)bytes[1] << 8) |
           ((uint32_t)bytes[2] << 16) |
           ((uint32_t)bytes[3] << 24);
}

static int16_t read_le16s_bytes(const uint8_t *bytes)
{
    return (int16_t)read_le16_bytes(bytes);
}

static int decode_entry_bytes(const CSB_V1_CSBGraphicsDatRealCache *cache,
                              uint32_t entry_index,
                              uint16_t expected_size,
                              uint8_t **out_bytes)
{
    uint8_t *bytes;
    size_t written = 0u;
    int rc;

    if (!cache_ready(cache) || !out_bytes || expected_size == 0u) {
        return CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_ERR_ARGUMENT;
    }
    *out_bytes = NULL;
    bytes = (uint8_t *)malloc((size_t)expected_size);
    if (!bytes) {
        return CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_ERR_APPLY;
    }
    rc = csb_v1_csbgraphics_dat_decode_entry(cache->file_buffer,
                                             cache->file_size,
                                             entry_index,
                                             bytes,
                                             (size_t)expected_size,
                                             &written);
    if (rc != CSB_V1_CSBGRAPHICS_CLASSIFY_OK ||
        written != (size_t)expected_size) {
        free(bytes);
        return CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_ERR_APPLY;
    }
    *out_bytes = bytes;
    return CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_OK;
}

static uint32_t *decode_entry_words32(
    const CSB_V1_CSBGraphicsDatRealCache *cache,
    uint32_t entry_index,
    uint16_t expected_size,
    size_t *out_word_count)
{
    uint8_t *bytes = NULL;
    uint32_t *words;
    size_t count;
    size_t i;
    int rc;

    if (out_word_count) {
        *out_word_count = 0u;
    }
    if (expected_size == 0u || (expected_size & 3u) != 0u) {
        return NULL;
    }
    rc = decode_entry_bytes(cache, entry_index, expected_size, &bytes);
    if (rc != CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_OK || !bytes) {
        return NULL;
    }
    count = (size_t)expected_size / 4u;
    words = (uint32_t *)malloc(count * sizeof(uint32_t));
    if (!words) {
        free(bytes);
        return NULL;
    }
    for (i = 0u; i < count; ++i) {
        words[i] = read_le32_bytes(bytes + i * 4u);
    }
    free(bytes);
    if (out_word_count) {
        *out_word_count = count;
    }
    return words;
}

static uint16_t *decode_entry_words16(
    const CSB_V1_CSBGraphicsDatRealCache *cache,
    uint32_t entry_index,
    uint16_t expected_size,
    size_t *out_word_count)
{
    uint8_t *bytes = NULL;
    uint16_t *words;
    size_t count;
    size_t i;
    int rc;

    if (out_word_count) {
        *out_word_count = 0u;
    }
    if (expected_size == 0u || (expected_size & 1u) != 0u) {
        return NULL;
    }
    rc = decode_entry_bytes(cache, entry_index, expected_size, &bytes);
    if (rc != CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_OK || !bytes) {
        return NULL;
    }
    count = (size_t)expected_size / 2u;
    words = (uint16_t *)malloc(count * sizeof(uint16_t));
    if (!words) {
        free(bytes);
        return NULL;
    }
    for (i = 0u; i < count; ++i) {
        words[i] = read_le16_bytes(bytes + i * 2u);
    }
    free(bytes);
    if (out_word_count) {
        *out_word_count = count;
    }
    return words;
}

static int decode_custom_background_mask_for_room(
    const CSB_V1_CSBGraphicsDatRealCache *cache,
    uint32_t mask_entry_index,
    uint16_t expected_size,
    int room_num,
    CSB_V1_ViewportCustomBackgroundMask *out_mask,
    uint16_t **out_mask_words)
{
    uint8_t *bytes = NULL;
    const uint8_t *mask_bytes;
    uint32_t mask_count;
    uint32_t mask_offset;
    size_t mask_word_count;
    size_t mask_word_offset;
    size_t i;
    int rc;

    if (!out_mask || !out_mask_words) {
        return CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_ERR_ARGUMENT;
    }
    memset(out_mask, 0, sizeof(*out_mask));
    *out_mask_words = NULL;
    if (!cache_ready(cache) || expected_size == 0u || room_num < 0) {
        return CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_ERR_ARGUMENT;
    }

    rc = decode_entry_bytes(cache, mask_entry_index, expected_size, &bytes);
    if (rc != CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_OK || !bytes) {
        return CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_ERR_APPLY;
    }

    /* CSBWin Viewport.cpp:5367-5381 GetMask(): decoded mask graphics start
     * with a uint32 count followed by uint32 offsets; each offset points to
     * BACKGROUND_MASK from CSB.h:330-338. */
    if (expected_size < 8u) {
        free(bytes);
        return CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_ERR_GEOMETRY;
    }
    mask_count = read_le32_bytes(bytes);
    if ((uint32_t)room_num >= mask_count ||
        ((size_t)room_num + 2u) * 4u > (size_t)expected_size) {
        free(bytes);
        return CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_ERR_NO_SUPPORTED_ENTRIES;
    }
    mask_offset = read_le32_bytes(bytes + ((size_t)room_num + 1u) * 4u);
    if (mask_offset == 0u ||
        mask_offset + 12u > (uint32_t)expected_size) {
        free(bytes);
        return CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_ERR_NO_SUPPORTED_ENTRIES;
    }

    mask_bytes = bytes + mask_offset;
    out_mask->src_x = (int)read_le16_bytes(mask_bytes + 0u);
    out_mask->src_y = (int)read_le16_bytes(mask_bytes + 2u);
    out_mask->dst_x = (int)read_le16s_bytes(mask_bytes + 4u);
    out_mask->dst_y = (int)read_le16_bytes(mask_bytes + 6u);
    out_mask->width = (int)read_le16_bytes(mask_bytes + 8u);
    out_mask->height = (int)read_le16_bytes(mask_bytes + 10u);
    if (out_mask->width <= 0 || out_mask->height <= 0) {
        free(bytes);
        return CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_ERR_GEOMETRY;
    }

    mask_word_count =
        ((size_t)out_mask->width / 16u) * (size_t)out_mask->height;
    mask_word_offset = (size_t)mask_offset + 12u;
    if ((out_mask->width & 15) != 0 ||
        mask_word_count == 0u ||
        mask_word_offset + mask_word_count * 2u > (size_t)expected_size) {
        free(bytes);
        return CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_ERR_GEOMETRY;
    }

    *out_mask_words = (uint16_t *)malloc(mask_word_count * sizeof(uint16_t));
    if (!*out_mask_words) {
        free(bytes);
        return CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_ERR_APPLY;
    }
    for (i = 0u; i < mask_word_count; ++i) {
        (*out_mask_words)[i] =
            read_le16_bytes(bytes + mask_word_offset + i * 2u);
    }
    out_mask->mask_words = *out_mask_words;
    out_mask->mask_word_count = mask_word_count;
    free(bytes);
    return CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_OK;
}

int csb_v1_csbgraphics_m11_runtime_plan_decode_custom_background_skin_def(
    const CSB_V1_CSBGraphicsDatRealCache *cache,
    uint16_t *out_skin_def_words,
    size_t out_skin_def_word_capacity,
    size_t *out_skin_def_word_count)
{
    CSB_V1_CSBGraphicsEntrySpan span;
    uint8_t *bytes = NULL;
    uint32_t skin_num;
    uint32_t skin_count;
    int rc;

    if (out_skin_def_word_count) {
        *out_skin_def_word_count = 0u;
    }
    if (!cache_ready(cache) || !out_skin_def_words ||
        out_skin_def_word_capacity == 0u) {
        return CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_ERR_ARGUMENT;
    }

    rc = csb_v1_csbgraphics_dat_entry_span(
        cache->file_buffer,
        cache->file_size,
        CSB_GRAPHIC_CUSTOM_BACKGROUND_SKIN_DEF,
        &span);
    if (rc != CSB_V1_CSBGRAPHICS_CLASSIFY_OK ||
        span.compressed_size == 0u ||
        span.decompressed_size <
            CSB_GRAPHIC_CUSTOM_BACKGROUND_SKIN_DEF_MIN_BYTES ||
        (span.decompressed_size & 1u) != 0u) {
        return CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_ERR_NO_SUPPORTED_ENTRIES;
    }

    rc = decode_entry_bytes(cache,
                            CSB_GRAPHIC_CUSTOM_BACKGROUND_SKIN_DEF,
                            span.decompressed_size,
                            &bytes);
    if (rc != CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_OK || !bytes) {
        return CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_ERR_APPLY;
    }

    skin_count = read_le16_bytes(bytes);
    for (skin_num = 0u; skin_num < skin_count; ++skin_num) {
        uint32_t offset_index = skin_num + 1u;
        uint32_t skin_def_offset;
        if ((size_t)(offset_index + 1u) * 2u > (size_t)span.decompressed_size) {
            break;
        }
        skin_def_offset = read_le16_bytes(bytes + (size_t)offset_index * 2u);
        if (skin_def_offset == 0u ||
            (skin_def_offset & 1u) != 0u ||
            skin_def_offset +
                (uint32_t)CSB_GRAPHIC_CUSTOM_BACKGROUND_SKIN_DEF_MIN_BYTES >
                span.decompressed_size) {
            continue;
        }
        free(bytes);
        return csb_v1_csbgraphics_m11_runtime_plan_decode_custom_background_skin_def_for_skin(
            cache,
            skin_num,
            out_skin_def_words,
            out_skin_def_word_capacity,
            out_skin_def_word_count);
    }

    free(bytes);
    return CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_ERR_NO_SUPPORTED_ENTRIES;
}

int csb_v1_csbgraphics_m11_runtime_plan_decode_custom_background_skin_def_for_skin(
    const CSB_V1_CSBGraphicsDatRealCache *cache,
    uint32_t skin_num,
    uint16_t *out_skin_def_words,
    size_t out_skin_def_word_capacity,
    size_t *out_skin_def_word_count)
{
    CSB_V1_CSBGraphicsEntrySpan span;
    uint8_t *bytes = NULL;
    uint32_t skin_count;
    uint32_t skin_def_offset;
    uint32_t next_skin_def_offset = 0u;
    size_t skin_def_bytes;
    size_t word_count;
    size_t i;
    int rc;

    if (out_skin_def_word_count) {
        *out_skin_def_word_count = 0u;
    }
    if (!cache_ready(cache) || !out_skin_def_words ||
        out_skin_def_word_capacity == 0u) {
        return CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_ERR_ARGUMENT;
    }

    rc = csb_v1_csbgraphics_dat_entry_span(
        cache->file_buffer,
        cache->file_size,
        CSB_GRAPHIC_CUSTOM_BACKGROUND_SKIN_DEF,
        &span);
    if (rc != CSB_V1_CSBGRAPHICS_CLASSIFY_OK ||
        span.compressed_size == 0u ||
        span.decompressed_size <
            CSB_GRAPHIC_CUSTOM_BACKGROUND_SKIN_DEF_MIN_BYTES ||
        (span.decompressed_size & 1u) != 0u) {
        return CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_ERR_NO_SUPPORTED_ENTRIES;
    }

    rc = decode_entry_bytes(cache,
                            CSB_GRAPHIC_CUSTOM_BACKGROUND_SKIN_DEF,
                            span.decompressed_size,
                            &bytes);
    if (rc != CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_OK || !bytes) {
        return CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_ERR_APPLY;
    }

    /* CSBWin Viewport.cpp:5662-5690 GetSkinDef() treats CSBgraphics entry 1
     * as a skin index: word 0 is the skin count, word skin+1 is the byte
     * offset to that skin's pSkinDef table. */
    skin_count = read_le16_bytes(bytes);
    if (skin_num >= skin_count ||
        (size_t)(skin_num + 2u) * 2u > (size_t)span.decompressed_size) {
        free(bytes);
        return CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_ERR_NO_SUPPORTED_ENTRIES;
    }
    skin_def_offset = read_le16_bytes(bytes + (size_t)(skin_num + 1u) * 2u);
    if (skin_def_offset == 0u ||
        (skin_def_offset & 1u) != 0u ||
        skin_def_offset +
            (uint32_t)CSB_GRAPHIC_CUSTOM_BACKGROUND_SKIN_DEF_MIN_BYTES >
            span.decompressed_size) {
        free(bytes);
        return CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_ERR_NO_SUPPORTED_ENTRIES;
    }

    for (i = (size_t)skin_num + 1u; i < (size_t)skin_count; ++i) {
        uint32_t candidate;
        if ((i + 2u) * 2u > (size_t)span.decompressed_size) {
            break;
        }
        candidate = read_le16_bytes(bytes + (i + 1u) * 2u);
        if (candidate > skin_def_offset &&
            candidate <= span.decompressed_size &&
            (candidate & 1u) == 0u &&
            (next_skin_def_offset == 0u || candidate < next_skin_def_offset)) {
            next_skin_def_offset = candidate;
        }
    }
    skin_def_bytes =
        (size_t)(next_skin_def_offset ? next_skin_def_offset
                                      : span.decompressed_size) -
        (size_t)skin_def_offset;
    word_count = skin_def_bytes / 2u;
    if (word_count > out_skin_def_word_capacity) {
        free(bytes);
        return CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_ERR_GEOMETRY;
    }

    for (i = 0u; i < word_count; ++i) {
        out_skin_def_words[i] =
            read_le16_bytes(bytes + (size_t)skin_def_offset + i * 2u);
    }
    free(bytes);
    if (out_skin_def_word_count) {
        *out_skin_def_word_count = word_count;
    }
    return CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_OK;
}

int csb_v1_csbgraphics_m11_runtime_plan_add_explicit_entry(
    const CSB_V1_CSBGraphicsDatRealCache *cache,
    uint32_t entry_index,
    uint16_t expected_width,
    uint16_t expected_height,
    CSB_V1_CSBGraphicsM11RuntimePlan *plan)
{
    CSB_V1_CSBGraphicsEntrySpan span;
    CSB_V1_CSBGraphicsM11Route route = CSB_V1_CSBGRAPHICS_M11_ROUTE_NONE;
    uint16_t expected_pixels;
    int rc;

    if (!plan || !cache_ready(cache) ||
        expected_width == 0u || expected_height == 0u) {
        return CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_ERR_ARGUMENT;
    }
    if ((size_t)expected_width * (size_t)expected_height > 65535u) {
        return CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_ERR_GEOMETRY;
    }
    expected_pixels = (uint16_t)(expected_width * expected_height);
    rc = csb_v1_csbgraphics_dat_entry_span(cache->file_buffer,
                                           cache->file_size,
                                           entry_index,
                                           &span);
    if (rc != CSB_V1_CSBGRAPHICS_CLASSIFY_OK ||
        span.compressed_size == 0u ||
        span.decompressed_size != expected_pixels) {
        return CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_ERR_GEOMETRY;
    }

    if (entry_index == CSB_GRAPHIC_INVENTORY) {
        route = CSB_V1_CSBGRAPHICS_M11_ROUTE_HUD_INVENTORY;
    } else if (entry_index == CSB_GRAPHIC_PANEL_RESURRECT_REINCARNATE) {
        route = CSB_V1_CSBGRAPHICS_M11_ROUTE_HUD_RESURRECT_PANEL;
    } else if (route_is_viewport(entry_index)) {
        route = CSB_V1_CSBGRAPHICS_M11_ROUTE_VIEWPORT_DERIVED;
    }
    if (route == CSB_V1_CSBGRAPHICS_M11_ROUTE_NONE) {
        return CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_ERR_GEOMETRY;
    }

    copy_cache_metadata(cache, plan);
    ++plan->supported_present_count;
    return append_entry(plan, &span, expected_width, expected_height,
                        route, 1);
}

static int add_custom_layer(
    const CSB_V1_CSBGraphicsDatRealCache *cache,
    const uint16_t *skin_def_words,
    int bitmap_index,
    int mask_index,
    CSB_V1_CSBGraphicsM11CustomBackgroundLayer layer,
    CSB_V1_CSBGraphicsM11RuntimePlan *plan)
{
    CSB_V1_CSBGraphicsEntrySpan bitmap_span;
    CSB_V1_CSBGraphicsEntrySpan mask_span;
    uint32_t bitmap_entry_index;
    uint32_t mask_entry_index;
    int rc_bitmap;
    int rc_mask;

    if (!cache_ready(cache) || !skin_def_words || !plan ||
        bitmap_index < 0 || mask_index < 0) {
        return CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_ERR_ARGUMENT;
    }
    bitmap_entry_index = skin_def_words[bitmap_index];
    mask_entry_index = skin_def_words[mask_index];
    if (bitmap_entry_index == 0u || mask_entry_index == 0u) {
        return CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_ERR_NO_SUPPORTED_ENTRIES;
    }

    rc_bitmap = csb_v1_csbgraphics_dat_entry_span(cache->file_buffer,
                                                  cache->file_size,
                                                  bitmap_entry_index,
                                                  &bitmap_span);
    rc_mask = csb_v1_csbgraphics_dat_entry_span(cache->file_buffer,
                                                cache->file_size,
                                                mask_entry_index,
                                                &mask_span);
    if (rc_bitmap != CSB_V1_CSBGRAPHICS_CLASSIFY_OK ||
        rc_mask != CSB_V1_CSBGRAPHICS_CLASSIFY_OK ||
        bitmap_span.compressed_size == 0u ||
        mask_span.compressed_size == 0u) {
        return CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_ERR_NO_SUPPORTED_ENTRIES;
    }
    if (find_pair(plan, bitmap_span.entry_index, mask_span.entry_index)) {
        return CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_ERR_NO_SUPPORTED_ENTRIES;
    }
    return append_custom_background_pair(plan, &bitmap_span, &mask_span, layer);
}

int csb_v1_csbgraphics_m11_runtime_plan_add_custom_background_skin_def(
    const CSB_V1_CSBGraphicsDatRealCache *cache,
    const uint16_t *skin_def_words,
    size_t skin_def_word_count,
    CSB_V1_CSBGraphicsM11RuntimePlan *plan)
{
    const CSB_V1_CustomBackgroundsRoomSlotContract *contract;
    int rc;
    int added = 0;
    int missing = 0;

    if (!plan || !cache_ready(cache) || !skin_def_words) {
        return CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_ERR_ARGUMENT;
    }
    contract = csb_v1_viewport_custom_backgrounds_room_slot_contract_pc34();
    if (!contract ||
        skin_def_word_count < (size_t)contract->skin_def_min_words) {
        return CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_ERR_ARGUMENT;
    }

    copy_cache_metadata(cache, plan);

    rc = add_custom_layer(
        cache, skin_def_words,
        contract->large_bitmap_skin_def_index,
        contract->large_mask_skin_def_index,
        CSB_V1_CSBGRAPHICS_M11_CUSTOM_BACKGROUND_LAYER_LARGE,
        plan);
    if (rc == CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_OK) {
        ++added;
    } else if (rc == CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_ERR_FULL ||
               rc == CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_ERR_GEOMETRY) {
        return rc;
    } else {
        ++missing;
    }

    rc = add_custom_layer(
        cache, skin_def_words,
        contract->middle_bitmap_skin_def_index,
        contract->middle_mask_skin_def_index,
        CSB_V1_CSBGRAPHICS_M11_CUSTOM_BACKGROUND_LAYER_MIDDLE,
        plan);
    if (rc == CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_OK) {
        ++added;
    } else if (rc == CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_ERR_FULL ||
               rc == CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_ERR_GEOMETRY) {
        return rc;
    } else {
        ++missing;
    }

    rc = add_custom_layer(
        cache, skin_def_words,
        contract->near_bitmap_skin_def_index,
        contract->near_mask_skin_def_index,
        CSB_V1_CSBGRAPHICS_M11_CUSTOM_BACKGROUND_LAYER_NEAR,
        plan);
    if (rc == CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_OK) {
        ++added;
    } else if (rc == CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_ERR_FULL ||
               rc == CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_ERR_GEOMETRY) {
        return rc;
    } else {
        ++missing;
    }

    plan->supported_present_count += (uint32_t)added;
    plan->skipped_unknown_geometry_count += (uint32_t)missing;
    return added > 0 ? CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_OK
                     : CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_ERR_NO_SUPPORTED_ENTRIES;
}

int csb_v1_csbgraphics_m11_runtime_plan_build_from_cache(
    const CSB_V1_CSBGraphicsDatRealCache *cache,
    CSB_V1_CSBGraphicsM11RuntimePlan *plan)
{
    static const uint32_t candidates[] = {
        CSB_GRAPHIC_INVENTORY,
        CSB_GRAPHIC_PANEL_RESURRECT_REINCARNATE,
        CSB_GRAPHIC_FIELD_MIN,
        CSB_GRAPHIC_FIELD_MAX,
        351u, 352u, 353u, 354u, 355u, 356u, 357u, 358u, 359u
    };
    size_t i;

    if (!plan) {
        return CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_ERR_ARGUMENT;
    }
    csb_v1_csbgraphics_m11_runtime_plan_init(plan);
    if (!cache_ready(cache)) {
        return CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_ERR_NO_CACHE;
    }
    copy_cache_metadata(cache, plan);

    for (i = 0u; i < sizeof(candidates) / sizeof(candidates[0]); ++i) {
        CSB_V1_CSBGraphicsEntrySpan span;
        uint16_t width = 0u;
        uint16_t height = 0u;
        CSB_V1_CSBGraphicsM11Route route =
            CSB_V1_CSBGRAPHICS_M11_ROUTE_NONE;
        int rc;

        if (candidates[i] >= cache->index.count) {
            continue;
        }
        rc = csb_v1_csbgraphics_dat_entry_span(cache->file_buffer,
                                               cache->file_size,
                                               candidates[i],
                                               &span);
        if (rc != CSB_V1_CSBGRAPHICS_CLASSIFY_OK ||
            span.compressed_size == 0u ||
            span.decompressed_size == 0u) {
            continue;
        }
        ++plan->supported_present_count;
        if (!known_geometry_for_entry(span.entry_index,
                                      span.decompressed_size,
                                      &width,
                                      &height,
                                      &route)) {
            ++plan->skipped_unknown_geometry_count;
            continue;
        }
        rc = append_entry(plan, &span, width, height, route, 0);
        if (rc != CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_OK) {
            return rc;
        }
    }

    if (plan->planned_count == 0u) {
        return CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_ERR_NO_SUPPORTED_ENTRIES;
    }
    return CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_OK;
}

const CSB_V1_CSBGraphicsM11RuntimePlanEntry *
csb_v1_csbgraphics_m11_runtime_plan_find_entry(
    const CSB_V1_CSBGraphicsM11RuntimePlan *plan,
    uint32_t entry_index)
{
    uint32_t i;
    if (!plan) {
        return NULL;
    }
    for (i = 0u; i < plan->planned_count; ++i) {
        if (plan->entries[i].entry_index == entry_index) {
            return &plan->entries[i];
        }
    }
    return NULL;
}

int csb_v1_csbgraphics_m11_runtime_plan_apply_entry(
    const CSB_V1_CSBGraphicsM11RuntimePlan *plan,
    const CSB_V1_CSBGraphicsDatRealCache *cache,
    uint32_t entry_index,
    uint8_t *framebuffer,
    int framebuffer_width,
    int framebuffer_height,
    int framebuffer_stride,
    CSB_V1_CSBGraphicsM11Binding *out_binding)
{
    const CSB_V1_CSBGraphicsM11RuntimePlanEntry *entry;
    int applied;

    if (!plan || !plan->ready || !cache_ready(cache) || !framebuffer) {
        return CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_ERR_ARGUMENT;
    }
    entry = csb_v1_csbgraphics_m11_runtime_plan_find_entry(plan,
                                                           entry_index);
    if (!entry) {
        return CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_ERR_NO_SUPPORTED_ENTRIES;
    }
    if (entry->deferred_masked_composite) {
        return CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_ERR_DEFERRED_COMPOSITE;
    }
    applied = csb_v1_csbgraphics_m11_decode_entry_and_apply(
        cache->file_buffer,
        cache->file_size,
        entry->entry_index,
        entry->expected_width,
        entry->expected_height,
        framebuffer,
        framebuffer_width,
        framebuffer_height,
        framebuffer_stride,
        out_binding);
    return applied ? CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_OK
                   : CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_ERR_APPLY;
}

int csb_v1_csbgraphics_m11_runtime_plan_apply_custom_background_entry(
    const CSB_V1_CSBGraphicsM11RuntimePlan *plan,
    const CSB_V1_CSBGraphicsDatRealCache *cache,
    uint32_t entry_index,
    const CSB_V1_ViewportCustomBackgroundMask *mask_geometry,
    uint32_t *viewport_words,
    size_t viewport_word_count,
    int viewport_width_pixels)
{
    const CSB_V1_CSBGraphicsM11RuntimePlanEntry *entry;
    CSB_V1_ViewportCustomBackgroundMask mask;
    uint32_t *bitmap_words = NULL;
    uint16_t *mask_words = NULL;
    size_t bitmap_word_count = 0u;
    size_t mask_word_count = 0u;
    int copied;

    if (!plan || !plan->ready || !cache_ready(cache) ||
        !mask_geometry || !viewport_words) {
        return CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_ERR_ARGUMENT;
    }
    entry = csb_v1_csbgraphics_m11_runtime_plan_find_entry(plan,
                                                           entry_index);
    if (!entry ||
        entry->route != CSB_V1_CSBGRAPHICS_M11_ROUTE_VIEWPORT_CUSTOM_BACKGROUND ||
        entry->mask_entry_index == 0u) {
        return CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_ERR_NO_SUPPORTED_ENTRIES;
    }

    bitmap_words = decode_entry_words32(cache,
                                        entry->entry_index,
                                        entry->decompressed_size,
                                        &bitmap_word_count);
    mask_words = decode_entry_words16(cache,
                                      entry->mask_entry_index,
                                      entry->mask_decompressed_size,
                                      &mask_word_count);
    if (!bitmap_words || !mask_words) {
        free(bitmap_words);
        free(mask_words);
        return CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_ERR_APPLY;
    }

    mask = *mask_geometry;
    mask.mask_words = mask_words;
    mask.mask_word_count = mask_word_count;
    copied = csb_v1_viewport_custom_background_apply_aligned_mask_pc34(
        &mask,
        bitmap_words,
        bitmap_word_count,
        viewport_words,
        viewport_word_count,
        viewport_width_pixels);
    free(bitmap_words);
    free(mask_words);
    if (copied <= 0) {
        return copied == -2
            ? CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_ERR_DEFERRED_COMPOSITE
            : CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_ERR_APPLY;
    }
    return CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_OK;
}

int csb_v1_csbgraphics_m11_runtime_plan_apply_custom_background_room_layer(
    const CSB_V1_CSBGraphicsM11RuntimePlan *plan,
    const CSB_V1_CSBGraphicsDatRealCache *cache,
    int room_num,
    CSB_V1_CSBGraphicsM11CustomBackgroundLayer layer,
    const uint16_t *skin_def_words,
    size_t skin_def_word_count,
    const CSB_V1_ViewportCustomBackgroundMask *mask_geometry,
    uint32_t *viewport_words,
    size_t viewport_word_count,
    int viewport_width_pixels)
{
    const CSB_V1_CSBGraphicsM11RuntimePlanEntry *entry = NULL;
    uint32_t bitmap_entry_index;
    uint32_t mask_entry_index;
    int bitmap_skin_def_index = -1;
    int mask_skin_def_index = -1;

    if (!plan || !plan->ready || !cache_ready(cache) ||
        !skin_def_words || !mask_geometry || !viewport_words) {
        return CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_ERR_ARGUMENT;
    }

    if (!custom_background_layer_skin_def_indices(room_num,
                                                  layer,
                                                  &bitmap_skin_def_index,
                                                  &mask_skin_def_index)) {
        return CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_ERR_NO_SUPPORTED_ENTRIES;
    }
    if ((size_t)bitmap_skin_def_index >= skin_def_word_count ||
        (size_t)mask_skin_def_index >= skin_def_word_count) {
        return CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_ERR_NO_SUPPORTED_ENTRIES;
    }

    bitmap_entry_index = (uint32_t)skin_def_words[bitmap_skin_def_index];
    mask_entry_index = (uint32_t)skin_def_words[mask_skin_def_index];
    if (bitmap_entry_index == 0u || mask_entry_index == 0u) {
        return CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_ERR_NO_SUPPORTED_ENTRIES;
    }

    entry = find_custom_background_entry_by_pair(plan,
                                                 bitmap_entry_index,
                                                 mask_entry_index,
                                                 layer);

    if (!entry) {
        return CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_ERR_NO_SUPPORTED_ENTRIES;
    }

    /* CSB-lineage Viewport.cpp:6599-6619 chooses the pSkinDef bitmap/mask
     * pair for this room layer. The shared entry helper owns the actual
     * CSBgraphics.dat decode and ApplyBackground aligned-mask composite. */
    return csb_v1_csbgraphics_m11_runtime_plan_apply_custom_background_entry(
        plan,
        cache,
        entry->entry_index,
        mask_geometry,
        viewport_words,
        viewport_word_count,
        viewport_width_pixels);
}

int csb_v1_csbgraphics_m11_runtime_plan_apply_custom_background_room_layer_auto_mask(
    const CSB_V1_CSBGraphicsM11RuntimePlan *plan,
    const CSB_V1_CSBGraphicsDatRealCache *cache,
    int room_num,
    CSB_V1_CSBGraphicsM11CustomBackgroundLayer layer,
    const uint16_t *skin_def_words,
    size_t skin_def_word_count,
    uint32_t *viewport_words,
    size_t viewport_word_count,
    int viewport_width_pixels)
{
    const CSB_V1_CSBGraphicsM11RuntimePlanEntry *entry = NULL;
    CSB_V1_ViewportCustomBackgroundMask mask;
    uint32_t bitmap_entry_index;
    uint32_t mask_entry_index;
    uint32_t *bitmap_words = NULL;
    uint16_t *mask_words = NULL;
    size_t bitmap_word_count = 0u;
    int bitmap_skin_def_index = -1;
    int mask_skin_def_index = -1;
    int copied;
    int rc;

    if (!plan || !plan->ready || !cache_ready(cache) ||
        !skin_def_words || !viewport_words) {
        return CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_ERR_ARGUMENT;
    }
    if (!custom_background_layer_skin_def_indices(room_num,
                                                  layer,
                                                  &bitmap_skin_def_index,
                                                  &mask_skin_def_index)) {
        return CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_ERR_NO_SUPPORTED_ENTRIES;
    }
    if ((size_t)bitmap_skin_def_index >= skin_def_word_count ||
        (size_t)mask_skin_def_index >= skin_def_word_count) {
        return CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_ERR_NO_SUPPORTED_ENTRIES;
    }

    bitmap_entry_index = (uint32_t)skin_def_words[bitmap_skin_def_index];
    mask_entry_index = (uint32_t)skin_def_words[mask_skin_def_index];
    if (bitmap_entry_index == 0u || mask_entry_index == 0u) {
        return CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_ERR_NO_SUPPORTED_ENTRIES;
    }

    entry = find_custom_background_entry_by_pair(plan,
                                                 bitmap_entry_index,
                                                 mask_entry_index,
                                                 layer);
    if (!entry) {
        return CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_ERR_NO_SUPPORTED_ENTRIES;
    }

    bitmap_words = decode_entry_words32(cache,
                                        entry->entry_index,
                                        entry->decompressed_size,
                                        &bitmap_word_count);
    rc = decode_custom_background_mask_for_room(cache,
                                                entry->mask_entry_index,
                                                entry->mask_decompressed_size,
                                                room_num,
                                                &mask,
                                                &mask_words);
    if (!bitmap_words ||
        rc != CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_OK ||
        !mask_words) {
        free(bitmap_words);
        free(mask_words);
        return bitmap_words ? rc
                            : CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_ERR_APPLY;
    }

    copied = csb_v1_viewport_custom_background_apply_aligned_mask_pc34(
        &mask,
        bitmap_words,
        bitmap_word_count,
        viewport_words,
        viewport_word_count,
        viewport_width_pixels);
    free(bitmap_words);
    free(mask_words);
    if (copied <= 0) {
        return copied == -2
            ? CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_ERR_DEFERRED_COMPOSITE
            : CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_ERR_APPLY;
    }
    return CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_OK;
}

const char *csb_v1_csbgraphics_m11_runtime_plan_result_name(int result)
{
    switch (result) {
    case CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_OK:
        return "ok";
    case CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_ERR_ARGUMENT:
        return "argument";
    case CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_ERR_NO_CACHE:
        return "no_cache";
    case CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_ERR_NO_SUPPORTED_ENTRIES:
        return "no_supported_entries";
    case CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_ERR_FULL:
        return "full";
    case CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_ERR_GEOMETRY:
        return "geometry";
    case CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_ERR_APPLY:
        return "apply";
    case CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_ERR_DEFERRED_COMPOSITE:
        return "deferred_composite";
    default:
        return "unknown";
    }
}

const char *csb_v1_csbgraphics_m11_runtime_plan_source_evidence(void)
{
    return s_source_evidence;
}
