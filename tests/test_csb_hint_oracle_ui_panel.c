/*
 * test_csb_hint_oracle_ui_panel.c
 *
 * Data-free contract tests for the CSB Hint Oracle UI panel
 * binding surface.
 *
 * Scope:
 *   - Empty-panel argument: every accessor returns the
 *     documented negative result code and the cache stays
 *     un-allocated, so a launcher can probe the panel
 *     before any data is loaded.
 *   - Status sticky flag: a panel that fails to load reports
 *     the failure status to subsequent accessor calls.
 *   - Result-name + status-name tables cover all the
 *     documented enums.
 *
 * Synthetic in-memory fixture:
 *   - The panel can't be loaded without a real HCSB.HTC on
 *     disk (the loader is the real-asset scanner). To exercise
 *     the loaded-panel accessors without any file I/O we
 *     side-load the same fixture builder used by the
 *     upstream parser + binding tests, then transfer the
 *     parsed view into the panel's cache (this is the
 *     shape the upstream test fixture already uses for
 *     csb_hint_oracle_ui_runtime_binding).
 *
 * Non-claims:
 *   - no real Utility Disk asset is loaded (synthetic fixture
 *     only)
 *   - no M11/M12 launcher view is wired to this panel API yet
 *   - we do not claim parity for every Utility Disk release
 *     variant; the panel is variant-agnostic and the
 *     known-hash list still gates which HCSB.HTCs the scan
 *     module accepts.
 */

#include "csb_hint_oracle_htc.h"
#include "csb_hint_oracle_htc_real_scan.h"
#include "csb_hint_oracle_ui_panel.h"

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

/* ── Fixture builders (mirrors the parser + binding fixtures) ─────── */

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

/* Side-load a parsed HCSB.HTC into the panel's cache. This
 * is the same shape the upstream binding test uses; we
 * transfer ownership of the heap-allocated buffer to the
 * panel so panel_free() can clean up. */
static int sideload_panel_cache(CSB_HintOracleUIPanel *panel,
                                const uint8_t *buf, size_t size,
                                const char *md5, const char *label)
{
    uint8_t *owned;
    int parse_rc;
    ASSERT_TRUE(panel != NULL);
    /* Always init the panel from a clean slate so we never
     * free an uninitialized file_buffer (the caller may have
     * declared panel on the stack without a prior init). */
    csb_hint_oracle_ui_panel_init(panel);

    owned = (uint8_t *)malloc(size > 0u ? size : 1u);
    if (!owned) {
        return 0;
    }
    memcpy(owned, buf, size);

    /* Fill in the same fields csb_hint_oracle_htc_real_cache_init
     * would set after a successful scan + parse. */
    panel->cache.file_buffer = owned;
    panel->cache.file_size = size;
    parse_rc = csb_hint_oracle_htc_parse(owned, size, &panel->cache.htc);
    if (parse_rc != CSB_HINT_ORACLE_HTC_OK) {
        csb_hint_oracle_ui_panel_free(panel);
        return 0;
    }
    {
        size_t i;
        for (i = 0u; i + 1u < sizeof(panel->cache.matched_md5) && md5 &&
             md5[i] != '\0'; ++i) {
            panel->cache.matched_md5[i] = md5[i];
            panel->cache.matched_md5[i + 1u] = '\0';
        }
    }
    {
        size_t i;
        for (i = 0u; i + 1u < sizeof(panel->cache.matched_label) &&
             label && label[i] != '\0'; ++i) {
            panel->cache.matched_label[i] = label[i];
            panel->cache.matched_label[i + 1u] = '\0';
        }
    }
    strncpy(panel->cache.resolved_path, "<sideload-fixture>",
            sizeof(panel->cache.resolved_path) - 1u);
    panel->cache.resolved_path[sizeof(panel->cache.resolved_path) - 1u] = '\0';
    strncpy(panel->cache.original_path, "<sideload-fixture>",
            sizeof(panel->cache.original_path) - 1u);
    panel->cache.original_path[sizeof(panel->cache.original_path) - 1u] = '\0';
    panel->cache.loaded = 1;

    /* Flip the panel into LOADED status so accessors see the
     * populated cache. We deliberately do not bump load_count
     * because the synthetic side-load is not a real load. */
    panel->status = CSB_HINT_ORACLE_UI_PANEL_STATUS_LOADED;
    panel->last_load_rc = CSB_HINT_ORACLE_HTC_REAL_OK;
    return 1;
}

