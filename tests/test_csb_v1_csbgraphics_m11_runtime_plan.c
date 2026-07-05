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

typedef struct {
    uint16_t entry_index;
    const uint8_t *decoded;
    size_t decoded_size;
} CompressedEntryFixture;

static uint8_t *build_csbgraphics_entries_compressed(
    const CompressedEntryFixture *entries,
    size_t entry_count,
    size_t *out_size)
{
    uint16_t max_entry = 0u;
    uint32_t count;
    size_t header_size;
    size_t payload_size = 0u;
    size_t payload_cursor;
    size_t i;
    uint8_t **compressed = NULL;
    size_t *compressed_sizes = NULL;
    uint8_t *buf = NULL;

    if (!entries || entry_count == 0u || !out_size) {
        return NULL;
    }
    for (i = 0u; i < entry_count; ++i) {
        if (entries[i].entry_index > max_entry) {
            max_entry = entries[i].entry_index;
        }
    }
    compressed = (uint8_t **)calloc(entry_count, sizeof(compressed[0]));
    compressed_sizes = (size_t *)calloc(entry_count, sizeof(compressed_sizes[0]));
    if (!compressed || !compressed_sizes) {
        free(compressed);
        free(compressed_sizes);
        return NULL;
    }
    for (i = 0u; i < entry_count; ++i) {
        if (!entries[i].decoded || entries[i].decoded_size == 0u ||
            entries[i].decoded_size > 65535u ||
            ref_lzw_encode(entries[i].decoded,
                           entries[i].decoded_size,
                           &compressed[i],
                           &compressed_sizes[i]) != 0 ||
            !compressed[i] ||
            compressed_sizes[i] == 0u ||
            compressed_sizes[i] > 65535u) {
            goto cleanup;
        }
        payload_size += compressed_sizes[i];
    }

    count = (uint32_t)max_entry + 1u;
    header_size = 2u + (size_t)count * 4u;
    buf = (uint8_t *)calloc(1u, header_size + payload_size);
    if (!buf) {
        goto cleanup;
    }
    write_be16(buf, 0u, (uint16_t)count);
    for (i = 0u; i < entry_count; ++i) {
        uint16_t id = entries[i].entry_index;
        write_be16(buf, 2u + (size_t)id * 2u,
                   (uint16_t)compressed_sizes[i]);
        write_be16(buf, 2u + (size_t)count * 2u + (size_t)id * 2u,
                   (uint16_t)entries[i].decoded_size);
    }
    payload_cursor = header_size;
    for (uint32_t id = 0u; id < count; ++id) {
        for (i = 0u; i < entry_count; ++i) {
            if (entries[i].entry_index == id) {
                memcpy(buf + payload_cursor, compressed[i], compressed_sizes[i]);
                payload_cursor += compressed_sizes[i];
            }
        }
    }
    *out_size = header_size + payload_size;

cleanup:
    if (compressed) {
        for (i = 0u; i < entry_count; ++i) {
            free(compressed[i]);
        }
    }
    free(compressed);
    free(compressed_sizes);
    return buf;
}

static void write_le16(uint8_t *buf, size_t off, uint16_t value)
{
    buf[off] = (uint8_t)(value & 0xffu);
    buf[off + 1u] = (uint8_t)((value >> 8) & 0xffu);
}

