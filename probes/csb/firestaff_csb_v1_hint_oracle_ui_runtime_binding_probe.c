/*
 * firestaff_csb_v1_hint_oracle_ui_runtime_binding_probe.c
 *
 * Real-asset UI/runtime binding probe for the CSB Utility Disk
 * HCSB.HTC Hint Oracle.
 *
 * Source-lock boundary:
 *   - ReDMCSB HINTLOAD.C:11-18 names HCSB.HTC as the canonical
 *     CSB Utility Disk Hint Oracle content file.
 *   - ReDMCSB HINTHTC.C:177-358 validates the format 2 /
 *     dungeon 13 big-endian table the parser expects.
 *   - ReDMCSB HINTLZW.C:122-212 decompresses the page-content
 *     LZW stream the binding surface renders.
 *   - dmweb Hint Oracle Files page describes the same layout
 *     the parser and binding surface operate on.
 *
 * What this proves:
 *   - A decoded HCSB.HTC page (or page-slice) can reach a
 *     Firestaff-facing oracle/hint surface (a printable
 *     text/buffer that the launcher or M11 view can render).
 *   - The binding surface produces a non-empty diagnostic
 *     report that contains: matched MD5 + label, format
 *     word + dungeon id, documented variant/release/language
 *     metadata, location/hint/page counts, the hint 0
 *     first-page decoded text, and the wildcard (level=0,
 *     x=255, y=255) resolve.
 *   - The binding surface produces a non-empty
 *     hint-formatted text that contains: hint name + first
 *     page decoded text (rounded out with the same
 *     printable-ASCII contract the existing real-asset scan
 *     probe uses).
 *   - The (level, x, y) location resolver turns a wildcard
 *     lookup into a non-empty printable-ASCII hint name +
 *     first-page text.
 *   - Determinism: a second call after a fresh scan produces
 *     the same hint name + first-page text byte-for-byte.
 *
 * Skip-safe by design: when no known HCSB.HTC is present,
 * the probe exits 0 with a SKIP message so it does not
 * block hosts without CSB Utility Disk assets.
 */

#include "csb_hint_oracle_htc.h"
#include "csb_hint_oracle_htc_real_scan.h"
#include "csb_hint_oracle_htc_variant.h"
#include "csb_hint_oracle_ui_runtime_binding.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int g_checks;
static int g_failures;

#define CHECK(cond, msg) do {                                              \
    ++g_checks;                                                            \
    if (cond) {                                                            \
        printf("  PASS: %s\n", msg);                                       \
    } else {                                                               \
        ++g_failures;                                                      \
        printf("  FAIL: %s\n", msg);                                       \
    }                                                                      \
} while (0)

static const char *data_dir_arg(int argc, char **argv,
                                char *buf, size_t buf_size)
{
    const char *env;
    const char *home;

    if (argc > 1 && argv[1] && argv[1][0] != '\0') {
        return argv[1];
    }
    env = getenv("FIRESTAFF_CSB_HTC_DATA");
    if (env && env[0] != '\0') {
        return env;
    }
    env = getenv("FIRESTAFF_DATA_DIR");
    if (env && env[0] != '\0') {
        return env;
    }
    home = getenv("HOME");
    if (!home || home[0] == '\0') {
        return NULL;
    }
    snprintf(buf, buf_size, "%s/.firestaff/data", home);
    return buf;
}

static int is_printable_ascii(const uint8_t *buf, size_t len)
{
    size_t i;
    if (!buf || len == 0u) {
        return 0;
    }
    for (i = 0u; i < len; ++i) {
        unsigned char c = buf[i];
        if (c == '\r' || c == '\n' || c == '\t') {
            continue;
        }
        if (c < 0x20u || c > 0x7eu) {
            return 0;
        }
    }
    return 1;
}

