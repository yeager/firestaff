#include <stdio.h>
#include <string.h>

#include "theron_v1_track02_later_route_candidate_plan_corpus_admission.h"

static void fixture(Theron_V1Track02LaterRouteCandidateCampaignIndex *index,
                    Theron_V1Track02CaptureTargetPlan *plan,
                    Theron_V1Track02HandoffArtifactCorpusReceipt *corpus,
                    Theron_V1Track02LoaderTraceReplayConsistencyReceipt *replay,
                    Theron_V1Track02LaunchTraceIdentityReceipt *trace)
{
    size_t i;
    const char *media_md5 = "f23601102138f87c33025877767ebf76";
    const char *trace_md5 = "11111111111111111111111111111111";
    memset(index, 0, sizeof(*index)); memset(plan, 0, sizeof(*plan));
    memset(corpus, 0, sizeof(*corpus)); memset(replay, 0, sizeof(*replay));
    memset(trace, 0, sizeof(*trace));
    plan->valid = plan->cue_track_consumed = plan->cd_read_chain_consumed = 1;
    plan->loader_output_consumed = plan->palette_output_consumed = 1;
    plan->bitmap_transfer_consumed = plan->destination_record_consumed = 1;
    for (i = 0; i < THERON_V1_TRACK02_CAPTURE_TARGET_COUNT; ++i) {
        Theron_V1Track02CaptureTarget *t = &plan->targets[i];
        t->route = (Theron_V1Track02CaptureTargetRoute)i;
        t->track02_variant = THERON_TRACK02_VARIANT_US_BIN;
        snprintf(t->track02_md5, sizeof(t->track02_md5), "%s", media_md5);
        t->cd_read_record = 0x510u; t->loader_output_raw_offset = 0x100u + i;
        t->loader_output_bytes = 0x20u; t->loader_output_checksum = 0x1000u + i;
        t->palette_output_identity = 0x2000u + i; t->bitmap_transfer_capture_required = 1;
        t->bitmap_raw_offset = 0x300u + i; t->bitmap_bytes = 0x40u;
        t->bitmap_identity = 0x3000u + i; t->destination_record = 0x510u;
        t->destination_offset = 0x400u + i; t->destination_bytes = 0x80u;
        t->destination_identity = 0x4000u + i;
    }
    corpus->status = THERON_V1_TRACK02_HANDOFF_ARTIFACT_CORPUS_READY;
    corpus->supplied_candidate_count = corpus->direct_candidate_count = 1u;
    corpus->direct_cue_bin_consumed = corpus->source_trace_md5_verified = 1;
    corpus->capture_target_plan_consumed = corpus->opaque_artifact_consumed = 1;
    corpus->capture_required_only = corpus->no_draw_only = 1;
    corpus->capture_target_plan_identity = theron_v1_track02_capture_target_plan_identity(plan);
    snprintf(corpus->track02_md5, sizeof(corpus->track02_md5), "%s", media_md5);
    snprintf(corpus->source_trace_md5, sizeof(corpus->source_trace_md5), "%s", trace_md5);
    corpus->artifact.status = THERON_V1_TRACK02_CAPTURE_ARTIFACT_READY;
    corpus->artifact.bundle_md5_verified = corpus->artifact.mednafen_trace_md5_verified = 1;
    corpus->artifact.complete_route_set_consumed = corpus->artifact.opaque_envelope_verified = 1;
    corpus->artifact.opaque_runtime_ready = 1; corpus->artifact.track02_variant = THERON_TRACK02_VARIANT_US_BIN;
    corpus->artifact.campaign_route = THERON_V1_TRACK02_CAPTURE_TARGET_DUNGEON_HANDOFF;
    corpus->artifact.descriptor_selector = corpus->artifact.descriptor_source_hash = 1u;
    corpus->artifact.capture_target_plan_identity = corpus->capture_target_plan_identity;
    snprintf(corpus->artifact.track02_md5, sizeof(corpus->artifact.track02_md5), "%s", media_md5);
    snprintf(corpus->artifact.bundle_md5, sizeof(corpus->artifact.bundle_md5), "22222222222222222222222222222222");
    snprintf(corpus->artifact.mednafen_trace_md5, sizeof(corpus->artifact.mednafen_trace_md5), "%s", trace_md5);
    for (i = 0; i < THERON_V1_TRACK02_CAPTURE_TARGET_COUNT; ++i) {
        Theron_V1Track02CaptureTarget *t = &plan->targets[i];
        corpus->artifact.cd_read_record[i] = t->cd_read_record;
        corpus->artifact.loader_output_raw_offset[i] = t->loader_output_raw_offset;
        corpus->artifact.loader_output_bytes[i] = t->loader_output_bytes;
        corpus->artifact.loader_output_identity[i] = t->loader_output_checksum;
        corpus->artifact.palette_output_identity[i] = t->palette_output_identity;
        corpus->artifact.bitmap_transfer_identity[i] = t->bitmap_identity;
        corpus->artifact.destination_record[i] = t->destination_record;
        corpus->artifact.destination_offset[i] = t->destination_offset;
        corpus->artifact.destination_bytes[i] = t->destination_bytes;
        corpus->artifact.destination_identity[i] = t->destination_identity;
    }
    replay->active = replay->direct_campaign_layout_consumed = replay->dynamic_cd_read_records_consumed = 1;
    replay->track02_variant = THERON_TRACK02_VARIANT_US_BIN; replay->campaign_layout_epoch = 7u;
    replay->accepted_record_count = 1u; replay->last_track02_record = 0x510u; replay->last_raw_sector = 0x510u;
    snprintf(replay->track02_md5, sizeof(replay->track02_md5), "%s", media_md5);
    trace->valid = trace->direct_campaign_consumed = trace->loader_trace_consumed = trace->event_log_consumed = 1;
    trace->track02_variant = THERON_TRACK02_VARIANT_US_BIN; trace->campaign_layout_epoch = 7u;
    snprintf(trace->track02_md5, sizeof(trace->track02_md5), "%s", media_md5);
    snprintf(trace->source_trace_md5, sizeof(trace->source_trace_md5), "%s", trace_md5);
    index->valid = index->capture_required_only = 1; index->count = 1u;
    index->entries[0].status = THERON_V1_TRACK02_LATER_ROUTE_CAPTURE_REQUIRED;
    index->entries[0].observed_trace_row_consumed = index->entries[0].direct_media_consumed = 1;
    index->entries[0].replay_tail_consumed = index->entries[0].opaque_only = 1;
    index->entries[0].loader_pc = 0x4010u; index->entries[0].record = index->entries[0].raw_sector = 0x510u;
    index->entries[0].destination_identity = plan->targets[THERON_V1_TRACK02_CAPTURE_TARGET_DUNGEON_HANDOFF].destination_identity;
    index->entries[0].campaign_layout_epoch = 7u;
    snprintf(index->entries[0].track02_md5, sizeof(index->entries[0].track02_md5), "%s", media_md5);
    snprintf(index->entries[0].source_trace_md5, sizeof(index->entries[0].source_trace_md5), "%s", trace_md5);
}