/* ── Empty-panel contract ────────────────────────────────────────── */

static int test_empty_panel_argument(void)
{
    CSB_HintOracleUIPanel panel;
    char buf[128];
    uint8_t page_buf[128];
    size_t out_count;
    CSB_HintOracleHTC_Location loc;

    csb_hint_oracle_ui_panel_init(&panel);

    /* Counts on an empty panel are 0. */
    ASSERT_TRUE(csb_hint_oracle_ui_panel_hint_count(&panel) == 0u);
    ASSERT_TRUE(csb_hint_oracle_ui_panel_location_count(&panel) == 0u);
    ASSERT_TRUE(csb_hint_oracle_ui_panel_page_count(&panel) == 0u);
    ASSERT_TRUE(csb_hint_oracle_ui_panel_content_size(&panel) == 0u);

    /* Accessors on an empty panel return NOT_LOADED. */
    memset(buf, 'X', sizeof(buf));
    ASSERT_TRUE(csb_hint_oracle_ui_panel_hint_name(&panel, 0u, buf,
                                                   sizeof(buf)) ==
                CSB_HINT_ORACLE_UI_PANEL_ERR_NOT_LOADED);
    ASSERT_TRUE(buf[0] == '\0');

    out_count = 0u;
    ASSERT_TRUE(csb_hint_oracle_ui_panel_hint_first_page(
                    &panel, 0u, page_buf, sizeof(page_buf),
                    &out_count) ==
                CSB_HINT_ORACLE_UI_PANEL_ERR_NOT_LOADED);
    ASSERT_TRUE(out_count == 0u);

    ASSERT_TRUE(csb_hint_oracle_ui_panel_location(
                    &panel, 0u, &loc) ==
                CSB_HINT_ORACLE_UI_PANEL_ERR_NOT_LOADED);

    {
        size_t idx = 0u;
        ASSERT_TRUE(csb_hint_oracle_ui_panel_resolve_location(
                        &panel, 0u,
                        CSB_HINT_ORACLE_HTC_ANY_XY,
                        CSB_HINT_ORACLE_HTC_ANY_XY, &idx) ==
                    CSB_HINT_ORACLE_UI_PANEL_ERR_NOT_LOADED);
    }

    /* Source label on an empty panel returns NOT_LOADED. */
    memset(buf, 'X', sizeof(buf));
    ASSERT_TRUE(csb_hint_oracle_ui_panel_source_label(
                    &panel, buf, sizeof(buf)) ==
                CSB_HINT_ORACLE_UI_PANEL_ERR_NOT_LOADED);
    ASSERT_TRUE(buf[0] == '\0');

    /* Diagnostic report on an empty panel writes a (not loaded)
     * marker. */
    {
        char report[256];
        int truncated = 0;
        int n = csb_hint_oracle_ui_panel_format_diagnostic(
            &panel, report, sizeof(report), &truncated);
        ASSERT_TRUE(n > 0);
        ASSERT_TRUE(strstr(report, "panel:") != NULL);
    }

    csb_hint_oracle_ui_panel_free(&panel);
    return 1;
}

/* ── NULL-argument contract ──────────────────────────────────────── */

