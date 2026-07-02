/*
 * test_csb_v1_csbgraphics_dat_lzw_boundary.c
 *
 * Data-free contract tests for the CSBWin "CSBgraphics.dat"
 * LZW block-boundary walker.
 *
 * Scope:
 *   - Bit-stream orientation (LSB-first per ReDMCSB LZW.C:35).
 *   - Code-width ladder: 9 -> 10 -> 11 -> 12 bits as the
 *     dictionary fills.
 *   - Clear code (256) resets next_code to FIRST_CODE (258).
 *   - End-of-info code (257) terminates cleanly.
 *   - Code >= MAX_CODE (4096) is reported as OVERFLOW.
 *   - Truncated bit-streams report the exact bits_consumed
 *     where the cursor ran dry.
 *   - Per-entry summary counters stay consistent across
 *     clear/EOI/overflow/empty code paths.
 *
 * Source lock:
 *   - ReDMCSB LZW.C F0495_LZW_GetNextInputCode
 *   - ReDMCSB LZW.C G0664..G0669 LZW state variables
 *   - CSBWin/Graphics.cpp:1717 ReadGraphic
 *   - include/csb_v1_csbgraphics_dat_classify.h
 *     (bytes -> index contract)
 *
 * Non-claims:
 *   - No real CSBWin "CSBgraphics.dat" is loaded.
 *   - No LZW decompression. No pixel decode. No override.
 *   - We do not bind the walker into M11/M12; that remains a
 *     separate CSBWin custom-resource gap (FIRESTAFF_GAP_LIST.md
 *     row C3 / A3, OPEN-LARGE).
 */

#include "csb_v1_csbgraphics_dat_classify.h"
#include "csb_v1_csbgraphics_dat_lzw_boundary.h"

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

/* ── Bit-stream writer (LSB-first, matches F0495 orientation) ─── */

/* Build a CSBgraphics.dat-shaped buffer with `count` entries,
 * each entry consuming `comp_bytes` from a caller-supplied
 * per-entry payload table. The buffers + tables stay owned by
 * the caller; the walker reads through the table without
 * copying.
 *
 * Helper kept small — we only need enough variation to exercise
 * each test branch. */
typedef struct {
    uint8_t *buf;
    size_t   size;
    uint16_t *comp_table;
    uint16_t *deco_table;
    uint32_t count;
    size_t   payload_off;
} fixture;

static void fixture_free(fixture *f)
{
    if (!f) return;
    free(f->buf);
    free(f->comp_table);
    free(f->deco_table);
    memset(f, 0, sizeof(*f));
}

