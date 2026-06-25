/*
 * firestaff_ftl_hunk_data_zero_run.c
 *
 * HUNK_DATA zero-run decompression for the FTL container format.
 *
 * Source of truth:
 *   greatstone d_ftl.html, "Technical documentation - FTL file format",
 *   "Note 7: How to decompress HUNK_DATA"
 *   (Pierre Monnot's Swoosh Construction Kit documentation).
 *
 * The Note 7 algorithm is intentionally simple: it shrinks runs of
 * consecutive 0x00 bytes in the in-memory area_1 resource pool. The
 * loop walks the compressed stream 2 bytes at a time, but a "00 00"
 * pair is itself always copied through and then expanded by a 16-bit
 * run length. The surrounding FTL container is big-endian, so the
 * bounded decoder treats this word the same way.
 *
 * This file is a direct port of that pseudocode into C11, with the
 * addition of explicit bounds checks (so a malformed FTL cannot
 * overrun the output buffer) and a few helper functions to make the
 * tests readable.
 *
 * Out of scope on purpose:
 *   - HUNK_CODE 0x5223 decompression (different algorithm; see
 *     firestaff_pak_decode.c, which shares the spec but operates on
 *     the Atari ST PAK wrapper).
 *   - HUNK_DATA mapfile-driven item extraction (greatstone
 *     d_mapfile.html). That is a separate, larger pass.
 *   - Checksum recomputation (firestaff_ftl_container.c owns the
 *     common / BSS / DATA / uncompressed-CODE checksums per
 *     Notes 1, 2, 4, and 5).
 *   - Runtime loading. We do not hand the decoded bytes to any
 *     asset loader here; the decoder is bounded and data-free so it
 *     can stay CTest-friendly.
 */

#include "firestaff_ftl_hunk_data_zero_run.h"

#include <stdio.h>
#include <string.h>

/* The Note 7 pseudocode reads 2 bytes for the literal pair, or
 * 4 bytes for a "00 00" + 16-bit additional-count run header. We
 * cap the input length so that we can sum pair counts without
 * worrying about size_t overflow in the worst-case expansion. The
 * cap is generous (32 MiB) but enough that any real FTL HUNK_DATA
 * area_1 will fit. If somebody hands us a bigger buffer we refuse
 * rather than silently allocating gigabytes. */
#define FIRESTAFF_FTL_HUNK_DATA_MAX_INPUT (32u * 1024u * 1024u)

/* 16-bit additional-count ceiling: matches the greatstone word()
 * interpretation (uint16). Note that the pseudocode uses
 * `int additional_0x00 = word(area_1[i+2], area_1[i+3])` and never
 * specifies endianness; greatstone's tools run on Atari ST / Amiga
 * (big-endian), and the rest of the FTL container (greatstone
 * d_ftl.html Notes 1, 2, 4, 5 + the common header "20-byte common
 * header" + "12-byte hunk headers") is big-endian throughout, so we
 * interpret the run count as big-endian. */
static uint16_t read_be16(const uint8_t* p) {
    return (uint16_t)(((uint16_t)p[0] << 8) | (uint16_t)p[1]);
}

