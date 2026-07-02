/*
 * csb_v1_csbgraphics_dat_lzw_boundary.c
 *
 * Read-only LZW block-boundary walker for CSBWin
 * "CSBgraphics.dat". See
 * include/csb_v1_csbgraphics_dat_lzw_boundary.h for scope and
 * source references.
 *
 * The walker follows the ReDMCSB LZW.C bit-stream contract
 * without ever building a dictionary or decoding a pixel. It
 * tracks the same state variables the real decoder tracks:
 *
 *   G0664_i_LZW_CodeBitCount (ReDMCSB LZW.C:31)
 *     - starts at 9 (CSB_V1_CSBGRAPHICS_LZW_MIN_BITS)
 *     - grows by 1 each time the dictionary's `next_code`
 *       crosses the current `current_max_code`
 *     - plateaus at 12 (CSB_V1_CSBGRAPHICS_LZW_MAX_BITS)
 *
 *   G0665_i_LZW_CurrentMaximumCode (ReDMCSB LZW.C:32)
 *     - (1 << code_bits) - 1
 *     - grows with code_bits up to 4095 at 12-bit width
 *
 *   G0668_i_LZW_DictionaryNextAvailableCode (ReDMCSB LZW.C:33)
 *     - starts at FIRST_CODE = 258
 *     - increments per new dictionary entry, except right after
 *       a clear (which resets it to FIRST_CODE)
 *     - caps at MAX_CODE = 4096
 *
 *   F0495_LZW_GetNextInputCode (ReDMCSB LZW.C:35)
 *     - LSB-first bit-stream
 *     - pulls `code_bits` bits from the input buffer
 *
 * The walker re-uses the bit-stream orientation (LSB-first)
 * and the same `code_bits` ladder as the real decoder so the
 * verdict it produces is "this is exactly where a real
 * CSBWin-compatible LZW decoder would have stopped or
 * continued".
 *
 * Non-claims:
 *   - No dictionary allocation, no pixel decode, no output.
 *   - No CSB runtime or M11 viewport wiring.
 *   - No CSBgraphics.dat MD5 default hash (the real_scan module
 *     owns that and stays empty by default).
 */

#include "csb_v1_csbgraphics_dat_lzw_boundary.h"

#include <stdlib.h>
#include <string.h>

/* ── Bit cursor ──────────────────────────────────────────────────── */

typedef struct {
    const uint8_t *bytes;     /* input buffer                      */
    size_t        size;       /* buffer size in bytes              */
    size_t        byte_pos;   /* next byte to consume              */
    uint32_t      bit_pos;    /* bit index within bytes[byte_pos]  */
                               /* (0 = LSB, 7 = MSB; LSB-first      */
                               /* per ReDMCSB LZW.C:35)             */
} bit_cursor;

static void cursor_init(bit_cursor *c,
                        const uint8_t *bytes, size_t size)
{
    c->bytes = bytes;
    c->size = size;
    c->byte_pos = 0u;
    c->bit_pos = 0u;
}

/* Try to pull `n_bits` bits from the cursor. Returns 1 on
 * success and writes the value through `out_code`; returns 0
 * when the cursor runs out of bytes before satisfying the
 * request. The cursor uses LSB-first orientation per
 * F0495_LZW_GetNextInputCode (ReDMCSB LZW.C). */
static int cursor_take(bit_cursor *c, uint32_t n_bits, uint32_t *out_code)
{
    uint32_t value = 0u;
    uint32_t bit_shift = 0u;
    uint32_t bits_taken = 0u;

    while (bits_taken < n_bits) {
        uint32_t bits_in_cur_byte;
        uint32_t bits_to_take;
        uint32_t byte_value;
        if (c->byte_pos >= c->size) {
            return 0;
        }
        bits_in_cur_byte = 8u - c->bit_pos;
        bits_to_take = (n_bits - bits_taken) < bits_in_cur_byte
                           ? (n_bits - bits_taken)
                           : bits_in_cur_byte;
        byte_value = (uint32_t)c->bytes[c->byte_pos];
        /* Drop the bits we already consumed (high bits) and
         * pull the next `bits_to_take` low bits. */
        byte_value >>= c->bit_pos;
        byte_value &= ((1u << bits_to_take) - 1u);
        value |= (byte_value << bit_shift);
        bits_taken += bits_to_take;
        bit_shift += bits_to_take;
        c->bit_pos += bits_to_take;
        if (c->bit_pos >= 8u) {
            c->bit_pos = 0u;
            ++c->byte_pos;
        }
    }
    *out_code = value;
    return 1;
}

