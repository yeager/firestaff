/*
 * csb_hint_oracle_ui_panel.c
 *
 * Panel-shaped binding surface for the CSB Utility Disk
 * HCSB.HTC Hint Oracle. See include/csb_hint_oracle_ui_panel.h
 * for scope, source references, and non-claims.
 *
 * Implementation strategy:
 *
 *   - The panel owns one CSB_HintOracleHTC_RealCache across
 *     calls (allocated/freed through the existing
 *     csb_hint_oracle_htc_real_cache_init/_free helpers, which
 *     in turn own the file buffer and the parsed view).
 *
 *   - csb_hint_oracle_ui_panel_load() forwards to the existing
 *     csb_hint_oracle_htc_real_scan_and_load() so hash-based
 *     discovery, virtual-container materialization, and parser
 *     validation all stay in the existing module. The panel
 *     just owns the lifecycle and translates result codes into
 *     a sticky Status enum the UI can render without re-reading
 *     the scan code.
 *
 *   - The accessor functions are deliberately small and are
 *     thin wrappers over the existing csb_hint_oracle_htc_*
 *     + csb_hint_oracle_htc_real_* APIs so we do not duplicate
 *     any parse / LZW / location-lookup logic. The goal of
 *     this module is to prove the panel surface itself is
 *     callable from a launcher view.
 *
 *   - The diagnostic report is a strict subset of the existing
 *     csb_hint_oracle_ui_binding_format_report() surface so a
 *     single launcher popup can use either the structured
 *     panel accessors or the preformatted text buffer.
 */

#include "csb_hint_oracle_ui_panel.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "csb_hint_oracle_ui_runtime_binding.h"
#include "fs_portable_compat.h"

/* ── Status-name + result-name tables ────────────────────────────── */

const char *csb_hint_oracle_ui_panel_status_label(
    CSB_HintOracleUIPanel_Status status)
{
    switch (status) {
    case CSB_HINT_ORACLE_UI_PANEL_STATUS_EMPTY:            return "empty";
    case CSB_HINT_ORACLE_UI_PANEL_STATUS_LOADED:           return "loaded";
    case CSB_HINT_ORACLE_UI_PANEL_STATUS_NOT_FOUND:        return "not-found";
    case CSB_HINT_ORACLE_UI_PANEL_STATUS_NO_DATA_DIR:      return "no-data-dir";
    case CSB_HINT_ORACLE_UI_PANEL_STATUS_READ_ERROR:       return "read-error";
    case CSB_HINT_ORACLE_UI_PANEL_STATUS_PARSE_ERROR:      return "parse-error";
    case CSB_HINT_ORACLE_UI_PANEL_STATUS_ARGUMENT_ERROR:   return "argument-error";
    default:                                               return "unknown";
    }
}

const char *csb_hint_oracle_ui_panel_result_name(int result)
{
    switch (result) {
    case CSB_HINT_ORACLE_UI_PANEL_OK:                          return "OK";
    case CSB_HINT_ORACLE_UI_PANEL_ERR_ARGUMENT:                return "argument";
    case CSB_HINT_ORACLE_UI_PANEL_ERR_NOT_LOADED:              return "not-loaded";
    case CSB_HINT_ORACLE_UI_PANEL_ERR_HINT_OUT_OF_RANGE:       return "hint-out-of-range";
    case CSB_HINT_ORACLE_UI_PANEL_ERR_LOCATION_OUT_OF_RANGE:   return "location-out-of-range";
    case CSB_HINT_ORACLE_UI_PANEL_ERR_NO_HINT_AT_LOCATION:     return "no-hint-at-location";
    case CSB_HINT_ORACLE_UI_PANEL_ERR_OUTPUT_TOO_SMALL:        return "output-too-small";
    case CSB_HINT_ORACLE_UI_PANEL_ERR_DECODE:                  return "decode";
    default:                                                   return "unknown";
    }
}

/* Translate the upstream CSB_HintOracleHTC_RealResult code into
 * our sticky Status enum so the UI can render a header line
 * without re-reading the result enum. */
