/*
 * csb_hint_oracle_ui_panel.h
 *
 * Panel-shaped binding surface for the CSB Utility Disk
 * HCSB.HTC Hint Oracle.
 *
 * Scope of this module:
 *   - The smallest binding surface that an M12 launcher view
 *     (a "Hint Oracle panel") can call into to render an HCSB.HTC
 *     hint table directly from C. It owns the parsed-cache
 *     lifecycle (lazy scan + load + free) and exposes the
 *     hint list, location table, location resolver, and source
 *     metadata through small accessor functions shaped like a
 *     UI panel.
 *   - This is the next step up the stack from the text-format
 *     binding surface in include/csb_hint_oracle_ui_runtime_binding.h:
 *     that module produced printable text buffers that an M12
 *     popup could re-render verbatim. This module instead exposes
 *     the structured hint list / location table so a future
 *     launcher view (or a debug overlay) can render its own
 *     layout, scroller, or filter UI without re-parsing.
 *
 * What this module does:
 *   - Owns one CSB_HintOracleHTC_RealCache across calls so the
 *     launcher does not have to manage file buffers or the
 *     parser API directly.
 *   - Wraps csb_hint_oracle_htc_real_scan_and_load() so the
 *     panel only needs a data_dir (+ optional cache_dir) to
 *     come up.
 *   - Exposes status (loaded / not-loaded / not-found / read-
 *     error / parse-error), source label, hint count, location
 *     count, hint name, hint first-page text, location record,
 *     and a (level, x, y) → first hint resolver.
 *   - Exposes a multi-line diagnostic/oracle report identical
 *     in shape to the one in csb_hint_oracle_ui_runtime_binding.h
 *     so a single UI panel can either render the structured
 *     hint list or fall back to the textual report.
 *
 * What this module does NOT do:
 *   - It does not draw the Hint Oracle overlay. That is a
 *     separate, broader UI feature still tracked in
 *     docs/FIRESTAFF_GAP_LIST.md row "HTC hint oracle text
 *     format (CSB)".
 *   - It does not bind into the M11 game loop, M12 launch flow,
 *     or any savegame. It only proves the panel side of the
 *     binding surface; wiring into menu_startup_m12.c is a
 *     separate, future lane.
 *   - It does not claim parity for every Utility Disk release
 *     variant. The known-hash list in csb_hint_oracle_htc_real_*
 *     still gates which HCSB.HTCs we accept; the panel itself
 *     is variant-agnostic.
 *
 * Source references (shared with the upstream binding surface):
 *   - ReDMCSB HINTLOAD.C:11-18 names HCSB.HTC as the canonical
 *     CSB Utility Disk Hint Oracle content file.
 *   - ReDMCSB HINTHTC.C:177-358 validates the format 2 /
 *     dungeon 13 big-endian table the parser expects.
 *   - ReDMCSB HINTLZW.C:122-212 decompresses the page-content
 *     LZW stream the binding surface renders.
 *   - dmweb Hint Oracle Files page describes the same layout
 *     the parser and binding surface operate on.
 *
 * Skip-safe by design: when the supplied data_dir does not
 * resolve to a known HCSB.HTC, the panel stays empty (status
 * = CSB_HINT_ORACLE_UI_PANEL_STATUS_NOT_FOUND), accessor
 * counts return 0, and accessor writes return documented
 * negative result codes so callers can produce a non-NULL
 * output even on empty panels.
 */

#ifndef FIRESTAFF_CSB_HINT_ORACLE_UI_PANEL_H
#define FIRESTAFF_CSB_HINT_ORACLE_UI_PANEL_H

#include <stddef.h>
#include <stdint.h>

#include "csb_hint_oracle_htc.h"
#include "csb_hint_oracle_htc_real_scan.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ── Result + status enums ───────────────────────────────────────── */

typedef enum {
    CSB_HINT_ORACLE_UI_PANEL_OK = 0,
    CSB_HINT_ORACLE_UI_PANEL_ERR_ARGUMENT = -1,
    CSB_HINT_ORACLE_UI_PANEL_ERR_NOT_LOADED = -2,
    CSB_HINT_ORACLE_UI_PANEL_ERR_HINT_OUT_OF_RANGE = -3,
    CSB_HINT_ORACLE_UI_PANEL_ERR_LOCATION_OUT_OF_RANGE = -4,
    CSB_HINT_ORACLE_UI_PANEL_ERR_NO_HINT_AT_LOCATION = -5,
    CSB_HINT_ORACLE_UI_PANEL_ERR_OUTPUT_TOO_SMALL = -6,
    CSB_HINT_ORACLE_UI_PANEL_ERR_DECODE = -7
} CSB_HintOracleUIPanel_Result;

