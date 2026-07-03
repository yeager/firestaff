#include "csb_v1_csbgraphics_m11_runtime_plan.h"
#include "dm1_v1_graphics_loader_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int g_assertions;
static int g_failures;

static void check_int(const char *label, int actual, int expected)
{
    ++g_assertions;
    if (actual != expected) {
        ++g_failures;
        printf("FAIL %s actual=%d expected=%d\n", label, actual, expected);
    } else {
        printf("ok %s = %d\n", label, actual);
    }
}

static void check_true(const char *label, int condition)
{
    check_int(label, condition ? 1 : 0, 1);
}

static void write_be16(uint8_t *buf, size_t off, uint16_t value)
{
    buf[off] = (uint8_t)((value >> 8) & 0xffu);
    buf[off + 1u] = (uint8_t)(value & 0xffu);
}

typedef struct {
    uint8_t *buf;
    size_t cap;
    size_t bit_pos;
} BitWriter;

static void bw_init(BitWriter *bw)
{
    bw->cap = 1024u;
    bw->buf = (uint8_t *)calloc(1u, bw->cap);
    bw->bit_pos = 0u;
}

static int bw_grow(BitWriter *bw)
{
    size_t old_cap = bw->cap;
    size_t new_cap = bw->cap * 2u;
    uint8_t *new_buf = (uint8_t *)realloc(bw->buf, new_cap);
    if (!new_buf) {
        free(bw->buf);
        bw->buf = NULL;
        bw->cap = 0u;
        return 0;
    }
    memset(new_buf + old_cap, 0, new_cap - old_cap);
    bw->buf = new_buf;
    bw->cap = new_cap;
    return 1;
}

static int bw_write_bits(BitWriter *bw, uint32_t value, int n_bits)
{
    int i;
    for (i = 0; i < n_bits; ++i) {
        size_t bp = bw->bit_pos++;
        size_t byte_idx;
        int bit_in_byte;
        if ((bp >> 3) >= bw->cap && !bw_grow(bw)) {
            return 0;
        }
        byte_idx = bp >> 3;
        bit_in_byte = (int)(bp & 7u);
        if (value & (1u << (uint32_t)i)) {
            bw->buf[byte_idx] |= (uint8_t)(1u << (uint32_t)bit_in_byte);
        }
    }
    return 1;
}

typedef struct {
    uint8_t dict_first[4096];
    uint16_t dict_prefix[4096];
    int dict_count;
    int code_bits;
} RefLZW;

static void ref_lzw_init(RefLZW *e)
{
    int i;
    e->dict_count = DM1_GFX_LZW_FIRST_CODE;
    e->code_bits = 9;
    for (i = 0; i < 256; ++i) {
        e->dict_first[i] = (uint8_t)i;
        e->dict_prefix[i] = 0xffffu;
    }
}

static int ref_lzw_find_or_add(RefLZW *e, uint16_t prefix, uint8_t append)
{
    int i;
    for (i = DM1_GFX_LZW_FIRST_CODE; i < e->dict_count; ++i) {
        if (e->dict_prefix[i] == prefix && e->dict_first[i] == append) {
            return i;
        }
    }
    if (e->dict_count >= DM1_GFX_LZW_MAX_CODE) {
        return -1;
    }
    e->dict_prefix[e->dict_count] = prefix;
    e->dict_first[e->dict_count] = append;
    ++e->dict_count;
    return -1;
}

static void ref_lzw_maybe_grow(RefLZW *e)
{
    if (e->dict_count > ((1 << e->code_bits) - 1) && e->code_bits < 12) {
        ++e->code_bits;
    }
}

