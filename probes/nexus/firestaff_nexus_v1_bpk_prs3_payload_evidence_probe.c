/*
 * firestaff_nexus_v1_bpk_prs3_payload_evidence_probe.c
 * =======================================================
 *
 * pass1084 — real-bytes evidence probe for the DM Nexus MENU.BPK PRS3
 * payload stream format.
 *
 * Walks the BPPK/BMPD directory and produces per-entry structural
 * receipts for every PRS3-bearing entry WITHOUT claiming to decode the
 * compression format. Each receipt surfaces:
 *
 *   - the first 4 bytes of the payload read as a BE u32 (the strongest
 *     receipt we currently have that each PRS3 frame starts with a
 *     leading size-class word; this header_minus_payload is small and
 *     non-negative across every observed entry);
 *   - the first 8 bytes of the payload verbatim, so probe code can spot
 *     identical-prefix patterns between adjacent frames;
 *   - a bounded byte-frequency receipt over the first `sample_size`
 *     payload bytes (capped at 4 KiB) per entry;
 *   - per-mode compression-ratio distributions and aggregate
 *     sum/mean/min/max so callers can see which pixel-mode is most
 *     amenable to the unknown stream format.
 *
 * Source-lock / provenance:
 *   docs/source-lock/nexus_v1_phase0_provenance_gate_H2315.md:291-306
 *     (MENU.BPK identified as packed, game-specific, no formal
 *      compression analysis documented)
 *   docs/VERIFIED_HASHES.md:103
 *     (size 89060 / sha256 740ab2a864f04b89cddb172ce2560044fcc8c6a7f98ae2fe50461aa8da886636)
 *   ReDMCSB has no Saturn/Nexus implementation. This pass is a pure
 *     evidence-ledger pass; it does NOT attempt decompression, does NOT
 *     call any third-party codec, and does NOT advance any read cursor
 *     past the bounded receipt bytes.
 *
 * Optional path: when `~/.firestaff/data/nexus/MENU.BPK` is present the
 * probe surfaces receipts for all 162 PRS3-bearing entries on a 32 KB
 * per-entry sample (capped at 4 KiB via the lib cap); otherwise it
 * exercises a data-free synthetic BPPK archive that locks the size,
 * first-N-bytes, and frequency-tally contract so CI still has something
 * concrete to verify in the absence of copyrighted assets.
 */

#include "nexus_v1_bpk_archive.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, msg) do {                                      \
    if (cond) { printf("  PASS: %s\n", msg); ++g_pass; }          \
    else      { printf("  FAIL: %s\n", msg); ++g_fail; }          \
} while (0)

static void wb16(uint8_t *p, uint16_t v) {
    p[0] = (uint8_t)((v >> 8) & 0xffu);
    p[1] = (uint8_t)(v & 0xffu);
}

static void wb32(uint8_t *p, uint32_t v) {
    p[0] = (uint8_t)((v >> 24) & 0xffu);
    p[1] = (uint8_t)((v >> 16) & 0xffu);
    p[2] = (uint8_t)((v >> 8) & 0xffu);
    p[3] = (uint8_t)(v & 0xffu);
}

/* Synthetic BPPK archive: 3 PRS3-bearing entries + 1 directory trailer.
 * The PRS3 payloads are deliberately varied so the receipts cover:
 *   - a 144-byte payload whose first 4 BE bytes are 0x00000095 (mirrors
 *     the 16x15 14bpp MENU.BPK entry[1] observation);
 *   - a 28-byte payload whose first 4 BE bytes are 0x00000023 (mirrors
 *     the 16x16 8bpp MENU.BPK entry[7] observation);
 *   - a 24-byte payload whose first 4 BE bytes are 0x0000001d (mirrors
 *     the 14x7 14bpp MENU.BPK entry[14] observation).
 *
 * The payloads contain a mix of zero bytes and random-looking bytes so
 * the byte-frequency receipt has something concrete to verify (no
 * claim is made about the real MENU.BPK byte distribution).
 */