typedef enum {
    /* No load attempt has been made yet (panel just initialized). */
    CSB_HINT_ORACLE_UI_PANEL_STATUS_EMPTY = 0,
    /* Last load succeeded; the panel owns a parsed cache. */
    CSB_HINT_ORACLE_UI_PANEL_STATUS_LOADED,
    /* Last load failed because no known HCSB.HTC was found in
     * the data tree. The launcher should treat this as a SKIP
     * (no panel content to render). */
    CSB_HINT_ORACLE_UI_PANEL_STATUS_NOT_FOUND,
    /* Last load failed because the data dir was empty or missing. */
    CSB_HINT_ORACLE_UI_PANEL_STATUS_NO_DATA_DIR,
    /* Last load failed because the matched virtual path could
     * not be materialized. */
    CSB_HINT_ORACLE_UI_PANEL_STATUS_READ_ERROR,
    /* Last load failed because the matched file did not parse
     * as a valid HCSB.HTC (rejected by csb_hint_oracle_htc_parse). */
    CSB_HINT_ORACLE_UI_PANEL_STATUS_PARSE_ERROR,
    /* A load argument was malformed (NULL cache, NULL data_dir
     * with no fallback). */
    CSB_HINT_ORACLE_UI_PANEL_STATUS_ARGUMENT_ERROR
} CSB_HintOracleUIPanel_Status;

/* ── Panel state ─────────────────────────────────────────────────── */

#define CSB_HINT_ORACLE_UI_PANEL_LABEL_CAP   128
#define CSB_HINT_ORACLE_UI_PANEL_PATH_CAP    512
#define CSB_HINT_ORACLE_UI_PANEL_MD5_CAP     33

typedef struct {
    /* Owned cache. Valid between csb_hint_oracle_ui_panel_load()
     * success and the next load / free / destroy. */
    CSB_HintOracleHTC_RealCache cache;

    /* Last load status — drives the panel UI's header + the
     * accessors' return codes. Sticky: a NOT_FOUND from a
     * previous load attempt still reports NOT_FOUND until a
     * new load attempt is made. */
    CSB_HintOracleUIPanel_Status status;

    /* Last load diagnostic (raw result code from
     * csb_hint_oracle_htc_real_scan_and_load). 0 on a successful
     * load, otherwise the documented negative code. */
    int last_load_rc;

    /* Last load attempt's data dir (resolved through FSP). Used
     * so the launcher's diagnostic surface can show "scanned
     * <data_dir>" without re-resolving. */
    char last_data_dir[CSB_HINT_ORACLE_UI_PANEL_PATH_CAP];

    /* Number of times csb_hint_oracle_ui_panel_load() has been
     * called successfully. Lets the launcher test that a
     * refresh did or did not actually re-load. */
    int load_count;
} CSB_HintOracleUIPanel;

/* Initialize an empty panel (no allocation, no file I/O). */
void csb_hint_oracle_ui_panel_init(CSB_HintOracleUIPanel *panel);

/* Free the owned cache + reset the panel in place. Safe to
 * call multiple times. */
void csb_hint_oracle_ui_panel_free(CSB_HintOracleUIPanel *panel);

/* Lazy-load an HCSB.HTC into the panel from the user's data
 * tree. Replaces any previously loaded cache.
 *
 *   - `data_dir`: optional override; when NULL or empty, falls
 *     back to FSP_ResolveDataDir() (FIRESTAFF_DATA env var or
 *     ~/.firestaff/data default).
 *   - `cache_dir`: optional virtual-container materialization
 *     root (matches csb_hint_oracle_htc_real_scan_and_load).
 *   - `max_depth`: scanner depth; pass 6 (or any positive) to
 *     recursively descend ZIP/ISO entries.
 *
 * Returns CSB_HINT_ORACLE_UI_PANEL_OK on a successful load.
 * Returns CSB_HINT_ORACLE_UI_PANEL_ERR_NOT_FOUND when no
 * known HCSB.HTC was present (the launcher should treat this
 * as a SKIP — the panel stays empty and accessors return
 * NOT_LOADED / 0). Returns CSB_HINT_ORACLE_UI_PANEL_ERR_ARGUMENT
 * on a NULL panel.
 *
 * The function also updates panel->status and panel->last_load_rc
 * so the UI can render a status line without re-querying the
 * loader.
 */
int csb_hint_oracle_ui_panel_load(CSB_HintOracleUIPanel *panel,
                                  const char *data_dir,
                                  const char *cache_dir,
                                  int max_depth);

/* Return the current status. */
CSB_HintOracleUIPanel_Status
csb_hint_oracle_ui_panel_status(const CSB_HintOracleUIPanel *panel);

/* Render a single-line status label (e.g. "loaded", "not-found",
 * "no-data-dir", "read-error", "parse-error", "empty",
 * "argument-error") suitable for an M12 header line. The
 * returned pointer is to a static string table and remains
 * valid for the lifetime of the program. */
const char *
csb_hint_oracle_ui_panel_status_label(CSB_HintOracleUIPanel_Status status);

/* Source-citation metadata for the currently loaded HCSB.HTC.
 * `buf` receives a printable line of the shape:
 *
 *   "<matched_label> md5=<matched_md5> <size_bytes>B"
 *
 * Returns CSB_HINT_ORACLE_UI_PANEL_OK, or
 * CSB_HINT_ORACLE_UI_PANEL_ERR_NOT_LOADED, or
 * CSB_HINT_ORACLE_UI_PANEL_ERR_ARGUMENT, or
 * CSB_HINT_ORACLE_UI_PANEL_ERR_OUTPUT_TOO_SMALL when `buf_size`
 * is too small (in which case a truncated NUL-terminated line
 * is still written).
 */