static int ref_lzw_encode(const uint8_t *input, size_t in_size,
                          uint8_t **out_buf, size_t *out_size)
{
    BitWriter bw;
    RefLZW e;
    uint16_t prefix_code;
    size_t i;

    if (!input || !out_buf || !out_size) {
        return -1;
    }
    *out_buf = NULL;
    *out_size = 0u;
    bw_init(&bw);
    if (!bw.buf) {
        return -1;
    }
    ref_lzw_init(&e);
    if (!bw_write_bits(&bw, DM1_GFX_LZW_CLEAR_CODE, e.code_bits)) {
        free(bw.buf);
        return -1;
    }
    if (in_size == 0u) {
        free(bw.buf);
        return -1;
    }
    prefix_code = input[0];
    for (i = 1u; i < in_size; ++i) {
        uint8_t next_byte = input[i];
        int existing = ref_lzw_find_or_add(&e, prefix_code, next_byte);
        if (existing >= 0) {
            prefix_code = (uint16_t)existing;
        } else {
            if (!bw_write_bits(&bw, prefix_code, e.code_bits)) {
                free(bw.buf);
                return -1;
            }
            ref_lzw_maybe_grow(&e);
            prefix_code = next_byte;
        }
    }
    if (!bw_write_bits(&bw, prefix_code, e.code_bits) ||
        !bw_write_bits(&bw, DM1_GFX_LZW_END_CODE, e.code_bits)) {
        free(bw.buf);
        return -1;
    }
    *out_buf = bw.buf;
    *out_size = (bw.bit_pos + 7u) / 8u;
    return 0;
}

static uint8_t *build_csbgraphics_single_entry(
    uint32_t entry_index,
    const uint8_t *decoded,
    size_t decoded_size,
    size_t *out_size)
{
    uint8_t *compressed = NULL;
    size_t compressed_size = 0u;
    uint32_t count = entry_index + 1u;
    size_t header_size = 2u + (size_t)count * 4u;
    uint8_t *buf;

    if (!out_size || !decoded || decoded_size == 0u ||
        decoded_size > 65535u ||
        ref_lzw_encode(decoded, decoded_size,
                       &compressed, &compressed_size) != 0 ||
        !compressed || compressed_size == 0u || compressed_size > 65535u) {
        free(compressed);
        return NULL;
    }
    buf = (uint8_t *)calloc(1u, header_size + compressed_size);
    if (!buf) {
        free(compressed);
        return NULL;
    }
    write_be16(buf, 0u, (uint16_t)count);
    write_be16(buf, 2u + (size_t)entry_index * 2u,
               (uint16_t)compressed_size);
    write_be16(buf, 2u + (size_t)count * 2u + (size_t)entry_index * 2u,
               (uint16_t)decoded_size);
    memcpy(buf + header_size, compressed, compressed_size);
    *out_size = header_size + compressed_size;
    free(compressed);
    return buf;
}

static void cache_from_bytes(CSB_V1_CSBGraphicsDatRealCache *cache,
                             uint8_t *bytes,
                             size_t size)
{
    csb_v1_csbgraphics_dat_real_cache_init(cache);
    cache->file_buffer = bytes;
    cache->file_size = size;
    cache->loaded = 1;
    snprintf(cache->resolved_path, sizeof(cache->resolved_path),
             "/synthetic/CSBgraphics.dat");
    snprintf(cache->matched_md5, sizeof(cache->matched_md5),
             "00000000000000000000000000000000");
    snprintf(cache->matched_label, sizeof(cache->matched_label),
             "synthetic");
    check_int("cache.classify",
              csb_v1_csbgraphics_dat_classify(bytes, size, &cache->index),
              CSB_V1_CSBGRAPHICS_CLASSIFY_OK);
}

static uint8_t *build_header_only_entry(uint32_t entry_index,
                                        uint16_t comp_size,
                                        uint16_t deco_size,
                                        size_t *out_size)
{
    uint32_t count = entry_index + 1u;
    size_t header_size = 2u + (size_t)count * 4u;
    uint8_t *buf;
    if (!out_size || comp_size == 0u) {
        return NULL;
    }
    buf = (uint8_t *)calloc(1u, header_size + comp_size);
    if (!buf) {
        return NULL;
    }
    write_be16(buf, 0u, (uint16_t)count);
    write_be16(buf, 2u + (size_t)entry_index * 2u, comp_size);
    write_be16(buf, 2u + (size_t)count * 2u + (size_t)entry_index * 2u,
               deco_size);
    *out_size = header_size + comp_size;
    return buf;
}

