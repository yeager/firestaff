#include "theron_v1_track02_later_route_candidate_plan_corpus_admission.h"

#include <stdio.h>
#include <string.h>

int theron_v1_track02_later_route_candidate_plan_corpus_admit(
    const Theron_V1Track02LaterRouteCandidateCampaignIndex *candidates,
    const Theron_V1Track02CaptureTargetPlan *plan,
    const Theron_V1Track02HandoffArtifactCorpusReceipt *artifact_corpus,
    const Theron_V1Track02LoaderTraceReplayConsistencyReceipt *replay,
    const Theron_V1Track02LaunchTraceIdentityReceipt *trace_identity,
    uint32_t campaign_layout_epoch, uint32_t campaign_media_scan_epoch,
    Theron_V1Track02LaterRouteCandidatePlanCorpusAdmissionReceipt *out)
{
    Theron_V1Track02LaterRouteCandidatePlanCorpusAdmissionReceipt receipt = {0};
    Theron_V1Track02LaterRouteCandidateCampaignIndex current;
    const Theron_V1Track02LaterRouteCandidateReceipt *candidate;
    const Theron_V1Track02CaptureTarget *target;
    uint32_t plan_identity;

    if (!out) return 0;
    *out = receipt;
    plan_identity = theron_v1_track02_capture_target_plan_identity(plan);
    if (!campaign_layout_epoch || !campaign_media_scan_epoch || !plan_identity ||
        !theron_v1_track02_later_route_candidate_campaign_index_current(
            candidates, campaign_layout_epoch, &current) || current.count != 1u ||
        !artifact_corpus || !replay || !trace_identity || !trace_identity->valid ||
        !trace_identity->direct_campaign_consumed || !trace_identity->loader_trace_consumed ||
        !trace_identity->event_log_consumed ||
        trace_identity->campaign_layout_epoch != campaign_layout_epoch ||
        !theron_v1_track02_handoff_artifact_corpus_matches_plan(
            artifact_corpus, plan, trace_identity->source_trace_md5) ||
        !replay->active || !replay->direct_campaign_layout_consumed ||
        !replay->dynamic_cd_read_records_consumed ||
        replay->campaign_layout_epoch != campaign_layout_epoch ||
        replay->level_object_semantics_allowed || replay->bitmap_palette_admission_allowed ||
        replay->pixel_decode_allowed || replay->dungeon_draw_allowed ||
        replay->fallback_visuals_allowed ||
        trace_identity->level_object_semantics_allowed ||
        trace_identity->bitmap_palette_admission_allowed || trace_identity->pixel_decode_allowed ||
        trace_identity->dungeon_draw_allowed || trace_identity->fallback_visuals_allowed) return 0;
    candidate = &current.entries[0];
    target = &plan->targets[THERON_V1_TRACK02_CAPTURE_TARGET_DUNGEON_HANDOFF];
    if (candidate->record != replay->last_track02_record ||
        candidate->raw_sector != replay->last_raw_sector ||
        candidate->record != target->cd_read_record ||
        candidate->destination_identity != target->destination_identity ||
        candidate->destination_identity != artifact_corpus->artifact.destination_identity[
            THERON_V1_TRACK02_CAPTURE_TARGET_DUNGEON_HANDOFF] ||
        candidate->loader_pc == 0u || candidate->campaign_layout_epoch != campaign_layout_epoch ||
        candidate->record != candidate->raw_sector ||
        candidate->status != THERON_V1_TRACK02_LATER_ROUTE_CAPTURE_REQUIRED ||
        !candidate->observed_trace_row_consumed || !candidate->direct_media_consumed ||
        !candidate->replay_tail_consumed || !candidate->opaque_only ||
        candidate->track02_md5[0] == '\0' || candidate->source_trace_md5[0] == '\0' ||
        strcmp(candidate->track02_md5, artifact_corpus->track02_md5) ||
        strcmp(candidate->track02_md5, replay->track02_md5) ||
        strcmp(candidate->track02_md5, trace_identity->track02_md5) ||
        strcmp(candidate->source_trace_md5, artifact_corpus->source_trace_md5) ||
        strcmp(candidate->source_trace_md5, trace_identity->source_trace_md5) ||
        replay->track02_variant != target->track02_variant ||
        replay->track02_variant != trace_identity->track02_variant ||
        artifact_corpus->capture_target_plan_identity != plan_identity) return 0;
    receipt.valid = receipt.candidate_family_consumed = receipt.artifact_corpus_consumed = 1;
    receipt.capture_plan_consumed = receipt.replay_tail_consumed = 1;
    receipt.capture_required_only = receipt.no_draw_only = 1;
    receipt.campaign_layout_epoch = campaign_layout_epoch;
    receipt.campaign_media_scan_epoch = campaign_media_scan_epoch;
    receipt.capture_target_plan_identity = plan_identity;
    receipt.track02_variant = target->track02_variant;
    snprintf(receipt.track02_md5, sizeof(receipt.track02_md5), "%s", candidate->track02_md5);
    snprintf(receipt.source_trace_md5, sizeof(receipt.source_trace_md5), "%s", candidate->source_trace_md5);
    receipt.loader_pc = candidate->loader_pc;
    receipt.record = candidate->record;
    receipt.raw_sector = candidate->raw_sector;
    receipt.destination_identity = candidate->destination_identity;
    *out = receipt;
    return 1;
}