static void print_ascii_preview(const char *buf, size_t max)
{
    size_t i;
    size_t len = buf ? strlen(buf) : 0u;
    size_t show = len < max ? len : max;
    for (i = 0u; i < show; ++i) {
        unsigned char c = (unsigned char)buf[i];
        if (c == '\r' || c == '\n' || c == '\t' ||
            (c >= 0x20u && c <= 0x7eu)) {
            putchar((int)c);
        } else {
            putchar('.');
        }
    }
}

int main(int argc, char **argv)
{
    char default_dir[1024];
    char hint_text[CSB_HINT_ORACLE_UI_BINDING_PAGE_CAP * 2];
    char report[4096];
    size_t known_count = 0u;
    const CSB_HintOracleHTC_RealKnownHash *known;
    size_t i;
    const char *dir;
    CSB_HintOracleHTC_RealCache cache;
    int rc;
    int n;
    int report_truncated = 0;
    int hint_truncated = 0;
    int saw_real_load = 0;
    size_t wildcard_hint_index = 0u;
    CSB_HintOracleHTC_Variant variant;

    printf("=== CSB V1 Hint Oracle UI/runtime binding probe ===\n\n");

    known = csb_hint_oracle_htc_real_known_hashes(&known_count);
    printf("known_hashes=%zu\n", known_count);
    for (i = 0u; i < known_count; ++i) {
        printf("  [%zu] %s  md5=%s  size=%zu\n",
               i, known[i].label, known[i].md5, known[i].size_bytes);
    }
    CHECK(known_count >= 1u,
          "at least one source-cited HCSB.HTC MD5 is registered");

    dir = data_dir_arg(argc, argv, default_dir, sizeof(default_dir));
    printf("data_dir=%s\n", dir ? dir : "(none)");

    csb_hint_oracle_htc_real_cache_init(&cache);

    rc = csb_hint_oracle_htc_real_scan_and_load(dir, NULL, 6, &cache);
    printf("scan_and_load rc=%d (%s)\n", rc,
           csb_hint_oracle_htc_real_result_name(rc));
    if (rc == CSB_HINT_ORACLE_HTC_REAL_ERR_NOT_FOUND) {
        printf("SKIP: no known HCSB.HTC found under data_dir; "
               "set FIRESTAFF_CSB_HTC_DATA to a directory containing "
               "a verified HCSB.HTC to enable this gate.\n");
        csb_hint_oracle_htc_real_cache_free(&cache);
        return 0;
    }
    if (rc != CSB_HINT_ORACLE_HTC_REAL_OK) {
        printf("FAIL: scan_and_load returned %d (%s); expected OK or "
               "NOT_FOUND.\n", rc,
               csb_hint_oracle_htc_real_result_name(rc));
        csb_hint_oracle_htc_real_cache_free(&cache);
        return 1;
    }
    saw_real_load = 1;

    printf("matched_md5=%s\n", cache.matched_md5);
    printf("matched_label=%s\n", cache.matched_label);
    printf("file_size=%zu\n", cache.file_size);
    variant = csb_hint_oracle_htc_variant_from_cache(&cache);
    printf("variant=%s release=%s language=%s\n",
           csb_hint_oracle_htc_variant_name(variant),
           csb_hint_oracle_htc_variant_release_name(variant),
           csb_hint_oracle_htc_variant_language(variant));

    /* ── Diagnostic / oracle report ────────────────────────────── */
    memset(report, 0, sizeof(report));
    n = csb_hint_oracle_ui_binding_format_report(
        &cache, report, sizeof(report), &report_truncated);
    CHECK(n > 0,
          "binding report formatter returns positive byte count");
    CHECK(report_truncated == 0,
          "binding report fits the default 4 KiB buffer");
    CHECK(strstr(report, "CSB Hint Oracle") != NULL,
          "binding report header is present");
    CHECK(strstr(report, cache.matched_md5) != NULL,
          "binding report surfaces the matched MD5");
    CHECK(strstr(report, cache.matched_label) != NULL,
          "binding report surfaces the matched label");
    CHECK(strstr(report, "format_word=2") != NULL,
          "binding report surfaces the format word");
    CHECK(strstr(report, "dungeon_id=13") != NULL,
          "binding report surfaces the dungeon id");
    CHECK(variant != CSB_HINT_ORACLE_HTC_VARIANT_UNKNOWN,
          "loaded verified HCSB.HTC classifies to a documented variant");
    CHECK(strstr(report, "variant=") != NULL &&
          strstr(report, csb_hint_oracle_htc_variant_name(variant)) != NULL,
          "binding report surfaces the documented variant name");
    CHECK(strstr(report, "variant_drift=match") != NULL,
          "binding report surfaces exact variant count/size drift match");
    CHECK(strstr(report, "language=") != NULL &&
          strstr(report, csb_hint_oracle_htc_variant_language(variant)) != NULL,
          "binding report surfaces the variant language tag");
    CHECK(strstr(report, "location_count=") != NULL &&
          strstr(report, "hint_count=") != NULL &&
          strstr(report, "page_count=") != NULL,
          "binding report surfaces location/hint/page counts");
    CHECK(strstr(report, "hint[0] (binding smoke)") != NULL,
          "binding report includes the hint 0 binding smoke header");
    /* The hint 0 binding smoke must include the parsed hint
     * name from the real-asset cache (proves the binding
     * surface reads the parser's hint table). */
    {
        char hint_name[CSB_HINT_ORACLE_HTC_HINT_NAME_BYTES + 1u];
        rc = csb_hint_oracle_htc_real_get_hint_name(
            &cache, 0u, hint_name, sizeof(hint_name));
        if (rc == CSB_HINT_ORACLE_HTC_REAL_OK && hint_name[0] != '\0') {
            char needle[64];
            snprintf(needle, sizeof(needle), "hint[0] %s", hint_name);
            CHECK(strstr(report, needle) != NULL,
                  "binding report includes the hint 0 name");
        }
    }
    CHECK(strstr(report, "level=0 (255,255) wildcard") != NULL,
          "binding report includes the wildcard smoke header");
    /* The wildcard resolve must surface a hint name (the
     * Atari ST 2.x file uses 255/255 for level-wide hints). */
    if (cache.htc.hint_count > 0u) {
        CHECK(strstr(report, "=== CSB Hint Oracle") != NULL,
              "binding report surfaces resolved hint header");
    }

    /* ── Single-hint formatter ─────────────────────────────────── */
    memset(hint_text, 0, sizeof(hint_text));
    n = csb_hint_oracle_ui_binding_format_hint(
        &cache, 0u, hint_text, sizeof(hint_text), &hint_truncated);
    CHECK(n > 0,
          "binding format_hint(0) returns positive byte count");
    CHECK(hint_truncated == 0,
          "binding format_hint(0) fits the default 4 KiB buffer");
    CHECK(strstr(hint_text, "hint[0]") != NULL,
          "binding format_hint(0) includes the hint index header");
    /* The hint text must contain the parsed name (real-asset
     * hint 0 = "FULYA PIT" on the Atari ST 2.x file). */
    {
        char hint_name[CSB_HINT_ORACLE_HTC_HINT_NAME_BYTES + 1u];
        rc = csb_hint_oracle_htc_real_get_hint_name(
            &cache, 0u, hint_name, sizeof(hint_name));
        if (rc == CSB_HINT_ORACLE_HTC_REAL_OK && hint_name[0] != '\0') {
            CHECK(strstr(hint_text, hint_name) != NULL,
                  "binding format_hint(0) includes the hint name");
        }
    }
    /* The first-page text must be printable ASCII (the LZW
     * decompressor round-trips the real CSB hint content). */
    {
        size_t body_off = 0u;
        size_t j;
        for (j = 0u; j < strlen(hint_text); ++j) {
            if (j > 0u && hint_text[j - 1u] == '\n' &&
                hint_text[j] != '\n') {
                body_off = j;
                break;
            }
        }
        if (body_off > 0u) {
            CHECK(is_printable_ascii((const uint8_t *)(hint_text + body_off),
                                     strlen(hint_text) - body_off),
                  "binding format_hint(0) first-page text is printable ASCII");
        }
    }
    printf("hint[0] binding text (first 96 bytes): \"");
    print_ascii_preview(hint_text, 96u);
    printf("\"\n");

    /* Out-of-range hint index returns HINT_OUT_OF_RANGE and
     * leaves the buffer empty. */
    {
        memset(hint_text, 'X', sizeof(hint_text));
        n = csb_hint_oracle_ui_binding_format_hint(
            &cache, 99999u, hint_text, sizeof(hint_text),
            &hint_truncated);
        CHECK(n == CSB_HINT_ORACLE_UI_BINDING_ERR_HINT_OUT_OF_RANGE,
              "binding format_hint with out-of-range index returns "
              "HINT_OUT_OF_RANGE");
        CHECK(hint_text[0] == '\0',
              "binding format_hint with out-of-range index leaves buffer empty");
    }

    /* ── (level, x, y) → hint binding resolver ─────────────────── */
    memset(hint_text, 0, sizeof(hint_text));
    n = csb_hint_oracle_ui_binding_resolve_location(
        &cache, 0u,
        CSB_HINT_ORACLE_HTC_ANY_XY, CSB_HINT_ORACLE_HTC_ANY_XY,
        &wildcard_hint_index,
        hint_text, sizeof(hint_text), &hint_truncated);
    CHECK(n > 0,
          "binding resolve_location(level=0, 255, 255) returns positive "
          "byte count (wildcard hint found)");
    CHECK(hint_truncated == 0,
          "binding resolve_location output fits the default buffer");
    CHECK(wildcard_hint_index < cache.htc.hint_count,
          "binding resolve_location returns a valid hint index");
    CHECK(strstr(hint_text, "CSB Hint Oracle") != NULL,
          "binding resolve_location output includes the hint header");
    /* The resolved hint's name must be present in the buffer. */
    if (wildcard_hint_index < cache.htc.hint_count) {
        char hint_name[CSB_HINT_ORACLE_HTC_HINT_NAME_BYTES + 1u];
        rc = csb_hint_oracle_htc_real_get_hint_name(
            &cache, wildcard_hint_index, hint_name, sizeof(hint_name));
        if (rc == CSB_HINT_ORACLE_HTC_REAL_OK && hint_name[0] != '\0') {
            char needle[64];
            snprintf(needle, sizeof(needle), "hint[%zu] %s",
                     wildcard_hint_index, hint_name);
            CHECK(strstr(hint_text, needle) != NULL,
                  "binding resolve_location output includes the resolved "
                  "hint name");
        }
    }
    printf("level=0 (255,255) binding text (first 96 bytes): \"");
    print_ascii_preview(hint_text, 96u);
    printf("\"\n");

    /* Snapshot the wildcard resolve output so the determinism
     * block can compare against a fresh second scan. */
    char wildcard_snapshot[CSB_HINT_ORACLE_UI_BINDING_PAGE_CAP * 2];
    {
        size_t wl = strlen(hint_text);
        if (wl >= sizeof(wildcard_snapshot)) {
            wl = sizeof(wildcard_snapshot) - 1u;
        }
        memcpy(wildcard_snapshot, hint_text, wl);
        wildcard_snapshot[wl] = '\0';
    }

    /* Snapshot the format_hint(0) output (already written to
     * hint_text before the wildcard block, but a fresh capture
     * keeps the determinism block self-contained). */
    char hint0_snapshot[CSB_HINT_ORACLE_UI_BINDING_PAGE_CAP * 2];
    {
        memset(hint_text, 0, sizeof(hint_text));
        n = csb_hint_oracle_ui_binding_format_hint(
            &cache, 0u, hint_text, sizeof(hint_text), &hint_truncated);
        if (n > 0) {
            size_t hl = strlen(hint_text);
            if (hl >= sizeof(hint0_snapshot)) {
                hl = sizeof(hint0_snapshot) - 1u;
            }
            memcpy(hint0_snapshot, hint_text, hl);
            hint0_snapshot[hl] = '\0';
        } else {
            hint0_snapshot[0] = '\0';
        }
    }

    /* A non-matching (level, x, y) returns
     * NO_HINT_AT_LOCATION and leaves the buffer empty. */
    {
        memset(hint_text, 'X', sizeof(hint_text));
        n = csb_hint_oracle_ui_binding_resolve_location(
            &cache, 0u, 0u, 0u,
            NULL, hint_text, sizeof(hint_text), &hint_truncated);
        CHECK(n == CSB_HINT_ORACLE_UI_BINDING_ERR_NO_HINT_AT_LOCATION,
              "binding resolve_location with non-matching tuple returns "
              "NO_HINT_AT_LOCATION");
        CHECK(hint_text[0] == '\0',
              "binding resolve_location with non-matching tuple leaves "
              "buffer empty");
    }

    /* ── Determinism: a second scan + binding reproduces the
     *    same hint name + first-page text byte-for-byte. ──────── */
    {
        CSB_HintOracleHTC_RealCache cache2;
        char hint_text2[CSB_HINT_ORACLE_UI_BINDING_PAGE_CAP * 2];
        int n2;
        size_t wildcard2 = 0u;

        csb_hint_oracle_htc_real_cache_init(&cache2);
        rc = csb_hint_oracle_htc_real_scan_and_load(
            dir, NULL, 6, &cache2);
        CHECK(rc == CSB_HINT_ORACLE_HTC_REAL_OK,
              "second scan_and_load succeeds (binding determinism gate)");
        if (rc == CSB_HINT_ORACLE_HTC_REAL_OK) {
            memset(hint_text2, 0, sizeof(hint_text2));
            n2 = csb_hint_oracle_ui_binding_format_hint(
                &cache2, 0u, hint_text2, sizeof(hint_text2),
                &hint_truncated);
            CHECK(n2 > 0 &&
                  strcmp(hint0_snapshot, hint_text2) == 0,
                  "second binding format_hint(0) is byte-identical to first");

            memset(hint_text2, 0, sizeof(hint_text2));
            n2 = csb_hint_oracle_ui_binding_resolve_location(
                &cache2, 0u,
                CSB_HINT_ORACLE_HTC_ANY_XY, CSB_HINT_ORACLE_HTC_ANY_XY,
                &wildcard2,
                hint_text2, sizeof(hint_text2), &hint_truncated);
            CHECK(n2 > 0 && wildcard2 == wildcard_hint_index &&
                  strcmp(wildcard_snapshot, hint_text2) == 0,
                  "second binding resolve_location is byte-identical to first");
        }
        csb_hint_oracle_htc_real_cache_free(&cache2);
    }

    /* Negative: empty data_dir (no discovery) returns
     * NOT_FOUND on the binding report side. */
    {
        CSB_HintOracleHTC_RealCache cache3;
        csb_hint_oracle_htc_real_cache_init(&cache3);
        rc = csb_hint_oracle_htc_real_scan_and_load(
            "/tmp/firestaff-htc-binding-definitely-missing-12345",
            NULL, 0, &cache3);
        CHECK(rc == CSB_HINT_ORACLE_HTC_REAL_ERR_NOT_FOUND,
              "scan with empty/invalid data_dir returns NOT_FOUND");

        memset(report, 0, sizeof(report));
        n = csb_hint_oracle_ui_binding_format_report(
            &cache3, report, sizeof(report), &report_truncated);
        CHECK(n > 0 && strstr(report, "not loaded") != NULL,
              "binding format_report on empty cache writes 'not loaded' "
              "marker");
        csb_hint_oracle_htc_real_cache_free(&cache3);
    }

    csb_hint_oracle_htc_real_cache_free(&cache);

    printf("\nchecks=%d failures=%d saw_real_load=%d\n",
           g_checks, g_failures, saw_real_load);
    return g_failures == 0 ? 0 : 1;
}
