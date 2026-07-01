/*
 * test_csb_v1_csbgraphics_dat_classify.c
 *
 * Data-free contract tests for the CSBWin "CSBgraphics.dat"
 * index classifier.
 *
 * Scope:
 *   - Bytes-header parsing: 2-byte count, two parallel uint16
 *     tables (compressed-size + decompressed-size), payload-offset
 *     derivation matches CSBWin/Graphics.cpp:1643 LocateNthGraphic.
 *   - Byte-order detection via the 0x8001 little-endian sentinel
 *     documented at CSBWin/Graphics.cpp:1918-1934.
 *   - Bounds: rejects too-small input, empty (count==0), oversized
 *     count, and total-compressed > available payload bytes.
 *   - Payload-span handoff: one entry maps to a compressed byte
 *     range via the same LocateNthGraphic(n) offset math, including
 *     zero-length entries and little-endian-marker tables.
 *   - Bounded payload decode: one entry can be decompressed through
 *     the existing ReDMCSB-compatible DM1 graphics LZW decoder when
 *     the declared output size and caller capacity match.
 *   - Diagnostic preservation: max_compressed / max_decompressed
 *     track the largest single entry across the file.
 *   - Source evidence + result-name strings are non-empty so the
 *     probe and docs can quote them.
 *
 * Non-claims:
 *   - No real CSBWin "CSBgraphics.dat" is loaded.
 *   - No bitmap interpretation, no overlay blit, no runtime override.
 *   - We do not bind the classifier into M11/M12; that remains a
 *     separate CSBWin custom-resource gap (FIRESTAFF_GAP_LIST.md
 *     row C3 / A3, OPEN-LARGE).
 */

#include "csb_v1_csbgraphics_dat_classify.h"
#include "dm1_v1_graphics_loader_pc34_compat.h"

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

/* ── Fixture builders ───────────────────────────────────────────── */

/* Build a big-endian CSBgraphics.dat-shape buffer with `count`
 * graphics, all entries sized `comp` / `deco`. */
static uint8_t *build_big_endian(size_t count,
                                 uint16_t comp,
                                 uint16_t deco,
                                 size_t *out_size)
{
    /* Total payload pad must comfortably exceed (comp * count) so the
     * classifier does not trip its overflow guard. We default to
     * 4096 bytes so post-mutation test fixtures can bump single
     * entries into the kilobyte range without retuning the pad. */
    size_t tables = (size_t)count * 4u;
    size_t pad = (size_t)comp * count + 64u;
    if (pad < 4096u) pad = 4096u;
    size_t total = 2u + tables + pad;
    uint8_t *buf = (uint8_t *)calloc(1u, total);
    size_t i;
    if (!buf) return NULL;
    buf[0] = (uint8_t)((count >> 8) & 0xffu);
    buf[1] = (uint8_t)(count & 0xffu);
    for (i = 0u; i < count; ++i) {
        size_t comp_off = 2u + i * 2u;
        size_t deco_off = 2u + tables / 2u + i * 2u;
        buf[comp_off]     = (uint8_t)((comp >> 8) & 0xffu);
        buf[comp_off + 1] = (uint8_t)(comp & 0xffu);
        buf[deco_off]     = (uint8_t)((deco >> 8) & 0xffu);
        buf[deco_off + 1] = (uint8_t)(deco & 0xffu);
    }
    *out_size = total;
    return buf;
}

/* Build a little-endian-marker CSBgraphics.dat-shape buffer. */
static uint8_t *build_le_marker(size_t count,
                                uint16_t comp,
                                uint16_t deco,
                                size_t *out_size)
{
    size_t tables = (size_t)count * 4u;
    size_t pad = (size_t)comp * count + 64u;
    if (pad < 4096u) pad = 4096u;
    size_t total = 4u + tables + pad;
    uint8_t *buf = (uint8_t *)calloc(1u, total);
    size_t i;
    if (!buf) return NULL;
    buf[0] = 0x80u;
    buf[1] = 0x01u; /* little-endian sentinel */
    buf[2] = (uint8_t)(count & 0xffu);
    buf[3] = (uint8_t)((count >> 8) & 0xffu);
    for (i = 0u; i < count; ++i) {
        size_t comp_off = 4u + i * 2u;
        size_t deco_off = 4u + tables / 2u + i * 2u;
        buf[comp_off]     = (uint8_t)(comp & 0xffu);
        buf[comp_off + 1] = (uint8_t)((comp >> 8) & 0xffu);
        buf[deco_off]     = (uint8_t)(deco & 0xffu);
        buf[deco_off + 1] = (uint8_t)((deco >> 8) & 0xffu);
    }
    *out_size = total;
    return buf;
}