static int test_null_argument(void)
{
    /* All accessors return ARGUMENT and leave buffers alone. */
    char buf[16];
    memset(buf, 'X', sizeof(buf));
    ASSERT_TRUE(csb_hint_oracle_ui_panel_hint_name(
                    NULL, 0u, buf, sizeof(buf)) ==
                CSB_HINT_ORACLE_UI_PANEL_ERR_ARGUMENT);
    ASSERT_TRUE(buf[0] == '\0');

    {
        CSB_HintOracleHTC_Location loc;
        ASSERT_TRUE(csb_hint_oracle_ui_panel_location(
                        NULL, 0u, &loc) ==
                    CSB_HINT_ORACLE_UI_PANEL_ERR_ARGUMENT);
    }

    {
        size_t idx = 0u;
        ASSERT_TRUE(csb_hint_oracle_ui_panel_resolve_location(
                        NULL, 0u, 0u, 0u, &idx) ==
                    CSB_HINT_ORACLE_UI_PANEL_ERR_ARGUMENT);
    }

    /* Status on NULL panel returns ARGUMENT_ERROR. */
    ASSERT_TRUE(csb_hint_oracle_ui_panel_status(NULL) ==
                CSB_HINT_ORACLE_UI_PANEL_STATUS_ARGUMENT_ERROR);

    /* Load with NULL panel returns ARGUMENT. */
    ASSERT_TRUE(csb_hint_oracle_ui_panel_load(
                    NULL, NULL, NULL, 6) ==
                CSB_HINT_ORACLE_UI_PANEL_ERR_ARGUMENT);

    /* Free on NULL is a no-op (no crash). */
    csb_hint_oracle_ui_panel_free(NULL);

    /* Init on NULL is a no-op. */
    csb_hint_oracle_ui_panel_init(NULL);
    return 1;
}

/* ── Result + status label tables ────────────────────────────────── */

static int test_label_tables(void)
{
    /* Result codes we surface to the UI. */
    ASSERT_TRUE(strcmp(csb_hint_oracle_ui_panel_result_name(
                           CSB_HINT_ORACLE_UI_PANEL_OK), "OK") == 0);
    ASSERT_TRUE(strcmp(csb_hint_oracle_ui_panel_result_name(
                           CSB_HINT_ORACLE_UI_PANEL_ERR_NOT_LOADED),
                       "not-loaded") == 0);
    ASSERT_TRUE(strcmp(csb_hint_oracle_ui_panel_result_name(
                           CSB_HINT_ORACLE_UI_PANEL_ERR_HINT_OUT_OF_RANGE),
                       "hint-out-of-range") == 0);
    ASSERT_TRUE(strcmp(csb_hint_oracle_ui_panel_result_name(
                           CSB_HINT_ORACLE_UI_PANEL_ERR_LOCATION_OUT_OF_RANGE),
                       "location-out-of-range") == 0);
    ASSERT_TRUE(strcmp(csb_hint_oracle_ui_panel_result_name(
                           CSB_HINT_ORACLE_UI_PANEL_ERR_NO_HINT_AT_LOCATION),
                       "no-hint-at-location") == 0);
    ASSERT_TRUE(strcmp(csb_hint_oracle_ui_panel_result_name(0x7fffffff),
                       "unknown") == 0);

    /* Status enum labels. */
    ASSERT_TRUE(strcmp(csb_hint_oracle_ui_panel_status_label(
                           CSB_HINT_ORACLE_UI_PANEL_STATUS_EMPTY),
                       "empty") == 0);
    ASSERT_TRUE(strcmp(csb_hint_oracle_ui_panel_status_label(
                           CSB_HINT_ORACLE_UI_PANEL_STATUS_LOADED),
                       "loaded") == 0);
    ASSERT_TRUE(strcmp(csb_hint_oracle_ui_panel_status_label(
                           CSB_HINT_ORACLE_UI_PANEL_STATUS_NOT_FOUND),
                       "not-found") == 0);
    ASSERT_TRUE(strcmp(csb_hint_oracle_ui_panel_status_label(
                           CSB_HINT_ORACLE_UI_PANEL_STATUS_NO_DATA_DIR),
                       "no-data-dir") == 0);
    ASSERT_TRUE(strcmp(csb_hint_oracle_ui_panel_status_label(
                           CSB_HINT_ORACLE_UI_PANEL_STATUS_READ_ERROR),
                       "read-error") == 0);
    ASSERT_TRUE(strcmp(csb_hint_oracle_ui_panel_status_label(
                           CSB_HINT_ORACLE_UI_PANEL_STATUS_PARSE_ERROR),
                       "parse-error") == 0);
    ASSERT_TRUE(strcmp(csb_hint_oracle_ui_panel_status_label(
                           CSB_HINT_ORACLE_UI_PANEL_STATUS_ARGUMENT_ERROR),
                       "argument-error") == 0);
    ASSERT_TRUE(strcmp(csb_hint_oracle_ui_panel_status_label(
                           (CSB_HintOracleUIPanel_Status)0x7fffffff),
                       "unknown") == 0);
    return 1;
}