static CSB_HintOracleUIPanel_Status
translate_load_status(int rc)
{
    switch (rc) {
    case CSB_HINT_ORACLE_HTC_REAL_OK:               return CSB_HINT_ORACLE_UI_PANEL_STATUS_LOADED;
    case CSB_HINT_ORACLE_HTC_REAL_ERR_NOT_FOUND:    return CSB_HINT_ORACLE_UI_PANEL_STATUS_NOT_FOUND;
    case CSB_HINT_ORACLE_HTC_REAL_ERR_NO_DATA_DIR:  return CSB_HINT_ORACLE_UI_PANEL_STATUS_NO_DATA_DIR;
    case CSB_HINT_ORACLE_HTC_REAL_ERR_READ:         return CSB_HINT_ORACLE_UI_PANEL_STATUS_READ_ERROR;
    case CSB_HINT_ORACLE_HTC_REAL_ERR_PARSE:        return CSB_HINT_ORACLE_UI_PANEL_STATUS_PARSE_ERROR;
    case CSB_HINT_ORACLE_HTC_REAL_ERR_ARGUMENT:     return CSB_HINT_ORACLE_UI_PANEL_STATUS_ARGUMENT_ERROR;
    default:                                        return CSB_HINT_ORACLE_UI_PANEL_STATUS_ARGUMENT_ERROR;
    }
}

/* ── Lifecycle ───────────────────────────────────────────────────── */

void csb_hint_oracle_ui_panel_init(CSB_HintOracleUIPanel *panel)
{
    if (!panel) {
        return;
    }
    csb_hint_oracle_htc_real_cache_init(&panel->cache);
    panel->status = CSB_HINT_ORACLE_UI_PANEL_STATUS_EMPTY;
    panel->last_load_rc = 0;
    panel->last_data_dir[0] = '\0';
    panel->load_count = 0;
}

void csb_hint_oracle_ui_panel_free(CSB_HintOracleUIPanel *panel)
{
    if (!panel) {
        return;
    }
    csb_hint_oracle_htc_real_cache_free(&panel->cache);
    /* Re-initialize so the panel is reusable after free. */
    panel->status = CSB_HINT_ORACLE_UI_PANEL_STATUS_EMPTY;
    panel->last_load_rc = 0;
    panel->last_data_dir[0] = '\0';
    panel->load_count = 0;
}

/* ── Load (lazy scan + cache) ────────────────────────────────────── */

int csb_hint_oracle_ui_panel_load(CSB_HintOracleUIPanel *panel,
                                  const char *data_dir,
                                  const char *cache_dir,
                                  int max_depth)
{
    char resolved_data_dir[CSB_HINT_ORACLE_UI_PANEL_PATH_CAP];
    const char *search_dir;
    int rc;
    int cache_max_depth;

    if (!panel) {
        return CSB_HINT_ORACLE_UI_PANEL_ERR_ARGUMENT;
    }

    /* Reset the owned cache so a failed reload never leaves
     * a stale partially-loaded file buffer behind. */
    csb_hint_oracle_htc_real_cache_free(&panel->cache);
    panel->status = CSB_HINT_ORACLE_UI_PANEL_STATUS_EMPTY;
    panel->last_load_rc = 0;

    if (data_dir && data_dir[0] != '\0') {
        search_dir = data_dir;
    } else {
        if (!FSP_ResolveDataDir(resolved_data_dir,
                                sizeof(resolved_data_dir), NULL)) {
            panel->status = CSB_HINT_ORACLE_UI_PANEL_STATUS_NO_DATA_DIR;
            panel->last_load_rc = CSB_HINT_ORACLE_HTC_REAL_ERR_NO_DATA_DIR;
            return CSB_HINT_ORACLE_UI_PANEL_ERR_NOT_LOADED;
        }
        search_dir = resolved_data_dir;
    }

    /* Track the resolved search root so the diagnostic report
     * can show it. The buffer is large enough for any path
     * FSP_ResolveDataDir or a caller-supplied data_dir can
     * produce; if a too-long path is supplied, we silently
     * truncate the diagnostic copy but keep the load. */
    {
        size_t i;
        for (i = 0u; i + 1u < sizeof(panel->last_data_dir); ++i) {
            char c = search_dir[i];
            panel->last_data_dir[i] = c;
            if (c == '\0') {
                break;
            }
        }
        panel->last_data_dir[sizeof(panel->last_data_dir) - 1u] = '\0';
    }

    cache_max_depth = max_depth > 0 ? max_depth : 6;

    rc = csb_hint_oracle_htc_real_scan_and_load(
        search_dir, cache_dir, cache_max_depth, &panel->cache);
    panel->last_load_rc = rc;
    panel->status = translate_load_status(rc);

    if (rc == CSB_HINT_ORACLE_HTC_REAL_OK) {
        panel->load_count += 1;
        return CSB_HINT_ORACLE_UI_PANEL_OK;
    }
    /* Make sure the cache stays cleared on failure (the
     * underlying _scan_and_load() contract is that the cache
     * is only populated on OK; we double-free to be safe). */
    csb_hint_oracle_htc_real_cache_free(&panel->cache);
    panel->cache.loaded = 0;

    if (rc == CSB_HINT_ORACLE_HTC_REAL_ERR_NOT_FOUND) {
        return CSB_HINT_ORACLE_UI_PANEL_ERR_NOT_LOADED;
    }
    /* Any other negative rc leaves the panel empty and the
     * UI can read panel->status to render an explanation. */
    return CSB_HINT_ORACLE_UI_PANEL_ERR_NOT_LOADED;
}

