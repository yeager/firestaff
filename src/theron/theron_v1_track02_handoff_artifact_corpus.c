#include "theron_v1_track02_handoff_artifact_corpus.h"

#include <stdio.h>
#include <string.h>

static int plan_matches_handoff(const Theron_V1Track02ExternalCaptureReceipt *handoff,
                                const Theron_V1Track02CaptureTargetPlan *plan)
{
    uint32_t identity = theron_v1_track02_capture_target_plan_identity(plan);
    size_t i;
    if (!handoff || !plan || handoff->status != THERON_V1_TRACK02_EXTERNAL_CAPTURE_RUNTIME_READY ||
        !handoff->raw_media_intake_verified || !handoff->mednafen_trace_source_verified ||
        !handoff->capture_target_plan_verified || !handoff->positive_handoff_capture_required ||
        !handoff->track02_md5[0] || !handoff->mednafen_trace_source_md5[0] ||
        !identity || identity != handoff->capture_target_plan_identity) return 0;
    for (i = 0u; i < THERON_V1_TRACK02_CAPTURE_TARGET_COUNT; ++i) {
        if (plan->targets[i].route != (Theron_V1Track02CaptureTargetRoute)i ||
            strcmp(plan->targets[i].track02_md5, handoff->track02_md5)) return 0;
    }
    return 1;
}

static int artifact_is_opaque_and_complete(
    const Theron_V1Track02HandoffArtifactCorpusReceipt *receipt)
{
    const Theron_V1Track02CaptureArtifactRuntimeAdmissionReceipt *artifact;
    size_t i;

    if (!receipt ||
        receipt->status != THERON_V1_TRACK02_HANDOFF_ARTIFACT_CORPUS_READY ||
        receipt->supplied_candidate_count != 1u ||
        receipt->direct_candidate_count != 1u || receipt->rejected_candidate_count ||
        !receipt->direct_cue_bin_consumed || !receipt->source_trace_md5_verified ||
        !receipt->capture_target_plan_consumed || !receipt->opaque_artifact_consumed ||
        !receipt->capture_required_only || !receipt->no_draw_only ||
        !receipt->track02_md5[0] || !receipt->source_trace_md5[0] ||
        !receipt->capture_target_plan_identity) return 0;
    artifact = &receipt->artifact;
    if (artifact->status != THERON_V1_TRACK02_CAPTURE_ARTIFACT_READY ||
        !artifact->bundle_md5_verified || !artifact->mednafen_trace_md5_verified ||
        !artifact->complete_route_set_consumed || !artifact->opaque_envelope_verified ||
        !artifact->opaque_runtime_ready || artifact->pixel_decode_allowed ||
        artifact->level_object_semantics_allowed || artifact->render_allowed ||
        artifact->fallback_visuals_allowed || !artifact->track02_md5[0] ||
        !artifact->bundle_md5[0] || !artifact->mednafen_trace_md5[0] ||
        artifact->capture_target_plan_identity != receipt->capture_target_plan_identity ||
        strcmp(artifact->track02_md5, receipt->track02_md5) ||
        strcmp(artifact->mednafen_trace_md5, receipt->source_trace_md5) ||
        artifact->campaign_route != THERON_V1_TRACK02_CAPTURE_TARGET_DUNGEON_HANDOFF ||
        !artifact->descriptor_selector || !artifact->descriptor_source_hash) return 0;
    for (i = 0u; i < THERON_V1_TRACK02_CAPTURE_TARGET_COUNT; ++i) {
        if (!artifact->cd_read_record[i] || !artifact->loader_output_bytes[i] ||
            !artifact->loader_output_identity[i] ||
            !artifact->palette_output_identity[i] ||
            !artifact->bitmap_transfer_identity[i] || !artifact->destination_record[i] ||
            !artifact->destination_bytes[i] || !artifact->destination_identity[i]) return 0;
    }
    return 1;
}

int theron_v1_track02_handoff_artifact_corpus_matches_identity(
    const Theron_V1Track02HandoffArtifactCorpusReceipt *receipt,
    const char *track02_md5,
    const char *source_trace_md5,
    uint32_t capture_target_plan_identity)
{
    if (!artifact_is_opaque_and_complete(receipt) || !track02_md5 ||
        !source_trace_md5 || !track02_md5[0] || !source_trace_md5[0] ||
        !capture_target_plan_identity ||
        receipt->capture_target_plan_identity != capture_target_plan_identity ||
        strcmp(receipt->track02_md5, track02_md5) ||
        strcmp(receipt->source_trace_md5, source_trace_md5)) return 0;
    return 1;
}