FirestaffFtlHunkDataStatus FirestaffFtlHunkData_DecompressZeroRun(
    const uint8_t* compressed,
    size_t compressed_size,
    size_t uncompressed_size,
    uint8_t* out,
    size_t* out_written) {
    if (out_written) *out_written = 0u;

    /* Argument validation. The Note 7 pseudocode has no failure mode;
     * we add one so callers can react to malformed FTL inputs. */
    if (!out_written) {
        return FIRESTAFF_FTL_HUNK_DATA_ERR_ARG;
    }
    if (compressed_size == 0u) {
        /* An empty compressed area_1 must correspond to an empty
         * uncompressed area_1; otherwise the in-memory size declared
         * in HUNK_BSS is inconsistent with the on-disk HUNK_DATA
         * header. Reject the mismatch. The compressed pointer is
         * allowed to be NULL in this case (zero-byte read). */
        if (uncompressed_size != 0u) {
            return FIRESTAFF_FTL_HUNK_DATA_ERR_OUTPUT_OVERFLOW;
        }
        return FIRESTAFF_FTL_HUNK_DATA_OK;
    }
    if (!compressed || !out) {
        return FIRESTAFF_FTL_HUNK_DATA_ERR_ARG;
    }
    if (compressed_size > FIRESTAFF_FTL_HUNK_DATA_MAX_INPUT) {
        return FIRESTAFF_FTL_HUNK_DATA_ERR_ARG;
    }
    if ((compressed_size & 1u) != 0u) {
        /* greatstone's Note 7 always advances by 2 or 4 bytes; an odd
         * compressed size is malformed and would otherwise leave one
         * dangling byte. Reject explicitly. */
        return FIRESTAFF_FTL_HUNK_DATA_ERR_ODD_INPUT;
    }
    if (uncompressed_size == 0u) {
        /* Non-empty compressed input claims to expand to zero bytes,
         * which is impossible (every iteration writes at least the
         * two leading bytes). */
        return FIRESTAFF_FTL_HUNK_DATA_ERR_OUTPUT_OVERFLOW;
    }

    /* Walk the compressed stream per Note 7. We maintain `i` as the
     * next compressed input index and `j` as the next uncompressed
     * output index. Both are bounded by the explicit
     * compressed_size / uncompressed_size fields so we never write
     * past `out` and never read past `compressed`. */
    size_t i = 0u;
    size_t j = 0u;
    while (i < compressed_size) {
        /* Need at least 2 bytes for the literal pair / run-header
         * sentinel. */
        if (i + 2u > compressed_size) {
            return FIRESTAFF_FTL_HUNK_DATA_ERR_TRUNCATED;
        }

        if (compressed[i] == 0x00u && compressed[i + 1u] == 0x00u) {
            /* Run header: copy the "00 00" pair through, then emit
             * `additional` more zero bytes. */
            if (j + 2u > uncompressed_size) {
                return FIRESTAFF_FTL_HUNK_DATA_ERR_OUTPUT_OVERFLOW;
            }
            if (i + 4u > compressed_size) {
                return FIRESTAFF_FTL_HUNK_DATA_ERR_TRUNCATED;
            }
            out[j]     = 0x00u;
            out[j + 1u] = 0x00u;
            j += 2u;

            uint16_t additional = read_be16(compressed + i + 2u);
            /* Guard against size_t overflow when summing j +
             * additional. The Note 7 pseudocode's `int additional`
             * is at most 0xFFFF, so the worst-case expansion is
             * j + 65535. We pre-check against uncompressed_size. */
            if ((size_t)additional > uncompressed_size - j) {
                return FIRESTAFF_FTL_HUNK_DATA_ERR_OUTPUT_OVERFLOW;
            }
            /* additional can legitimately be 0 (the spec says "the
             * number of additional 0x00"; zero additional zeros is
             * just the two copied leading bytes). memmove-style fill
             * is overkill here; a small memset is fine. */
            if (additional > 0u) {
                memset(out + j, 0x00, (size_t)additional);
                j += (size_t)additional;
            }
            i += 4u;
        } else {
            /* Literal pair: copy the two bytes unchanged. */
            if (j + 2u > uncompressed_size) {
                return FIRESTAFF_FTL_HUNK_DATA_ERR_OUTPUT_OVERFLOW;
            }
            out[j]     = compressed[i];
            out[j + 1u] = compressed[i + 1u];
            j += 2u;
            i += 2u;
        }
    }

    /* greatstone's loop has no end-of-stream length check; the FTL
     * loader is expected to size the output buffer from the HUNK_BSS
     * "size of area 1 of hunk 0x011 in memory" field. We honour that
     * contract: j must match uncompressed_size exactly. If it does
     * not, the FTL's area_1 sizes are inconsistent. */
    if (j != uncompressed_size) {
        return FIRESTAFF_FTL_HUNK_DATA_ERR_OUTPUT_OVERFLOW;
    }

    *out_written = j;
    return FIRESTAFF_FTL_HUNK_DATA_OK;
}

