/*
 * csb_hint_oracle_ui_runtime_binding.c
 *
 * Runtime-adjacent binding surface for the CSB Utility Disk
 * HCSB.HTC Hint Oracle.
 *
 * See include/csb_hint_oracle_ui_runtime_binding.h for scope and
 * source-lock boundary. The implementation is deliberately a
 * thin pass-through on top of the existing parser + scan/cache:
 *   - csb_hint_oracle_htc.h owns the format contract.
 *   - csb_hint_oracle_htc_real_scan.h owns the real-asset load.
 *   - this module owns the "decoded page reaches a Firestaff-
 *     facing surface" gate (a printable text buffer that the
 *     launcher / M11 / future overlay can render).
 *
 * Nothing here reaches into the CSB runtime, the launcher UI,
 * or the Hint Oracle rendering path. That is still a broader
 * UI feature tracked in docs/FIRESTAFF_GAP_LIST.md row C1/A1.
 */

#include "csb_hint_oracle_ui_runtime_binding.h"
#include "csb_hint_oracle_htc_variant.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

/* ── Result-name table ──────────────────────────────────────────── */

const char *csb_hint_oracle_ui_binding_result_name(int result)
{
    switch (result) {
    case CSB_HINT_ORACLE_UI_BINDING_OK: return "OK";
    case CSB_HINT_ORACLE_UI_BINDING_ERR_ARGUMENT: return "argument";
    case CSB_HINT_ORACLE_UI_BINDING_ERR_NOT_LOADED: return "not-loaded";
    case CSB_HINT_ORACLE_UI_BINDING_ERR_HINT_OUT_OF_RANGE: return "hint-out-of-range";
    case CSB_HINT_ORACLE_UI_BINDING_ERR_NO_HINT_AT_LOCATION: return "no-hint-at-location";
    case CSB_HINT_ORACLE_UI_BINDING_ERR_OUTPUT_TOO_SMALL: return "output-too-small";
    case CSB_HINT_ORACLE_UI_BINDING_ERR_DECODE: return "decode";
    default: return "unknown";
    }
}

/* ── Helpers ────────────────────────────────────────────────────── */

/* Decode a single hint's first page into `page_buf` (capacity
 * `page_cap`) and return the number of decoded bytes (excl. the
 * trailing NUL). The decoded text is NUL-terminated when there
 * is room. Returns a negative CSB_HintOracleUIBinding_Result on
 * error. */
static int decode_first_page(const CSB_HintOracleHTC_RealCache *cache,
                             size_t hint_index,
                             uint8_t *page_buf,
                             size_t page_cap,
                             size_t *out_size)
{
    int rc;
    size_t got = 0u;

    if (!cache || !cache->loaded || !page_buf || page_cap == 0u ||
        !out_size) {
        return CSB_HINT_ORACLE_UI_BINDING_ERR_ARGUMENT;
    }
    if (hint_index >= cache->htc.hint_count) {
        return CSB_HINT_ORACLE_UI_BINDING_ERR_HINT_OUT_OF_RANGE;
    }
    rc = csb_hint_oracle_htc_real_decompress_first_page(
        cache, hint_index, page_buf, page_cap - 1u, &got);
    if (rc != CSB_HINT_ORACLE_HTC_REAL_OK) {
        return CSB_HINT_ORACLE_UI_BINDING_ERR_DECODE;
    }
    /* NUL-terminate whatever fits. */
    if (got >= page_cap) {
        got = page_cap - 1u;
    }
    page_buf[got] = '\0';
    *out_size = got;
    return CSB_HINT_ORACLE_UI_BINDING_OK;
}

/* `snprintf`-style append that returns 1 if `src` was fully
 * appended, or 0 if the buffer was already exhausted / too
 * small. `*in_out_written` carries the running offset; when
 * the call drops below the capacity it freezes the offset to
 * the capacity so subsequent calls short-circuit. */
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
        /* Truncated. Freeze the offset and report failure. */
        *in_out_written = buf_size;
        return 0;
    }
    *in_out_written = written + (size_t)n;
    return 1;
}

static int append_char(char *buf, size_t buf_size,
                       size_t *in_out_written, char c)
{
    char tmp[2];
    tmp[0] = c;
    tmp[1] = '\0';
    return append_str(buf, buf_size, in_out_written, tmp);
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

/* Append a single line of printable hint-page text into `buf`,
 * masking non-printable bytes. Stops at the first NUL or at
 * `len`, whichever comes first. */
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
            /* Skip CR. */
            continue;
        }
        if (c == '\n') {
            append_char(buf, buf_size, in_out_written, '\n');
            continue;
        }
        if (c == '\t') {
            append_char(buf, buf_size, in_out_written, ' ');
            continue;
        }
        if (c < 0x20u || c > 0x7eu) {
            append_char(buf, buf_size, in_out_written, '.');
            continue;
        }
        append_char(buf, buf_size, in_out_written, (char)c);
    }
}