static int fixture_build(fixture *f, uint32_t count,
                         const uint16_t *comp_sizes,
                         const uint16_t *deco_sizes,
                         const uint8_t *const *payloads,
                         int le_marker)
{
    size_t tables = (size_t)count * 4u;
    size_t header = le_marker ? 4u : 2u;
    size_t total_payload = 0u;
    size_t i;
    size_t off;

    memset(f, 0, sizeof(*f));
    f->count = count;
    f->comp_table = (uint16_t *)calloc(count, sizeof(uint16_t));
    f->deco_table = (uint16_t *)calloc(count, sizeof(uint16_t));
    if (!f->comp_table || !f->deco_table) {
        fixture_free(f);
        return 0;
    }
    for (i = 0u; i < count; ++i) {
        total_payload += (size_t)comp_sizes[i];
        f->comp_table[i] = comp_sizes[i];
        f->deco_table[i] = deco_sizes[i];
    }
    f->size = header + tables + total_payload;
    f->buf = (uint8_t *)calloc(1u, f->size > 0u ? f->size : 1u);
    if (!f->buf) {
        fixture_free(f);
        return 0;
    }
    f->payload_off = header + tables;
    if (le_marker) {
        f->buf[0] = 0x80u; f->buf[1] = 0x01u;
        f->buf[2] = (uint8_t)(count & 0xffu);
        f->buf[3] = (uint8_t)((count >> 8) & 0xffu);
    } else {
        f->buf[0] = (uint8_t)((count >> 8) & 0xffu);
        f->buf[1] = (uint8_t)(count & 0xffu);
    }
    /* Tables. */
    off = header;
    for (i = 0u; i < count; ++i) {
        uint16_t c = comp_sizes[i];
        if (le_marker) {
            f->buf[off + 0u] = (uint8_t)(c & 0xffu);
            f->buf[off + 1u] = (uint8_t)((c >> 8) & 0xffu);
        } else {
            f->buf[off + 0u] = (uint8_t)((c >> 8) & 0xffu);
            f->buf[off + 1u] = (uint8_t)(c & 0xffu);
        }
        off += 2u;
    }
    for (i = 0u; i < count; ++i) {
        uint16_t d = deco_sizes[i];
        if (le_marker) {
            f->buf[off + 0u] = (uint8_t)(d & 0xffu);
            f->buf[off + 1u] = (uint8_t)((d >> 8) & 0xffu);
        } else {
            f->buf[off + 0u] = (uint8_t)((d >> 8) & 0xffu);
            f->buf[off + 1u] = (uint8_t)(d & 0xffu);
        }
        off += 2u;
    }
    /* Payloads. */
    for (i = 0u; i < count; ++i) {
        if (comp_sizes[i] > 0u && payloads && payloads[i]) {
            memcpy(f->buf + off, payloads[i], comp_sizes[i]);
        }
        off += comp_sizes[i];
    }
    return 1;
}

/* Build a single-entry LZW block that contains exactly the
 * given code sequence (LSB-first). Returns the number of bytes
 * written into `out`. `codes` is parallel arrays of (code,
 * code_bits) pairs terminated by an entry with code_bits == 0.
 * Caller owns `out` and must size it to at least
 * (sum of code_bits + 7) / 8 bytes.
 */
static size_t encode_lzw_sequence(const uint16_t *codes,
                                  const uint8_t  *code_bits,
                                  uint8_t *out, size_t out_cap)
{
    size_t bit_pos = 0u; /* LSB-first, counts from bit 0 */
    size_t i;
    for (i = 0u; code_bits[i] != 0u; ++i) {
        uint16_t code = codes[i];
        uint8_t  bits = code_bits[i];
        size_t b;
        for (b = 0u; b < bits; ++b) {
            size_t abs_bit = bit_pos + b;
            size_t byte_idx = abs_bit >> 3;
            size_t bit_in_byte = abs_bit & 7u;
            if (byte_idx >= out_cap) return 0u;
            if ((code >> b) & 1u) {
                out[byte_idx] |= (uint8_t)(1u << bit_in_byte);
            }
        }
        bit_pos += bits;
    }
    return (bit_pos + 7u) >> 3;
}

/* ── Tests ──────────────────────────────────────────────────────── */

static int test_argument_rejected(void)
{
    CSB_V1_CSBGraphicsIndex idx;
    uint8_t buf[8] = {0, 1, 0, 0, 0, 0, 0, 0};
    CSB_V1_CSBGraphicsLZWBoundaryReport report;
    memset(&idx, 0, sizeof(idx));
    /* bytes == NULL */
    ASSERT_TRUE(csb_v1_csbgraphics_dat_lzw_boundary_walk(
                    NULL, 8, &idx, &report) ==
                CSB_V1_CSBGRAPHICS_LZW_RESULT_ERR_ARGUMENT);
    /* index == NULL */
    ASSERT_TRUE(csb_v1_csbgraphics_dat_lzw_boundary_walk(
                    buf, sizeof(buf), NULL, &report) ==
                CSB_V1_CSBGRAPHICS_LZW_RESULT_ERR_ARGUMENT);
    /* report == NULL */
    ASSERT_TRUE(csb_v1_csbgraphics_dat_lzw_boundary_walk(
                    buf, sizeof(buf), &idx, NULL) ==
                CSB_V1_CSBGRAPHICS_LZW_RESULT_ERR_ARGUMENT);
    return 1;
}

