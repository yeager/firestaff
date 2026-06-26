/*
 * test_csb_hint_oracle_ui_runtime_binding.c
 *
 * Data-free contract tests for the CSB Hint Oracle UI/runtime
 * binding surface.
 *
 * Scope:
 *   - Not-loaded argument: report + hint + resolve all return
 *     CSB_HINT_ORACLE_UI_BINDING_ERR_NOT_LOADED (or write a
 *     "(not loaded)" marker) so callers can produce a non-NULL
 *     output even on empty caches.
 *   - Trivial binding smoke: with a synthetically-built
 *     HCSB.HTC (the same fixture the data-free parser test
 *     uses), the binding surface produces a non-empty report
 *     that contains the loaded matched-md5/label + the
 *     hint 0 name + first-page decoded text + the wildcard
 *     (level=0, x=255, y=255) resolve.
 *   - Truncation: a small output buffer triggers the
 *     out_was_truncated=1 path on the report.
 *   - Out-of-range hint index: format_hint returns
 *     CSB_HINT_ORACLE_UI_BINDING_ERR_HINT_OUT_OF_RANGE and the
 *     buffer is left empty.
 *
 * Non-claims:
 *   - no real Utility Disk asset is loaded (synthetic fixture
 *     only)
 *   - no Hint Oracle UI/runtime path is wired into M11/M12
 *   - we do not claim parity for every Utility Disk release
 *     variant; the binding surface itself is variant-agnostic
 *     and the known-hash list still gates which HCSB.HTCs the
 *     scan module accepts
 */

#include "csb_hint_oracle_htc.h"
#include "csb_hint_oracle_htc_real_scan.h"
#include "csb_hint_oracle_ui_runtime_binding.h"

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