static void write_be16(uint8_t *buf, size_t off, uint16_t value)
{
    buf[off] = (uint8_t)((value >> 8) & 0xffu);
    buf[off + 1u] = (uint8_t)(value & 0xffu);
}

static void write_le16(uint8_t *buf, size_t off, uint16_t value)
{
    buf[off] = (uint8_t)(value & 0xffu);
    buf[off + 1u] = (uint8_t)((value >> 8) & 0xffu);
}

/* ── Test-side LZW encoder ──────────────────────────────────────── */

typedef struct {
    uint8_t *buf;
    size_t cap;
    size_t bit_pos;
} BitWriter;

static void bw_init(BitWriter *bw)
{
    bw->cap = 1024u;
    bw->buf = (uint8_t *)calloc(1u, bw->cap);
    bw->bit_pos = 0u;
}

static int bw_grow(BitWriter *bw)
{
    size_t old_cap = bw->cap;
    size_t new_cap = bw->cap * 2u;
    uint8_t *new_buf = (uint8_t *)realloc(bw->buf, new_cap);
    if (!new_buf) {
        free(bw->buf);
        bw->buf = NULL;
        bw->cap = 0u;
        return 0;
    }
    memset(new_buf + old_cap, 0, new_cap - old_cap);
    bw->buf = new_buf;
    bw->cap = new_cap;
    return 1;
}

static int bw_write_bits(BitWriter *bw, uint32_t value, int n_bits)
{
    int i;
    for (i = 0; i < n_bits; ++i) {
        size_t bp = bw->bit_pos++;
        size_t byte_idx;
        int bit_in_byte;
        if ((bp >> 3) >= bw->cap && !bw_grow(bw)) {
            return 0;
        }
        byte_idx = bp >> 3;
        bit_in_byte = (int)(bp & 7u);
        if (value & (1u << (uint32_t)i)) {
            bw->buf[byte_idx] |= (uint8_t)(1u << (uint32_t)bit_in_byte);
        }
    }
    return 1;
}

typedef struct {
    uint8_t dict_first[4096];
    uint16_t dict_prefix[4096];
    int dict_count;
    int code_bits;
} RefLZW;

static void ref_lzw_init(RefLZW *e)
{
    int i;
    e->dict_count = DM1_GFX_LZW_FIRST_CODE;
    e->code_bits = 9;
    for (i = 0; i < 256; ++i) {
        e->dict_first[i] = (uint8_t)i;
        e->dict_prefix[i] = 0xffffu;
    }
}

static int ref_lzw_find_or_add(RefLZW *e, uint16_t prefix, uint8_t append)
{
    int i;
    for (i = DM1_GFX_LZW_FIRST_CODE; i < e->dict_count; ++i) {
        if (e->dict_prefix[i] == prefix && e->dict_first[i] == append) {
            return i;
        }
    }
    if (e->dict_count >= DM1_GFX_LZW_MAX_CODE) {
        return -1;
    }
    e->dict_prefix[e->dict_count] = prefix;
    e->dict_first[e->dict_count] = append;
    ++e->dict_count;
    return -1;
}

static void ref_lzw_maybe_grow(RefLZW *e)
{
    if (e->dict_count > ((1 << e->code_bits) - 1) && e->code_bits < 12) {
        ++e->code_bits;
    }
}