/* ── Status + source label ───────────────────────────────────────── */

CSB_HintOracleUIPanel_Status
csb_hint_oracle_ui_panel_status(const CSB_HintOracleUIPanel *panel)
{
    if (!panel) {
        return CSB_HINT_ORACLE_UI_PANEL_STATUS_ARGUMENT_ERROR;
    }
    return panel->status;
}

int csb_hint_oracle_ui_panel_source_label(
    const CSB_HintOracleUIPanel *panel,
    char *buf, size_t buf_size)
{
    if (!buf || buf_size == 0u) {
        return CSB_HINT_ORACLE_UI_PANEL_ERR_ARGUMENT;
    }
    if (!panel) {
        buf[0] = '\0';
        return CSB_HINT_ORACLE_UI_PANEL_ERR_ARGUMENT;
    }
    if (panel->status != CSB_HINT_ORACLE_UI_PANEL_STATUS_LOADED) {
        buf[0] = '\0';
        return CSB_HINT_ORACLE_UI_PANEL_ERR_NOT_LOADED;
    }
    /* Format: "<label> md5=<md5> <size>B" (matches the
     * csb_hint_oracle_ui_binding_format_report() header so
     * a launcher can show either surface with one render
     * routine). */
    {
        const char *label = panel->cache.matched_label[0]
                                 ? panel->cache.matched_label
                                 : "(unknown)";
        const char *md5 = panel->cache.matched_md5[0]
                              ? panel->cache.matched_md5
                              : "(unknown)";
        int n = snprintf(buf, buf_size, "%s md5=%s %zuB",
                         label, md5, panel->cache.file_size);
        if (n < 0) {
            buf[0] = '\0';
            return CSB_HINT_ORACLE_UI_PANEL_ERR_OUTPUT_TOO_SMALL;
        }
        if ((size_t)n >= buf_size) {
            /* snprintf already NUL-terminated. */
            return CSB_HINT_ORACLE_UI_PANEL_ERR_OUTPUT_TOO_SMALL;
        }
        return CSB_HINT_ORACLE_UI_PANEL_OK;
    }
}

/* ── Counts ──────────────────────────────────────────────────────── */

size_t csb_hint_oracle_ui_panel_hint_count(const CSB_HintOracleUIPanel *panel)
{
    if (!panel || panel->status != CSB_HINT_ORACLE_UI_PANEL_STATUS_LOADED) {
        return 0u;
    }
    return panel->cache.htc.hint_count;
}

size_t csb_hint_oracle_ui_panel_location_count(const CSB_HintOracleUIPanel *panel)
{
    if (!panel || panel->status != CSB_HINT_ORACLE_UI_PANEL_STATUS_LOADED) {
        return 0u;
    }
    return panel->cache.htc.location_count;
}

