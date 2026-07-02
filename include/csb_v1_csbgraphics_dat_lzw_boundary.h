/*
 * csb_v1_csbgraphics_dat_lzw_boundary.h
 *
 * Read-only LZW block-boundary walker for CSBWin
 * "CSBgraphics.dat" custom graphics entries.
 *
 * This module sits beside (not under) the bytes → index
 * classifier in include/csb_v1_csbgraphics_dat_classify.h.
 * Where the classifier walks the file's count + parallel
 * compressed/decompressed size tables, this walker walks each
 * entry's compressed LZW bit-stream and verifies it is
 * well-formed *as a bit-stream* — without decompressing any
 * pixels.
 *
 * What "well-formed as a bit-stream" means:
 *   - the bit-stream starts on a 9-bit code width
 *     (ReDMCSB LZW.C G0664_i_LZW_CodeBitCount = 9)
 *   - clear code = 256 (reset dictionary)
 *   - end-of-info code = 257 (terminate block)
 *   - dictionary grows from FIRST_CODE = 258 up to MAX_CODE = 4096
 *   - code width grows 9 → 10 → 11 → 12 bits as the dictionary
 *     fills, then plateaus at 12 (no more width growth past 12)
 *
 * The walker is bounded on purpose:
 *   - it does not allocate a dictionary (the real decoder uses
 *     5003 prefix + 5003 append bytes per LZW.C; we only track
 *     `next_code` as a counter)
 *   - it does not decode pixels; it only advances the bit cursor
 *     and validates the code-stream invariants
 *   - it terminates early on the first malformed code so a
 *     single bad entry cannot stall a full-file scan
 *   - it never reaches into the CSB runtime, the M11 viewport
 *     renderer, or the M12 launcher; that remains tracked
 *     separately under docs/FIRESTAFF_GAP_LIST.md row C3 / A3
 *
 * Non-claims:
 *   - No LZW decode. No pixel output. No override hook.
 *   - No full CSBWin custom-dungeon support.
 *   - This walker is a *boundary test* gate — the next CSBWin
 *     custom-resource work that consumes a CSBgraphics.dat entry
 *     must first prove the bit-stream is well-formed via this
 *     module.
 *
 * Source references:
 *   - CSBWin/Graphics.cpp:1717 ReadGraphic (cluster-bounded read,
 *     1024-byte chunks)
 *   - CSBWin/Graphics.cpp:1918 ReadGraphicsIndex (count + size
 *     tables, optional 0x8001 LE sentinel)
 *   - ReDMCSB LZW.C F0495_LZW_GetNextInputCode (LSB-first bit
 *     stream, 9 → 12 bit code widths)
 *   - ReDMCSB LZW.C G0664_i_LZW_CodeBitCount = 9 init
 *   - ReDMCSB LZW.C G0665_i_LZW_CurrentMaximumCode = 511 init
 *   - ReDMCSB LZW.C G0666_i_LZW_AbsoluteMaximumCode = 4096
 *   - ReDMCSB LZW.C clear code 256, end-of-info code 257,
 *     FIRST_CODE 258
 *   - DM1 LZW.C companion in
 *     src/dm1/dm1_v1_graphics_loader_pc34_compat.c:
 *     DM1_GFX_LZW_{CLEAR,END,FIRST,MAX}_CODE mirror the same
 *     values (256/257/258/4096) so the CSBWin LZW block layout
 *     matches what the DM1 V1 graphics loader already decodes.
 */

#ifndef FIRESTAFF_CSB_V1_CSBGRAPHICS_DAT_LZW_BOUNDARY_H
#define FIRESTAFF_CSB_V1_CSBGRAPHICS_DAT_LZW_BOUNDARY_H

#include <stddef.h>
#include <stdint.h>

#include "csb_v1_csbgraphics_dat_classify.h"