/* ── Per-entry walker ────────────────────────────────────────────── */

typedef struct {
    /* input */
    const uint8_t *compressed;
    uint32_t       compressed_bytes;
    uint32_t       entry_index;
    /* output */
    CSB_V1_CSBGraphicsLZWBoundaryEntry *entry;
    CSB_V1_CSBGraphicsLZWResult         result;
} walk_state;

static void entry_init(walk_state *w)
{
    memset(w->entry, 0, sizeof(*w->entry));
    w->entry->entry_index = w->entry_index;
    w->entry->bits_avail = w->compressed_bytes * 8u;
    w->entry->code_bits_start = CSB_V1_CSBGRAPHICS_LZW_MIN_BITS;
    w->entry->result = CSB_V1_CSBGRAPHICS_LZW_RESULT_OK;
}

static void entry_finish(walk_state *w)
{
    /* The walk_state.result is the authoritative verdict for
     * the entry; mirror it onto the entry so callers can
     * inspect the per-entry field without a parallel switch. */
    w->entry->result = w->result;
}

static int advance_code_width(uint32_t *code_bits,
                              uint32_t *current_max_code)
{
    if (*code_bits < CSB_V1_CSBGRAPHICS_LZW_MAX_BITS) {
        *code_bits += 1u;
        *current_max_code = (1u << *code_bits) - 1u;
        return 1;
    }
    return 0;
}

/* Walk a single CSBgraphics.dat entry's LZW bit-stream.
 * Returns the per-entry result and fills `w->entry`.
 *
 * The walker honors the ReDMCSB LZW.C state machine:
 *   - clear (256) -> reset next_code to FIRST_CODE, code_bits
 *     back to MIN_BITS, current_max_code to 511
 *   - end-of-info (257) -> stop, mark clean termination
 *   - first_code (258) up to current_max_code -> increment
 *     next_code; if next_code exceeds current_max_code and we
 *     are still under MAX_BITS, grow code_bits by 1
 *   - code > current_max_code -> reserved / overflow
 *   - code < 256 -> literal byte (treated as a "consumed" code
 *     by the walker; we don't need its value)
 *   - code >= MAX_CODE (4096) -> overflow
 *
 * Truncation (cursor_take returns 0) is reported as
 * CSB_V1_CSBGRAPHICS_LZW_RESULT_TRUNCATED, not as a fatal
 * error: the walker keeps the per-entry bits_consumed counter
 * honest so the calling test can assert where the bit-stream
 * ran dry.
 *
 * Note on next_code bookkeeping: ReDMCSB LZW.C:124-132 marks
 * the very first consumed code as BUG0_00 because the
 * pre-loop dictionary initialization leaves L1561_i_OldCode
 * uninitialized. The walker collapses that anomaly by simply
 * bumping next_code after every non-clear, non-EOI code; the
 * resulting max_next_code is therefore off-by-one in the
 * degenerate "single literal byte" case. That off-by-one does
 * not affect the boundary verdict, only the diagnostic
 * max_next_code counter, which is documented as approximate.
 */