size_t csb_hint_oracle_ui_panel_page_count(const CSB_HintOracleUIPanel *panel)
{
    if (!panel || panel->status != CSB_HINT_ORACLE_UI_PANEL_STATUS_LOADED) {
        return 0u;
    }
    return panel->cache.htc.page_count;
}

size_t csb_hint_oracle_ui_panel_content_size(const CSB_HintOracleUIPanel *panel)
{
    if (!panel || panel->status != CSB_HINT_ORACLE_UI_PANEL_STATUS_LOADED) {
        return 0u;
    }
    return panel->cache.htc.content_size;
}

/* ── Hint + location accessors ───────────────────────────────────── */

int csb_hint_oracle_ui_panel_hint_name(
    const CSB_HintOracleUIPanel *panel,
    size_t hint_index,
    char *buf, size_t buf_size)
{
    int rc;
    if (!buf || buf_size == 0u) {
        return CSB_HINT_ORACLE_UI_PANEL_ERR_ARGUMENT;
    }
    if (!panel) {
        buf[0] = '\0';
        return CSB_HINT_ORACLE_UI_PANEL_ERR_ARGUMENT;
    }
    if (panel->status != CSB_HINT_ORACLE_UI_PANEL_STATUS_LOADED) {
        buf[0] = '\0';
        return CSB_HINT_ORACLE_UI_PANEL_ERR_NOT_LOADED;
    }
    /* Pre-check the index against the parsed hint count so we
     * can return the documented HINT_OUT_OF_RANGE code (the
     * upstream real_get_hint_name collapses out-of-range +
     * argument + not-loaded into a single NOT_LOADED result). */
    if (hint_index >= panel->cache.htc.hint_count) {
        buf[0] = '\0';
        return CSB_HINT_ORACLE_UI_PANEL_ERR_HINT_OUT_OF_RANGE;
    }
    rc = csb_hint_oracle_htc_real_get_hint_name(
        &panel->cache, hint_index, buf, buf_size);
    switch (rc) {
    case CSB_HINT_ORACLE_HTC_REAL_OK:
        return CSB_HINT_ORACLE_UI_PANEL_OK;
    case CSB_HINT_ORACLE_HTC_REAL_ERR_OUTPUT_TOO_SMALL:
        return CSB_HINT_ORACLE_UI_PANEL_ERR_OUTPUT_TOO_SMALL;
    case CSB_HINT_ORACLE_HTC_REAL_ERR_ARGUMENT:
        buf[0] = '\0';
        return CSB_HINT_ORACLE_UI_PANEL_ERR_ARGUMENT;
    default:
        buf[0] = '\0';
        return CSB_HINT_ORACLE_UI_PANEL_ERR_NOT_LOADED;
    }
}

int csb_hint_oracle_ui_panel_hint_first_page(
    const CSB_HintOracleUIPanel *panel,
    size_t hint_index,
    uint8_t *out_buf,
    size_t out_capacity,
    size_t *out_size)
{
    int rc;
    if (!out_buf || out_capacity == 0u || !out_size) {
        if (out_size) *out_size = 0u;
        return CSB_HINT_ORACLE_UI_PANEL_ERR_ARGUMENT;
    }
    if (!panel) {
        *out_size = 0u;
        return CSB_HINT_ORACLE_UI_PANEL_ERR_ARGUMENT;
    }
    if (panel->status != CSB_HINT_ORACLE_UI_PANEL_STATUS_LOADED) {
        *out_size = 0u;
        return CSB_HINT_ORACLE_UI_PANEL_ERR_NOT_LOADED;
    }
    if (hint_index >= panel->cache.htc.hint_count) {
        *out_size = 0u;
        return CSB_HINT_ORACLE_UI_PANEL_ERR_HINT_OUT_OF_RANGE;
    }
    rc = csb_hint_oracle_htc_real_decompress_first_page(
        &panel->cache, hint_index, out_buf, out_capacity, out_size);
    switch (rc) {
    case CSB_HINT_ORACLE_HTC_REAL_OK:
        return CSB_HINT_ORACLE_UI_PANEL_OK;
    case CSB_HINT_ORACLE_HTC_REAL_ERR_OUTPUT_TOO_SMALL:
        return CSB_HINT_ORACLE_UI_PANEL_ERR_OUTPUT_TOO_SMALL;
    case CSB_HINT_ORACLE_HTC_REAL_ERR_NOT_LOADED:
        *out_size = 0u;
        return CSB_HINT_ORACLE_UI_PANEL_ERR_NOT_LOADED;
    case CSB_HINT_ORACLE_HTC_REAL_ERR_ARGUMENT:
        *out_size = 0u;
        return CSB_HINT_ORACLE_UI_PANEL_ERR_ARGUMENT;
    default:
        *out_size = 0u;
        return CSB_HINT_ORACLE_UI_PANEL_ERR_DECODE;
    }
}