static size_t make_synthetic_bppk(uint8_t *buf, size_t cap) {
    /* Layout: 24-byte BPPK header, then 4 BE u32 candidate offsets, then
     * entry 0 (trailer, 20 bytes), entry 1 (16x15 14bpp, 32-byte prefix
     * + 144-byte payload), entry 2 (16x16 8bpp, 32-byte prefix + 28-byte
     * payload), entry 3 (14x7 14bpp, 32-byte prefix + 24-byte payload). */
    const uint32_t trailer_off = 24U + 4U * 4U;       /* = 40 */
    const uint32_t entry1_off  = trailer_off + 20U;   /* = 60 */
    const uint32_t entry2_off  = entry1_off + 32U + 154U; /* = 246 */
    const uint32_t entry3_off  = entry2_off + 32U + 28U;  /* = 296 */
    const uint32_t end_off     = entry3_off + 32U + 24U;
    /* Payload bytes crafted to put specific values at byte 0..4 while
     * looking plausible elsewhere. */
        static const uint8_t payload1[154] = {
        /* 0..3 = 0x000000A5 = 165 (header_minus_payload == 11). */
        0x00,0x00,0x00,0xA5,
        /* 4..143 = 0xaa (140 bytes) so the most-common-byte / distinct
         * value / sample statistics are deterministic. */
        0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,
        0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,
        0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,
        0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,
        0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,
        0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,
        0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,
        0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,
        0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,
        0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,
        0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,
        0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,
        0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,
        0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,
        0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,
        0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,
        0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,
        0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,
        0xaa,0xaa,0xaa,0xaa,
    };
    static const uint8_t payload2[28] = {
        /* 0..3 = 0x00000023 = 35 */
        0x00,0x00,0x00,0x23,
        0x05,0x04,0xee,0xfd, 0x41,0xff,0xfa,0xfd, 0xff,0x0f,0x0f,0x21,
        0x0f,0x33,0x0f,0x00, 0x45,0x0f,0x57,0x0f, 0x69,0x0f,0x7b,0x0f,
    };
    static const uint8_t payload3[24] = {
        /* 0..3 = 0x0000001d = 29 */
        0x00,0x00,0x00,0x1d,
        0x0b,0xf6,0xf6,0xe6, 0xf7,0xf6,0xed,0xf7, 0xef,0xf0,0xf8,0xf0,
        0xef,0xf6,0x00,0x06, 0x0e,0xed,0xf8,0x04,
    };

    if (cap < end_off) return 0;
    memset(buf, 0, cap);
    memcpy(buf, "BPPK", 4);
    wb32(buf + 4, (uint32_t)end_off);
    memcpy(buf + 12, "BMPD", 4);
    wb32(buf + 16, (uint32_t)(end_off - 20U));
    wb32(buf + 20, 4U); /* 4 candidate offsets */
    wb32(buf + 24, trailer_off);
    wb32(buf + 28, entry1_off);
    wb32(buf + 32, entry2_off);
    wb32(buf + 36, entry3_off);

    /* Entry 0: directory trailer — no PRS3 magic. */
    {
        uint8_t *r = buf + trailer_off;
        memset(r, 0, NEXUS_V1_BPK_ENTRY_PREFIX_BYTES);
        /* 8 bytes point at the last two real entries (mirrors MENU.BPK). */
        wb32(r + 0, entry1_off);
        wb32(r + 4, entry3_off);
        r[NEXUS_V1_BPK_PREFIX_MODE_OFFSET] = NEXUS_V1_BPK_MODE_TRAILER;
    }

    /* Entry 1: 16x15 14bpp, total 32 bytes prefix + payload1 (144). */
    {
        uint8_t *r = buf + entry1_off;
        memset(r, 0, NEXUS_V1_BPK_ENTRY_PREFIX_BYTES);
        wb16(r + NEXUS_V1_BPK_PREFIX_WIDTH_OFFSET, 16U);
        r[NEXUS_V1_BPK_PREFIX_HEIGHT_OFFSET] = 15U;
        r[NEXUS_V1_BPK_PREFIX_MODE_OFFSET] = NEXUS_V1_BPK_MODE_16BPP;
        memcpy(r + NEXUS_V1_BPK_ENTRY_PREFIX_BYTES, "PRS3", 4);
        wb32(r + NEXUS_V1_BPK_ENTRY_PREFIX_BYTES + 4U, 1U); /* version */
        wb32(r + NEXUS_V1_BPK_ENTRY_PREFIX_BYTES + 8U,
             16U * 15U); /* pixel count */
        memcpy(r + NEXUS_V1_BPK_ENTRY_PREFIX_BYTES + 12U,
               payload1, sizeof(payload1));
    }

    /* Entry 2: 16x16 8bpp, total 32 bytes prefix + payload2 (28). */
    {
        uint8_t *r = buf + entry2_off;
        memset(r, 0, NEXUS_V1_BPK_ENTRY_PREFIX_BYTES);
        wb16(r + NEXUS_V1_BPK_PREFIX_WIDTH_OFFSET, 16U);
        r[NEXUS_V1_BPK_PREFIX_HEIGHT_OFFSET] = 16U;
        r[NEXUS_V1_BPK_PREFIX_MODE_OFFSET] = NEXUS_V1_BPK_MODE_8BPP;
        memcpy(r + NEXUS_V1_BPK_ENTRY_PREFIX_BYTES, "PRS3", 4);
        wb32(r + NEXUS_V1_BPK_ENTRY_PREFIX_BYTES + 4U, 1U);
        wb32(r + NEXUS_V1_BPK_ENTRY_PREFIX_BYTES + 8U,
             16U * 16U);
        memcpy(r + NEXUS_V1_BPK_ENTRY_PREFIX_BYTES + 12U,
               payload2, sizeof(payload2));
    }

    /* Entry 3: 14x7 14bpp, total 32 bytes prefix + payload3 (24). */
    {
        uint8_t *r = buf + entry3_off;
        memset(r, 0, NEXUS_V1_BPK_ENTRY_PREFIX_BYTES);
        wb16(r + NEXUS_V1_BPK_PREFIX_WIDTH_OFFSET, 14U);
        r[NEXUS_V1_BPK_PREFIX_HEIGHT_OFFSET] = 7U;
        r[NEXUS_V1_BPK_PREFIX_MODE_OFFSET] = NEXUS_V1_BPK_MODE_16BPP;
        memcpy(r + NEXUS_V1_BPK_ENTRY_PREFIX_BYTES, "PRS3", 4);
        wb32(r + NEXUS_V1_BPK_ENTRY_PREFIX_BYTES + 4U, 1U);
        wb32(r + NEXUS_V1_BPK_ENTRY_PREFIX_BYTES + 8U,
             14U * 7U);
        memcpy(r + NEXUS_V1_BPK_ENTRY_PREFIX_BYTES + 12U,
               payload3, sizeof(payload3));
    }

    return end_off;
}