int theron_v1_track02_handoff_artifact_corpus_matches_plan(
    const Theron_V1Track02HandoffArtifactCorpusReceipt *receipt,
    const Theron_V1Track02CaptureTargetPlan *plan,
    const char *expected_source_trace_md5)
{
    uint32_t plan_identity;
    size_t i;

    if (!plan) return 0;
    plan_identity = theron_v1_track02_capture_target_plan_identity(plan);
    if (!plan_identity || !theron_v1_track02_handoff_artifact_corpus_matches_identity(
            receipt, plan->targets[0].track02_md5, receipt ? receipt->source_trace_md5 : NULL,
            plan_identity) ||
        (expected_source_trace_md5 &&
         (!expected_source_trace_md5[0] ||
          strcmp(receipt->source_trace_md5, expected_source_trace_md5)))) return 0;
    for (i = 0u; i < THERON_V1_TRACK02_CAPTURE_TARGET_COUNT; ++i) {
        const Theron_V1Track02CaptureTarget *target = &plan->targets[i];
        if (target->route != (Theron_V1Track02CaptureTargetRoute)i ||
            target->track02_variant != receipt->artifact.track02_variant ||
            strcmp(target->track02_md5, receipt->track02_md5) ||
            receipt->artifact.cd_read_record[i] != target->cd_read_record ||
            receipt->artifact.loader_output_raw_offset[i] !=
                target->loader_output_raw_offset ||
            receipt->artifact.loader_output_bytes[i] != target->loader_output_bytes ||
            receipt->artifact.loader_output_identity[i] != target->loader_output_checksum ||
            receipt->artifact.palette_output_identity[i] != target->palette_output_identity ||
            receipt->artifact.bitmap_transfer_identity[i] != target->bitmap_identity ||
            receipt->artifact.destination_record[i] != target->destination_record ||
            receipt->artifact.destination_offset[i] != target->destination_offset ||
            receipt->artifact.destination_bytes[i] != target->destination_bytes ||
            receipt->artifact.destination_identity[i] != target->destination_identity) return 0;
    }
    return 1;
}

int theron_v1_track02_handoff_artifact_corpus_matches_sector_record(
    const Theron_V1Track02HandoffArtifactCorpusReceipt *receipt,
    const Theron_V1Track02SectorRecordCorpusDiscoveryReceipt *sector_corpus,
    const Theron_V1Track02CaptureTargetPlan *plan,
    const char *expected_source_trace_md5)
{
    const Theron_V1Track02SectorRecordAdmissionReceipt *sector;
    const Theron_V1Track02CaptureTarget *target;
    const Theron_V1Track02CaptureArtifactRuntimeAdmissionReceipt *artifact;

    if (!sector_corpus || !plan ||
        sector_corpus->status != THERON_V1_TRACK02_SECTOR_RECORD_CORPUS_READY ||
        sector_corpus->direct_candidate_count != 1u ||
        !sector_corpus->direct_regular_files_verified ||
        !sector_corpus->track02_md5_verified || !sector_corpus->trace_md5_verified ||
        !sector_corpus->coalesced_trace_md5[0] ||
        !theron_v1_track02_handoff_artifact_corpus_matches_plan(
            receipt, plan, expected_source_trace_md5) ||
        strcmp(receipt->track02_md5, sector_corpus->media.track02_md5) ||
        strcmp(receipt->source_trace_md5, sector_corpus->coalesced_trace_md5)) {
        return 0;
    }
    sector = &sector_corpus->sector_record;
    target = &plan->targets[THERON_V1_TRACK02_CAPTURE_TARGET_DUNGEON_HANDOFF];
    artifact = &receipt->artifact;
    return sector->status == THERON_V1_TRACK02_SECTOR_RECORD_READY &&
        sector->raw_cue_bin_identity_consumed && sector->stage3_directory_consumed &&
        sector->observed_later_loader_consumed && sector->nonstartup_record_consumed &&
        !sector->level_object_semantics_allowed &&
        !sector->bitmap_palette_admission_allowed && !sector->pixel_decode_allowed &&
        !sector->dungeon_draw_allowed && !sector->fallback_visuals_allowed &&
        sector->descriptor_selector && sector->descriptor_source_hash &&
        sector->resolved_track02_record > sector->stage3_track02_record &&
        sector->loader_destination && sector->loader_destination_span_bytes &&
        sector->loader_destination_span_checksum &&
        sector->loader_destination_payload_bytes &&
        sector->loader_destination_payload_checksum &&
        sector->track02_variant == artifact->track02_variant &&
        !strcmp(sector->track02_md5, receipt->track02_md5) &&
        target->cd_read_record == sector->resolved_track02_record &&
        target->loader_output_bytes == sector->loader_destination_span_bytes &&
        target->loader_output_checksum == sector->loader_destination_span_checksum &&
        artifact->cd_read_record[THERON_V1_TRACK02_CAPTURE_TARGET_DUNGEON_HANDOFF] ==
            sector->resolved_track02_record &&
        artifact->loader_output_bytes[THERON_V1_TRACK02_CAPTURE_TARGET_DUNGEON_HANDOFF] ==
            sector->loader_destination_span_bytes &&
        artifact->loader_output_identity[THERON_V1_TRACK02_CAPTURE_TARGET_DUNGEON_HANDOFF] ==
            sector->loader_destination_span_checksum &&
        artifact->destination_record[THERON_V1_TRACK02_CAPTURE_TARGET_DUNGEON_HANDOFF] ==
            sector->resolved_track02_record &&
        artifact->descriptor_selector == sector->descriptor_selector &&
        artifact->descriptor_ordinal == sector->descriptor_ordinal &&
        artifact->descriptor_source_hash == sector->descriptor_source_hash;
}