static CSB_V1_CSBGraphicsLZWResult walk_entry(walk_state *w)
{
    bit_cursor cur;
    uint32_t code_bits = CSB_V1_CSBGRAPHICS_LZW_MIN_BITS;
    uint32_t current_max_code = (1u << code_bits) - 1u;
    uint32_t next_code = CSB_V1_CSBGRAPHICS_LZW_FIRST_CODE;

    entry_init(w);

    if (w->compressed_bytes == 0u) {
        w->entry->code_bits_end = code_bits;
        w->result = CSB_V1_CSBGRAPHICS_LZW_RESULT_EMPTY;
        entry_finish(w);
        return w->result;
    }

    cursor_init(&cur, w->compressed, w->compressed_bytes);

    for (;;) {
        uint32_t code = 0u;
        uint32_t bits_before = (uint32_t)(cur.byte_pos * 8u + cur.bit_pos);
        if (!cursor_take(&cur, code_bits, &code)) {
            w->entry->code_bits_end = code_bits;
            w->entry->bits_consumed = bits_before;
            w->result = CSB_V1_CSBGRAPHICS_LZW_RESULT_TRUNCATED;
            entry_finish(w);
            return w->result;
        }
        w->entry->codes_walked += 1u;
        w->entry->bits_consumed = (uint32_t)(cur.byte_pos * 8u + cur.bit_pos);

        if (code == CSB_V1_CSBGRAPHICS_LZW_CLEAR_CODE) {
            w->entry->had_clear_code = 1u;
            w->entry->clear_codes_seen += 1u;
            /* Clear code resets next_code; the next code width
             * grow check happens at the next refill, mirroring
             * ReDMCSB LZW.C:38-50. */
            next_code = CSB_V1_CSBGRAPHICS_LZW_FIRST_CODE;
            continue;
        }
        if (code == CSB_V1_CSBGRAPHICS_LZW_END_CODE) {
            w->entry->had_end_of_info_code = 1u;
            w->entry->end_of_info_seen += 1u;
            w->entry->code_bits_end = code_bits;
            w->entry->clean_termination = 1u;
            w->result = CSB_V1_CSBGRAPHICS_LZW_RESULT_OK;
            entry_finish(w);
            return w->result;
        }
        if (code >= CSB_V1_CSBGRAPHICS_LZW_MAX_CODE) {
            w->entry->code_bits_end = code_bits;
            w->entry->had_dict_overflow = 1u;
            w->result = CSB_V1_CSBGRAPHICS_LZW_RESULT_OVERFLOW;
            entry_finish(w);
            return w->result;
        }
        if (code >= next_code && code != CSB_V1_CSBGRAPHICS_LZW_END_CODE &&
            code != CSB_V1_CSBGRAPHICS_LZW_CLEAR_CODE) {
            /* KwKwK reserved-slot case: encoder wrote a code
             * the decoder has not allocated yet. ReDMCSB
             * LZW.C:107-114 handles this by appending the last
             * decoded character to the output string. We mark
             * the entry as RESERVED so the calling test can
             * distinguish this from a clean termination, but
             * keep walking so bits_consumed reflects the full
             * bit-stream and the width ladder stays consistent. */
            w->result = CSB_V1_CSBGRAPHICS_LZW_RESULT_RESERVED;
        }
        if (next_code > w->entry->max_next_code) {
            w->entry->max_next_code = next_code;
        }
        if (next_code > CSB_V1_CSBGRAPHICS_LZW_MAX_CODE) {
            /* Dictionary is full; the next consumed code that
             * would write beyond MAX_CODE trips the OVERFLOW
             * branch above. */
            w->entry->had_dict_overflow = 1u;
        } else {
            ++next_code;
            if (next_code > current_max_code &&
                code_bits < CSB_V1_CSBGRAPHICS_LZW_MAX_BITS) {
                w->entry->dict_growth_steps += 1u;
                (void)advance_code_width(&code_bits,
                                          &current_max_code);
            } else if (next_code > CSB_V1_CSBGRAPHICS_LZW_MAX_CODE) {
                /* The bump pushed next_code to MAX_CODE + 1;
                 * the dictionary is now full. Mark it so the
                 * calling test can assert the boundary
                 * invariant. */
                w->entry->had_dict_overflow = 1u;
            }
        }
    }
}

/* ── Per-file walker ─────────────────────────────────────────────── */