static int read_optional_menumenu_bpk(const char *home,
                                      uint8_t **out_data,
                                      size_t *out_size) {
    char path[1024];
    FILE *fp;
    long size;
    uint8_t *data;

    if (!home || !home[0]) return 0;
    if (snprintf(path, sizeof(path), "%s/.firestaff/data/nexus/MENU.BPK",
                 home) <= 0) return 0;
    fp = fopen(path, "rb");
    if (!fp) return 0;
    if (fseek(fp, 0, SEEK_END) != 0) { fclose(fp); return 0; }
    size = ftell(fp);
    if (size <= 0) { fclose(fp); return 0; }
    if (fseek(fp, 0, SEEK_SET) != 0) { fclose(fp); return 0; }
    data = (uint8_t *)malloc((size_t)size);
    if (!data) { fclose(fp); return 0; }
    if (fread(data, 1, (size_t)size, fp) != (size_t)size) {
        free(data); fclose(fp); return 0;
    }
    fclose(fp);
    *out_data = data;
    *out_size = (size_t)size;
    return 1;
}

static int hex_eq(const uint8_t *got, const uint8_t *want, size_t n) {
    return memcmp(got, want, n) == 0;
}

static void test_synthetic_bppk_evidence(void) {
    uint8_t buf[512];
    size_t size;
    Nexus_V1_BpkPrs3PayloadEvidence rows[8];
    Nexus_V1_BpkPrs3PayloadEvidenceSummary summary;
    int rc;

    printf("\n--- synthetic BPPK evidence (data-free) ---\n");

    size = make_synthetic_bppk(buf, sizeof(buf));
    CHECK(size > 0, "synthetic BPPK build returns non-zero size");

    memset(rows, 0, sizeof(rows));
    rc = nexus_v1_bpk_archive_prs3_payload_evidence(
        buf, size, 32U, rows, 8U, &summary);
    CHECK(rc == 0, "synthetic BPPK evidence walker returns ok");

    /* 4 candidate offsets, 1 trailer + 3 PRS3 entries. */
    CHECK(summary.entries_seen == 4U, "summary.entries_seen == 4");
    CHECK(summary.trailer_skipped == 1U, "summary.trailer_skipped == 1");
    CHECK(summary.unknown_skipped == 0U, "summary.unknown_skipped == 0");
    CHECK(summary.used == 3U, "summary.used == 3 (3 PRS3-bearing entries)");
    CHECK(summary.truncated == 0, "summary.truncated == 0 (capacity not hit)");

    /* Per-mode bucket: 2 entries use mode 14, 1 uses mode 6. */
    CHECK(summary.mode_count[NEXUS_V1_BPK_MODE_16BPP] == 2U,
          "summary.mode_count[14] == 2");
    CHECK(summary.mode_count[NEXUS_V1_BPK_MODE_8BPP] == 1U,
          "summary.mode_count[6] == 1");

    /* Entry 1 row: 16x15 14bpp, payload 144, header_first == 0x95. */
    CHECK(rows[0].entry_index == 1U, "row 0 entry_index == 1");
    CHECK(rows[0].width == 16 && rows[0].height == 15 &&
          rows[0].mode == NEXUS_V1_BPK_MODE_16BPP,
          "row 0 width=16 height=15 mode=14");
    CHECK(rows[0].pixel_count == 240U, "row 0 pixel_count == 240");
    CHECK(rows[0].bpp == 2U, "row 0 bpp == 2");
    CHECK(rows[0].uncompressed_size == 480U, "row 0 uncompressed_size == 480");
    CHECK(rows[0].payload_size == 154U, "row 0 payload_size == 154");
    CHECK(rows[0].header_first_readable == 1, "row 0 header_first_readable");
    CHECK(rows[0].header_first_u32 == 0xA5U,
          "row 0 header_first_u32 == 0xA5 (165)");
    CHECK(rows[0].header_minus_payload == 11U,
          "row 0 header_minus_payload == 11 (165 - 154)");
    {
        uint8_t want[8] = {
            0x00,0x00,0x00,0xA5, 0xaa,0xaa,0xaa,0xaa,
        };
        CHECK(hex_eq(rows[0].first_payload, want, 8),
              "row 0 first_payload == 000000a5 aaaaaaaa");
    }
    CHECK(rows[0].compression_ratio > 0.320 &&
          rows[0].compression_ratio < 0.322,
          "row 0 compression_ratio == 0.321 (154/480)");

    /* Frequency receipt on entry 1: 0xaa dominates the synthetic
     * payload after byte 4 (140 bytes of 0xaa). sample_size=32 sees
     * bytes 0..31 = 0x00 (3x), 0x95 (1x), 0xaa (28x), so 0xaa is the
     * most common and distinct values are 3. */
    CHECK(rows[0].sample_size_used == 32U,
          "row 0 sample_size_used == 32 (capped by lib, full sample)");
    CHECK(rows[0].most_common_byte == 0xAAU,
          "row 0 most_common_byte == 0xaa");
    CHECK(rows[0].most_common_byte_count >= 28U,
          "row 0 most_common_byte_count >= 28 (28 of 0xaa in 32 sampled bytes)");
    CHECK(rows[0].distinct_byte_values == 3U,
          "row 0 distinct_byte_values == 3 (0x00, 0x95, 0xaa)");
    /* Byte-class tally (pass1084b): row 0 sample is 32 bytes,
     * 3 x 0x00 (quad 0), 1 x 0xA5 (quad 2, since 0xA5 >> 6 == 2),
     * 28 x 0xAA (quad 2). */
    CHECK(rows[0].byte_class_count[0] == 3U &&
              rows[0].byte_class_count[1] == 0U &&
              rows[0].byte_class_count[2] == 29U &&
              rows[0].byte_class_count[3] == 0U,
          "row 0 byte_class_count[*] == {3,0,29,0}");
    CHECK(rows[0].byte_class_count[0] +
              rows[0].byte_class_count[1] +
              rows[0].byte_class_count[2] +
              rows[0].byte_class_count[3] ==
              rows[0].sample_size_used,
          "row 0 byte_class_count[*] sum == sample_size_used");

    /* Entry 2 row: 16x16 8bpp, payload 28, header_first == 0x23. */
    CHECK(rows[1].entry_index == 2U, "row 1 entry_index == 2");
    CHECK(rows[1].bpp == 1U, "row 1 bpp == 1");
    CHECK(rows[1].uncompressed_size == 256U, "row 1 uncompressed_size == 256");
    CHECK(rows[1].payload_size == 28U, "row 1 payload_size == 28");
    CHECK(rows[1].header_first_u32 == 0x23U,
          "row 1 header_first_u32 == 0x23 (35)");
    CHECK(rows[1].header_minus_payload == 7U,
          "row 1 header_minus_payload == 7 (35 - 28)");
    {
        uint8_t want[8] = {
            0x00,0x00,0x00,0x23, 0x05,0x04,0xee,0xfd,
        };
        CHECK(hex_eq(rows[1].first_payload, want, 8),
              "row 1 first_payload == 00000023 0504eefd");
    }
    CHECK(rows[1].compression_ratio > 0.108 &&
          rows[1].compression_ratio < 0.110,
          "row 1 compression_ratio == 0.109 (28/256)");
    /* Byte-class tally (pass1084b): row 1 sample is 28 bytes
     * (0x00 x 4, 0x23, 0x05, 0x04, 0x0f x 8, 0x21, 0x33, 0x41, 0x45,
     *  0x57, 0x69, 0x7b, 0xee, 0xfd x 2, 0xff x 2, 0xfa).
     *  Quad 0: 4+1+1+1+8+1+1=17  Quad 1: 1+1+1+1+1=5
     *  Quad 3: 1+2+2+1=6          Quad 2: 0. */
    CHECK(rows[1].byte_class_count[0] == 17U &&
              rows[1].byte_class_count[1] == 5U &&
              rows[1].byte_class_count[2] == 0U &&
              rows[1].byte_class_count[3] == 6U,
          "row 1 byte_class_count[*] == {17,5,0,6}");

    /* Entry 3 row: 14x7 14bpp, payload 24, header_first == 0x1d. */
    CHECK(rows[2].entry_index == 3U, "row 2 entry_index == 3");
    CHECK(rows[2].bpp == 2U, "row 2 bpp == 2");
    CHECK(rows[2].uncompressed_size == 196U, "row 2 uncompressed_size == 196");
    CHECK(rows[2].payload_size == 24U, "row 2 payload_size == 24");
    CHECK(rows[2].header_first_u32 == 0x1DU,
          "row 2 header_first_u32 == 0x1d (29)");
    CHECK(rows[2].header_minus_payload == 5U,
          "row 2 header_minus_payload == 5 (29 - 24)");
    {
        uint8_t want[8] = {
            0x00,0x00,0x00,0x1d, 0x0b,0xf6,0xf6,0xe6,
        };
        CHECK(hex_eq(rows[2].first_payload, want, 8),
              "row 2 first_payload == 0000001d 0bf6f6e6");
    }
    CHECK(rows[2].compression_ratio > 0.122 &&
          rows[2].compression_ratio < 0.123,
          "row 2 compression_ratio == 0.122 (24/196)");
    /* Byte-class tally (pass1084b): row 2 sample is 24 bytes
     *  (0x00 x 4, 0x1d, 0x0b, 0x06, 0x0e, 0x04, 0xf6 x 4, 0xe6, 0xf7 x 2,
     *   0xed x 2, 0xef x 2, 0xf0 x 2, 0xf8 x 2).
     *  Quad 0: 4+1+1+1+1+1=9   Quad 3: 4+1+2+2+2+2+2=15. */
    CHECK(rows[2].byte_class_count[0] == 9U &&
              rows[2].byte_class_count[1] == 0U &&
              rows[2].byte_class_count[2] == 0U &&
              rows[2].byte_class_count[3] == 15U,
          "row 2 byte_class_count[*] == {9,0,0,15}");

    /* Aggregate checks. */
    CHECK(summary.total_uncompressed == 480U + 256U + 196U,
          "summary.total_uncompressed == 932 (480+256+196)");
    CHECK(summary.total_payload == 154U + 28U + 24U,
          "summary.total_payload == 206 (154+28+24)");
    CHECK(summary.smallest_payload == 24U, "summary.smallest_payload == 24");
    CHECK(summary.largest_payload == 154U, "summary.largest_payload == 154");
    CHECK(summary.min_compression_ratio > 0.108 &&
          summary.min_compression_ratio < 0.110,
          "summary.min_compression_ratio ~ row 1's ratio");
    CHECK(summary.max_compression_ratio > 0.320 &&
          summary.max_compression_ratio < 0.322,
          "summary.max_compression_ratio ~ row 0's ratio");

    /* Capacity exhaustion: 0 rows + 1 row capacity should mark
     * summary.used == 3 (always counted) and truncated == 1 (only 1
     * row actually written). */
    {
        Nexus_V1_BpkPrs3PayloadEvidence tiny_rows[1];
        Nexus_V1_BpkPrs3PayloadEvidenceSummary tiny_summary;
        memset(tiny_rows, 0, sizeof(tiny_rows));
        rc = nexus_v1_bpk_archive_prs3_payload_evidence(
            buf, size, 32U, tiny_rows, 1U, &tiny_summary);
        CHECK(rc == 0, "capacity-exhausted walker returns ok");
        CHECK(tiny_summary.used == 3U,
              "capacity-exhausted summary.used == 3");
        CHECK(tiny_summary.truncated == 1,
              "capacity-exhausted summary.truncated == 1");
    }

    /* Sample size 0 must not crash and must skip the frequency pass. */
    {
        Nexus_V1_BpkPrs3PayloadEvidence rows2[8];
        Nexus_V1_BpkPrs3PayloadEvidenceSummary summary2;
        memset(rows2, 0, sizeof(rows2));
        rc = nexus_v1_bpk_archive_prs3_payload_evidence(
            buf, size, 0U, rows2, 8U, &summary2);
        CHECK(rc == 0, "sample_size=0 walker returns ok");
        CHECK(rows2[0].sample_size_used == 0U,
              "sample_size=0 row 0 sample_size_used == 0");
        CHECK(rows2[0].most_common_byte_count == 0U,
              "sample_size=0 row 0 most_common_byte_count == 0");
        CHECK(rows2[0].byte_class_count[0] == 0U &&
                  rows2[0].byte_class_count[1] == 0U &&
                  rows2[0].byte_class_count[2] == 0U &&
                  rows2[0].byte_class_count[3] == 0U,
              "sample_size=0 row 0 byte_class_count[*] all zero");
    }

    /* NULL out_summary is rejected. */
    rc = nexus_v1_bpk_archive_prs3_payload_evidence(
        buf, size, 32U, rows, 8U, NULL);
    CHECK(rc != 0, "NULL summary is rejected");

    /* NULL data is rejected. */
    rc = nexus_v1_bpk_archive_prs3_payload_evidence(
        NULL, size, 32U, rows, 8U, &summary);
    CHECK(rc != 0, "NULL data is rejected");

    /* Bad BPPK magic is rejected. */
    buf[0] = 'X';
    rc = nexus_v1_bpk_archive_prs3_payload_evidence(
        buf, size, 32U, rows, 8U, &summary);
    CHECK(rc != 0, "bad BPPK magic is rejected");
    buf[0] = 'B';
}