int theron_v1_track02_handoff_artifact_corpus_import(
    const Theron_V1Track02ExternalCaptureReceipt *handoff,
    const Theron_V1Track02CaptureTargetPlan *plan,
    const Theron_V1Track02HandoffArtifactCorpusCandidate *candidates,
    unsigned int candidate_count,
    Theron_V1Track02HandoffArtifactCorpusReceipt *out)
{
    Theron_V1Track02HandoffArtifactCorpusReceipt receipt = {0};
    Theron_V1Track02RawMediaIntakeReceipt media;
    Theron_V1Track02MednafenTraceConvertReceipt trace;
    Theron_V1Track02CaptureArtifactImportRequest request;

    if (!out) return 0;
    *out = receipt;
    receipt.supplied_candidate_count = candidate_count;
    if (!candidate_count) {
        receipt.status = THERON_V1_TRACK02_HANDOFF_ARTIFACT_CORPUS_UNAVAILABLE;
        *out = receipt;
        return 1;
    }
    if (candidate_count != 1u || !candidates || !candidates[0].bundle_path ||
        !candidates[0].expected_bundle_md5 || !candidates[0].bundle_path[0] ||
        !candidates[0].expected_bundle_md5[0] || strstr(candidates[0].bundle_path, "::") ||
        !plan_matches_handoff(handoff, plan) ||
        !theron_v1_track02_raw_media_intake_discover(handoff->media_path, &media) ||
        media.status != THERON_V1_TRACK02_MEDIA_INTAKE_READY || !media.cue_consumed ||
        !media.mode1_2352 || !media.raw_trace_preparation_allowed ||
        strcmp(media.track02_md5, handoff->track02_md5) ||
        !theron_v1_track02_mednafen_trace_inspect_file(
            handoff->mednafen_trace_source_path, &trace) ||
        trace.status != THERON_V1_TRACK02_MEDNAFEN_TRACE_INSPECTED ||
        strcmp(trace.source_trace_md5, handoff->mednafen_trace_source_md5)) {
        receipt.status = THERON_V1_TRACK02_HANDOFF_ARTIFACT_CORPUS_REJECTED;
        receipt.rejected_candidate_count = candidate_count;
        *out = receipt;
        return 1;
    }
    request.bundle_path = candidates[0].bundle_path;
    request.expected_bundle_md5 = candidates[0].expected_bundle_md5;
    request.mednafen_trace_path = handoff->mednafen_trace_source_path;
    request.expected_mednafen_trace_md5 = handoff->mednafen_trace_source_md5;
    if (!theron_v1_track02_capture_artifact_import(plan, &request, &receipt.artifact) ||
        receipt.artifact.status != THERON_V1_TRACK02_CAPTURE_ARTIFACT_READY ||
        receipt.artifact.capture_target_plan_identity != handoff->capture_target_plan_identity ||
        strcmp(receipt.artifact.track02_md5, handoff->track02_md5) ||
        strcmp(receipt.artifact.mednafen_trace_md5, handoff->mednafen_trace_source_md5)) {
        receipt.status = THERON_V1_TRACK02_HANDOFF_ARTIFACT_CORPUS_REJECTED;
        memset(&receipt.artifact, 0, sizeof(receipt.artifact));
        *out = receipt;
        return 1;
    }
    receipt.status = THERON_V1_TRACK02_HANDOFF_ARTIFACT_CORPUS_READY;
    receipt.direct_candidate_count = 1u;
    receipt.direct_cue_bin_consumed = receipt.source_trace_md5_verified = 1;
    receipt.capture_target_plan_consumed = receipt.opaque_artifact_consumed = 1;
    receipt.capture_required_only = receipt.no_draw_only = 1;
    snprintf(receipt.track02_md5, sizeof(receipt.track02_md5), "%s", handoff->track02_md5);
    snprintf(receipt.source_trace_md5, sizeof(receipt.source_trace_md5), "%s",
             handoff->mednafen_trace_source_md5);
    receipt.capture_target_plan_identity = handoff->capture_target_plan_identity;
    *out = receipt;
    return 1;
}