static int test_single_literal_end_of_info(void)
{
    /* One entry, 9 bits: literal 0x42 + end-of-info. Expected
     * verdict: clean termination with codes_walked = 2. */
    fixture f;
    CSB_V1_CSBGraphicsIndex idx;
    CSB_V1_CSBGraphicsLZWBoundaryReport report;
    uint16_t codes[]   = { 0x42u, 257u };
    uint8_t  bits[]    = { 9u,    9u,   0u };
    uint8_t  payload[8] = {0};
    const uint8_t *payloads[1] = { payload };
    uint16_t comp_sizes[1] = { 0u };
    uint16_t deco_sizes[1] = { 0u };
    int rc;

    comp_sizes[0] = (uint16_t)encode_lzw_sequence(codes, bits,
                                                  payload, sizeof(payload));
    ASSERT_TRUE(comp_sizes[0] > 0u);
    deco_sizes[0] = 1u;

    ASSERT_TRUE(fixture_build(&f, 1u, comp_sizes, deco_sizes,
                              payloads, 0));
    ASSERT_TRUE(csb_v1_csbgraphics_dat_classify(f.buf, f.size, &idx) ==
                CSB_V1_CSBGRAPHICS_CLASSIFY_OK);

    memset(&report, 0, sizeof(report));
    rc = csb_v1_csbgraphics_dat_lzw_boundary_walk(f.buf, f.size,
                                                   &idx, &report);
    ASSERT_TRUE(rc == CSB_V1_CSBGRAPHICS_LZW_RESULT_OK);
    ASSERT_TRUE(report.entries_ok == 1u);
    ASSERT_TRUE(report.entries[0].result ==
                CSB_V1_CSBGRAPHICS_LZW_RESULT_OK);
    ASSERT_TRUE(report.entries[0].clean_termination == 1u);
    ASSERT_TRUE(report.entries[0].had_end_of_info_code == 1u);
    ASSERT_TRUE(report.entries[0].had_clear_code == 0u);
    ASSERT_TRUE(report.entries[0].codes_walked == 2u);
    ASSERT_TRUE(report.entries[0].code_bits_start == 9u);
    ASSERT_TRUE(report.entries[0].code_bits_end == 9u);
    ASSERT_TRUE(report.entries[0].bits_consumed == 18u);

    csb_v1_csbgraphics_dat_lzw_boundary_report_free(&report);
    fixture_free(&f);
    return 1;
}

static int test_clear_code_resets_width(void)
{
    /* One entry, 9 bits: literal + CLEAR (256) + literal +
     * EOI (257). Expected: clear_codes_seen = 1, codes_walked
     * = 4, clean_termination = 1. */
    fixture f;
    CSB_V1_CSBGraphicsIndex idx;
    CSB_V1_CSBGraphicsLZWBoundaryReport report;
    uint16_t codes[]   = { 0x10u, 256u, 0x20u, 257u };
    uint8_t  bits[]    = { 9u,    9u,   9u,    9u,   0u };
    uint8_t  payload[8] = {0};
    const uint8_t *payloads[1] = { payload };
    uint16_t comp_sizes[1] = { 0u };
    uint16_t deco_sizes[1] = { 0u };
    int rc;

    comp_sizes[0] = (uint16_t)encode_lzw_sequence(codes, bits,
                                                  payload, sizeof(payload));
    ASSERT_TRUE(comp_sizes[0] > 0u);
    deco_sizes[0] = 1u;

    ASSERT_TRUE(fixture_build(&f, 1u, comp_sizes, deco_sizes,
                              payloads, 0));
    ASSERT_TRUE(csb_v1_csbgraphics_dat_classify(f.buf, f.size, &idx) ==
                CSB_V1_CSBGRAPHICS_CLASSIFY_OK);

    memset(&report, 0, sizeof(report));
    rc = csb_v1_csbgraphics_dat_lzw_boundary_walk(f.buf, f.size,
                                                   &idx, &report);
    ASSERT_TRUE(rc == CSB_V1_CSBGRAPHICS_LZW_RESULT_OK);
    ASSERT_TRUE(report.entries_ok == 1u);
    ASSERT_TRUE(report.entries[0].clean_termination == 1u);
    ASSERT_TRUE(report.entries[0].had_clear_code == 1u);
    ASSERT_TRUE(report.entries[0].had_end_of_info_code == 1u);
    ASSERT_TRUE(report.entries[0].clear_codes_seen == 1u);
    ASSERT_TRUE(report.entries[0].end_of_info_seen == 1u);
    ASSERT_TRUE(report.entries[0].codes_walked == 4u);
    ASSERT_TRUE(report.entries[0].code_bits_end == 9u);

    csb_v1_csbgraphics_dat_lzw_boundary_report_free(&report);
    fixture_free(&f);
    return 1;
}