#ifdef __cplusplus
extern "C" {
#endif

/* DM/CSB LZW constants — mirrored from ReDMCSB LZW.C so the
 * CSBWin "CSBgraphics.dat" walker and the DM1 V1 graphics
 * loader share one set of values. The DM1 V1 loader already
 * declares these as DM1_GFX_LZW_* in
 * include/dm1_v1_graphics_loader_pc34_compat.h; we duplicate
 * the literals here rather than #include that header so this
 * module has zero DM1 coupling. */
#define CSB_V1_CSBGRAPHICS_LZW_CLEAR_CODE   256u
#define CSB_V1_CSBGRAPHICS_LZW_END_CODE     257u
#define CSB_V1_CSBGRAPHICS_LZW_FIRST_CODE   258u
#define CSB_V1_CSBGRAPHICS_LZW_MAX_CODE     4096u
#define CSB_V1_CSBGRAPHICS_LZW_MIN_BITS     9u
#define CSB_V1_CSBGRAPHICS_LZW_MAX_BITS     12u

/* Per-entry boundary verdict.
 *
 * `clean_termination` is set when the bit-stream ends exactly
 * on a clear-or-end-of-info code that fits inside the entry's
 * declared compressed-size slot. Truncated entries (bit-stream
 * exhausted before end-of-info) report clean_termination = 0
 * and result_kind = CSB_V1_CSBGRAPHICS_LZW_RESULT_TRUNCATED.
 *
 * `had_clear_code` and `had_end_of_info_code` mirror what a
 * real LZW decoder would observe as it consumed the same
 * bit-stream; they are useful diagnostic flags for callers
 * that want to know whether a CSBgraphics.dat entry is a
 * one-shot tiny graphic (no clear, end-of-info only) or a
 * dictionary-resetting overlay (clear + populate + end).
 */
typedef enum {
    CSB_V1_CSBGRAPHICS_LZW_RESULT_OK = 0,
    CSB_V1_CSBGRAPHICS_LZW_RESULT_TRUNCATED = -1,
    CSB_V1_CSBGRAPHICS_LZW_RESULT_OVERFLOW = -2,
    CSB_V1_CSBGRAPHICS_LZW_RESULT_RESERVED = -3,
    CSB_V1_CSBGRAPHICS_LZW_RESULT_EMPTY = -4,
    CSB_V1_CSBGRAPHICS_LZW_RESULT_ERR_ARGUMENT = -5,
    CSB_V1_CSBGRAPHICS_LZW_RESULT_ERR_BAD_PAYLOAD = -6
} CSB_V1_CSBGraphicsLZWResult;

typedef struct {
    uint32_t entry_index;          /* which graphics entry        */
    uint32_t code_bits_start;      /* code width at entry start    */
    uint32_t code_bits_end;        /* code width at entry end      */
    uint32_t codes_walked;         /* total codes consumed         */
    uint32_t clear_codes_seen;     /* # clear (256) codes          */
    uint32_t end_of_info_seen;     /* # end-of-info (257) codes    */
    uint32_t dict_growth_steps;    /* # times next_code crossed    */
                                   /* current_max_code             */
    uint32_t max_next_code;        /* highest next_code observed   */
    uint32_t bits_consumed;        /* total bits pulled from stream*/
    uint32_t bits_avail;           /* compressed_size * 8          */
    int32_t  result;               /* per-entry verdict            */
                                   /* CSB_V1_CSBGraphicsLZWResult  */
    uint8_t  clean_termination;    /* 1 = bit-stream ended on EOI  */
                                   /* or clean clear               */
    uint8_t  had_clear_code;       /* saw at least one clear code  */
    uint8_t  had_end_of_info_code; /* saw at least one EOI code    */
    uint8_t  had_dict_overflow;    /* walked past MAX_CODE (4096)  */
} CSB_V1_CSBGraphicsLZWBoundaryEntry;

typedef struct {
    uint32_t entry_count;          /* size of `entries` array       */
    uint32_t entries_ok;           /* entries with result == OK     */
    uint32_t entries_truncated;    /* entries with TRUNCATED        */
    uint32_t entries_overflow;     /* entries with OVERFLOW         */
    uint32_t entries_reserved;     /* entries that hit a reserved   */
                                   /* (illegal 258..MAX_CODE)      */
                                   /* code                         */
    uint32_t entries_empty;        /* entries with zero compressed */
                                   /* size                         */
    CSB_V1_CSBGraphicsLZWBoundaryEntry *entries;
} CSB_V1_CSBGraphicsLZWBoundaryReport;

/* Walk every entry's compressed LZW block boundary in the
 * already-classified CSBgraphics.dat buffer. The walker never
 * decompresses pixels; it only advances the bit cursor and
 * records the per-entry verdict.
 *
 * `bytes` / `size` / `index` must come from
 * csb_v1_csbgraphics_dat_classify(). The walker re-derives each
 * entry's compressed-size slot from `index->count` and the
 * `bytes` view directly, so callers do not have to expose the
 * parallel compressed/decompressed size tables separately.
 *
 * `out_report->entries` must be either NULL (the walker
 * allocates a report sized to `index->count`) or a caller-
 * supplied array of at least `index->count` elements; the
 * walker fills the entries in entry order and writes the
 * summary counters. When `out_report->entries` is non-NULL,
 * the caller owns the storage; when NULL, the walker owns it
 * and the caller must call
 * csb_v1_csbgraphics_dat_lzw_boundary_report_free().
 *
 * Returns CSB_V1_CSBGRAPHICS_LZW_RESULT_OK on success even when
 * individual entries are truncated or overflow — the per-entry
 * verdict is what callers should inspect. Returns a negative
 * error code only when arguments are missing or the buffer is
 * shorter than the classifier already accepted.
 */
int csb_v1_csbgraphics_dat_lzw_boundary_walk(
    const uint8_t *bytes, size_t size,
    const CSB_V1_CSBGraphicsIndex *index,
    CSB_V1_CSBGraphicsLZWBoundaryReport *out_report);

/* Free a walker-owned entries array. Safe to call on a report
 * the walker did not allocate (entries == NULL is OK). */
void csb_v1_csbgraphics_dat_lzw_boundary_report_free(
    CSB_V1_CSBGraphicsLZWBoundaryReport *report);

/* Human-readable label for a result code. */
const char *csb_v1_csbgraphics_dat_lzw_boundary_result_name(int result);

/* Source-evidence string for tests + docs. */
const char *csb_v1_csbgraphics_dat_lzw_boundary_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CSB_V1_CSBGRAPHICS_DAT_LZW_BOUNDARY_H */
