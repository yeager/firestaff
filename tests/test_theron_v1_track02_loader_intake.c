#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "theron_v1_track02_loader_intake.h"

static int failures;

typedef struct {
    int calls;
    uint32_t grid_hash;
    uint32_t grid_bytes;
    uint8_t first_byte;
    uint8_t last_byte;
    int accept;
} RuntimeConsumerCapture;

#define CHECK(condition) do { \
    if (!(condition)) { \
        fprintf(stderr, "failed: %s (%s:%d)\n", #condition, __FILE__, __LINE__); \
        ++failures; \
    } \
} while (0)

static Theron_V1Track02LoaderReadFacts valid_facts(void) {
    Theron_V1Track02LoaderReadFacts facts = {
        1, 1, THERON_V1_INITIAL_ENVELOPE_RECORD,
        THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET,
        0x3800u, 0x800u
    };
    return facts;
}

static unsigned char *read_real_track02(const char *path, size_t *out_bytes) {
    FILE *file;
    long file_bytes;
    unsigned char *bytes;

    if (!path || !out_bytes || !(file = fopen(path, "rb"))) return NULL;
    if (fseek(file, 0L, SEEK_END) != 0 ||
        (file_bytes = ftell(file)) <= 0 ||
        fseek(file, 0L, SEEK_SET) != 0) {
        fclose(file);
        return NULL;
    }
    bytes = malloc((size_t)file_bytes);
    if (!bytes || fread(bytes, 1u, (size_t)file_bytes, file) !=
        (size_t)file_bytes) {
        free(bytes);
        fclose(file);
        return NULL;
    }
    fclose(file);
    *out_bytes = (size_t)file_bytes;
    return bytes;
}

static int capture_runtime_grid(const Theron_V1Track02RawGridReceipt *grid,
                                void *context) {
    RuntimeConsumerCapture *capture = context;

    if (!grid || !capture) return 0;
    ++capture->calls;
    capture->grid_hash = grid->raw_grid_hash;
    capture->grid_bytes = grid->raw_grid_bytes;
    capture->first_byte = grid->raw_grid[0];
    capture->last_byte = grid->raw_grid[grid->raw_grid_bytes - 1u];
    return capture->accept;
}