static void write_le32(uint8_t *buf, size_t off, uint32_t value)
{
    buf[off] = (uint8_t)(value & 0xffu);
    buf[off + 1u] = (uint8_t)((value >> 8) & 0xffu);
    buf[off + 2u] = (uint8_t)((value >> 16) & 0xffu);
    buf[off + 3u] = (uint8_t)((value >> 24) & 0xffu);
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

static void test_custom_background_skin_def_decode(void)
{
    uint8_t skin_def_decoded[42u];
    static const uint16_t expected_words[] = {
        100u, 108u, 102u, 0u, 104u, 110u, 106u, 0u, 0u
    };
    static const uint16_t expected_words_skin1[] = {
        120u, 128u, 122u, 0u, 124u, 130u, 126u, 0u, 0u
    };
    CompressedEntryFixture entries[1];
    CSB_V1_CSBGraphicsDatRealCache cache;
    uint16_t words[CSB_V1_CSBGRAPHICS_M11_SKIN_DEF_MAX_WORDS];
    size_t word_count = 0u;
    uint8_t *bytes;
    size_t size = 0u;
    size_t i;

    memset(skin_def_decoded, 0, sizeof(skin_def_decoded));
    write_le16(skin_def_decoded, 0u, 2u);
    write_le16(skin_def_decoded, 2u, 6u);
    write_le16(skin_def_decoded, 4u, 24u);
    for (i = 0u; i < sizeof(expected_words) / sizeof(expected_words[0]); ++i) {
        write_le16(skin_def_decoded, 6u + i * 2u, expected_words[i]);
        write_le16(skin_def_decoded, 24u + i * 2u, expected_words_skin1[i]);
    }
    entries[0].entry_index = 1u;
    entries[0].decoded = skin_def_decoded;
    entries[0].decoded_size = sizeof(skin_def_decoded);

    bytes = build_csbgraphics_entries_compressed(entries,
                                                 sizeof(entries) / sizeof(entries[0]),
                                                 &size);
    check_true("custom_bg_skin_def_decode.fixture", bytes != NULL);
    if (!bytes) {
        return;
    }

    cache_from_bytes(&cache, bytes, size);
    memset(words, 0, sizeof(words));
    check_int("custom_bg_skin_def_decode.rc",
              csb_v1_csbgraphics_m11_runtime_plan_decode_custom_background_skin_def(
                  &cache,
                  words,
                  sizeof(words) / sizeof(words[0]),
                  &word_count),
              CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_OK);
    check_int("custom_bg_skin_def_decode.count", (int)word_count,
              (int)(sizeof(expected_words) / sizeof(expected_words[0])));
    check_int("custom_bg_skin_def_decode.word0", (int)words[0], 100);
    check_int("custom_bg_skin_def_decode.near_bitmap", (int)words[1], 108);
    check_int("custom_bg_skin_def_decode.middle_mask", (int)words[6], 106);
    memset(words, 0, sizeof(words));
    word_count = 0u;
    check_int("custom_bg_skin_def_decode.skin1_rc",
              csb_v1_csbgraphics_m11_runtime_plan_decode_custom_background_skin_def_for_skin(
                  &cache,
                  1u,
                  words,
                  sizeof(words) / sizeof(words[0]),
                  &word_count),
              CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_OK);
    check_int("custom_bg_skin_def_decode.skin1_large_bitmap",
              (int)words[0], 120);
    check_int("custom_bg_skin_def_decode.skin1_middle_mask",
              (int)words[6], 126);

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

static void test_custom_background_aligned_mask_apply(void)
{
    uint8_t bitmap_decoded[9u * 4u];
    uint8_t mask_decoded[2u];
    static const uint16_t skin_def_words[] = {
        100u, 0u, 0u, 0u, 104u, 0u, 0u
    };
    CompressedEntryFixture entries[2];
    CSB_V1_CSBGraphicsDatRealCache cache;
    CSB_V1_CSBGraphicsM11RuntimePlan plan;
    CSB_V1_ViewportCustomBackgroundMask mask;
    uint32_t viewport[56];
    uint8_t *bytes;
    size_t size = 0u;
    size_t i;

    memset(bitmap_decoded, 0, sizeof(bitmap_decoded));
    write_le32(bitmap_decoded, 0u, 32u);
    write_le32(bitmap_decoded, 7u * 4u, 0x12345678u);
    write_le32(bitmap_decoded, 8u * 4u, 0x87654321u);
    write_le16(mask_decoded, 0u, 0x00ffu);

    entries[0].entry_index = 100u;
    entries[0].decoded = bitmap_decoded;
    entries[0].decoded_size = sizeof(bitmap_decoded);
    entries[1].entry_index = 104u;
    entries[1].decoded = mask_decoded;
    entries[1].decoded_size = sizeof(mask_decoded);

    bytes = build_csbgraphics_entries_compressed(entries,
                                                 sizeof(entries) / sizeof(entries[0]),
                                                 &size);
    check_true("custom_bg_apply.fixture", bytes != NULL);
    if (!bytes) {
        return;
    }
    cache_from_bytes(&cache, bytes, size);
    csb_v1_csbgraphics_m11_runtime_plan_init(&plan);
    check_int("custom_bg_apply.add_skin_def",
              csb_v1_csbgraphics_m11_runtime_plan_add_custom_background_skin_def(
                  &cache,
                  skin_def_words,
                  sizeof(skin_def_words) / sizeof(skin_def_words[0]),
                  &plan),
              CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_OK);
    check_int("custom_bg_apply.planned_count", (int)plan.planned_count, 1);
    check_int("custom_bg_apply.mask_size",
              (int)plan.entries[0].mask_decompressed_size,
              (int)sizeof(mask_decoded));

    for (i = 0u; i < sizeof(viewport) / sizeof(viewport[0]); ++i) {
        viewport[i] = 0xccccccccu;
    }
    viewport[30] = 0xaaaaaaaau;
    viewport[31] = 0xbbbbbbbbu;
    memset(&mask, 0, sizeof(mask));
    mask.src_x = 16;
    mask.src_y = 1;
    mask.dst_x = 16;
    mask.dst_y = 1;
    mask.width = 16;
    mask.height = 1;

    check_int("custom_bg_apply.apply",
              csb_v1_csbgraphics_m11_runtime_plan_apply_custom_background_entry(
                  &plan,
                  &cache,
                  100u,
                  &mask,
                  viewport,
                  sizeof(viewport) / sizeof(viewport[0]),
                  224),
              CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_OK);
    check_int("custom_bg_apply.word0", (int)viewport[30], (int)0xaa34aa78u);
    check_int("custom_bg_apply.word1", (int)viewport[31], (int)0xbb65bb21u);

    mask.src_x = 16;
    mask.dst_x = 24;
    for (i = 0u; i < sizeof(viewport) / sizeof(viewport[0]); ++i) {
        viewport[i] = 0xccccccccu;
    }
    viewport[30] = 0xaaaaaaaau;
    viewport[31] = 0xbbbbbbbbu;
    check_int("custom_bg_apply.unaligned_apply",
              csb_v1_csbgraphics_m11_runtime_plan_apply_custom_background_entry(
                  &plan,
                  &cache,
                  100u,
                  &mask,
                  viewport,
                  sizeof(viewport) / sizeof(viewport[0]),
                  224),
              CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_OK);
    check_int("custom_bg_apply.unaligned_word0",
              (int)viewport[30], (int)0x34aa78aau);
    check_int("custom_bg_apply.unaligned_word1",
              (int)viewport[31], (int)0x65bb21bbu);

    mask.src_x = 8;
    mask.dst_x = 16;
    check_int("custom_bg_apply.unaligned_csbwin_not_implemented",
              csb_v1_csbgraphics_m11_runtime_plan_apply_custom_background_entry(
                  &plan,
                  &cache,
                  100u,
                  &mask,
                  viewport,
                  sizeof(viewport) / sizeof(viewport[0]),
                  224),
              CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_ERR_DEFERRED_COMPOSITE);

    cache.file_buffer = NULL;
    csb_v1_csbgraphics_dat_real_cache_free(&cache);
    free(bytes);
}

static void test_custom_background_room_layer_apply(void)
{
    uint8_t bitmap_decoded[9u * 4u];
    uint8_t mask_decoded[2u];
    static const uint16_t skin_def_words[] = {
        100u, 108u, 102u, 0u, 104u, 110u, 106u
    };
    CompressedEntryFixture entries[6];
    CSB_V1_CSBGraphicsDatRealCache cache;
    CSB_V1_CSBGraphicsM11RuntimePlan plan;
    CSB_V1_ViewportCustomBackgroundMask mask;
    uint32_t viewport[56];
    uint8_t *bytes;
    size_t size = 0u;
    size_t i;

    memset(bitmap_decoded, 0, sizeof(bitmap_decoded));
    write_le32(bitmap_decoded, 0u, 32u);
    write_le32(bitmap_decoded, 7u * 4u, 0x12345678u);
    write_le32(bitmap_decoded, 8u * 4u, 0x87654321u);
    write_le16(mask_decoded, 0u, 0x00ffu);

    entries[0].entry_index = 100u;
    entries[0].decoded = bitmap_decoded;
    entries[0].decoded_size = sizeof(bitmap_decoded);
    entries[1].entry_index = 102u;
    entries[1].decoded = bitmap_decoded;
    entries[1].decoded_size = sizeof(bitmap_decoded);
    entries[2].entry_index = 104u;
    entries[2].decoded = mask_decoded;
    entries[2].decoded_size = sizeof(mask_decoded);
    entries[3].entry_index = 106u;
    entries[3].decoded = mask_decoded;
    entries[3].decoded_size = sizeof(mask_decoded);
    entries[4].entry_index = 108u;
    entries[4].decoded = bitmap_decoded;
    entries[4].decoded_size = sizeof(bitmap_decoded);
    entries[5].entry_index = 110u;
    entries[5].decoded = mask_decoded;
    entries[5].decoded_size = sizeof(mask_decoded);

    bytes = build_csbgraphics_entries_compressed(entries,
                                                 sizeof(entries) / sizeof(entries[0]),
                                                 &size);
    check_true("custom_bg_room_layer.fixture", bytes != NULL);
    if (!bytes) {
        return;
    }

    cache_from_bytes(&cache, bytes, size);
    csb_v1_csbgraphics_m11_runtime_plan_init(&plan);
    check_int("custom_bg_room_layer.add_skin_def",
              csb_v1_csbgraphics_m11_runtime_plan_add_custom_background_skin_def(
                  &cache,
                  skin_def_words,
                  sizeof(skin_def_words) / sizeof(skin_def_words[0]),
                  &plan),
              CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_OK);
    check_int("custom_bg_room_layer.pairs",
              (int)plan.custom_background_pair_count, 3);

    for (i = 0u; i < sizeof(viewport) / sizeof(viewport[0]); ++i) {
        viewport[i] = 0xccccccccu;
    }
    viewport[30] = 0xaaaaaaaau;
    viewport[31] = 0xbbbbbbbbu;

    memset(&mask, 0, sizeof(mask));
    mask.src_x = 16;
    mask.src_y = 1;
    mask.dst_x = 16;
    mask.dst_y = 1;
    mask.width = 16;
    mask.height = 1;

    /* CSB-lineage Viewport.cpp:6599-6619 applies large/middle/near
     * pSkinDef pairs, but the near pair is gated by roomNum < 5. */
    check_int("custom_bg_room_layer.room4_near_apply",
              csb_v1_csbgraphics_m11_runtime_plan_apply_custom_background_room_layer(
                  &plan,
                  &cache,
                  4,
                  CSB_V1_CSBGRAPHICS_M11_CUSTOM_BACKGROUND_LAYER_NEAR,
                  skin_def_words,
                  sizeof(skin_def_words) / sizeof(skin_def_words[0]),
                  &mask,
                  viewport,
                  sizeof(viewport) / sizeof(viewport[0]),
                  224),
              CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_OK);
    check_int("custom_bg_room_layer.room4_word0",
              (int)viewport[30], (int)0xaa34aa78u);
    check_int("custom_bg_room_layer.room4_word1",
              (int)viewport[31], (int)0xbb65bb21u);

    check_int("custom_bg_room_layer.room5_near_rejected",
              csb_v1_csbgraphics_m11_runtime_plan_apply_custom_background_room_layer(
                  &plan,
                  &cache,
                  5,
                  CSB_V1_CSBGRAPHICS_M11_CUSTOM_BACKGROUND_LAYER_NEAR,
                  skin_def_words,
                  sizeof(skin_def_words) / sizeof(skin_def_words[0]),
                  &mask,
                  viewport,
                  sizeof(viewport) / sizeof(viewport[0]),
                  224),
              CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_ERR_NO_SUPPORTED_ENTRIES);

    check_int("custom_bg_room_layer.room5_middle_apply",
              csb_v1_csbgraphics_m11_runtime_plan_apply_custom_background_room_layer(
                  &plan,
                  &cache,
                  5,
                  CSB_V1_CSBGRAPHICS_M11_CUSTOM_BACKGROUND_LAYER_MIDDLE,
                  skin_def_words,
                  sizeof(skin_def_words) / sizeof(skin_def_words[0]),
                  &mask,
                  viewport,
                  sizeof(viewport) / sizeof(viewport[0]),
                  224),
              CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_OK);

    cache.file_buffer = NULL;
    csb_v1_csbgraphics_dat_real_cache_free(&cache);
    free(bytes);
}

static void write_background_mask_fixture(uint8_t *buf,
                                          size_t size,
                                          uint32_t room_num,
                                          uint16_t mask_word)
{
    size_t mask_offset;

    memset(buf, 0, size);
    write_le32(buf, 0u, 16u);
    mask_offset = 4u + 16u * 4u;
    write_le32(buf, 4u + (size_t)room_num * 4u, (uint32_t)mask_offset);
    write_le16(buf, mask_offset + 0u, 16u);
    write_le16(buf, mask_offset + 2u, 1u);
    write_le16(buf, mask_offset + 4u, 16u);
    write_le16(buf, mask_offset + 6u, 1u);
    write_le16(buf, mask_offset + 8u, 16u);
    write_le16(buf, mask_offset + 10u, 1u);
    write_le16(buf, mask_offset + 12u, mask_word);
}

static void write_background_mask_dual_room_fixture(uint8_t *buf,
                                                    size_t size,
                                                    uint32_t room_a,
                                                    uint32_t room_b,
                                                    uint16_t mask_word)
{
    size_t mask_a_offset;
    size_t mask_b_offset;

    memset(buf, 0, size);
    write_le32(buf, 0u, 16u);
    mask_a_offset = 4u + 16u * 4u;
    mask_b_offset = mask_a_offset + 12u + 2u;
    write_le32(buf, 4u + (size_t)room_a * 4u, (uint32_t)mask_a_offset);
    write_le32(buf, 4u + (size_t)room_b * 4u, (uint32_t)mask_b_offset);

    write_le16(buf, mask_a_offset + 0u, 16u);
    write_le16(buf, mask_a_offset + 2u, 1u);
    write_le16(buf, mask_a_offset + 4u, 16u);
    write_le16(buf, mask_a_offset + 6u, 1u);
    write_le16(buf, mask_a_offset + 8u, 16u);
    write_le16(buf, mask_a_offset + 10u, 1u);
    write_le16(buf, mask_a_offset + 12u, mask_word);

    write_le16(buf, mask_b_offset + 0u, 16u);
    write_le16(buf, mask_b_offset + 2u, 2u);
    write_le16(buf, mask_b_offset + 4u, 16u);
    write_le16(buf, mask_b_offset + 6u, 2u);
    write_le16(buf, mask_b_offset + 8u, 16u);
    write_le16(buf, mask_b_offset + 10u, 1u);
    write_le16(buf, mask_b_offset + 12u, mask_word);
}

static void test_custom_background_room_layer_auto_mask_apply(void)
{
    uint8_t bitmap_decoded[9u * 4u];
    uint8_t mask_decoded[4u + 16u * 4u + 12u + 2u];
    static const uint16_t skin_def_words[] = {
        100u, 0u, 0u, 0u, 104u, 0u, 0u
    };
    CompressedEntryFixture entries[2];
    CSB_V1_CSBGraphicsDatRealCache cache;
    CSB_V1_CSBGraphicsM11RuntimePlan plan;
    uint32_t viewport[56];
    uint8_t *bytes;
    size_t size = 0u;
    size_t i;

    memset(bitmap_decoded, 0, sizeof(bitmap_decoded));
    write_le32(bitmap_decoded, 0u, 32u);
    write_le32(bitmap_decoded, 7u * 4u, 0x12345678u);
    write_le32(bitmap_decoded, 8u * 4u, 0x87654321u);
    write_background_mask_fixture(mask_decoded, sizeof(mask_decoded), 4u, 0x00ffu);

    entries[0].entry_index = 100u;
    entries[0].decoded = bitmap_decoded;
    entries[0].decoded_size = sizeof(bitmap_decoded);
    entries[1].entry_index = 104u;
    entries[1].decoded = mask_decoded;
    entries[1].decoded_size = sizeof(mask_decoded);

    bytes = build_csbgraphics_entries_compressed(entries,
                                                 sizeof(entries) / sizeof(entries[0]),
                                                 &size);
    check_true("custom_bg_auto_mask.fixture", bytes != NULL);
    if (!bytes) {
        return;
    }
    cache_from_bytes(&cache, bytes, size);
    csb_v1_csbgraphics_m11_runtime_plan_init(&plan);
    check_int("custom_bg_auto_mask.add_skin_def",
              csb_v1_csbgraphics_m11_runtime_plan_add_custom_background_skin_def(
                  &cache,
                  skin_def_words,
                  sizeof(skin_def_words) / sizeof(skin_def_words[0]),
                  &plan),
              CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_OK);
    check_int("custom_bg_auto_mask.mask_size",
              (int)plan.entries[0].mask_decompressed_size,
              (int)sizeof(mask_decoded));

    for (i = 0u; i < sizeof(viewport) / sizeof(viewport[0]); ++i) {
        viewport[i] = 0xccccccccu;
    }
    viewport[30] = 0xaaaaaaaau;
    viewport[31] = 0xbbbbbbbbu;

    check_int("custom_bg_auto_mask.apply",
              csb_v1_csbgraphics_m11_runtime_plan_apply_custom_background_room_layer_auto_mask(
                  &plan,
                  &cache,
                  4,
                  CSB_V1_CSBGRAPHICS_M11_CUSTOM_BACKGROUND_LAYER_LARGE,
                  skin_def_words,
                  sizeof(skin_def_words) / sizeof(skin_def_words[0]),
                  viewport,
                  sizeof(viewport) / sizeof(viewport[0]),
                  224),
              CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_OK);
    check_int("custom_bg_auto_mask.word0", (int)viewport[30], (int)0xaa34aa78u);
    check_int("custom_bg_auto_mask.word1", (int)viewport[31], (int)0xbb65bb21u);

    check_int("custom_bg_auto_mask.missing_room",
              csb_v1_csbgraphics_m11_runtime_plan_apply_custom_background_room_layer_auto_mask(
                  &plan,
                  &cache,
                  3,
                  CSB_V1_CSBGRAPHICS_M11_CUSTOM_BACKGROUND_LAYER_LARGE,
                  skin_def_words,
                  sizeof(skin_def_words) / sizeof(skin_def_words[0]),
                  viewport,
                  sizeof(viewport) / sizeof(viewport[0]),
                  224),
              CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_ERR_NO_SUPPORTED_ENTRIES);

    cache.file_buffer = NULL;
    csb_v1_csbgraphics_dat_real_cache_free(&cache);
    free(bytes);
}

static void test_viewport_render_applies_configured_custom_background_layer(void)
{
    enum { VIEWPORT_WORD_STRIDE = 224 / 8, VIEWPORT_WORD_COUNT = (224 / 8) * 136 };
    uint8_t *bitmap_decoded;
    uint8_t mask_decoded[2u];
    static const uint16_t skin_def_words[] = {
        0u, 108u, 0u, 0u, 0u, 110u, 0u
    };
    CompressedEntryFixture entries[2];
    CSB_V1_CSBGraphicsDatRealCache cache;
    CSB_V1_CSBGraphicsM11RuntimePlan plan;
    CSB_V1_ViewportConfig cfg;
    uint8_t framebuffer[320 * 200];
    uint8_t *bytes;
    size_t size = 0u;

    bitmap_decoded = (uint8_t *)calloc((size_t)(1 + VIEWPORT_WORD_COUNT),
                                       sizeof(uint32_t));
    check_true("viewport_custom_bg.fixture_bitmap_alloc",
               bitmap_decoded != NULL);
    if (!bitmap_decoded) {
        return;
    }
    write_le32(bitmap_decoded, 0u, 224u);
    write_le32(bitmap_decoded, 4u, 0x11111111u);
    write_le32(bitmap_decoded, 8u, 0x22222222u);
    write_le16(mask_decoded, 0u, 0xffffu);

    entries[0].entry_index = 108u;
    entries[0].decoded = bitmap_decoded;
    entries[0].decoded_size =
        (size_t)(1 + VIEWPORT_WORD_COUNT) * sizeof(uint32_t);
    entries[1].entry_index = 110u;
    entries[1].decoded = mask_decoded;
    entries[1].decoded_size = sizeof(mask_decoded);

    bytes = build_csbgraphics_entries_compressed(entries,
                                                 sizeof(entries) / sizeof(entries[0]),
                                                 &size);
    check_true("viewport_custom_bg.fixture_csbgraphics", bytes != NULL);
    if (!bytes) {
        free(bitmap_decoded);
        return;
    }

    cache_from_bytes(&cache, bytes, size);
    csb_v1_csbgraphics_m11_runtime_plan_init(&plan);
    check_int("viewport_custom_bg.add_skin_def",
              csb_v1_csbgraphics_m11_runtime_plan_add_custom_background_skin_def(
                  &cache,
                  skin_def_words,
                  sizeof(skin_def_words) / sizeof(skin_def_words[0]),
                  &plan),
              CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_OK);
    check_int("viewport_custom_bg.pairs",
              (int)plan.custom_background_pair_count, 1);

    memset(framebuffer, 0x09, sizeof(framebuffer));
    csb_v1_viewport_init(&cfg);
    cfg.viewport_pixels = framebuffer;
    cfg.viewport_stride = 320;
    cfg.csbgraphics_plan = &plan;
    cfg.csbgraphics_cache = &cache;
    cfg.custom_background_skin_def_words = skin_def_words;
    cfg.custom_background_skin_def_word_count =
        sizeof(skin_def_words) / sizeof(skin_def_words[0]);
    cfg.custom_background_room_num = 4;
    cfg.custom_background_layer_masks
        [CSB_V1_CUSTOM_BACKGROUND_LAYER_NEAR].src_x = 0;
    cfg.custom_background_layer_masks
        [CSB_V1_CUSTOM_BACKGROUND_LAYER_NEAR].src_y = 0;
    cfg.custom_background_layer_masks
        [CSB_V1_CUSTOM_BACKGROUND_LAYER_NEAR].dst_x = 0;
    cfg.custom_background_layer_masks
        [CSB_V1_CUSTOM_BACKGROUND_LAYER_NEAR].dst_y = 0;
    cfg.custom_background_layer_masks
        [CSB_V1_CUSTOM_BACKGROUND_LAYER_NEAR].width = 16;
    cfg.custom_background_layer_masks
        [CSB_V1_CUSTOM_BACKGROUND_LAYER_NEAR].height = 1;
    cfg.custom_background_layer_mask_valid
        [CSB_V1_CUSTOM_BACKGROUND_LAYER_NEAR] = 1u;

    csb_v1_viewport_render_frame(&cfg, 0, 1, 2);
    check_int("viewport_custom_bg.applied_count",
              cfg.custom_background_applied_count, 1);
    check_int("viewport_custom_bg.first_pixel",
              framebuffer[33 * 320 + 0], 1);
    check_int("viewport_custom_bg.eighth_pixel",
              framebuffer[33 * 320 + 7], 1);
    check_int("viewport_custom_bg.ninth_pixel",
              framebuffer[33 * 320 + 8], 2);
    check_int("viewport_custom_bg.sixteenth_pixel",
              framebuffer[33 * 320 + 15], 2);

    cache.file_buffer = NULL;
    csb_v1_csbgraphics_dat_real_cache_free(&cache);
    free(bytes);
    free(bitmap_decoded);
}

static void test_viewport_render_applies_auto_mask_custom_background_layer(void)
{
    enum { VIEWPORT_WORD_STRIDE = 224 / 8, VIEWPORT_WORD_COUNT = (224 / 8) * 136 };
    uint8_t *bitmap_decoded;
    uint8_t mask_decoded[4u + 16u * 4u + 12u + 2u];
    static const uint16_t skin_def_words[] = {
        0u, 108u, 0u, 0u, 0u, 110u, 0u
    };
    CompressedEntryFixture entries[2];
    CSB_V1_CSBGraphicsDatRealCache cache;
    CSB_V1_CSBGraphicsM11RuntimePlan plan;
    CSB_V1_ViewportConfig cfg;
    uint8_t framebuffer[320 * 200];
    uint8_t *bytes;
    size_t size = 0u;

    bitmap_decoded = (uint8_t *)calloc((size_t)(1 + VIEWPORT_WORD_COUNT),
                                       sizeof(uint32_t));
    check_true("viewport_custom_bg_auto.fixture_bitmap_alloc",
               bitmap_decoded != NULL);
    if (!bitmap_decoded) {
        return;
    }
    write_le32(bitmap_decoded, 0u, 224u);
    write_le32(bitmap_decoded, (1u + 30u) * 4u, 0x11111111u);
    write_le32(bitmap_decoded, (1u + 31u) * 4u, 0x22222222u);
    write_background_mask_fixture(mask_decoded, sizeof(mask_decoded), 4u, 0xffffu);

    entries[0].entry_index = 108u;
    entries[0].decoded = bitmap_decoded;
    entries[0].decoded_size =
        (size_t)(1 + VIEWPORT_WORD_COUNT) * sizeof(uint32_t);
    entries[1].entry_index = 110u;
    entries[1].decoded = mask_decoded;
    entries[1].decoded_size = sizeof(mask_decoded);

    bytes = build_csbgraphics_entries_compressed(entries,
                                                 sizeof(entries) / sizeof(entries[0]),
                                                 &size);
    check_true("viewport_custom_bg_auto.fixture_csbgraphics", bytes != NULL);
    if (!bytes) {
        free(bitmap_decoded);
        return;
    }

    cache_from_bytes(&cache, bytes, size);
    csb_v1_csbgraphics_m11_runtime_plan_init(&plan);
    check_int("viewport_custom_bg_auto.add_skin_def",
              csb_v1_csbgraphics_m11_runtime_plan_add_custom_background_skin_def(
                  &cache,
                  skin_def_words,
                  sizeof(skin_def_words) / sizeof(skin_def_words[0]),
                  &plan),
              CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_OK);

    memset(framebuffer, 0x09, sizeof(framebuffer));
    csb_v1_viewport_init(&cfg);
    cfg.viewport_pixels = framebuffer;
    cfg.viewport_stride = 320;
    cfg.csbgraphics_plan = &plan;
    cfg.csbgraphics_cache = &cache;
    cfg.custom_background_skin_def_words = skin_def_words;
    cfg.custom_background_skin_def_word_count =
        sizeof(skin_def_words) / sizeof(skin_def_words[0]);
    cfg.custom_background_room_num = 4;

    csb_v1_viewport_render_frame(&cfg, 0, 1, 2);
    check_int("viewport_custom_bg_auto.applied_count",
              cfg.custom_background_applied_count, 1);
    check_int("viewport_custom_bg_auto.first_pixel",
              framebuffer[34 * 320 + 16], 1);
    check_int("viewport_custom_bg_auto.eighth_pixel",
              framebuffer[34 * 320 + 23], 1);
    check_int("viewport_custom_bg_auto.ninth_pixel",
              framebuffer[34 * 320 + 24], 2);
    check_int("viewport_custom_bg_auto.sixteenth_pixel",
              framebuffer[34 * 320 + 31], 2);

    cache.file_buffer = NULL;
    csb_v1_csbgraphics_dat_real_cache_free(&cache);
    free(bytes);
    free(bitmap_decoded);
}

static void test_viewport_render_selects_cell_skin_custom_background_layer(void)
{
    enum { VIEWPORT_WORD_STRIDE = 224 / 8, VIEWPORT_WORD_COUNT = (224 / 8) * 136 };
    uint8_t skin_index_decoded[42u];
    uint8_t *bitmap_skin0;
    uint8_t *bitmap_skin1;
    uint8_t mask_skin0[4u + 16u * 4u + 12u + 2u];
    uint8_t mask_skin1[4u + 16u * 4u + 12u + 2u];
    static const uint16_t skin0_words[] = {
        100u, 0u, 0u, 0u, 104u, 0u, 0u, 0u, 0u
    };
    static const uint16_t skin1_words[] = {
        120u, 0u, 0u, 0u, 124u, 0u, 0u, 0u, 0u
    };
    CompressedEntryFixture entries[5];
    CSB_V1_CSBGraphicsDatRealCache cache;
    CSB_V1_CSBGraphicsM11RuntimePlan plan;
    CSB_V1_ViewportConfig cfg;
    uint8_t framebuffer[320 * 200];
    uint8_t cell_skins[4 * 4];
    uint8_t *bytes;
    size_t size = 0u;
    size_t i;

    bitmap_skin0 = (uint8_t *)calloc((size_t)(1 + VIEWPORT_WORD_COUNT),
                                     sizeof(uint32_t));
    bitmap_skin1 = (uint8_t *)calloc((size_t)(1 + VIEWPORT_WORD_COUNT),
                                     sizeof(uint32_t));
    check_true("viewport_custom_bg_cell.fixture_bitmap0", bitmap_skin0 != NULL);
    check_true("viewport_custom_bg_cell.fixture_bitmap1", bitmap_skin1 != NULL);
    if (!bitmap_skin0 || !bitmap_skin1) {
        free(bitmap_skin0);
        free(bitmap_skin1);
        return;
    }

    memset(skin_index_decoded, 0, sizeof(skin_index_decoded));
    write_le16(skin_index_decoded, 0u, 2u);
    write_le16(skin_index_decoded, 2u, 6u);
    write_le16(skin_index_decoded, 4u, 24u);
    for (i = 0u; i < sizeof(skin0_words) / sizeof(skin0_words[0]); ++i) {
        write_le16(skin_index_decoded, 6u + i * 2u, skin0_words[i]);
        write_le16(skin_index_decoded, 24u + i * 2u, skin1_words[i]);
    }

    write_le32(bitmap_skin0, 0u, 224u);
    write_le32(bitmap_skin0, (1u + 30u) * 4u, 0x11111111u);
    write_le32(bitmap_skin0, (1u + 31u) * 4u, 0x22222222u);
    write_le32(bitmap_skin1, 0u, 224u);
    write_le32(bitmap_skin1, (1u + 30u) * 4u, 0x33333333u);
    write_le32(bitmap_skin1, (1u + 31u) * 4u, 0x44444444u);
    write_background_mask_fixture(mask_skin0, sizeof(mask_skin0), 4u, 0xffffu);
    write_background_mask_fixture(mask_skin1, sizeof(mask_skin1), 4u, 0xffffu);

    entries[0].entry_index = 1u;
    entries[0].decoded = skin_index_decoded;
    entries[0].decoded_size = sizeof(skin_index_decoded);
    entries[1].entry_index = 100u;
    entries[1].decoded = bitmap_skin0;
    entries[1].decoded_size = (size_t)(1 + VIEWPORT_WORD_COUNT) * sizeof(uint32_t);
    entries[2].entry_index = 104u;
    entries[2].decoded = mask_skin0;
    entries[2].decoded_size = sizeof(mask_skin0);
    entries[3].entry_index = 120u;
    entries[3].decoded = bitmap_skin1;
    entries[3].decoded_size = (size_t)(1 + VIEWPORT_WORD_COUNT) * sizeof(uint32_t);
    entries[4].entry_index = 124u;
    entries[4].decoded = mask_skin1;
    entries[4].decoded_size = sizeof(mask_skin1);

    bytes = build_csbgraphics_entries_compressed(entries,
                                                 sizeof(entries) / sizeof(entries[0]),
                                                 &size);
    check_true("viewport_custom_bg_cell.fixture_csbgraphics", bytes != NULL);
    if (!bytes) {
        free(bitmap_skin0);
        free(bitmap_skin1);
        return;
    }

    cache_from_bytes(&cache, bytes, size);
    csb_v1_csbgraphics_m11_runtime_plan_init(&plan);
    check_int("viewport_custom_bg_cell.add_skin0",
              csb_v1_csbgraphics_m11_runtime_plan_add_custom_background_skin_def(
                  &cache,
                  skin0_words,
                  sizeof(skin0_words) / sizeof(skin0_words[0]),
                  &plan),
              CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_OK);

    memset(framebuffer, 0x09, sizeof(framebuffer));
    memset(cell_skins, 0, sizeof(cell_skins));
    cell_skins[1u * 4u + 1u] = 1u;
    csb_v1_viewport_init(&cfg);
    cfg.viewport_pixels = framebuffer;
    cfg.viewport_stride = 320;
    cfg.csbgraphics_plan = &plan;
    cfg.csbgraphics_cache = &cache;
    cfg.custom_background_skin_def_words = skin0_words;
    cfg.custom_background_skin_def_word_count =
        sizeof(skin0_words) / sizeof(skin0_words[0]);
    cfg.custom_background_room_num = 4;
    cfg.custom_background_cell_skins = cell_skins;
    cfg.custom_background_cell_skin_width = 4;
    cfg.custom_background_cell_skin_height = 4;

    csb_v1_viewport_render_frame(&cfg, 0, 1, 4);
    check_int("viewport_custom_bg_cell.selected_skin",
              cfg.custom_background_selected_skin_num, 1);
    check_int("viewport_custom_bg_cell.used_default",
              cfg.custom_background_used_default_skin, 0);
    check_int("viewport_custom_bg_cell.applied_count",
              cfg.custom_background_applied_count, 1);
    check_int("viewport_custom_bg_cell.first_pixel",
              framebuffer[34 * 320 + 16], 3);
    check_int("viewport_custom_bg_cell.ninth_pixel",
              framebuffer[34 * 320 + 24], 4);

    cache.file_buffer = NULL;
    csb_v1_csbgraphics_dat_real_cache_free(&cache);
    free(bytes);
    free(bitmap_skin0);
    free(bitmap_skin1);
}

static void test_viewport_render_auto_room_slots_custom_background_layer(void)
{
    enum { VIEWPORT_WORD_STRIDE = 224 / 8, VIEWPORT_WORD_COUNT = (224 / 8) * 136 };
    uint8_t skin_index_decoded[24u];
    uint8_t *bitmap_skin1;
    uint8_t mask_skin1[4u + 16u * 4u + (12u + 2u) * 2u];
    static const uint16_t skin1_words[] = {
        120u, 0u, 0u, 0u, 124u, 0u, 0u, 0u, 0u
    };
    CompressedEntryFixture entries[3];
    CSB_V1_CSBGraphicsDatRealCache cache;
    CSB_V1_CSBGraphicsM11RuntimePlan plan;
    CSB_V1_ViewportConfig cfg;
    uint8_t framebuffer[320 * 200];
    uint8_t *bytes;
    size_t size = 0u;
    size_t i;

    bitmap_skin1 = (uint8_t *)calloc((size_t)(1 + VIEWPORT_WORD_COUNT),
                                     sizeof(uint32_t));
    check_true("viewport_custom_bg_rooms.fixture_bitmap", bitmap_skin1 != NULL);
    if (!bitmap_skin1) {
        return;
    }

    memset(skin_index_decoded, 0, sizeof(skin_index_decoded));
    write_le16(skin_index_decoded, 0u, 2u);
    write_le16(skin_index_decoded, 4u, 6u);
    for (i = 0u; i < sizeof(skin1_words) / sizeof(skin1_words[0]); ++i) {
        write_le16(skin_index_decoded, 6u + i * 2u, skin1_words[i]);
    }

    write_le32(bitmap_skin1, 0u, 224u);
    write_le32(bitmap_skin1, (1u + 30u) * 4u, 0x55555555u);
    write_le32(bitmap_skin1, (1u + 31u) * 4u, 0x66666666u);
    write_le32(bitmap_skin1, (1u + 58u) * 4u, 0x77777777u);
    write_le32(bitmap_skin1, (1u + 59u) * 4u, 0x88888888u);
    write_background_mask_dual_room_fixture(mask_skin1,
                                            sizeof(mask_skin1),
                                            0u,
                                            15u,
                                            0xffffu);

    entries[0].entry_index = 1u;
    entries[0].decoded = skin_index_decoded;
    entries[0].decoded_size = sizeof(skin_index_decoded);
    entries[1].entry_index = 120u;
    entries[1].decoded = bitmap_skin1;
    entries[1].decoded_size = (size_t)(1 + VIEWPORT_WORD_COUNT) * sizeof(uint32_t);
    entries[2].entry_index = 124u;
    entries[2].decoded = mask_skin1;
    entries[2].decoded_size = sizeof(mask_skin1);

    bytes = build_csbgraphics_entries_compressed(entries,
                                                 sizeof(entries) / sizeof(entries[0]),
                                                 &size);
    check_true("viewport_custom_bg_rooms.fixture_csbgraphics", bytes != NULL);
    if (!bytes) {
        free(bitmap_skin1);
        return;
    }

    cache_from_bytes(&cache, bytes, size);
    csb_v1_csbgraphics_m11_runtime_plan_init(&plan);
    check_int("viewport_custom_bg_rooms.add_skin1",
              csb_v1_csbgraphics_m11_runtime_plan_add_custom_background_skin_def(
                  &cache,
                  skin1_words,
                  sizeof(skin1_words) / sizeof(skin1_words[0]),
                  &plan),
              CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_OK);

    memset(framebuffer, 0x09, sizeof(framebuffer));
    csb_v1_viewport_init(&cfg);
    cfg.viewport_pixels = framebuffer;
    cfg.viewport_stride = 320;
    cfg.csbgraphics_plan = &plan;
    cfg.csbgraphics_cache = &cache;
    cfg.custom_background_default_skin = 1;

    csb_v1_viewport_render_frame(&cfg, 0, 1, 4);
    check_int("viewport_custom_bg_rooms.selected_skin",
              cfg.custom_background_selected_skin_num, 1);
    check_int("viewport_custom_bg_rooms.used_default",
              cfg.custom_background_used_default_skin, 1);
    check_int("viewport_custom_bg_rooms.applied_count",
              cfg.custom_background_applied_count, 2);
    check_int("viewport_custom_bg_rooms.room_mask",
              (int)cfg.custom_background_applied_room_mask,
              (int)((1u << 0u) | (1u << 15u)));
    check_int("viewport_custom_bg_rooms.last_room",
              cfg.custom_background_last_room_num, 15);
    check_int("viewport_custom_bg_rooms.room0_pixel",
              framebuffer[34 * 320 + 16], 5);
    check_int("viewport_custom_bg_rooms.room15_pixel",
              framebuffer[35 * 320 + 16], 7);

    cache.file_buffer = NULL;
    csb_v1_csbgraphics_dat_real_cache_free(&cache);
    free(bytes);
    free(bitmap_skin1);
}

int main(void)
{
    test_build_known_c040_plan();
    test_explicit_geometry_apply();
    test_unknown_geometry_is_honest();
    test_custom_background_skin_def_decode();
    test_custom_background_skin_def_pairs_are_deferred();
    test_custom_background_aligned_mask_apply();
    test_custom_background_room_layer_apply();
    test_custom_background_room_layer_auto_mask_apply();
    test_viewport_render_applies_configured_custom_background_layer();
    test_viewport_render_applies_auto_mask_custom_background_layer();
    test_viewport_render_selects_cell_skin_custom_background_layer();
    test_viewport_render_auto_room_slots_custom_background_layer();
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