static uint8_t *build_header_only_entries(const uint16_t *entry_ids,
                                          size_t entry_count,
                                          uint16_t comp_size,
                                          uint16_t deco_size,
                                          size_t *out_size)
{
    uint32_t max_entry = 0u;
    uint32_t count;
    size_t header_size;
    size_t payload_cursor;
    size_t i;
    uint8_t *buf;

    if (!entry_ids || entry_count == 0u || !out_size || comp_size == 0u) {
        return NULL;
    }
    for (i = 0u; i < entry_count; ++i) {
        if (entry_ids[i] > max_entry) {
            max_entry = entry_ids[i];
        }
    }
    count = max_entry + 1u;
    header_size = 2u + (size_t)count * 4u;
    buf = (uint8_t *)calloc(1u,
                            header_size + (size_t)comp_size * entry_count);
    if (!buf) {
        return NULL;
    }
    write_be16(buf, 0u, (uint16_t)count);
    payload_cursor = header_size;
    for (i = 0u; i < entry_count; ++i) {
        uint16_t id = entry_ids[i];
        write_be16(buf, 2u + (size_t)id * 2u, comp_size);
        write_be16(buf, 2u + (size_t)count * 2u + (size_t)id * 2u,
                   deco_size);
        buf[payload_cursor] = (uint8_t)(0x80u + i);
        payload_cursor += comp_size;
    }
    *out_size = header_size + (size_t)comp_size * entry_count;
    return buf;
}

static void test_build_known_c040_plan(void)
{
    CSB_V1_CSBGraphicsDatRealCache cache;
    CSB_V1_CSBGraphicsM11RuntimePlan plan;
    uint8_t *bytes;
    size_t size = 0u;
    const CSB_V1_CSBGraphicsM11RuntimePlanEntry *entry;

    bytes = build_header_only_entry(
        40u, 1u,
        (uint16_t)(CSB_V1_CSBGRAPHICS_M11_C040_PANEL_W *
                   CSB_V1_CSBGRAPHICS_M11_C040_PANEL_H),
        &size);
    check_true("known_c040.fixture", bytes != NULL);
    if (!bytes) {
        return;
    }
    cache_from_bytes(&cache, bytes, size);
    check_int("known_c040.build",
              csb_v1_csbgraphics_m11_runtime_plan_build_from_cache(
                  &cache, &plan),
              CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_OK);
    check_int("known_c040.ready", plan.ready, 1);
    check_int("known_c040.planned_count", (int)plan.planned_count, 1);
    check_int("known_c040.supported_present",
              (int)plan.supported_present_count, 1);
    entry = csb_v1_csbgraphics_m11_runtime_plan_find_entry(&plan, 40u);
    check_true("known_c040.find", entry != NULL);
    if (entry) {
        check_int("known_c040.route", entry->route,
                  CSB_V1_CSBGRAPHICS_M11_ROUTE_HUD_RESURRECT_PANEL);
        check_int("known_c040.width", entry->expected_width,
                  CSB_V1_CSBGRAPHICS_M11_C040_PANEL_W);
        check_int("known_c040.height", entry->expected_height,
                  CSB_V1_CSBGRAPHICS_M11_C040_PANEL_H);
        check_int("known_c040.hud", entry->needs_hud_redraw, 1);
    }
    cache.file_buffer = NULL;
    csb_v1_csbgraphics_dat_real_cache_free(&cache);
    free(bytes);
}