static int test_truncated_reports_bits_consumed(void)
{
    /* One entry, declared comp_size = 4 bytes but the bit-stream
     * never reaches an EOI. The walker should report TRUNCATED
     * with bits_consumed < bits_avail. */
    fixture f;
    CSB_V1_CSBGraphicsIndex idx;
    CSB_V1_CSBGraphicsLZWBoundaryReport report;
    uint8_t  payload[4] = { 0xFFu, 0xFFu, 0xFFu, 0xFFu };
    const uint8_t *payloads[1] = { payload };
    uint16_t comp_sizes[1] = { 4u };
    uint16_t deco_sizes[1] = { 1u };
    int rc;

    ASSERT_TRUE(fixture_build(&f, 1u, comp_sizes, deco_sizes,
                              payloads, 0));
    ASSERT_TRUE(csb_v1_csbgraphics_dat_classify(f.buf, f.size, &idx) ==
                CSB_V1_CSBGRAPHICS_CLASSIFY_OK);

    memset(&report, 0, sizeof(report));
    rc = csb_v1_csbgraphics_dat_lzw_boundary_walk(f.buf, f.size,
                                                   &idx, &report);
    ASSERT_TRUE(rc == CSB_V1_CSBGRAPHICS_LZW_RESULT_OK);
    ASSERT_TRUE(report.entries_truncated == 1u);
    ASSERT_TRUE(report.entries[0].result ==
                CSB_V1_CSBGRAPHICS_LZW_RESULT_TRUNCATED);
    ASSERT_TRUE(report.entries[0].clean_termination == 0u);
    /* The bit-stream is 0xFF padding, which is all 1s — every
     * code reads as 0x1FF (9 bits) until the width ladder
     * grows. 32 bits / 9 bits per code = 3 full codes (27
     * bits) followed by a 4th pull that cannot satisfy a
     * full 9-bit code, so the cursor reports TRUNCATED with
     * bits_consumed == 27. The width does not grow because
     * none of the codes are EOI/clear and we stop on
     * truncation before next_code crosses 511. */
    ASSERT_TRUE(report.entries[0].bits_consumed == 3u * 9u);
    ASSERT_TRUE(report.entries[0].codes_walked == 3u);
    ASSERT_TRUE(report.entries[0].code_bits_end ==
                CSB_V1_CSBGRAPHICS_LZW_MIN_BITS);

    csb_v1_csbgraphics_dat_lzw_boundary_report_free(&report);
    fixture_free(&f);
    return 1;
}