static void test_optional_real_menumenu_bpk(void) {
    const char *home = getenv("HOME");
    uint8_t *data = NULL;
    size_t size = 0;

    printf("\n--- optional real MENU.BPK (no asset loaded in CI) ---\n");
    if (!read_optional_menumenu_bpk(home, &data, &size)) {
        printf("  SKIP: real MENU.BPK not present\n");
        return;
    }

    {
        Nexus_V1_BpkPrs3PayloadEvidence *rows;
        Nexus_V1_BpkPrs3PayloadEvidenceSummary summary;
        uint32_t capacity = 256U;
        uint32_t sample_size = 4096U;
        int rc;

        printf("  loaded %zu bytes from ~/.firestaff/data/nexus/MENU.BPK\n",
               size);
        printf("  walking up to %u entries with %u byte sample per entry\n",
               capacity, sample_size);

        rows = (Nexus_V1_BpkPrs3PayloadEvidence *)calloc(
            capacity, sizeof(*rows));
        if (!rows) {
            printf("  FAIL: cannot allocate %u receipts\n", capacity);
            ++g_fail;
            free(data);
            return;
        }
        memset(&summary, 0, sizeof(summary));
        rc = nexus_v1_bpk_archive_prs3_payload_evidence(
            data, size, sample_size, rows, capacity, &summary);
        CHECK(rc == 0, "real MENU.BPK evidence walker returns ok");
        CHECK(summary.entries_seen == 163U,
              "real MENU.BPK entries_seen == 163");
        CHECK(summary.trailer_skipped == 1U,
              "real MENU.BPK trailer_skipped == 1");
        CHECK(summary.used == 162U,
              "real MENU.BPK used == 162 PRS3-bearing entries");
        CHECK(summary.truncated == 0,
              "real MENU.BPK capacity not exhausted (162 <= 256)");
        CHECK(summary.mode_count[NEXUS_V1_BPK_MODE_8BPP] == 14U,
              "real MENU.BPK mode_count[6] == 14");
        CHECK(summary.mode_count[NEXUS_V1_BPK_MODE_16BPP] == 62U,
              "real MENU.BPK mode_count[14] == 62");
        CHECK(summary.mode_count[NEXUS_V1_BPK_MODE_24BPP] == 39U,
              "real MENU.BPK mode_count[22] == 39");
        CHECK(summary.mode_count[NEXUS_V1_BPK_MODE_32BPP] == 47U,
              "real MENU.BPK mode_count[30] == 47");

        CHECK(summary.total_uncompressed > 0U,
              "real MENU.BPK total_uncompressed > 0");
        CHECK(summary.total_payload > 0U,
              "real MENU.BPK total_payload > 0");
        CHECK(summary.total_payload < summary.total_uncompressed,
              "real MENU.BPK compressed bytes < uncompressed bytes");

        /* Spot-check three known entries: 1, 7, 14. */
        if (summary.used > 0U) {
            uint32_t e1 = 0xFFFu, e7 = 0xFFFu, e14 = 0xFFFu;
            for (uint32_t i = 0; i < summary.used; ++i) {
                if (rows[i].entry_index == 1U)  e1  = i;
                if (rows[i].entry_index == 7U)  e7  = i;
                if (rows[i].entry_index == 14U) e14 = i;
            }
            if (e1 != 0xFFFu) {
                char msg[96];
                snprintf(msg, sizeof(msg),
                         "real entry[1] header_first_u32 == 0x95 (got 0x%lx)",
                         (unsigned long)rows[e1].header_first_u32);
                CHECK(rows[e1].header_first_u32 == 0x95U, msg);
            }
            if (e7 != 0xFFFu) {
                char msg[96];
                snprintf(msg, sizeof(msg),
                         "real entry[7] header_first_u32 == 0x23 (got 0x%lx)",
                         (unsigned long)rows[e7].header_first_u32);
                CHECK(rows[e7].header_first_u32 == 0x23U, msg);
            }
            if (e14 != 0xFFFu) {
                char msg[96];
                snprintf(msg, sizeof(msg),
                         "real entry[14] header_first_u32 == 0x1d (got 0x%lx)",
                         (unsigned long)rows[e14].header_first_u32);
                CHECK(rows[e14].header_first_u32 == 0x1DU, msg);
            }

            /* Strongest receipt: across the 162 PRS3 entries, almost
             * every header_first_u32 must be >= payload_size, with one
             * known edge case (entry[162], the last entry in the
             * archive) where the BMPD next_offset boundary includes
             * trailing file padding so the lib's payload_size can
             * exceed the leading header. We assert the count is in
             * [161, 162] and report the saturated-difference
             * surfaced by the walker. */
            {
                uint32_t readable = 0U;
                uint32_t header_ge_payload = 0U;
                uint32_t header_lt_payload = 0U;
                for (uint32_t i = 0; i < summary.used; ++i) {
                    if (rows[i].header_first_readable) {
                        ++readable;
                        if (rows[i].header_first_u32 >=
                            rows[i].payload_size) {
                            ++header_ge_payload;
                        } else {
                            ++header_lt_payload;
                        }
                    }
                }
                CHECK(readable == 162U,
                      "real MENU.BPK every PRS3 entry has readable header u32");
                CHECK(header_ge_payload >= 161U &&
                          header_ge_payload <= 162U,
                      "real MENU.BPK 161 or 162 PRS3 entries satisfy header >= payload");
                CHECK(header_lt_payload <= 1U,
                      "real MENU.BPK at most 1 PRS3 entry has header < payload");
            }

            /* pass1084b — 4-quadrant byte-class receipt over the real
             * MENU.BPK payloads. Aggregate the per-row byte_class_count
             * across all 162 PRS3 entries (each row's sample_size_used
             * caps the contribution) and check that:
             *   (1) every quadrant is non-zero (the stream is not
             *       concentrated in one byte range);
             *   (2) the sum equals the aggregate sample bytes seen;
             *   (3) quadrant 0 [0x00..0x3F] is the largest bucket
             *       (consistent with length-class hints in the
             *       header word and possible repeated literal zeros
             *       in the stream — the strongest single receipt
             *       that the stream has structure rather than being
             *       uniformly random);
             *   (4) the per-row byte_class_count sums equal each
             *       row's sample_size_used (sanity invariant). */
            {
                uint64_t agg_bc[4] = {0U, 0U, 0U, 0U};
                uint64_t agg_sample = 0U;
                int per_row_sum_ok = 1;
                for (uint32_t i = 0; i < summary.used; ++i) {
                    agg_bc[0] += rows[i].byte_class_count[0];
                    agg_bc[1] += rows[i].byte_class_count[1];
                    agg_bc[2] += rows[i].byte_class_count[2];
                    agg_bc[3] += rows[i].byte_class_count[3];
                    agg_sample += rows[i].sample_size_used;
                    uint32_t row_sum =
                        rows[i].byte_class_count[0] +
                        rows[i].byte_class_count[1] +
                        rows[i].byte_class_count[2] +
                        rows[i].byte_class_count[3];
                    if (row_sum != rows[i].sample_size_used) {
                        per_row_sum_ok = 0;
                    }
                }
                CHECK(per_row_sum_ok,
                      "real MENU.BPK every row's byte_class_count sum == sample_size_used");
                CHECK(agg_bc[0] + agg_bc[1] + agg_bc[2] + agg_bc[3] ==
                          agg_sample,
                      "real MENU.BPK aggregate byte_class_count sum == aggregate sample bytes");
                CHECK(agg_bc[0] > 0U && agg_bc[1] > 0U &&
                          agg_bc[2] > 0U && agg_bc[3] > 0U,
                      "real MENU.BPK every byte quadrant has at least one byte");
                /* Surface the actual quadrant distribution so future
                 * algorithm-narrowing passes have a concrete receipt
                 * they can compare against the synthetic archive. */
                {
                    char dist_msg[200];
                    snprintf(dist_msg, sizeof(dist_msg),
                             "real MENU.BPK quadrant distribution: "
                             "q0=0x00..0x3F=%llu q1=0x40..0x7F=%llu "
                             "q2=0x80..0xBF=%llu q3=0xC0..0xFF=%llu (sample=%llu)",
                             (unsigned long long)agg_bc[0],
                             (unsigned long long)agg_bc[1],
                             (unsigned long long)agg_bc[2],
                             (unsigned long long)agg_bc[3],
                             (unsigned long long)agg_sample);
                    printf("  INFO: %s\n", dist_msg);
                }
                /* Low-byte dominance: the PRS3 stream is biased
                 * toward the lower byte range, with quadrant 0
                 * (0x00..0x3F) carrying the most bytes. This is the
                 * strongest single receipt that the stream has
                 * structure (length-class hints, repeated literal
                 * zeros, etc.) rather than uniformly random bytes.
                 * Assert q0 exceeds the average of the other three
                 * quadrants by at least 1.2x (i.e. q0 > avg(q1..q3)
                 * by a meaningful margin). */
                {
                    uint64_t others_sum = agg_bc[1] + agg_bc[2] + agg_bc[3];
                    char msg[200];
                    snprintf(msg, sizeof(msg),
                             "real MENU.BPK q0 > 1.2 * avg(q1..q3) "
                             "(q0=%llu, avg(q1..q3)=%.1f)",
                             (unsigned long long)agg_bc[0],
                             (others_sum > 0U)
                                 ? ((double)others_sum / 3.0)
                                 : 0.0);
                    CHECK(others_sum > 0U &&
                              (3U * agg_bc[0]) > (36U * others_sum) / 30U,
                          msg);
                }
            }
        }

        free(rows);
        free(data);
    }
}

int main(void) {
    printf("=== Nexus V1 PRS3 payload evidence probe (pass1084) ===\n");

    test_synthetic_bppk_evidence();
    test_optional_real_menumenu_bpk();

    printf("\n# summary: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail > 0 ? 1 : 0;
}
