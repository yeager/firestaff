#include "nexus_v1_bpk_archive.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int g_failures;

static void expect(int condition, const char *message) {
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        ++g_failures;
    } else {
        printf("PASS: %s\n", message);
    }
}

static void wr16_be(uint8_t *p, uint16_t v) {
    p[0] = (uint8_t)(v >> 8);
    p[1] = (uint8_t)v;
}

static void wr32_be(uint8_t *p, uint32_t v) {
    p[0] = (uint8_t)(v >> 24);
    p[1] = (uint8_t)(v >> 16);
    p[2] = (uint8_t)(v >> 8);
    p[3] = (uint8_t)v;
}

/* Synthetic BPPK layout (mirrors the probe's make_synthetic_bppk but
 * with smaller payloads so the test executes quickly). The schema is:
 *   - 24-byte BPPK header
 *   - 4 BE u32 candidate offsets
 *   - entry 0: 20-byte directory trailer (mode 10, no PRS3)
 *   - entry 1: 32-byte prefix (16x15 14bpp) + 32-byte PRS3 payload
 *     whose first 4 bytes are 0x00000083 (so header_minus_payload == 5)
 *   - entry 2: 32-byte prefix (8x4 8bpp) + 4-byte PRS3 payload
 *     whose 4 bytes are 0x00000007 (so header_minus_payload == 3)
 *
 * After entry 2 the archive ends. */
#define ARCHIVE_SIZE 24 + 16 + (20) + (32 + 32) + (32 + 4)

static uint8_t archive[ARCHIVE_SIZE];

static void build_archive(void) {
    const uint32_t trailer_off = 24U + 16U;          /* = 40 */
    const uint32_t entry1_off  = trailer_off + 20U;   /* = 60 */
    const uint32_t entry2_off  = entry1_off + 32U + 32U; /* = 124 */
    /* end_off = entry2_off + 32U + 4U = 160. archive size matches. */

    memset(archive, 0, sizeof(archive));
    memcpy(archive, "BPPK", 4);
    wr32_be(archive + 4, (uint32_t)sizeof(archive));
    memcpy(archive + 12, "BMPD", 4);
    wr32_be(archive + 16, (uint32_t)(sizeof(archive) - 20U));
    wr32_be(archive + 20, 3U); /* 3 candidate offsets */
    wr32_be(archive + 24, trailer_off);
    wr32_be(archive + 28, entry1_off);
    wr32_be(archive + 32, entry2_off);

    /* Entry 0: directory trailer. */
    {
        uint8_t *r = archive + trailer_off;
        memset(r, 0, NEXUS_V1_BPK_ENTRY_PREFIX_BYTES);
        r[NEXUS_V1_BPK_PREFIX_MODE_OFFSET] = NEXUS_V1_BPK_MODE_TRAILER;
    }

    /* Entry 1: 16x15 14bpp + 32-byte payload with header_first=0x83. */
    {
        uint8_t *r = archive + entry1_off;
        memset(r, 0, NEXUS_V1_BPK_ENTRY_PREFIX_BYTES);
        wr16_be(r + NEXUS_V1_BPK_PREFIX_WIDTH_OFFSET, 16U);
        r[NEXUS_V1_BPK_PREFIX_HEIGHT_OFFSET] = 15U;
        r[NEXUS_V1_BPK_PREFIX_MODE_OFFSET] = NEXUS_V1_BPK_MODE_16BPP;
        memcpy(r + NEXUS_V1_BPK_ENTRY_PREFIX_BYTES, "PRS3", 4);
        wr32_be(r + NEXUS_V1_BPK_ENTRY_PREFIX_BYTES + 4U, 1U);
        wr32_be(r + NEXUS_V1_BPK_ENTRY_PREFIX_BYTES + 8U, 16U * 15U);
        /* 32-byte payload, first 4 bytes BE = 0x83 (131). Rest is zeros. */
        wr32_be(r + NEXUS_V1_BPK_ENTRY_PREFIX_BYTES + 12U, 0x83U);
    }

    /* Entry 2: 8x4 8bpp + 4-byte payload with header_first=0x07 (7). */
    {
        uint8_t *r = archive + entry2_off;
        memset(r, 0, NEXUS_V1_BPK_ENTRY_PREFIX_BYTES);
        wr16_be(r + NEXUS_V1_BPK_PREFIX_WIDTH_OFFSET, 8U);
        r[NEXUS_V1_BPK_PREFIX_HEIGHT_OFFSET] = 4U;
        r[NEXUS_V1_BPK_PREFIX_MODE_OFFSET] = NEXUS_V1_BPK_MODE_8BPP;
        memcpy(r + NEXUS_V1_BPK_ENTRY_PREFIX_BYTES, "PRS3", 4);
        wr32_be(r + NEXUS_V1_BPK_ENTRY_PREFIX_BYTES + 4U, 1U);
        wr32_be(r + NEXUS_V1_BPK_ENTRY_PREFIX_BYTES + 8U, 8U * 4U);
        wr32_be(r + NEXUS_V1_BPK_ENTRY_PREFIX_BYTES + 12U, 0x07U);
    }
}