static int test_dict_full_after_width_ladder(void)
{
    /* Emit a code stream that fills the dictionary at the maximum
     * allowed width, then terminates cleanly. ReDMCSB LZW.C:185
     * only writes a new table entry while
     * G0668_i_LZW_DictionaryNextAvailableCode < 4096; reaching
     * 4096 is therefore a full-table boundary, not an overflow. */
    fixture f;
    CSB_V1_CSBGraphicsIndex idx;
    CSB_V1_CSBGraphicsLZWBoundaryReport report;
    /* 3838 literal 0x00 codes through the width ladder + EOI:
     *  - 254 codes bump next_code from 258 to 512
     *    -> first width grow 9 -> 10
     *  - 512 codes bump next_code from 512 to 1024
     *    -> width grow 10 -> 11
     *  - 1024 codes bump next_code from 1024 to 2048
     *    -> width grow 11 -> 12
     *  - 2048 codes bump next_code from 2048 to 4096
     *    -> width stays at 12 (max), dictionary is full
     *  Total codes to reach next_code = 4096: 258 + N = 4096
     *  -> N = 3838 codes. We then emit EOI (257) at 12 bits to
     *  terminate cleanly without walking past MAX_CODE. */
    {
        size_t bit_pos = 0u;
        size_t i;
        uint8_t payload[8192] = {0};
        (void)payload;
        const uint8_t *payloads[1] = { payload };
        uint16_t comp_sizes[1] = { 0u };
        uint16_t deco_sizes[1] = { 0u };

        for (i = 0u; i < 254u; ++i) {
            bit_pos += 9u;
        }
        for (i = 0u; i < 512u; ++i) {
            bit_pos += 10u;
        }
        for (i = 0u; i < 1024u; ++i) {
            bit_pos += 11u;
        }
        for (i = 0u; i < 2048u; ++i) {
            bit_pos += 12u;
        }
        /* Emit EOI (257 = 0b100000001) at the current width
         * (which is 12 bits after the width-ladder walks).
         * The walker will pull 12 bits and read 257. */
        {
            size_t abs_bit = bit_pos;
            size_t b;
            for (b = 0u; b < 12u; ++b) {
                size_t byte_idx = (abs_bit + b) >> 3;
                size_t bit_in_byte = (abs_bit + b) & 7u;
                if (byte_idx >= sizeof(payload)) break;
                if ((257u >> b) & 1u) {
                    payload[byte_idx] |= (uint8_t)(1u << bit_in_byte);
                }
            }
            bit_pos += 12u;
        }
        comp_sizes[0] = (uint16_t)((bit_pos + 7u) >> 3);
        ASSERT_TRUE(comp_sizes[0] > 0u && comp_sizes[0] <= sizeof(payload));
        deco_sizes[0] = 3838u;

        ASSERT_TRUE(fixture_build(&f, 1u, comp_sizes, deco_sizes,
                                  payloads, 0));
        ASSERT_TRUE(csb_v1_csbgraphics_dat_classify(f.buf, f.size, &idx) ==
                    CSB_V1_CSBGRAPHICS_CLASSIFY_OK);

        memset(&report, 0, sizeof(report));
        csb_v1_csbgraphics_dat_lzw_boundary_walk(f.buf, f.size,
                                                  &idx, &report);
        ASSERT_TRUE(report.entries_ok == 1u);
        ASSERT_TRUE(report.entries[0].result ==
                    CSB_V1_CSBGRAPHICS_LZW_RESULT_OK);
        ASSERT_TRUE(report.entries[0].clean_termination == 1u);
        ASSERT_TRUE(report.entries[0].had_dict_overflow == 0u);
        ASSERT_TRUE(report.entries[0].had_end_of_info_code == 1u);
        ASSERT_TRUE(report.entries[0].max_next_code ==
                    CSB_V1_CSBGRAPHICS_LZW_MAX_CODE - 1u);
        ASSERT_TRUE(report.entries[0].code_bits_end ==
                    CSB_V1_CSBGRAPHICS_LZW_MAX_BITS);

        csb_v1_csbgraphics_dat_lzw_boundary_report_free(&report);
        fixture_free(&f);
    }
    return 1;
}