int csb_hint_oracle_ui_panel_source_label(
    const CSB_HintOracleUIPanel *panel,
    char *buf, size_t buf_size);

/* Number of hints (0 if not loaded). */
size_t csb_hint_oracle_ui_panel_hint_count(const CSB_HintOracleUIPanel *panel);

/* Number of locations (0 if not loaded). */
size_t csb_hint_oracle_ui_panel_location_count(const CSB_HintOracleUIPanel *panel);

/* Number of pages (0 if not loaded). */
size_t csb_hint_oracle_ui_panel_page_count(const CSB_HintOracleUIPanel *panel);

/* Number of compressed content bytes (0 if not loaded). */
size_t csb_hint_oracle_ui_panel_content_size(const CSB_HintOracleUIPanel *panel);

/* Copy the hint name for `hint_index` into `buf` (NUL-terminated).
 * Returns CSB_HINT_ORACLE_UI_PANEL_OK, or
 * CSB_HINT_ORACLE_UI_PANEL_ERR_NOT_LOADED, or
 * CSB_HINT_ORACLE_UI_PANEL_ERR_HINT_OUT_OF_RANGE, or
 * CSB_HINT_ORACLE_UI_PANEL_ERR_ARGUMENT, or
 * CSB_HINT_ORACLE_UI_PANEL_ERR_OUTPUT_TOO_SMALL. */
int csb_hint_oracle_ui_panel_hint_name(
    const CSB_HintOracleUIPanel *panel,
    size_t hint_index,
    char *buf, size_t buf_size);

/* Decompress hint `hint_index`'s first page into `out_buf`
 * (capacity `out_capacity`). `*out_size` is set to the
 * decompressed byte count.
 *
 * Returns CSB_HINT_ORACLE_UI_PANEL_OK, or
 * CSB_HINT_ORACLE_UI_PANEL_ERR_NOT_LOADED, or
 * CSB_HINT_ORACLE_UI_PANEL_ERR_HINT_OUT_OF_RANGE, or
 * CSB_HINT_ORACLE_UI_PANEL_ERR_ARGUMENT, or
 * CSB_HINT_ORACLE_UI_PANEL_ERR_DECODE.
 */
int csb_hint_oracle_ui_panel_hint_first_page(
    const CSB_HintOracleUIPanel *panel,
    size_t hint_index,
    uint8_t *out_buf,
    size_t out_capacity,
    size_t *out_size);

/* Get a location record by index. Returns
 * CSB_HINT_ORACLE_UI_PANEL_OK on success. */
int csb_hint_oracle_ui_panel_location(
    const CSB_HintOracleUIPanel *panel,
    size_t location_index,
    CSB_HintOracleHTC_Location *out_location);

/* Resolve a (level, x, y) location into the first matching
 * hint index. `out_hint_index` is set on success.
 *
 * Returns CSB_HINT_ORACLE_UI_PANEL_OK when at least one hint
 * matched. Returns CSB_HINT_ORACLE_UI_PANEL_ERR_NO_HINT_AT_LOCATION
 * when no hint matches the (level, x, y) tuple.
 * Returns CSB_HINT_ORACLE_UI_PANEL_ERR_NOT_LOADED when the
 * panel is empty. Returns CSB_HINT_ORACLE_UI_PANEL_ERR_ARGUMENT
 * when the panel or out_hint_index pointer is NULL.
 */
int csb_hint_oracle_ui_panel_resolve_location(
    const CSB_HintOracleUIPanel *panel,
    uint8_t level,
    uint8_t x,
    uint8_t y,
    size_t *out_hint_index);

/* Format a multi-line diagnostic/oracle report. The output is
 * the same shape as csb_hint_oracle_ui_binding_format_report():
 * a header, source-citation block, format/dungeon/header
 * summary, location/hint/page/content counts, the hint 0
 * binding smoke, and the (level=0, x=255, y=255) wildcard
 * resolve smoke.
 *
 * When the panel is not loaded, the report writes a one-line
 * "(not loaded)" marker and returns CSB_HINT_ORACLE_UI_PANEL_OK
 * with `*out_was_truncated` set to 0.
 *
 * Returns the number of bytes written (excl. the trailing NUL),
 * a negative CSB_HintOracleUIPanel_Result on argument errors,
 * or 0 when `buf_size` is too small to fit even the not-loaded
 * marker. `*out_was_truncated` is set to 1 if the buffer could
 * not hold the full report (may be NULL).
 */
int csb_hint_oracle_ui_panel_format_diagnostic(
    const CSB_HintOracleUIPanel *panel,
    char *buf, size_t buf_size,
    int *out_was_truncated);

/* Result-name table (mirrors the result enum). */
const char *csb_hint_oracle_ui_panel_result_name(int result);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CSB_HINT_ORACLE_UI_PANEL_H */