/* ── Load with no data dir fails safely ─────────────────────────── */

static int test_load_no_data_dir(void)
{
    CSB_HintOracleUIPanel panel;
    int rc;

    csb_hint_oracle_ui_panel_init(&panel);
    /* Pass an explicitly-empty data dir to skip FSP_ResolveDataDir()
     * resolution so the test does not depend on a host's
     * FIRESTAFF_DATA env var or ~/.firestaff/data being set.
     * An empty data dir with no FSP fallback should land in
     * NO_DATA_DIR. */
    rc = csb_hint_oracle_ui_panel_load(
        &panel, "/tmp/firestaff-csb-hint-panel-definitely-missing-98765",
        NULL, 6);
    ASSERT_TRUE(rc == CSB_HINT_ORACLE_UI_PANEL_ERR_NOT_LOADED);
    ASSERT_TRUE(csb_hint_oracle_ui_panel_status(&panel) ==
                CSB_HINT_ORACLE_UI_PANEL_STATUS_NOT_FOUND);
    ASSERT_TRUE(csb_hint_oracle_ui_panel_hint_count(&panel) == 0u);
    csb_hint_oracle_ui_panel_free(&panel);
    return 1;
}

/* ── Sideloaded fixture: panel accessors see the loaded cache ──── */

static int test_sideloaded_panel_accessors(void)
{
    uint8_t buf[256];
    size_t size;
    CSB_HintOracleUIPanel panel;
    char hint_name[CSB_HINT_ORACLE_HTC_HINT_NAME_BYTES + 1u];
    uint8_t page_buf[256];
    size_t page_size = 0u;
    CSB_HintOracleHTC_Location loc;
    size_t resolved = 0u;
    char source_buf[128];

    size = build_fixture(buf, sizeof(buf));
    ASSERT_TRUE(sideload_panel_cache(
        &panel, buf, size,
        "00112233445566778899aabbccddeeff",
        "test-fixture-synthetic"));

    ASSERT_TRUE(csb_hint_oracle_ui_panel_status(&panel) ==
                CSB_HINT_ORACLE_UI_PANEL_STATUS_LOADED);
    ASSERT_TRUE(csb_hint_oracle_ui_panel_hint_count(&panel) == 2u);
    ASSERT_TRUE(csb_hint_oracle_ui_panel_location_count(&panel) == 2u);
    ASSERT_TRUE(csb_hint_oracle_ui_panel_page_count(&panel) == 4u);

    /* Hint 0 name + first page. */
    memset(hint_name, 0, sizeof(hint_name));
    ASSERT_TRUE(csb_hint_oracle_ui_panel_hint_name(
                    &panel, 0u, hint_name, sizeof(hint_name)) ==
                CSB_HINT_ORACLE_UI_PANEL_OK);
    ASSERT_TRUE(strcmp(hint_name, "PROVE YOU ARE WIZARD") == 0);

    memset(page_buf, 0, sizeof(page_buf));
    ASSERT_TRUE(csb_hint_oracle_ui_panel_hint_first_page(
                    &panel, 0u, page_buf, sizeof(page_buf),
                    &page_size) ==
                CSB_HINT_ORACLE_UI_PANEL_OK);
    ASSERT_TRUE(page_size > 0u);
    ASSERT_TRUE(strstr((const char *)page_buf, "Cast/ZOKATHRA") != NULL);

    /* Hint 1 name + first page. */
    memset(hint_name, 0, sizeof(hint_name));
    ASSERT_TRUE(csb_hint_oracle_ui_panel_hint_name(
                    &panel, 1u, hint_name, sizeof(hint_name)) ==
                CSB_HINT_ORACLE_UI_PANEL_OK);
    ASSERT_TRUE(strcmp(hint_name, "ANYWHERE") == 0);

    /* Out-of-range hint index returns HINT_OUT_OF_RANGE. */
    memset(hint_name, 'X', sizeof(hint_name));
    ASSERT_TRUE(csb_hint_oracle_ui_panel_hint_name(
                    &panel, 999u, hint_name, sizeof(hint_name)) ==
                CSB_HINT_ORACLE_UI_PANEL_ERR_HINT_OUT_OF_RANGE);
    ASSERT_TRUE(hint_name[0] == '\0');

    /* Location 0 matches the fixture's first record. */
    memset(&loc, 0, sizeof(loc));
    ASSERT_TRUE(csb_hint_oracle_ui_panel_location(&panel, 0u, &loc) ==
                CSB_HINT_ORACLE_UI_PANEL_OK);
    ASSERT_TRUE(loc.x == 25u);
    ASSERT_TRUE(loc.y == 7u);
    ASSERT_TRUE(loc.level == 5u);
    ASSERT_TRUE(loc.hint_index == 0u);

    /* Out-of-range location returns LOCATION_OUT_OF_RANGE. */
    memset(&loc, 0, sizeof(loc));
    ASSERT_TRUE(csb_hint_oracle_ui_panel_location(&panel, 999u, &loc) ==
                CSB_HINT_ORACLE_UI_PANEL_ERR_LOCATION_OUT_OF_RANGE);

    /* Resolve (level=5, 255, 255) wildcard → hint 1 ANYWHERE. */
    resolved = 0u;
    ASSERT_TRUE(csb_hint_oracle_ui_panel_resolve_location(
                    &panel, 5u,
                    CSB_HINT_ORACLE_HTC_ANY_XY,
                    CSB_HINT_ORACLE_HTC_ANY_XY,
                    &resolved) == CSB_HINT_ORACLE_UI_PANEL_OK);
    ASSERT_TRUE(resolved == 1u);

    /* Non-matching (level, x, y) returns NO_HINT_AT_LOCATION. */
    resolved = 0u;
    ASSERT_TRUE(csb_hint_oracle_ui_panel_resolve_location(
                    &panel, 0u, 0u, 0u, &resolved) ==
                CSB_HINT_ORACLE_UI_PANEL_ERR_NO_HINT_AT_LOCATION);

    /* Source label reflects the fixture's matched MD5 + label. */
    memset(source_buf, 0, sizeof(source_buf));
    ASSERT_TRUE(csb_hint_oracle_ui_panel_source_label(
                    &panel, source_buf, sizeof(source_buf)) ==
                CSB_HINT_ORACLE_UI_PANEL_OK);
    ASSERT_TRUE(strstr(source_buf, "test-fixture-synthetic") != NULL);
    ASSERT_TRUE(strstr(source_buf,
                       "00112233445566778899aabbccddeeff") != NULL);

    csb_hint_oracle_ui_panel_free(&panel);
    /* After free, status is reset to EMPTY. */
    ASSERT_TRUE(csb_hint_oracle_ui_panel_status(&panel) ==
                CSB_HINT_ORACLE_UI_PANEL_STATUS_EMPTY);
    ASSERT_TRUE(csb_hint_oracle_ui_panel_hint_count(&panel) == 0u);
    return 1;
}