static int test_width_growth_9_to_10(void)
{
    /* Force a width transition. After consuming enough codes
     * that next_code crosses 511, code_bits must grow to 10.
     * We do that by emitting 254 literal codes + 1 EOI code,
     * all at 9 bits. After 254 codes the walker has bumped
     * next_code from 258 to 512, triggering the width grow. */
    fixture f;
    CSB_V1_CSBGraphicsIndex idx;
    CSB_V1_CSBGraphicsLZWBoundaryReport report;
    uint8_t  payload[512] = {0};
    const uint8_t *payloads[1] = { payload };
    uint16_t comp_sizes[1] = { 0u };
    uint16_t deco_sizes[1] = { 0u };
    /* Encode 254 literal 0x00 + 1 EOI (257) at 9 bits. */
    {
        size_t bit_pos = 0u;
        size_t i;
        for (i = 0u; i < 254u; ++i) {
            size_t abs_bit = bit_pos;
            size_t byte_idx = abs_bit >> 3;
            /* code 0x00 = no bits set */
            (void)byte_idx;
            bit_pos += 9u;
        }
        /* Last code: 257 (EOI). 257 = 0b100000001 */
        {
            size_t abs_bit = bit_pos;
            size_t b;
            for (b = 0u; b < 9u; ++b) {
                size_t byte_idx = (abs_bit + b) >> 3;
                size_t bit_in_byte = (abs_bit + b) & 7u;
                if ((257u >> b) & 1u) {
                    payload[byte_idx] |= (uint8_t)(1u << bit_in_byte);
                }
            }
            bit_pos += 9u;
        }
        comp_sizes[0] = (uint16_t)((bit_pos + 7u) >> 3);
    }
    ASSERT_TRUE(comp_sizes[0] > 0u && comp_sizes[0] <= sizeof(payload));
    deco_sizes[0] = 254u;

    ASSERT_TRUE(fixture_build(&f, 1u, comp_sizes, deco_sizes,
                              payloads, 0));
    ASSERT_TRUE(csb_v1_csbgraphics_dat_classify(f.buf, f.size, &idx) ==
                CSB_V1_CSBGRAPHICS_CLASSIFY_OK);

    memset(&report, 0, sizeof(report));
    csb_v1_csbgraphics_dat_lzw_boundary_walk(f.buf, f.size,
                                              &idx, &report);
    ASSERT_TRUE(report.entries_ok == 1u);
    ASSERT_TRUE(report.entries[0].clean_termination == 1u);
    ASSERT_TRUE(report.entries[0].code_bits_start == 9u);
    /* After 254 codes the dict grew enough to push code_bits
     * to 10 — but the EOI itself is consumed AFTER the width
     * grow since LZW.C grows width at refill of the NEXT code.
     * The 254th code is the 254th, and the bump for the 254th
     * code makes next_code = 258 + 254 = 512 > current_max_code
     * (511). The next refill grows to 10 bits, then EOI is
     * pulled at 10 bits. So code_bits_end should be 10. */
    ASSERT_TRUE(report.entries[0].code_bits_end == 10u);
    ASSERT_TRUE(report.entries[0].dict_growth_steps >= 1u);
    /* codes_walked = 254 literals + 1 EOI = 255 */
    ASSERT_TRUE(report.entries[0].codes_walked == 255u);

    csb_v1_csbgraphics_dat_lzw_boundary_report_free(&report);
    fixture_free(&f);
    return 1;
}

static int test_empty_entry(void)
{
    /* An entry with comp_size == 0 must report EMPTY without
     * touching the cursor. */
    fixture f;
    CSB_V1_CSBGraphicsIndex idx;
    CSB_V1_CSBGraphicsLZWBoundaryReport report;
    uint16_t comp_sizes[1] = { 0u };
    uint16_t deco_sizes[1] = { 0u };
    const uint8_t *payloads[1] = { NULL };
    int rc;

    ASSERT_TRUE(fixture_build(&f, 1u, comp_sizes, deco_sizes,
                              payloads, 0));
    ASSERT_TRUE(csb_v1_csbgraphics_dat_classify(f.buf, f.size, &idx) ==
                CSB_V1_CSBGRAPHICS_CLASSIFY_OK);

    memset(&report, 0, sizeof(report));
    rc = csb_v1_csbgraphics_dat_lzw_boundary_walk(f.buf, f.size,
                                                   &idx, &report);
    ASSERT_TRUE(rc == CSB_V1_CSBGRAPHICS_LZW_RESULT_OK);
    ASSERT_TRUE(report.entries_empty == 1u);
    ASSERT_TRUE(report.entries[0].result ==
                CSB_V1_CSBGRAPHICS_LZW_RESULT_EMPTY);
    ASSERT_TRUE(report.entries[0].bits_avail == 0u);
    ASSERT_TRUE(report.entries[0].codes_walked == 0u);

    csb_v1_csbgraphics_dat_lzw_boundary_report_free(&report);
    fixture_free(&f);
    return 1;
}