size_t FirestaffFtlHunkData_MaxUncompressedSize(size_t compressed_size) {
    /* Worst case per 4 input bytes is 2 + 0xFFFF output bytes. That
     * means compressed_size/4 * (2 + 65535) + leftover. compressed_size
     * is at most FIRESTAFF_FTL_HUNK_DATA_MAX_INPUT (32 MiB), so the
     * return value fits comfortably in size_t on 32-bit hosts and is
     * astronomically larger than any real FTL area_1 on 64-bit
     * hosts. We compute the bound carefully to avoid size_t overflow
     * on 32-bit platforms. */
    if (compressed_size > FIRESTAFF_FTL_HUNK_DATA_MAX_INPUT) return 0u;
    if (compressed_size == 0u) return 0u;
    size_t pairs = compressed_size / 2u;          /* 2-byte stride */
    /* Maximum pairs that can start a 4-byte run is pairs / 2. */
    size_t run_headers = pairs / 2u;
    size_t literal_pairs = pairs - run_headers * 2u;
    /* Each run header contributes 2 leading bytes + at most 0xFFFF
     * additional bytes. Each literal pair contributes 2 bytes. */
    size_t worst = run_headers * ((size_t)2u + (size_t)0xFFFFu) +
                   literal_pairs * 2u;
    return worst;
}

/* ── self-test ─────────────────────────────────────────────── */

#define ST_ASSERT(cond, msg) do {                                     \
    if (!(cond)) {                                                    \
        fprintf(stderr, "%s:%d: %s (%s)\n", __FILE__, __LINE__,       \
                msg, #cond);                                          \
        return 0;                                                     \
    }                                                                 \
} while (0)

static int test_all_zero_stream(void) {
    /* "00 00 00 FF" => copy "00 00" + 255 additional 0x00 = 257 bytes. */
    uint8_t in[4] = { 0x00u, 0x00u, 0x00u, 0xFFu };
    uint8_t out[260];
    size_t written = 0;
    FirestaffFtlHunkDataStatus rc = FirestaffFtlHunkData_DecompressZeroRun(
        in, sizeof(in), 257u, out, &written);
    ST_ASSERT(rc == FIRESTAFF_FTL_HUNK_DATA_OK, "all-zero ok");
    ST_ASSERT(written == 257u, "all-zero length");
    for (size_t i = 0; i < 257u; ++i) {
        ST_ASSERT(out[i] == 0x00u, "all-zero byte");
    }
    return 1;
}

static int test_no_runs_literal(void) {
    /* "11 22 33 44" with no "00 00" pair: copy as two literal pairs. */
    uint8_t in[4] = { 0x11u, 0x22u, 0x33u, 0x44u };
    uint8_t out[4];
    size_t written = 0;
    FirestaffFtlHunkDataStatus rc = FirestaffFtlHunkData_DecompressZeroRun(
        in, sizeof(in), 4u, out, &written);
    ST_ASSERT(rc == FIRESTAFF_FTL_HUNK_DATA_OK, "literal ok");
    ST_ASSERT(written == 4u, "literal length");
    ST_ASSERT(out[0] == 0x11u && out[1] == 0x22u &&
              out[2] == 0x33u && out[3] == 0x44u, "literal bytes");
    return 1;
}

static int test_mixed_runs(void) {
    /* Input: literal pair "11 22", run header "00 00 00 05" => 7 bytes,
     * literal pair "AA BB". Total uncompressed = 2 + 7 + 2 = 11 bytes.
     *   0x11 0x22 -> 11 22
     *   0x00 0x00 0x00 0x05 -> 00 00 00 00 00 00 00 (7 zeros)
     *   0xAA 0xBB -> AA BB
     */
    uint8_t in[8] = { 0x11u, 0x22u,
                      0x00u, 0x00u, 0x00u, 0x05u,
                      0xAAu, 0xBBu };
    uint8_t out[16];
    size_t written = 0;
    FirestaffFtlHunkDataStatus rc = FirestaffFtlHunkData_DecompressZeroRun(
        in, sizeof(in), 11u, out, &written);
    ST_ASSERT(rc == FIRESTAFF_FTL_HUNK_DATA_OK, "mixed ok");
    ST_ASSERT(written == 11u, "mixed length");
    static const uint8_t expected[11] = {
        0x11u, 0x22u,
        0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u,
        0xAAu, 0xBBu
    };
    ST_ASSERT(memcmp(out, expected, 11u) == 0, "mixed bytes");
    return 1;
}