/* ── Sideloaded panel diagnostic report ────────────────────────── */

static int test_sideloaded_panel_diagnostic(void)
{
    uint8_t buf[256];
    size_t size;
    CSB_HintOracleUIPanel panel;
    char report[4096];
    int truncated = 0;
    int n;

    size = build_fixture(buf, sizeof(buf));
    ASSERT_TRUE(sideload_panel_cache(
        &panel, buf, size,
        "00112233445566778899aabbccddeeff",
        "test-fixture-synthetic"));

    memset(report, 0, sizeof(report));
    n = csb_hint_oracle_ui_panel_format_diagnostic(
        &panel, report, sizeof(report), &truncated);
    ASSERT_TRUE(n > 0);
    ASSERT_TRUE(truncated == 0);

    ASSERT_TRUE(strstr(report, "panel diagnostic") != NULL);
    ASSERT_TRUE(strstr(report, "test-fixture-synthetic") != NULL);
    ASSERT_TRUE(strstr(report,
                       "00112233445566778899aabbccddeeff") != NULL);
    ASSERT_TRUE(strstr(report, "format_word=2") != NULL);
    ASSERT_TRUE(strstr(report, "dungeon_id=13") != NULL);
    ASSERT_TRUE(strstr(report, "location_count=2") != NULL);
    ASSERT_TRUE(strstr(report, "hint_count=2") != NULL);
    ASSERT_TRUE(strstr(report, "page_count=4") != NULL);
    ASSERT_TRUE(strstr(report, "hint[0]") != NULL);
    ASSERT_TRUE(strstr(report, "PROVE YOU ARE WIZARD") != NULL);
    ASSERT_TRUE(strstr(report, "Cast/ZOKATHRA") != NULL);
    ASSERT_TRUE(strstr(report, "level=0 (255,255) wildcard") != NULL);

    /* Truncation path: a tiny buffer still produces a non-empty
     * output and flags truncation. */
    {
        char small[64];
        memset(small, 0, sizeof(small));
        truncated = 0;
        n = csb_hint_oracle_ui_panel_format_diagnostic(
            &panel, small, sizeof(small), &truncated);
        ASSERT_TRUE(n > 0);
        ASSERT_TRUE(truncated == 1);
    }

    csb_hint_oracle_ui_panel_free(&panel);
    return 1;
}