static int test_multi_entry_summary(void)
{
    /* Three entries: (a) literal+EOI (clean), (b) truncated,
     * (c) empty. Verify the report-level summary counters. */
    fixture f;
    CSB_V1_CSBGraphicsIndex idx;
    CSB_V1_CSBGraphicsLZWBoundaryReport report;
    uint16_t codes_a[]   = { 0x55u, 257u };
    uint8_t  bits_a[]    = { 9u,    9u,   0u };
    uint8_t  payload_a[4] = {0};
    uint8_t  payload_b[4] = { 0xFFu, 0xFFu, 0xFFu, 0xFFu };
    const uint8_t *payloads[3] = { payload_a, payload_b, NULL };
    uint16_t comp_sizes[3] = { 0u, 4u, 0u };
    uint16_t deco_sizes[3] = { 1u, 1u, 0u };
    int rc;

    comp_sizes[0] = (uint16_t)encode_lzw_sequence(codes_a, bits_a,
                                                  payload_a,
                                                  sizeof(payload_a));
    ASSERT_TRUE(comp_sizes[0] > 0u);
    ASSERT_TRUE(fixture_build(&f, 3u, comp_sizes, deco_sizes,
                              payloads, 0));
    ASSERT_TRUE(csb_v1_csbgraphics_dat_classify(f.buf, f.size, &idx) ==
                CSB_V1_CSBGRAPHICS_CLASSIFY_OK);

    memset(&report, 0, sizeof(report));
    rc = csb_v1_csbgraphics_dat_lzw_boundary_walk(f.buf, f.size,
                                                   &idx, &report);
    ASSERT_TRUE(rc == CSB_V1_CSBGRAPHICS_LZW_RESULT_OK);
    ASSERT_TRUE(report.entry_count == 3u);
    ASSERT_TRUE(report.entries_ok == 1u);
    ASSERT_TRUE(report.entries_truncated == 1u);
    ASSERT_TRUE(report.entries_empty == 1u);
    ASSERT_TRUE(report.entries[0].clean_termination == 1u);
    ASSERT_TRUE(report.entries[1].result ==
                CSB_V1_CSBGRAPHICS_LZW_RESULT_TRUNCATED);
    ASSERT_TRUE(report.entries[2].result ==
                CSB_V1_CSBGRAPHICS_LZW_RESULT_EMPTY);

    csb_v1_csbgraphics_dat_lzw_boundary_report_free(&report);
    fixture_free(&f);
    return 1;
}

static int test_le_marker_passthrough(void)
{
    /* Same single-literal+EOI test as above but with the
     * little-endian marker. Verify that the walker still
     * locates the payload region correctly. */
    fixture f;
    CSB_V1_CSBGraphicsIndex idx;
    CSB_V1_CSBGraphicsLZWBoundaryReport report;
    uint16_t codes[]   = { 0x33u, 257u };
    uint8_t  bits[]    = { 9u,    9u,   0u };
    uint8_t  payload[8] = {0};
    const uint8_t *payloads[1] = { payload };
    uint16_t comp_sizes[1] = { 0u };
    uint16_t deco_sizes[1] = { 0u };
    int rc;

    comp_sizes[0] = (uint16_t)encode_lzw_sequence(codes, bits,
                                                  payload, sizeof(payload));
    ASSERT_TRUE(comp_sizes[0] > 0u);
    deco_sizes[0] = 1u;

    ASSERT_TRUE(fixture_build(&f, 1u, comp_sizes, deco_sizes,
                              payloads, 1));
    ASSERT_TRUE(csb_v1_csbgraphics_dat_classify(f.buf, f.size, &idx) ==
                CSB_V1_CSBGRAPHICS_CLASSIFY_OK);
    ASSERT_TRUE(idx.byte_order ==
                CSB_V1_CSBGRAPHICS_BYTE_ORDER_LITTLE_ENDIAN_MARKER);

    memset(&report, 0, sizeof(report));
    rc = csb_v1_csbgraphics_dat_lzw_boundary_walk(f.buf, f.size,
                                                   &idx, &report);
    ASSERT_TRUE(rc == CSB_V1_CSBGRAPHICS_LZW_RESULT_OK);
    ASSERT_TRUE(report.entries_ok == 1u);
    ASSERT_TRUE(report.entries[0].clean_termination == 1u);
    ASSERT_TRUE(report.entries[0].codes_walked == 2u);
    ASSERT_TRUE(report.entries[0].bits_consumed == 18u);

    csb_v1_csbgraphics_dat_lzw_boundary_report_free(&report);
    fixture_free(&f);
    return 1;
}