static int test_zero_run_with_zero_additional(void) {
    /* "00 00 00 00" -> copy the leading "00 00", zero additional bytes,
     * total 2 bytes uncompressed. */
    uint8_t in[4] = { 0x00u, 0x00u, 0x00u, 0x00u };
    uint8_t out[4];
    size_t written = 0;
    FirestaffFtlHunkDataStatus rc = FirestaffFtlHunkData_DecompressZeroRun(
        in, sizeof(in), 2u, out, &written);
    ST_ASSERT(rc == FIRESTAFF_FTL_HUNK_DATA_OK, "zero-additional ok");
    ST_ASSERT(written == 2u, "zero-additional length");
    ST_ASSERT(out[0] == 0x00u && out[1] == 0x00u, "zero-additional bytes");
    return 1;
}

static int test_truncated_run_header(void) {
    /* "00 00" with no additional-count pair left: must reject. */
    uint8_t in[2] = { 0x00u, 0x00u };
    uint8_t out[8];
    size_t written = 0;
    FirestaffFtlHunkDataStatus rc = FirestaffFtlHunkData_DecompressZeroRun(
        in, sizeof(in), 8u, out, &written);
    ST_ASSERT(rc == FIRESTAFF_FTL_HUNK_DATA_ERR_TRUNCATED,
              "truncated run header rejects");
    ST_ASSERT(written == 0u, "no output on truncated");
    return 1;
}

static int test_truncated_literal_pair(void) {
    /* Single dangling byte (odd input): rejected by ODD_INPUT
     * validation. */
    uint8_t in[1] = { 0x11u };
    uint8_t out[4];
    size_t written = 0;
    FirestaffFtlHunkDataStatus rc = FirestaffFtlHunkData_DecompressZeroRun(
        in, sizeof(in), 2u, out, &written);
    ST_ASSERT(rc == FIRESTAFF_FTL_HUNK_DATA_ERR_ODD_INPUT,
              "odd input rejects");
    ST_ASSERT(written == 0u, "no output on odd");
    return 1;
}

static int test_uncompressed_size_too_small(void) {
    /* "00 00 00 FF" expands to 257 bytes but we declare only 4. */
    uint8_t in[4] = { 0x00u, 0x00u, 0x00u, 0xFFu };
    uint8_t out[4];
    size_t written = 0;
    FirestaffFtlHunkDataStatus rc = FirestaffFtlHunkData_DecompressZeroRun(
        in, sizeof(in), 4u, out, &written);
    ST_ASSERT(rc == FIRESTAFF_FTL_HUNK_DATA_ERR_OUTPUT_OVERFLOW,
              "size mismatch rejects");
    ST_ASSERT(written == 0u, "no output on overflow");
    return 1;
}

static int test_uncompressed_size_too_large(void) {
    /* "11 22" decompresses to 2 bytes; if the caller passes 4 the
     * decoder must reject (FTL loader sizes output from HUNK_BSS so
     * a mismatch is always a sign of bad input). */
    uint8_t in[2] = { 0x11u, 0x22u };
    uint8_t out[8];
    size_t written = 0;
    FirestaffFtlHunkDataStatus rc = FirestaffFtlHunkData_DecompressZeroRun(
        in, sizeof(in), 4u, out, &written);
    ST_ASSERT(rc == FIRESTAFF_FTL_HUNK_DATA_ERR_OUTPUT_OVERFLOW,
              "oversize declared output rejects");
    ST_ASSERT(written == 0u, "no output on oversize");
    return 1;
}

