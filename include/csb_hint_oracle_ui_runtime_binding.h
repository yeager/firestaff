/*
 * csb_hint_oracle_ui_runtime_binding.h
 *
 * Runtime-adjacent binding surface for the CSB Utility Disk
 * HCSB.HTC Hint Oracle. This is the smallest module that proves
 * a decoded HCSB.HTC page or page-slice can reach a Firestaff-
 * facing oracle/hint surface (a printable text/hint-name buffer
 * the launcher, M11 view, or a future overlay can read).
 *
 * It is intentionally a thin layer on top of the existing
 * format-contract parser (include/csb_hint_oracle_htc.h) and
 * the real-asset scan/cache (include/csb_hint_oracle_htc_real_scan.h):
 *
 *   - ReDMCSB HINTHTC.C:177-358 is the source of truth for what
 *     the HCSB.HTC location/hint/page tables mean;
 *   - ReDMCSB HINTLZW.C:122-212 is the source of truth for the
 *     LZW decompression that produces printable hint text;
 *   - csb_hint_oracle_htc_variant.h owns release/language
 *     metadata so English/French/German variant expansion stays
 *     separate from this runtime-facing text surface;
 *   - dmweb's "Hint Oracle Files" page describes the same shape.
 *
 * What this module does:
 *   - Format a multi-line diagnostic/oracle report (matched MD5,
 *     label, variant metadata, location/hint/page counts, file
 *     size, resolved path) that any M12/M13 popup or status
 *     panel can show.
 *   - Format a single hint's name + first-page decoded text into
 *     a fixed bounded buffer (the "hint page slice" the task asks
 *     for).
 *   - Resolve a (level, x, y) into the first matching hint's
 *     name + first-page decoded text (the "binding" between
 *     the location table and a Firestaff-facing hint surface).
 *
 * What this module does NOT do:
 *   - It does not draw a graphical Hint Oracle overlay. That is
 *     a separate, broader UI feature still tracked in
 *     docs/FIRESTAFF_GAP_LIST.md row "HTC hint oracle text
 *     format (CSB)".
 *   - It does not claim parity for every Utility Disk release
 *     variant. The known-hash list in csb_hint_oracle_htc_real_*
 *     still gates which HCSB.HTCs we accept; the binding surface
 *     itself is variant-agnostic.
 *   - It does not bind into the M11 game loop, M12 launch flow,
 *     or any savegame. It only proves the text-side path.
 *
 * Skip-safe by design: when the supplied cache is empty (no
 * real HCSB.HTC was loaded) the binding helpers return
 * CSB_HINT_ORACLE_UI_BINDING_ERR_NOT_LOADED, and the
 * diagnostic/oracle report writes a one-line "not loaded"
 * marker so callers can still produce a non-NULL output.
 */

#ifndef FIRESTAFF_CSB_HINT_ORACLE_UI_RUNTIME_BINDING_H
#define FIRESTAFF_CSB_HINT_ORACLE_UI_RUNTIME_BINDING_H

#include <stddef.h>
#include <stdint.h>

#include "csb_hint_oracle_htc.h"
#include "csb_hint_oracle_htc_real_scan.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    CSB_HINT_ORACLE_UI_BINDING_OK = 0,
    CSB_HINT_ORACLE_UI_BINDING_ERR_ARGUMENT = -1,
    CSB_HINT_ORACLE_UI_BINDING_ERR_NOT_LOADED = -2,
    CSB_HINT_ORACLE_UI_BINDING_ERR_HINT_OUT_OF_RANGE = -3,
    CSB_HINT_ORACLE_UI_BINDING_ERR_NO_HINT_AT_LOCATION = -4,
    CSB_HINT_ORACLE_UI_BINDING_ERR_OUTPUT_TOO_SMALL = -5,
    CSB_HINT_ORACLE_UI_BINDING_ERR_DECODE = -6
} CSB_HintOracleUIBinding_Result;