static int test_result_and_evidence_strings(void)
{
    const char *r;
    ASSERT_TRUE(strcmp(csb_v1_csbgraphics_dat_lzw_boundary_result_name(
                           CSB_V1_CSBGRAPHICS_LZW_RESULT_OK), "OK") == 0);
    ASSERT_TRUE(strcmp(csb_v1_csbgraphics_dat_lzw_boundary_result_name(
                           CSB_V1_CSBGRAPHICS_LZW_RESULT_TRUNCATED),
                       "truncated") == 0);
    ASSERT_TRUE(strcmp(csb_v1_csbgraphics_dat_lzw_boundary_result_name(
                           CSB_V1_CSBGRAPHICS_LZW_RESULT_OVERFLOW),
                       "overflow") == 0);
    ASSERT_TRUE(strcmp(csb_v1_csbgraphics_dat_lzw_boundary_result_name(
                           CSB_V1_CSBGRAPHICS_LZW_RESULT_RESERVED),
                       "reserved") == 0);
    ASSERT_TRUE(strcmp(csb_v1_csbgraphics_dat_lzw_boundary_result_name(
                           CSB_V1_CSBGRAPHICS_LZW_RESULT_EMPTY),
                       "empty") == 0);

    r = csb_v1_csbgraphics_dat_lzw_boundary_source_evidence();
    ASSERT_TRUE(r != NULL);
    ASSERT_TRUE(strstr(r, "ReDMCSB LZW.C") != NULL);
    ASSERT_TRUE(strstr(r, "CSBWin/Graphics.cpp") != NULL);
    ASSERT_TRUE(strstr(r, "F0495") != NULL);
    return 1;
}

/* ── Driver ─────────────────────────────────────────────────────── */

typedef int (*test_fn)(void);
struct { const char *name; test_fn fn; } tests[] = {
    { "argument-rejected",           test_argument_rejected },
    { "single-literal-end-of-info", test_single_literal_end_of_info },
    { "clear-code-resets-width",     test_clear_code_resets_width },
    { "truncated-reports-bits-consumed",
      test_truncated_reports_bits_consumed },
    { "dict-full-after-width-ladder",
      test_dict_full_after_width_ladder },
    { "width-growth-9-to-10",        test_width_growth_9_to_10 },
    { "empty-entry",                 test_empty_entry },
    { "multi-entry-summary",         test_multi_entry_summary },
    { "le-marker-passthrough",       test_le_marker_passthrough },
    { "result-and-evidence-strings", test_result_and_evidence_strings },
};

int main(void)
{
    size_t i;
    int pass = 0;
    int fail = 0;
    for (i = 0; i < sizeof(tests) / sizeof(tests[0]); ++i) {
        printf("  TEST %s\n", tests[i].name);
        if (tests[i].fn()) {
            printf("  PASS %s\n", tests[i].name);
            ++pass;
        } else {
            printf("  FAIL %s\n", tests[i].name);
            ++fail;
        }
    }
    printf("Result: %d/%d PASS\n", pass, pass + fail);
    return fail == 0 ? 0 : 1;
}