int csb_hint_oracle_ui_panel_location(
    const CSB_HintOracleUIPanel *panel,
    size_t location_index,
    CSB_HintOracleHTC_Location *out_location)
{
    if (!panel || !out_location) {
        return CSB_HINT_ORACLE_UI_PANEL_ERR_ARGUMENT;
    }
    if (panel->status != CSB_HINT_ORACLE_UI_PANEL_STATUS_LOADED) {
        return CSB_HINT_ORACLE_UI_PANEL_ERR_NOT_LOADED;
    }
    return csb_hint_oracle_htc_get_location(
        &panel->cache.htc, location_index, out_location) == 0
        ? CSB_HINT_ORACLE_UI_PANEL_OK
        : CSB_HINT_ORACLE_UI_PANEL_ERR_LOCATION_OUT_OF_RANGE;
}

int csb_hint_oracle_ui_panel_resolve_location(
    const CSB_HintOracleUIPanel *panel,
    uint8_t level,
    uint8_t x,
    uint8_t y,
    size_t *out_hint_index)
{
    uint16_t indices[8];
    size_t count = 0u;
    int rc;

    if (!panel) {
        return CSB_HINT_ORACLE_UI_PANEL_ERR_ARGUMENT;
    }
    if (panel->status != CSB_HINT_ORACLE_UI_PANEL_STATUS_LOADED) {
        return CSB_HINT_ORACLE_UI_PANEL_ERR_NOT_LOADED;
    }
    rc = csb_hint_oracle_htc_real_find_hints_for_location(
        &panel->cache, level, x, y,
        indices, sizeof(indices) / sizeof(indices[0]), &count);
    if (rc == CSB_HINT_ORACLE_HTC_REAL_ERR_NOT_LOADED) {
        return CSB_HINT_ORACLE_UI_PANEL_ERR_NOT_LOADED;
    }
    if (rc != CSB_HINT_ORACLE_HTC_REAL_OK || count == 0u) {
        return CSB_HINT_ORACLE_UI_PANEL_ERR_NO_HINT_AT_LOCATION;
    }
    if (out_hint_index) {
        *out_hint_index = (size_t)indices[0];
    }
    return CSB_HINT_ORACLE_UI_PANEL_OK;
}

/* ── Diagnostic / oracle report (subset of the existing binding
 *    report — uses panel->cache directly to keep the surface
 *    in one place). ─────────────────────────────────────────────── */

/* Small append helpers (mirrors the upstream binding module
 * so a launcher can compare the two reports side by side). */

static int append_str(char *buf, size_t buf_size,
                      size_t *in_out_written,
                      const char *src)
{
    size_t written;
    size_t remain;
    size_t src_len;
    int n;

    if (!buf || buf_size == 0u || !in_out_written || !src) {
        return 0;
    }
    written = *in_out_written;
    if (written >= buf_size) {
        return 0;
    }
    remain = buf_size - written;
    src_len = strlen(src);
    if (src_len == 0u) {
        return 1;
    }
    n = snprintf(buf + written, remain, "%s", src);
    if (n < 0 || (size_t)n >= remain) {
        *in_out_written = buf_size;
        return 0;
    }
    *in_out_written = written + (size_t)n;
    return 1;
}