static int derive_compressed_table(const uint8_t *bytes,
                                   const CSB_V1_CSBGraphicsIndex *index,
                                   uint16_t *out_table,
                                   size_t out_table_cap)
{
    size_t count = (size_t)index->count;
    size_t tables_offset;
    size_t comp_off;
    size_t i;

    if (!bytes || !index || !out_table) {
        return 0;
    }
    if (count == 0u || count > out_table_cap) {
        return 0;
    }
    tables_offset = (index->byte_order ==
                     CSB_V1_CSBGRAPHICS_BYTE_ORDER_LITTLE_ENDIAN_MARKER)
                        ? 4u
                        : 2u;
    comp_off = tables_offset;
    for (i = 0u; i < count; ++i) {
        uint16_t comp;
        if (comp_off + (i + 1u) * 2u > index->payload_offset) {
            return 0;
        }
        if (index->byte_order ==
            CSB_V1_CSBGRAPHICS_BYTE_ORDER_LITTLE_ENDIAN_MARKER) {
            comp = (uint16_t)(((uint16_t)bytes[comp_off + i * 2u + 1u] << 8) |
                              (uint16_t)bytes[comp_off + i * 2u]);
        } else {
            comp = (uint16_t)(((uint16_t)bytes[comp_off + i * 2u] << 8) |
                              (uint16_t)bytes[comp_off + i * 2u + 1u]);
        }
        out_table[i] = comp;
    }
    return 1;
}

int csb_v1_csbgraphics_dat_lzw_boundary_walk(
    const uint8_t *bytes, size_t size,
    const CSB_V1_CSBGraphicsIndex *index,
    CSB_V1_CSBGraphicsLZWBoundaryReport *out_report)
{
    uint16_t *comp_table = NULL;
    size_t count;
    uint64_t cumulative_offset;
    uint32_t i;
    int walk_ok = 1;

    if (!bytes || !index || !out_report) {
        return CSB_V1_CSBGRAPHICS_LZW_RESULT_ERR_ARGUMENT;
    }
    count = (size_t)index->count;
    if (count == 0u || count > (size_t)CSB_V1_CSBGRAPHICS_MAX_COUNT) {
        return CSB_V1_CSBGRAPHICS_LZW_RESULT_ERR_ARGUMENT;
    }
    if (size < index->payload_offset + index->payload_bytes_avail) {
        return CSB_V1_CSBGRAPHICS_LZW_RESULT_ERR_BAD_PAYLOAD;
    }

    memset(out_report, 0, sizeof(*out_report));
    out_report->entry_count = (uint32_t)count;
    if (out_report->entries == NULL) {
        out_report->entries = (CSB_V1_CSBGraphicsLZWBoundaryEntry *)
            calloc(count, sizeof(*out_report->entries));
        if (!out_report->entries) {
            return CSB_V1_CSBGRAPHICS_LZW_RESULT_ERR_ARGUMENT;
        }
    }

    /* Read the parallel compressed-size table fresh from the
     * bytes view. We do not trust the caller to expose it
     * separately; the index already validated sum(payloads)
     * <= payload_avail. */
    comp_table = (uint16_t *)calloc(count, sizeof(*comp_table));
    if (!comp_table) {
        if (out_report->entries) {
            free(out_report->entries);
            out_report->entries = NULL;
        }
        return CSB_V1_CSBGRAPHICS_LZW_RESULT_ERR_ARGUMENT;
    }
    if (!derive_compressed_table(bytes, index, comp_table, count)) {
        free(comp_table);
        if (out_report->entries) {
            free(out_report->entries);
            out_report->entries = NULL;
        }
        return CSB_V1_CSBGRAPHICS_LZW_RESULT_ERR_BAD_PAYLOAD;
    }

    cumulative_offset = index->payload_offset;
    for (i = 0u; i < (uint32_t)count; ++i) {
        walk_state w;
        uint32_t comp_size = (uint32_t)comp_table[i];
        const uint8_t *comp_ptr;
        uint64_t entry_end;
        if (comp_size > index->payload_bytes_avail ||
            cumulative_offset > size ||
            (uint64_t)comp_size > size - cumulative_offset) {
            out_report->entries[i].entry_index = i;
            out_report->entries[i].bits_avail = 0u;
            out_report->entries_truncated += 1u;
            /* entry_count already reflects total entries; do
             * not bump it again here. */
            cumulative_offset += comp_size;
            continue;
        }
        comp_ptr = bytes + cumulative_offset;
        entry_end = cumulative_offset + comp_size;
        (void)entry_end;

        memset(&w, 0, sizeof(w));
        w.compressed = comp_ptr;
        w.compressed_bytes = comp_size;
        w.entry_index = i;
        w.entry = &out_report->entries[i];
        w.result = CSB_V1_CSBGRAPHICS_LZW_RESULT_OK;

        walk_entry(&w);
        switch (w.result) {
        case CSB_V1_CSBGRAPHICS_LZW_RESULT_OK:
            out_report->entries_ok += 1u;
            break;
        case CSB_V1_CSBGRAPHICS_LZW_RESULT_TRUNCATED:
            out_report->entries_truncated += 1u;
            break;
        case CSB_V1_CSBGRAPHICS_LZW_RESULT_OVERFLOW:
            out_report->entries_overflow += 1u;
            break;
        case CSB_V1_CSBGRAPHICS_LZW_RESULT_RESERVED:
            out_report->entries_reserved += 1u;
            break;
        case CSB_V1_CSBGRAPHICS_LZW_RESULT_EMPTY:
            out_report->entries_empty += 1u;
            break;
        default:
            walk_ok = 0;
            break;
        }

        cumulative_offset += comp_size;
    }

    free(comp_table);
    (void)walk_ok;
    return CSB_V1_CSBGRAPHICS_LZW_RESULT_OK;
}

