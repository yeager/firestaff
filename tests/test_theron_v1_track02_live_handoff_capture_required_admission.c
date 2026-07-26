#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "theron_v1_track02_live_handoff_capture_required_admission.h"

static int write_sidecar(const char *path)
{
    FILE *file = fopen(path, "wb");
    if (!file) return 0;
    if (fputs("g8_fifo_output_capture generation=8 source_lba=4859 dispatch_sequence=4\n", file) == EOF ||
        fputs("pce_cd_fifo_origin_main_ram_receipt generation=8 source_lba=4859 source_offset=0 fifo_sequence=1 reader_pc=ea50 logical_destination=2200 physical_destination=1f0200 writer_pc=ea52 writer_physical_pc=1f0252 value=ab\n", file) == EOF ||
        fclose(file) != 0) return 0;
    return 1;
}

static void make_corpus(Theron_V1Track02HandoffArtifactCorpusReceipt *corpus)
{
    size_t i;
    memset(corpus, 0, sizeof(*corpus));
    corpus->status = THERON_V1_TRACK02_HANDOFF_ARTIFACT_CORPUS_READY;
    corpus->supplied_candidate_count = corpus->direct_candidate_count = 1u;
    corpus->direct_cue_bin_consumed = corpus->source_trace_md5_verified = 1;
    corpus->capture_target_plan_consumed = corpus->opaque_artifact_consumed = 1;
    corpus->capture_required_only = corpus->no_draw_only = 1;
    corpus->capture_target_plan_identity = 0x1234u;
    snprintf(corpus->track02_md5, sizeof(corpus->track02_md5), "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
    snprintf(corpus->source_trace_md5, sizeof(corpus->source_trace_md5), "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb");
    corpus->artifact.status = THERON_V1_TRACK02_CAPTURE_ARTIFACT_READY;
    corpus->artifact.bundle_md5_verified = corpus->artifact.mednafen_trace_md5_verified = 1;
    corpus->artifact.complete_route_set_consumed = corpus->artifact.opaque_envelope_verified = 1;
    corpus->artifact.opaque_runtime_ready = 1;
    corpus->artifact.track02_variant = THERON_TRACK02_VARIANT_US_BIN;
    corpus->artifact.campaign_route = THERON_V1_TRACK02_CAPTURE_TARGET_DUNGEON_HANDOFF;
    corpus->artifact.descriptor_selector = 1u;
    corpus->artifact.descriptor_source_hash = 2u;
    corpus->artifact.capture_target_plan_identity = corpus->capture_target_plan_identity;
    snprintf(corpus->artifact.track02_md5, sizeof(corpus->artifact.track02_md5), "%s", corpus->track02_md5);
    snprintf(corpus->artifact.bundle_md5, sizeof(corpus->artifact.bundle_md5), "cccccccccccccccccccccccccccccccc");
    snprintf(corpus->artifact.mednafen_trace_md5, sizeof(corpus->artifact.mednafen_trace_md5), "%s", corpus->source_trace_md5);
    for (i = 0; i < THERON_V1_TRACK02_CAPTURE_TARGET_COUNT; ++i) {
        corpus->artifact.cd_read_record[i] = (uint32_t)(i + 1u);
        corpus->artifact.loader_output_bytes[i] = i + 1u;
        corpus->artifact.loader_output_identity[i] = (uint32_t)(i + 2u);
        corpus->artifact.palette_output_identity[i] = (uint32_t)(i + 3u);
        corpus->artifact.bitmap_transfer_identity[i] = (uint32_t)(i + 4u);
        corpus->artifact.destination_record[i] = (uint32_t)(i + 5u);
        corpus->artifact.destination_bytes[i] = i + 6u;
        corpus->artifact.destination_identity[i] = (uint32_t)(i + 7u);
    }
}

static void make_receipts(Theron_V1Track02LoaderOutputRecordAdmissionReceipt *loader,
                          Theron_V1Track02LiveLoaderRouteAdmissionReceipt *live,
                          Theron_V1Track02DynamicCdReadOwnershipReceipt *ownership,
                          const Theron_V1Track02HandoffArtifactCorpusReceipt *corpus)
{
    memset(loader, 0, sizeof(*loader));
    loader->valid = loader->original_loader_output_consumed = 1;
    loader->envelope_header_fields_proven = loader->level_boundary_proven = 1;
    loader->object_continuation_boundary_proven = loader->no_draw_only = 1;
    loader->track02_variant = THERON_TRACK02_VARIANT_US_BIN;
    snprintf(loader->track02_md5, sizeof(loader->track02_md5), "%s", corpus->track02_md5);
    loader->record = 0x0b52u; loader->destination = 0x3800u;
    loader->loader_output_bytes = 0x100u; loader->loader_output_checksum = 1u;
    loader->level_bytes = 12u; loader->level_checksum = 2u;
    loader->object_continuation_bytes = 8u; loader->object_continuation_checksum = 3u;

    memset(live, 0, sizeof(*live));
    live->valid = live->dynamic_cd_read_ownership_consumed = 1;
    live->huc6280_event_log_consumed = live->manifest_bound = 1;
    live->opaque_runtime_route_ready = 1;
    live->track02_variant = THERON_TRACK02_VARIANT_US_BIN;
    snprintf(live->track02_md5, sizeof(live->track02_md5), "%s", corpus->track02_md5);
    snprintf(live->source_trace_md5, sizeof(live->source_trace_md5), "%s", corpus->source_trace_md5);
    snprintf(live->huc6280_event_log_md5, sizeof(live->huc6280_event_log_md5), "dddddddddddddddddddddddddddddddd");
    live->loader_record = 0x500u; live->loader_destination = 0x2200u;
    live->consumer_trace_checksum = 4u;

    memset(ownership, 0, sizeof(*ownership));
    ownership->valid = ownership->raw_cue_bin_identity_consumed = 1;
    ownership->loader_trace_consumed = ownership->register_record_normalized = 1;
    ownership->raw_sector_window_owned = 1;
    ownership->track02_variant = THERON_TRACK02_VARIANT_US_BIN;
    snprintf(ownership->track02_md5, sizeof(ownership->track02_md5), "%s", corpus->track02_md5);
    ownership->track02_record = 0x500u; ownership->destination = 0x2200u;
    ownership->raw_sector = 1u; ownership->user_data_bytes = 2048u;
    ownership->destination_span_checksum = 5u; ownership->full_payload_checksum = 6u;
}

int main(void)
{
    char path[] = "/tmp/firestaff-theron-live-handoff-XXXXXX";
    Theron_V1Track02HandoffArtifactCorpusReceipt corpus;
    Theron_V1Track02G8FifoSidecarReceipt sidecar;
    Theron_V1Track02G8FifoCaptureBindingReceipt g8;
    Theron_V1Track02LoaderOutputRecordAdmissionReceipt loader;
    Theron_V1Track02LiveLoaderRouteAdmissionReceipt live;
    Theron_V1Track02DynamicCdReadOwnershipReceipt ownership;
    Theron_V1Track02LiveHandoffCaptureRequiredAdmissionReceipt receipt;
    int descriptor = mkstemp(path);

    if (descriptor < 0) return 1;
    close(descriptor);
    if (!write_sidecar(path) || !theron_v1_track02_g8_fifo_sidecar_parse_file(path, &sidecar)) return 2;
    make_corpus(&corpus);
    make_receipts(&loader, &live, &ownership, &corpus);
    if (!theron_v1_track02_g8_fifo_capture_binding_bind(&sidecar, &corpus,
            corpus.source_trace_md5, 9u, &g8) ||
        !theron_v1_track02_live_handoff_capture_required_admit(
            &loader, &g8, &live, &ownership, &corpus, 9u, &receipt) ||
        !receipt.valid || !receipt.capture_required_only || !receipt.no_draw_only ||
        receipt.loader_output_record != loader.record ||
        receipt.live_handoff_record != live.loader_record ||
        receipt.g8_lba != 4859u || receipt.bitmap_palette_admission_allowed ||
        receipt.level_object_semantics_allowed || receipt.dungeon_draw_allowed) return 3;

    ++ownership.destination;
    if (theron_v1_track02_live_handoff_capture_required_admit(
            &loader, &g8, &live, &ownership, &corpus, 9u, &receipt) || receipt.valid) return 4;
    --ownership.destination;
    if (theron_v1_track02_live_handoff_capture_required_admit(
            &loader, &g8, &live, &ownership, &corpus, 10u, &receipt) || receipt.valid) return 5;
    live.bitmap_palette_admission_allowed = 1;
    if (theron_v1_track02_live_handoff_capture_required_admit(
            &loader, &g8, &live, &ownership, &corpus, 9u, &receipt) || receipt.valid) return 6;
    live.bitmap_palette_admission_allowed = 0;
    loader.object_semantics_allowed = 1;
    if (theron_v1_track02_live_handoff_capture_required_admit(
            &loader, &g8, &live, &ownership, &corpus, 9u, &receipt) || receipt.valid) return 7;
    loader.object_semantics_allowed = 0;
    live.source_trace_md5[0] = 'd';
    if (theron_v1_track02_live_handoff_capture_required_admit(
            &loader, &g8, &live, &ownership, &corpus, 9u, &receipt) || receipt.valid) return 8;
    remove(path);
    puts("test_theron_v1_track02_live_handoff_capture_required_admission: PASS");
    return 0;
}
