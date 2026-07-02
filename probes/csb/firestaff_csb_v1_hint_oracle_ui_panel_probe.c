/*
 * firestaff_csb_v1_hint_oracle_ui_panel_probe.c
 *
 * Real-asset UI/runtime panel probe for the CSB Utility Disk
 * HCSB.HTC Hint Oracle.
 *
 * Source-lock boundary:
 *   - ReDMCSB HINTLOAD.C:11-18 names HCSB.HTC as the canonical
 *     CSB Utility Disk Hint Oracle content file.
 *   - ReDMCSB HINTHTC.C:177-358 validates the format 2 /
 *     dungeon 13 big-endian table the parser expects.
 *   - ReDMCSB HINTLZW.C:122-212 decompresses the page-content
 *     LZW stream the panel reads back.
 *   - dmweb Hint Oracle Files page describes the same layout
 *     the parser and panel surface operate on.
 *
 * What this proves:
 *   - The panel-shaped UI binding surface (the layer an M12
 *     Hint Oracle view would consume) can read a verified
 *     HCSB.HTC end-to-end through csb_hint_oracle_ui_panel_*:
 *     lazy scan + load + status + hint count + hint name +
 *     hint first-page decode + location record + (level, x, y)
 *     → hint index resolver.
 *   - The panel can be reloaded on a fresh scan and produces
 *     the same hint name + first-page text byte-for-byte
 *     (deterministic binding gate).
 *   - The diagnostic report (a panel-side counterpart to the
 *     upstream binding report) carries the matched MD5/label,
 *     the format/dungeon/header summary, the location/hint/
 *     page counts, the hint 0 binding smoke, and the wildcard
 *     (level=0, x=255, y=255) resolve smoke.
 *
 * Skip-safe by design: when no known HCSB.HTC is present, the
 * probe exits 0 with a SKIP message so it does not block
 * hosts without CSB Utility Disk assets.
 */

