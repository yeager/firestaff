#include "csb_v1_csbgraphics_m11_runtime_plan.h"
#include "csb_v1_viewport_custom_backgrounds_room_slot_pc34_compat.h"

#include <stdio.h>
#include <string.h>

enum {
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
    if (bitmap_span->decompressed_size != mask_span->decompressed_size) {
        return CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_ERR_GEOMETRY;
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