static void test_explicit_geometry_apply(void)
{
    enum { W = 8, H = 8 };
    uint8_t pixels[W * H];
    uint8_t framebuffer[CSB_V1_CSBGRAPHICS_M11_SOURCE_W *
                        CSB_V1_CSBGRAPHICS_M11_SOURCE_H];
    uint8_t *bytes = NULL;
    size_t size = 0u;
    CSB_V1_CSBGraphicsDatRealCache cache;
    CSB_V1_CSBGraphicsM11RuntimePlan plan;
    CSB_V1_CSBGraphicsM11Binding binding;
    int i;

    for (i = 0; i < W * H; ++i) {
        pixels[i] = (uint8_t)((i % 15) + 1);
    }
    bytes = build_csbgraphics_single_entry(73u, pixels, sizeof(pixels),
                                           &size);
    check_true("explicit_apply.fixture", bytes != NULL);
    if (!bytes) {
        return;
    }
    cache_from_bytes(&cache, bytes, size);
    csb_v1_csbgraphics_m11_runtime_plan_init(&plan);
    check_int("explicit_apply.add",
              csb_v1_csbgraphics_m11_runtime_plan_add_explicit_entry(
                  &cache, 73u, (uint16_t)W, (uint16_t)H, &plan),
              CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_OK);
    check_int("explicit_apply.ready", plan.ready, 1);

    memset(framebuffer, 0, sizeof(framebuffer));
    memset(&binding, 0, sizeof(binding));
    check_int("explicit_apply.apply",
              csb_v1_csbgraphics_m11_runtime_plan_apply_entry(
                  &plan, &cache, 73u, framebuffer,
                  CSB_V1_CSBGRAPHICS_M11_SOURCE_W,
                  CSB_V1_CSBGRAPHICS_M11_SOURCE_H,
                  CSB_V1_CSBGRAPHICS_M11_SOURCE_W,
                  &binding),
              CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_OK);
    check_int("explicit_apply.route", binding.route,
              CSB_V1_CSBGRAPHICS_M11_ROUTE_VIEWPORT_DERIVED);
    check_int("explicit_apply.pixel",
              framebuffer[CSB_V1_CSBGRAPHICS_M11_VIEWPORT_Y *
                          CSB_V1_CSBGRAPHICS_M11_SOURCE_W +
                          CSB_V1_CSBGRAPHICS_M11_VIEWPORT_X],
              pixels[0]);
    check_int("explicit_apply.unsupported",
              csb_v1_csbgraphics_m11_runtime_plan_apply_entry(
                  &plan, &cache, 74u, framebuffer,
                  CSB_V1_CSBGRAPHICS_M11_SOURCE_W,
                  CSB_V1_CSBGRAPHICS_M11_SOURCE_H,
                  CSB_V1_CSBGRAPHICS_M11_SOURCE_W,
                  &binding),
              CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_ERR_NO_SUPPORTED_ENTRIES);

    cache.file_buffer = NULL;
    csb_v1_csbgraphics_dat_real_cache_free(&cache);
    free(bytes);
}

static void test_unknown_geometry_is_honest(void)
{
    CSB_V1_CSBGraphicsDatRealCache cache;
    CSB_V1_CSBGraphicsM11RuntimePlan plan;
    uint8_t *bytes;
    size_t size = 0u;

    bytes = build_header_only_entry(73u, 1u, 64u, &size);
    check_true("unknown_geometry.fixture", bytes != NULL);
    if (!bytes) {
        return;
    }
    cache_from_bytes(&cache, bytes, size);
    check_int("unknown_geometry.build",
              csb_v1_csbgraphics_m11_runtime_plan_build_from_cache(
                  &cache, &plan),
              CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_ERR_NO_SUPPORTED_ENTRIES);
    check_int("unknown_geometry.supported_present",
              (int)plan.supported_present_count, 1);
    check_int("unknown_geometry.skipped",
              (int)plan.skipped_unknown_geometry_count, 1);
    check_int("unknown_geometry.ready", plan.ready, 0);

    cache.file_buffer = NULL;
    csb_v1_csbgraphics_dat_real_cache_free(&cache);
    free(bytes);
}

