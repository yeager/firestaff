#include "theron_v1_track02_live_handoff_capture_required_admission.h"

#include <stdio.h>
#include <string.h>

static int loader_output_is_no_draw(
    const Theron_V1Track02LoaderOutputRecordAdmissionReceipt *receipt)
{
    return receipt && receipt->valid && receipt->original_loader_output_consumed &&
        receipt->envelope_header_fields_proven && receipt->level_boundary_proven &&
        !receipt->bitmap_boundary_proven && receipt->object_continuation_boundary_proven &&
        receipt->no_draw_only && receipt->track02_variant != THERON_TRACK02_VARIANT_UNKNOWN &&
        receipt->track02_md5[0] && receipt->record && receipt->destination &&
        receipt->loader_output_bytes && receipt->loader_output_checksum &&
        receipt->level_bytes && receipt->level_checksum &&
        receipt->object_continuation_bytes && receipt->object_continuation_checksum &&
        !receipt->level_semantics_allowed &&
        !receipt->header_level_identifier_semantics_allowed &&
        !receipt->object_semantics_allowed && !receipt->bitmap_semantics_allowed &&
        !receipt->pixel_decode_allowed && !receipt->render_allowed &&
        !receipt->fallback_visuals_allowed;
}

static int ownership_is_no_draw(const Theron_V1Track02DynamicCdReadOwnershipReceipt *receipt)
{
    return receipt && receipt->valid && receipt->raw_cue_bin_identity_consumed &&
        receipt->loader_trace_consumed && receipt->register_record_normalized &&
        receipt->raw_sector_window_owned &&
        receipt->track02_variant != THERON_TRACK02_VARIANT_UNKNOWN &&
        receipt->track02_md5[0] && receipt->track02_record && receipt->destination &&
        receipt->raw_sector && receipt->user_data_bytes == 2048u &&
        receipt->destination_span_checksum && receipt->full_payload_checksum &&
        !receipt->level_object_semantics_allowed &&
        !receipt->bitmap_palette_admission_allowed && !receipt->pixel_decode_allowed &&
        !receipt->dungeon_draw_allowed && !receipt->fallback_visuals_allowed;
}

static int live_handoff_is_no_draw(const Theron_V1Track02LiveLoaderRouteAdmissionReceipt *receipt)
{
    return receipt && receipt->valid && receipt->dynamic_cd_read_ownership_consumed &&
        receipt->huc6280_event_log_consumed && receipt->manifest_bound &&
        receipt->opaque_runtime_route_ready &&
        receipt->track02_variant != THERON_TRACK02_VARIANT_UNKNOWN &&
        receipt->track02_md5[0] && receipt->source_trace_md5[0] &&
        receipt->huc6280_event_log_md5[0] && receipt->loader_record &&
        receipt->loader_destination && receipt->consumer_trace_checksum &&
        !receipt->level_object_semantics_allowed &&
        !receipt->bitmap_palette_admission_allowed && !receipt->pixel_decode_allowed &&
        !receipt->dungeon_draw_allowed && !receipt->fallback_visuals_allowed;
}

int theron_v1_track02_live_handoff_capture_required_admit(
    const Theron_V1Track02LoaderOutputRecordAdmissionReceipt *loader_output,
    const Theron_V1Track02G8FifoCaptureBindingReceipt *g8_binding,
    const Theron_V1Track02LiveLoaderRouteAdmissionReceipt *live_handoff,
    const Theron_V1Track02DynamicCdReadOwnershipReceipt *ownership,
    const Theron_V1Track02HandoffArtifactCorpusReceipt *artifact_corpus,
    uint32_t lifecycle_scan_epoch,
    Theron_V1Track02LiveHandoffCaptureRequiredAdmissionReceipt *out)
{
    Theron_V1Track02LiveHandoffCaptureRequiredAdmissionReceipt receipt = {0};

    if (!out) return 0;
    *out = receipt;
    if (!lifecycle_scan_epoch || !loader_output_is_no_draw(loader_output) ||
        !ownership_is_no_draw(ownership) || !live_handoff_is_no_draw(live_handoff) ||
        !artifact_corpus ||
        !theron_v1_track02_g8_fifo_capture_binding_matches_lifecycle(
            g8_binding, artifact_corpus, live_handoff->source_trace_md5,
            lifecycle_scan_epoch) ||
        !theron_v1_track02_handoff_artifact_corpus_matches_identity(
            artifact_corpus, live_handoff->track02_md5,
            live_handoff->source_trace_md5,
            g8_binding->capture_target_plan_identity) ||
        loader_output->track02_variant != live_handoff->track02_variant ||
        loader_output->track02_variant != ownership->track02_variant ||
        strcmp(loader_output->track02_md5, live_handoff->track02_md5) ||
        strcmp(loader_output->track02_md5, ownership->track02_md5) ||
        ownership->track02_record != live_handoff->loader_record ||
        ownership->destination != live_handoff->loader_destination ||
        g8_binding->lifecycle_scan_epoch != lifecycle_scan_epoch ||
        strcmp(g8_binding->source_trace_md5, live_handoff->source_trace_md5) ||
        g8_binding->capture_target_plan_identity !=
            artifact_corpus->capture_target_plan_identity) {
        return 0;
    }

    receipt.valid = receipt.loader_output_record_consumed = 1;
    receipt.g8_fifo_capture_consumed = receipt.live_handoff_consumed = 1;
    receipt.dynamic_cd_read_ownership_consumed = 1;
    receipt.capture_required_only = receipt.no_draw_only = 1;
    receipt.lifecycle_scan_epoch = lifecycle_scan_epoch;
    receipt.capture_target_plan_identity = g8_binding->capture_target_plan_identity;
    receipt.track02_variant = loader_output->track02_variant;
    snprintf(receipt.track02_md5, sizeof(receipt.track02_md5), "%s",
             loader_output->track02_md5);
    snprintf(receipt.source_trace_md5, sizeof(receipt.source_trace_md5), "%s",
             live_handoff->source_trace_md5);
    receipt.loader_output_record = loader_output->record;
    receipt.loader_output_destination = loader_output->destination;
    receipt.live_handoff_record = live_handoff->loader_record;
    receipt.live_handoff_destination = live_handoff->loader_destination;
    receipt.g8_lba = g8_binding->lba;
    receipt.g8_capture_file_identity = g8_binding->capture_file_identity;
    *out = receipt;
    return 1;
}