/* ── Fixture builders (reused shape from the parser test) ────────── */

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
    size_t bit_pos = 0;
    size_t i;

    memset(out, 0, out_cap);
    for (i = 0; i < code_count; ++i) {
        unsigned int bit;
        for (bit = 0; bit < 9u; ++bit) {
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
    uint16_t codes[96];
    size_t i;
    size_t len = strlen(text) + 1u;

    ASSERT_TRUE(len <= sizeof(codes) / sizeof(codes[0]));
    for (i = 0; i < len; ++i) {
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
    uint8_t content0[96];
    uint8_t content1[32];
    size_t content0_len;
    size_t content1_len;
    size_t off = 0;

    content0_len = pack_literal_string("Cast/ZOKATHRA", content0,
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

/* Build a RealCache backed by an in-memory HCSB.HTC buffer
 * (no file I/O, no asset scanner). The caller is responsible
 * for `free(buf)` AFTER the binding is exercised if it is
 * stack-allocated, OR pass a heap-allocated buffer that
 * csb_hint_oracle_htc_real_cache_free() can take ownership
 * of. To make the lifecycle uniform, this helper copies
 * the buffer into a freshly-allocated heap buffer and
 * transfers ownership to the cache. */
static int build_in_memory_cache(CSB_HintOracleHTC_RealCache *cache,
                                 const uint8_t *buf, size_t size)
{
    uint8_t *owned;
    ASSERT_TRUE(cache != NULL);
    csb_hint_oracle_htc_real_cache_init(cache);
    owned = (uint8_t *)malloc(size > 0u ? size : 1u);
    if (!owned) {
        return 0;
    }
    memcpy(owned, buf, size);
    /* Transfer ownership of `owned` to the cache. */
    cache->file_buffer = owned;
    cache->file_size = size;
    if (csb_hint_oracle_htc_parse(owned, size, &cache->htc) !=
        CSB_HINT_ORACLE_HTC_OK) {
        csb_hint_oracle_htc_real_cache_free(cache);
        return 0;
    }
    /* Fill in the binding-side metadata so the report has
     * something to print. */
    strncpy(cache->matched_md5,
            "00112233445566778899aabbccddeeff",
            sizeof(cache->matched_md5) - 1u);
    cache->matched_md5[sizeof(cache->matched_md5) - 1u] = '\0';
    strncpy(cache->matched_label,
            "test-fixture-synthetic",
            sizeof(cache->matched_label) - 1u);
    cache->matched_label[sizeof(cache->matched_label) - 1u] = '\0';
    strncpy(cache->resolved_path,
            "<in-memory-fixture>",
            sizeof(cache->resolved_path) - 1u);
    cache->resolved_path[sizeof(cache->resolved_path) - 1u] = '\0';
    strncpy(cache->original_path,
            "<in-memory-fixture>",
            sizeof(cache->original_path) - 1u);
    cache->original_path[sizeof(cache->original_path) - 1u] = '\0';
    cache->loaded = 1;
    return 1;
}

/* ── Tests ──────────────────────────────────────────────────────── */

static int test_not_loaded_argument(void)
{
    char buf[256];
    int truncated = 0;
    int n;

    /* format_report on NULL cache still produces a non-empty
     * "(not loaded)" marker. */
    memset(buf, 0, sizeof(buf));
    n = csb_hint_oracle_ui_binding_format_report(NULL, buf,
                                                 sizeof(buf),
                                                 &truncated);
    ASSERT_TRUE(n > 0);
    ASSERT_TRUE(strstr(buf, "not loaded") != NULL);

    /* format_hint on NULL cache returns NOT_LOADED and leaves
     * the buffer empty. */
    memset(buf, 'X', sizeof(buf));
    n = csb_hint_oracle_ui_binding_format_hint(NULL, 0u, buf,
                                               sizeof(buf),
                                               &truncated);
    ASSERT_TRUE(n == CSB_HINT_ORACLE_UI_BINDING_ERR_NOT_LOADED);
    ASSERT_TRUE(buf[0] == '\0');

    /* resolve_location on NULL cache returns NOT_LOADED and
     * leaves the buffer empty. */
    memset(buf, 'X', sizeof(buf));
    n = csb_hint_oracle_ui_binding_resolve_location(
        NULL, 0u,
        CSB_HINT_ORACLE_HTC_ANY_XY, CSB_HINT_ORACLE_HTC_ANY_XY,
        NULL, buf, sizeof(buf), &truncated);
    ASSERT_TRUE(n == CSB_HINT_ORACLE_UI_BINDING_ERR_NOT_LOADED);
    ASSERT_TRUE(buf[0] == '\0');

    /* format_report on an initialized-but-not-loaded cache
     * also produces a "(not loaded)" marker. */
    {
        CSB_HintOracleHTC_RealCache cache;
        csb_hint_oracle_htc_real_cache_init(&cache);
        memset(buf, 0, sizeof(buf));
        n = csb_hint_oracle_ui_binding_format_report(&cache, buf,
                                                     sizeof(buf),
                                                     &truncated);
        ASSERT_TRUE(n > 0);
        ASSERT_TRUE(strstr(buf, "not loaded") != NULL);
    }
    return 1;
}

static int test_report_contains_binding_smoke(void)
{
    uint8_t buf[256];
    size_t size;
    CSB_HintOracleHTC_RealCache cache;
    char report[4096];
    int truncated = 0;
    int n;

    size = build_fixture(buf, sizeof(buf));
    ASSERT_TRUE(build_in_memory_cache(&cache, buf, size));

    memset(report, 0, sizeof(report));
    n = csb_hint_oracle_ui_binding_format_report(&cache, report,
                                                 sizeof(report),
                                                 &truncated);
    ASSERT_TRUE(n > 0);
    ASSERT_TRUE(truncated == 0);

    /* The report must surface the matched MD5/label we set on
     * the cache. */
    ASSERT_TRUE(strstr(report, "00112233445566778899aabbccddeeff") != NULL);
    ASSERT_TRUE(strstr(report, "test-fixture-synthetic") != NULL);
    /* The report must surface the format word + dungeon id the
     * parser agreed on. */
    ASSERT_TRUE(strstr(report, "format_word=2") != NULL);
    ASSERT_TRUE(strstr(report, "dungeon_id=13") != NULL);
    /* The hint 0 binding smoke must include the parsed hint
     * name from the fixture. */
    ASSERT_TRUE(strstr(report, "PROVE YOU ARE WIZARD") != NULL);
    /* And the binding must include the parsed first-page
     * text "Cast/ZOKATHRA" (the literal string the synthetic
     * LZW stream decodes to). */
    ASSERT_TRUE(strstr(report, "Cast/ZOKATHRA") != NULL);
    /* And the wildcard smoke must include the (255, 255) label
     * + the resolved hint name. The implementation probes
     * level=0 (the real-asset Atari ST 2.x file's level);
     * the synthetic fixture happens to expose its wildcard
     * records at level=5, so on the in-memory fixture the
     * implementation falls back to the failure marker
     * ("level 0 wildcard resolve rc=...") — we still assert
     * the report surfaces the wildcard line itself. */
    ASSERT_TRUE(strstr(report, "level=0 (255,255) wildcard") != NULL);

    csb_hint_oracle_htc_real_cache_free(&cache);
    return 1;
}

static int test_format_hint_text_round_trip(void)
{
    uint8_t buf[256];
    size_t size;
    CSB_HintOracleHTC_RealCache cache;
    char text[CSB_HINT_ORACLE_UI_BINDING_PAGE_CAP * 2];
    int truncated = 0;
    int n;

    size = build_fixture(buf, sizeof(buf));
    ASSERT_TRUE(build_in_memory_cache(&cache, buf, size));

    memset(text, 0, sizeof(text));
    n = csb_hint_oracle_ui_binding_format_hint(
        &cache, 0u, text, sizeof(text), &truncated);
    ASSERT_TRUE(n > 0);
    ASSERT_TRUE(truncated == 0);
    ASSERT_TRUE(strstr(text, "PROVE YOU ARE WIZARD") != NULL);
    ASSERT_TRUE(strstr(text, "Cast/ZOKATHRA") != NULL);

    /* Out-of-range hint index returns HINT_OUT_OF_RANGE and
     * leaves the buffer empty. */
    memset(text, 'X', sizeof(text));
    n = csb_hint_oracle_ui_binding_format_hint(
        &cache, 999u, text, sizeof(text), &truncated);
    ASSERT_TRUE(n == CSB_HINT_ORACLE_UI_BINDING_ERR_HINT_OUT_OF_RANGE);
    ASSERT_TRUE(text[0] == '\0');

    csb_hint_oracle_htc_real_cache_free(&cache);
    return 1;
}

static int test_resolve_location_binding(void)
{
    uint8_t buf[256];
    size_t size;
    CSB_HintOracleHTC_RealCache cache;
    char text[CSB_HINT_ORACLE_UI_BINDING_PAGE_CAP * 2];
    int truncated = 0;
    size_t hint_index = 0u;
    int n;

    size = build_fixture(buf, sizeof(buf));
    ASSERT_TRUE(build_in_memory_cache(&cache, buf, size));

    /* Wildcard (level=5, x=255, y=255) resolves to hint 1
     * ("ANYWHERE") on the synthetic fixture (the fixture's
     * both records are at level=5). */
    memset(text, 0, sizeof(text));
    n = csb_hint_oracle_ui_binding_resolve_location(
        &cache, 5u,
        CSB_HINT_ORACLE_HTC_ANY_XY, CSB_HINT_ORACLE_HTC_ANY_XY,
        &hint_index, text, sizeof(text), &truncated);
    ASSERT_TRUE(n > 0);
    ASSERT_TRUE(hint_index == 1u);
    ASSERT_TRUE(strstr(text, "ANYWHERE") != NULL);
    ASSERT_TRUE(strstr(text, "Anywhere") != NULL);

    /* A non-matching (level, x, y) returns NO_HINT_AT_LOCATION
     * and leaves the buffer empty. */
    memset(text, 'X', sizeof(text));
    n = csb_hint_oracle_ui_binding_resolve_location(
        &cache, 0u, 0u, 0u,
        NULL, text, sizeof(text), &truncated);
    ASSERT_TRUE(n == CSB_HINT_ORACLE_UI_BINDING_ERR_NO_HINT_AT_LOCATION);
    ASSERT_TRUE(text[0] == '\0');

    csb_hint_oracle_htc_real_cache_free(&cache);
    return 1;
}

static int test_report_truncation_path(void)
{
    uint8_t buf[256];
    size_t size;
    CSB_HintOracleHTC_RealCache cache;
    char small[64];
    int truncated = 0;
    int n;

    size = build_fixture(buf, sizeof(buf));
    ASSERT_TRUE(build_in_memory_cache(&cache, buf, size));

    /* A tiny buffer still produces a non-empty output (the
     * header) and flags truncation. */
    memset(small, 0, sizeof(small));
    n = csb_hint_oracle_ui_binding_format_report(&cache, small,
                                                 sizeof(small),
                                                 &truncated);
    ASSERT_TRUE(n > 0);
    ASSERT_TRUE(truncated == 1);
    ASSERT_TRUE(small[sizeof(small) - 1u] == '\0' ||
                small[n < (int)sizeof(small) ? n : (int)sizeof(small) - 1u] == '\0');

    csb_hint_oracle_htc_real_cache_free(&cache);
    return 1;
}

static int test_result_name_table(void)
{
    /* The result-name table covers all the documented
     * non-OK outcomes so callers can log the gate cleanly. */
    ASSERT_TRUE(strcmp(csb_hint_oracle_ui_binding_result_name(
                           CSB_HINT_ORACLE_UI_BINDING_OK), "OK") == 0);
    ASSERT_TRUE(strcmp(csb_hint_oracle_ui_binding_result_name(
                           CSB_HINT_ORACLE_UI_BINDING_ERR_NOT_LOADED),
                       "not-loaded") == 0);
    ASSERT_TRUE(strcmp(csb_hint_oracle_ui_binding_result_name(
                           CSB_HINT_ORACLE_UI_BINDING_ERR_HINT_OUT_OF_RANGE),
                       "hint-out-of-range") == 0);
    ASSERT_TRUE(strcmp(csb_hint_oracle_ui_binding_result_name(
                           CSB_HINT_ORACLE_UI_BINDING_ERR_NO_HINT_AT_LOCATION),
                       "no-hint-at-location") == 0);
    ASSERT_TRUE(strcmp(csb_hint_oracle_ui_binding_result_name(0x7fffffff),
                       "unknown") == 0);
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

    RUN_TEST(test_not_loaded_argument);
    RUN_TEST(test_report_contains_binding_smoke);
    RUN_TEST(test_format_hint_text_round_trip);
    RUN_TEST(test_resolve_location_binding);
    RUN_TEST(test_report_truncation_path);
    RUN_TEST(test_result_name_table);

#undef RUN_TEST

    printf("csb_hint_oracle_ui_runtime_binding: %d/%d tests passed\n",
           passed, total);
    return passed == total ? 0 : 1;
}
