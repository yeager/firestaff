#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "m11_game_view.h"
#include "theron_v1_track02_g8_fifo_capture_binding.h"

static int failures;

static void expect(int condition, const char* message)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        ++failures;
    }
}

static int write_sidecar(const char* path)
{
    FILE* file = fopen(path, "wb");
    if (!file) return 0;
    if (fputs("g8_fifo_output_capture generation=8 source_lba=4859 dispatch_sequence=4\n",
              file) == EOF ||
        fputs("pce_cd_fifo_origin_main_ram_receipt generation=8 source_lba=4859 "
              "source_offset=0 fifo_sequence=1 reader_pc=ea50 logical_destination=2200 "
              "physical_destination=1f0200 writer_pc=ea52 writer_physical_pc=1f0252 "
              "value=ab\n", file) == EOF || fclose(file) != 0) {
        return 0;
    }
    return 1;
}

static void make_corpus(Theron_V1Track02HandoffArtifactCorpusReceipt* corpus)
{
    size_t i;
    memset(corpus, 0, sizeof(*corpus));
    corpus->status = THERON_V1_TRACK02_HANDOFF_ARTIFACT_CORPUS_READY;
    corpus->supplied_candidate_count = corpus->direct_candidate_count = 1u;
    corpus->direct_cue_bin_consumed = corpus->source_trace_md5_verified = 1;
    corpus->capture_target_plan_consumed = corpus->opaque_artifact_consumed = 1;
    corpus->capture_required_only = corpus->no_draw_only = 1;
    corpus->capture_target_plan_identity = 0x1234u;
    snprintf(corpus->track02_md5, sizeof(corpus->track02_md5),
             "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
    snprintf(corpus->source_trace_md5, sizeof(corpus->source_trace_md5),
             "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb");
    corpus->artifact.status = THERON_V1_TRACK02_CAPTURE_ARTIFACT_READY;
    corpus->artifact.bundle_md5_verified = corpus->artifact.mednafen_trace_md5_verified = 1;
    corpus->artifact.complete_route_set_consumed = corpus->artifact.opaque_envelope_verified = 1;
    corpus->artifact.opaque_runtime_ready = 1;
    corpus->artifact.track02_variant = THERON_TRACK02_VARIANT_US_BIN;
    corpus->artifact.campaign_route = THERON_V1_TRACK02_CAPTURE_TARGET_DUNGEON_HANDOFF;
    corpus->artifact.descriptor_selector = 1u;
    corpus->artifact.descriptor_source_hash = 2u;
    corpus->artifact.capture_target_plan_identity = corpus->capture_target_plan_identity;
    snprintf(corpus->artifact.track02_md5, sizeof(corpus->artifact.track02_md5), "%s",
             corpus->track02_md5);
    snprintf(corpus->artifact.bundle_md5, sizeof(corpus->artifact.bundle_md5),
             "cccccccccccccccccccccccccccccccc");
    snprintf(corpus->artifact.mednafen_trace_md5,
             sizeof(corpus->artifact.mednafen_trace_md5), "%s",
             corpus->source_trace_md5);
    for (i = 0u; i < THERON_V1_TRACK02_CAPTURE_TARGET_COUNT; ++i) {
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

int main(void)
{
    char path[] = "/tmp/firestaff-theron-g8-binding-XXXXXX";
    Theron_V1Track02G8FifoSidecarReceipt sidecar;
    Theron_V1Track02G8FifoCaptureBindingReceipt binding;
    Theron_V1Track02HandoffArtifactCorpusReceipt corpus;
    M11_GameViewState state;
    int descriptor = mkstemp(path);

    expect(descriptor >= 0, "temporary sidecar allocates");
    if (descriptor < 0) return 1;
    close(descriptor);
    expect(write_sidecar(path), "valid immutable G8 sidecar writes");
    expect(theron_v1_track02_g8_fifo_sidecar_parse_file(path, &sidecar) &&
               sidecar.valid,
           "valid G8 sidecar parses before binding");
    make_corpus(&corpus);

    expect(theron_v1_track02_g8_fifo_capture_binding_bind(
               &sidecar, &corpus, corpus.source_trace_md5, 9u, &binding) &&
               binding.status == THERON_V1_TRACK02_G8_FIFO_CAPTURE_BINDING_CAPTURE_REQUIRED &&
               binding.capture_required_only && binding.no_draw_only &&
               binding.fingerprint == sidecar.fingerprint &&
               binding.source_offset == sidecar.source_offset &&
               binding.first_fifo_sequence == sidecar.first_fifo_sequence &&
               binding.last_fifo_sequence == sidecar.last_fifo_sequence &&
               binding.capture_byte_count == sidecar.capture_byte_count &&
               binding.source_window_offset == sidecar.source_window_offset &&
               binding.source_window_bytes == sidecar.source_window_bytes &&
               binding.sequence_window_identity == sidecar.sequence_window_identity &&
               binding.capture_row_count == sidecar.capture_row_count &&
               binding.capture_file_fnv1a == sidecar.capture_file_fnv1a &&
               binding.capture_file_identity == sidecar.capture_file_identity &&
               binding.dispatch_logical_pc == sidecar.dispatch_logical_pc &&
               binding.dispatch_physical_pc == sidecar.dispatch_physical_pc &&
               binding.dispatch_a == sidecar.dispatch_a &&
               binding.dispatch_x == sidecar.dispatch_x &&
               binding.dispatch_y == sidecar.dispatch_y &&
               binding.cdb_opcode == sidecar.cdb_opcode &&
               binding.cdb_lba == sidecar.cdb_lba &&
               binding.cdb_sector_count == sidecar.cdb_sector_count &&
               !memcmp(binding.cdb, sidecar.cdb, sizeof(binding.cdb)) &&
               binding.capture_cdb_identity == sidecar.capture_cdb_identity &&
               !strcmp(binding.capture_file_md5, sidecar.capture_file_md5) &&
               binding.reader_pc == sidecar.reader_pc &&
               binding.logical_destination == sidecar.logical_destination &&
               binding.physical_destination == sidecar.physical_destination &&
               binding.writer_pc == sidecar.writer_pc &&
               binding.writer_physical_pc == sidecar.writer_physical_pc &&
               binding.value == sidecar.value &&
               binding.capture_target_plan_identity == corpus.capture_target_plan_identity &&
               !strcmp(binding.artifact_bundle_md5, corpus.artifact.bundle_md5) &&
               !strcmp(binding.sidecar_trace_md5, sidecar.trace_md5) &&
               theron_v1_track02_g8_fifo_capture_binding_matches_lifecycle(
                   &binding, &corpus, corpus.source_trace_md5, 9u),
           "sidecar binds only to the matching opaque corpus trace lifecycle");
    expect(!theron_v1_track02_g8_fifo_capture_binding_matches_lifecycle(
               &binding, &corpus, "dddddddddddddddddddddddddddddddd", 9u),
           "source-trace drift rejects the binding");
    expect(!theron_v1_track02_g8_fifo_capture_binding_matches_lifecycle(
               &binding, &corpus, corpus.source_trace_md5, 10u),
           "lifecycle epoch drift rejects the binding");
    ++sidecar.source_window_bytes;
    expect(theron_v1_track02_g8_fifo_capture_binding_bind(
               &sidecar, &corpus, corpus.source_trace_md5, 9u, &binding) &&
               binding.status == THERON_V1_TRACK02_G8_FIFO_CAPTURE_BINDING_REJECTED,
           "sidecar sequence/window length drift rejects before binding");
    --sidecar.source_window_bytes;
    expect(theron_v1_track02_g8_fifo_capture_binding_bind(
               &sidecar, &corpus, corpus.source_trace_md5, 9u, &binding) &&
               binding.status == THERON_V1_TRACK02_G8_FIFO_CAPTURE_BINDING_CAPTURE_REQUIRED,
           "exact one-row sequence/window identity restores capture-required binding");
    ++binding.writer_pc;
    expect(!theron_v1_track02_g8_fifo_capture_binding_matches_lifecycle(
               &binding, &corpus, corpus.source_trace_md5, 9u),
           "captured writer-PC drift rejects the binding fingerprint");
    --binding.writer_pc;
    ++binding.source_window_bytes;
    expect(!theron_v1_track02_g8_fifo_capture_binding_matches_lifecycle(
               &binding, &corpus, corpus.source_trace_md5, 9u),
           "captured sequence/window length drift rejects the binding identity");
    --binding.source_window_bytes;
    ++binding.capture_file_fnv1a;
    expect(!theron_v1_track02_g8_fifo_capture_binding_matches_lifecycle(
               &binding, &corpus, corpus.source_trace_md5, 9u),
           "capture-file FNV drift rejects the lifecycle binding");
    --binding.capture_file_fnv1a;
    ++binding.capture_row_count;
    expect(!theron_v1_track02_g8_fifo_capture_binding_matches_lifecycle(
               &binding, &corpus, corpus.source_trace_md5, 9u),
           "capture-file row-count drift rejects the lifecycle binding");
    --binding.capture_row_count;
    ++binding.cdb_lba;
    expect(!theron_v1_track02_g8_fifo_capture_binding_matches_lifecycle(
               &binding, &corpus, corpus.source_trace_md5, 9u),
           "G8 READ(6) LBA drift rejects the capture-required lifecycle binding");
    --binding.cdb_lba;
    ++binding.dispatch_logical_pc;
    expect(!theron_v1_track02_g8_fifo_capture_binding_matches_lifecycle(
               &binding, &corpus, corpus.source_trace_md5, 9u),
           "G8 dispatch callsite drift rejects the capture-required lifecycle binding");
    --binding.dispatch_logical_pc;

    memset(&state, 0, sizeof(state));
    state.sourceKind = M11_GAME_SOURCE_THERON_TRACK02;
    state.theronState.campaign_media_scan_epoch = 9u;
    state.theronState.launch_trace_identity_bound = 1;
    state.theronState.launch_trace_identity.valid = 1;
    snprintf(state.theronState.launch_trace_identity.source_trace_md5,
             sizeof(state.theronState.launch_trace_identity.source_trace_md5), "%s",
             corpus.source_trace_md5);
    state.theronState.handoff_artifact_corpus = corpus;
    state.theronState.handoff_artifact_corpus_bound = 1;
    state.theronState.handoff_artifact_corpus_scan_epoch = 9u;
    expect(M11_GameView_TheronBindTrack02G8FifoSidecarCaptureRequired(
               &state, &sidecar, &corpus, 9u) &&
               state.theronState.g8_fifo_capture_binding_bound &&
               state.theronState.g8_fifo_capture_binding.capture_required_only &&
               state.theronState.g8_fifo_capture_binding.no_draw_only &&
               state.theronState.g8_fifo_capture_binding.capture_row_count == 1u &&
               state.theronState.g8_fifo_capture_binding.capture_file_fnv1a ==
                   sidecar.capture_file_fnv1a &&
               state.theronState.g8_fifo_capture_binding.capture_file_identity ==
                   sidecar.capture_file_identity &&
               state.theronState.g8_fifo_capture_binding.capture_cdb_identity ==
                   sidecar.capture_cdb_identity &&
               !strcmp(state.theronState.g8_fifo_capture_binding.artifact_bundle_md5,
                       corpus.artifact.bundle_md5) &&
               state.theronState.g8_fifo_capture_binding.fingerprint == sidecar.fingerprint,
           "M11 binds the immutable sidecar only as no-draw lifecycle evidence");
    ++sidecar.cdb_lba;
    expect(!M11_GameView_TheronBindTrack02G8FifoSidecarCaptureRequired(
               &state, &sidecar, &corpus, 9u) &&
               !state.theronState.g8_fifo_capture_binding_bound,
           "M11 clears capture-required evidence on G8 READ(6) CDB drift");
    --sidecar.cdb_lba;
    expect(M11_GameView_TheronBindTrack02G8FifoSidecarCaptureRequired(
               &state, &sidecar, &corpus, 9u),
           "M11 restores only the exact canonical capture-file CDB identity");
    corpus.artifact.bundle_md5[0] = 'd';
    expect(!M11_GameView_TheronBindTrack02G8FifoSidecarCaptureRequired(
               &state, &sidecar, &corpus, 9u) &&
               !state.theronState.g8_fifo_capture_binding_bound,
           "M11 rejects a sidecar bind from an artifact corpus that differs from its lifecycle copy");
    corpus.artifact.bundle_md5[0] = 'c';
    expect(M11_GameView_TheronBindTrack02G8FifoSidecarCaptureRequired(
               &state, &sidecar, &corpus, 9u),
           "M11 restores only the exact lifecycle-bound artifact corpus");
    state.theronState.launch_trace_identity.source_trace_md5[0] = 'd';
    expect(!M11_GameView_TheronBindTrack02G8FifoSidecarCaptureRequired(
               &state, &sidecar, &corpus, 9u) &&
               !state.theronState.g8_fifo_capture_binding_bound,
           "M11 clears the capture-only binding on trace drift");

    remove(path);
    if (failures) return 1;
    puts("test_theron_v1_track02_g8_fifo_capture_binding: PASS");
    return 0;
}