static void test_evidence_walker(void) {
    Nexus_V1_BpkPrs3PayloadEvidence rows[8];
    Nexus_V1_BpkPrs3PayloadEvidenceSummary summary;
    int rc;

    build_archive();

    memset(rows, 0, sizeof(rows));
    memset(&summary, 0, sizeof(summary));

    rc = nexus_v1_bpk_archive_prs3_payload_evidence(
        archive, sizeof(archive), 32U, rows, 8U, &summary);
    expect(rc == 0, "evidence walker returns ok");
    expect(summary.entries_seen == 3U,
           "summary.entries_seen == 3 (3 candidate offsets)");
    expect(summary.trailer_skipped == 1U, "trailer is skipped and counted");
    expect(summary.used == 2U, "two PRS3-bearing rows are emitted");

    /* Row 0 is the 16x15 14bpp entry. */
    expect(rows[0].entry_index == 1U, "row 0 entry_index == 1");
    expect(rows[0].mode == NEXUS_V1_BPK_MODE_16BPP,
           "row 0 mode == MODE_16BPP");
    expect(rows[0].bpp == 2U, "row 0 bpp == 2");
    expect(rows[0].uncompressed_size == 480U,
           "row 0 uncompressed_size == 480");
    expect(rows[0].payload_size == 32U, "row 0 payload_size == 32");
    expect(rows[0].header_first_readable == 1,
           "row 0 header_first_readable");
    expect(rows[0].header_first_u32 == 0x83U,
           "row 0 header_first_u32 == 0x83");
    expect(rows[0].header_minus_payload == 99U,
           "row 0 header_minus_payload == 99 (131 - 32)");
    /* Byte-class tally: sample is 32 bytes, byte 0..2 = 0x00 (quadrant 0,
     * since 0x00 >> 6 == 0), byte 3 = 0x83 (quadrant 2, since
     * 0x83 >> 6 == 2 — 0x83 is in [0x80..0xBF]), bytes 4..31 = 0x00
     * (quadrant 0, 28 bytes). */
    expect(rows[0].byte_class_count[0] == 31U,
           "row 0 byte_class_count[0] == 31 (3 leading zeros + 28 trailing zeros)");
    expect(rows[0].byte_class_count[1] == 0U,
           "row 0 byte_class_count[1] == 0 (no byte in [0x40..0x7F])");
    expect(rows[0].byte_class_count[2] == 1U,
           "row 0 byte_class_count[2] == 1 (1 x 0x83 — last byte of 0x00000083)");
    expect(rows[0].byte_class_count[3] == 0U,
           "row 0 byte_class_count[3] == 0");
    expect(rows[0].byte_class_count[0] +
               rows[0].byte_class_count[1] +
               rows[0].byte_class_count[2] +
               rows[0].byte_class_count[3] ==
               rows[0].sample_size_used,
           "row 0 byte_class_count[*] sum == sample_size_used");

    /* Row 1 is the 8x4 8bpp entry; payload is exactly 4 bytes. */
    expect(rows[1].entry_index == 2U, "row 1 entry_index == 2");
    expect(rows[1].mode == NEXUS_V1_BPK_MODE_8BPP,
           "row 1 mode == MODE_8BPP");
    expect(rows[1].bpp == 1U, "row 1 bpp == 1");
    expect(rows[1].uncompressed_size == 32U,
           "row 1 uncompressed_size == 32");
    expect(rows[1].payload_size == 4U, "row 1 payload_size == 4");
    expect(rows[1].header_first_u32 == 0x07U,
           "row 1 header_first_u32 == 0x07");
    expect(rows[1].header_minus_payload == 3U,
           "row 1 header_minus_payload == 3 (7 - 4)");
    /* Byte-class tally: sample is 4 bytes, all 0x07 (quadrant 0). */
    expect(rows[1].byte_class_count[0] == 4U,
           "row 1 byte_class_count[0] == 4 (4 x 0x07)");
    expect(rows[1].byte_class_count[1] == 0U &&
               rows[1].byte_class_count[2] == 0U &&
               rows[1].byte_class_count[3] == 0U,
           "row 1 byte_class_count[1..3] all == 0");

    /* Aggregates. */
    expect(summary.smallest_payload == 4U, "summary.smallest_payload == 4");
    expect(summary.largest_payload == 32U, "summary.largest_payload == 32");
    expect(summary.total_uncompressed == 512U,
           "summary.total_uncompressed == 512 (480+32)");
    expect(summary.total_payload == 36U,
           "summary.total_payload == 36 (32+4)");
    expect(summary.truncated == 0, "no truncation (capacity not hit)");
}

