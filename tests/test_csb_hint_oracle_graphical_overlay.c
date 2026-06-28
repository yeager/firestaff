/*
 * test_csb_hint_oracle_graphical_overlay.c
 *
 * Data-free contract tests for the CSB Hint Oracle graphical
 * overlay boundary. Synthetic HCSB.HTC data is used only to stage a
 * decoded first page; no real Utility Disk asset is loaded.
 */

#include "csb_hint_oracle_graphical_overlay.h"
#include "csb_hint_oracle_htc.h"
#include "csb_hint_oracle_htc_real_scan.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ASSERT_TRUE(cond) do {                                             \
    if (!(cond)) {                                                         \
        fprintf(stderr, "ASSERTION FAILED at %s:%d: %s\n",                \
                __FILE__, __LINE__, #cond);                                \
        return 0;                                                          \
    }                                                                      \
} while (0)

static void put_be16(uint8_t *p, uint16_t v)
{
    p[0] = (uint8_t)(v >> 8u);
    p[1] = (uint8_t)(v & 0xffu);
}

static size_t pack_codes_9bit(const uint16_t *codes,
                              size_t code_count,
                              uint8_t *out,
                              size_t out_cap)
{
    size_t bit_pos = 0u;
    size_t i;

    memset(out, 0, out_cap);
    for (i = 0u; i < code_count; ++i) {
        unsigned int bit;
        for (bit = 0u; bit < 9u; ++bit) {
            size_t pos = bit_pos + bit;
            if ((codes[i] & (uint16_t)(1u << bit)) != 0u) {
                out[pos / 8u] |= (uint8_t)(1u << (pos % 8u));
            }
        }
        bit_pos += 9u;
    }
    return (bit_pos + 7u) / 8u;
}

static size_t pack_literal_string(const char *text,
                                  uint8_t *out,
                                  size_t out_cap)
{
    uint16_t codes[128];
    size_t i;
    size_t len = strlen(text) + 1u;

    ASSERT_TRUE(len <= sizeof(codes) / sizeof(codes[0]));
    for (i = 0u; i < len; ++i) {
        codes[i] = (uint16_t)(uint8_t)text[i];
    }
    return pack_codes_9bit(codes, len, out, out_cap);
}

static void write_hint_record(uint8_t *dst,
                              const char *name,
                              uint16_t first_page,
                              uint16_t page_count)
{
    size_t len = strlen(name);
    memset(dst, 0, CSB_HINT_ORACLE_HTC_HINT_RECORD_SIZE);
    if (len > CSB_HINT_ORACLE_HTC_HINT_NAME_BYTES) {
        len = CSB_HINT_ORACLE_HTC_HINT_NAME_BYTES;
    }
    memcpy(dst, name, len);
    put_be16(dst + CSB_HINT_ORACLE_HTC_HINT_NAME_BYTES, first_page);
    put_be16(dst + CSB_HINT_ORACLE_HTC_HINT_NAME_BYTES + 2u, page_count);
}

static size_t build_fixture(uint8_t *buf, size_t cap)
{
    uint8_t content0[128];
    uint8_t content1[32];
    size_t content0_len;
    size_t content1_len;
    size_t off = 0u;

    content0_len = pack_literal_string(
        "Cast/ZOKATHRA and read the oracle page.", content0,
        sizeof(content0));
    content1_len = pack_literal_string("Anywhere", content1,
                                       sizeof(content1));

    ASSERT_TRUE(cap >= 10u + 12u + 4u + 52u + 2u + 8u +
                content0_len + content1_len);

    put_be16(buf + off, CSB_HINT_ORACLE_HTC_FORMAT_WORD); off += 2u;
    put_be16(buf + off, CSB_HINT_ORACLE_HTC_DUNGEON_ID); off += 2u;
    put_be16(buf + off, 3u); off += 2u;

    put_be16(buf + off, 2u); off += 2u;
    put_be16(buf + off, CSB_HINT_ORACLE_HTC_LOCATION_RECORD_SIZE); off += 2u;
    buf[off++] = 25u; buf[off++] = 7u; buf[off++] = 5u; buf[off++] = 0u;
    put_be16(buf + off, 0u); off += 2u;
    buf[off++] = CSB_HINT_ORACLE_HTC_ANY_XY;
    buf[off++] = CSB_HINT_ORACLE_HTC_ANY_XY;
    buf[off++] = 5u; buf[off++] = 0u;
    put_be16(buf + off, 1u); off += 2u;

    put_be16(buf + off, 2u); off += 2u;
    put_be16(buf + off, CSB_HINT_ORACLE_HTC_HINT_RECORD_SIZE); off += 2u;
    write_hint_record(buf + off, "PROVE YOU ARE WIZARD", 0u, 3u);
    off += CSB_HINT_ORACLE_HTC_HINT_RECORD_SIZE;
    write_hint_record(buf + off, "ANYWHERE", 3u, 1u);
    off += CSB_HINT_ORACLE_HTC_HINT_RECORD_SIZE;

    put_be16(buf + off, 4u); off += 2u;
    put_be16(buf + off, (uint16_t)content0_len); off += 2u;
    put_be16(buf + off, 0u); off += 2u;
    put_be16(buf + off, 0u); off += 2u;
    put_be16(buf + off, (uint16_t)content1_len); off += 2u;

    memcpy(buf + off, content0, content0_len); off += content0_len;
    memcpy(buf + off, content1, content1_len); off += content1_len;
    return off;
}