#include "csb_hint_oracle_htc.h"
#include "csb_hint_oracle_htc_real_scan.h"
#include "csb_hint_oracle_ui_panel.h"
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
    size_t known_count = 0u;
    const CSB_HintOracleHTC_RealKnownHash *known;
    size_t i;
    const char *dir;
    CSB_HintOracleUIPanel panel;
    int rc;
    size_t hint_count;
    size_t location_count;
    size_t page_count;
    size_t content_size;
    char source_buf[CSB_HINT_ORACLE_UI_PANEL_LABEL_CAP +
                    CSB_HINT_ORACLE_UI_PANEL_MD5_CAP + 64];
    char hint_name[CSB_HINT_ORACLE_HTC_HINT_NAME_BYTES + 1u];
    uint8_t page_buf[CSB_HINT_ORACLE_UI_BINDING_PAGE_CAP];
    size_t page_size = 0u;
    CSB_HintOracleHTC_Location loc;
    size_t resolved_index = 0u;
    char report[4096];
    int report_truncated = 0;
    int saw_real_load = 0;

    printf("=== CSB V1 Hint Oracle UI/runtime panel probe ===\n\n");

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

    csb_hint_oracle_ui_panel_init(&panel);

    rc = csb_hint_oracle_ui_panel_load(&panel, dir, NULL, 6);
    printf("panel_load rc=%d (%s)\n", rc,
           csb_hint_oracle_ui_panel_result_name(rc));
    printf("panel_status=%s last_load_rc=%d\n",
           csb_hint_oracle_ui_panel_status_label(
               csb_hint_oracle_ui_panel_status(&panel)),
           panel.last_load_rc);
    if (rc == CSB_HINT_ORACLE_UI_PANEL_ERR_NOT_LOADED &&
        csb_hint_oracle_ui_panel_status(&panel) ==
            CSB_HINT_ORACLE_UI_PANEL_STATUS_NOT_FOUND) {
        printf("SKIP: no known HCSB.HTC found under data_dir; "
               "set FIRESTAFF_CSB_HTC_DATA to a directory containing "
               "a verified HCSB.HTC to enable this gate.\n");
        csb_hint_oracle_ui_panel_free(&panel);
        return 0;
    }
    if (rc != CSB_HINT_ORACLE_UI_PANEL_OK) {
        printf("FAIL: panel_load returned %d (%s); expected OK or "
               "NOT_FOUND.\n", rc,
               csb_hint_oracle_ui_panel_result_name(rc));
        csb_hint_oracle_ui_panel_free(&panel);
        return 1;
    }
    saw_real_load = 1;

    /* ── Counts ────────────────────────────────────────────────── */
    hint_count = csb_hint_oracle_ui_panel_hint_count(&panel);
    location_count = csb_hint_oracle_ui_panel_location_count(&panel);
    page_count = csb_hint_oracle_ui_panel_page_count(&panel);
    content_size = csb_hint_oracle_ui_panel_content_size(&panel);
    CHECK(hint_count > 0u,
          "panel hint_count > 0 after a real load");
    CHECK(location_count > 0u,
          "panel location_count > 0 after a real load");
    CHECK(page_count > 0u,
          "panel page_count > 0 after a real load");
    CHECK(content_size > 0u,
          "panel content_size > 0 after a real load");
    printf("counts: hints=%zu locations=%zu pages=%zu content=%zu\n",
           hint_count, location_count, page_count, content_size);

    /* ── Source label ─────────────────────────────────────────── */
    memset(source_buf, 0, sizeof(source_buf));
    rc = csb_hint_oracle_ui_panel_source_label(
        &panel, source_buf, sizeof(source_buf));
    CHECK(rc == CSB_HINT_ORACLE_UI_PANEL_OK,
          "panel source_label returns OK after a real load");
    CHECK(strstr(source_buf, panel.cache.matched_md5) != NULL,
          "panel source_label surfaces the matched MD5");
    CHECK(strstr(source_buf, panel.cache.matched_label) != NULL,
          "panel source_label surfaces the matched label");
    printf("source_label=%s\n", source_buf);

    /* ── Hint 0 name + first page ────────────────────────────── */
    memset(hint_name, 0, sizeof(hint_name));
    rc = csb_hint_oracle_ui_panel_hint_name(
        &panel, 0u, hint_name, sizeof(hint_name));
    CHECK(rc == CSB_HINT_ORACLE_UI_PANEL_OK,
          "panel hint_name(0) returns OK");
    CHECK(hint_name[0] != '\0',
          "panel hint_name(0) returns a non-empty hint name");
    printf("hint[0] name=\"%s\"\n", hint_name);

    memset(page_buf, 0, sizeof(page_buf));
    page_size = 0u;
    rc = csb_hint_oracle_ui_panel_hint_first_page(
        &panel, 0u, page_buf, sizeof(page_buf), &page_size);
    CHECK(rc == CSB_HINT_ORACLE_UI_PANEL_OK,
          "panel hint_first_page(0) returns OK");
    CHECK(page_size > 0u,
          "panel hint_first_page(0) returns > 0 bytes");
    /* The first page is the prefix of the decompressed buffer
     * up to the first NUL separator (CSB hint pages are
     * separated by NULs in the page-content stream). The
     * trailing bytes after the first NUL belong to subsequent
     * pages and may carry graphic/font data, so we only assert
     * that the FIRST page is printable ASCII (same contract the
     * upstream binding surface uses on its formatted text). */
    {
        size_t first_page_size = 0u;
        size_t k;
        for (k = 0u; k < page_size; ++k) {
            if (page_buf[k] == 0u) {
                break;
            }
            ++first_page_size;
        }
        if (first_page_size == 0u && page_size > 0u) {
            first_page_size = page_size;
        }
        CHECK(is_printable_ascii(page_buf, first_page_size),
              "panel hint_first_page(0) first-page text is printable ASCII");
    }
    printf("hint[0] first-page text (first 96 bytes): \"");
    print_ascii_preview((const char *)page_buf, 96u);
    printf("\"\n");

    /* ── Location 0 ──────────────────────────────────────────── */
    memset(&loc, 0, sizeof(loc));
    rc = csb_hint_oracle_ui_panel_location(&panel, 0u, &loc);
    CHECK(rc == CSB_HINT_ORACLE_UI_PANEL_OK,
          "panel location(0) returns OK");
    CHECK(loc.hint_index < hint_count,
          "panel location(0) hint_index is in range");

    /* Out-of-range location returns LOCATION_OUT_OF_RANGE. */
    memset(&loc, 0, sizeof(loc));
    rc = csb_hint_oracle_ui_panel_location(&panel, hint_count + 9999u, &loc);
    CHECK(rc == CSB_HINT_ORACLE_UI_PANEL_ERR_LOCATION_OUT_OF_RANGE,
          "panel location with out-of-range index returns "
          "LOCATION_OUT_OF_RANGE");

    /* ── Wildcard resolve ────────────────────────────────────── */
    resolved_index = 0u;
    rc = csb_hint_oracle_ui_panel_resolve_location(
        &panel, 0u,
        CSB_HINT_ORACLE_HTC_ANY_XY, CSB_HINT_ORACLE_HTC_ANY_XY,
        &resolved_index);
    CHECK(rc == CSB_HINT_ORACLE_UI_PANEL_OK,
          "panel resolve_location(level=0, 255, 255) returns OK");
    CHECK(resolved_index < hint_count,
          "panel resolve_location returns a valid hint index");

    /* Snapshot the resolved hint's name + first-page text so
     * the determinism block can compare against a fresh scan. */
    char resolved_name_snapshot[CSB_HINT_ORACLE_HTC_HINT_NAME_BYTES + 1u];
    uint8_t resolved_page_snapshot[CSB_HINT_ORACLE_UI_BINDING_PAGE_CAP];
    size_t resolved_page_size_snapshot = 0u;
    if (rc == CSB_HINT_ORACLE_UI_PANEL_OK) {
        csb_hint_oracle_ui_panel_hint_name(
            &panel, resolved_index, resolved_name_snapshot,
            sizeof(resolved_name_snapshot));
        csb_hint_oracle_ui_panel_hint_first_page(
            &panel, resolved_index, resolved_page_snapshot,
            sizeof(resolved_page_snapshot),
            &resolved_page_size_snapshot);
    } else {
        resolved_name_snapshot[0] = '\0';
        resolved_page_snapshot[0] = '\0';
    }

    /* Snapshot the hint 0 first-page text + name for the same
     * reason. */
    char hint0_name_snapshot[CSB_HINT_ORACLE_HTC_HINT_NAME_BYTES + 1u];
    uint8_t hint0_page_snapshot[CSB_HINT_ORACLE_UI_BINDING_PAGE_CAP];
    size_t hint0_page_size_snapshot = page_size;
    {
        size_t hl = strlen(hint_name);
        if (hl >= sizeof(hint0_name_snapshot)) {
            hl = sizeof(hint0_name_snapshot) - 1u;
        }
        memcpy(hint0_name_snapshot, hint_name, hl);
        hint0_name_snapshot[hl] = '\0';
        memcpy(hint0_page_snapshot, page_buf,
               page_size < sizeof(hint0_page_snapshot)
                   ? page_size
                   : sizeof(hint0_page_snapshot));
    }

    /* Non-matching tuple returns NO_HINT_AT_LOCATION. */
    rc = csb_hint_oracle_ui_panel_resolve_location(
        &panel, 0u, 0u, 0u, NULL);
    CHECK(rc == CSB_HINT_ORACLE_UI_PANEL_ERR_NO_HINT_AT_LOCATION,
          "panel resolve_location with non-matching tuple returns "
          "NO_HINT_AT_LOCATION");

    /* ── Diagnostic report ───────────────────────────────────── */
    memset(report, 0, sizeof(report));
    rc = csb_hint_oracle_ui_panel_format_diagnostic(
        &panel, report, sizeof(report), &report_truncated);
    CHECK(rc > 0,
          "panel diagnostic report returns positive byte count");
    CHECK(report_truncated == 0,
          "panel diagnostic report fits the default 4 KiB buffer");
    CHECK(strstr(report, "panel diagnostic") != NULL,
          "panel diagnostic report surfaces its own header");
    CHECK(strstr(report, panel.cache.matched_md5) != NULL,
          "panel diagnostic report surfaces the matched MD5");
    CHECK(strstr(report, panel.cache.matched_label) != NULL,
          "panel diagnostic report surfaces the matched label");
    CHECK(strstr(report, "format_word=2") != NULL,
          "panel diagnostic report surfaces the format word");
    CHECK(strstr(report, "dungeon_id=13") != NULL,
          "panel diagnostic report surfaces the dungeon id");
    CHECK(strstr(report, "location_count=") != NULL &&
          strstr(report, "hint_count=") != NULL &&
          strstr(report, "page_count=") != NULL,
          "panel diagnostic report surfaces the parsed counts");
    CHECK(strstr(report, "hint[0]") != NULL,
          "panel diagnostic report includes the hint 0 binding smoke");
    CHECK(strstr(report, "level=0 (255,255) wildcard") != NULL,
          "panel diagnostic report includes the wildcard smoke header");

    /* ── Determinism: a second load reproduces the same hint
     *    name + first-page text + wildcard resolve byte-for-byte. */
    {
        CSB_HintOracleUIPanel panel2;
        char hint_name2[CSB_HINT_ORACLE_HTC_HINT_NAME_BYTES + 1u];
        uint8_t page2[CSB_HINT_ORACLE_UI_BINDING_PAGE_CAP];
        size_t page2_size = 0u;
        size_t resolved2 = 0u;

        csb_hint_oracle_ui_panel_init(&panel2);
        rc = csb_hint_oracle_ui_panel_load(&panel2, dir, NULL, 6);
        CHECK(rc == CSB_HINT_ORACLE_UI_PANEL_OK,
              "second panel_load succeeds (panel determinism gate)");

        memset(hint_name2, 0, sizeof(hint_name2));
        csb_hint_oracle_ui_panel_hint_name(
            &panel2, 0u, hint_name2, sizeof(hint_name2));
        memset(page2, 0, sizeof(page2));
        csb_hint_oracle_ui_panel_hint_first_page(
            &panel2, 0u, page2, sizeof(page2), &page2_size);
        CHECK(strcmp(hint0_name_snapshot, hint_name2) == 0 &&
              hint0_page_size_snapshot == page2_size &&
              memcmp(hint0_page_snapshot, page2,
                     page2_size < sizeof(page2)
                         ? page2_size
                         : sizeof(page2)) == 0,
              "second panel hint[0] name + first-page is byte-identical "
              "to first");

        rc = csb_hint_oracle_ui_panel_resolve_location(
            &panel2, 0u,
            CSB_HINT_ORACLE_HTC_ANY_XY, CSB_HINT_ORACLE_HTC_ANY_XY,
            &resolved2);
        if (rc == CSB_HINT_ORACLE_UI_PANEL_OK) {
            char rname2[CSB_HINT_ORACLE_HTC_HINT_NAME_BYTES + 1u];
            uint8_t rpage2[CSB_HINT_ORACLE_UI_BINDING_PAGE_CAP];
            size_t rpage2_size = 0u;
            csb_hint_oracle_ui_panel_hint_name(
                &panel2, resolved2, rname2, sizeof(rname2));
            csb_hint_oracle_ui_panel_hint_first_page(
                &panel2, resolved2, rpage2, sizeof(rpage2),
                &rpage2_size);
            CHECK(resolved2 == resolved_index &&
                  strcmp(resolved_name_snapshot, rname2) == 0 &&
                  resolved_page_size_snapshot == rpage2_size &&
                  memcmp(resolved_page_snapshot, rpage2,
                         rpage2_size < sizeof(rpage2)
                             ? rpage2_size
                             : sizeof(rpage2)) == 0,
                  "second panel wildcard resolve is byte-identical "
                  "to first");
        } else {
            ++g_failures;
            printf("  FAIL: second panel wildcard resolve rc=%d "
                   "(expected OK)\n", rc);
        }

        /* The reload also bumps panel.load_count on panel2. */
        CHECK(panel2.load_count == 1,
              "panel.load_count is 1 after the second load");

        csb_hint_oracle_ui_panel_free(&panel2);
    }

    /* Negative: a third load against an empty data dir flips
     * the status back to NOT_FOUND. */
    {
        CSB_HintOracleUIPanel panel3;
        csb_hint_oracle_ui_panel_init(&panel3);
        rc = csb_hint_oracle_ui_panel_load(
            &panel3,
            "/tmp/firestaff-htc-panel-definitely-missing-54321",
            NULL, 0);
        CHECK(rc == CSB_HINT_ORACLE_UI_PANEL_ERR_NOT_LOADED,
              "panel.load with empty/invalid data_dir returns "
              "NOT_LOADED");
        CHECK(csb_hint_oracle_ui_panel_status(&panel3) ==
              CSB_HINT_ORACLE_UI_PANEL_STATUS_NOT_FOUND,
              "panel_status is NOT_FOUND after a failed load");

        memset(report, 0, sizeof(report));
        rc = csb_hint_oracle_ui_panel_format_diagnostic(
            &panel3, report, sizeof(report), &report_truncated);
        CHECK(rc > 0 && strstr(report, "not-found") != NULL,
              "panel diagnostic report on empty panel writes "
              "'not-found' marker");
        csb_hint_oracle_ui_panel_free(&panel3);
    }

    csb_hint_oracle_ui_panel_free(&panel);

    printf("\nchecks=%d failures=%d saw_real_load=%d\n",
           g_checks, g_failures, saw_real_load);
    return g_failures == 0 ? 0 : 1;
}
