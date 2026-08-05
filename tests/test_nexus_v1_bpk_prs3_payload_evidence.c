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

static uint64_t fnv1a64(const uint8_t *data, size_t size) {
    uint64_t hash = UINT64_C(1469598103934665603);
    size_t i;

    if (!data || size == 0U) return 0U;
    for (i = 0U; i < size; ++i) {
        hash ^= (uint64_t)data[i];
        hash *= UINT64_C(1099511628211);
    }
    return hash;
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

#define OPCODE_ARCHIVE_SIZE (24 + 8 + 20 + 32 + 12)
static uint8_t opcode_archive[OPCODE_ARCHIVE_SIZE];

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

static void build_opcode_archive(void) {
    const uint32_t trailer_off = 24U + 8U;
    const uint32_t entry1_off = trailer_off + 20U;

    memset(opcode_archive, 0, sizeof(opcode_archive));
    memcpy(opcode_archive, "BPPK", 4);
    wr32_be(opcode_archive + 4, (uint32_t)sizeof(opcode_archive));
    memcpy(opcode_archive + 12, "BMPD", 4);
    wr32_be(opcode_archive + 16, (uint32_t)(sizeof(opcode_archive) - 20U));
    wr32_be(opcode_archive + 20, 2U);
    wr32_be(opcode_archive + 24, trailer_off);
    wr32_be(opcode_archive + 28, entry1_off);

    opcode_archive[trailer_off + NEXUS_V1_BPK_PREFIX_MODE_OFFSET] =
        NEXUS_V1_BPK_MODE_TRAILER;

    {
        uint8_t *r = opcode_archive + entry1_off;
        memset(r, 0, NEXUS_V1_BPK_ENTRY_PREFIX_BYTES);
        wr16_be(r + NEXUS_V1_BPK_PREFIX_WIDTH_OFFSET, 4U);
        r[NEXUS_V1_BPK_PREFIX_HEIGHT_OFFSET] = 2U;
        r[NEXUS_V1_BPK_PREFIX_MODE_OFFSET] = NEXUS_V1_BPK_MODE_8BPP;
        memcpy(r + NEXUS_V1_BPK_ENTRY_PREFIX_BYTES, "PRS3", 4);
        wr32_be(r + NEXUS_V1_BPK_ENTRY_PREFIX_BYTES + 4U, 1U);
        wr32_be(r + NEXUS_V1_BPK_ENTRY_PREFIX_BYTES + 8U, 8U);
        /* stream_size is 12, so the validated frame word is stream+4. */
        wr32_be(r + NEXUS_V1_BPK_ENTRY_PREFIX_BYTES + 12U, 16U);
        /* LSB-first witness: literal 0xaa, backref 0x34 0x20, literal 0xbb. */
        r[NEXUS_V1_BPK_ENTRY_PREFIX_BYTES + 16U] = 0x05U;
        r[NEXUS_V1_BPK_ENTRY_PREFIX_BYTES + 17U] = 0xaaU;
        r[NEXUS_V1_BPK_ENTRY_PREFIX_BYTES + 18U] = 0x34U;
        r[NEXUS_V1_BPK_ENTRY_PREFIX_BYTES + 19U] = 0x20U;
        r[NEXUS_V1_BPK_ENTRY_PREFIX_BYTES + 20U] = 0xbbU;
    }
}

static void test_evidence_walker(void) {
    Nexus_V1_BpkPrs3PayloadEvidence rows[8];
    Nexus_V1_BpkPrs3PayloadEvidenceSummary summary;
    Nexus_V1_BpkPrs3StreamPlan plan;
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

    /* Row 0 is the 16x15 PRS3 entry; mode 14 is opaque and output is indexed. */
    expect(rows[0].entry_index == 1U, "row 0 entry_index == 1");
    expect(rows[0].mode == NEXUS_V1_BPK_MODE_16BPP,
           "row 0 mode == MODE_16BPP");
    expect(rows[0].bpp == 2U, "row 0 PRS3 bpp == 2");
    expect(rows[0].uncompressed_size == 480U,
           "row 0 PRS3 uncompressed_size == 480");
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

    memset(&plan, 0, sizeof(plan));
    rc = nexus_v1_bpk_archive_prs3_stream_plan(
        archive, sizeof(archive), 1U, &plan);
    expect(rc == NEXUS_V1_BPK_PRS3_STREAM_OK,
           "stream plan returns OK for PRS3 entry 1");
    expect(strcmp(nexus_v1_bpk_prs3_stream_status_name(rc), "ok") == 0,
           "stream plan status name is ok");
    expect(plan.entry_index == 1U &&
               plan.mode == NEXUS_V1_BPK_MODE_16BPP &&
               plan.width == 16U &&
               plan.height == 15U &&
               plan.bpp == 2U &&
               plan.pixel_count == 240U &&
               plan.expected_output_bytes == 480U,
           "stream plan carries surface dimensions and output byte count");
    expect(plan.stream_size == 32U &&
               plan.body_size == 28U &&
               plan.body_offset == plan.stream_offset + 4U,
           "stream plan separates leading word from body bytes");
    expect(plan.header_first_u32 == 0x83U &&
               plan.header_minus_payload == 99U &&
               plan.header_underflow == 0 &&
               plan.bounded_header_candidate == 0 &&
               plan.decode_blocked == 1,
           "stream plan records oversized synthetic header word and blocks decode");

    memset(&plan, 0, sizeof(plan));
    rc = nexus_v1_bpk_archive_prs3_stream_plan(
        archive, sizeof(archive), 2U, &plan);
    expect(rc == NEXUS_V1_BPK_PRS3_STREAM_OK,
           "stream plan returns OK for PRS3 entry 2");
    expect(plan.stream_size == 4U &&
               plan.body_size == 0U &&
               plan.header_first_u32 == 0x07U &&
               plan.header_minus_payload == 3U &&
               plan.bounded_header_candidate == 1,
           "stream plan accepts a bounded 4-byte header-only stream");
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
    Nexus_V1_BpkPrs3StreamPlan plan;
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

    memset(&plan, 0, sizeof(plan));
    rc = nexus_v1_bpk_archive_prs3_stream_plan(
        archive, sizeof(archive), 0U, &plan);
    expect(rc == NEXUS_V1_BPK_PRS3_STREAM_ERR_NOT_PRS3,
           "stream plan rejects directory trailer as not-prs3");
    expect(strcmp(nexus_v1_bpk_prs3_stream_status_name(rc),
                  "not-prs3") == 0,
           "stream plan not-prs3 status name is stable");

    rc = nexus_v1_bpk_archive_prs3_stream_plan(
        archive, sizeof(archive), 1U, NULL);
    expect(rc == NEXUS_V1_BPK_PRS3_STREAM_ERR_NULL,
           "stream plan rejects NULL output");
}

static void test_framing_evidence(void) {
    Nexus_V1_BpkPrs3FramingEvidence rows[8];
    Nexus_V1_BpkPrs3FramingEvidenceSummary summary;
    int rc;

    build_archive();
    rc = nexus_v1_bpk_archive_prs3_framing_evidence(
        archive, sizeof(archive), rows, 8U, &summary);
    expect(rc == 0, "framing evidence accepts bounded synthetic archive");
    expect(summary.prs3_entries == 2U && summary.readable_first_words == 2U,
           "framing evidence reports both synthetic PRS3 entries");
    expect(summary.be_shorter_than_stream == 0U &&
               summary.decoder_promoted == 0,
           "framing evidence remains diagnostic-only");
    expect(nexus_v1_bpk_archive_prs3_framing_evidence(
               archive, sizeof(archive), rows, 8U, NULL) != 0,
           "framing evidence rejects NULL summary");
}

static void test_framed_decode_evaluation(void) {
    Nexus_V1_BpkPrs3FramedEvalEvidence rows[8];
    Nexus_V1_BpkPrs3FramedEvalSummary summary;
    int rc;

    build_archive();
    rc = nexus_v1_bpk_archive_prs3_framed_decode_evidence(
        archive, sizeof(archive),
        NEXUS_V1_BPK_PRS3_CANDIDATE_BIT_ORDER_LSB_FIRST,
        rows, 8U, &summary);
    expect(rc == 0, "framed LSB evaluation accepts bounded synthetic archive");
    expect(summary.prs3_surfaces == 2U && summary.frame_validated == 0U &&
               summary.unvalidated_frames == 2U && summary.evaluated == 0U &&
               summary.command_failures == 0U && summary.complete_exact == 0U &&
               summary.decoder_promoted == 0,
           "framed LSB evaluation rejects non-span-close synthetic frames");
    expect(rows[0].status == NEXUS_V1_BPK_PRS3_FRAMED_EVAL_UNVALIDATED_FRAME &&
               rows[1].status == NEXUS_V1_BPK_PRS3_FRAMED_EVAL_UNVALIDATED_FRAME,
           "framed evaluation records unvalidated synthetic frame boundaries");
    expect(strcmp(nexus_v1_bpk_prs3_framed_eval_status_name(rows[1].status),
                  "unvalidated-frame") == 0,
           "framed evaluation status name is stable");

    rc = nexus_v1_bpk_archive_prs3_framed_decode_evidence(
        archive, sizeof(archive),
        NEXUS_V1_BPK_PRS3_CANDIDATE_BIT_ORDER_MSB_FIRST,
        rows, 8U, &summary);
    expect(rc == 0 && summary.frame_validated == 0U &&
               summary.unvalidated_frames == 2U && summary.complete_exact == 0U &&
               summary.decoder_promoted == 0,
           "framed MSB evaluation is equally diagnostic-only");
    expect(nexus_v1_bpk_archive_prs3_framed_decode_evidence(
               archive, sizeof(archive),
               (Nexus_V1_BpkPrs3CandidateBitOrder)99,
               rows, 8U, &summary) != 0,
           "framed evaluation rejects an unknown bit order");
}

static void test_opcode_prefix_witness(void) {
    Nexus_V1_BpkPrs3OpcodePrefixEvidence rows[4];
    Nexus_V1_BpkPrs3OpcodePrefixSummary summary;
    int rc;

    build_opcode_archive();
    memset(rows, 0, sizeof(rows));
    memset(&summary, 0, sizeof(summary));

    rc = nexus_v1_bpk_archive_prs3_opcode_prefix_witness(
        opcode_archive, sizeof(opcode_archive),
        NEXUS_V1_BPK_PRS3_CANDIDATE_BIT_ORDER_LSB_FIRST,
        3U, rows, 4U, &summary);
    expect(rc == 0, "opcode-prefix witness accepts validated synthetic frame");
    expect(summary.prs3_surfaces == 1U &&
               summary.frame_validated == 1U &&
               summary.witnessed == 1U &&
               summary.command_limit_reached == 1U &&
               summary.decoder_promoted == 0,
           "opcode-prefix witness reports one bounded diagnostic row");
    expect(rows[0].entry_index == 1U &&
               rows[0].body_size == 8U &&
               rows[0].first_control_byte == 0x05U &&
               rows[0].commands_observed == 3U,
           "opcode-prefix row preserves frame body and first control byte");
    expect(rows[0].literal_commands == 2U &&
               rows[0].backref_commands == 1U &&
               rows[0].control_bytes_consumed == 1U &&
               rows[0].operand_bytes_consumed == 4U &&
               rows[0].body_bytes_consumed == 5U,
           "opcode-prefix row counts LSB-first control and operands only");
    expect(rows[0].first_backref_observed == 1 &&
               rows[0].first_backref_raw_offset == 0x234U &&
               rows[0].first_backref_length == 3U,
           "opcode-prefix row records first backref operands without output");
    expect(strcmp(nexus_v1_bpk_prs3_opcode_prefix_status_name(rows[0].status),
                  "command-limit") == 0,
           "opcode-prefix status name is stable");

    rc = nexus_v1_bpk_archive_prs3_opcode_prefix_witness(
        opcode_archive, sizeof(opcode_archive),
        NEXUS_V1_BPK_PRS3_CANDIDATE_BIT_ORDER_LSB_FIRST,
        0U, rows, 4U, &summary);
    expect(rc != 0, "opcode-prefix witness rejects zero command limit");

    rc = nexus_v1_bpk_archive_prs3_opcode_prefix_witness(
        opcode_archive, sizeof(opcode_archive),
        (Nexus_V1_BpkPrs3CandidateBitOrder)99,
        3U, rows, 4U, &summary);
    expect(rc != 0, "opcode-prefix witness rejects unknown bit order");
}

static void test_optional_local_menumenu_bpk(void) {
    const char *home = getenv("HOME");
    char path[2048];
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

    {
        Nexus_V1_BpkPrs3StreamPlan plan;
        memset(&plan, 0, sizeof(plan));
        rc = nexus_v1_bpk_archive_prs3_stream_plan(data, size, 1U, &plan);
        expect(rc == NEXUS_V1_BPK_PRS3_STREAM_OK,
               "local MENU.BPK entry[1] stream plan returns ok");
        expect(plan.header_first_readable == 1 &&
                   plan.header_first_u32 == 0x95U &&
                   plan.header_minus_payload <= 16U &&
                   plan.bounded_header_candidate == 1 &&
                   plan.decode_blocked == 1,
               "local MENU.BPK entry[1] stream plan locks bounded PRS3 header");
    }

    {
        Nexus_V1_BpkPrs3FramedEvalEvidence framed_rows[256];
        Nexus_V1_BpkPrs3FramedEvalSummary framed;
        Nexus_V1_BpkPrs3OpcodePrefixEvidence opcode_rows[256];
        Nexus_V1_BpkPrs3OpcodePrefixSummary opcode;
        for (int order = NEXUS_V1_BPK_PRS3_CANDIDATE_BIT_ORDER_LSB_FIRST;
             order <= NEXUS_V1_BPK_PRS3_CANDIDATE_BIT_ORDER_MSB_FIRST;
             ++order) {
            rc = nexus_v1_bpk_archive_prs3_framed_decode_evidence(
                data, size, (Nexus_V1_BpkPrs3CandidateBitOrder)order,
                framed_rows, 256U, &framed);
            expect(rc == 0 && framed.prs3_surfaces == 162U &&
                       framed.frame_validated == 158U &&
                       framed.unvalidated_frames == 4U &&
                       framed.evaluated == 158U &&
                       framed.complete_exact == 0U &&
                       framed.decoder_promoted == 0,
                   "local MENU.BPK framed evaluation remains diagnostic-only");
            rc = nexus_v1_bpk_archive_prs3_opcode_prefix_witness(
                data, size, (Nexus_V1_BpkPrs3CandidateBitOrder)order,
                16U, opcode_rows, 256U, &opcode);
            expect(rc == 0 && opcode.prs3_surfaces == 162U &&
                       opcode.frame_validated == 158U &&
                       opcode.unvalidated_frames == 4U &&
                       opcode.witnessed == 158U &&
                       opcode.decoder_promoted == 0,
                   "local MENU.BPK opcode-prefix witness remains diagnostic-only");
        }
    }

    {
        Nexus_V1_BpkPrs3StreamPlan plan;
        Nexus_V1_BpkPrs3DecodedOutputProofReceipt proof;
        uint8_t *captured_output;
        uint64_t captured_hash;

        memset(&plan, 0, sizeof(plan));
        rc = nexus_v1_bpk_archive_prs3_stream_plan(data, size, 1U, &plan);
        if (rc == NEXUS_V1_BPK_PRS3_STREAM_OK &&
            plan.expected_output_bytes > 0U &&
            plan.expected_output_bytes <= 1024U * 1024U) {
            captured_output = (uint8_t *)calloc(
                plan.expected_output_bytes, sizeof(*captured_output));
            if (captured_output) {
                captured_hash = fnv1a64(captured_output,
                                        plan.expected_output_bytes);
                memset(&proof, 0, sizeof(proof));
                rc = nexus_v1_bpk_archive_prs3_decoded_output_proof_gate(
                    data, size, 1U, captured_output,
                    plan.expected_output_bytes, captured_hash,
                    1, 0, &proof);
                expect(rc == 0 &&
                           proof.length_matches &&
                           proof.hash_matches &&
                           proof.decoded_output_sidecar_bound &&
                           !proof.original_saturn_provenance_verified &&
                           !proof.decoded_output_proof_ready &&
                           !proof.decoder_promoted &&
                           !proof.runtime_upload_permitted &&
                           !proof.fallback_visuals_permitted &&
                           proof.status ==
                               NEXUS_V1_BPK_PRS3_OUTPUT_PROOF_PROVENANCE_REQUIRED,
                       "local MENU.BPK decoded-output sidecar remains provenance-blocked");
                expect(strcmp(
                           nexus_v1_bpk_prs3_decoded_output_proof_status_name(
                               proof.status),
                           "provenance-required") == 0,
                       "decoded-output proof status name is stable");

                rc = nexus_v1_bpk_archive_prs3_decoded_output_proof_gate(
                    data, size, 1U, captured_output,
                    plan.expected_output_bytes, captured_hash,
                    1, 1, &proof);
                expect(rc == 1 &&
                           proof.length_matches &&
                           proof.hash_matches &&
                           proof.capture_source_bound &&
                           proof.decoded_output_sidecar_bound &&
                           proof.original_saturn_provenance_verified &&
                           proof.decoded_output_proof_ready &&
                           !proof.opcode_grammar_proven &&
                           !proof.decoder_promoted &&
                           !proof.runtime_upload_permitted &&
                           !proof.fallback_visuals_permitted &&
                           proof.status ==
                               NEXUS_V1_BPK_PRS3_OUTPUT_PROOF_SOURCE_BOUND_NO_RUNTIME,
                       "local MENU.BPK decoded-output proof admits original Saturn provenance without runtime promotion");
                expect(strcmp(
                           nexus_v1_bpk_prs3_decoded_output_proof_status_name(
                               proof.status),
                           "source-bound-no-runtime") == 0,
                       "decoded-output proof ready status still names the no-runtime blocker");

                rc = nexus_v1_bpk_archive_prs3_decoded_output_proof_gate(
                    data, size, 1U, captured_output,
                    plan.expected_output_bytes, captured_hash ^ 1U,
                    1, 0, &proof);
                expect(rc == 0 &&
                           proof.status ==
                               NEXUS_V1_BPK_PRS3_OUTPUT_PROOF_HASH_MISMATCH &&
                           !proof.decoded_output_sidecar_bound &&
                           !proof.decoder_promoted &&
                           !proof.runtime_upload_permitted,
                       "local MENU.BPK decoded-output proof rejects hash drift");
                free(captured_output);
            }
        }
    }

    free(rows);
    free(data);
}

int main(void) {
    test_evidence_walker();
    test_capacity_exhaustion();
    test_zero_sample();
    test_rejections();
    test_framing_evidence();
    test_framed_decode_evaluation();
    test_opcode_prefix_witness();
    test_optional_local_menumenu_bpk();

    if (g_failures) {
        fprintf(stderr, "test_nexus_v1_bpk_prs3_payload_evidence: FAIL (%d)\n",
                g_failures);
        return 1;
    }
    puts("test_nexus_v1_bpk_prs3_payload_evidence: PASS");
    return 0;
}