static int build_in_memory_cache(CSB_HintOracleHTC_RealCache *cache,
                                 const uint8_t *buf,
                                 size_t size)
{
    uint8_t *owned;
    ASSERT_TRUE(cache != NULL);
    csb_hint_oracle_htc_real_cache_init(cache);
    owned = (uint8_t *)malloc(size > 0u ? size : 1u);
    if (!owned) {
        return 0;
    }
    memcpy(owned, buf, size);
    cache->file_buffer = owned;
    cache->file_size = size;
    if (csb_hint_oracle_htc_parse(owned, size, &cache->htc) !=
        CSB_HINT_ORACLE_HTC_OK) {
        csb_hint_oracle_htc_real_cache_free(cache);
        return 0;
    }
    strcpy(cache->matched_md5, "00112233445566778899aabbccddeeff");
    strcpy(cache->matched_label, "overlay-test-fixture");
    strcpy(cache->resolved_path, "<in-memory-fixture>");
    strcpy(cache->original_path, "<in-memory-fixture>");
    cache->loaded = 1;
    return 1;
}

static size_t count_color(const uint8_t *fb, size_t len, uint8_t color)
{
    size_t count = 0u;
    size_t i;
    for (i = 0u; i < len; ++i) {
        if (fb[i] == color) {
            ++count;
        }
    }
    return count;
}

static int test_default_config_and_result_names(void)
{
    CSB_HintOracleOverlay_Config cfg;
    csb_hint_oracle_overlay_default_config(&cfg);
    ASSERT_TRUE(cfg.x == 28);
    ASSERT_TRUE(cfg.y == 20);
    ASSERT_TRUE(cfg.w == 264);
    ASSERT_TRUE(cfg.h == 152);
    ASSERT_TRUE(strcmp(csb_hint_oracle_overlay_result_name(
                           CSB_HINT_ORACLE_OVERLAY_OK), "OK") == 0);
    ASSERT_TRUE(strcmp(csb_hint_oracle_overlay_result_name(
                           CSB_HINT_ORACLE_OVERLAY_ERR_NOT_LOADED),
                       "not-loaded") == 0);
    ASSERT_TRUE(strcmp(csb_hint_oracle_overlay_result_name(9999),
                       "unknown") == 0);
    return 1;
}

static int test_render_text_draws_frame_and_glyphs(void)
{
    uint8_t fb[320 * 200];
    CSB_HintOracleOverlay_Stats stats;
    int rc;

    memset(fb, 0, sizeof(fb));
    rc = csb_hint_oracle_overlay_render_text(
        "CSB HINT ORACLE",
        "Cast/ZOKATHRA and read the oracle page.",
        fb, 320, 200, NULL, &stats);

    ASSERT_TRUE(rc == CSB_HINT_ORACLE_OVERLAY_OK);
    ASSERT_TRUE(stats.background_pixels > 30000u);
    ASSERT_TRUE(stats.border_pixels > 1000u);
    ASSERT_TRUE(stats.glyph_pixels > 100u);
    ASSERT_TRUE(stats.chars_drawn >= 30u);
    ASSERT_TRUE(count_color(fb, sizeof(fb), 14u) > 0u);
    ASSERT_TRUE(count_color(fb, sizeof(fb), 15u) > 0u);
    ASSERT_TRUE(count_color(fb, sizeof(fb), 12u) > 0u);
    ASSERT_TRUE(fb[20u * 320u + 28u] == 14u);
    return 1;
}