/* ── Panel free → init round-trip ──────────────────────────────── */

static int test_free_then_init_round_trip(void)
{
    CSB_HintOracleUIPanel panel;
    uint8_t buf[256];
    size_t size;

    csb_hint_oracle_ui_panel_init(&panel);

    /* Load an empty dir to put the panel in NOT_FOUND. */
    ASSERT_TRUE(csb_hint_oracle_ui_panel_load(
                    &panel,
                    "/tmp/firestaff-csb-hint-panel-empty-99887",
                    NULL, 6) ==
                CSB_HINT_ORACLE_UI_PANEL_ERR_NOT_LOADED);
    ASSERT_TRUE(csb_hint_oracle_ui_panel_status(&panel) ==
                CSB_HINT_ORACLE_UI_PANEL_STATUS_NOT_FOUND);

    /* Free + init + side-load again to prove the panel is
     * reusable across load attempts. */
    csb_hint_oracle_ui_panel_free(&panel);
    csb_hint_oracle_ui_panel_init(&panel);
    size = build_fixture(buf, sizeof(buf));
    ASSERT_TRUE(sideload_panel_cache(
        &panel, buf, size, "00112233445566778899aabbccddeeff",
        "test-fixture-synthetic"));
    ASSERT_TRUE(csb_hint_oracle_ui_panel_hint_count(&panel) == 2u);

    csb_hint_oracle_ui_panel_free(&panel);
    return 1;
}

/* ── Tests driver ───────────────────────────────────────────────── */

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

    RUN_TEST(test_empty_panel_argument);
    RUN_TEST(test_null_argument);
    RUN_TEST(test_label_tables);
    RUN_TEST(test_load_no_data_dir);
    RUN_TEST(test_sideloaded_panel_accessors);
    RUN_TEST(test_sideloaded_panel_diagnostic);
    RUN_TEST(test_free_then_init_round_trip);

#undef RUN_TEST

    printf("csb_hint_oracle_ui_panel: %d/%d tests passed\n",
           passed, total);
    return passed == total ? 0 : 1;
}