/* Maximum bytes of decoded hint-page text we surface in a single
 * call (incl. trailing NUL). The original CSB Hint Oracle pages
 * are short prose (the local Atari ST 2.x HCSB.HTC decompresses
 * to a few hundred bytes per page); 2048 is enough to carry the
 * full first page and a meaningful excerpt of a longer page. */
#define CSB_HINT_ORACLE_UI_BINDING_PAGE_CAP 2048u

/* Format a multi-line diagnostic/oracle report into `buf`.
 *
 *   - If `cache` is NULL, the report writes "(not loaded)" and
 *     returns OK with one line written.
 *   - If `cache->loaded == 0`, the report writes "(not loaded)"
 *     and returns OK with one line written.
 *   - If `cache` is loaded, the report contains: header line,
 *     matched MD5, matched label, variant/release/language
 *     metadata from the separate variant classifier, file size,
 *     resolved path, original virtual path, location/hint/page
 *     counts, format word, dungeon id, and the first wildcard
 *     lookup result (the (level, x, y) tuple and the first
 *     matching hint name + first-page text excerpt).
 *
 * The first-page text is the same surface a Firestaff Hint
 * Oracle overlay would render, so seeing it in the report is
 * the binding gate between the parsed HCSB.HTC and a Firestaff-
 * facing oracle surface.
 *
 * Returns the number of bytes written (excl. the trailing NUL),
 * 0 on argument error, or -1 on truncation. `*out_was_truncated`
 * is set to 1 if the buffer could not hold the full report
 * (callers can decide whether to re-allocate or simply show
 * what fits).
 */
int csb_hint_oracle_ui_binding_format_report(
    const CSB_HintOracleHTC_RealCache *cache,
    char *buf,
    size_t buf_size,
    int *out_was_truncated);

/* Format a single hint's name + first-page decoded text into
 * `buf`. The output is two lines: a header line with the hint
 * name + index, and the hint text (truncated to fit `buf_size`).
 *
 * The first-page text is the same surface a Firestaff Hint
 * Oracle overlay would render, so the call is the binding gate
 * between the parsed HCSB.HTC and a Firestaff-facing hint
 * surface.
 *
 * Returns the number of bytes written (excl. the trailing NUL),
 * or a negative CSB_HintOracleUIBinding_Result on error.
 * `*out_was_truncated` is set to 1 if the buffer could not hold
 * the full first-page text (callers can decide whether to
 * re-allocate or show what fits).
 */
int csb_hint_oracle_ui_binding_format_hint(
    const CSB_HintOracleHTC_RealCache *cache,
    size_t hint_index,
    char *buf,
    size_t buf_size,
    int *out_was_truncated);

/* Resolve a (level, x, y) location into the first matching
 * hint's name + first-page decoded text. The (255, 255) XY
 * wildcard is supported and matches the real Atari ST 2.x
 * HCSB.HTC level-wide hints.
 *
 * `out_hint_index` receives the resolved hint index (caller-
 * owned, may be NULL).
 *
 * The first-page text is the same surface a Firestaff Hint
 * Oracle overlay would render, so the call is the binding gate
 * between the location table and a Firestaff-facing hint
 * surface.
 *
 * Returns the number of bytes written (excl. the trailing NUL),
 * or a negative CSB_HintOracleUIBinding_Result on error.
 * `*out_was_truncated` is set to 1 if the buffer could not hold
 * the full first-page text.
 */
int csb_hint_oracle_ui_binding_resolve_location(
    const CSB_HintOracleHTC_RealCache *cache,
    uint8_t level,
    uint8_t x,
    uint8_t y,
    size_t *out_hint_index,
    char *buf,
    size_t buf_size,
    int *out_was_truncated);

const char *csb_hint_oracle_ui_binding_result_name(int result);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CSB_HINT_ORACLE_UI_RUNTIME_BINDING_H */