/* ── Single-hint formatter ──────────────────────────────────────── */

int csb_hint_oracle_ui_binding_format_hint(
    const CSB_HintOracleHTC_RealCache *cache,
    size_t hint_index,
    char *buf,
    size_t buf_size,
    int *out_was_truncated)
{
    char hint_name[CSB_HINT_ORACLE_HTC_HINT_NAME_BYTES + 1u];
    int name_rc;
    uint8_t page_buf[CSB_HINT_ORACLE_UI_BINDING_PAGE_CAP];
    size_t page_size = 0u;
    int decode_rc;
    size_t written = 0u;
    int full = 1;

    if (out_was_truncated) {
        *out_was_truncated = 0;
    }
    if (!buf || buf_size == 0u) {
        return CSB_HINT_ORACLE_UI_BINDING_ERR_ARGUMENT;
    }
    if (!cache || !cache->loaded) {
        /* Always NUL-terminate. */
        if (buf_size > 0u) {
            buf[0] = '\0';
        }
        return CSB_HINT_ORACLE_UI_BINDING_ERR_NOT_LOADED;
    }
    if (hint_index >= cache->htc.hint_count) {
        if (buf_size > 0u) {
            buf[0] = '\0';
        }
        return CSB_HINT_ORACLE_UI_BINDING_ERR_HINT_OUT_OF_RANGE;
    }

    /* Resolve the hint name through the existing helper so the
     * binding surface reuses the same NUL-terminated contract. */
    name_rc = csb_hint_oracle_htc_real_get_hint_name(
        cache, hint_index, hint_name, sizeof(hint_name));
    if (name_rc != CSB_HINT_ORACLE_HTC_REAL_OK) {
        if (buf_size > 0u) {
            buf[0] = '\0';
        }
        return CSB_HINT_ORACLE_UI_BINDING_ERR_NOT_LOADED;
    }

    decode_rc = decode_first_page(cache, hint_index,
                                  page_buf, sizeof(page_buf),
                                  &page_size);
    if (decode_rc != CSB_HINT_ORACLE_UI_BINDING_OK) {
        if (buf_size > 0u) {
            buf[0] = '\0';
        }
        return decode_rc;
    }

    full &= append_format(buf, buf_size, &written,
                          "=== CSB Hint Oracle — hint[%zu] %s ===\n",
                          hint_index, hint_name);
    append_hint_text_line(buf, buf_size, &written, page_buf, page_size);
    /* Always end with a newline + NUL when room remains. */
    if (written < buf_size) {
        append_char(buf, buf_size, &written, '\n');
    } else {
        full = 0;
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

/* ── Location resolver ──────────────────────────────────────────── */

int csb_hint_oracle_ui_binding_resolve_location(
    const CSB_HintOracleHTC_RealCache *cache,
    uint8_t level,
    uint8_t x,
    uint8_t y,
    size_t *out_hint_index,
    char *buf,
    size_t buf_size,
    int *out_was_truncated)
{
    uint16_t indices[8];
    size_t count = 0u;
    int rc;
    int n;

    if (out_was_truncated) {
        *out_was_truncated = 0;
    }
    if (!buf || buf_size == 0u) {
        return CSB_HINT_ORACLE_UI_BINDING_ERR_ARGUMENT;
    }
    if (!cache || !cache->loaded) {
        if (buf_size > 0u) {
            buf[0] = '\0';
        }
        return CSB_HINT_ORACLE_UI_BINDING_ERR_NOT_LOADED;
    }
    rc = csb_hint_oracle_htc_real_find_hints_for_location(
        cache, level, x, y,
        indices, sizeof(indices) / sizeof(indices[0]), &count);
    if (rc == CSB_HINT_ORACLE_HTC_REAL_ERR_NOT_LOADED) {
        if (buf_size > 0u) {
            buf[0] = '\0';
        }
        return CSB_HINT_ORACLE_UI_BINDING_ERR_NOT_LOADED;
    }
    if (rc != CSB_HINT_ORACLE_HTC_REAL_OK) {
        if (buf_size > 0u) {
            buf[0] = '\0';
        }
        return CSB_HINT_ORACLE_UI_BINDING_ERR_NO_HINT_AT_LOCATION;
    }
    if (count == 0u) {
        if (buf_size > 0u) {
            buf[0] = '\0';
        }
        return CSB_HINT_ORACLE_UI_BINDING_ERR_NO_HINT_AT_LOCATION;
    }
    if (out_hint_index) {
        *out_hint_index = (size_t)indices[0];
    }
    n = csb_hint_oracle_ui_binding_format_hint(
        cache, (size_t)indices[0], buf, buf_size, out_was_truncated);
    return n;
}

/* ── Diagnostic / oracle report ─────────────────────────────────── */

int csb_hint_oracle_ui_binding_format_report(
    const CSB_HintOracleHTC_RealCache *cache,
    char *buf,
    size_t buf_size,
    int *out_was_truncated)
{
    size_t written = 0u;
    int full = 1;
    size_t level0_hint_index = 0u;
    CSB_HintOracleHTC_Variant variant;
    CSB_HintOracleHTC_VariantDrift drift;
    int resolve_rc;
    int drift_rc;
    int n;

    if (out_was_truncated) {
        *out_was_truncated = 0;
    }
    if (!buf || buf_size == 0u) {
        return CSB_HINT_ORACLE_UI_BINDING_ERR_ARGUMENT;
    }
    if (!cache || !cache->loaded) {
        full &= append_str(buf, buf_size, &written,
                           "(CSB Hint Oracle: not loaded)\n");
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

    full &= append_str(buf, buf_size, &written,
                       "=== CSB Hint Oracle — diagnostic report ===\n");
    full &= append_format(buf, buf_size, &written,
                          "matched_md5=%s\n",
                          cache->matched_md5[0] ? cache->matched_md5 : "(unknown)");
    full &= append_format(buf, buf_size, &written,
                          "matched_label=%s\n",
                          cache->matched_label[0] ? cache->matched_label : "(unknown)");
    variant = csb_hint_oracle_htc_variant_from_cache(cache);
    drift_rc = csb_hint_oracle_htc_variant_drift(
        variant, &cache->htc, cache->file_size, &drift);
    full &= append_format(buf, buf_size, &written,
                          "variant=%s release=%s language=%s\n",
                          csb_hint_oracle_htc_variant_name(variant),
                          csb_hint_oracle_htc_variant_release_name(variant),
                          csb_hint_oracle_htc_variant_language(variant));
    if (drift_rc == 1) {
        full &= append_format(buf, buf_size, &written,
                              "variant_drift=%s expected_size=%zu "
                              "expected_location_count=%zu "
                              "expected_hint_count=%zu "
                              "expected_page_count=%zu\n",
                              drift.matches ? "match" : "drift",
                              drift.expected_size,
                              drift.expected_location_count,
                              drift.expected_hint_count,
                              drift.expected_page_count);
    } else {
        full &= append_str(buf, buf_size, &written,
                           "variant_drift=unknown\n");
    }
    full &= append_format(buf, buf_size, &written,
                          "file_size=%zu\n", cache->file_size);
    full &= append_format(buf, buf_size, &written,
                          "resolved_path=%s\n",
                          cache->resolved_path[0] ? cache->resolved_path : "(unset)");
    full &= append_format(buf, buf_size, &written,
                          "original_path=%s\n",
                          cache->original_path[0] ? cache->original_path : "(unset)");
    full &= append_format(buf, buf_size, &written,
                          "format_word=%u dungeon_id=%u\n",
                          (unsigned)cache->htc.format_word,
                          (unsigned)cache->htc.dungeon_id);
    full &= append_format(buf, buf_size, &written,
                          "location_count=%zu hint_count=%zu page_count=%zu "
                          "content_size=%zu\n",
                          cache->htc.location_count,
                          cache->htc.hint_count,
                          cache->htc.page_count,
                          cache->htc.content_size);

    /* Hint 0 binding: prove the first hint's name + first-page
     * text round-trips through the same surface an overlay
     * would render. We deliberately do not pass the truncated
     * flag through so the report itself reports a clean
     * overall status. */
    full &= append_str(buf, buf_size, &written,
                       "--- hint[0] (binding smoke) ---\n");
    {
        int hint_truncated = 0;
        size_t before = written;
        int hint_rc = csb_hint_oracle_ui_binding_format_hint(
            cache, 0u,
            buf + written,
            written < buf_size ? buf_size - written : 0u,
            &hint_truncated);
        if (hint_rc > 0) {
            written += (size_t)hint_rc;
        } else {
            full &= append_str(buf, buf_size, &written,
                               "(hint 0 binding failed)\n");
            (void)before;
        }
    }

    /* Wildcard (level=0, x=255, y=255) binding: prove the
     * location table resolves to a real, decodable hint on
     * the loaded cache. The real Atari ST 2.x HCSB.HTC and
     * the synthetic test fixture both expose wildcard
     * records, so this surface is the canonical smoke. */
    full &= append_str(buf, buf_size, &written,
                       "--- level=0 (255,255) wildcard (binding smoke) ---\n");
    resolve_rc = csb_hint_oracle_ui_binding_resolve_location(
        cache, 0u,
        CSB_HINT_ORACLE_HTC_ANY_XY, CSB_HINT_ORACLE_HTC_ANY_XY,
        &level0_hint_index,
        buf + written,
        written < buf_size ? buf_size - written : 0u,
        NULL);
    if (resolve_rc > 0) {
        written += (size_t)resolve_rc;
    } else {
        full &= append_format(buf, buf_size, &written,
                              "(level 0 wildcard resolve rc=%d %s)\n",
                              resolve_rc,
                              csb_hint_oracle_ui_binding_result_name(resolve_rc));
    }
    (void)level0_hint_index;

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