static void test_custom_background_skin_def_pairs_are_deferred(void)
{
    static const uint16_t entry_ids[] = {
        100u, 101u, 102u, 104u, 105u, 106u
    };
    static const uint16_t skin_def_words[] = {
        100u, 101u, 102u, 0u, 104u, 105u, 106u
    };
    CSB_V1_CSBGraphicsDatRealCache cache;
    CSB_V1_CSBGraphicsM11RuntimePlan plan;
    CSB_V1_CSBGraphicsM11Binding binding;
    const CSB_V1_CSBGraphicsM11RuntimePlanEntry *large;
    const CSB_V1_CSBGraphicsM11RuntimePlanEntry *middle;
    const CSB_V1_CSBGraphicsM11RuntimePlanEntry *near;
    uint8_t framebuffer[CSB_V1_CSBGRAPHICS_M11_SOURCE_W *
                        CSB_V1_CSBGRAPHICS_M11_SOURCE_H];
    uint8_t *bytes;
    size_t size = 0u;

    bytes = build_header_only_entries(entry_ids,
                                      sizeof(entry_ids) / sizeof(entry_ids[0]),
                                      1u, 64u, &size);
    check_true("custom_bg.fixture", bytes != NULL);
    if (!bytes) {
        return;
    }
    cache_from_bytes(&cache, bytes, size);
    csb_v1_csbgraphics_m11_runtime_plan_init(&plan);
    check_int("custom_bg.add_skin_def",
              csb_v1_csbgraphics_m11_runtime_plan_add_custom_background_skin_def(
                  &cache,
                  skin_def_words,
                  sizeof(skin_def_words) / sizeof(skin_def_words[0]),
                  &plan),
              CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_OK);
    check_int("custom_bg.ready", plan.ready, 1);
    check_int("custom_bg.planned_count", (int)plan.planned_count, 3);
    check_int("custom_bg.pair_count",
              (int)plan.custom_background_pair_count, 3);
    check_int("custom_bg.supported_present",
              (int)plan.supported_present_count, 3);

    large = csb_v1_csbgraphics_m11_runtime_plan_find_entry(&plan, 100u);
    middle = csb_v1_csbgraphics_m11_runtime_plan_find_entry(&plan, 102u);
    near = csb_v1_csbgraphics_m11_runtime_plan_find_entry(&plan, 101u);
    check_true("custom_bg.large.find", large != NULL);
    check_true("custom_bg.middle.find", middle != NULL);
    check_true("custom_bg.near.find", near != NULL);
    if (large) {
        check_int("custom_bg.large.mask", (int)large->mask_entry_index, 104);
        check_int("custom_bg.large.route", large->route,
                  CSB_V1_CSBGRAPHICS_M11_ROUTE_VIEWPORT_CUSTOM_BACKGROUND);
        check_int("custom_bg.large.deferred",
                  large->deferred_masked_composite, 1);
        check_int("custom_bg.large.layer",
                  large->custom_background_layer,
                  CSB_V1_CSBGRAPHICS_M11_CUSTOM_BACKGROUND_LAYER_LARGE);
    }
    if (middle) {
        check_int("custom_bg.middle.mask", (int)middle->mask_entry_index, 106);
        check_int("custom_bg.middle.layer",
                  middle->custom_background_layer,
                  CSB_V1_CSBGRAPHICS_M11_CUSTOM_BACKGROUND_LAYER_MIDDLE);
    }
    if (near) {
        check_int("custom_bg.near.mask", (int)near->mask_entry_index, 105);
        check_int("custom_bg.near.layer",
                  near->custom_background_layer,
                  CSB_V1_CSBGRAPHICS_M11_CUSTOM_BACKGROUND_LAYER_NEAR);
    }

    memset(framebuffer, 0, sizeof(framebuffer));
    memset(&binding, 0, sizeof(binding));
    check_int("custom_bg.apply_deferred",
              csb_v1_csbgraphics_m11_runtime_plan_apply_entry(
                  &plan, &cache, 100u, framebuffer,
                  CSB_V1_CSBGRAPHICS_M11_SOURCE_W,
                  CSB_V1_CSBGRAPHICS_M11_SOURCE_H,
                  CSB_V1_CSBGRAPHICS_M11_SOURCE_W,
                  &binding),
              CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_ERR_DEFERRED_COMPOSITE);
    check_true("custom_bg.route_name",
               strstr(csb_v1_csbgraphics_m11_route_name(
                          CSB_V1_CSBGRAPHICS_M11_ROUTE_VIEWPORT_CUSTOM_BACKGROUND),
                      "custom-background") != NULL);

    cache.file_buffer = NULL;
    csb_v1_csbgraphics_dat_real_cache_free(&cache);
    free(bytes);
}

int main(void)
{
    test_build_known_c040_plan();
    test_explicit_geometry_apply();
    test_unknown_geometry_is_honest();
    test_custom_background_skin_def_pairs_are_deferred();
    check_true("result_name",
               strcmp(csb_v1_csbgraphics_m11_runtime_plan_result_name(
                          CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_ERR_GEOMETRY),
                      "geometry") == 0);
    check_true("source_evidence",
               strstr(csb_v1_csbgraphics_m11_runtime_plan_source_evidence(),
                      "CSBWin/Graphics.cpp") != NULL);

    if (g_failures) {
        printf("CSBgraphics M11 runtime plan FAILED: %d/%d failed\n",
               g_failures, g_assertions);
        return 1;
    }
    printf("CSBgraphics M11 runtime plan passed: %d assertions\n",
           g_assertions);
    return 0;
}