int main(void) {
    Theron_V1Track02LoaderReadFacts facts = valid_facts();
    Theron_V1Track02LoaderReadFacts separate_object_facts = {
        1, 1, THERON_V1_INITIAL_ENVELOPE_RECORD + 1u, 0x114u, 0x5200u, 0x180u
    };
    Theron_V1Track02LoaderReadFacts separate_dungeon_facts = {
        1, 1, THERON_V1_INITIAL_ENVELOPE_RECORD + 2u, 0x114u, 0x6000u, 0x240u
    };
    Theron_V1RuntimeAdmissionReceipt runtime_admission = {1, 1};
    Theron_V1TraceProvenanceReceipt trace = {
        1, 1, "trace_accepted_runtime_admitted"
    };
    Theron_V1DungeonHandoffReceipt initial_envelope = {
        .selected = 1,
        .runtime_route_consumed = 1,
        .record = THERON_V1_INITIAL_ENVELOPE_RECORD,
        .record_user_data_offset =
            THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET,
        .envelope_bytes = THERON_V1_INITIAL_ENVELOPE_BYTES,
        .header_width = THERON_V1_INITIAL_ENVELOPE_HEADER_WIDTH,
        .header_height = THERON_V1_INITIAL_ENVELOPE_HEADER_HEIGHT,
        .header_seed = THERON_V1_INITIAL_ENVELOPE_HEADER_SEED,
        .header_identifier = THERON_V1_INITIAL_ENVELOPE_HEADER_IDENTIFIER,
        .cue_track02_index01_raw_sector = 225u,
        .track02_raw_sector = 3123u,
        .raw_sector_offset = 0x124u,
        .raw_track02_md5_verified = 1,
        .adjacent_boundary_opaque = 1,
        .route = "raw_track02_initial_envelope"
    };
    Theron_V1AuthenticatedTrack02LoaderReadFacts authenticated_facts = {
        &trace, 1, THERON_V1_INITIAL_ENVELOPE_RECORD,
        THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET, 0x3800u, 0x800u
    };
    Theron_V1Track02LoaderIntakeReceipt receipt;
    Theron_V1Track02LoaderIntakeReceipt decoded_receipt;
    Theron_V1Track02RawGridCoordinateReceipt coordinate_receipt;
    Theron_V1Track02RawGridRowReceipt row_receipt;
    Theron_V1Track02RawGridReceipt grid_receipt;
    Theron_V1Track02RawGridRuntimeReceipt runtime_receipt;
    Theron_V1Track02RawGridObjectTableProjectionReceipt object_projection;
    Theron_V1Track02ObjectTableReadBlockReceipt object_read_block;
    Theron_V1Track02RawGridBitmapRouteReceipt bitmap_route;
    Theron_V1Track02BitmapReadBlockReceipt bitmap_read_block;
    Theron_V1Track02RawGridLevelRouteReceipt level_route;
    Theron_V1Track02RawGridDungeonRouteReceipt dungeon_route;
    Theron_V1Track02RawGridDungeonRecordEvidenceReceipt dungeon_record;
    Theron_V1Track02DungeonReadBlockReceipt dungeon_read_block;
    Theron_V1Track02ObjectDungeonHandoffGateReceipt object_dungeon_gate;
    Theron_V1Track02RawCueAdmissionFacts raw_cue_facts;
    Theron_V1Track02RawCueAdmissionReceipt raw_cue_receipt;
    Theron_V1Track02LaterReadRawMediaGateReceipt raw_media_gate;
    Theron_V1Track02LaterReadCdRecordReceipt later_cd_record;
    Theron_V1Track02LaterReadCdRecordReceipt object_cd_record;
    Theron_V1Track02LaterReadCdRecordReceipt dungeon_cd_record;
    Theron_V1Track02ObjectDungeonLoaderReadTableReceipt read_table;
    Theron_V1Track02ObjectDungeonReadTableLayoutBindingReceipt read_layout_binding;
    Theron_V1Track02LaterReadLayoutReceipt later_layout;
    Theron_V1Track02LaterReadLayoutReceipt object_layout;
    Theron_V1Track02LaterReadLayoutReceipt dungeon_layout;
    Theron_V1Track02ObjectDungeonLayoutPairReceipt layout_pair;
    Theron_V1Track02ObjectDungeonReadLayoutPairBridgeReceipt read_layout_pair_bridge;
    Theron_V1Track02LaterReadLayoutBytesReceipt layout_bytes_receipt;
    Theron_V1Track02LaterReadLayoutBytesReceipt object_layout_bytes;
    Theron_V1Track02LaterReadLayoutBytesReceipt dungeon_layout_bytes;
    Theron_V1Track02ObjectDungeonLayoutBytesPairReceipt layout_bytes_pair;
    Theron_V1Track02ObjectDungeonReadToBytesBridgeReceipt read_to_bytes_bridge;
    Theron_V1Track02ObjectDungeonDecoderGateReceipt decoder_gate;
    Theron_V1Track02ObjectDungeonReadToDecoderGateReceipt read_to_decoder_gate;
    Theron_V1Track02ObjectDungeonPredecodeEvidenceReceipt predecode_evidence;
    Theron_V1Track02ObjectDungeonPostPredecodeGateReceipt post_predecode_gate;
    Theron_V1Track02ObjectDungeonLevelHandoffGateReceipt level_handoff_gate;
    Theron_V1Track02ObjectDungeonGrammarAdmissionGateReceipt grammar_gate;
    Theron_V1Track02ObjectDungeonGrammarReadEvidenceGateReceipt
        grammar_read_gate;
    Theron_V1Track02ObjectDungeonParserGrammarWitnessFacts parser_witness;
    Theron_V1Track02ObjectDungeonParserGrammarWitnessReceipt
        parser_witness_receipt;
    uint8_t layout_bytes[0x300u];
    const char *real_track02_path;
    const char *real_track02_md5 = NULL;
    unsigned char *real_track02;
    size_t real_track02_bytes;
    unsigned char *synthetic_raw;
    size_t synthetic_raw_bytes;

    memset(&raw_cue_receipt, 0, sizeof(raw_cue_receipt));
    CHECK(theron_v1_track02_loader_intake_observe(&facts, &receipt));
    CHECK(receipt.observed);
    CHECK(!receipt.authenticated_v3_trace);
    CHECK(!receipt.payload_intake_admitted);
    CHECK(receipt.record == THERON_V1_INITIAL_ENVELOPE_RECORD);
    CHECK(receipt.record_user_data_offset ==
          THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET);
    CHECK(receipt.observed_destination == facts.destination);
    CHECK(receipt.observed_byte_count == facts.byte_count);
    CHECK(strcmp(receipt.status,
                 "initial_envelope_loader_read_observed_payload_blocked") == 0);
    CHECK(theron_v1_track02_loader_intake_bind_initial_envelope(
        &receipt, &initial_envelope, &receipt));
    CHECK(receipt.initial_envelope_source_bound);
    CHECK(!receipt.authenticated_v3_trace);
    CHECK(!receipt.payload_intake_admitted);
    CHECK(strcmp(receipt.status,
                 "initial_envelope_loader_read_source_bound_payload_blocked") == 0);

    /* Success remains available only for an operator-supplied canonical BIN. */
    real_track02_path = getenv("FIRESTAFF_THERON_TRACK02_RAW");
    real_track02 = read_real_track02(real_track02_path, &real_track02_bytes);
    CHECK(theron_v1_track02_loader_intake_gate_later_read_raw_media(
        NULL, &raw_media_gate));
    CHECK(raw_media_gate.gate_evaluated);
    CHECK(!raw_media_gate.raw_media_bound);
    CHECK(raw_media_gate.raw_media_missing_blocked);
    CHECK(!raw_media_gate.raw_cue_admission_consumed);
    CHECK(raw_media_gate.canonical_raw_bin_required);
    CHECK(!raw_media_gate.canonical_raw_bin_present);
    CHECK(raw_media_gate.iso_image_blocked);
    CHECK(raw_media_gate.parser_semantics_blocked);
    CHECK(raw_media_gate.runtime_handoff_blocked);
    CHECK(raw_media_gate.rendering_blocked);
    CHECK(raw_media_gate.fallback_visuals_blocked);
    CHECK(raw_media_gate.no_synthetic_bytes);
    CHECK(raw_media_gate.raw_track02_variant ==
          THERON_V1_TRACK02_VARIANT_NONE);
    CHECK(raw_media_gate.raw_track02_bytes == 0u);
    CHECK(raw_media_gate.track02_md5 == NULL);
    CHECK(strcmp(raw_media_gate.status,
                 "later_loader_raw_track02_media_missing_handoff_blocked_no_fallback") == 0);
    CHECK(theron_v1_track02_loader_intake_bind_object_dungeon_loader_read_table(
        &raw_media_gate, NULL, NULL, &read_table));
    CHECK(!read_table.table_bound);
    CHECK(read_table.raw_media_gate_consumed);
    CHECK(!read_table.object_cd_record_consumed);
    CHECK(!read_table.dungeon_cd_record_consumed);
    CHECK(read_table.raw_media_missing_blocked);
    CHECK(read_table.parser_semantics_blocked);
    CHECK(read_table.dungeon_grammar_blocked);
    CHECK(read_table.runtime_handoff_blocked);
    CHECK(read_table.rendering_blocked);
    CHECK(read_table.fallback_visuals_blocked);
    CHECK(read_table.no_synthetic_handoff);
    CHECK(read_table.track02_md5 == NULL);
    CHECK(strcmp(read_table.status,
                 "object_dungeon_loader_read_table_blocked_missing_raw_media_no_fallback") == 0);
    CHECK(theron_v1_track02_loader_intake_bind_read_table_to_layouts(
        &read_table, NULL, NULL, &read_layout_binding));
    CHECK(!read_layout_binding.binding_bound);
    CHECK(read_layout_binding.read_table_consumed);
    CHECK(!read_layout_binding.object_layout_consumed);
    CHECK(!read_layout_binding.dungeon_layout_consumed);
    CHECK(read_layout_binding.raw_media_missing_blocked);
    CHECK(read_layout_binding.parser_semantics_blocked);
    CHECK(read_layout_binding.dungeon_grammar_blocked);
    CHECK(read_layout_binding.runtime_handoff_blocked);
    CHECK(read_layout_binding.rendering_blocked);
    CHECK(read_layout_binding.fallback_visuals_blocked);
    CHECK(read_layout_binding.no_synthetic_layout);
    CHECK(read_layout_binding.track02_md5 == NULL);
    CHECK(strcmp(read_layout_binding.status,
                 "object_dungeon_read_table_layout_binding_blocked_missing_raw_media_no_fallback") == 0);
    CHECK(theron_v1_track02_loader_intake_bridge_read_layout_binding_to_layout_pair(
        &read_layout_binding, NULL, &read_layout_pair_bridge));
    CHECK(!read_layout_pair_bridge.bridge_bound);
    CHECK(read_layout_pair_bridge.read_layout_binding_consumed);
    CHECK(!read_layout_pair_bridge.layout_pair_consumed);
    CHECK(read_layout_pair_bridge.raw_media_missing_blocked);
    CHECK(read_layout_pair_bridge.parser_semantics_blocked);
    CHECK(read_layout_pair_bridge.dungeon_grammar_blocked);
    CHECK(read_layout_pair_bridge.runtime_handoff_blocked);
    CHECK(read_layout_pair_bridge.rendering_blocked);
    CHECK(read_layout_pair_bridge.fallback_visuals_blocked);
    CHECK(read_layout_pair_bridge.no_synthetic_layout);
    CHECK(read_layout_pair_bridge.track02_md5 == NULL);
    CHECK(strcmp(read_layout_pair_bridge.status,
                 "object_dungeon_read_layout_pair_bridge_blocked_missing_raw_media_no_fallback") == 0);
    CHECK(theron_v1_track02_loader_intake_bridge_read_layout_pair_to_bytes(
        &read_layout_pair_bridge, NULL, &read_to_bytes_bridge));
    CHECK(!read_to_bytes_bridge.bridge_bound);
    CHECK(read_to_bytes_bridge.read_layout_pair_bridge_consumed);
    CHECK(!read_to_bytes_bridge.byte_pair_consumed);
    CHECK(read_to_bytes_bridge.raw_media_missing_blocked);
    CHECK(read_to_bytes_bridge.parser_semantics_blocked);
    CHECK(read_to_bytes_bridge.dungeon_grammar_blocked);
    CHECK(read_to_bytes_bridge.runtime_handoff_blocked);
    CHECK(read_to_bytes_bridge.rendering_blocked);
    CHECK(read_to_bytes_bridge.fallback_visuals_blocked);
    CHECK(read_to_bytes_bridge.no_synthetic_bytes);
    CHECK(read_to_bytes_bridge.track02_md5 == NULL);
    CHECK(strcmp(read_to_bytes_bridge.status,
                 "object_dungeon_read_to_bytes_bridge_blocked_missing_raw_media_no_fallback") == 0);
    CHECK(theron_v1_track02_loader_intake_bind_read_to_bytes_to_decoder_gate(
        &read_to_bytes_bridge, NULL, &read_to_decoder_gate));
    CHECK(!read_to_decoder_gate.gate_bound);
    CHECK(read_to_decoder_gate.read_to_bytes_bridge_consumed);
    CHECK(!read_to_decoder_gate.decoder_gate_consumed);
    CHECK(!read_to_decoder_gate.source_bytes_ready);
    CHECK(read_to_decoder_gate.raw_media_missing_blocked);
    CHECK(read_to_decoder_gate.decoder_semantics_blocked);
    CHECK(read_to_decoder_gate.dungeon_grammar_blocked);
    CHECK(read_to_decoder_gate.runtime_handoff_blocked);
    CHECK(read_to_decoder_gate.rendering_blocked);
    CHECK(read_to_decoder_gate.fallback_visuals_blocked);
    CHECK(read_to_decoder_gate.no_synthetic_bytes);
    CHECK(read_to_decoder_gate.track02_md5 == NULL);
    CHECK(strcmp(read_to_decoder_gate.status,
                 "object_dungeon_read_to_decoder_gate_blocked_missing_raw_media_no_fallback") == 0);
    CHECK(theron_v1_track02_loader_intake_gate_object_dungeon_post_predecode(
        &read_to_decoder_gate, NULL, &post_predecode_gate));
    CHECK(!post_predecode_gate.readiness_bound);
    CHECK(post_predecode_gate.read_to_decoder_gate_consumed);
    CHECK(!post_predecode_gate.predecode_evidence_consumed);
    CHECK(!post_predecode_gate.topology_ready);
    CHECK(post_predecode_gate.raw_media_missing_blocked);
    CHECK(post_predecode_gate.decoder_semantics_blocked);
    CHECK(post_predecode_gate.dungeon_grammar_blocked);
    CHECK(post_predecode_gate.runtime_handoff_blocked);
    CHECK(post_predecode_gate.rendering_blocked);
    CHECK(post_predecode_gate.fallback_visuals_blocked);
    CHECK(post_predecode_gate.no_synthetic_bytes);
    CHECK(post_predecode_gate.track02_md5 == NULL);
    CHECK(strcmp(post_predecode_gate.status,
                 "object_dungeon_post_predecode_blocked_missing_raw_media_no_fallback") == 0);
    CHECK(theron_v1_track02_loader_intake_gate_object_dungeon_level_handoff(
        &post_predecode_gate, NULL, &level_handoff_gate));
    CHECK(!level_handoff_gate.level_handoff_bound);
    CHECK(level_handoff_gate.post_predecode_gate_consumed);
    CHECK(!level_handoff_gate.initial_level_handoff_consumed);
    CHECK(level_handoff_gate.raw_media_missing_blocked);
    CHECK(level_handoff_gate.object_records_blocked);
    CHECK(level_handoff_gate.dungeon_records_blocked);
    CHECK(level_handoff_gate.decoder_semantics_blocked);
    CHECK(level_handoff_gate.dungeon_grammar_blocked);
    CHECK(level_handoff_gate.runtime_handoff_blocked);
    CHECK(level_handoff_gate.rendering_blocked);
    CHECK(level_handoff_gate.fallback_visuals_blocked);
    CHECK(level_handoff_gate.no_synthetic_bytes);
    CHECK(level_handoff_gate.track02_md5 == NULL);
    CHECK(strcmp(level_handoff_gate.status,
                 "object_dungeon_level_handoff_blocked_missing_raw_media_no_fallback") == 0);
    CHECK(theron_v1_track02_loader_intake_gate_object_dungeon_grammar_admission(
        &level_handoff_gate, &grammar_gate));
    CHECK(grammar_gate.grammar_gate_evaluated);
    CHECK(grammar_gate.level_handoff_gate_consumed);
    CHECK(!grammar_gate.source_topology_ready);
    CHECK(grammar_gate.raw_media_missing_blocked);
    CHECK(grammar_gate.object_table_grammar_required);
    CHECK(grammar_gate.dungeon_record_grammar_required);
    CHECK(!grammar_gate.object_table_grammar_admitted);
    CHECK(!grammar_gate.dungeon_record_grammar_admitted);
    CHECK(grammar_gate.decoder_semantics_blocked);
    CHECK(grammar_gate.runtime_handoff_blocked);
    CHECK(grammar_gate.rendering_blocked);
    CHECK(grammar_gate.fallback_visuals_blocked);
    CHECK(grammar_gate.no_synthetic_bytes);
    CHECK(grammar_gate.track02_md5 == NULL);
    CHECK(strcmp(grammar_gate.status,
                 "object_dungeon_grammar_admission_blocked_missing_raw_media_no_fallback") == 0);
    CHECK(theron_v1_track02_loader_intake_bind_grammar_admission_to_loader_reads(
        &grammar_gate, NULL, &grammar_read_gate));
    CHECK(!grammar_read_gate.read_evidence_bound);
    CHECK(grammar_read_gate.grammar_gate_consumed);
    CHECK(!grammar_read_gate.read_layout_binding_consumed);
    CHECK(grammar_read_gate.raw_media_missing_blocked);
    CHECK(grammar_read_gate.object_table_grammar_required);
    CHECK(grammar_read_gate.dungeon_record_grammar_required);
    CHECK(!grammar_read_gate.object_table_grammar_admitted);
    CHECK(!grammar_read_gate.dungeon_record_grammar_admitted);
    CHECK(grammar_read_gate.decoder_semantics_blocked);
    CHECK(grammar_read_gate.runtime_handoff_blocked);
    CHECK(grammar_read_gate.rendering_blocked);
    CHECK(grammar_read_gate.fallback_visuals_blocked);
    CHECK(grammar_read_gate.no_synthetic_bytes);
    CHECK(grammar_read_gate.track02_md5 == NULL);
    CHECK(strcmp(grammar_read_gate.status,
                 "object_dungeon_grammar_read_evidence_blocked_missing_raw_media_no_fallback") == 0);
    CHECK(theron_v1_track02_loader_intake_admit_object_dungeon_parser_witness(
        &grammar_read_gate, NULL, &parser_witness_receipt));
    CHECK(!parser_witness_receipt.parser_witness_bound);
    CHECK(parser_witness_receipt.grammar_read_evidence_consumed);
    CHECK(!parser_witness_receipt.original_loader_trace_consumed);
    CHECK(!parser_witness_receipt.original_parser_trace_consumed);
    CHECK(parser_witness_receipt.raw_media_missing_blocked);
    CHECK(!parser_witness_receipt.object_table_grammar_admitted);
    CHECK(!parser_witness_receipt.dungeon_record_grammar_admitted);
    CHECK(parser_witness_receipt.object_table_fields_blocked);
    CHECK(parser_witness_receipt.dungeon_record_fields_blocked);
    CHECK(parser_witness_receipt.decoder_semantics_blocked);
    CHECK(parser_witness_receipt.runtime_handoff_blocked);
    CHECK(parser_witness_receipt.rendering_blocked);
    CHECK(parser_witness_receipt.fallback_visuals_blocked);
    CHECK(parser_witness_receipt.no_synthetic_bytes);
    CHECK(parser_witness_receipt.track02_md5 == NULL);
    CHECK(strcmp(parser_witness_receipt.status,
                 "object_dungeon_parser_witness_blocked_missing_raw_media_no_fallback") == 0);
    CHECK(theron_v1_track02_loader_intake_gate_object_dungeon_decoder_bytes(
        &raw_media_gate, NULL, &decoder_gate));
    CHECK(decoder_gate.gate_evaluated);
    CHECK(decoder_gate.raw_media_gate_consumed);
    CHECK(!decoder_gate.byte_pair_consumed);
    CHECK(!decoder_gate.source_bytes_ready);
    CHECK(decoder_gate.raw_media_missing_blocked);
    CHECK(decoder_gate.decoder_semantics_blocked);
    CHECK(decoder_gate.dungeon_grammar_blocked);
    CHECK(decoder_gate.runtime_handoff_blocked);
    CHECK(decoder_gate.rendering_blocked);
    CHECK(decoder_gate.fallback_visuals_blocked);
    CHECK(decoder_gate.no_synthetic_bytes);
    CHECK(decoder_gate.track02_md5 == NULL);
    CHECK(strcmp(decoder_gate.status,
                 "object_dungeon_decoder_bytes_blocked_missing_raw_media_no_fallback") == 0);
    CHECK(theron_v1_track02_loader_intake_record_object_dungeon_predecode_evidence(
        &decoder_gate, &predecode_evidence));
    CHECK(predecode_evidence.evidence_recorded);
    CHECK(predecode_evidence.decoder_gate_consumed);
    CHECK(!predecode_evidence.source_bytes_ready);
    CHECK(predecode_evidence.raw_media_missing_blocked);
    CHECK(predecode_evidence.decoder_semantics_blocked);
    CHECK(predecode_evidence.dungeon_grammar_blocked);
    CHECK(predecode_evidence.runtime_handoff_blocked);
    CHECK(predecode_evidence.rendering_blocked);
    CHECK(predecode_evidence.fallback_visuals_blocked);
    CHECK(predecode_evidence.no_synthetic_bytes);
    CHECK(predecode_evidence.predecode_evidence_hash == 0u);
    CHECK(predecode_evidence.track02_md5 == NULL);
    CHECK(strcmp(predecode_evidence.status,
                 "object_dungeon_predecode_evidence_blocked_missing_raw_media_no_fallback") == 0);
    if (real_track02) {
        RuntimeConsumerCapture consumer = {0};
        int is_us = theron_v1_track02_raw_bytes_match_md5(
            real_track02, real_track02_bytes, THERON_V1_TRACK02_MD5_US_BIN);

        real_track02_md5 = is_us ? THERON_V1_TRACK02_MD5_US_BIN :
            THERON_V1_TRACK02_MD5_JP_BIN;
        CHECK(theron_v1_track02_raw_bytes_match_md5(
            real_track02, real_track02_bytes, real_track02_md5));
        initial_envelope.cue_track02_index01_raw_sector = is_us ? 225u : 224u;
        initial_envelope.track02_raw_sector = is_us ? 3123u : 3122u;
        initial_envelope.raw_track02_variant =
            is_us ? THERON_V1_TRACK02_VARIANT_US_BIN :
                THERON_V1_TRACK02_VARIANT_JP_BIN;
        raw_cue_facts.runtime_admission = &runtime_admission;
        raw_cue_facts.cue_track02_index01_observed = 1;
        raw_cue_facts.cue_track02_index01_raw_sector =
            initial_envelope.cue_track02_index01_raw_sector;
        raw_cue_facts.raw_bin_present = 1;
        raw_cue_facts.raw_track02 = real_track02;
        raw_cue_facts.raw_track02_bytes = real_track02_bytes;
        raw_cue_facts.track02_md5 = real_track02_md5;
        CHECK(theron_v1_track02_raw_cue_admit(
            &raw_cue_facts, &raw_cue_receipt));
        CHECK(raw_cue_receipt.admitted);
        CHECK(raw_cue_receipt.no_fallback);
        CHECK(theron_v1_track02_loader_intake_gate_later_read_raw_media(
            &raw_cue_receipt, &raw_media_gate));
        CHECK(raw_media_gate.gate_evaluated);
        CHECK(raw_media_gate.raw_media_bound);
        CHECK(!raw_media_gate.raw_media_missing_blocked);
        CHECK(raw_media_gate.raw_cue_admission_consumed);
        CHECK(raw_media_gate.canonical_raw_bin_required);
        CHECK(raw_media_gate.canonical_raw_bin_present);
        CHECK(raw_media_gate.iso_image_blocked);
        CHECK(raw_media_gate.parser_semantics_blocked);
        CHECK(raw_media_gate.runtime_handoff_blocked);
        CHECK(raw_media_gate.rendering_blocked);
        CHECK(raw_media_gate.fallback_visuals_blocked);
        CHECK(raw_media_gate.no_synthetic_bytes);
        CHECK(raw_media_gate.raw_track02_variant ==
              raw_cue_receipt.raw_track02_variant);
        CHECK(raw_media_gate.cue_track02_index01_raw_sector ==
              raw_cue_receipt.cue_track02_index01_raw_sector);
        CHECK(raw_media_gate.raw_track02_bytes ==
              raw_cue_receipt.raw_track02_bytes);
        CHECK(raw_media_gate.track02_md5 == raw_cue_receipt.track02_md5);
        CHECK(strcmp(raw_media_gate.status,
                     "later_loader_raw_track02_media_bound_handoff_still_blocked_no_fallback") == 0);
        raw_cue_receipt.no_fallback = 0;
        CHECK(!theron_v1_track02_loader_intake_gate_later_read_raw_media(
            &raw_cue_receipt, &raw_media_gate));
        CHECK(!raw_media_gate.gate_evaluated);
        raw_cue_receipt.no_fallback = 1;
        CHECK(theron_v1_track02_loader_intake_observe_authenticated_trace(
            &authenticated_facts, &receipt));
        CHECK(receipt.authenticated_v3_trace);
        CHECK(theron_v1_track02_loader_intake_bind_initial_envelope(
            &receipt, &initial_envelope, &receipt));
        CHECK(receipt.authenticated_v3_trace);
        CHECK(theron_v1_track02_loader_intake_decode_initial_envelope(
            &receipt, &initial_envelope, real_track02, real_track02_bytes,
            real_track02_md5, &decoded_receipt));
        CHECK(decoded_receipt.authenticated_v3_trace);
        CHECK(decoded_receipt.payload_intake_admitted);
        CHECK(decoded_receipt.initial_envelope_decoded);
        CHECK(decoded_receipt.decoded_grid_row_count == 0x001bu);
        CHECK(decoded_receipt.decoded_grid_row_bytes == 0x0020u);
        CHECK(decoded_receipt.decoded_grid_raw_sector ==
              initial_envelope.track02_raw_sector);
        CHECK(decoded_receipt.decoded_grid_raw_sector_offset == 0x130u);
        if (is_us) {
            CHECK(decoded_receipt.decoded_grid_first_row_hash == 0x4b97e3abu);
            CHECK(decoded_receipt.decoded_grid_last_row_hash == 0x0b2ae445u);
        }
        CHECK(theron_v1_track02_loader_intake_handoff_raw_grid(
            &decoded_receipt, real_track02, real_track02_bytes,
            real_track02_md5, &grid_receipt));
        CHECK(grid_receipt.handed_off);
        CHECK(grid_receipt.authenticated_v3_trace);
        CHECK(grid_receipt.raw_grid_width == 0x0020u);
        CHECK(grid_receipt.raw_grid_height == 0x001bu);
        CHECK(grid_receipt.raw_grid_bytes == 0x0360u);
        if (is_us) {
            CHECK(grid_receipt.raw_grid[0] == 0x84u);
            CHECK(grid_receipt.raw_grid[31] == 0x56u);
            CHECK(grid_receipt.raw_grid[0x035fu] == 0u);
        }
        CHECK(grid_receipt.raw_track02_sector == initial_envelope.track02_raw_sector);
        CHECK(grid_receipt.raw_sector_offset == 0x130u);
        CHECK(grid_receipt.raw_track02_offset == 0x7015c0u);
        CHECK(grid_receipt.raw_grid_hash == decoded_receipt.decoded_grid_hash);
        CHECK(strcmp(grid_receipt.status,
                     "initial_envelope_raw_grid_handoff_no_semantics") == 0);
        CHECK(theron_v1_track02_loader_intake_block_raw_grid_object_table_projection(
            &grid_receipt, &object_projection));
        CHECK(object_projection.projection_blocked);
        CHECK(object_projection.authenticated_v3_trace);
        CHECK(object_projection.no_fallback);
        CHECK(object_projection.raw_grid_bytes == grid_receipt.raw_grid_bytes);
        CHECK(object_projection.raw_grid_hash == grid_receipt.raw_grid_hash);
        CHECK(object_projection.raw_track02_offset == grid_receipt.raw_track02_offset);
        CHECK(strcmp(object_projection.status,
                     "initial_envelope_raw_grid_object_table_projection_blocked_no_fallback") == 0);
        CHECK(theron_v1_track02_loader_intake_block_object_table_read_claim(
            &grid_receipt, &facts, &object_read_block));
        CHECK(object_read_block.object_table_read_blocked);
        CHECK(object_read_block.authenticated_v3_trace);
        CHECK(object_read_block.candidate_read_seen);
        CHECK(object_read_block.candidate_read_authenticated);
        CHECK(object_read_block.startup_record_rejected_as_object_table);
        CHECK(object_read_block.separate_object_table_record_required);
        CHECK(object_read_block.later_loader_read_required);
        CHECK(object_read_block.no_fallback);
        CHECK(object_read_block.candidate_record ==
              THERON_V1_INITIAL_ENVELOPE_RECORD);
        CHECK(object_read_block.candidate_record_user_data_offset ==
              THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET);
        CHECK(object_read_block.candidate_destination == facts.destination);
        CHECK(object_read_block.candidate_byte_count == facts.byte_count);
        CHECK(object_read_block.startup_grid_record ==
              THERON_V1_INITIAL_ENVELOPE_RECORD);
        CHECK(object_read_block.raw_grid_bytes == grid_receipt.raw_grid_bytes);
        CHECK(object_read_block.raw_grid_hash == grid_receipt.raw_grid_hash);
        CHECK(object_read_block.raw_track02_offset == grid_receipt.raw_track02_offset);
        CHECK(strcmp(object_read_block.status,
                     "initial_envelope_object_table_read_claim_blocked_separate_later_read_required_no_fallback") == 0);
        CHECK(theron_v1_track02_loader_intake_block_object_table_read_claim(
            &grid_receipt, NULL, &object_read_block));
        CHECK(object_read_block.object_table_read_blocked);
        CHECK(!object_read_block.candidate_read_seen);
        CHECK(!object_read_block.startup_record_rejected_as_object_table);
        CHECK(object_read_block.separate_object_table_record_required);
        CHECK(object_read_block.later_loader_read_required);
        CHECK(object_read_block.no_fallback);
        CHECK(theron_v1_track02_loader_intake_block_raw_grid_bitmap_route(
            &grid_receipt, &bitmap_route));
        CHECK(bitmap_route.bitmap_route_blocked);
        CHECK(bitmap_route.authenticated_v3_trace);
        CHECK(bitmap_route.no_fallback);
        CHECK(bitmap_route.raw_grid_bytes == grid_receipt.raw_grid_bytes);
        CHECK(bitmap_route.raw_grid_hash == grid_receipt.raw_grid_hash);
        CHECK(bitmap_route.raw_track02_offset == grid_receipt.raw_track02_offset);
        CHECK(strcmp(bitmap_route.status,
                     "initial_envelope_raw_grid_bitmap_route_blocked_no_fallback") == 0);
        CHECK(theron_v1_track02_loader_intake_block_bitmap_read_claim(
            &grid_receipt, &facts, &bitmap_read_block));
        CHECK(bitmap_read_block.bitmap_read_blocked);
        CHECK(bitmap_read_block.authenticated_v3_trace);
        CHECK(bitmap_read_block.candidate_read_seen);
        CHECK(bitmap_read_block.candidate_read_authenticated);
        CHECK(bitmap_read_block.startup_record_rejected_as_bitmap);
        CHECK(bitmap_read_block.separate_bitmap_record_required);
        CHECK(bitmap_read_block.later_loader_read_required);
        CHECK(bitmap_read_block.palette_binding_required);
        CHECK(bitmap_read_block.no_fallback_visual);
        CHECK(bitmap_read_block.candidate_record ==
              THERON_V1_INITIAL_ENVELOPE_RECORD);
        CHECK(bitmap_read_block.candidate_record_user_data_offset ==
              THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET);
        CHECK(bitmap_read_block.candidate_destination == facts.destination);
        CHECK(bitmap_read_block.candidate_byte_count == facts.byte_count);
        CHECK(bitmap_read_block.startup_grid_record ==
              THERON_V1_INITIAL_ENVELOPE_RECORD);
        CHECK(bitmap_read_block.raw_grid_bytes == grid_receipt.raw_grid_bytes);
        CHECK(bitmap_read_block.raw_grid_hash == grid_receipt.raw_grid_hash);
        CHECK(bitmap_read_block.raw_track02_offset == grid_receipt.raw_track02_offset);
        CHECK(strcmp(bitmap_read_block.status,
                     "initial_envelope_bitmap_read_claim_blocked_separate_later_read_palette_required_no_fallback") == 0);
        CHECK(theron_v1_track02_loader_intake_block_bitmap_read_claim(
            &grid_receipt, NULL, &bitmap_read_block));
        CHECK(bitmap_read_block.bitmap_read_blocked);
        CHECK(!bitmap_read_block.candidate_read_seen);
        CHECK(!bitmap_read_block.startup_record_rejected_as_bitmap);
        CHECK(bitmap_read_block.separate_bitmap_record_required);
        CHECK(bitmap_read_block.palette_binding_required);
        CHECK(bitmap_read_block.no_fallback_visual);
        CHECK(theron_v1_track02_loader_intake_admit_raw_grid_level_route(
            &grid_receipt, &level_route));
        CHECK(level_route.level_route_admitted);
        CHECK(level_route.authenticated_v3_trace);
        CHECK(level_route.bitmap_route_blocked);
        CHECK(level_route.object_route_blocked);
        CHECK(level_route.no_fallback);
        CHECK(level_route.raw_grid_bytes == grid_receipt.raw_grid_bytes);
        CHECK(level_route.raw_grid_hash == grid_receipt.raw_grid_hash);
        CHECK(level_route.raw_track02_offset == grid_receipt.raw_track02_offset);
        CHECK(strcmp(level_route.status,
                     "initial_envelope_raw_grid_level_route_bitmap_object_blocked_no_fallback") == 0);
        CHECK(theron_v1_track02_loader_intake_admit_raw_grid_dungeon_route(
            &grid_receipt, &dungeon_route));
        CHECK(dungeon_route.dungeon_route_admitted);
        CHECK(dungeon_route.authenticated_v3_trace);
        CHECK(dungeon_route.bitmap_route_blocked);
        CHECK(dungeon_route.object_route_blocked);
        CHECK(dungeon_route.no_fallback);
        CHECK(dungeon_route.raw_grid_bytes == grid_receipt.raw_grid_bytes);
        CHECK(dungeon_route.raw_grid_hash == grid_receipt.raw_grid_hash);
        CHECK(dungeon_route.raw_track02_offset == grid_receipt.raw_track02_offset);
        CHECK(strcmp(dungeon_route.status,
                     "initial_envelope_raw_grid_dungeon_route_bitmap_object_blocked_no_fallback") == 0);
        CHECK(theron_v1_track02_loader_intake_block_raw_grid_dungeon_record_evidence(
            &grid_receipt, &dungeon_record));
        CHECK(dungeon_record.dungeon_record_blocked);
        CHECK(dungeon_record.object_table_record_blocked);
        CHECK(dungeon_record.authenticated_v3_trace);
        CHECK(dungeon_record.later_loader_read_required);
        CHECK(dungeon_record.no_fallback);
        CHECK(dungeon_record.expected_dungeon_record ==
              THERON_V1_INITIAL_ENVELOPE_RECORD);
        CHECK(dungeon_record.observed_raw_grid_record ==
              THERON_V1_INITIAL_ENVELOPE_RECORD);
        CHECK(dungeon_record.raw_grid_bytes == grid_receipt.raw_grid_bytes);
        CHECK(dungeon_record.raw_grid_hash == grid_receipt.raw_grid_hash);
        CHECK(dungeon_record.raw_track02_offset == grid_receipt.raw_track02_offset);
        CHECK(strcmp(dungeon_record.status,
                     "initial_envelope_raw_grid_dungeon_record_object_table_blocked_later_read_required_no_fallback") == 0);
        CHECK(theron_v1_track02_loader_intake_block_dungeon_read_claim(
            &grid_receipt, &facts, &dungeon_read_block));
        CHECK(dungeon_read_block.dungeon_read_blocked);
        CHECK(dungeon_read_block.authenticated_v3_trace);
        CHECK(dungeon_read_block.candidate_read_seen);
        CHECK(dungeon_read_block.candidate_read_authenticated);
        CHECK(dungeon_read_block.startup_record_rejected_as_dungeon);
        CHECK(dungeon_read_block.separate_dungeon_record_required);
        CHECK(dungeon_read_block.later_loader_read_required);
        CHECK(dungeon_read_block.grammar_binding_required);
        CHECK(dungeon_read_block.no_fallback_dungeon);
        CHECK(dungeon_read_block.candidate_record ==
              THERON_V1_INITIAL_ENVELOPE_RECORD);
        CHECK(dungeon_read_block.candidate_record_user_data_offset ==
              THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET);
        CHECK(dungeon_read_block.candidate_destination == facts.destination);
        CHECK(dungeon_read_block.candidate_byte_count == facts.byte_count);
        CHECK(dungeon_read_block.startup_grid_record ==
              THERON_V1_INITIAL_ENVELOPE_RECORD);
        CHECK(dungeon_read_block.raw_grid_bytes == grid_receipt.raw_grid_bytes);
        CHECK(dungeon_read_block.raw_grid_hash == grid_receipt.raw_grid_hash);
        CHECK(dungeon_read_block.raw_track02_offset ==
              grid_receipt.raw_track02_offset);
        CHECK(strcmp(dungeon_read_block.status,
                     "initial_envelope_dungeon_read_claim_blocked_separate_later_read_grammar_required_no_fallback") == 0);
        CHECK(theron_v1_track02_loader_intake_block_dungeon_read_claim(
            &grid_receipt, NULL, &dungeon_read_block));
        CHECK(dungeon_read_block.dungeon_read_blocked);
        CHECK(!dungeon_read_block.candidate_read_seen);
        CHECK(!dungeon_read_block.startup_record_rejected_as_dungeon);
        CHECK(dungeon_read_block.separate_dungeon_record_required);
        CHECK(dungeon_read_block.later_loader_read_required);
        CHECK(dungeon_read_block.grammar_binding_required);
        CHECK(dungeon_read_block.no_fallback_dungeon);
        CHECK(theron_v1_track02_loader_intake_gate_object_dungeon_handoff(
            &grid_receipt, &facts, &facts, &object_dungeon_gate));
        CHECK(object_dungeon_gate.handoff_blocked);
        CHECK(object_dungeon_gate.authenticated_v3_trace);
        CHECK(object_dungeon_gate.object_read_seen);
        CHECK(object_dungeon_gate.dungeon_read_seen);
        CHECK(object_dungeon_gate.object_read_authenticated);
        CHECK(object_dungeon_gate.dungeon_read_authenticated);
        CHECK(object_dungeon_gate.object_startup_record_rejected);
        CHECK(object_dungeon_gate.dungeon_startup_record_rejected);
        CHECK(!object_dungeon_gate.object_later_read_proven);
        CHECK(!object_dungeon_gate.dungeon_later_read_proven);
        CHECK(object_dungeon_gate.object_decoder_binding_required);
        CHECK(object_dungeon_gate.dungeon_grammar_binding_required);
        CHECK(object_dungeon_gate.object_handoff_blocked);
        CHECK(object_dungeon_gate.dungeon_handoff_blocked);
        CHECK(object_dungeon_gate.no_fallback_visuals);
        CHECK(object_dungeon_gate.no_synthetic_handoff);
        CHECK(object_dungeon_gate.startup_grid_record ==
              THERON_V1_INITIAL_ENVELOPE_RECORD);
        CHECK(object_dungeon_gate.raw_grid_hash == grid_receipt.raw_grid_hash);
        CHECK(strcmp(object_dungeon_gate.status,
                     "initial_envelope_object_dungeon_handoff_blocked_later_reads_decoder_grammar_required_no_fallback") == 0);
        CHECK(theron_v1_track02_loader_intake_gate_object_dungeon_handoff(
            &grid_receipt, &separate_object_facts, &separate_dungeon_facts,
            &object_dungeon_gate));
        CHECK(object_dungeon_gate.handoff_blocked);
        CHECK(object_dungeon_gate.object_read_authenticated);
        CHECK(object_dungeon_gate.dungeon_read_authenticated);
        CHECK(!object_dungeon_gate.object_startup_record_rejected);
        CHECK(!object_dungeon_gate.dungeon_startup_record_rejected);
        CHECK(object_dungeon_gate.object_later_read_proven);
        CHECK(object_dungeon_gate.dungeon_later_read_proven);
        CHECK(object_dungeon_gate.object_candidate_record ==
              separate_object_facts.track02_record);
        CHECK(object_dungeon_gate.object_candidate_destination ==
              separate_object_facts.destination);
        CHECK(object_dungeon_gate.object_candidate_byte_count ==
              separate_object_facts.byte_count);
        CHECK(object_dungeon_gate.dungeon_candidate_record ==
              separate_dungeon_facts.track02_record);
        CHECK(object_dungeon_gate.dungeon_candidate_destination ==
              separate_dungeon_facts.destination);
        CHECK(object_dungeon_gate.dungeon_candidate_byte_count ==
              separate_dungeon_facts.byte_count);
        CHECK(object_dungeon_gate.object_decoder_binding_required);
        CHECK(object_dungeon_gate.dungeon_grammar_binding_required);
        CHECK(object_dungeon_gate.object_handoff_blocked);
        CHECK(object_dungeon_gate.dungeon_handoff_blocked);
        CHECK(object_dungeon_gate.no_fallback_visuals);
        CHECK(object_dungeon_gate.no_synthetic_handoff);
        CHECK(theron_v1_track02_loader_intake_admit_later_cd_record_read(
            &raw_cue_receipt, &separate_object_facts, &later_cd_record));
        CHECK(later_cd_record.cd_record_read_proven);
        CHECK(later_cd_record.raw_cue_admission_consumed);
        CHECK(later_cd_record.authenticated_later_loader_read);
        CHECK(!later_cd_record.startup_record_rejected);
        CHECK(later_cd_record.object_semantics_blocked);
        CHECK(later_cd_record.dungeon_semantics_blocked);
        CHECK(later_cd_record.decoder_binding_required);
        CHECK(later_cd_record.grammar_binding_required);
        CHECK(later_cd_record.no_fallback_visuals);
        CHECK(later_cd_record.no_synthetic_handoff);
        CHECK(later_cd_record.raw_track02_variant ==
              raw_cue_receipt.raw_track02_variant);
        CHECK(later_cd_record.track02_record ==
              separate_object_facts.track02_record);
        CHECK(later_cd_record.record_user_data_offset ==
              separate_object_facts.record_user_data_offset);
        CHECK(later_cd_record.destination == separate_object_facts.destination);
        CHECK(later_cd_record.byte_count == separate_object_facts.byte_count);
        CHECK(later_cd_record.raw_track02_sector ==
              raw_cue_receipt.cue_track02_index01_raw_sector +
                  separate_object_facts.track02_record);
        CHECK(later_cd_record.raw_sector_offset ==
              THERON_V1_TRACK02_MODE1_HEADER_BYTES +
                  separate_object_facts.record_user_data_offset);
        CHECK(later_cd_record.raw_track02_offset ==
              later_cd_record.raw_track02_sector *
                  THERON_V1_TRACK02_RAW_SECTOR_BYTES +
                  later_cd_record.raw_sector_offset);
        CHECK(later_cd_record.track02_md5 == raw_cue_receipt.track02_md5);
        CHECK(strcmp(later_cd_record.status,
                     "later_loader_cd_record_read_proven_semantics_blocked_no_fallback") == 0);
        object_cd_record = later_cd_record;
        CHECK(theron_v1_track02_loader_intake_bind_later_read_layout(
            &later_cd_record, THERON_V1_TRACK02_LAYOUT_ROLE_OBJECT_TABLE,
            &later_layout));
        CHECK(later_layout.layout_window_bound);
        CHECK(later_layout.cd_record_read_consumed);
        CHECK(later_layout.object_layout_bound);
        CHECK(!later_layout.dungeon_layout_bound);
        CHECK(later_layout.parser_semantics_blocked);
        CHECK(later_layout.runtime_handoff_blocked);
        CHECK(later_layout.rendering_blocked);
        CHECK(later_layout.fallback_visuals_blocked);
        CHECK(later_layout.decoder_or_grammar_required);
        CHECK(later_layout.no_synthetic_layout);
        CHECK(later_layout.role == THERON_V1_TRACK02_LAYOUT_ROLE_OBJECT_TABLE);
        CHECK(later_layout.track02_record == later_cd_record.track02_record);
        CHECK(later_layout.record_user_data_offset ==
              later_cd_record.record_user_data_offset);
        CHECK(later_layout.destination == later_cd_record.destination);
        CHECK(later_layout.layout_bytes == later_cd_record.byte_count);
        CHECK(later_layout.raw_track02_sector ==
              later_cd_record.raw_track02_sector);
        CHECK(later_layout.raw_sector_offset ==
              later_cd_record.raw_sector_offset);
        CHECK(later_layout.raw_track02_offset ==
              later_cd_record.raw_track02_offset);
        CHECK(later_layout.track02_md5 == later_cd_record.track02_md5);
        CHECK(strcmp(later_layout.status,
                     "later_loader_layout_window_bound_parser_runtime_render_blocked") == 0);
        object_layout = later_layout;
        CHECK(theron_v1_track02_loader_intake_admit_later_cd_record_read(
            &raw_cue_receipt, &separate_dungeon_facts, &later_cd_record));
        CHECK(later_cd_record.track02_record ==
              separate_dungeon_facts.track02_record);
        CHECK(later_cd_record.dungeon_semantics_blocked);
        CHECK(later_cd_record.grammar_binding_required);
        dungeon_cd_record = later_cd_record;
        CHECK(theron_v1_track02_loader_intake_bind_object_dungeon_loader_read_table(
            &raw_media_gate, &object_cd_record, &dungeon_cd_record,
            &read_table));
        CHECK(read_table.table_bound);
        CHECK(read_table.raw_media_gate_consumed);
        CHECK(read_table.object_cd_record_consumed);
        CHECK(read_table.dungeon_cd_record_consumed);
        CHECK(!read_table.raw_media_missing_blocked);
        CHECK(read_table.same_track02_media);
        CHECK(read_table.distinct_records);
        CHECK(read_table.non_overlapping_raw_windows);
        CHECK(read_table.parser_semantics_blocked);
        CHECK(read_table.dungeon_grammar_blocked);
        CHECK(read_table.runtime_handoff_blocked);
        CHECK(read_table.rendering_blocked);
        CHECK(read_table.fallback_visuals_blocked);
        CHECK(read_table.no_synthetic_handoff);
        CHECK(read_table.raw_track02_variant ==
              raw_media_gate.raw_track02_variant);
        CHECK(read_table.object_track02_record ==
              object_cd_record.track02_record);
        CHECK(read_table.object_record_user_data_offset ==
              object_cd_record.record_user_data_offset);
        CHECK(read_table.object_destination == object_cd_record.destination);
        CHECK(read_table.object_byte_count == object_cd_record.byte_count);
        CHECK(read_table.object_raw_track02_offset ==
              object_cd_record.raw_track02_offset);
        CHECK(read_table.dungeon_track02_record ==
              dungeon_cd_record.track02_record);
        CHECK(read_table.dungeon_record_user_data_offset ==
              dungeon_cd_record.record_user_data_offset);
        CHECK(read_table.dungeon_destination ==
              dungeon_cd_record.destination);
        CHECK(read_table.dungeon_byte_count == dungeon_cd_record.byte_count);
        CHECK(read_table.dungeon_raw_track02_offset ==
              dungeon_cd_record.raw_track02_offset);
        CHECK(read_table.track02_md5 == raw_media_gate.track02_md5);
        CHECK(strcmp(read_table.status,
                     "object_dungeon_loader_read_table_bound_semantics_blocked_no_fallback") == 0);
        dungeon_cd_record.no_synthetic_handoff = 0;
        CHECK(!theron_v1_track02_loader_intake_bind_object_dungeon_loader_read_table(
            &raw_media_gate, &object_cd_record, &dungeon_cd_record,
            &read_table));
        CHECK(!read_table.table_bound);
        dungeon_cd_record.no_synthetic_handoff = 1;
        dungeon_cd_record.track02_record = object_cd_record.track02_record;
        dungeon_cd_record.raw_track02_sector = object_cd_record.raw_track02_sector;
        dungeon_cd_record.raw_track02_offset = object_cd_record.raw_track02_offset;
        CHECK(!theron_v1_track02_loader_intake_bind_object_dungeon_loader_read_table(
            &raw_media_gate, &object_cd_record, &dungeon_cd_record,
            &read_table));
        CHECK(!read_table.table_bound);
        dungeon_cd_record = later_cd_record;
        CHECK(theron_v1_track02_loader_intake_bind_later_read_layout(
            &later_cd_record, THERON_V1_TRACK02_LAYOUT_ROLE_DUNGEON_RECORD,
            &later_layout));
        CHECK(later_layout.layout_window_bound);
        CHECK(!later_layout.object_layout_bound);
        CHECK(later_layout.dungeon_layout_bound);
        CHECK(later_layout.parser_semantics_blocked);
        CHECK(later_layout.runtime_handoff_blocked);
        CHECK(later_layout.rendering_blocked);
        CHECK(later_layout.fallback_visuals_blocked);
        CHECK(later_layout.decoder_or_grammar_required);
        CHECK(later_layout.no_synthetic_layout);
        CHECK(later_layout.role == THERON_V1_TRACK02_LAYOUT_ROLE_DUNGEON_RECORD);
        dungeon_layout = later_layout;
        CHECK(theron_v1_track02_loader_intake_bind_read_table_to_layouts(
            &read_table, &object_layout, &dungeon_layout,
            &read_layout_binding));
        CHECK(read_layout_binding.binding_bound);
        CHECK(read_layout_binding.read_table_consumed);
        CHECK(read_layout_binding.object_layout_consumed);
        CHECK(read_layout_binding.dungeon_layout_consumed);
        CHECK(!read_layout_binding.raw_media_missing_blocked);
        CHECK(read_layout_binding.same_track02_media);
        CHECK(read_layout_binding.destinations_preserved);
        CHECK(read_layout_binding.layout_windows_match_reads);
        CHECK(read_layout_binding.parser_semantics_blocked);
        CHECK(read_layout_binding.dungeon_grammar_blocked);
        CHECK(read_layout_binding.runtime_handoff_blocked);
        CHECK(read_layout_binding.rendering_blocked);
        CHECK(read_layout_binding.fallback_visuals_blocked);
        CHECK(read_layout_binding.no_synthetic_layout);
        CHECK(read_layout_binding.raw_track02_variant ==
              read_table.raw_track02_variant);
        CHECK(read_layout_binding.object_track02_record ==
              read_table.object_track02_record);
        CHECK(read_layout_binding.object_record_user_data_offset ==
              read_table.object_record_user_data_offset);
        CHECK(read_layout_binding.object_destination ==
              read_table.object_destination);
        CHECK(read_layout_binding.object_layout_bytes ==
              read_table.object_byte_count);
        CHECK(read_layout_binding.object_raw_track02_offset ==
              read_table.object_raw_track02_offset);
        CHECK(read_layout_binding.dungeon_track02_record ==
              read_table.dungeon_track02_record);
        CHECK(read_layout_binding.dungeon_record_user_data_offset ==
              read_table.dungeon_record_user_data_offset);
        CHECK(read_layout_binding.dungeon_destination ==
              read_table.dungeon_destination);
        CHECK(read_layout_binding.dungeon_layout_bytes ==
              read_table.dungeon_byte_count);
        CHECK(read_layout_binding.dungeon_raw_track02_offset ==
              read_table.dungeon_raw_track02_offset);
        CHECK(read_layout_binding.track02_md5 == read_table.track02_md5);
        CHECK(strcmp(read_layout_binding.status,
                     "object_dungeon_read_table_layout_binding_bound_semantics_blocked_no_fallback") == 0);
        dungeon_layout.destination = object_layout.destination;
        CHECK(!theron_v1_track02_loader_intake_bind_read_table_to_layouts(
            &read_table, &object_layout, &dungeon_layout,
            &read_layout_binding));
        CHECK(!read_layout_binding.binding_bound);
        dungeon_layout = later_layout;
        CHECK(theron_v1_track02_loader_intake_bind_object_dungeon_layout_pair(
            &object_layout, &dungeon_layout, &layout_pair));
        CHECK(layout_pair.layout_pair_bound);
        CHECK(layout_pair.object_layout_consumed);
        CHECK(layout_pair.dungeon_layout_consumed);
        CHECK(layout_pair.same_track02_media);
        CHECK(layout_pair.distinct_records);
        CHECK(layout_pair.non_overlapping_windows);
        CHECK(layout_pair.parser_semantics_blocked);
        CHECK(layout_pair.runtime_handoff_blocked);
        CHECK(layout_pair.rendering_blocked);
        CHECK(layout_pair.fallback_visuals_blocked);
        CHECK(layout_pair.no_synthetic_layout);
        CHECK(layout_pair.raw_track02_variant == object_layout.raw_track02_variant);
        CHECK(layout_pair.object_track02_record == object_layout.track02_record);
        CHECK(layout_pair.object_record_user_data_offset ==
              object_layout.record_user_data_offset);
        CHECK(layout_pair.object_layout_bytes == object_layout.layout_bytes);
        CHECK(layout_pair.object_raw_track02_offset ==
              object_layout.raw_track02_offset);
        CHECK(layout_pair.dungeon_track02_record ==
              dungeon_layout.track02_record);
        CHECK(layout_pair.dungeon_record_user_data_offset ==
              dungeon_layout.record_user_data_offset);
        CHECK(layout_pair.dungeon_layout_bytes == dungeon_layout.layout_bytes);
        CHECK(layout_pair.dungeon_raw_track02_offset ==
              dungeon_layout.raw_track02_offset);
        CHECK(layout_pair.track02_md5 == object_layout.track02_md5);
        CHECK(strcmp(layout_pair.status,
                     "object_dungeon_layout_pair_bound_nonoverlap_render_blocked") == 0);
        CHECK(theron_v1_track02_loader_intake_bridge_read_layout_binding_to_layout_pair(
            &read_layout_binding, &layout_pair, &read_layout_pair_bridge));
        CHECK(read_layout_pair_bridge.bridge_bound);
        CHECK(read_layout_pair_bridge.read_layout_binding_consumed);
        CHECK(read_layout_pair_bridge.layout_pair_consumed);
        CHECK(!read_layout_pair_bridge.raw_media_missing_blocked);
        CHECK(read_layout_pair_bridge.same_track02_media);
        CHECK(read_layout_pair_bridge.read_windows_preserved);
        CHECK(read_layout_pair_bridge.non_overlapping_windows);
        CHECK(read_layout_pair_bridge.parser_semantics_blocked);
        CHECK(read_layout_pair_bridge.dungeon_grammar_blocked);
        CHECK(read_layout_pair_bridge.runtime_handoff_blocked);
        CHECK(read_layout_pair_bridge.rendering_blocked);
        CHECK(read_layout_pair_bridge.fallback_visuals_blocked);
        CHECK(read_layout_pair_bridge.no_synthetic_layout);
        CHECK(read_layout_pair_bridge.raw_track02_variant ==
              read_layout_binding.raw_track02_variant);
        CHECK(read_layout_pair_bridge.object_track02_record ==
              read_layout_binding.object_track02_record);
        CHECK(read_layout_pair_bridge.object_record_user_data_offset ==
              read_layout_binding.object_record_user_data_offset);
        CHECK(read_layout_pair_bridge.object_destination ==
              read_layout_binding.object_destination);
        CHECK(read_layout_pair_bridge.object_layout_bytes ==
              read_layout_binding.object_layout_bytes);
        CHECK(read_layout_pair_bridge.object_raw_track02_offset ==
              read_layout_binding.object_raw_track02_offset);
        CHECK(read_layout_pair_bridge.dungeon_track02_record ==
              read_layout_binding.dungeon_track02_record);
        CHECK(read_layout_pair_bridge.dungeon_record_user_data_offset ==
              read_layout_binding.dungeon_record_user_data_offset);
        CHECK(read_layout_pair_bridge.dungeon_destination ==
              read_layout_binding.dungeon_destination);
        CHECK(read_layout_pair_bridge.dungeon_layout_bytes ==
              read_layout_binding.dungeon_layout_bytes);
        CHECK(read_layout_pair_bridge.dungeon_raw_track02_offset ==
              read_layout_binding.dungeon_raw_track02_offset);
        CHECK(read_layout_pair_bridge.track02_md5 ==
              read_layout_binding.track02_md5);
        CHECK(strcmp(read_layout_pair_bridge.status,
                     "object_dungeon_read_layout_pair_bridge_bound_semantics_blocked_no_fallback") == 0);
        layout_pair.dungeon_layout_bytes = layout_pair.object_layout_bytes;
        CHECK(!theron_v1_track02_loader_intake_bridge_read_layout_binding_to_layout_pair(
            &read_layout_binding, &layout_pair, &read_layout_pair_bridge));
        CHECK(!read_layout_pair_bridge.bridge_bound);
        layout_pair.dungeon_layout_bytes = dungeon_layout.layout_bytes;
        CHECK(theron_v1_track02_loader_intake_handoff_later_layout_bytes(
            &object_layout, real_track02, real_track02_bytes, real_track02_md5,
            layout_bytes, sizeof(layout_bytes), &layout_bytes_receipt));
        CHECK(layout_bytes_receipt.bytes_handed_off);
        CHECK(layout_bytes_receipt.layout_receipt_consumed);
        CHECK(layout_bytes_receipt.object_layout_bytes);
        CHECK(!layout_bytes_receipt.dungeon_layout_bytes);
        CHECK(layout_bytes_receipt.exact_source_bytes);
        CHECK(layout_bytes_receipt.parser_semantics_blocked);
        CHECK(layout_bytes_receipt.runtime_handoff_blocked);
        CHECK(layout_bytes_receipt.rendering_blocked);
        CHECK(layout_bytes_receipt.fallback_visuals_blocked);
        CHECK(layout_bytes_receipt.no_synthetic_bytes);
        CHECK(layout_bytes_receipt.role ==
              THERON_V1_TRACK02_LAYOUT_ROLE_OBJECT_TABLE);
        CHECK(layout_bytes_receipt.track02_record ==
              object_layout.track02_record);
        CHECK(layout_bytes_receipt.layout_bytes ==
              object_layout.layout_bytes);
        CHECK(layout_bytes_receipt.raw_track02_offset ==
              object_layout.raw_track02_offset);
        CHECK(layout_bytes_receipt.track02_md5 == object_layout.track02_md5);
        CHECK(layout_bytes[0] == real_track02[object_layout.raw_track02_offset]);
        CHECK(layout_bytes[object_layout.layout_bytes - 1u] ==
              real_track02[object_layout.raw_track02_offset +
                  object_layout.layout_bytes - 1u]);
        CHECK(strcmp(layout_bytes_receipt.status,
                     "later_loader_layout_bytes_handoff_opaque_render_blocked") == 0);
        object_layout_bytes = layout_bytes_receipt;
        CHECK(theron_v1_track02_loader_intake_handoff_later_layout_bytes(
            &dungeon_layout, real_track02, real_track02_bytes, real_track02_md5,
            layout_bytes, sizeof(layout_bytes), &layout_bytes_receipt));
        CHECK(layout_bytes_receipt.bytes_handed_off);
        CHECK(!layout_bytes_receipt.object_layout_bytes);
        CHECK(layout_bytes_receipt.dungeon_layout_bytes);
        CHECK(layout_bytes_receipt.role ==
              THERON_V1_TRACK02_LAYOUT_ROLE_DUNGEON_RECORD);
        CHECK(layout_bytes_receipt.layout_bytes ==
              dungeon_layout.layout_bytes);
        CHECK(layout_bytes[0] == real_track02[dungeon_layout.raw_track02_offset]);
        dungeon_layout_bytes = layout_bytes_receipt;
        CHECK(theron_v1_track02_loader_intake_bind_object_dungeon_layout_bytes_pair(
            &layout_pair, &object_layout_bytes, &dungeon_layout_bytes,
            &layout_bytes_pair));
        CHECK(layout_bytes_pair.byte_pair_bound);
        CHECK(layout_bytes_pair.layout_pair_consumed);
        CHECK(layout_bytes_pair.object_bytes_consumed);
        CHECK(layout_bytes_pair.dungeon_bytes_consumed);
        CHECK(layout_bytes_pair.same_track02_media);
        CHECK(layout_bytes_pair.non_overlapping_windows);
        CHECK(layout_bytes_pair.exact_source_bytes);
        CHECK(layout_bytes_pair.parser_semantics_blocked);
        CHECK(layout_bytes_pair.runtime_handoff_blocked);
        CHECK(layout_bytes_pair.rendering_blocked);
        CHECK(layout_bytes_pair.fallback_visuals_blocked);
        CHECK(layout_bytes_pair.no_synthetic_bytes);
        CHECK(layout_bytes_pair.object_track02_record ==
              object_layout_bytes.track02_record);
        CHECK(layout_bytes_pair.object_layout_bytes ==
              object_layout_bytes.layout_bytes);
        CHECK(layout_bytes_pair.object_layout_hash ==
              object_layout_bytes.layout_hash);
        CHECK(layout_bytes_pair.object_raw_track02_offset ==
              object_layout_bytes.raw_track02_offset);
        CHECK(layout_bytes_pair.dungeon_track02_record ==
              dungeon_layout_bytes.track02_record);
        CHECK(layout_bytes_pair.dungeon_layout_bytes ==
              dungeon_layout_bytes.layout_bytes);
        CHECK(layout_bytes_pair.dungeon_layout_hash ==
              dungeon_layout_bytes.layout_hash);
        CHECK(layout_bytes_pair.dungeon_raw_track02_offset ==
              dungeon_layout_bytes.raw_track02_offset);
        CHECK(layout_bytes_pair.track02_md5 == layout_pair.track02_md5);
        CHECK(strcmp(layout_bytes_pair.status,
                     "object_dungeon_layout_bytes_pair_bound_opaque_render_blocked") == 0);
        CHECK(theron_v1_track02_loader_intake_bridge_read_layout_pair_to_bytes(
            &read_layout_pair_bridge, &layout_bytes_pair,
            &read_to_bytes_bridge));
        CHECK(read_to_bytes_bridge.bridge_bound);
        CHECK(read_to_bytes_bridge.read_layout_pair_bridge_consumed);
        CHECK(read_to_bytes_bridge.byte_pair_consumed);
        CHECK(!read_to_bytes_bridge.raw_media_missing_blocked);
        CHECK(read_to_bytes_bridge.same_track02_media);
        CHECK(read_to_bytes_bridge.source_windows_preserved);
        CHECK(read_to_bytes_bridge.byte_hashes_recorded);
        CHECK(read_to_bytes_bridge.parser_semantics_blocked);
        CHECK(read_to_bytes_bridge.dungeon_grammar_blocked);
        CHECK(read_to_bytes_bridge.runtime_handoff_blocked);
        CHECK(read_to_bytes_bridge.rendering_blocked);
        CHECK(read_to_bytes_bridge.fallback_visuals_blocked);
        CHECK(read_to_bytes_bridge.no_synthetic_bytes);
        CHECK(read_to_bytes_bridge.raw_track02_variant ==
              read_layout_pair_bridge.raw_track02_variant);
        CHECK(read_to_bytes_bridge.object_track02_record ==
              read_layout_pair_bridge.object_track02_record);
        CHECK(read_to_bytes_bridge.object_layout_bytes ==
              read_layout_pair_bridge.object_layout_bytes);
        CHECK(read_to_bytes_bridge.object_layout_hash ==
              layout_bytes_pair.object_layout_hash);
        CHECK(read_to_bytes_bridge.object_raw_track02_offset ==
              read_layout_pair_bridge.object_raw_track02_offset);
        CHECK(read_to_bytes_bridge.dungeon_track02_record ==
              read_layout_pair_bridge.dungeon_track02_record);
        CHECK(read_to_bytes_bridge.dungeon_layout_bytes ==
              read_layout_pair_bridge.dungeon_layout_bytes);
        CHECK(read_to_bytes_bridge.dungeon_layout_hash ==
              layout_bytes_pair.dungeon_layout_hash);
        CHECK(read_to_bytes_bridge.dungeon_raw_track02_offset ==
              read_layout_pair_bridge.dungeon_raw_track02_offset);
        CHECK(read_to_bytes_bridge.track02_md5 ==
              read_layout_pair_bridge.track02_md5);
        CHECK(strcmp(read_to_bytes_bridge.status,
                     "object_dungeon_read_to_bytes_bridge_bound_semantics_blocked_no_fallback") == 0);
        layout_bytes_pair.object_layout_hash = 0u;
        CHECK(!theron_v1_track02_loader_intake_bridge_read_layout_pair_to_bytes(
            &read_layout_pair_bridge, &layout_bytes_pair,
            &read_to_bytes_bridge));
        CHECK(!read_to_bytes_bridge.bridge_bound);
        layout_bytes_pair.object_layout_hash = object_layout_bytes.layout_hash;
        CHECK(theron_v1_track02_loader_intake_gate_object_dungeon_decoder_bytes(
            &raw_media_gate, &layout_bytes_pair, &decoder_gate));
        CHECK(decoder_gate.gate_evaluated);
        CHECK(decoder_gate.raw_media_gate_consumed);
        CHECK(decoder_gate.byte_pair_consumed);
        CHECK(decoder_gate.source_bytes_ready);
        CHECK(!decoder_gate.raw_media_missing_blocked);
        CHECK(decoder_gate.decoder_semantics_blocked);
        CHECK(decoder_gate.dungeon_grammar_blocked);
        CHECK(decoder_gate.runtime_handoff_blocked);
        CHECK(decoder_gate.rendering_blocked);
        CHECK(decoder_gate.fallback_visuals_blocked);
        CHECK(decoder_gate.no_synthetic_bytes);
        CHECK(decoder_gate.raw_track02_variant ==
              layout_bytes_pair.raw_track02_variant);
        CHECK(decoder_gate.object_track02_record ==
              layout_bytes_pair.object_track02_record);
        CHECK(decoder_gate.object_layout_bytes ==
              layout_bytes_pair.object_layout_bytes);
        CHECK(decoder_gate.object_layout_hash ==
              layout_bytes_pair.object_layout_hash);
        CHECK(decoder_gate.object_raw_track02_offset ==
              layout_bytes_pair.object_raw_track02_offset);
        CHECK(decoder_gate.dungeon_track02_record ==
              layout_bytes_pair.dungeon_track02_record);
        CHECK(decoder_gate.dungeon_layout_bytes ==
              layout_bytes_pair.dungeon_layout_bytes);
        CHECK(decoder_gate.dungeon_layout_hash ==
              layout_bytes_pair.dungeon_layout_hash);
        CHECK(decoder_gate.dungeon_raw_track02_offset ==
              layout_bytes_pair.dungeon_raw_track02_offset);
        CHECK(decoder_gate.track02_md5 == layout_bytes_pair.track02_md5);
        CHECK(strcmp(decoder_gate.status,
                     "object_dungeon_decoder_bytes_source_ready_semantics_blocked_no_fallback") == 0);
        CHECK(theron_v1_track02_loader_intake_bind_read_to_bytes_to_decoder_gate(
            &read_to_bytes_bridge, &decoder_gate, &read_to_decoder_gate));
        CHECK(read_to_decoder_gate.gate_bound);
        CHECK(read_to_decoder_gate.read_to_bytes_bridge_consumed);
        CHECK(read_to_decoder_gate.decoder_gate_consumed);
        CHECK(read_to_decoder_gate.source_bytes_ready);
        CHECK(!read_to_decoder_gate.raw_media_missing_blocked);
        CHECK(read_to_decoder_gate.same_track02_media);
        CHECK(read_to_decoder_gate.source_windows_preserved);
        CHECK(read_to_decoder_gate.byte_hashes_preserved);
        CHECK(read_to_decoder_gate.decoder_semantics_blocked);
        CHECK(read_to_decoder_gate.dungeon_grammar_blocked);
        CHECK(read_to_decoder_gate.runtime_handoff_blocked);
        CHECK(read_to_decoder_gate.rendering_blocked);
        CHECK(read_to_decoder_gate.fallback_visuals_blocked);
        CHECK(read_to_decoder_gate.no_synthetic_bytes);
        CHECK(read_to_decoder_gate.raw_track02_variant ==
              read_to_bytes_bridge.raw_track02_variant);
        CHECK(read_to_decoder_gate.object_track02_record ==
              read_to_bytes_bridge.object_track02_record);
        CHECK(read_to_decoder_gate.object_layout_bytes ==
              read_to_bytes_bridge.object_layout_bytes);
        CHECK(read_to_decoder_gate.object_layout_hash ==
              read_to_bytes_bridge.object_layout_hash);
        CHECK(read_to_decoder_gate.object_raw_track02_offset ==
              read_to_bytes_bridge.object_raw_track02_offset);
        CHECK(read_to_decoder_gate.dungeon_track02_record ==
              read_to_bytes_bridge.dungeon_track02_record);
        CHECK(read_to_decoder_gate.dungeon_layout_bytes ==
              read_to_bytes_bridge.dungeon_layout_bytes);
        CHECK(read_to_decoder_gate.dungeon_layout_hash ==
              read_to_bytes_bridge.dungeon_layout_hash);
        CHECK(read_to_decoder_gate.dungeon_raw_track02_offset ==
              read_to_bytes_bridge.dungeon_raw_track02_offset);
        CHECK(read_to_decoder_gate.track02_md5 ==
              read_to_bytes_bridge.track02_md5);
        CHECK(strcmp(read_to_decoder_gate.status,
                     "object_dungeon_read_to_decoder_gate_bound_semantics_blocked_no_fallback") == 0);
        decoder_gate.dungeon_layout_hash = 0u;
        CHECK(!theron_v1_track02_loader_intake_bind_read_to_bytes_to_decoder_gate(
            &read_to_bytes_bridge, &decoder_gate, &read_to_decoder_gate));
        CHECK(!read_to_decoder_gate.gate_bound);
        decoder_gate.dungeon_layout_hash = read_to_bytes_bridge.dungeon_layout_hash;
        CHECK(theron_v1_track02_loader_intake_record_object_dungeon_predecode_evidence(
            &decoder_gate, &predecode_evidence));
        CHECK(predecode_evidence.evidence_recorded);
        CHECK(predecode_evidence.decoder_gate_consumed);
        CHECK(predecode_evidence.source_bytes_ready);
        CHECK(!predecode_evidence.raw_media_missing_blocked);
        CHECK(predecode_evidence.same_track02_media);
        CHECK(predecode_evidence.non_overlapping_windows);
        CHECK(predecode_evidence.object_window_before_dungeon ||
              predecode_evidence.dungeon_window_before_object);
        CHECK(!(predecode_evidence.object_window_before_dungeon &&
                predecode_evidence.dungeon_window_before_object));
        CHECK(predecode_evidence.decoder_semantics_blocked);
        CHECK(predecode_evidence.dungeon_grammar_blocked);
        CHECK(predecode_evidence.runtime_handoff_blocked);
        CHECK(predecode_evidence.rendering_blocked);
        CHECK(predecode_evidence.fallback_visuals_blocked);
        CHECK(predecode_evidence.no_synthetic_bytes);
        CHECK(predecode_evidence.raw_track02_variant ==
              decoder_gate.raw_track02_variant);
        CHECK(predecode_evidence.object_track02_record ==
              decoder_gate.object_track02_record);
        CHECK(predecode_evidence.object_layout_bytes ==
              decoder_gate.object_layout_bytes);
        CHECK(predecode_evidence.object_layout_hash ==
              decoder_gate.object_layout_hash);
        CHECK(predecode_evidence.object_raw_track02_offset ==
              decoder_gate.object_raw_track02_offset);
        CHECK(predecode_evidence.dungeon_track02_record ==
              decoder_gate.dungeon_track02_record);
        CHECK(predecode_evidence.dungeon_layout_bytes ==
              decoder_gate.dungeon_layout_bytes);
        CHECK(predecode_evidence.dungeon_layout_hash ==
              decoder_gate.dungeon_layout_hash);
        CHECK(predecode_evidence.dungeon_raw_track02_offset ==
              decoder_gate.dungeon_raw_track02_offset);
        CHECK(predecode_evidence.total_span_bytes ==
              predecode_evidence.object_layout_bytes +
              predecode_evidence.dungeon_layout_bytes +
              predecode_evidence.gap_bytes);
        CHECK(predecode_evidence.predecode_evidence_hash != 0u);
        CHECK(predecode_evidence.track02_md5 == decoder_gate.track02_md5);
        CHECK(strcmp(predecode_evidence.status,
                     "object_dungeon_predecode_evidence_recorded_semantics_blocked_no_fallback") == 0);
        CHECK(theron_v1_track02_loader_intake_gate_object_dungeon_post_predecode(
            &read_to_decoder_gate, &predecode_evidence, &post_predecode_gate));
        CHECK(post_predecode_gate.readiness_bound);
        CHECK(post_predecode_gate.read_to_decoder_gate_consumed);
        CHECK(post_predecode_gate.predecode_evidence_consumed);
        CHECK(post_predecode_gate.topology_ready);
        CHECK(!post_predecode_gate.raw_media_missing_blocked);
        CHECK(post_predecode_gate.same_track02_media);
        CHECK(post_predecode_gate.source_windows_preserved);
        CHECK(post_predecode_gate.byte_hashes_preserved);
        CHECK(post_predecode_gate.topology_hash_preserved);
        CHECK(post_predecode_gate.decoder_semantics_blocked);
        CHECK(post_predecode_gate.dungeon_grammar_blocked);
        CHECK(post_predecode_gate.runtime_handoff_blocked);
        CHECK(post_predecode_gate.rendering_blocked);
        CHECK(post_predecode_gate.fallback_visuals_blocked);
        CHECK(post_predecode_gate.no_synthetic_bytes);
        CHECK(post_predecode_gate.raw_track02_variant ==
              read_to_decoder_gate.raw_track02_variant);
        CHECK(post_predecode_gate.object_track02_record ==
              read_to_decoder_gate.object_track02_record);
        CHECK(post_predecode_gate.object_layout_bytes ==
              read_to_decoder_gate.object_layout_bytes);
        CHECK(post_predecode_gate.object_layout_hash ==
              read_to_decoder_gate.object_layout_hash);
        CHECK(post_predecode_gate.object_raw_track02_offset ==
              read_to_decoder_gate.object_raw_track02_offset);
        CHECK(post_predecode_gate.dungeon_track02_record ==
              read_to_decoder_gate.dungeon_track02_record);
        CHECK(post_predecode_gate.dungeon_layout_bytes ==
              read_to_decoder_gate.dungeon_layout_bytes);
        CHECK(post_predecode_gate.dungeon_layout_hash ==
              read_to_decoder_gate.dungeon_layout_hash);
        CHECK(post_predecode_gate.dungeon_raw_track02_offset ==
              read_to_decoder_gate.dungeon_raw_track02_offset);
        CHECK(post_predecode_gate.gap_bytes == predecode_evidence.gap_bytes);
        CHECK(post_predecode_gate.total_span_bytes ==
              predecode_evidence.total_span_bytes);
        CHECK(post_predecode_gate.predecode_evidence_hash ==
              predecode_evidence.predecode_evidence_hash);
        CHECK(post_predecode_gate.track02_md5 == read_to_decoder_gate.track02_md5);
        CHECK(strcmp(post_predecode_gate.status,
                     "object_dungeon_post_predecode_topology_ready_semantics_blocked_no_fallback") == 0);
        CHECK(theron_v1_track02_loader_intake_gate_object_dungeon_level_handoff(
            &post_predecode_gate, &initial_envelope, &level_handoff_gate));
        CHECK(level_handoff_gate.level_handoff_bound);
        CHECK(level_handoff_gate.post_predecode_gate_consumed);
        CHECK(level_handoff_gate.initial_level_handoff_consumed);
        CHECK(!level_handoff_gate.raw_media_missing_blocked);
        CHECK(level_handoff_gate.same_track02_media);
        CHECK(level_handoff_gate.initial_level_source_locked);
        CHECK(level_handoff_gate.initial_level_boundary_opaque);
        CHECK(level_handoff_gate.topology_evidence_preserved);
        CHECK(level_handoff_gate.object_records_blocked);
        CHECK(level_handoff_gate.dungeon_records_blocked);
        CHECK(level_handoff_gate.decoder_semantics_blocked);
        CHECK(level_handoff_gate.dungeon_grammar_blocked);
        CHECK(level_handoff_gate.runtime_handoff_blocked);
        CHECK(level_handoff_gate.rendering_blocked);
        CHECK(level_handoff_gate.fallback_visuals_blocked);
        CHECK(level_handoff_gate.no_synthetic_bytes);
        CHECK(level_handoff_gate.raw_track02_variant ==
              post_predecode_gate.raw_track02_variant);
        CHECK(level_handoff_gate.initial_level_track02_record ==
              THERON_V1_INITIAL_ENVELOPE_RECORD);
        CHECK(level_handoff_gate.initial_level_user_data_offset ==
              THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET);
        CHECK(level_handoff_gate.initial_level_raw_track02_sector ==
              initial_envelope.track02_raw_sector);
        CHECK(level_handoff_gate.initial_level_raw_sector_offset ==
              initial_envelope.raw_sector_offset);
        CHECK(level_handoff_gate.initial_level_width ==
              THERON_V1_INITIAL_ENVELOPE_HEADER_WIDTH);
        CHECK(level_handoff_gate.initial_level_height ==
              THERON_V1_INITIAL_ENVELOPE_HEADER_HEIGHT);
        CHECK(level_handoff_gate.object_track02_record ==
              post_predecode_gate.object_track02_record);
        CHECK(level_handoff_gate.object_layout_bytes ==
              post_predecode_gate.object_layout_bytes);
        CHECK(level_handoff_gate.object_layout_hash ==
              post_predecode_gate.object_layout_hash);
        CHECK(level_handoff_gate.object_raw_track02_offset ==
              post_predecode_gate.object_raw_track02_offset);
        CHECK(level_handoff_gate.dungeon_track02_record ==
              post_predecode_gate.dungeon_track02_record);
        CHECK(level_handoff_gate.dungeon_layout_bytes ==
              post_predecode_gate.dungeon_layout_bytes);
        CHECK(level_handoff_gate.dungeon_layout_hash ==
              post_predecode_gate.dungeon_layout_hash);
        CHECK(level_handoff_gate.dungeon_raw_track02_offset ==
              post_predecode_gate.dungeon_raw_track02_offset);
        CHECK(level_handoff_gate.gap_bytes == post_predecode_gate.gap_bytes);
        CHECK(level_handoff_gate.total_span_bytes ==
              post_predecode_gate.total_span_bytes);
        CHECK(level_handoff_gate.predecode_evidence_hash ==
              post_predecode_gate.predecode_evidence_hash);
        CHECK(level_handoff_gate.track02_md5 == post_predecode_gate.track02_md5);
        CHECK(strcmp(level_handoff_gate.status,
                     "object_dungeon_level_handoff_bound_topology_preserved_runtime_blocked_no_fallback") == 0);
        CHECK(theron_v1_track02_loader_intake_gate_object_dungeon_grammar_admission(
            &level_handoff_gate, &grammar_gate));
        CHECK(grammar_gate.grammar_gate_evaluated);
        CHECK(grammar_gate.level_handoff_gate_consumed);
        CHECK(grammar_gate.source_topology_ready);
        CHECK(!grammar_gate.raw_media_missing_blocked);
        CHECK(grammar_gate.same_track02_media);
        CHECK(grammar_gate.original_cd_read_evidence_preserved);
        CHECK(grammar_gate.topology_evidence_preserved);
        CHECK(grammar_gate.object_table_grammar_required);
        CHECK(grammar_gate.dungeon_record_grammar_required);
        CHECK(!grammar_gate.object_table_grammar_admitted);
        CHECK(!grammar_gate.dungeon_record_grammar_admitted);
        CHECK(grammar_gate.decoder_semantics_blocked);
        CHECK(grammar_gate.runtime_handoff_blocked);
        CHECK(grammar_gate.rendering_blocked);
        CHECK(grammar_gate.fallback_visuals_blocked);
        CHECK(grammar_gate.no_synthetic_bytes);
        CHECK(grammar_gate.raw_track02_variant ==
              level_handoff_gate.raw_track02_variant);
        CHECK(grammar_gate.object_track02_record ==
              level_handoff_gate.object_track02_record);
        CHECK(grammar_gate.object_layout_bytes ==
              level_handoff_gate.object_layout_bytes);
        CHECK(grammar_gate.object_layout_hash ==
              level_handoff_gate.object_layout_hash);
        CHECK(grammar_gate.object_raw_track02_offset ==
              level_handoff_gate.object_raw_track02_offset);
        CHECK(grammar_gate.dungeon_track02_record ==
              level_handoff_gate.dungeon_track02_record);
        CHECK(grammar_gate.dungeon_layout_bytes ==
              level_handoff_gate.dungeon_layout_bytes);
        CHECK(grammar_gate.dungeon_layout_hash ==
              level_handoff_gate.dungeon_layout_hash);
        CHECK(grammar_gate.dungeon_raw_track02_offset ==
              level_handoff_gate.dungeon_raw_track02_offset);
        CHECK(grammar_gate.gap_bytes == level_handoff_gate.gap_bytes);
        CHECK(grammar_gate.total_span_bytes ==
              level_handoff_gate.total_span_bytes);
        CHECK(grammar_gate.predecode_evidence_hash ==
              level_handoff_gate.predecode_evidence_hash);
        CHECK(grammar_gate.track02_md5 == level_handoff_gate.track02_md5);
        CHECK(strcmp(grammar_gate.status,
                     "object_dungeon_grammar_admission_blocked_original_grammar_witness_required_no_fallback") == 0);
        CHECK(theron_v1_track02_loader_intake_bind_grammar_admission_to_loader_reads(
            &grammar_gate, &read_layout_binding, &grammar_read_gate));
        CHECK(grammar_read_gate.read_evidence_bound);
        CHECK(grammar_read_gate.grammar_gate_consumed);
        CHECK(grammar_read_gate.read_layout_binding_consumed);
        CHECK(!grammar_read_gate.raw_media_missing_blocked);
        CHECK(grammar_read_gate.same_track02_media);
        CHECK(grammar_read_gate.original_cd_read_destinations_preserved);
        CHECK(grammar_read_gate.layout_windows_preserved);
        CHECK(grammar_read_gate.topology_evidence_preserved);
        CHECK(grammar_read_gate.object_table_grammar_required);
        CHECK(grammar_read_gate.dungeon_record_grammar_required);
        CHECK(!grammar_read_gate.object_table_grammar_admitted);
        CHECK(!grammar_read_gate.dungeon_record_grammar_admitted);
        CHECK(grammar_read_gate.decoder_semantics_blocked);
        CHECK(grammar_read_gate.runtime_handoff_blocked);
        CHECK(grammar_read_gate.rendering_blocked);
        CHECK(grammar_read_gate.fallback_visuals_blocked);
        CHECK(grammar_read_gate.no_synthetic_bytes);
        CHECK(grammar_read_gate.raw_track02_variant ==
              grammar_gate.raw_track02_variant);
        CHECK(grammar_read_gate.object_track02_record ==
              read_layout_binding.object_track02_record);
        CHECK(grammar_read_gate.object_record_user_data_offset ==
              read_layout_binding.object_record_user_data_offset);
        CHECK(grammar_read_gate.object_destination ==
              read_layout_binding.object_destination);
        CHECK(grammar_read_gate.object_layout_bytes ==
              grammar_gate.object_layout_bytes);
        CHECK(grammar_read_gate.object_layout_hash ==
              grammar_gate.object_layout_hash);
        CHECK(grammar_read_gate.object_raw_track02_offset ==
              grammar_gate.object_raw_track02_offset);
        CHECK(grammar_read_gate.dungeon_track02_record ==
              read_layout_binding.dungeon_track02_record);
        CHECK(grammar_read_gate.dungeon_record_user_data_offset ==
              read_layout_binding.dungeon_record_user_data_offset);
        CHECK(grammar_read_gate.dungeon_destination ==
              read_layout_binding.dungeon_destination);
        CHECK(grammar_read_gate.dungeon_layout_bytes ==
              grammar_gate.dungeon_layout_bytes);
        CHECK(grammar_read_gate.dungeon_layout_hash ==
              grammar_gate.dungeon_layout_hash);
        CHECK(grammar_read_gate.dungeon_raw_track02_offset ==
              grammar_gate.dungeon_raw_track02_offset);
        CHECK(grammar_read_gate.predecode_evidence_hash ==
              grammar_gate.predecode_evidence_hash);
        CHECK(grammar_read_gate.track02_md5 == grammar_gate.track02_md5);
        CHECK(strcmp(grammar_read_gate.status,
                     "object_dungeon_grammar_read_evidence_bound_original_loader_reads_grammar_blocked_no_fallback") == 0);
        memset(&parser_witness, 0, sizeof(parser_witness));
        parser_witness.original_loader_trace = 1;
        parser_witness.original_parser_trace = 1;
        parser_witness.object_table_parser_entered = 1;
        parser_witness.dungeon_record_parser_entered = 1;
        parser_witness.no_fallback_visuals = 1;
        parser_witness.no_synthetic_bytes = 1;
        parser_witness.raw_track02_variant =
            grammar_read_gate.raw_track02_variant;
        parser_witness.object_track02_record =
            grammar_read_gate.object_track02_record;
        parser_witness.object_record_user_data_offset =
            grammar_read_gate.object_record_user_data_offset;
        parser_witness.object_destination =
            grammar_read_gate.object_destination;
        parser_witness.object_byte_count =
            grammar_read_gate.object_layout_bytes;
        parser_witness.object_raw_track02_offset =
            grammar_read_gate.object_raw_track02_offset;
        parser_witness.dungeon_track02_record =
            grammar_read_gate.dungeon_track02_record;
        parser_witness.dungeon_record_user_data_offset =
            grammar_read_gate.dungeon_record_user_data_offset;
        parser_witness.dungeon_destination =
            grammar_read_gate.dungeon_destination;
        parser_witness.dungeon_byte_count =
            grammar_read_gate.dungeon_layout_bytes;
        parser_witness.dungeon_raw_track02_offset =
            grammar_read_gate.dungeon_raw_track02_offset;
        parser_witness.track02_md5 = grammar_read_gate.track02_md5;
        CHECK(theron_v1_track02_loader_intake_admit_object_dungeon_parser_witness(
            &grammar_read_gate, &parser_witness, &parser_witness_receipt));
        CHECK(parser_witness_receipt.parser_witness_bound);
        CHECK(parser_witness_receipt.grammar_read_evidence_consumed);
        CHECK(parser_witness_receipt.original_loader_trace_consumed);
        CHECK(parser_witness_receipt.original_parser_trace_consumed);
        CHECK(!parser_witness_receipt.raw_media_missing_blocked);
        CHECK(parser_witness_receipt.same_track02_media);
        CHECK(parser_witness_receipt.object_table_parser_witnessed);
        CHECK(parser_witness_receipt.dungeon_record_parser_witnessed);
        CHECK(parser_witness_receipt.object_table_grammar_admitted);
        CHECK(parser_witness_receipt.dungeon_record_grammar_admitted);
        CHECK(parser_witness_receipt.object_table_fields_blocked);
        CHECK(parser_witness_receipt.dungeon_record_fields_blocked);
        CHECK(parser_witness_receipt.decoder_semantics_blocked);
        CHECK(parser_witness_receipt.runtime_handoff_blocked);
        CHECK(parser_witness_receipt.rendering_blocked);
        CHECK(parser_witness_receipt.fallback_visuals_blocked);
        CHECK(parser_witness_receipt.no_synthetic_bytes);
        CHECK(parser_witness_receipt.raw_track02_variant ==
              grammar_read_gate.raw_track02_variant);
        CHECK(parser_witness_receipt.object_track02_record ==
              grammar_read_gate.object_track02_record);
        CHECK(parser_witness_receipt.object_record_user_data_offset ==
              grammar_read_gate.object_record_user_data_offset);
        CHECK(parser_witness_receipt.object_destination ==
              grammar_read_gate.object_destination);
        CHECK(parser_witness_receipt.object_layout_bytes ==
              grammar_read_gate.object_layout_bytes);
        CHECK(parser_witness_receipt.object_layout_hash ==
              grammar_read_gate.object_layout_hash);
        CHECK(parser_witness_receipt.object_raw_track02_offset ==
              grammar_read_gate.object_raw_track02_offset);
        CHECK(parser_witness_receipt.dungeon_track02_record ==
              grammar_read_gate.dungeon_track02_record);
        CHECK(parser_witness_receipt.dungeon_record_user_data_offset ==
              grammar_read_gate.dungeon_record_user_data_offset);
        CHECK(parser_witness_receipt.dungeon_destination ==
              grammar_read_gate.dungeon_destination);
        CHECK(parser_witness_receipt.dungeon_layout_bytes ==
              grammar_read_gate.dungeon_layout_bytes);
        CHECK(parser_witness_receipt.dungeon_layout_hash ==
              grammar_read_gate.dungeon_layout_hash);
        CHECK(parser_witness_receipt.dungeon_raw_track02_offset ==
              grammar_read_gate.dungeon_raw_track02_offset);
        CHECK(parser_witness_receipt.predecode_evidence_hash ==
              grammar_read_gate.predecode_evidence_hash);
        CHECK(parser_witness_receipt.track02_md5 == grammar_read_gate.track02_md5);
        CHECK(strcmp(parser_witness_receipt.status,
                     "object_dungeon_parser_witness_bound_grammar_proven_fields_runtime_render_blocked") == 0);
        parser_witness.original_parser_trace = 0;
        CHECK(!theron_v1_track02_loader_intake_admit_object_dungeon_parser_witness(
            &grammar_read_gate, &parser_witness, &parser_witness_receipt));
        CHECK(!parser_witness_receipt.parser_witness_bound);
        parser_witness.original_parser_trace = 1;
        parser_witness.object_destination++;
        CHECK(!theron_v1_track02_loader_intake_admit_object_dungeon_parser_witness(
            &grammar_read_gate, &parser_witness, &parser_witness_receipt));
        CHECK(!parser_witness_receipt.parser_witness_bound);
        parser_witness.object_destination =
            grammar_read_gate.object_destination;
        parser_witness.dungeon_byte_count++;
        CHECK(!theron_v1_track02_loader_intake_admit_object_dungeon_parser_witness(
            &grammar_read_gate, &parser_witness, &parser_witness_receipt));
        CHECK(!parser_witness_receipt.parser_witness_bound);
        parser_witness.dungeon_byte_count =
            grammar_read_gate.dungeon_layout_bytes;
        read_layout_binding.dungeon_destination =
            read_layout_binding.object_destination;
        CHECK(!theron_v1_track02_loader_intake_bind_grammar_admission_to_loader_reads(
            &grammar_gate, &read_layout_binding, &grammar_read_gate));
        CHECK(!grammar_read_gate.read_evidence_bound);
        read_layout_binding.dungeon_destination =
            dungeon_layout.destination;
        read_layout_binding.object_layout_bytes =
            grammar_gate.object_layout_bytes + 1u;
        CHECK(!theron_v1_track02_loader_intake_bind_grammar_admission_to_loader_reads(
            &grammar_gate, &read_layout_binding, &grammar_read_gate));
        CHECK(!grammar_read_gate.read_evidence_bound);
        read_layout_binding.object_layout_bytes =
            grammar_gate.object_layout_bytes;
        grammar_gate.object_table_grammar_admitted = 1;
        CHECK(!theron_v1_track02_loader_intake_bind_grammar_admission_to_loader_reads(
            &grammar_gate, &read_layout_binding, &grammar_read_gate));
        CHECK(!grammar_read_gate.read_evidence_bound);
        grammar_gate.object_table_grammar_admitted = 0;
        level_handoff_gate.predecode_evidence_hash = 0u;
        CHECK(!theron_v1_track02_loader_intake_gate_object_dungeon_grammar_admission(
            &level_handoff_gate, &grammar_gate));
        CHECK(!grammar_gate.grammar_gate_evaluated);
        level_handoff_gate.predecode_evidence_hash =
            post_predecode_gate.predecode_evidence_hash;
        level_handoff_gate.dungeon_layout_hash = 0u;
        CHECK(!theron_v1_track02_loader_intake_gate_object_dungeon_grammar_admission(
            &level_handoff_gate, &grammar_gate));
        CHECK(!grammar_gate.grammar_gate_evaluated);
        level_handoff_gate.dungeon_layout_hash =
            post_predecode_gate.dungeon_layout_hash;
        level_handoff_gate.fallback_visuals_blocked = 0;
        CHECK(!theron_v1_track02_loader_intake_gate_object_dungeon_grammar_admission(
            &level_handoff_gate, &grammar_gate));
        CHECK(!grammar_gate.grammar_gate_evaluated);
        level_handoff_gate.fallback_visuals_blocked = 1;
        initial_envelope.adjacent_boundary_opaque = 0;
        CHECK(!theron_v1_track02_loader_intake_gate_object_dungeon_level_handoff(
            &post_predecode_gate, &initial_envelope, &level_handoff_gate));
        CHECK(!level_handoff_gate.level_handoff_bound);
        initial_envelope.adjacent_boundary_opaque = 1;
        initial_envelope.raw_track02_variant = THERON_V1_TRACK02_VARIANT_NONE;
        CHECK(!theron_v1_track02_loader_intake_gate_object_dungeon_level_handoff(
            &post_predecode_gate, &initial_envelope, &level_handoff_gate));
        CHECK(!level_handoff_gate.level_handoff_bound);
        initial_envelope.raw_track02_variant =
            post_predecode_gate.raw_track02_variant;
        post_predecode_gate.predecode_evidence_hash = 0u;
        CHECK(!theron_v1_track02_loader_intake_gate_object_dungeon_level_handoff(
            &post_predecode_gate, &initial_envelope, &level_handoff_gate));
        CHECK(!level_handoff_gate.level_handoff_bound);
        post_predecode_gate.predecode_evidence_hash =
            predecode_evidence.predecode_evidence_hash;
        predecode_evidence.predecode_evidence_hash = 0u;
        CHECK(!theron_v1_track02_loader_intake_gate_object_dungeon_post_predecode(
            &read_to_decoder_gate, &predecode_evidence, &post_predecode_gate));
        CHECK(!post_predecode_gate.readiness_bound);
        predecode_evidence.predecode_evidence_hash = 1u;
        predecode_evidence.object_raw_track02_offset =
            read_to_decoder_gate.object_raw_track02_offset + 1u;
        CHECK(!theron_v1_track02_loader_intake_gate_object_dungeon_post_predecode(
            &read_to_decoder_gate, &predecode_evidence, &post_predecode_gate));
        CHECK(!post_predecode_gate.readiness_bound);
        predecode_evidence.object_raw_track02_offset =
            read_to_decoder_gate.object_raw_track02_offset;
        decoder_gate.decoder_semantics_blocked = 0;
        CHECK(!theron_v1_track02_loader_intake_record_object_dungeon_predecode_evidence(
            &decoder_gate, &predecode_evidence));
        CHECK(!predecode_evidence.evidence_recorded);
        decoder_gate.decoder_semantics_blocked = 1;
        decoder_gate.object_layout_hash = 0u;
        CHECK(!theron_v1_track02_loader_intake_record_object_dungeon_predecode_evidence(
            &decoder_gate, &predecode_evidence));
        CHECK(!predecode_evidence.evidence_recorded);
        decoder_gate.object_layout_hash = layout_bytes_pair.object_layout_hash;
        layout_bytes_pair.fallback_visuals_blocked = 0;
        CHECK(!theron_v1_track02_loader_intake_gate_object_dungeon_decoder_bytes(
            &raw_media_gate, &layout_bytes_pair, &decoder_gate));
        CHECK(!decoder_gate.gate_evaluated);
        layout_bytes_pair.fallback_visuals_blocked = 1;
        raw_media_gate.raw_track02_bytes =
            layout_bytes_pair.dungeon_raw_track02_offset;
        CHECK(!theron_v1_track02_loader_intake_gate_object_dungeon_decoder_bytes(
            &raw_media_gate, &layout_bytes_pair, &decoder_gate));
        CHECK(!decoder_gate.gate_evaluated);
        raw_media_gate.raw_track02_bytes = raw_cue_receipt.raw_track02_bytes;
        CHECK(!theron_v1_track02_loader_intake_bind_object_dungeon_layout_bytes_pair(
            &layout_pair, &dungeon_layout_bytes, &object_layout_bytes,
            &layout_bytes_pair));
        CHECK(!layout_bytes_pair.byte_pair_bound);
        dungeon_layout_bytes.rendering_blocked = 0;
        CHECK(!theron_v1_track02_loader_intake_bind_object_dungeon_layout_bytes_pair(
            &layout_pair, &object_layout_bytes, &dungeon_layout_bytes,
            &layout_bytes_pair));
        CHECK(!layout_bytes_pair.byte_pair_bound);
        dungeon_layout_bytes = layout_bytes_receipt;
        dungeon_layout_bytes.layout_hash = 0u;
        CHECK(!theron_v1_track02_loader_intake_bind_object_dungeon_layout_bytes_pair(
            &layout_pair, &object_layout_bytes, &dungeon_layout_bytes,
            &layout_bytes_pair));
        CHECK(!layout_bytes_pair.byte_pair_bound);
        dungeon_layout_bytes = layout_bytes_receipt;
        CHECK(!theron_v1_track02_loader_intake_handoff_later_layout_bytes(
            &dungeon_layout, real_track02, real_track02_bytes, real_track02_md5,
            layout_bytes, dungeon_layout.layout_bytes - 1u,
            &layout_bytes_receipt));
        CHECK(!layout_bytes_receipt.bytes_handed_off);
        CHECK(!layout_bytes_receipt.rendering_blocked);
        CHECK(!theron_v1_track02_loader_intake_handoff_later_layout_bytes(
            &object_layout, real_track02, real_track02_bytes,
            THERON_V1_TRACK02_MD5_JP_BIN, layout_bytes, sizeof(layout_bytes),
            &layout_bytes_receipt));
        CHECK(!layout_bytes_receipt.bytes_handed_off);
        CHECK(!theron_v1_track02_loader_intake_bind_object_dungeon_layout_pair(
            &dungeon_layout, &object_layout, &layout_pair));
        CHECK(!layout_pair.layout_pair_bound);
        dungeon_layout.raw_track02_offset = object_layout.raw_track02_offset;
        CHECK(!theron_v1_track02_loader_intake_bind_object_dungeon_layout_pair(
            &object_layout, &dungeon_layout, &layout_pair));
        CHECK(!layout_pair.layout_pair_bound);
        dungeon_layout = later_layout;
        dungeon_layout.fallback_visuals_blocked = 0;
        CHECK(!theron_v1_track02_loader_intake_bind_object_dungeon_layout_pair(
            &object_layout, &dungeon_layout, &layout_pair));
        CHECK(!layout_pair.layout_pair_bound);
        dungeon_layout = later_layout;
        CHECK(!theron_v1_track02_loader_intake_admit_later_cd_record_read(
            &raw_cue_receipt, &facts, &later_cd_record));
        CHECK(!later_cd_record.cd_record_read_proven);
        CHECK(later_cd_record.startup_record_rejected);
        CHECK(!later_cd_record.no_fallback_visuals);
        CHECK(!theron_v1_track02_loader_intake_bind_later_read_layout(
            &later_cd_record, THERON_V1_TRACK02_LAYOUT_ROLE_DUNGEON_RECORD,
            &later_layout));
        CHECK(!later_layout.layout_window_bound);
        CHECK(!later_layout.rendering_blocked);
        CHECK(!theron_v1_track02_loader_intake_bind_object_dungeon_layout_pair(
            &object_layout, &later_layout, &layout_pair));
        CHECK(!layout_pair.layout_pair_bound);
        ++grid_receipt.raw_grid[0];
        CHECK(!theron_v1_track02_loader_intake_block_raw_grid_bitmap_route(
            &grid_receipt, &bitmap_route));
        CHECK(!bitmap_route.bitmap_route_blocked);
        CHECK(!bitmap_route.no_fallback);
        CHECK(!theron_v1_track02_loader_intake_admit_raw_grid_level_route(
            &grid_receipt, &level_route));
        CHECK(!level_route.level_route_admitted);
        CHECK(!level_route.no_fallback);
        CHECK(!theron_v1_track02_loader_intake_admit_raw_grid_dungeon_route(
            &grid_receipt, &dungeon_route));
        CHECK(!dungeon_route.dungeon_route_admitted);
        CHECK(!dungeon_route.no_fallback);
        CHECK(!theron_v1_track02_loader_intake_block_raw_grid_dungeon_record_evidence(
            &grid_receipt, &dungeon_record));
        CHECK(!dungeon_record.dungeon_record_blocked);
        CHECK(!dungeon_record.no_fallback);
        CHECK(!theron_v1_track02_loader_intake_block_object_table_read_claim(
            &grid_receipt, &facts, &object_read_block));
        CHECK(!object_read_block.object_table_read_blocked);
        CHECK(!object_read_block.no_fallback);
        CHECK(!theron_v1_track02_loader_intake_block_bitmap_read_claim(
            &grid_receipt, &facts, &bitmap_read_block));
        CHECK(!bitmap_read_block.bitmap_read_blocked);
        CHECK(!bitmap_read_block.no_fallback_visual);
        CHECK(!theron_v1_track02_loader_intake_block_dungeon_read_claim(
            &grid_receipt, &facts, &dungeon_read_block));
        CHECK(!dungeon_read_block.dungeon_read_blocked);
        CHECK(!dungeon_read_block.no_fallback_dungeon);
        CHECK(!theron_v1_track02_loader_intake_gate_object_dungeon_handoff(
            &grid_receipt, &separate_object_facts, &separate_dungeon_facts,
            &object_dungeon_gate));
        CHECK(!object_dungeon_gate.handoff_blocked);
        CHECK(!object_dungeon_gate.no_fallback_visuals);
        CHECK(!object_dungeon_gate.no_synthetic_handoff);
        raw_cue_receipt.raw_track02_bytes =
            later_cd_record.raw_track02_offset;
        CHECK(!theron_v1_track02_loader_intake_admit_later_cd_record_read(
            &raw_cue_receipt, &separate_object_facts, &later_cd_record));
        CHECK(!later_cd_record.cd_record_read_proven);
        CHECK(!theron_v1_track02_loader_intake_bind_later_read_layout(
            &later_cd_record, THERON_V1_TRACK02_LAYOUT_ROLE_OBJECT_TABLE,
            &later_layout));
        CHECK(!later_layout.layout_window_bound);
        CHECK(!theron_v1_track02_loader_intake_bind_object_dungeon_layout_pair(
            &later_layout, &dungeon_layout, &layout_pair));
        CHECK(!layout_pair.layout_pair_bound);
        CHECK(!theron_v1_track02_loader_intake_handoff_later_layout_bytes(
            &later_layout, real_track02, real_track02_bytes, real_track02_md5,
            layout_bytes, sizeof(layout_bytes), &layout_bytes_receipt));
        CHECK(!layout_bytes_receipt.bytes_handed_off);
        CHECK(!theron_v1_track02_loader_intake_bind_object_dungeon_layout_bytes_pair(
            &layout_pair, &layout_bytes_receipt, &dungeon_layout_bytes,
            &layout_bytes_pair));
        CHECK(!layout_bytes_pair.byte_pair_bound);
        raw_cue_receipt.raw_track02_bytes = real_track02_bytes;
        --grid_receipt.raw_grid[0];
        consumer.accept = 1;
        CHECK(theron_v1_track02_loader_intake_deliver_raw_grid_to_runtime(
            &decoded_receipt, real_track02, real_track02_bytes,
            real_track02_md5, capture_runtime_grid, &consumer,
            &runtime_receipt));
        CHECK(runtime_receipt.delivered && runtime_receipt.no_fallback);
        CHECK(runtime_receipt.authenticated_v3_trace);
        CHECK(runtime_receipt.raw_grid_bytes == grid_receipt.raw_grid_bytes);
        CHECK(runtime_receipt.raw_grid_hash == grid_receipt.raw_grid_hash);
        CHECK(runtime_receipt.raw_track02_offset == grid_receipt.raw_track02_offset);
        CHECK(strcmp(runtime_receipt.status,
                     "initial_envelope_raw_grid_delivered_no_fallback") == 0);
        CHECK(consumer.calls == 1);
        CHECK(consumer.grid_hash == grid_receipt.raw_grid_hash);
        CHECK(consumer.grid_bytes == grid_receipt.raw_grid_bytes);
        CHECK(consumer.first_byte == grid_receipt.raw_grid[0]);
        CHECK(consumer.last_byte == grid_receipt.raw_grid[
            grid_receipt.raw_grid_bytes - 1u]);
        consumer.accept = 0;
        CHECK(!theron_v1_track02_loader_intake_deliver_raw_grid_to_runtime(
            &decoded_receipt, real_track02, real_track02_bytes,
            real_track02_md5, capture_runtime_grid, &consumer,
            &runtime_receipt));
        CHECK(!runtime_receipt.delivered && !runtime_receipt.no_fallback);
        CHECK(runtime_receipt.status == NULL);
        CHECK(consumer.calls == 2);
        CHECK(theron_v1_track02_loader_intake_handoff_raw_grid_row(
            &decoded_receipt, real_track02, real_track02_bytes,
            real_track02_md5, 0u, &row_receipt));
        CHECK(row_receipt.handed_off);
        CHECK(row_receipt.raw_grid_y == 0u);
        CHECK(row_receipt.raw_grid_bytes == 0x20u);
        if (is_us) {
            CHECK(row_receipt.raw_grid_row[0] == 0x84u);
            CHECK(row_receipt.raw_grid_row[31] == 0x56u);
        }
        CHECK(row_receipt.raw_track02_sector == initial_envelope.track02_raw_sector);
        CHECK(row_receipt.raw_sector_offset == 0x130u);
        CHECK(row_receipt.raw_track02_offset == 0x7015c0u);
        CHECK(row_receipt.raw_grid_row_hash == 0x4b97e3abu);
        CHECK(strcmp(row_receipt.status,
                     "initial_envelope_raw_grid_row_handoff_no_semantics") == 0);
        CHECK(theron_v1_track02_loader_intake_handoff_raw_grid_row(
            &decoded_receipt, real_track02, real_track02_bytes,
            real_track02_md5, 26u, &row_receipt));
        CHECK(row_receipt.raw_sector_offset == 0x470u);
        if (is_us) {
            CHECK(row_receipt.raw_grid_row[0] == 0u);
            CHECK(row_receipt.raw_grid_row_hash == 0x0b2ae445u);
        }
        CHECK(!theron_v1_track02_loader_intake_handoff_raw_grid_row(
            &decoded_receipt, real_track02, real_track02_bytes,
            real_track02_md5, 27u, &row_receipt));
        CHECK(!row_receipt.handed_off);
        CHECK(row_receipt.status == NULL);
        CHECK(theron_v1_track02_loader_intake_handoff_raw_grid_coordinate(
            &decoded_receipt, real_track02, real_track02_bytes,
            real_track02_md5, 0u, 0u, &coordinate_receipt));
        CHECK(coordinate_receipt.handed_off);
        if (is_us) CHECK(coordinate_receipt.raw_grid_byte == 0x84u);
        CHECK(coordinate_receipt.raw_track02_sector ==
              initial_envelope.track02_raw_sector);
        CHECK(coordinate_receipt.raw_sector_offset == 0x130u);
        CHECK(coordinate_receipt.raw_track02_offset == 0x7015c0u);
        CHECK(strcmp(coordinate_receipt.status,
                     "initial_envelope_raw_grid_coordinate_handoff_no_semantics") == 0);
        CHECK(theron_v1_track02_loader_intake_handoff_raw_grid_coordinate(
            &decoded_receipt, real_track02, real_track02_bytes,
            real_track02_md5, 31u, 0u, &coordinate_receipt));
        if (is_us) CHECK(coordinate_receipt.raw_grid_byte == 0x56u);
        CHECK(coordinate_receipt.raw_sector_offset == 0x14fu);
        CHECK(theron_v1_track02_loader_intake_handoff_raw_grid_coordinate(
            &decoded_receipt, real_track02, real_track02_bytes,
            real_track02_md5, 0u, 26u, &coordinate_receipt));
        if (is_us) CHECK(coordinate_receipt.raw_grid_byte == 0u);
        CHECK(coordinate_receipt.raw_sector_offset == 0x470u);
        CHECK(!theron_v1_track02_loader_intake_handoff_raw_grid_coordinate(
            &decoded_receipt, real_track02, real_track02_bytes,
            real_track02_md5, 32u, 0u, &coordinate_receipt));
        CHECK(!coordinate_receipt.handed_off);
        CHECK(coordinate_receipt.status == NULL);
        ++decoded_receipt.decoded_grid_hash;
        CHECK(!theron_v1_track02_loader_intake_handoff_raw_grid_coordinate(
            &decoded_receipt, real_track02, real_track02_bytes,
            real_track02_md5, 0u, 0u, &coordinate_receipt));
        CHECK(!coordinate_receipt.handed_off);
        CHECK(coordinate_receipt.status == NULL);
        --decoded_receipt.decoded_grid_hash;
        ++decoded_receipt.decoded_grid_row_bytes;
        CHECK(!theron_v1_track02_loader_intake_handoff_raw_grid_row(
            &decoded_receipt, real_track02, real_track02_bytes,
            real_track02_md5, 0u, &row_receipt));
        CHECK(!row_receipt.handed_off);
        CHECK(row_receipt.status == NULL);
        --decoded_receipt.decoded_grid_row_bytes;
        ++decoded_receipt.decoded_grid_row_count;
        CHECK(!theron_v1_track02_loader_intake_handoff_raw_grid(
            &decoded_receipt, real_track02, real_track02_bytes,
            real_track02_md5, &grid_receipt));
        CHECK(!grid_receipt.handed_off);
        CHECK(grid_receipt.status == NULL);
        --decoded_receipt.decoded_grid_row_count;
        ++decoded_receipt.decoded_grid_raw_sector;
        CHECK(!theron_v1_track02_loader_intake_handoff_raw_grid_coordinate(
            &decoded_receipt, real_track02, real_track02_bytes,
            real_track02_md5, 0u, 0u, &coordinate_receipt));
        CHECK(!coordinate_receipt.handed_off);
        CHECK(coordinate_receipt.status == NULL);
        free(real_track02);
        initial_envelope.cue_track02_index01_raw_sector = 225u;
        initial_envelope.track02_raw_sector = 3123u;
    }

    /* A header-shaped local fixture cannot substitute for the canonical BIN. */
    synthetic_raw_bytes = ((size_t)initial_envelope.track02_raw_sector + 1u) *
        THERON_V1_TRACK02_RAW_SECTOR_BYTES;
    synthetic_raw = calloc(1u, synthetic_raw_bytes);
    CHECK(synthetic_raw != NULL);
    if (synthetic_raw) {
        size_t raw_offset = (size_t)initial_envelope.track02_raw_sector *
            THERON_V1_TRACK02_RAW_SECTOR_BYTES + initial_envelope.raw_sector_offset;
        unsigned char header[] = {
            0x00, 0x20, 0x00, 0x1b, 0x01, 0x08,
            0xe9, 0x38, 0x00, 0x26, 0x01, 0x03
        };

        memcpy(synthetic_raw + raw_offset, header, sizeof(header));
        CHECK(!theron_v1_track02_loader_intake_decode_initial_envelope(
            &receipt, &initial_envelope, synthetic_raw, synthetic_raw_bytes,
            THERON_V1_TRACK02_MD5_US_BIN, &decoded_receipt));
        CHECK(!decoded_receipt.payload_intake_admitted);
        CHECK(!decoded_receipt.initial_envelope_decoded);
        CHECK(decoded_receipt.status == NULL);
        CHECK(!theron_v1_track02_loader_intake_handoff_raw_grid_row(
            &receipt, synthetic_raw, synthetic_raw_bytes,
            THERON_V1_TRACK02_MD5_US_BIN, 0u, &row_receipt));
        CHECK(!row_receipt.handed_off);
        CHECK(row_receipt.status == NULL);
        CHECK(!theron_v1_track02_loader_intake_handoff_raw_grid(
            &receipt, synthetic_raw, synthetic_raw_bytes,
            THERON_V1_TRACK02_MD5_US_BIN, &grid_receipt));
        CHECK(!grid_receipt.handed_off);
        CHECK(grid_receipt.status == NULL);
        CHECK(!theron_v1_track02_loader_intake_block_raw_grid_object_table_projection(
            &grid_receipt, &object_projection));
        CHECK(!object_projection.projection_blocked);
        CHECK(!object_projection.no_fallback);
        CHECK(!theron_v1_track02_loader_intake_block_object_table_read_claim(
            &grid_receipt, &facts, &object_read_block));
        CHECK(!object_read_block.object_table_read_blocked);
        CHECK(!object_read_block.no_fallback);
        CHECK(!theron_v1_track02_loader_intake_block_raw_grid_bitmap_route(
            &grid_receipt, &bitmap_route));
        CHECK(!bitmap_route.bitmap_route_blocked);
        CHECK(!bitmap_route.no_fallback);
        CHECK(!theron_v1_track02_loader_intake_block_bitmap_read_claim(
            &grid_receipt, &facts, &bitmap_read_block));
        CHECK(!bitmap_read_block.bitmap_read_blocked);
        CHECK(!bitmap_read_block.no_fallback_visual);
        CHECK(!theron_v1_track02_loader_intake_block_raw_grid_dungeon_record_evidence(
            &grid_receipt, &dungeon_record));
        CHECK(!dungeon_record.dungeon_record_blocked);
        CHECK(!dungeon_record.no_fallback);
        CHECK(!theron_v1_track02_loader_intake_block_dungeon_read_claim(
            &grid_receipt, &facts, &dungeon_read_block));
        CHECK(!dungeon_read_block.dungeon_read_blocked);
        CHECK(!dungeon_read_block.no_fallback_dungeon);
        CHECK(!theron_v1_track02_loader_intake_gate_object_dungeon_handoff(
            &grid_receipt, &separate_object_facts, &separate_dungeon_facts,
            &object_dungeon_gate));
        CHECK(!object_dungeon_gate.handoff_blocked);
        CHECK(!object_dungeon_gate.no_fallback_visuals);
        CHECK(!theron_v1_track02_loader_intake_admit_later_cd_record_read(
            &raw_cue_receipt, &separate_object_facts, &later_cd_record));
        CHECK(!later_cd_record.cd_record_read_proven);
        CHECK(!theron_v1_track02_loader_intake_bind_later_read_layout(
            &later_cd_record, THERON_V1_TRACK02_LAYOUT_ROLE_OBJECT_TABLE,
            &later_layout));
        CHECK(!later_layout.layout_window_bound);
        CHECK(receipt.initial_envelope_source_bound);
        free(synthetic_raw);
    }

    memset(&grid_receipt, 0, sizeof(grid_receipt));
    grid_receipt.handed_off = 1;
    grid_receipt.authenticated_v3_trace = 0;
    grid_receipt.raw_grid_width = THERON_V1_INITIAL_ENVELOPE_HEADER_WIDTH;
    grid_receipt.raw_grid_height = THERON_V1_INITIAL_ENVELOPE_HEADER_HEIGHT;
    grid_receipt.raw_grid_bytes =
        THERON_V1_INITIAL_ENVELOPE_HEADER_WIDTH *
        THERON_V1_INITIAL_ENVELOPE_HEADER_HEIGHT;
    grid_receipt.raw_grid_hash = 0x12345678u;
    grid_receipt.raw_track02_sector = initial_envelope.track02_raw_sector;
    grid_receipt.raw_sector_offset = 0x130u;
    grid_receipt.raw_track02_offset = 0x7015c0u;
    grid_receipt.status = "initial_envelope_raw_grid_handoff_no_semantics";
    CHECK(!theron_v1_track02_loader_intake_block_raw_grid_object_table_projection(
        &grid_receipt, &object_projection));
    CHECK(!object_projection.projection_blocked);
    CHECK(!object_projection.no_fallback);
    CHECK(!theron_v1_track02_loader_intake_block_object_table_read_claim(
        &grid_receipt, &facts, &object_read_block));
    CHECK(!object_read_block.object_table_read_blocked);
    CHECK(!object_read_block.no_fallback);
    CHECK(!theron_v1_track02_loader_intake_block_raw_grid_bitmap_route(
        &grid_receipt, &bitmap_route));
    CHECK(!bitmap_route.bitmap_route_blocked);
    CHECK(!bitmap_route.no_fallback);
    CHECK(!theron_v1_track02_loader_intake_block_bitmap_read_claim(
        &grid_receipt, &facts, &bitmap_read_block));
    CHECK(!bitmap_read_block.bitmap_read_blocked);
    CHECK(!bitmap_read_block.no_fallback_visual);
    CHECK(!theron_v1_track02_loader_intake_admit_raw_grid_level_route(
        &grid_receipt, &level_route));
    CHECK(!level_route.level_route_admitted);
    CHECK(!level_route.no_fallback);
    CHECK(!theron_v1_track02_loader_intake_admit_raw_grid_dungeon_route(
        &grid_receipt, &dungeon_route));
    CHECK(!dungeon_route.dungeon_route_admitted);
    CHECK(!dungeon_route.no_fallback);
    CHECK(!theron_v1_track02_loader_intake_block_raw_grid_dungeon_record_evidence(
        &grid_receipt, &dungeon_record));
    CHECK(!dungeon_record.dungeon_record_blocked);
    CHECK(!dungeon_record.no_fallback);
    CHECK(!theron_v1_track02_loader_intake_block_dungeon_read_claim(
        &grid_receipt, &facts, &dungeon_read_block));
    CHECK(!dungeon_read_block.dungeon_read_blocked);
    CHECK(!dungeon_read_block.no_fallback_dungeon);
    CHECK(!theron_v1_track02_loader_intake_gate_object_dungeon_handoff(
        &grid_receipt, &separate_object_facts, &separate_dungeon_facts,
        &object_dungeon_gate));
    CHECK(!object_dungeon_gate.handoff_blocked);
    CHECK(!object_dungeon_gate.no_fallback_visuals);
    grid_receipt.authenticated_v3_trace = 1;
    CHECK(!theron_v1_track02_loader_intake_block_raw_grid_object_table_projection(
        &grid_receipt, &object_projection));
    CHECK(!object_projection.projection_blocked);
    CHECK(!object_projection.no_fallback);
    CHECK(!theron_v1_track02_loader_intake_block_object_table_read_claim(
        &grid_receipt, &facts, &object_read_block));
    CHECK(!object_read_block.object_table_read_blocked);
    CHECK(!object_read_block.no_fallback);
    CHECK(!theron_v1_track02_loader_intake_block_raw_grid_bitmap_route(
        &grid_receipt, &bitmap_route));
    CHECK(!bitmap_route.bitmap_route_blocked);
    CHECK(!bitmap_route.no_fallback);
    CHECK(!theron_v1_track02_loader_intake_block_bitmap_read_claim(
        &grid_receipt, &facts, &bitmap_read_block));
    CHECK(!bitmap_read_block.bitmap_read_blocked);
    CHECK(!bitmap_read_block.no_fallback_visual);
    CHECK(!theron_v1_track02_loader_intake_admit_raw_grid_level_route(
        &grid_receipt, &level_route));
    CHECK(!level_route.level_route_admitted);
    CHECK(!level_route.no_fallback);
    CHECK(!theron_v1_track02_loader_intake_admit_raw_grid_dungeon_route(
        &grid_receipt, &dungeon_route));
    CHECK(!dungeon_route.dungeon_route_admitted);
    CHECK(!dungeon_route.no_fallback);
    CHECK(!theron_v1_track02_loader_intake_block_raw_grid_dungeon_record_evidence(
        &grid_receipt, &dungeon_record));
    CHECK(!dungeon_record.dungeon_record_blocked);
    CHECK(!dungeon_record.no_fallback);
    CHECK(!theron_v1_track02_loader_intake_block_dungeon_read_claim(
        &grid_receipt, &facts, &dungeon_read_block));
    CHECK(!dungeon_read_block.dungeon_read_blocked);
    CHECK(!dungeon_read_block.no_fallback_dungeon);
    CHECK(!theron_v1_track02_loader_intake_gate_object_dungeon_handoff(
        &grid_receipt, &separate_object_facts, &separate_dungeon_facts,
        &object_dungeon_gate));
    CHECK(!object_dungeon_gate.handoff_blocked);
    CHECK(!object_dungeon_gate.no_fallback_visuals);
    grid_receipt.status = NULL;
    CHECK(!theron_v1_track02_loader_intake_block_raw_grid_object_table_projection(
        &grid_receipt, &object_projection));
    CHECK(!object_projection.projection_blocked);
    CHECK(!object_projection.no_fallback);
    CHECK(!theron_v1_track02_loader_intake_block_object_table_read_claim(
        &grid_receipt, &facts, &object_read_block));
    CHECK(!object_read_block.object_table_read_blocked);
    CHECK(!object_read_block.no_fallback);
    CHECK(!theron_v1_track02_loader_intake_block_raw_grid_bitmap_route(
        &grid_receipt, &bitmap_route));
    CHECK(!bitmap_route.bitmap_route_blocked);
    CHECK(!bitmap_route.no_fallback);
    CHECK(!theron_v1_track02_loader_intake_block_bitmap_read_claim(
        &grid_receipt, &facts, &bitmap_read_block));
    CHECK(!bitmap_read_block.bitmap_read_blocked);
    CHECK(!bitmap_read_block.no_fallback_visual);
    CHECK(!theron_v1_track02_loader_intake_block_raw_grid_dungeon_record_evidence(
        &grid_receipt, &dungeon_record));
    CHECK(!dungeon_record.dungeon_record_blocked);
    CHECK(!dungeon_record.no_fallback);
    CHECK(!theron_v1_track02_loader_intake_block_dungeon_read_claim(
        &grid_receipt, &facts, &dungeon_read_block));
    CHECK(!dungeon_read_block.dungeon_read_blocked);
    CHECK(!dungeon_read_block.no_fallback_dungeon);
    CHECK(!theron_v1_track02_loader_intake_gate_object_dungeon_handoff(
        &grid_receipt, &separate_object_facts, &separate_dungeon_facts,
        &object_dungeon_gate));
    CHECK(!object_dungeon_gate.handoff_blocked);
    CHECK(!object_dungeon_gate.no_fallback_visuals);
    grid_receipt.status = "initial_envelope_raw_grid_handoff_no_semantics";
    grid_receipt.raw_grid_hash = 0u;
    CHECK(!theron_v1_track02_loader_intake_block_raw_grid_object_table_projection(
        &grid_receipt, &object_projection));
    CHECK(!object_projection.projection_blocked);
    CHECK(!object_projection.no_fallback);
    CHECK(!theron_v1_track02_loader_intake_block_object_table_read_claim(
        &grid_receipt, &facts, &object_read_block));
    CHECK(!object_read_block.object_table_read_blocked);
    CHECK(!object_read_block.no_fallback);
    CHECK(!theron_v1_track02_loader_intake_block_raw_grid_bitmap_route(
        &grid_receipt, &bitmap_route));
    CHECK(!bitmap_route.bitmap_route_blocked);
    CHECK(!bitmap_route.no_fallback);
    CHECK(!theron_v1_track02_loader_intake_block_bitmap_read_claim(
        &grid_receipt, &facts, &bitmap_read_block));
    CHECK(!bitmap_read_block.bitmap_read_blocked);
    CHECK(!bitmap_read_block.no_fallback_visual);
    CHECK(!theron_v1_track02_loader_intake_admit_raw_grid_dungeon_route(
        &grid_receipt, &dungeon_route));
    CHECK(!dungeon_route.dungeon_route_admitted);
    CHECK(!dungeon_route.no_fallback);
    CHECK(!theron_v1_track02_loader_intake_block_raw_grid_dungeon_record_evidence(
        &grid_receipt, &dungeon_record));
    CHECK(!dungeon_record.dungeon_record_blocked);
    CHECK(!dungeon_record.no_fallback);
    CHECK(!theron_v1_track02_loader_intake_block_dungeon_read_claim(
        &grid_receipt, &facts, &dungeon_read_block));
    CHECK(!dungeon_read_block.dungeon_read_blocked);
    CHECK(!dungeon_read_block.no_fallback_dungeon);
    CHECK(!theron_v1_track02_loader_intake_gate_object_dungeon_handoff(
        &grid_receipt, &separate_object_facts, &separate_dungeon_facts,
        &object_dungeon_gate));
    CHECK(!object_dungeon_gate.handoff_blocked);
    CHECK(!object_dungeon_gate.no_fallback_visuals);

    initial_envelope.envelope_bytes = facts.byte_count + 1u;
    CHECK(!theron_v1_track02_loader_intake_bind_initial_envelope(
        &receipt, &initial_envelope, &receipt));
    initial_envelope.envelope_bytes = THERON_V1_INITIAL_ENVELOPE_BYTES;
    initial_envelope.header_width = 0x001fu;
    CHECK(!theron_v1_track02_loader_intake_bind_initial_envelope(
        &receipt, &initial_envelope, &receipt));
    initial_envelope.header_width = THERON_V1_INITIAL_ENVELOPE_HEADER_WIDTH;
    initial_envelope.header_height = 0x001au;
    CHECK(!theron_v1_track02_loader_intake_bind_initial_envelope(
        &receipt, &initial_envelope, &receipt));
    initial_envelope.header_height = THERON_V1_INITIAL_ENVELOPE_HEADER_HEIGHT;
    initial_envelope.header_seed = 0x0108e939u;
    CHECK(!theron_v1_track02_loader_intake_bind_initial_envelope(
        &receipt, &initial_envelope, &receipt));
    initial_envelope.header_seed = THERON_V1_INITIAL_ENVELOPE_HEADER_SEED;
    initial_envelope.record = 0x04e0u;
    CHECK(!theron_v1_track02_loader_intake_bind_initial_envelope(
        &receipt, &initial_envelope, &receipt));
    initial_envelope.record = THERON_V1_INITIAL_ENVELOPE_RECORD;
    initial_envelope.track02_raw_sector = 3122u;
    CHECK(!theron_v1_track02_loader_intake_bind_initial_envelope(
        &receipt, &initial_envelope, &receipt));
    initial_envelope.track02_raw_sector = 3123u;
    initial_envelope.raw_sector_offset = 0x123u;
    CHECK(!theron_v1_track02_loader_intake_bind_initial_envelope(
        &receipt, &initial_envelope, &receipt));
    initial_envelope.raw_sector_offset = 0x124u;
    initial_envelope.raw_track02_md5_verified = 0;
    CHECK(!theron_v1_track02_loader_intake_bind_initial_envelope(
        &receipt, &initial_envelope, &receipt));
    initial_envelope.raw_track02_md5_verified = 1;
    memset(&later_cd_record, 0, sizeof(later_cd_record));
    CHECK(!theron_v1_track02_loader_intake_bind_later_read_layout(
        &later_cd_record, THERON_V1_TRACK02_LAYOUT_ROLE_OBJECT_TABLE,
        &later_layout));
    CHECK(!later_layout.layout_window_bound);
    CHECK(!later_layout.rendering_blocked);
    memset(&object_layout, 0, sizeof(object_layout));
    memset(&dungeon_layout, 0, sizeof(dungeon_layout));
    CHECK(!theron_v1_track02_loader_intake_bind_object_dungeon_layout_pair(
        &object_layout, &dungeon_layout, &layout_pair));
    CHECK(!layout_pair.layout_pair_bound);
    CHECK(!theron_v1_track02_loader_intake_handoff_later_layout_bytes(
        &object_layout, NULL, 0u, THERON_V1_TRACK02_MD5_US_BIN,
        layout_bytes, sizeof(layout_bytes), &layout_bytes_receipt));
    CHECK(!layout_bytes_receipt.bytes_handed_off);
    memset(&object_layout_bytes, 0, sizeof(object_layout_bytes));
    memset(&dungeon_layout_bytes, 0, sizeof(dungeon_layout_bytes));
    CHECK(!theron_v1_track02_loader_intake_bind_object_dungeon_layout_bytes_pair(
        &layout_pair, &object_layout_bytes, &dungeon_layout_bytes,
        &layout_bytes_pair));
    CHECK(!layout_bytes_pair.byte_pair_bound);
    later_cd_record.cd_record_read_proven = 1;
    later_cd_record.raw_cue_admission_consumed = 1;
    later_cd_record.authenticated_later_loader_read = 1;
    later_cd_record.object_semantics_blocked = 1;
    later_cd_record.dungeon_semantics_blocked = 1;
    later_cd_record.decoder_binding_required = 1;
    later_cd_record.grammar_binding_required = 1;
    later_cd_record.no_fallback_visuals = 1;
    later_cd_record.no_synthetic_handoff = 1;
    later_cd_record.raw_track02_variant = THERON_V1_TRACK02_VARIANT_US_BIN;
    later_cd_record.track02_record = separate_object_facts.track02_record;
    later_cd_record.record_user_data_offset =
        separate_object_facts.record_user_data_offset;
    later_cd_record.destination = separate_object_facts.destination;
    later_cd_record.byte_count = separate_object_facts.byte_count;
    later_cd_record.raw_track02_sector = 225u + later_cd_record.track02_record;
    later_cd_record.raw_sector_offset =
        THERON_V1_TRACK02_MODE1_HEADER_BYTES +
        later_cd_record.record_user_data_offset;
    later_cd_record.raw_track02_offset =
        later_cd_record.raw_track02_sector *
        THERON_V1_TRACK02_RAW_SECTOR_BYTES +
        later_cd_record.raw_sector_offset;
    later_cd_record.track02_md5 = THERON_V1_TRACK02_MD5_US_BIN;
    later_cd_record.status =
        "later_loader_cd_record_read_proven_semantics_blocked_no_fallback";
    CHECK(!theron_v1_track02_loader_intake_bind_later_read_layout(
        &later_cd_record, THERON_V1_TRACK02_LAYOUT_ROLE_NONE,
        &later_layout));
    CHECK(!later_layout.layout_window_bound);
    later_cd_record.raw_track02_sector = 224u + later_cd_record.track02_record;
    CHECK(!theron_v1_track02_loader_intake_bind_later_read_layout(
        &later_cd_record, THERON_V1_TRACK02_LAYOUT_ROLE_OBJECT_TABLE,
        &later_layout));
    CHECK(!later_layout.layout_window_bound);
    later_cd_record.raw_track02_sector = 225u + later_cd_record.track02_record;
    later_cd_record.no_fallback_visuals = 0;
    CHECK(!theron_v1_track02_loader_intake_bind_later_read_layout(
        &later_cd_record, THERON_V1_TRACK02_LAYOUT_ROLE_DUNGEON_RECORD,
        &later_layout));
    CHECK(!later_layout.layout_window_bound);
    later_cd_record.no_fallback_visuals = 1;

    CHECK(theron_v1_track02_loader_intake_observe_authenticated_trace(
        &authenticated_facts, &receipt));
    CHECK(receipt.observed);
    CHECK(receipt.authenticated_v3_trace);
    CHECK(!receipt.payload_intake_admitted);
    CHECK(receipt.record == authenticated_facts.track02_record);
    CHECK(receipt.record_user_data_offset ==
          authenticated_facts.record_user_data_offset);
    CHECK(receipt.observed_destination == authenticated_facts.destination);
    CHECK(receipt.observed_byte_count == authenticated_facts.byte_count);
    trace.runtime_admitted = 0;
    CHECK(!theron_v1_track02_loader_intake_observe_authenticated_trace(
        &authenticated_facts, &receipt));
    CHECK(!receipt.observed);
    CHECK(!receipt.payload_intake_admitted);
    CHECK(receipt.record == 0u);
    CHECK(receipt.record_user_data_offset == 0u);
    CHECK(receipt.observed_destination == 0u);
    CHECK(receipt.observed_byte_count == 0u);
    CHECK(receipt.status == NULL);
    trace.runtime_admitted = 1;
    trace.valid = 0;
    CHECK(!theron_v1_track02_loader_intake_observe_authenticated_trace(
        &authenticated_facts, &receipt));
    CHECK(!receipt.observed);
    CHECK(!receipt.payload_intake_admitted);
    CHECK(receipt.record == 0u);
    CHECK(receipt.record_user_data_offset == 0u);
    CHECK(receipt.observed_destination == 0u);
    CHECK(receipt.observed_byte_count == 0u);
    CHECK(receipt.status == NULL);
    trace.valid = 1;
    authenticated_facts.byte_count = 0u;
    CHECK(!theron_v1_track02_loader_intake_observe_authenticated_trace(
        &authenticated_facts, &receipt));
    authenticated_facts.byte_count = 0x800u;

    facts.authenticated_original_trace = 0;
    CHECK(!theron_v1_track02_loader_intake_observe(&facts, &receipt));
    facts.authenticated_original_trace = 1;
    facts.later_than_stage2_transfer = 0;
    CHECK(!theron_v1_track02_loader_intake_observe(&facts, &receipt));
    facts.later_than_stage2_transfer = 1;
    facts.track02_record = 0x04e0u;
    CHECK(!theron_v1_track02_loader_intake_observe(&facts, &receipt));
    facts.track02_record = 0x04dfu;
    CHECK(!theron_v1_track02_loader_intake_observe(&facts, &receipt));
    facts.track02_record = THERON_V1_INITIAL_ENVELOPE_RECORD;
    facts.record_user_data_offset = 0u;
    CHECK(!theron_v1_track02_loader_intake_observe(&facts, &receipt));
    facts.record_user_data_offset =
        THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET;
    facts.byte_count = 0u;
    CHECK(!theron_v1_track02_loader_intake_observe(&facts, &receipt));

    return failures != 0;
}