static int ref_lzw_encode(const uint8_t *input, size_t in_size,
                          uint8_t **out_buf, size_t *out_size)
{
    BitWriter bw;
    RefLZW e;
    uint16_t prefix_code;
    size_t i;

    if (!input || !out_buf || !out_size) {
        return -1;
    }
    *out_buf = NULL;
    *out_size = 0u;

    bw_init(&bw);
    if (!bw.buf) {
        return -1;
    }
    ref_lzw_init(&e);

    if (!bw_write_bits(&bw, DM1_GFX_LZW_CLEAR_CODE, e.code_bits)) {
        return -1;
    }
    if (in_size == 0u) {
        if (!bw_write_bits(&bw, DM1_GFX_LZW_END_CODE, e.code_bits)) {
            return -1;
        }
        *out_buf = bw.buf;
        *out_size = (bw.bit_pos + 7u) / 8u;
        return 0;
    }

    prefix_code = input[0];
    for (i = 1u; i < in_size; ++i) {
        uint8_t next_byte = input[i];
        int existing = ref_lzw_find_or_add(&e, prefix_code, next_byte);
        if (existing >= 0) {
            prefix_code = (uint16_t)existing;
        } else {
            if (!bw_write_bits(&bw, prefix_code, e.code_bits)) {
                return -1;
            }
            ref_lzw_maybe_grow(&e);
            prefix_code = next_byte;
        }
    }
    if (!bw_write_bits(&bw, prefix_code, e.code_bits)) {
        return -1;
    }
    if (!bw_write_bits(&bw, DM1_GFX_LZW_END_CODE, e.code_bits)) {
        return -1;
    }

    *out_buf = bw.buf;
    *out_size = (bw.bit_pos + 7u) / 8u;
    return 0;
}

/* ── Tests ──────────────────────────────────────────────────────── */

static int test_argument_rejected(void)
{
    CSB_V1_CSBGraphicsIndex idx;
    uint8_t buf[16] = {0};
    ASSERT_TRUE(csb_v1_csbgraphics_dat_classify(NULL, 16, &idx) ==
                CSB_V1_CSBGRAPHICS_CLASSIFY_ERR_ARGUMENT);
    ASSERT_TRUE(csb_v1_csbgraphics_dat_classify(buf, 16, NULL) ==
                CSB_V1_CSBGRAPHICS_CLASSIFY_ERR_ARGUMENT);
    return 1;
}

static int test_too_small_rejected(void)
{
    CSB_V1_CSBGraphicsIndex idx;
    uint8_t buf[3] = {0, 1, 2};
    /* 3 bytes < 4 (LE marker + count) minimum */
    ASSERT_TRUE(csb_v1_csbgraphics_dat_classify(buf, sizeof(buf), &idx) ==
                CSB_V1_CSBGRAPHICS_CLASSIFY_ERR_TOO_SMALL);
    return 1;
}