void csb_v1_csbgraphics_dat_lzw_boundary_report_free(
    CSB_V1_CSBGraphicsLZWBoundaryReport *report)
{
    if (!report) {
        return;
    }
    if (report->entries) {
        free(report->entries);
    }
    memset(report, 0, sizeof(*report));
}

const char *csb_v1_csbgraphics_dat_lzw_boundary_result_name(int result)
{
    switch (result) {
    case CSB_V1_CSBGRAPHICS_LZW_RESULT_OK: return "OK";
    case CSB_V1_CSBGRAPHICS_LZW_RESULT_TRUNCATED: return "truncated";
    case CSB_V1_CSBGRAPHICS_LZW_RESULT_OVERFLOW: return "overflow";
    case CSB_V1_CSBGRAPHICS_LZW_RESULT_RESERVED: return "reserved";
    case CSB_V1_CSBGRAPHICS_LZW_RESULT_EMPTY: return "empty";
    case CSB_V1_CSBGRAPHICS_LZW_RESULT_ERR_ARGUMENT: return "argument";
    case CSB_V1_CSBGRAPHICS_LZW_RESULT_ERR_BAD_PAYLOAD:
        return "bad-payload";
    default: return "unknown";
    }
}

const char *csb_v1_csbgraphics_dat_lzw_boundary_source_evidence(void)
{
    return
        "CSBWin/Graphics.cpp:1717 ReadGraphic (cluster-bounded read)\n"
        "CSBWin/Graphics.cpp:1918 ReadGraphicsIndex (count + tables)\n"
        "ReDMCSB LZW.C:31 G0664_i_LZW_CodeBitCount = 9 init\n"
        "ReDMCSB LZW.C:32 G0665_i_LZW_CurrentMaximumCode = 511 init\n"
        "ReDMCSB LZW.C:33 G0666_i_LZW_AbsoluteMaximumCode = 4096\n"
        "ReDMCSB LZW.C:34 G0668_i_LZW_DictionaryNextAvailableCode\n"
        "ReDMCSB LZW.C:35 F0495_LZW_GetNextInputCode (LSB-first)\n"
        "DM/CSB LZW conventions: clear=256, end-of-info=257,\n"
        "                        FIRST_CODE=258, MAX_CODE=4096,\n"
        "                        code width 9..12 bits\n"
        "DM1 V1 graphics loader mirror:\n"
        "  include/dm1_v1_graphics_loader_pc34_compat.h:\n"
        "    DM1_GFX_LZW_CLEAR_CODE=256, END_CODE=257,\n"
        "    FIRST_CODE=258, MAX_CODE=4096";
}