int main(void)
{
    Theron_V1Track02LaterRouteCandidateCampaignIndex index;
    Theron_V1Track02CaptureTargetPlan plan;
    Theron_V1Track02HandoffArtifactCorpusReceipt corpus;
    Theron_V1Track02LoaderTraceReplayConsistencyReceipt replay;
    Theron_V1Track02LaunchTraceIdentityReceipt trace;
    Theron_V1Track02LaterRouteCandidatePlanCorpusAdmissionReceipt receipt;
    fixture(&index, &plan, &corpus, &replay, &trace);
    if (!theron_v1_track02_later_route_candidate_plan_corpus_admit(&index, &plan, &corpus, &replay, &trace, 7u, 9u, &receipt) ||
        !receipt.valid || !receipt.capture_required_only || !receipt.no_draw_only || receipt.record != 0x510u ||
        receipt.level_object_semantics_allowed || receipt.bitmap_palette_admission_allowed || receipt.dungeon_draw_allowed) return 1;
    ++replay.last_track02_record;
    if (theron_v1_track02_later_route_candidate_plan_corpus_admit(&index, &plan, &corpus, &replay, &trace, 7u, 9u, &receipt) || receipt.valid) return 2;
    --replay.last_track02_record;
    ++index.entries[0].destination_identity;
    if (theron_v1_track02_later_route_candidate_plan_corpus_admit(&index, &plan, &corpus, &replay, &trace, 7u, 9u, &receipt) || receipt.valid) return 3;
    --index.entries[0].destination_identity;
    if (theron_v1_track02_later_route_candidate_plan_corpus_admit(&index, &plan, &corpus, &replay, &trace, 8u, 9u, &receipt) || receipt.valid) return 4;
    trace.source_trace_md5[0] = '3';
    if (theron_v1_track02_later_route_candidate_plan_corpus_admit(&index, &plan, &corpus, &replay, &trace, 7u, 9u, &receipt) || receipt.valid) return 5;
    puts("test_theron_v1_track02_later_route_candidate_plan_corpus_admission: PASS"); return 0;
}