static int test_empty_count_rejected(void)
{
    CSB_V1_CSBGraphicsIndex idx;
    uint8_t buf[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    ASSERT_TRUE(csb_v1_csbgraphics_dat_classify(buf, sizeof(buf), &idx) ==
                CSB_V1_CSBGRAPHICS_CLASSIFY_ERR_BAD_COUNT);
    return 1;
}

static int test_oversized_count_rejected(void)
{
    CSB_V1_CSBGraphicsIndex idx;
    size_t size = 0u;
    /* count = 0x9000 > CSB_V1_CSBGRAPHICS_MAX_COUNT (8192) */
    uint8_t *buf = build_big_endian(0x9000u, 0u, 0u, &size);
    ASSERT_TRUE(buf != NULL);
    ASSERT_TRUE(csb_v1_csbgraphics_dat_classify(buf, size, &idx) ==
                CSB_V1_CSBGRAPHICS_CLASSIFY_ERR_BAD_COUNT);
    free(buf);
    return 1;
}

static int test_big_endian_round_trip(void)
{
    CSB_V1_CSBGraphicsIndex idx;
    size_t size = 0u;
    uint8_t *buf = build_big_endian(3u, 0x0123u, 0x0456u, &size);
    int rc;
    ASSERT_TRUE(buf != NULL);
    memset(&idx, 0, sizeof(idx));
    rc = csb_v1_csbgraphics_dat_classify(buf, size, &idx);
    ASSERT_TRUE(rc == CSB_V1_CSBGRAPHICS_CLASSIFY_OK);
    ASSERT_TRUE(idx.byte_order ==
                CSB_V1_CSBGRAPHICS_BYTE_ORDER_BIG_ENDIAN);
    ASSERT_TRUE(idx.count == 3u);
    ASSERT_TRUE(idx.total_compressed == (uint64_t)(0x0123u * 3u));
    ASSERT_TRUE(idx.total_decompressed == (uint64_t)(0x0456u * 3u));
    ASSERT_TRUE(idx.max_compressed == 0x0123u);
    ASSERT_TRUE(idx.max_decompressed == 0x0456u);
    /* payload_offset = 2 (count header) + 3 * 4 (tables) */
    ASSERT_TRUE(idx.payload_offset == 2u + 12u);
    /* payload_bytes_avail = total - payload_offset */
    ASSERT_TRUE(idx.payload_bytes_avail == size - idx.payload_offset);
    free(buf);
    return 1;
}

static int test_le_marker_round_trip(void)
{
    CSB_V1_CSBGraphicsIndex idx;
    size_t size = 0u;
    uint8_t *buf = build_le_marker(7u, 0x0010u, 0x0020u, &size);
    int rc;
    ASSERT_TRUE(buf != NULL);
    memset(&idx, 0, sizeof(idx));
    rc = csb_v1_csbgraphics_dat_classify(buf, size, &idx);
    ASSERT_TRUE(rc == CSB_V1_CSBGRAPHICS_CLASSIFY_OK);
    ASSERT_TRUE(idx.byte_order ==
                CSB_V1_CSBGRAPHICS_BYTE_ORDER_LITTLE_ENDIAN_MARKER);
    ASSERT_TRUE(idx.count == 7u);
    ASSERT_TRUE(idx.total_compressed == (uint64_t)(0x0010u * 7u));
    ASSERT_TRUE(idx.total_decompressed == (uint64_t)(0x0020u * 7u));
    /* payload_offset = 4 (LE marker + count) + 7 * 4 (tables) */
    ASSERT_TRUE(idx.payload_offset == 4u + 28u);
    ASSERT_TRUE(idx.payload_bytes_avail == size - idx.payload_offset);
    free(buf);
    return 1;
}

static int test_total_compressed_overflow_rejected(void)
{
    CSB_V1_CSBGraphicsIndex idx;
    size_t size = 0u;
    /* 2 entries × 0x4000 bytes = 0x8000 bytes compressed.
     * Tables only consume 2 + 2*4 = 10 bytes, then we add only
     * 4 bytes of "payload" padding — total_compressed (0x8000)
     * will exceed payload_avail (4) and the parser must reject. */
    uint8_t *buf = build_big_endian(2u, 0x4000u, 0x4000u, &size);
    /* Trim the calloc-padded payload so overflow actually trips. */
    size_t trim = 10u + 4u;
    int rc;
    ASSERT_TRUE(buf != NULL);
    rc = csb_v1_csbgraphics_dat_classify(buf, trim, &idx);
    ASSERT_TRUE(rc == CSB_V1_CSBGRAPHICS_CLASSIFY_ERR_OVERFLOW);
    free(buf);
    return 1;
}

static int test_le_marker_too_small_rejected(void)
{
    CSB_V1_CSBGraphicsIndex idx;
    uint8_t buf[3] = {0x80u, 0x01u, 0x05u};
    /* LE marker present but file is only 3 bytes (< 4 minimum). */
    ASSERT_TRUE(csb_v1_csbgraphics_dat_classify(buf, sizeof(buf), &idx) ==
                CSB_V1_CSBGRAPHICS_CLASSIFY_ERR_TOO_SMALL);
    return 1;
}

static int test_truncated_tables_rejected(void)
{
    CSB_V1_CSBGraphicsIndex idx;
    size_t size = 0u;
    uint8_t *buf = build_big_endian(4u, 0x0010u, 0x0020u, &size);
    /* Tables need 2 + 4 * 4 = 18 bytes; we keep only 10. */
    int rc;
    ASSERT_TRUE(buf != NULL);
    rc = csb_v1_csbgraphics_dat_classify(buf, 10u, &idx);
    ASSERT_TRUE(rc == CSB_V1_CSBGRAPHICS_CLASSIFY_ERR_TOO_SMALL);
    free(buf);
    return 1;
}

static int test_max_tracking(void)
{
    CSB_V1_CSBGraphicsIndex idx;
    size_t size = 0u;
    /* Build a 4-entry file where entry 2 is the largest. We do
     * this by post-mutating the build_big_endian output. The
     * base fixture uses small per-entry sizes so the post-mutation
     * total still fits inside the fixture's payload pad. */
    uint8_t *buf = build_big_endian(4u, 0x0001u, 0x0002u, &size);
    /* Tables start at offset 2; comp entries are at 2..9,
     * deco entries at 10..17. Bump entry 2's compressed and
     * decompressed sizes. */
    int rc;
    ASSERT_TRUE(buf != NULL);
    buf[2 + 2u * 2u + 0u] = 0x05u; /* comp entry 2 = 0x0500 */
    buf[2 + 2u * 2u + 1u] = 0x00u;
    buf[2 + 8u + 2u * 2u + 0u] = 0x07u; /* deco entry 2 = 0x0700 */
    buf[2 + 8u + 2u * 2u + 1u] = 0x00u;
    memset(&idx, 0, sizeof(idx));
    rc = csb_v1_csbgraphics_dat_classify(buf, size, &idx);
    ASSERT_TRUE(rc == CSB_V1_CSBGRAPHICS_CLASSIFY_OK);
    ASSERT_TRUE(idx.max_compressed == 0x0500u);
    ASSERT_TRUE(idx.max_decompressed == 0x0700u);
    /* total = 3 * 0x01 + 0x0500 = 0x0503; 3 * 0x02 + 0x0700 = 0x0706 */
    ASSERT_TRUE(idx.total_compressed == (uint64_t)0x0503u);
    ASSERT_TRUE(idx.total_decompressed == (uint64_t)0x0706u);
    free(buf);
    return 1;
}

static int test_big_endian_entry_spans(void)
{
    CSB_V1_CSBGraphicsEntrySpan span;
    CSB_V1_CSBGraphicsIndex idx;
    size_t size = 0u;
    uint8_t *buf = build_big_endian(4u, 0x0001u, 0x0002u, &size);
    int rc;
    ASSERT_TRUE(buf != NULL);

    /* Compressed sizes: [3, 0, 5, 2].
     * Decompressed sizes: [30, 0, 50, 20]. */
    write_be16(buf, 2u + 0u, 3u);
    write_be16(buf, 2u + 2u, 0u);
    write_be16(buf, 2u + 4u, 5u);
    write_be16(buf, 2u + 6u, 2u);
    write_be16(buf, 10u + 0u, 30u);
    write_be16(buf, 10u + 2u, 0u);
    write_be16(buf, 10u + 4u, 50u);
    write_be16(buf, 10u + 6u, 20u);

    rc = csb_v1_csbgraphics_dat_classify(buf, size, &idx);
    ASSERT_TRUE(rc == CSB_V1_CSBGRAPHICS_CLASSIFY_OK);
    ASSERT_TRUE(idx.payload_offset == 18u);
    ASSERT_TRUE(idx.total_compressed == 10u);

    memset(&span, 0, sizeof(span));
    rc = csb_v1_csbgraphics_dat_entry_span(buf, size, 0u, &span);
    ASSERT_TRUE(rc == CSB_V1_CSBGRAPHICS_CLASSIFY_OK);
    ASSERT_TRUE(span.entry_index == 0u);
    ASSERT_TRUE(span.payload_offset == 18u);
    ASSERT_TRUE(span.compressed_size == 3u);
    ASSERT_TRUE(span.decompressed_size == 30u);

    rc = csb_v1_csbgraphics_dat_entry_span(buf, size, 1u, &span);
    ASSERT_TRUE(rc == CSB_V1_CSBGRAPHICS_CLASSIFY_OK);
    ASSERT_TRUE(span.payload_offset == 21u);
    ASSERT_TRUE(span.compressed_size == 0u);
    ASSERT_TRUE(span.decompressed_size == 0u);

    rc = csb_v1_csbgraphics_dat_entry_span(buf, size, 2u, &span);
    ASSERT_TRUE(rc == CSB_V1_CSBGRAPHICS_CLASSIFY_OK);
    ASSERT_TRUE(span.payload_offset == 21u);
    ASSERT_TRUE(span.compressed_size == 5u);
    ASSERT_TRUE(span.decompressed_size == 50u);

    rc = csb_v1_csbgraphics_dat_entry_span(buf, size, 3u, &span);
    ASSERT_TRUE(rc == CSB_V1_CSBGRAPHICS_CLASSIFY_OK);
    ASSERT_TRUE(span.payload_offset == 26u);
    ASSERT_TRUE(span.compressed_size == 2u);
    ASSERT_TRUE(span.decompressed_size == 20u);

    free(buf);
    return 1;
}

static int test_le_marker_entry_spans(void)
{
    CSB_V1_CSBGraphicsEntrySpan span;
    size_t size = 0u;
    uint8_t *buf = build_le_marker(3u, 0x0001u, 0x0002u, &size);
    int rc;
    ASSERT_TRUE(buf != NULL);

    /* Compressed sizes: [4, 6, 8].
     * Decompressed sizes: [40, 60, 80]. */
    write_le16(buf, 4u + 0u, 4u);
    write_le16(buf, 4u + 2u, 6u);
    write_le16(buf, 4u + 4u, 8u);
    write_le16(buf, 10u + 0u, 40u);
    write_le16(buf, 10u + 2u, 60u);
    write_le16(buf, 10u + 4u, 80u);

    rc = csb_v1_csbgraphics_dat_entry_span(buf, size, 2u, &span);
    ASSERT_TRUE(rc == CSB_V1_CSBGRAPHICS_CLASSIFY_OK);
    /* payload_offset = 4 (marker+count) + 3*4 tables = 16.
     * preceding compressed bytes before entry 2: 4 + 6 = 10. */
    ASSERT_TRUE(span.entry_index == 2u);
    ASSERT_TRUE(span.payload_offset == 26u);
    ASSERT_TRUE(span.compressed_size == 8u);
    ASSERT_TRUE(span.decompressed_size == 80u);

    free(buf);
    return 1;
}

static int test_entry_span_range_rejected(void)
{
    CSB_V1_CSBGraphicsEntrySpan span;
    size_t size = 0u;
    uint8_t *buf = build_big_endian(2u, 0x0004u, 0x0008u, &size);
    int rc;
    ASSERT_TRUE(buf != NULL);
    rc = csb_v1_csbgraphics_dat_entry_span(buf, size, 2u, &span);
    ASSERT_TRUE(rc == CSB_V1_CSBGRAPHICS_CLASSIFY_ERR_ENTRY_RANGE);
    free(buf);
    return 1;
}

static int test_entry_span_argument_rejected(void)
{
    size_t size = 0u;
    uint8_t *buf = build_big_endian(1u, 0x0004u, 0x0008u, &size);
    CSB_V1_CSBGraphicsEntrySpan span;
    int rc;
    ASSERT_TRUE(buf != NULL);
    rc = csb_v1_csbgraphics_dat_entry_span(buf, size, 0u, NULL);
    ASSERT_TRUE(rc == CSB_V1_CSBGRAPHICS_CLASSIFY_ERR_ARGUMENT);
    rc = csb_v1_csbgraphics_dat_entry_span(NULL, size, 0u, &span);
    ASSERT_TRUE(rc == CSB_V1_CSBGRAPHICS_CLASSIFY_ERR_ARGUMENT);
    free(buf);
    return 1;
}

static int test_decode_entry_big_endian_lzw_round_trip(void)
{
    static const uint8_t input[] = "CSBGRAPHICS_PAYLOAD";
    uint8_t *compressed = NULL;
    size_t compressed_size = 0u;
    size_t size = 0u;
    uint8_t *buf;
    CSB_V1_CSBGraphicsIndex idx;
    uint8_t out[64];
    size_t written = 999u;
    int rc;

    rc = ref_lzw_encode(input, sizeof(input) - 1u,
                        &compressed, &compressed_size);
    ASSERT_TRUE(rc == 0);
    ASSERT_TRUE(compressed != NULL);
    ASSERT_TRUE(compressed_size > 0u);
    ASSERT_TRUE(compressed_size <= 0xffffu);

    buf = build_big_endian(3u, 0u, 0u, &size);
    ASSERT_TRUE(buf != NULL);
    write_be16(buf, 2u + 2u, (uint16_t)compressed_size);
    write_be16(buf, 8u + 2u, (uint16_t)(sizeof(input) - 1u));

    memset(&idx, 0, sizeof(idx));
    rc = csb_v1_csbgraphics_dat_classify(buf, size, &idx);
    ASSERT_TRUE(rc == CSB_V1_CSBGRAPHICS_CLASSIFY_OK);
    memcpy(buf + (size_t)idx.payload_offset, compressed, compressed_size);

    memset(out, 0, sizeof(out));
    rc = csb_v1_csbgraphics_dat_decode_entry(buf, size, 1u,
                                             out, sizeof(out), &written);
    ASSERT_TRUE(rc == CSB_V1_CSBGRAPHICS_CLASSIFY_OK);
    ASSERT_TRUE(written == sizeof(input) - 1u);
    ASSERT_TRUE(memcmp(out, input, sizeof(input) - 1u) == 0);

    free(compressed);
    free(buf);
    return 1;
}

static int test_decode_entry_output_too_small(void)
{
    static const uint8_t input[] = "CSB_OUT";
    uint8_t *compressed = NULL;
    size_t compressed_size = 0u;
    size_t size = 0u;
    uint8_t *buf;
    CSB_V1_CSBGraphicsIndex idx;
    uint8_t out[4];
    size_t written = 999u;
    int rc;

    rc = ref_lzw_encode(input, sizeof(input) - 1u,
                        &compressed, &compressed_size);
    ASSERT_TRUE(rc == 0);
    ASSERT_TRUE(compressed != NULL);

    buf = build_big_endian(1u, 0u, 0u, &size);
    ASSERT_TRUE(buf != NULL);
    write_be16(buf, 2u, (uint16_t)compressed_size);
    write_be16(buf, 4u, (uint16_t)(sizeof(input) - 1u));
    rc = csb_v1_csbgraphics_dat_classify(buf, size, &idx);
    ASSERT_TRUE(rc == CSB_V1_CSBGRAPHICS_CLASSIFY_OK);
    memcpy(buf + (size_t)idx.payload_offset, compressed, compressed_size);

    rc = csb_v1_csbgraphics_dat_decode_entry(buf, size, 0u,
                                             out, sizeof(out), &written);
    ASSERT_TRUE(rc == CSB_V1_CSBGRAPHICS_CLASSIFY_ERR_OUTPUT_TOO_SMALL);
    ASSERT_TRUE(written == 0u);

    free(compressed);
    free(buf);
    return 1;
}

static int test_decode_empty_entry(void)
{
    size_t size = 0u;
    uint8_t *buf = build_big_endian(1u, 0u, 0u, &size);
    size_t written = 777u;
    int rc;
    ASSERT_TRUE(buf != NULL);
    rc = csb_v1_csbgraphics_dat_decode_entry(buf, size, 0u,
                                             NULL, 0u, &written);
    ASSERT_TRUE(rc == CSB_V1_CSBGRAPHICS_CLASSIFY_OK);
    ASSERT_TRUE(written == 0u);
    free(buf);
    return 1;
}

static int test_decode_bad_lzw_rejected(void)
{
    size_t size = 0u;
    uint8_t *buf = build_big_endian(1u, 0u, 0u, &size);
    CSB_V1_CSBGraphicsIndex idx;
    uint8_t out[16];
    size_t written = 777u;
    int rc;
    ASSERT_TRUE(buf != NULL);
    write_be16(buf, 2u, 3u);
    write_be16(buf, 4u, 8u);
    rc = csb_v1_csbgraphics_dat_classify(buf, size, &idx);
    ASSERT_TRUE(rc == CSB_V1_CSBGRAPHICS_CLASSIFY_OK);
    memset(buf + (size_t)idx.payload_offset, 0xff, 3u);
    rc = csb_v1_csbgraphics_dat_decode_entry(buf, size, 0u,
                                             out, sizeof(out), &written);
    ASSERT_TRUE(rc == CSB_V1_CSBGRAPHICS_CLASSIFY_ERR_BAD_LZW);
    ASSERT_TRUE(written == 0u);
    free(buf);
    return 1;
}

static int test_result_and_evidence_strings(void)
{
    const char *r;
    ASSERT_TRUE(csb_v1_csbgraphics_dat_result_name(
                    CSB_V1_CSBGRAPHICS_CLASSIFY_OK) != NULL);
    ASSERT_TRUE(strcmp(csb_v1_csbgraphics_dat_result_name(
                           CSB_V1_CSBGRAPHICS_CLASSIFY_OK), "OK") == 0);
    ASSERT_TRUE(strcmp(csb_v1_csbgraphics_dat_result_name(
                           CSB_V1_CSBGRAPHICS_CLASSIFY_ERR_OVERFLOW),
                       "overflow") == 0);
    ASSERT_TRUE(strcmp(csb_v1_csbgraphics_dat_result_name(
                           CSB_V1_CSBGRAPHICS_CLASSIFY_ERR_ENTRY_RANGE),
                       "entry-range") == 0);
    ASSERT_TRUE(strcmp(csb_v1_csbgraphics_dat_result_name(
                           CSB_V1_CSBGRAPHICS_CLASSIFY_ERR_OUTPUT_TOO_SMALL),
                       "output-too-small") == 0);
    ASSERT_TRUE(strcmp(csb_v1_csbgraphics_dat_result_name(
                           CSB_V1_CSBGRAPHICS_CLASSIFY_ERR_BAD_LZW),
                       "bad-lzw") == 0);

    ASSERT_TRUE(strcmp(
        csb_v1_csbgraphics_dat_byte_order_name(
            CSB_V1_CSBGRAPHICS_BYTE_ORDER_BIG_ENDIAN),
        "big-endian") == 0);
    ASSERT_TRUE(strcmp(
        csb_v1_csbgraphics_dat_byte_order_name(
            CSB_V1_CSBGRAPHICS_BYTE_ORDER_LITTLE_ENDIAN_MARKER),
        "little-endian-marker") == 0);

    r = csb_v1_csbgraphics_dat_source_evidence();
    ASSERT_TRUE(r != NULL);
    /* Must mention CSBWin Graphics.cpp at minimum. */
    ASSERT_TRUE(strstr(r, "CSBWin/Graphics.cpp") != NULL);
    ASSERT_TRUE(strstr(r, "LZW.C") != NULL);
    ASSERT_TRUE(strstr(r, "ReadGraphicsIndex") != NULL ||
                strstr(r, "LocateNthGraphic") != NULL);
    return 1;
}

/* ── Driver ─────────────────────────────────────────────────────── */

typedef int (*test_fn)(void);
struct { const char *name; test_fn fn; } tests[] = {
    { "argument-rejected",        test_argument_rejected },
    { "too-small-rejected",       test_too_small_rejected },
    { "empty-count-rejected",     test_empty_count_rejected },
    { "oversized-count-rejected", test_oversized_count_rejected },
    { "big-endian-round-trip",    test_big_endian_round_trip },
    { "le-marker-round-trip",     test_le_marker_round_trip },
    { "total-compressed-overflow-rejected",
      test_total_compressed_overflow_rejected },
    { "le-marker-too-small-rejected",
      test_le_marker_too_small_rejected },
    { "truncated-tables-rejected",
      test_truncated_tables_rejected },
    { "max-tracking",             test_max_tracking },
    { "big-endian-entry-spans",    test_big_endian_entry_spans },
    { "le-marker-entry-spans",     test_le_marker_entry_spans },
    { "entry-span-range-rejected", test_entry_span_range_rejected },
    { "entry-span-argument-rejected",
      test_entry_span_argument_rejected },
    { "decode-entry-big-endian-lzw-round-trip",
      test_decode_entry_big_endian_lzw_round_trip },
    { "decode-entry-output-too-small",
      test_decode_entry_output_too_small },
    { "decode-empty-entry",        test_decode_empty_entry },
    { "decode-bad-lzw-rejected",   test_decode_bad_lzw_rejected },
    { "result-and-evidence-strings",
      test_result_and_evidence_strings },
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