static void test_capacity_exhaustion(void) {
    Nexus_V1_BpkPrs3PayloadEvidence rows[1];
    Nexus_V1_BpkPrs3PayloadEvidenceSummary summary;
    int rc;

    build_archive();
    memset(rows, 0, sizeof(rows));
    memset(&summary, 0, sizeof(summary));

    rc = nexus_v1_bpk_archive_prs3_payload_evidence(
        archive, sizeof(archive), 32U, rows, 1U, &summary);
    expect(rc == 0, "capacity-exhausted walker returns ok");
    expect(summary.used == 2U,
           "capacity-exhausted summary.used still == 2 (always counted)");
    expect(summary.truncated == 1,
           "capacity-exhausted summary.truncated == 1");
}

static void test_zero_sample(void) {
    Nexus_V1_BpkPrs3PayloadEvidence rows[8];
    Nexus_V1_BpkPrs3PayloadEvidenceSummary summary;
    int rc;

    build_archive();
    memset(rows, 0, sizeof(rows));
    memset(&summary, 0, sizeof(summary));

    rc = nexus_v1_bpk_archive_prs3_payload_evidence(
        archive, sizeof(archive), 0U, rows, 8U, &summary);
    expect(rc == 0, "sample_size=0 walker returns ok");
    expect(rows[0].sample_size_used == 0U,
           "sample_size=0 row 0 sample_size_used == 0");
    expect(rows[0].most_common_byte_count == 0U,
           "sample_size=0 row 0 most_common_byte_count == 0");
    expect(rows[0].header_first_u32 == 0x83U,
           "sample_size=0 row 0 header_first_u32 still readable");
    expect(rows[0].byte_class_count[0] == 0U &&
               rows[0].byte_class_count[1] == 0U &&
               rows[0].byte_class_count[2] == 0U &&
               rows[0].byte_class_count[3] == 0U,
           "sample_size=0 row 0 byte_class_count[*] all zero");
}

static void test_rejections(void) {
    Nexus_V1_BpkPrs3PayloadEvidence rows[1];
    Nexus_V1_BpkPrs3PayloadEvidenceSummary summary;
    int rc;

    build_archive();

    rc = nexus_v1_bpk_archive_prs3_payload_evidence(
        NULL, sizeof(archive), 32U, rows, 1U, &summary);
    expect(rc != 0, "NULL data is rejected");

    rc = nexus_v1_bpk_archive_prs3_payload_evidence(
        archive, sizeof(archive), 32U, rows, 1U, NULL);
    expect(rc != 0, "NULL summary is rejected");

    archive[0] = 'X';
    rc = nexus_v1_bpk_archive_prs3_payload_evidence(
        archive, sizeof(archive), 32U, rows, 1U, &summary);
    expect(rc != 0, "bad BPPK magic is rejected");
    archive[0] = 'B';
}

static void test_optional_local_menumenu_bpk(void) {
    const char *home = getenv("HOME");
    char path[1024];
    FILE *fp;
    long size;
    uint8_t *data;
    Nexus_V1_BpkPrs3PayloadEvidence *rows;
    Nexus_V1_BpkPrs3PayloadEvidenceSummary summary;
    int rc;
    uint32_t capacity = 256U;

    if (!home || !home[0]) {
        puts("SKIP: HOME is unset; no local Nexus MENU.BPK check");
        return;
    }
    if (snprintf(path, sizeof(path), "%s/.firestaff/data/nexus/MENU.BPK",
                 home) < 0) return;
    fp = fopen(path, "rb");
    if (!fp) { puts("SKIP: local MENU.BPK not present"); return; }
    if (fseek(fp, 0, SEEK_END) != 0) { fclose(fp); return; }
    size = ftell(fp);
    if (size <= 0) { fclose(fp); return; }
    if (fseek(fp, 0, SEEK_SET) != 0) { fclose(fp); return; }
    data = (uint8_t *)malloc((size_t)size);
    if (!data) { fclose(fp); return; }
    if (fread(data, 1, (size_t)size, fp) != (size_t)size) {
        free(data); fclose(fp); return;
    }
    fclose(fp);

    rows = (Nexus_V1_BpkPrs3PayloadEvidence *)calloc(
        capacity, sizeof(*rows));
    if (!rows) { free(data); return; }
    memset(&summary, 0, sizeof(summary));
    rc = nexus_v1_bpk_archive_prs3_payload_evidence(
        data, size, 4096U, rows, capacity, &summary);
    expect(rc == 0, "local MENU.BPK evidence walker returns ok");
    expect(summary.used == 162U,
           "local MENU.BPK evidence walker reports 162 PRS3 entries");
    expect(summary.total_payload < summary.total_uncompressed,
           "local MENU.BPK compressed bytes < uncompressed bytes");
    expect(summary.truncated == 0,
           "local MENU.BPK capacity not exhausted (162 <= 256)");

    free(rows);
    free(data);
}

int main(void) {
    test_evidence_walker();
    test_capacity_exhaustion();
    test_zero_sample();
    test_rejections();
    test_optional_local_menumenu_bpk();

    if (g_failures) {
        fprintf(stderr, "test_nexus_v1_bpk_prs3_payload_evidence: FAIL (%d)\n",
                g_failures);
        return 1;
    }
    puts("test_nexus_v1_bpk_prs3_payload_evidence: PASS");
    return 0;
}