static int test_empty_in_empty_out(void) {
    uint8_t out[1] = { 0xCDu }; /* sentinel */
    size_t written = 0xDEADBEEFu;
    FirestaffFtlHunkDataStatus rc = FirestaffFtlHunkData_DecompressZeroRun(
        NULL, 0u, 0u, out, &written);
    ST_ASSERT(rc == FIRESTAFF_FTL_HUNK_DATA_OK, "empty empty ok");
    ST_ASSERT(written == 0u, "empty empty written=0");
    return 1;
}

static int test_max_uncompressed_size_bound(void) {
    /* 4 input bytes worst case = 2 + 0xFFFF additional = 0x10001 bytes. */
    size_t worst = FirestaffFtlHunkData_MaxUncompressedSize(4u);
    ST_ASSERT(worst == ((size_t)2u + (size_t)0xFFFFu),
              "4 bytes -> 65537 worst case");
    /* 8 input bytes worst case = 2 * (2 + 0xFFFF) = 0x20002 bytes. */
    worst = FirestaffFtlHunkData_MaxUncompressedSize(8u);
    ST_ASSERT(worst == ((size_t)2u * ((size_t)2u + (size_t)0xFFFFu)),
              "8 bytes -> 131074 worst case");
    /* 0 input -> 0 worst case. */
    worst = FirestaffFtlHunkData_MaxUncompressedSize(0u);
    ST_ASSERT(worst == 0u, "0 bytes -> 0 worst case");
    /* Oversize input -> 0 (rejected). */
    worst = FirestaffFtlHunkData_MaxUncompressedSize(
        FIRESTAFF_FTL_HUNK_DATA_MAX_INPUT + 1u);
    ST_ASSERT(worst == 0u, "oversize -> 0 worst case");
    return 1;
}

static int test_realistic_run_storm(void) {
    /* Build a small area_1 manually: literal "AA BB", then a run
     * header "00 00 00 10" (16 additional zeros), then literal
     * "CC DD". Total uncompressed = 2 + 18 + 2 = 22 bytes. */
    uint8_t in[8] = { 0xAAu, 0xBBu,
                      0x00u, 0x00u, 0x00u, 0x10u,
                      0xCCu, 0xDDu };
    uint8_t out[32];
    size_t written = 0;
    FirestaffFtlHunkDataStatus rc = FirestaffFtlHunkData_DecompressZeroRun(
        in, sizeof(in), 22u, out, &written);
    ST_ASSERT(rc == FIRESTAFF_FTL_HUNK_DATA_OK, "storm ok");
    ST_ASSERT(written == 22u, "storm length");
    ST_ASSERT(out[0] == 0xAAu && out[1] == 0xBBu, "storm head");
    for (size_t k = 0; k < 18u; ++k) {
        ST_ASSERT(out[2u + k] == 0x00u, "storm run byte");
    }
    ST_ASSERT(out[20] == 0xCCu && out[21] == 0xDDu, "storm tail");
    return 1;
}

int FirestaffFtlHunkData_SelfTest(void) {
    int total = 0;
    int passed = 0;
#define RUN(test) do { ++total; if (test()) ++passed; } while (0)
    RUN(test_all_zero_stream);
    RUN(test_no_runs_literal);
    RUN(test_mixed_runs);
    RUN(test_zero_run_with_zero_additional);
    RUN(test_truncated_run_header);
    RUN(test_truncated_literal_pair);
    RUN(test_uncompressed_size_too_small);
    RUN(test_uncompressed_size_too_large);
    RUN(test_empty_in_empty_out);
    RUN(test_max_uncompressed_size_bound);
    RUN(test_realistic_run_storm);
#undef RUN
    if (passed != total) {
        fprintf(stderr, "firestaff_ftl_hunk_data_zero_run self-test: "
                "%d/%d passed\n", passed, total);
    }
    return passed == total ? 0 : -1;
}