static int append_format(char *buf, size_t buf_size,
                         size_t *in_out_written,
                         const char *fmt, ...)
{
    int n;
    va_list ap;
    size_t written;
    size_t remain;

    if (!buf || buf_size == 0u || !in_out_written || !fmt) {
        return 0;
    }
    written = *in_out_written;
    if (written >= buf_size) {
        return 0;
    }
    remain = buf_size - written;
    va_start(ap, fmt);
    n = vsnprintf(buf + written, remain, fmt, ap);
    va_end(ap);
    if (n < 0 || (size_t)n >= remain) {
        *in_out_written = buf_size;
        return 0;
    }
    *in_out_written = written + (size_t)n;
    return 1;
}

static void append_hint_text_line(char *buf, size_t buf_size,
                                  size_t *in_out_written,
                                  const uint8_t *text, size_t len)
{
    size_t i;
    size_t content_end = 0u;
    if (!text || len == 0u) {
        return;
    }
    for (i = 0u; i < len; ++i) {
        if (text[i] == 0u) {
            content_end = i;
            break;
        }
    }
    if (content_end == 0u) {
        content_end = len;
    }
    for (i = 0u; i < content_end; ++i) {
        unsigned char c = text[i];
        if (c == '\r') {
            continue;
        }
        if (c == '\n' || c == '\t') {
            append_str(buf, buf_size, in_out_written,
                       c == '\n' ? "\n" : " ");
            continue;
        }
        if (c < 0x20u || c > 0x7eu) {
            append_str(buf, buf_size, in_out_written, ".");
            continue;
        }
        {
            char tmp[2];
            tmp[0] = (char)c;
            tmp[1] = '\0';
            append_str(buf, buf_size, in_out_written, tmp);
        }
    }
}

/* Append the first decoded page of hint `hint_index` to `buf`.
 * Bounded to CSB_HINT_ORACLE_UI_BINDING_PAGE_CAP bytes (the
 * upstream binding module's contract). */
static void append_hint_first_page(char *buf, size_t buf_size,
                                   size_t *in_out_written,
                                   const CSB_HintOracleUIPanel *panel,
                                   size_t hint_index)
{
    uint8_t page_buf[CSB_HINT_ORACLE_UI_BINDING_PAGE_CAP];
    size_t page_size = 0u;
    char hint_name[CSB_HINT_ORACLE_HTC_HINT_NAME_BYTES + 1u];

    if (csb_hint_oracle_ui_panel_hint_name(panel, hint_index,
                                           hint_name,
                                           sizeof(hint_name)) !=
        CSB_HINT_ORACLE_UI_PANEL_OK) {
        append_str(buf, buf_size, in_out_written, "(unreadable hint name)\n");
        return;
    }
    if (csb_hint_oracle_ui_panel_hint_first_page(
            panel, hint_index, page_buf, sizeof(page_buf),
            &page_size) != CSB_HINT_ORACLE_UI_PANEL_OK) {
        append_format(buf, buf_size, in_out_written,
                      "(hint %zu \"%s\" decode failed)\n",
                      hint_index, hint_name);
        return;
    }
    if (page_size >= sizeof(page_buf)) {
        page_size = sizeof(page_buf) - 1u;
    }
    page_buf[page_size] = '\0';
    append_format(buf, buf_size, in_out_written,
                  "--- hint[%zu] %s ---\n", hint_index, hint_name);
    append_hint_text_line(buf, buf_size, in_out_written,
                          page_buf, page_size);
    append_str(buf, buf_size, in_out_written, "\n");
}