static int test_render_text_rejects_bad_geometry(void)
{
    uint8_t fb[320 * 200];
    CSB_HintOracleOverlay_Config cfg;
    int rc;

    memset(fb, 0, sizeof(fb));
    csb_hint_oracle_overlay_default_config(&cfg);
    cfg.x = 300;
    cfg.w = 64;
    rc = csb_hint_oracle_overlay_render_text(
        "TITLE", "TEXT", fb, 320, 200, &cfg, NULL);
    ASSERT_TRUE(rc == CSB_HINT_ORACLE_OVERLAY_ERR_GEOMETRY);
    ASSERT_TRUE(count_color(fb, sizeof(fb), 14u) == 0u);
    return 1;
}

static int test_render_text_clips_long_page(void)
{
    uint8_t fb[320 * 200];
    char long_text[4096];
    CSB_HintOracleOverlay_Stats stats;
    size_t i;
    int rc;

    for (i = 0u; i < sizeof(long_text) - 1u; ++i) {
        long_text[i] = (i % 17u == 16u) ? ' ' : (char)('A' + (int)(i % 26u));
    }
    long_text[sizeof(long_text) - 1u] = '\0';

    memset(fb, 0, sizeof(fb));
    rc = csb_hint_oracle_overlay_render_text(
        "LONG PAGE", long_text, fb, 320, 200, NULL, &stats);

    ASSERT_TRUE(rc == CSB_HINT_ORACLE_OVERLAY_OK);
    ASSERT_TRUE(stats.clipped == 1);
    ASSERT_TRUE(stats.lines_drawn <= 13u);
    ASSERT_TRUE(stats.glyph_pixels > 500u);
    return 1;
}

static int test_render_hint_uses_decoded_first_page(void)
{
    uint8_t fixture[512];
    size_t fixture_size;
    CSB_HintOracleHTC_RealCache cache;
    uint8_t fb[320 * 200];
    CSB_HintOracleOverlay_Stats stats;
    int rc;

    fixture_size = build_fixture(fixture, sizeof(fixture));
    ASSERT_TRUE(build_in_memory_cache(&cache, fixture, fixture_size));

    memset(fb, 0, sizeof(fb));
    rc = csb_hint_oracle_overlay_render_hint(
        &cache, 0u, fb, 320, 200, NULL, &stats);
    ASSERT_TRUE(rc == CSB_HINT_ORACLE_OVERLAY_OK);
    ASSERT_TRUE(stats.glyph_pixels > 150u);
    ASSERT_TRUE(stats.chars_drawn >= 40u);
    ASSERT_TRUE(count_color(fb, sizeof(fb), 12u) > 0u);

    memset(fb, 0, sizeof(fb));
    rc = csb_hint_oracle_overlay_render_hint(
        &cache, 999u, fb, 320, 200, NULL, &stats);
    ASSERT_TRUE(rc == CSB_HINT_ORACLE_OVERLAY_ERR_HINT_OUT_OF_RANGE);
    ASSERT_TRUE(count_color(fb, sizeof(fb), 12u) == 0u);

    csb_hint_oracle_htc_real_cache_free(&cache);
    return 1;
}

static int test_render_hint_not_loaded(void)
{
    uint8_t fb[320 * 200];
    CSB_HintOracleHTC_RealCache cache;
    int rc;

    memset(fb, 0, sizeof(fb));
    csb_hint_oracle_htc_real_cache_init(&cache);
    rc = csb_hint_oracle_overlay_render_hint(
        &cache, 0u, fb, 320, 200, NULL, NULL);
    ASSERT_TRUE(rc == CSB_HINT_ORACLE_OVERLAY_ERR_NOT_LOADED);
    ASSERT_TRUE(count_color(fb, sizeof(fb), 14u) == 0u);
    return 1;
}

int main(void)
{
    int passed = 0;
    int total = 0;

#define RUN_TEST(fn) do {                                                   \
    ++total;                                                                \
    if (fn()) {                                                             \
        ++passed;                                                           \
        printf("PASS: %s\n", #fn);                                          \
    } else {                                                                \
        printf("FAIL: %s\n", #fn);                                          \
    }                                                                       \
} while (0)

    RUN_TEST(test_default_config_and_result_names);
    RUN_TEST(test_render_text_draws_frame_and_glyphs);
    RUN_TEST(test_render_text_rejects_bad_geometry);
    RUN_TEST(test_render_text_clips_long_page);
    RUN_TEST(test_render_hint_uses_decoded_first_page);
    RUN_TEST(test_render_hint_not_loaded);

#undef RUN_TEST

    printf("csb_hint_oracle_graphical_overlay: %d/%d tests passed\n",
           passed, total);
    return passed == total ? 0 : 1;
}
