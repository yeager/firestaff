#include "nexus_v1_bpk_archive.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

static int failures;

static void check(int condition, const char *message)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        ++failures;
    }
}

static void write_be32(uint8_t *p, uint32_t value)
{
    p[0] = (uint8_t)(value >> 24);
    p[1] = (uint8_t)(value >> 16);
    p[2] = (uint8_t)(value >> 8);
    p[3] = (uint8_t)value;
}

static size_t build_exact_trial_prs3_bpk(uint8_t *data, size_t capacity)
{
    const uint32_t entry_offset = 28U;
    const uint32_t payload_offset =
        entry_offset + NEXUS_V1_BPK_ENTRY_PREFIX_BYTES;
    const uint32_t stream_offset =
        payload_offset + NEXUS_V1_BPK_PRS3_HEADER_BYTES;
    const uint8_t body[] = {0x0fU, 0x11U, 0x22U, 0x33U, 0x44U};
    const uint32_t stream_size = 4U + (uint32_t)sizeof(body);
    const uint32_t framed_size = stream_size + 4U;
    const size_t size = (size_t)stream_offset + stream_size;
    uint8_t *entry;

    if (!data || capacity < size) {
        return 0U;
    }
    memset(data, 0, capacity);
    write_be32(data + 0, NEXUS_V1_BPK_MAGIC_BPPK);
    write_be32(data + 4, (uint32_t)size);
    write_be32(data + 12, NEXUS_V1_BPK_MAGIC_BMPD);
    write_be32(data + 16, (uint32_t)(size - 12U));
    write_be32(data + 20, 1U);
    write_be32(data + 24, entry_offset);

    entry = data + entry_offset;
    entry[NEXUS_V1_BPK_PREFIX_WIDTH_OFFSET] = 0U;
    entry[NEXUS_V1_BPK_PREFIX_WIDTH_OFFSET + 1U] = 2U;
    entry[NEXUS_V1_BPK_PREFIX_HEIGHT_OFFSET] = 2U;
    entry[NEXUS_V1_BPK_PREFIX_MODE_OFFSET] = NEXUS_V1_BPK_MODE_8BPP;

    write_be32(data + payload_offset, NEXUS_V1_BPK_MAGIC_PRS3);
    write_be32(data + payload_offset + 4U, NEXUS_V1_BPK_PRS3_VERSION);
    write_be32(data + payload_offset + 8U, 4U);
    write_be32(data + stream_offset, framed_size);
    memcpy(data + stream_offset + 4U, body, sizeof(body));
    return size;
}