int csb_hint_oracle_ui_panel_format_diagnostic(
    const CSB_HintOracleUIPanel *panel,
    char *buf, size_t buf_size,
    int *out_was_truncated)
{
    size_t written = 0u;
    int full = 1;
    int n;

    if (out_was_truncated) {
        *out_was_truncated = 0;
    }
    if (!buf || buf_size == 0u) {
        return CSB_HINT_ORACLE_UI_PANEL_ERR_ARGUMENT;
    }

    if (!panel || panel->status != CSB_HINT_ORACLE_UI_PANEL_STATUS_LOADED) {
        const char *status_label = csb_hint_oracle_ui_panel_status_label(
            panel ? panel->status
                  : CSB_HINT_ORACLE_UI_PANEL_STATUS_EMPTY);
        full &= append_format(buf, buf_size, &written,
                              "(CSB Hint Oracle panel: %s)\n",
                              status_label);
        if (panel && panel->last_data_dir[0] != '\0') {
            full &= append_format(buf, buf_size, &written,
                                  "scanned=%s rc=%d\n",
                                  panel->last_data_dir,
                                  panel->last_load_rc);
        } else if (panel) {
            full &= append_format(buf, buf_size, &written,
                                  "rc=%d (%s)\n",
                                  panel->last_load_rc,
                                  csb_hint_oracle_htc_real_result_name(
                                      panel->last_load_rc));
        }
        if (written < buf_size) {
            buf[written] = '\0';
        } else if (buf_size > 0u) {
            buf[buf_size - 1u] = '\0';
            full = 0;
        }
        if (!full && out_was_truncated) {
            *out_was_truncated = 1;
        }
        return (int)written;
    }

    /* Loaded: render the same shape as the upstream binding
     * report so a launcher popup can use either surface. */
    {
        char source_buf[CSB_HINT_ORACLE_UI_PANEL_LABEL_CAP +
                        CSB_HINT_ORACLE_UI_PANEL_MD5_CAP + 64];
        full &= append_str(buf, buf_size, &written,
                           "=== CSB Hint Oracle — panel diagnostic ===\n");
        if (csb_hint_oracle_ui_panel_source_label(
                panel, source_buf, sizeof(source_buf)) ==
            CSB_HINT_ORACLE_UI_PANEL_OK) {
            full &= append_format(buf, buf_size, &written,
                                  "source=%s\n", source_buf);
        }
        full &= append_format(buf, buf_size, &written,
                              "format_word=%u dungeon_id=%u\n",
                              (unsigned)panel->cache.htc.format_word,
                              (unsigned)panel->cache.htc.dungeon_id);
        full &= append_format(buf, buf_size, &written,
                              "location_count=%zu hint_count=%zu "
                              "page_count=%zu content_size=%zu\n",
                              panel->cache.htc.location_count,
                              panel->cache.htc.hint_count,
                              panel->cache.htc.page_count,
                              panel->cache.htc.content_size);

        if (panel->cache.htc.hint_count > 0u) {
            append_hint_first_page(buf, buf_size, &written,
                                   panel, 0u);
        }

        /* Wildcard resolve smoke. */
        {
            size_t resolved_index = 0u;
            int rr = csb_hint_oracle_ui_panel_resolve_location(
                panel, 0u,
                CSB_HINT_ORACLE_HTC_ANY_XY, CSB_HINT_ORACLE_HTC_ANY_XY,
                &resolved_index);
            full &= append_str(buf, buf_size, &written,
                               "--- level=0 (255,255) wildcard "
                               "(binding smoke) ---\n");
            if (rr == CSB_HINT_ORACLE_UI_PANEL_OK) {
                if (resolved_index < panel->cache.htc.hint_count) {
                    append_hint_first_page(buf, buf_size, &written,
                                           panel, resolved_index);
                } else {
                    full &= append_format(buf, buf_size, &written,
                                          "(wildcard resolved hint %zu, "
                                          "but hint_count=%zu)\n",
                                          resolved_index,
                                          panel->cache.htc.hint_count);
                }
            } else {
                full &= append_format(buf, buf_size, &written,
                                      "(level 0 wildcard resolve rc=%d "
                                      "%s)\n",
                                      rr,
                                      csb_hint_oracle_ui_panel_result_name(
                                          rr));
            }
        }
    }

    if (written < buf_size) {
        buf[written] = '\0';
    } else if (buf_size > 0u) {
        buf[buf_size - 1u] = '\0';
        full = 0;
    }
    if (!full && out_was_truncated) {
        *out_was_truncated = 1;
    }
    n = (int)written;
    return n;
}