static void check_complete_exact_stays_evidence(void)
{
    uint8_t archive[128];
    uint8_t decoded[4];
    size_t written = 123U;
    Nexus_V1_BpkSurfaceEntry surface;
    Nexus_V1_BpkPrs3PayloadEvidence payload;
    Nexus_V1_BpkPrs3PayloadEvidenceSummary payload_summary;
    Nexus_V1_BpkPrs3CandidateEvidence candidate;
    Nexus_V1_BpkPrs3CandidateEvidenceSummary candidate_summary;
    Nexus_V1_BpkPrs3FramedEvalEvidence row;
    Nexus_V1_BpkPrs3FramedEvalSummary summary;
    Nexus_V1_BpkPrs3StreamPlan plan;
    Nexus_V1_BpkRuntimeDecodeReceipt receipt;
    size_t size = build_exact_trial_prs3_bpk(archive, sizeof(archive));

    check(size > 0U, "exact PRS3 trial fixture is bounded");
    check(nexus_v1_bpk_archive_prs3_stream_plan(
              archive, size, 0U, &plan) == NEXUS_V1_BPK_PRS3_STREAM_OK &&
              plan.decode_blocked &&
              plan.evidence_only &&
              plan.renderer_handoff_blocked &&
              plan.upload_blocked &&
              plan.decoder_promoted == 0 &&
              plan.decoded_pixels_emitted == 0U &&
              !plan.fallback_visuals_permitted,
          "PRS3 stream plan is evidence-only and cannot upload pixels");
    check(nexus_v1_bpk_archive_prs3_payload_evidence(
              archive, size, 16U, &payload, 1U, &payload_summary) == 0,
          "PRS3 payload evidence parses");
    check(payload_summary.used == 1U &&
              payload_summary.total_payload == 9U &&
              payload.payload_available &&
              payload.header_first_readable &&
              payload.header_first_u32 == 13U &&
              payload.header_minus_payload == 4U,
          "payload evidence records bounded PRS3 bytes without decoding");
    check(payload.runtime_decode_status == NEXUS_V1_BPK_DECODE_ERR_STREAM &&
              payload.runtime_decode_blocked &&
              payload.evidence_only &&
              payload.renderer_handoff_blocked &&
              payload.decoded_pixels_emitted == 0U &&
              !payload.fallback_visuals_permitted,
          "payload evidence cannot feed runtime decode or fallback visuals");
    check(payload_summary.decoder_promoted == 0 &&
              payload_summary.renderer_handoff_blocked &&
              payload_summary.decoded_pixels_emitted == 0U,
          "payload evidence summary is diagnostic-only");

    check(nexus_v1_bpk_archive_prs3_candidate_evidence_with_bit_order(
              archive, size, NEXUS_V1_BPK_PRS3_CANDIDATE_BIT_ORDER_LSB_FIRST,
              &candidate, 1U, &candidate_summary) == 0,
          "legacy PRS3 candidate evidence parses");
    check(candidate_summary.prs3_surfaces == 1U &&
              candidate_summary.evaluated == 1U &&
              candidate_summary.complete_exact == 1U &&
              candidate_summary.decoder_promoted == 0,
          "legacy complete-exact PRS3 candidate remains diagnostic evidence");
    check(candidate.status == NEXUS_V1_BPK_PRS3_CANDIDATE_COMPLETE_EXACT &&
              strcmp(nexus_v1_bpk_prs3_candidate_status_name(candidate.status),
                     "complete-exact") == 0 &&
              candidate.runtime_decode_status ==
                  NEXUS_V1_BPK_DECODE_ERR_STREAM &&
              candidate.runtime_decode_blocked &&
              candidate.evidence_only &&
              candidate.renderer_handoff_blocked &&
              candidate.decoded_pixels_emitted == 0U &&
              !candidate.fallback_visuals_permitted &&
              candidate_summary.renderer_handoff_blocked &&
              candidate_summary.decoded_pixels_emitted == 0U,
          "legacy candidate evidence records public runtime decode blocked");

    check(nexus_v1_bpk_archive_prs3_framed_decode_evidence(
              archive, size, NEXUS_V1_BPK_PRS3_CANDIDATE_BIT_ORDER_LSB_FIRST,
              &row, 1U, &summary) == 0,
          "framed PRS3 evidence parses");
    check(summary.prs3_surfaces == 1U &&
              summary.frame_validated == 1U &&
              summary.evaluated == 1U &&
              summary.complete_exact == 1U &&
              summary.decoder_promoted == 0,
          "complete-exact PRS3 trial remains diagnostic evidence");
    check(row.status == NEXUS_V1_BPK_PRS3_FRAMED_EVAL_COMPLETE_EXACT &&
              strcmp(nexus_v1_bpk_prs3_framed_eval_status_name(row.status),
                     "complete-exact") == 0 &&
              row.literal_commands == 4U &&
              row.backref_commands == 0U &&
              row.runtime_decode_status == NEXUS_V1_BPK_DECODE_ERR_STREAM &&
              row.runtime_decode_blocked &&
              row.evidence_only &&
              row.renderer_handoff_blocked &&
              row.decoded_pixels_emitted == 0U &&
              !row.fallback_visuals_permitted &&
              summary.renderer_handoff_blocked &&
              summary.decoded_pixels_emitted == 0U,
          "framed evidence records that public runtime decode is blocked");

    check(nexus_v1_bpk_archive_decode_surface(archive, size, 0U, decoded,
                                              sizeof(decoded), &surface,
                                              &written) ==
              NEXUS_V1_BPK_DECODE_ERR_STREAM &&
              written == 0U,
          "public decode_surface still rejects complete-exact PRS3 trial");
    check(nexus_v1_bpk_archive_runtime_decode_receipt(
              archive, size, &receipt) == 0,
          "runtime decode receipt parses exact PRS3 trial");
    check(receipt.route == NEXUS_V1_BPK_DECODE_ROUTE_BLOCKED_PRS3 &&
              receipt.requires_prs3_decoder &&
              receipt.prs3_decode_successes == 0U &&
              receipt.prs3_decode_failures == 1U &&
              receipt.prs3_evidence_only &&
              receipt.prs3_decoder_promoted == 0 &&
              receipt.prs3_decoded_pixels_emitted == 0U &&
              receipt.renderer_handoff_blocked &&
              !receipt.fallback_visuals_permitted &&
              receipt.decode_blocked,
          "runtime route remains blocked-prs3 after complete-exact evidence");
}

int main(void)
{
    check_complete_exact_stays_evidence();

    if (failures) {
        fprintf(stderr, "Nexus PRS3 evidence gate: %d failure(s)\n",
                failures);
        return 1;
    }
    puts("Nexus PRS3 evidence gate: PASS");
    return 0;
}
