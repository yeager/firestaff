/*
 * Bounded integration check for the future positive Track 02 loader/CD
 * handoff. A fixture receipt only exercises composition; it is not presented
 * as an original capture. A real positive result requires the operator to
 * supply a coalesced Mednafen transcript that the production parser accepts.
 */
#include "asset_status_m12.h"
#include "theron_v1_boot.h"
#include "theron_v1_raw_loader_trace.h"
#include "theron_v1_later_record_correlation.h"
#include "theron_v1_stage3_manifest_evidence.h"
#include "theron_v1_startup_runtime_entry.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

static uint8_t *read_file(const char *path, size_t *out_size)
{
    FILE *file = NULL;
    long size;
    uint8_t *bytes = NULL;

    if (!path || !out_size || !(file = fopen(path, "rb")) ||
        fseek(file, 0L, SEEK_END) != 0 || (size = ftell(file)) <= 0 ||
        fseek(file, 0L, SEEK_SET) != 0 ||
        !(bytes = (uint8_t *)malloc((size_t)size)) ||
        fread(bytes, 1u, (size_t)size, file) != (size_t)size) {
        if (file) fclose(file);
        free(bytes);
        return NULL;
    }
    fclose(file);
    *out_size = (size_t)size;
    return bytes;
}

static uint32_t fnv1a_bytes(const uint8_t *bytes, size_t byte_count)
{
    uint32_t hash = 2166136261u;
    size_t i;

    for (i = 0u; i < byte_count; ++i) {
        hash ^= bytes[i];
        hash *= 16777619u;
    }
    return hash;
}

static void fixture_receipt(Theron_V1RawLoaderTraceCoalescedLaterReceipt *out,
                            const char *md5, const uint8_t *raw,
                            size_t raw_size)
{
    Theron_Track02InitialLevelObjectBoundaryReceipt boundary;
    Theron_Track02Stage2DynamicPayloadReceipt payload;
    Theron_V1Stage3ManifestEvidence manifest;
    Theron_V1Stage3DescriptorRecordBoundary descriptor_boundary;
    size_t raw_offset;

    memset(out, 0, sizeof(*out));
    if (theron_v1_track02_capture_initial_level_object_boundary(
            raw, raw_size, md5, &boundary) != THERON_TRACK02_SIGNAL_OK ||
        theron_v1_track02_inspect_stage2_dynamic_payload(
            raw, raw_size, md5, &payload) != THERON_TRACK02_SIGNAL_OK ||
        !theron_v1_stage3_manifest_evidence_from_payload(
            raw, raw_size, &payload, &manifest) ||
        !theron_v1_stage3_descriptor_record_boundary_from_manifest(
            raw, raw_size, &manifest, 0u, &descriptor_boundary) ||
        !boundary.valid || boundary.track02_record != 0x0b52u) {
        return;
    }
    raw_offset = boundary.level_first_raw_sector *
        THERON_TRACK02_RAW_SECTOR_BYTES + THERON_TRACK02_RAW_USER_DATA_OFFSET;
    out->valid = 1;
    out->variant = THERON_TRACK02_VARIANT_US_BIN;
    snprintf(out->track02_md5, sizeof(out->track02_md5), "%s", md5);
    out->stage3_entry_pc = 0x3800u;
    out->stage3_irq2_selector = 0xffu;
    out->stage3_continuation_pc = 0x3802u;
    out->stage3_post_irq2_next_pc = 0x3803u;
    out->stage3_post_irq2_resume_verified = 1;
    out->later_track02_record = 0x0b52u;
    out->descriptor_selector = 0x067cu;
    out->descriptor_selector_ordinal = 0u;
    if (manifest.descriptors[0].word2 != out->descriptor_selector) {
        memset(out, 0, sizeof(*out));
        return;
    }
    out->descriptor_word0 = manifest.descriptors[0].word0;
    out->descriptor_word1 = manifest.descriptors[0].word1;
    out->descriptor_record_user_data_hash = fnv1a_bytes(
        raw + raw_offset, THERON_TRACK02_RAW_USER_DATA_BYTES);
    out->descriptor_row_media_bound = 1;
    out->descriptor_semantics_proven = 0;
    out->descriptor_source_raw_offset =
        descriptor_boundary.descriptor_source_raw_offset;
    out->descriptor_source_bytes = descriptor_boundary.descriptor_source_bytes;
    out->descriptor_source_hash = descriptor_boundary.descriptor_source_hash;
    out->descriptor_source_bytes_proven =
        descriptor_boundary.descriptor_source_bytes_proven;
    out->descriptor_selector_occurrence_count =
        descriptor_boundary.selector_occurrence_count;
    out->descriptor_selector_first_ordinal =
        descriptor_boundary.selector_first_ordinal;
    out->descriptor_selector_last_ordinal =
        descriptor_boundary.selector_last_ordinal;
    out->descriptor_selector_row_hash = descriptor_boundary.selector_row_hash;
    out->descriptor_selector_aliases_proven =
        descriptor_boundary.selector_aliases_proven;
    out->caller_pc = 0xea00u;
    out->return_pc = 0xea03u;
    out->later_caller_opcode = 0x20u;
    out->later_caller_target = 0xe009u;
    out->later_caller_control_verified = 1;
    out->sector_count = 1u;
    out->later_local_destination = 0x3800u;
    out->later_destination_span_bytes = 32u;
    out->later_destination_span_checksum = fnv1a_bytes(
        raw + raw_offset, 32u);
    out->later_destination_local_ram_verified = 1;
    out->later_destination_media_span_verified = 1;
    out->later_destination_payload_bytes = THERON_TRACK02_RAW_USER_DATA_BYTES;
    out->later_destination_payload_checksum = fnv1a_bytes(
        raw + raw_offset,
        THERON_TRACK02_RAW_USER_DATA_BYTES);
    out->later_destination_payload_verified = 1;
    out->later_post_return_resume_pc = 0xea03u;
    out->later_post_return_next_pc = 0xea04u;
    out->later_post_return_step_verified = 1;
    out->observation_order_verified = 1;
    out->selector_sector_bytes_verified = 1;
}

int main(void)
{
    const char *raw_path = getenv("FIRESTAFF_THERON_TRACK02_US_BIN");
    const char *trace_path = getenv("FIRESTAFF_THERON_COALESCED_LOADER_TRACE");
    char md5[33];
    char game_payload_capture[16384];
    uint8_t *raw;
    size_t raw_size;
    Theron_V1RawLoaderTraceCoalescedLaterReceipt coalesced;
    Theron_V1RawLoaderTraceInitialLevelHandoffReceipt handoff;
    Theron_V1RawLoaderTraceInitialLevelHandoffReceipt scratch_handoff;
    Theron_V1_BootProfile profile;
    Theron_V1StartupRuntimeInitialPayloadReceipt payload_receipt;
    Theron_V1StartupRuntimeInitialRouteReceipt route_receipt;
    Theron_V1_World world;
    Theron_V1CaptureManifest manifest;
    Theron_Track02InitialLevelObjectBoundaryReceipt boundary;
    Theron_Track02InitialLevelObjectBoundaryReceipt mutated_boundary;
    Theron_Track02IplLoaderReceipt loader;
    Theron_Track02CanonicalIsoProjectionReceipt iso_projection;
    uint8_t *complete_iso_projection = NULL;
    size_t complete_iso_projection_size;
    size_t projection_sector;
    uint8_t saved_tail_byte;
    uint8_t saved_game_payload_byte;
    uint8_t saved_envelope_byte;
    Theron_V1RawLoaderTraceGamePayloadReceipt game_payload;
    Theron_V1RawLoaderTraceGamePayloadReceipt envelope_payload;
    Theron_V1RawLoaderTraceGamePayloadReceipt header_payloads[
        THERON_V1_RAW_LOADER_INITIAL_ENVELOPE_HEADER_BYTES];
    Theron_V1RawLoaderTraceInitialEnvelopeByteReceipt envelope_byte;
    Theron_V1RawLoaderTraceInitialPostEnvelopeByteReceipt continuation_byte;
    Theron_V1RawLoaderTraceInitialPostEnvelopePrefixReceipt continuation_prefix;
    Theron_V1RawLoaderTraceInitialPostEnvelopeTransferReceipt continuation_transfer;
    Theron_V1RawLoaderTraceInitialPostEnvelopeExecutionReceipt continuation_execution;
    Theron_V1RawLoaderTraceInitialPostEnvelopePostReturnCallReceipt continuation_post_return_call;
    Theron_V1RawLoaderTraceInitialPostEnvelopePostReturnCallTerminationReceipt continuation_post_return_termination;
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextCallReceipt continuation_caller_next_call;
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextCallEntryReceipt continuation_caller_next_call_entry;
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextEntryNextReceipt continuation_caller_next_entry_next;
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferReceipt continuation_caller_next_transfer;
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallReceipt continuation_caller_next_transfer_call;
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryReceipt continuation_caller_next_transfer_call_entry;
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryCopyReceipt continuation_caller_next_transfer_call_entry_copy;
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryCopyNextReceipt continuation_caller_next_transfer_call_entry_copy_next;
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryCopySuccessorReceipt continuation_caller_next_transfer_call_entry_copy_successor;
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryBranchReceipt continuation_caller_next_transfer_call_entry_branch;
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryBranchTargetReceipt continuation_caller_next_transfer_call_entry_branch_target;
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryBranchTargetJsrReceipt continuation_caller_next_transfer_call_entry_branch_target_jsr;
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryBranchTargetJsrCdReceipt continuation_caller_next_transfer_call_entry_branch_target_jsr_cd;
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryBranchTargetJsrCdConsumerReceipt continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer;
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryBranchTargetJsrCdConsumerControlReceipt continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control;
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryBranchTargetJsrCdConsumerControlEntryReceipt continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_entry;
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryBranchTargetJsrCdConsumerControlEntryNextReceipt continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_entry_next;
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryBranchTargetJsrCdConsumerControlReturnReceipt continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return;
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryBranchTargetJsrCdConsumerControlReturnConsumerReceipt continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer;
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryBranchTargetJsrCdConsumerControlReturnConsumerControlReceipt continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control;
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryBranchTargetJsrCdConsumerControlReturnConsumerControlEntryReceipt continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control_entry;
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryBranchTargetJsrCdConsumerControlReturnConsumerControlEntryNextReceipt continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control_entry_next;
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryBranchTargetJsrCdConsumerControlReturnConsumerControlReturnReceipt continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control_return;
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryBranchTargetJsrCdConsumerControlReturnConsumerControlReturnConsumerReceipt continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control_return_consumer;
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryBranchTargetJsrCdConsumerControlReturnConsumerControlReturnConsumerLoopContinuationReceipt continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control_return_consumer_loop_continuation;
    Theron_V1RawLoaderTraceGamePayloadReceipt continuation_payloads[
        THERON_V1_RAW_LOADER_INITIAL_POST_ENVELOPE_PREFIX_BYTES];
    Theron_V1RawLoaderTraceInitialEnvelopeHeaderReceipt envelope_header;
    size_t header_index;
    const char *system_card_md5 = "ff1a674273fe3540ccef576376407d1d";
    const char *trace_md5 = "0123456789abcdef0123456789abcdef";
    uint8_t transfer_destination_entry_opcode;
    uint8_t transfer_destination_entry_next_opcode;
    uint8_t transfer_destination_entry_successor_opcode;
    uint8_t transfer_destination_branch_displacement;
    uint16_t transfer_destination_branch_target;
    uint16_t transfer_destination_branch_target_jsr_pc;
    uint16_t transfer_destination_branch_target_jsr_target;
    uint32_t transfer_destination_branch_target_lba;
    uint16_t transfer_destination_branch_target_source_offset;
    uint8_t transfer_destination_branch_target_source_byte;

    if (!raw_path) {
        printf("SKIP: set FIRESTAFF_THERON_TRACK02_US_BIN for raw-media handoff coverage\n");
        return 0;
    }
    raw = read_file(raw_path, &raw_size);
    if (!raw || !m12_file_md5_hex(raw_path, md5) ||
        strcmp(md5, THERON_TRACK02_MD5_US_BIN) != 0) {
        free(raw);
        printf("FAIL: supplied US Track 02 is not the source-locked raw corpus\n");
        return 1;
    }

    /* Parser-only fixture: its byte value is taken from the hash-verified
     * US raw sector, while a real runtime receipt still requires an original
     * Mednafen transcript with these exact rows. */
    saved_game_payload_byte = raw[0x484u * THERON_TRACK02_RAW_SECTOR_BYTES +
                                  17u];
    snprintf(game_payload_capture, sizeof(game_payload_capture),
             "source=mednafen-pce-instrumented-cd\n"
             "main_ram_loader_e009_dispatch sequence=1 logical_pc=3840 physical_pc=1f1840 a=20 x=00 y=04\n"
             "pce_cd_register_write cpu_pc=e90d physical=00001801 data=81\n"
             "pce_cd_register_write cpu_pc=e981 physical=00001801 data=08\n"
             "pce_cd_register_write cpu_pc=e981 physical=00001801 data=00\n"
             "pce_cd_register_write cpu_pc=e981 physical=00001801 data=10\n"
             "pce_cd_register_write cpu_pc=e981 physical=00001801 data=45\n"
             "pce_cd_register_write cpu_pc=e981 physical=00001801 data=01\n"
             "pce_cd_register_write cpu_pc=e981 physical=00001801 data=00\n"
             "scsi_read_command generation=5 opcode=08 cdb=080010450100 start_lba=4165 sector_count=1\n"
             "pce_cd_fifo_origin_main_ram_receipt generation=5 source_lba=4165 source_offset=17 fifo_sequence=7 reader_pc=ea9c logical_destination=3000 physical_destination=1f1000 writer_pc=1840 writer_physical_pc=1f1840 value=%02x\n"
             "pce_cd_fifo_origin_main_ram_consumer sequence=0 generation=5 source_lba=4165 source_offset=17 fifo_sequence=7 logical_address=3000 physical_address=1f1000 value=%02x reader_pc=1840 reader_physical_pc=1f1840\n",
             saved_game_payload_byte, saved_game_payload_byte);
    if (!theron_v1_raw_loader_trace_bind_game_owned_fifo_payload(
            game_payload_capture, raw, raw_size, md5, &game_payload) ||
        !game_payload.valid || game_payload.raw_track02_record != 0x484u ||
        game_payload.source_offset != 17u ||
        !game_payload.cdb_read6_verified ||
        !game_payload.fifo_to_game_ram_verified ||
        !game_payload.game_ram_consumer_verified ||
        game_payload.payload_semantics_proven) {
        free(raw);
        printf("FAIL: game-owned FIFO receipt did not remain source-bound and opaque\n");
        return 1;
    }
    raw[0x484u * THERON_TRACK02_RAW_SECTOR_BYTES + 17u] ^= 0x01u;
    if (theron_v1_raw_loader_trace_bind_game_owned_fifo_payload(
            game_payload_capture, raw, raw_size, md5, &game_payload)) {
        free(raw);
        printf("FAIL: altered FIFO source byte reached the game-owned receipt\n");
        return 1;
    }
    raw[0x484u * THERON_TRACK02_RAW_SECTOR_BYTES + 17u] =
        saved_game_payload_byte;
    memcpy(game_payload_capture +
               (strstr(game_payload_capture, "start_lba=4165") -
                game_payload_capture),
           "start_lba=4166", strlen("start_lba=4166"));
    if (theron_v1_raw_loader_trace_bind_game_owned_fifo_payload(
            game_payload_capture, raw, raw_size, md5, &game_payload)) {
        free(raw);
        printf("FAIL: CDB/LBA mismatch reached the game-owned receipt\n");
        return 1;
    }

    if (theron_v1_track02_capture_initial_level_object_boundary(
            raw, raw_size, md5, &boundary) != THERON_TRACK02_SIGNAL_OK ||
        !boundary.valid || boundary.following_user_data_bytes_in_record != 0x380u ||
        !boundary.following_user_data_hash ||
        boundary.object_boundary_raw_offset >= raw_size) {
        free(raw);
        printf("FAIL: raw Track 02 did not retain the opaque post-level witness\n");
        return 1;
    }
    memset(&envelope_payload, 0, sizeof(envelope_payload));
    envelope_payload.valid = 1;
    envelope_payload.variant = THERON_TRACK02_VARIANT_US_BIN;
    snprintf(envelope_payload.track02_md5,
             sizeof(envelope_payload.track02_md5), "%s", md5);
    envelope_payload.raw_track02_record =
        (uint32_t)boundary.level_first_raw_sector;
    envelope_payload.source_offset = THERON_TRACK02_RAW_USER_DATA_OFFSET +
        (unsigned int)boundary.level_user_data_offset_in_record;
    saved_envelope_byte = raw[boundary.level_first_raw_sector *
        THERON_TRACK02_RAW_SECTOR_BYTES + envelope_payload.source_offset];
    envelope_payload.source_byte = saved_envelope_byte;
    envelope_payload.cdb_read6_verified = 1;
    envelope_payload.fifo_to_game_ram_verified = 1;
    envelope_payload.game_ram_consumer_verified = 1;
    if (!theron_v1_raw_loader_trace_correlate_game_payload_initial_envelope(
            &envelope_payload, raw, raw_size, md5, &envelope_byte) ||
        !envelope_byte.valid ||
        envelope_byte.track02_record != boundary.track02_record ||
        envelope_byte.raw_sector != boundary.level_first_raw_sector ||
        envelope_byte.envelope_offset != 0u ||
        !envelope_byte.game_payload_chain_verified ||
        !envelope_byte.source_envelope_overlap_verified ||
        envelope_byte.level_semantics_proven) {
        free(raw);
        printf("FAIL: game-RAM payload did not remain an opaque initial-envelope byte\n");
        return 1;
    }
    --envelope_payload.source_offset;
    if (theron_v1_raw_loader_trace_correlate_game_payload_initial_envelope(
            &envelope_payload, raw, raw_size, md5, &envelope_byte)) {
        free(raw);
        printf("FAIL: pre-envelope game-RAM byte reached the initial-envelope receipt\n");
        return 1;
    }
    ++envelope_payload.source_offset;
    raw[boundary.level_first_raw_sector * THERON_TRACK02_RAW_SECTOR_BYTES +
        envelope_payload.source_offset] ^= 0x01u;
    if (theron_v1_raw_loader_trace_correlate_game_payload_initial_envelope(
            &envelope_payload, raw, raw_size, md5, &envelope_byte)) {
        free(raw);
        printf("FAIL: altered initial-envelope source byte reached the game-RAM receipt\n");
        return 1;
    }
    raw[boundary.level_first_raw_sector * THERON_TRACK02_RAW_SECTOR_BYTES +
        envelope_payload.source_offset] = saved_envelope_byte;

    envelope_payload.source_offset = THERON_TRACK02_RAW_USER_DATA_OFFSET +
        (unsigned int)boundary.object_boundary_user_data_offset_in_record;
    envelope_payload.source_byte = raw[boundary.level_first_raw_sector *
        THERON_TRACK02_RAW_SECTOR_BYTES + envelope_payload.source_offset];
    if (!theron_v1_raw_loader_trace_correlate_game_payload_initial_post_envelope(
            &envelope_payload, raw, raw_size, md5, &continuation_byte) ||
        !continuation_byte.valid ||
        continuation_byte.track02_record != boundary.track02_record ||
        continuation_byte.raw_sector != boundary.level_first_raw_sector ||
        continuation_byte.continuation_offset != 0u ||
        !continuation_byte.game_payload_chain_verified ||
        !continuation_byte.source_continuation_overlap_verified ||
        continuation_byte.object_table_semantics_proven) {
        free(raw);
        printf("FAIL: game-RAM payload did not remain an opaque continuation byte\n");
        return 1;
    }
    --envelope_payload.source_offset;
    if (theron_v1_raw_loader_trace_correlate_game_payload_initial_post_envelope(
            &envelope_payload, raw, raw_size, md5, &continuation_byte)) {
        free(raw);
        printf("FAIL: pre-continuation game-RAM byte reached the continuation receipt\n");
        return 1;
    }
    ++envelope_payload.source_offset;
    envelope_payload.source_offset = THERON_TRACK02_RAW_USER_DATA_OFFSET +
        (unsigned int)boundary.level_user_data_offset_in_record;
    envelope_payload.source_byte = raw[boundary.level_first_raw_sector *
        THERON_TRACK02_RAW_SECTOR_BYTES + envelope_payload.source_offset];

    for (header_index = 0u;
         header_index < THERON_V1_RAW_LOADER_INITIAL_POST_ENVELOPE_PREFIX_BYTES;
         ++header_index) {
        continuation_payloads[header_index] = envelope_payload;
        continuation_payloads[header_index].source_offset =
            THERON_TRACK02_RAW_USER_DATA_OFFSET +
            (unsigned int)boundary.object_boundary_user_data_offset_in_record +
            (unsigned int)header_index;
        continuation_payloads[header_index].source_byte = raw[
            boundary.level_first_raw_sector * THERON_TRACK02_RAW_SECTOR_BYTES +
            continuation_payloads[header_index].source_offset];
        continuation_payloads[header_index].dispatch_sequence = 5u;
        continuation_payloads[header_index].scsi_generation = 13u;
        continuation_payloads[header_index].scsi_lba = 0x17f0u;
        continuation_payloads[header_index].scsi_sector_count = 1u;
    }
    if (!theron_v1_raw_loader_trace_correlate_game_payload_initial_post_envelope_prefix(
            continuation_payloads,
            THERON_V1_RAW_LOADER_INITIAL_POST_ENVELOPE_PREFIX_BYTES,
            raw, raw_size, md5, &continuation_prefix) ||
        !continuation_prefix.valid ||
        continuation_prefix.track02_record != boundary.track02_record ||
        continuation_prefix.raw_sector != boundary.level_first_raw_sector ||
        !continuation_prefix.contiguous_capture_chain_verified ||
        !continuation_prefix.bytes_hash ||
        continuation_prefix.object_table_semantics_proven ||
        memcmp(continuation_prefix.bytes,
               raw + boundary.object_boundary_raw_offset,
               THERON_V1_RAW_LOADER_INITIAL_POST_ENVELOPE_PREFIX_BYTES) != 0) {
        free(raw);
        printf("FAIL: contiguous continuation capture did not remain opaque\n");
        return 1;
    }
    ++continuation_payloads[5].scsi_lba;
    if (theron_v1_raw_loader_trace_correlate_game_payload_initial_post_envelope_prefix(
            continuation_payloads,
            THERON_V1_RAW_LOADER_INITIAL_POST_ENVELOPE_PREFIX_BYTES,
            raw, raw_size, md5, &continuation_prefix)) {
        free(raw);
        printf("FAIL: split CD capture chain reached the continuation prefix\n");
        return 1;
    }
    for (header_index = 0u;
         header_index < THERON_V1_RAW_LOADER_INITIAL_ENVELOPE_HEADER_BYTES;
         ++header_index) {
        header_payloads[header_index] = envelope_payload;
        header_payloads[header_index].source_offset +=
            (unsigned int)header_index;
        header_payloads[header_index].source_byte = raw[
            boundary.level_first_raw_sector * THERON_TRACK02_RAW_SECTOR_BYTES +
            header_payloads[header_index].source_offset];
        header_payloads[header_index].dispatch_sequence = 4u;
        header_payloads[header_index].scsi_generation = 12u;
        header_payloads[header_index].scsi_lba = 0x17f0u;
        header_payloads[header_index].scsi_sector_count = 1u;
    }
    if (!theron_v1_raw_loader_trace_correlate_game_payload_initial_envelope_header(
            header_payloads,
            THERON_V1_RAW_LOADER_INITIAL_ENVELOPE_HEADER_BYTES,
            raw, raw_size, md5, &envelope_header) || !envelope_header.valid ||
        envelope_header.track02_record != boundary.track02_record ||
        envelope_header.raw_sector != boundary.level_first_raw_sector ||
        !envelope_header.contiguous_capture_chain_verified ||
        !envelope_header.bytes_hash || envelope_header.header_semantics_proven ||
        memcmp(envelope_header.bytes,
               raw + boundary.level_first_raw_sector *
                         THERON_TRACK02_RAW_SECTOR_BYTES +
                     envelope_payload.source_offset,
               THERON_V1_RAW_LOADER_INITIAL_ENVELOPE_HEADER_BYTES) != 0) {
        free(raw);
        printf("FAIL: contiguous game-RAM header receipt did not remain opaque\n");
        return 1;
    }
    ++header_payloads[5].scsi_lba;
    if (theron_v1_raw_loader_trace_correlate_game_payload_initial_envelope_header(
            header_payloads,
            THERON_V1_RAW_LOADER_INITIAL_ENVELOPE_HEADER_BYTES,
            raw, raw_size, md5, &envelope_header)) {
        free(raw);
        printf("FAIL: split CD capture chain reached the header receipt\n");
        return 1;
    }
    saved_tail_byte = raw[boundary.object_boundary_raw_offset];
    raw[boundary.object_boundary_raw_offset] ^= 0x01u;
    if (theron_v1_track02_capture_initial_level_object_boundary(
            raw, raw_size, md5, &mutated_boundary) != THERON_TRACK02_SIGNAL_OK ||
        mutated_boundary.following_user_data_hash ==
            boundary.following_user_data_hash ||
        mutated_boundary.receipt_hash == boundary.receipt_hash) {
        free(raw);
        printf("FAIL: opaque post-level bytes did not invalidate their raw receipt\n");
        return 1;
    }
    raw[boundary.object_boundary_raw_offset] = saved_tail_byte;

    if (theron_v1_track02_find_ipl_loader(raw, raw_size, md5, &loader) !=
            THERON_TRACK02_SIGNAL_OK ||
        !loader.valid || raw_size / 2352u <= loader.data_track_index01_raw_sector ||
        raw_size / 2352u - loader.data_track_index01_raw_sector >
            SIZE_MAX / 2048u) {
        free(raw);
        printf("FAIL: complete Track 02 ISO projection lacks INDEX 01\n");
        return 1;
    }
    complete_iso_projection_size =
        (raw_size / 2352u - loader.data_track_index01_raw_sector) * 2048u;
    complete_iso_projection = (uint8_t *)malloc(complete_iso_projection_size);
    if (!complete_iso_projection) {
        free(raw);
        printf("FAIL: could not allocate complete Track 02 ISO projection\n");
        return 1;
    }
    for (projection_sector = 0u;
         projection_sector < raw_size / 2352u - loader.data_track_index01_raw_sector;
         ++projection_sector) {
        memcpy(complete_iso_projection + projection_sector * 2048u,
               raw + (loader.data_track_index01_raw_sector + projection_sector) *
                   2352u + 16u, 2048u);
    }
    if (theron_v1_track02_verify_canonical_iso_projection(
            raw, raw_size, md5, complete_iso_projection,
            complete_iso_projection_size, &iso_projection) !=
            THERON_TRACK02_SIGNAL_OK ||
        !iso_projection.valid ||
        iso_projection.first_level_track02_record != 0x0b52u ||
        iso_projection.first_level_iso_user_data_offset !=
            0x0b52u * 2048u + 0x114u ||
        iso_projection.post_envelope_iso_user_data_offset !=
            0x0b52u * 2048u + 0x480u ||
        iso_projection.post_envelope_hash != boundary.following_user_data_hash ||
        iso_projection.object_semantics_proven || iso_projection.fallback_allowed) {
        free(complete_iso_projection);
        free(raw);
        printf("FAIL: complete ISO projection lost the bounded level continuation\n");
        return 1;
    }
    ++complete_iso_projection[iso_projection.post_envelope_iso_user_data_offset];
    if (theron_v1_track02_verify_canonical_iso_projection(
            raw, raw_size, md5, complete_iso_projection,
            complete_iso_projection_size, &iso_projection) ==
        THERON_TRACK02_SIGNAL_OK) {
        free(complete_iso_projection);
        free(raw);
        printf("FAIL: altered complete ISO projection reached the loader handoff\n");
        return 1;
    }
    --complete_iso_projection[0x0b52u * 2048u + 0x480u];
    free(complete_iso_projection);

    fixture_receipt(&coalesced, md5, raw, raw_size);
    if (!theron_v1_raw_loader_trace_bind_initial_level_handoff(
            &coalesced, raw, raw_size, md5, &handoff) || !handoff.valid ||
        handoff.observed_track02_record != 0x0b52u ||
        !handoff.complete_initial_level_envelope_proven ||
        handoff.initial_level_semantics_proven ||
        !handoff.complete_payload_witness_proven ||
        handoff.complete_payload_bytes != THERON_TRACK02_RAW_USER_DATA_BYTES ||
        !handoff.descriptor_row_media_bound ||
        handoff.descriptor_semantics_proven ||
        handoff.descriptor_record_user_data_hash !=
            handoff.complete_payload_checksum ||
        !handoff.descriptor_source_bytes_proven ||
        handoff.descriptor_source_bytes != 6u ||
        handoff.descriptor_source_hash == 0u ||
        !handoff.descriptor_selector_aliases_proven ||
        handoff.descriptor_selector_occurrence_count == 0u ||
        handoff.descriptor_selector_first_ordinal >
            handoff.descriptor_selector_ordinal ||
        handoff.descriptor_selector_ordinal >
            handoff.descriptor_selector_last_ordinal ||
        handoff.descriptor_selector_row_hash == 0u ||
        !handoff.loader_intake.observed ||
        handoff.loader_intake.payload_intake_admitted ||
        handoff.loader_intake.record != handoff.observed_track02_record ||
        handoff.loader_intake.observed_payload_checksum !=
            handoff.complete_payload_checksum ||
        !handoff.loader_payload.handed_off ||
        !handoff.loader_payload.no_fallback ||
        handoff.loader_payload.payload_checksum !=
            handoff.complete_payload_checksum ||
        !handoff.loader_level_envelope.handed_off ||
        !handoff.loader_level_envelope.no_fallback ||
        handoff.loader_level_envelope.record_user_data_offset != 0x114u ||
        handoff.loader_level_envelope.envelope_bytes !=
            THERON_V1_INITIAL_LEVEL_ENVELOPE_BYTES ||
        handoff.loader_level_envelope.envelope_checksum !=
            handoff.initial_level_boundary.level_payload_hash ||
        !handoff.loader_post_envelope.handed_off ||
        !handoff.loader_post_envelope.no_fallback ||
        handoff.loader_post_envelope.record != handoff.observed_track02_record ||
        handoff.loader_post_envelope.record_user_data_offset != 0x480u ||
        handoff.loader_post_envelope.byte_count != 0x380u ||
        handoff.loader_post_envelope.checksum !=
            handoff.initial_level_boundary.following_user_data_hash ||
        !handoff.loader_semantic_gate.valid ||
        !handoff.loader_semantic_gate.no_fallback ||
        !handoff.loader_semantic_gate.real_payload_available ||
        !handoff.loader_semantic_gate.level_envelope_available ||
        !handoff.loader_semantic_gate.post_envelope_available ||
        handoff.loader_semantic_gate.track02_variant != handoff.variant ||
        handoff.loader_semantic_gate.record != handoff.observed_track02_record ||
        handoff.loader_semantic_gate.payload_checksum !=
            handoff.complete_payload_checksum ||
        handoff.loader_semantic_gate.level_envelope_checksum !=
            handoff.loader_level_envelope.envelope_checksum ||
        handoff.loader_semantic_gate.post_envelope_checksum !=
            handoff.loader_post_envelope.checksum ||
        handoff.loader_semantic_gate.dungeon_record_semantics_proven ||
        handoff.loader_semantic_gate.object_table_semantics_proven ||
        handoff.loader_semantic_gate.bitmap_route_bound ||
        handoff.loader_semantic_gate.palette_binding_verified ||
        handoff.loader_semantic_gate.rgba_output_allowed ||
        handoff.loader_semantic_gate.fallback_visuals_allowed ||
        handoff.object_tail_semantics_proven || handoff.fallback_visuals_allowed) {
        free(raw);
        printf("FAIL: fixture composition did not preserve the bounded handoff contract\n");
        return 1;
    }
    scratch_handoff = handoff;
    scratch_handoff.loader_semantic_gate.object_table_semantics_proven = 1;
    if (theron_v1_raw_loader_trace_initial_level_handoff_is_complete(
            &scratch_handoff)) {
        free(raw);
        printf("FAIL: object-table semantic promotion bypassed the handoff gate\n");
        return 1;
    }
    ++coalesced.descriptor_word0;
    if (theron_v1_raw_loader_trace_bind_initial_level_handoff(
            &coalesced, raw, raw_size, md5, &scratch_handoff)) {
        free(raw);
        printf("FAIL: altered descriptor row reached the loader handoff\n");
        return 1;
    }
    --coalesced.descriptor_word0;
    transfer_destination_entry_opcode = handoff.loader_post_envelope.bytes[8u];
    transfer_destination_entry_next_opcode = handoff.loader_post_envelope.bytes[9u];
    transfer_destination_entry_successor_opcode = handoff.loader_post_envelope.bytes[10u];
    transfer_destination_branch_displacement = handoff.loader_post_envelope.bytes[9u];
    transfer_destination_branch_target = (uint16_t)(0x2402u +
        (int8_t)transfer_destination_branch_displacement);
    transfer_destination_branch_target_jsr_pc =
        (uint16_t)(transfer_destination_branch_target + 2u);
    transfer_destination_branch_target_jsr_target = 0x2600u;
    transfer_destination_branch_target_lba = handoff.observed_track02_record + 3009u;
    transfer_destination_branch_target_source_offset =
        THERON_TRACK02_RAW_USER_DATA_OFFSET;
    transfer_destination_branch_target_source_byte = raw[
        (size_t)handoff.observed_track02_record * THERON_TRACK02_RAW_SECTOR_BYTES +
        transfer_destination_branch_target_source_offset];

    {
        Theron_Track02InitialLevelLoaderSemanticReceipt direct_semantics;
        Theron_Track02InitialLevelLoaderRoute direct_route;

        if (theron_v1_track02_decode_initial_level_loader_semantics(
                raw, raw_size, md5, &direct_semantics) !=
                THERON_TRACK02_SIGNAL_NOT_FOUND ||
            direct_semantics.valid ||
            theron_v1_track02_load_initial_level_loader_route(
                raw, raw_size, md5, THERON_DUNGEON_1_AKUTUBA, 0,
                &direct_route) != THERON_TRACK02_SIGNAL_NOT_FOUND ||
            direct_route.valid) {
            free(raw);
            printf("FAIL: opaque $3800 sector bypassed the loader semantic gate\n");
            return 1;
        }
    }
    memset(&manifest, 0, sizeof(manifest));
    manifest.valid = 1;
    snprintf(manifest.track02_path, sizeof(manifest.track02_path),
             "%s", "/tmp/theron-track02.bin");
    snprintf(manifest.track02_md5, sizeof(manifest.track02_md5), "%s", md5);
    snprintf(manifest.system_card_path, sizeof(manifest.system_card_path),
             "%s", "/tmp/syscard3.pce");
    snprintf(manifest.system_card_md5, sizeof(manifest.system_card_md5),
             "%s", system_card_md5);
    snprintf(manifest.trace_path, sizeof(manifest.trace_path),
             "%s", "/tmp/theron-e009.trace");
    snprintf(manifest.trace_md5, sizeof(manifest.trace_md5), "%s", trace_md5);
    if (!theron_v1_raw_loader_trace_bind_capture_manifest_to_initial_level_handoff(
            &handoff, &manifest, manifest.track02_path, md5,
            manifest.system_card_path, system_card_md5, manifest.trace_path,
            trace_md5, &handoff) ||
        !theron_v1_raw_loader_trace_manifest_initial_level_handoff_is_complete(
            &handoff)) {
        free(raw);
        printf("FAIL: authenticated manifest did not bind the e009 handoff\n");
        return 1;
    }
    /* Parser fixture only: the source coordinate and bytes are derived from
     * the authenticated raw handoff. A runtime receipt still needs the exact
     * marker emitted by the instrumented original Mednafen producer. */
    snprintf(game_payload_capture, sizeof(game_payload_capture),
             "source=mednafen-pce-instrumented-main-ram-loader\n"
             "main_ram_loader_block_transfer logical_pc=3840 physical_pc=1f1840 operation=tii source=3c80 destination=2000 length=0020\n");
    if (!theron_v1_raw_loader_trace_bind_initial_post_envelope_tii_transfer(
            &handoff, game_payload_capture, &continuation_transfer) ||
        !continuation_transfer.valid ||
        continuation_transfer.track02_record != handoff.observed_track02_record ||
        continuation_transfer.source_address != 0x3c80u ||
        continuation_transfer.destination_address != 0x2000u ||
        continuation_transfer.byte_count != 0x20u ||
        continuation_transfer.source_checksum != fnv1a_bytes(
            handoff.loader_post_envelope.bytes, 0x20u) ||
        !continuation_transfer.manifest_bound ||
        !continuation_transfer.source_continuation_transfer_verified ||
        continuation_transfer.object_table_semantics_proven) {
        free(raw);
        printf("FAIL: authenticated continuation TII did not remain source-only\n");
        return 1;
    }
    game_payload_capture[0] = 'x';
    if (theron_v1_raw_loader_trace_bind_initial_post_envelope_tii_transfer(
            &handoff, game_payload_capture, &continuation_transfer)) {
        free(raw);
        printf("FAIL: unmarked TII trace reached the continuation receipt\n");
        return 1;
    }
    snprintf(game_payload_capture, sizeof(game_payload_capture),
             "source=mednafen-pce-instrumented-main-ram-loader\n"
             "main_ram_loader_block_transfer logical_pc=3840 physical_pc=1f1840 operation=tii source=3c80 destination=2000 length=0020\n"
             "main_ram_loader_jsr logical_pc=3850 physical_pc=1f1850 target=2000 a=00 x=00 y=00\n"
             "main_ram_loader_rts logical_pc=2010 physical_pc=1f2010\n"
             "main_ram_loader_post_rts source_logical_pc=2010 source_physical_pc=1f2010 logical_pc=3853 physical_pc=1f1853 opcode=ea\n");
    if (!theron_v1_raw_loader_trace_bind_initial_post_envelope_execution(
            &handoff, game_payload_capture, &continuation_execution) ||
        !continuation_execution.valid ||
        !continuation_execution.continuation_execution_proven ||
        !continuation_execution.continuation_termination_instruction_proven ||
        !continuation_execution.continuation_post_return_target_proven ||
        continuation_execution.level_or_object_semantics_proven ||
        continuation_execution.call_pc != 0x3850u ||
        continuation_execution.call_physical_pc != 0x1f1850u ||
        continuation_execution.call_target != 0x2000u ||
        continuation_execution.return_instruction_pc != 0x2010u ||
        continuation_execution.return_instruction_physical_pc != 0x1f2010u ||
        continuation_execution.post_return_pc != 0x3853u ||
        continuation_execution.post_return_physical_pc != 0x1f1853u ||
        continuation_execution.post_return_opcode != 0xeau ||
        continuation_execution.transfer.source_checksum !=
            continuation_transfer.source_checksum) {
        free(raw);
        printf("FAIL: continuation execution handoff was not source-bound\n");
        return 1;
    }
    *(strstr(game_payload_capture, "target=2000") + strlen("target=")) = '1';
    if (theron_v1_raw_loader_trace_bind_initial_post_envelope_execution(
            &handoff, game_payload_capture, &continuation_execution)) {
        free(raw);
        printf("FAIL: wrong continuation JSR target reached execution receipt\n");
        return 1;
    }
    snprintf(game_payload_capture, sizeof(game_payload_capture),
             "source=mednafen-pce-instrumented-main-ram-loader\n"
             "main_ram_loader_block_transfer logical_pc=3840 physical_pc=1f1840 operation=tii source=3c80 destination=2000 length=0020\n"
             "main_ram_loader_jsr logical_pc=3850 physical_pc=1f1850 target=2000 a=00 x=00 y=00\n"
             "main_ram_loader_rts logical_pc=2020 physical_pc=1f2020\n"
             "main_ram_loader_post_rts source_logical_pc=2020 source_physical_pc=1f2020 logical_pc=3853 physical_pc=1f1853 opcode=ea\n");
    if (theron_v1_raw_loader_trace_bind_initial_post_envelope_execution(
            &handoff, game_payload_capture, &continuation_execution)) {
        free(raw);
        printf("FAIL: RTS outside copied continuation reached execution receipt\n");
        return 1;
    }
    snprintf(game_payload_capture, sizeof(game_payload_capture),
             "source=mednafen-pce-instrumented-main-ram-loader\n"
             "main_ram_loader_block_transfer logical_pc=3840 physical_pc=1f1840 operation=tii source=3c80 destination=2000 length=0020\n"
             "main_ram_loader_jsr logical_pc=3850 physical_pc=1f1850 target=2000 a=00 x=00 y=00\n"
             "main_ram_loader_rts logical_pc=2010 physical_pc=1f2010\n"
             "main_ram_loader_post_rts source_logical_pc=2010 source_physical_pc=1f2010 logical_pc=3854 physical_pc=1f1854 opcode=ea\n");
    if (theron_v1_raw_loader_trace_bind_initial_post_envelope_execution(
            &handoff, game_payload_capture, &continuation_execution)) {
        free(raw);
        printf("FAIL: changed post-RTS target reached execution receipt\n");
        return 1;
    }
    snprintf(game_payload_capture, sizeof(game_payload_capture),
             "source=mednafen-pce-instrumented-main-ram-loader\n"
             "main_ram_loader_block_transfer logical_pc=3840 physical_pc=1f1840 operation=tii source=3c80 destination=2000 length=0020\n"
             "main_ram_loader_jsr logical_pc=3850 physical_pc=1f1850 target=2000 a=00 x=00 y=00\n"
             "main_ram_loader_rts logical_pc=2010 physical_pc=1f2010\n"
             "main_ram_loader_post_rts source_logical_pc=2010 source_physical_pc=1f2010 logical_pc=3853 physical_pc=1f1853 opcode=20\n"
             "main_ram_loader_jsr logical_pc=3853 physical_pc=1f1853 target=2100 a=00 x=00 y=00\n");
    if (!theron_v1_raw_loader_trace_bind_initial_post_envelope_post_return_call(
            &handoff, game_payload_capture, &continuation_post_return_call) ||
        !continuation_post_return_call.valid ||
        !continuation_post_return_call.post_return_call_proven ||
        continuation_post_return_call.level_or_object_semantics_proven ||
        continuation_post_return_call.call_pc != 0x3853u ||
        continuation_post_return_call.call_physical_pc != 0x1f1853u ||
        continuation_post_return_call.call_target != 0x2100u ||
        continuation_post_return_call.execution.transfer.source_checksum !=
            continuation_transfer.source_checksum) {
        free(raw);
        printf("FAIL: post-RTS routine call was not source-bound\n");
        return 1;
    }
    *(strstr(game_payload_capture,
             "main_ram_loader_jsr logical_pc=3853") +
      strlen("main_ram_loader_jsr logical_pc=385")) = '4';
    if (theron_v1_raw_loader_trace_bind_initial_post_envelope_post_return_call(
            &handoff, game_payload_capture, &continuation_post_return_call)) {
        free(raw);
        printf("FAIL: altered post-RTS routine call site reached receipt\n");
        return 1;
    }
    snprintf(game_payload_capture, sizeof(game_payload_capture),
             "source=mednafen-pce-instrumented-main-ram-loader\n"
             "main_ram_loader_block_transfer logical_pc=3840 physical_pc=1f1840 operation=tii source=3c80 destination=2000 length=0020\n"
             "main_ram_loader_jsr logical_pc=3850 physical_pc=1f1850 target=2000 a=00 x=00 y=00\n"
             "main_ram_loader_rts logical_pc=2010 physical_pc=1f2010\n"
             "main_ram_loader_post_rts source_logical_pc=2010 source_physical_pc=1f2010 logical_pc=3853 physical_pc=1f1853 opcode=20\n"
             "main_ram_loader_jsr logical_pc=3853 physical_pc=1f1853 target=2100 a=00 x=00 y=00\n"
             "main_ram_loader_rts logical_pc=2110 physical_pc=1f2110\n"
             "main_ram_loader_post_rts source_logical_pc=2110 source_physical_pc=1f2110 logical_pc=3856 physical_pc=1f1856 opcode=ea\n"
             "main_ram_loader_jsr logical_pc=3858 physical_pc=1f1858 target=2200 a=00 x=00 y=00\n");
    if (!theron_v1_raw_loader_trace_bind_initial_post_envelope_post_return_call_termination(
            &handoff, game_payload_capture, &continuation_post_return_termination) ||
        !continuation_post_return_termination.valid ||
        !continuation_post_return_termination.post_return_call_termination_proven ||
        !continuation_post_return_termination.post_return_call_return_proven ||
        continuation_post_return_termination.level_or_object_semantics_proven ||
        continuation_post_return_termination.return_instruction_pc != 0x2110u ||
        continuation_post_return_termination.return_instruction_physical_pc !=
            0x1f2110u ||
        continuation_post_return_termination.post_return_pc != 0x3856u ||
        continuation_post_return_termination.post_return_physical_pc !=
            0x1f1856u ||
        continuation_post_return_termination.post_return_opcode != 0xeau ||
        continuation_post_return_termination.call.execution.transfer.source_checksum !=
            continuation_transfer.source_checksum) {
        free(raw);
        printf("FAIL: post-return routine termination was not source-bound\n");
        return 1;
    }
    *(strstr(game_payload_capture,
             "source_logical_pc=2110 source_physical_pc=1f2110 logical_pc=3856") +
      strlen("source_logical_pc=2110 source_physical_pc=1f2110 logical_pc=385")) = '7';
    if (theron_v1_raw_loader_trace_bind_initial_post_envelope_post_return_call_termination(
            &handoff, game_payload_capture, &continuation_post_return_termination)) {
        free(raw);
        printf("FAIL: altered post-return routine target reached receipt\n");
        return 1;
    }
    snprintf(game_payload_capture, sizeof(game_payload_capture),
             "source=mednafen-pce-instrumented-main-ram-loader\n"
             "main_ram_loader_block_transfer logical_pc=3840 physical_pc=1f1840 operation=tii source=3c80 destination=2000 length=0020\n"
             "main_ram_loader_jsr logical_pc=3850 physical_pc=1f1850 target=2000 a=00 x=00 y=00\n"
             "main_ram_loader_rts logical_pc=2010 physical_pc=1f2010\n"
             "main_ram_loader_post_rts source_logical_pc=2010 source_physical_pc=1f2010 logical_pc=3853 physical_pc=1f1853 opcode=20\n"
             "main_ram_loader_jsr logical_pc=3853 physical_pc=1f1853 target=2100 a=00 x=00 y=00\n"
             "main_ram_loader_rts logical_pc=2110 physical_pc=1f2110\n"
             "main_ram_loader_post_rts source_logical_pc=2110 source_physical_pc=1f2110 logical_pc=3856 physical_pc=1f1856 opcode=ea\n"
             "main_ram_loader_jsr logical_pc=3858 physical_pc=1f1858 target=2200 a=00 x=00 y=00\n");
    if (!theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_call(
            &handoff, game_payload_capture, &continuation_caller_next_call) ||
        !continuation_caller_next_call.valid ||
        !continuation_caller_next_call.caller_next_call_proven ||
        continuation_caller_next_call.level_or_object_semantics_proven ||
        continuation_caller_next_call.call_pc != 0x3858u ||
        continuation_caller_next_call.call_physical_pc != 0x1f1858u ||
        continuation_caller_next_call.call_target != 0x2200u ||
        continuation_caller_next_call.termination.call.execution.transfer.source_checksum !=
            continuation_transfer.source_checksum) {
        free(raw);
        printf("FAIL: caller next routine call was not source-bound\n");
        return 1;
    }
    *(strstr(game_payload_capture,
             "main_ram_loader_jsr logical_pc=3858") +
      strlen("main_ram_loader_jsr logical_pc=385")) = '9';
    if (theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_call(
            &handoff, game_payload_capture, &continuation_caller_next_call)) {
        free(raw);
        printf("FAIL: altered caller next routine call reached receipt\n");
        return 1;
    }
    snprintf(game_payload_capture, sizeof(game_payload_capture),
             "source=mednafen-pce-instrumented-main-ram-loader\n"
             "main_ram_loader_block_transfer logical_pc=3840 physical_pc=1f1840 operation=tii source=3c80 destination=2000 length=0020\n"
             "main_ram_loader_jsr logical_pc=3850 physical_pc=1f1850 target=2000 a=00 x=00 y=00\n"
             "main_ram_loader_rts logical_pc=2010 physical_pc=1f2010\n"
             "main_ram_loader_post_rts source_logical_pc=2010 source_physical_pc=1f2010 logical_pc=3853 physical_pc=1f1853 opcode=20\n"
             "main_ram_loader_jsr logical_pc=3853 physical_pc=1f1853 target=2100 a=00 x=00 y=00\n"
             "main_ram_loader_rts logical_pc=2110 physical_pc=1f2110\n"
             "main_ram_loader_post_rts source_logical_pc=2110 source_physical_pc=1f2110 logical_pc=3856 physical_pc=1f1856 opcode=ea\n"
             "main_ram_loader_jsr logical_pc=3858 physical_pc=1f1858 target=2200 a=00 x=00 y=00\n"
             "main_ram_loader_call_entry caller_logical_pc=3858 caller_physical_pc=1f1858 target=2200 logical_pc=2200 physical_pc=1f2200 opcode=ea\n"
             "main_ram_loader_entry_next entry_logical_pc=2200 entry_physical_pc=1f2200 logical_pc=2201 physical_pc=1f2201 opcode=ea\n");
    if (!theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_call_entry(
            &handoff, game_payload_capture, &continuation_caller_next_call_entry) ||
        !continuation_caller_next_call_entry.valid ||
        !continuation_caller_next_call_entry.caller_next_call_entry_proven ||
        continuation_caller_next_call_entry.level_or_object_semantics_proven ||
        continuation_caller_next_call_entry.entry_pc != 0x2200u ||
        continuation_caller_next_call_entry.entry_physical_pc != 0x1f2200u ||
        continuation_caller_next_call_entry.entry_opcode != 0xeau ||
        continuation_caller_next_call_entry.call.termination.call.execution.transfer.source_checksum !=
            continuation_transfer.source_checksum) {
        free(raw);
        printf("FAIL: caller next routine entry was not source-bound\n");
        return 1;
    }
    *(strstr(game_payload_capture,
             "target=2200 logical_pc=2200 physical_pc=1f2200") +
      strlen("target=2200 logical_pc=220")) = '1';
    if (theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_call_entry(
            &handoff, game_payload_capture, &continuation_caller_next_call_entry)) {
        free(raw);
        printf("FAIL: altered caller next routine entry reached receipt\n");
        return 1;
    }
    snprintf(game_payload_capture, sizeof(game_payload_capture),
             "source=mednafen-pce-instrumented-main-ram-loader\n"
             "main_ram_loader_block_transfer logical_pc=3840 physical_pc=1f1840 operation=tii source=3c80 destination=2000 length=0020\n"
             "main_ram_loader_jsr logical_pc=3850 physical_pc=1f1850 target=2000 a=00 x=00 y=00\n"
             "main_ram_loader_rts logical_pc=2010 physical_pc=1f2010\n"
             "main_ram_loader_post_rts source_logical_pc=2010 source_physical_pc=1f2010 logical_pc=3853 physical_pc=1f1853 opcode=20\n"
             "main_ram_loader_jsr logical_pc=3853 physical_pc=1f1853 target=2100 a=00 x=00 y=00\n"
             "main_ram_loader_rts logical_pc=2110 physical_pc=1f2110\n"
             "main_ram_loader_post_rts source_logical_pc=2110 source_physical_pc=1f2110 logical_pc=3856 physical_pc=1f1856 opcode=ea\n"
             "main_ram_loader_jsr logical_pc=3858 physical_pc=1f1858 target=2200 a=00 x=00 y=00\n"
             "main_ram_loader_call_entry caller_logical_pc=3858 caller_physical_pc=1f1858 target=2200 logical_pc=2200 physical_pc=1f2200 opcode=ea\n"
             "main_ram_loader_entry_next entry_logical_pc=2200 entry_physical_pc=1f2200 logical_pc=2201 physical_pc=1f2201 opcode=ea\n");
    if (!theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_entry_next(
            &handoff, game_payload_capture, &continuation_caller_next_entry_next) ||
        !continuation_caller_next_entry_next.valid ||
        !continuation_caller_next_entry_next.caller_next_entry_next_instruction_proven ||
        continuation_caller_next_entry_next.level_or_object_semantics_proven ||
        continuation_caller_next_entry_next.next_pc != 0x2201u ||
        continuation_caller_next_entry_next.next_physical_pc != 0x1f2201u ||
        continuation_caller_next_entry_next.next_opcode != 0xeau ||
        continuation_caller_next_entry_next.entry.call.termination.call.execution.transfer.source_checksum !=
            continuation_transfer.source_checksum) {
        free(raw);
        printf("FAIL: caller next entry successor was not source-bound\n");
        return 1;
    }
    *(strstr(game_payload_capture,
             "logical_pc=2201 physical_pc=1f2201") +
      strlen("logical_pc=220")) = '2';
    if (theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_entry_next(
            &handoff, game_payload_capture, &continuation_caller_next_entry_next)) {
        free(raw);
        printf("FAIL: altered caller next entry successor reached receipt\n");
        return 1;
    }
    snprintf(game_payload_capture, sizeof(game_payload_capture),
             "source=mednafen-pce-instrumented-main-ram-loader\n"
             "main_ram_loader_block_transfer logical_pc=3840 physical_pc=1f1840 operation=tii source=3c80 destination=2000 length=0020\n"
             "main_ram_loader_jsr logical_pc=3850 physical_pc=1f1850 target=2000 a=00 x=00 y=00\n"
             "main_ram_loader_rts logical_pc=2010 physical_pc=1f2010\n"
             "main_ram_loader_post_rts source_logical_pc=2010 source_physical_pc=1f2010 logical_pc=3853 physical_pc=1f1853 opcode=20\n"
             "main_ram_loader_jsr logical_pc=3853 physical_pc=1f1853 target=2100 a=00 x=00 y=00\n"
             "main_ram_loader_rts logical_pc=2110 physical_pc=1f2110\n"
             "main_ram_loader_post_rts source_logical_pc=2110 source_physical_pc=1f2110 logical_pc=3856 physical_pc=1f1856 opcode=ea\n"
             "main_ram_loader_jsr logical_pc=3858 physical_pc=1f1858 target=2200 a=00 x=00 y=00\n"
             "main_ram_loader_call_entry caller_logical_pc=3858 caller_physical_pc=1f1858 target=2200 logical_pc=2200 physical_pc=1f2200 opcode=ea\n"
             "main_ram_loader_entry_next entry_logical_pc=2200 entry_physical_pc=1f2200 logical_pc=2201 physical_pc=1f2201 opcode=73\n"
             "main_ram_loader_block_transfer logical_pc=2201 physical_pc=1f2201 operation=tii source=2008 destination=2400 length=0010\n"
             "main_ram_loader_jsr logical_pc=2204 physical_pc=1f2204 target=2400 a=00 x=00 y=00\n"
             "main_ram_loader_call_entry caller_logical_pc=2204 caller_physical_pc=1f2204 target=2400 logical_pc=2400 physical_pc=1f2400 opcode=%02x\n",
             transfer_destination_entry_opcode);
    if (!theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer(
            &handoff, game_payload_capture, &continuation_caller_next_transfer) ||
        !continuation_caller_next_transfer.valid ||
        !continuation_caller_next_transfer.source_track02_bytes_proven ||
        continuation_caller_next_transfer.level_or_object_semantics_proven ||
        continuation_caller_next_transfer.transfer_pc != 0x2201u ||
        continuation_caller_next_transfer.transfer_physical_pc != 0x1f2201u ||
        continuation_caller_next_transfer.source_address != 0x2008u ||
        continuation_caller_next_transfer.destination_address != 0x2400u ||
        continuation_caller_next_transfer.byte_count != 0x10u ||
        continuation_caller_next_transfer.original_source_address != 0x3c88u ||
        continuation_caller_next_transfer.source_checksum !=
            fnv1a_bytes(handoff.loader_post_envelope.bytes + 8u, 0x10u)) {
        free(raw);
        printf("FAIL: caller next TII was not bound to Track 02 bytes\n");
        return 1;
    }
    *(strstr(game_payload_capture,
             "source=2008 destination=2400 length=0010") +
      strlen("source=20")) = '2';
    if (theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer(
            &handoff, game_payload_capture, &continuation_caller_next_transfer)) {
        free(raw);
        printf("FAIL: altered caller next TII source reached receipt\n");
        return 1;
    }
    snprintf(game_payload_capture, sizeof(game_payload_capture),
             "source=mednafen-pce-instrumented-main-ram-loader\n"
             "main_ram_loader_block_transfer logical_pc=3840 physical_pc=1f1840 operation=tii source=3c80 destination=2000 length=0020\n"
             "main_ram_loader_jsr logical_pc=3850 physical_pc=1f1850 target=2000 a=00 x=00 y=00\n"
             "main_ram_loader_rts logical_pc=2010 physical_pc=1f2010\n"
             "main_ram_loader_post_rts source_logical_pc=2010 source_physical_pc=1f2010 logical_pc=3853 physical_pc=1f1853 opcode=20\n"
             "main_ram_loader_jsr logical_pc=3853 physical_pc=1f1853 target=2100 a=00 x=00 y=00\n"
             "main_ram_loader_rts logical_pc=2110 physical_pc=1f2110\n"
             "main_ram_loader_post_rts source_logical_pc=2110 source_physical_pc=1f2110 logical_pc=3856 physical_pc=1f1856 opcode=ea\n"
             "main_ram_loader_jsr logical_pc=3858 physical_pc=1f1858 target=2200 a=00 x=00 y=00\n"
             "main_ram_loader_call_entry caller_logical_pc=3858 caller_physical_pc=1f1858 target=2200 logical_pc=2200 physical_pc=1f2200 opcode=ea\n"
             "main_ram_loader_entry_next entry_logical_pc=2200 entry_physical_pc=1f2200 logical_pc=2201 physical_pc=1f2201 opcode=73\n"
             "main_ram_loader_block_transfer logical_pc=2201 physical_pc=1f2201 operation=tii source=2008 destination=2400 length=0010\n"
             "main_ram_loader_jsr logical_pc=2204 physical_pc=1f2204 target=2400 a=00 x=00 y=00\n");
    if (!theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call(
            &handoff, game_payload_capture, &continuation_caller_next_transfer_call) ||
        !continuation_caller_next_transfer_call.valid ||
        !continuation_caller_next_transfer_call.transfer_destination_call_proven ||
        continuation_caller_next_transfer_call.level_or_object_semantics_proven ||
        continuation_caller_next_transfer_call.call_pc != 0x2204u ||
        continuation_caller_next_transfer_call.call_physical_pc != 0x1f2204u ||
        continuation_caller_next_transfer_call.call_target != 0x2400u ||
        continuation_caller_next_transfer_call.transfer.source_checksum !=
            fnv1a_bytes(handoff.loader_post_envelope.bytes + 8u, 0x10u)) {
        free(raw);
        printf("FAIL: caller next TII destination call was not source-bound\n");
        return 1;
    }
    *(strstr(game_payload_capture,
             "logical_pc=2204 physical_pc=1f2204 target=2400") +
      strlen("logical_pc=220")) = '5';
    if (theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call(
            &handoff, game_payload_capture, &continuation_caller_next_transfer_call)) {
        free(raw);
        printf("FAIL: altered caller next TII destination call reached receipt\n");
        return 1;
    }
    snprintf(game_payload_capture, sizeof(game_payload_capture),
             "source=mednafen-pce-instrumented-main-ram-loader\n"
             "main_ram_loader_block_transfer logical_pc=3840 physical_pc=1f1840 operation=tii source=3c80 destination=2000 length=0020\n"
             "main_ram_loader_jsr logical_pc=3850 physical_pc=1f1850 target=2000 a=00 x=00 y=00\n"
             "main_ram_loader_rts logical_pc=2010 physical_pc=1f2010\n"
             "main_ram_loader_post_rts source_logical_pc=2010 source_physical_pc=1f2010 logical_pc=3853 physical_pc=1f1853 opcode=20\n"
             "main_ram_loader_jsr logical_pc=3853 physical_pc=1f1853 target=2100 a=00 x=00 y=00\n"
             "main_ram_loader_rts logical_pc=2110 physical_pc=1f2110\n"
             "main_ram_loader_post_rts source_logical_pc=2110 source_physical_pc=1f2110 logical_pc=3856 physical_pc=1f1856 opcode=ea\n"
             "main_ram_loader_jsr logical_pc=3858 physical_pc=1f1858 target=2200 a=00 x=00 y=00\n"
             "main_ram_loader_call_entry caller_logical_pc=3858 caller_physical_pc=1f1858 target=2200 logical_pc=2200 physical_pc=1f2200 opcode=ea\n"
             "main_ram_loader_entry_next entry_logical_pc=2200 entry_physical_pc=1f2200 logical_pc=2201 physical_pc=1f2201 opcode=73\n"
             "main_ram_loader_block_transfer logical_pc=2201 physical_pc=1f2201 operation=tii source=2008 destination=2400 length=0010\n"
             "main_ram_loader_jsr logical_pc=2204 physical_pc=1f2204 target=2400 a=00 x=00 y=00\n"
             "main_ram_loader_call_entry caller_logical_pc=2204 caller_physical_pc=1f2204 target=2400 logical_pc=2400 physical_pc=1f2400 opcode=ea\n");
    if (!theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry(
            &handoff, game_payload_capture, &continuation_caller_next_transfer_call_entry) ||
        !continuation_caller_next_transfer_call_entry.valid ||
        !continuation_caller_next_transfer_call_entry.transfer_destination_call_entry_proven ||
        continuation_caller_next_transfer_call_entry.level_or_object_semantics_proven ||
        continuation_caller_next_transfer_call_entry.entry_pc != 0x2400u ||
        continuation_caller_next_transfer_call_entry.entry_physical_pc != 0x1f2400u ||
        continuation_caller_next_transfer_call_entry.entry_opcode !=
            transfer_destination_entry_opcode ||
        continuation_caller_next_transfer_call_entry.call.transfer.source_checksum !=
            fnv1a_bytes(handoff.loader_post_envelope.bytes + 8u, 0x10u)) {
        free(raw);
        printf("FAIL: caller next TII destination entry was not source-bound\n");
        return 1;
    }
    *(strstr(game_payload_capture,
             "target=2400 logical_pc=2400 physical_pc=1f2400") +
      strlen("target=2400 logical_pc=240")) = '1';
    if (theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry(
            &handoff, game_payload_capture, &continuation_caller_next_transfer_call_entry)) {
        free(raw);
        printf("FAIL: altered caller next TII destination entry reached receipt\n");
        return 1;
    }
    snprintf(game_payload_capture, sizeof(game_payload_capture),
             "source=mednafen-pce-instrumented-main-ram-loader\n"
             "main_ram_loader_block_transfer logical_pc=3840 physical_pc=1f1840 operation=tii source=3c80 destination=2000 length=0020\n"
             "main_ram_loader_jsr logical_pc=3850 physical_pc=1f1850 target=2000 a=00 x=00 y=00\n"
             "main_ram_loader_rts logical_pc=2010 physical_pc=1f2010\n"
             "main_ram_loader_post_rts source_logical_pc=2010 source_physical_pc=1f2010 logical_pc=3853 physical_pc=1f1853 opcode=20\n"
             "main_ram_loader_jsr logical_pc=3853 physical_pc=1f1853 target=2100 a=00 x=00 y=00\n"
             "main_ram_loader_rts logical_pc=2110 physical_pc=1f2110\n"
             "main_ram_loader_post_rts source_logical_pc=2110 source_physical_pc=1f2110 logical_pc=3856 physical_pc=1f1856 opcode=ea\n"
             "main_ram_loader_jsr logical_pc=3858 physical_pc=1f1858 target=2200 a=00 x=00 y=00\n"
             "main_ram_loader_call_entry caller_logical_pc=3858 caller_physical_pc=1f1858 target=2200 logical_pc=2200 physical_pc=1f2200 opcode=ea\n"
             "main_ram_loader_entry_next entry_logical_pc=2200 entry_physical_pc=1f2200 logical_pc=2201 physical_pc=1f2201 opcode=73\n"
             "main_ram_loader_block_transfer logical_pc=2201 physical_pc=1f2201 operation=tii source=2008 destination=2400 length=0010\n"
             "main_ram_loader_jsr logical_pc=2204 physical_pc=1f2204 target=2400 a=00 x=00 y=00\n"
             "main_ram_loader_call_entry caller_logical_pc=2204 caller_physical_pc=1f2204 target=2400 logical_pc=2400 physical_pc=1f2400 opcode=%02x\n",
             transfer_destination_entry_opcode);
    if (!theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry_copy(
            &handoff, game_payload_capture,
            &continuation_caller_next_transfer_call_entry_copy) ||
        !continuation_caller_next_transfer_call_entry_copy.valid ||
        !continuation_caller_next_transfer_call_entry_copy.copied_source_byte_proven ||
        continuation_caller_next_transfer_call_entry_copy.level_or_object_semantics_proven ||
        continuation_caller_next_transfer_call_entry_copy.copied_source_address != 0x2008u ||
        continuation_caller_next_transfer_call_entry_copy.original_source_address != 0x3c88u ||
        continuation_caller_next_transfer_call_entry_copy.copied_source_byte !=
            transfer_destination_entry_opcode) {
        free(raw);
        printf("FAIL: caller next TII destination entry was not bound to its copied byte\n");
        return 1;
    }
    ++handoff.loader_post_envelope.bytes[8u];
    if (theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry_copy(
            &handoff, game_payload_capture,
            &continuation_caller_next_transfer_call_entry_copy)) {
        free(raw);
        printf("FAIL: altered copied source byte reached destination entry receipt\n");
        return 1;
    }
    --handoff.loader_post_envelope.bytes[8u];
    snprintf(game_payload_capture, sizeof(game_payload_capture),
             "source=mednafen-pce-instrumented-main-ram-loader\n"
             "main_ram_loader_block_transfer logical_pc=3840 physical_pc=1f1840 operation=tii source=3c80 destination=2000 length=0020\n"
             "main_ram_loader_jsr logical_pc=3850 physical_pc=1f1850 target=2000 a=00 x=00 y=00\n"
             "main_ram_loader_rts logical_pc=2010 physical_pc=1f2010\n"
             "main_ram_loader_post_rts source_logical_pc=2010 source_physical_pc=1f2010 logical_pc=3853 physical_pc=1f1853 opcode=20\n"
             "main_ram_loader_jsr logical_pc=3853 physical_pc=1f1853 target=2100 a=00 x=00 y=00\n"
             "main_ram_loader_rts logical_pc=2110 physical_pc=1f2110\n"
             "main_ram_loader_post_rts source_logical_pc=2110 source_physical_pc=1f2110 logical_pc=3856 physical_pc=1f1856 opcode=ea\n"
             "main_ram_loader_jsr logical_pc=3858 physical_pc=1f1858 target=2200 a=00 x=00 y=00\n"
             "main_ram_loader_call_entry caller_logical_pc=3858 caller_physical_pc=1f1858 target=2200 logical_pc=2200 physical_pc=1f2200 opcode=ea\n"
             "main_ram_loader_entry_next entry_logical_pc=2200 entry_physical_pc=1f2200 logical_pc=2201 physical_pc=1f2201 opcode=73\n"
             "main_ram_loader_block_transfer logical_pc=2201 physical_pc=1f2201 operation=tii source=2008 destination=2400 length=0010\n"
             "main_ram_loader_jsr logical_pc=2204 physical_pc=1f2204 target=2400 a=00 x=00 y=00\n"
             "main_ram_loader_call_entry caller_logical_pc=2204 caller_physical_pc=1f2204 target=2400 logical_pc=2400 physical_pc=1f2400 opcode=%02x\n"
             "main_ram_loader_entry_next entry_logical_pc=2400 entry_physical_pc=1f2400 logical_pc=2401 physical_pc=1f2401 opcode=%02x\n",
             transfer_destination_entry_opcode,
             transfer_destination_entry_next_opcode);
    if (!theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry_copy_next(
            &handoff, game_payload_capture,
            &continuation_caller_next_transfer_call_entry_copy_next) ||
        !continuation_caller_next_transfer_call_entry_copy_next.valid ||
        !continuation_caller_next_transfer_call_entry_copy_next.copied_successor_byte_proven ||
        continuation_caller_next_transfer_call_entry_copy_next.level_or_object_semantics_proven ||
        continuation_caller_next_transfer_call_entry_copy_next.next_pc != 0x2401u ||
        continuation_caller_next_transfer_call_entry_copy_next.next_physical_pc !=
            0x1f2401u ||
        continuation_caller_next_transfer_call_entry_copy_next.original_source_address !=
            0x3c89u ||
        continuation_caller_next_transfer_call_entry_copy_next.next_source_byte !=
            transfer_destination_entry_next_opcode) {
        free(raw);
        printf("FAIL: copied destination entry successor was not source-bound\n");
        return 1;
    }
    ++handoff.loader_post_envelope.bytes[9u];
    if (theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry_copy_next(
            &handoff, game_payload_capture,
            &continuation_caller_next_transfer_call_entry_copy_next)) {
        free(raw);
        printf("FAIL: altered copied successor byte reached entry receipt\n");
        return 1;
    }
    --handoff.loader_post_envelope.bytes[9u];
    snprintf(game_payload_capture, sizeof(game_payload_capture),
             "source=mednafen-pce-instrumented-main-ram-loader\n"
             "main_ram_loader_block_transfer logical_pc=3840 physical_pc=1f1840 operation=tii source=3c80 destination=2000 length=0020\n"
             "main_ram_loader_jsr logical_pc=3850 physical_pc=1f1850 target=2000 a=00 x=00 y=00\n"
             "main_ram_loader_rts logical_pc=2010 physical_pc=1f2010\n"
             "main_ram_loader_post_rts source_logical_pc=2010 source_physical_pc=1f2010 logical_pc=3853 physical_pc=1f1853 opcode=20\n"
             "main_ram_loader_jsr logical_pc=3853 physical_pc=1f1853 target=2100 a=00 x=00 y=00\n"
             "main_ram_loader_rts logical_pc=2110 physical_pc=1f2110\n"
             "main_ram_loader_post_rts source_logical_pc=2110 source_physical_pc=1f2110 logical_pc=3856 physical_pc=1f1856 opcode=ea\n"
             "main_ram_loader_jsr logical_pc=3858 physical_pc=1f1858 target=2200 a=00 x=00 y=00\n"
             "main_ram_loader_call_entry caller_logical_pc=3858 caller_physical_pc=1f1858 target=2200 logical_pc=2200 physical_pc=1f2200 opcode=ea\n"
             "main_ram_loader_entry_next entry_logical_pc=2200 entry_physical_pc=1f2200 logical_pc=2201 physical_pc=1f2201 opcode=73\n"
             "main_ram_loader_block_transfer logical_pc=2201 physical_pc=1f2201 operation=tii source=2008 destination=2400 length=0010\n"
             "main_ram_loader_jsr logical_pc=2204 physical_pc=1f2204 target=2400 a=00 x=00 y=00\n"
             "main_ram_loader_call_entry caller_logical_pc=2204 caller_physical_pc=1f2204 target=2400 logical_pc=2400 physical_pc=1f2400 opcode=%02x\n"
             "main_ram_loader_entry_next entry_logical_pc=2400 entry_physical_pc=1f2400 logical_pc=2401 physical_pc=1f2401 opcode=%02x\n"
             "main_ram_loader_entry_successor_next successor_logical_pc=2401 successor_physical_pc=1f2401 logical_pc=2402 physical_pc=1f2402 opcode=%02x\n",
             transfer_destination_entry_opcode,
             transfer_destination_entry_next_opcode,
             transfer_destination_entry_successor_opcode);
    if (!theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry_copy_successor(
            &handoff, game_payload_capture,
            &continuation_caller_next_transfer_call_entry_copy_successor) ||
        !continuation_caller_next_transfer_call_entry_copy_successor.valid ||
        !continuation_caller_next_transfer_call_entry_copy_successor.copied_successor_next_byte_proven ||
        continuation_caller_next_transfer_call_entry_copy_successor.level_or_object_semantics_proven ||
        continuation_caller_next_transfer_call_entry_copy_successor.next_pc != 0x2402u ||
        continuation_caller_next_transfer_call_entry_copy_successor.next_physical_pc !=
            0x1f2402u ||
        continuation_caller_next_transfer_call_entry_copy_successor.original_source_address !=
            0x3c8au ||
        continuation_caller_next_transfer_call_entry_copy_successor.next_source_byte !=
            transfer_destination_entry_successor_opcode) {
        free(raw);
        printf("FAIL: copied destination successor was not source-bound\n");
        return 1;
    }
    ++handoff.loader_post_envelope.bytes[10u];
    if (theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry_copy_successor(
            &handoff, game_payload_capture,
            &continuation_caller_next_transfer_call_entry_copy_successor)) {
        free(raw);
        printf("FAIL: altered copied successor source reached entry receipt\n");
        return 1;
    }
    --handoff.loader_post_envelope.bytes[10u];
    snprintf(game_payload_capture, sizeof(game_payload_capture),
             "source=mednafen-pce-instrumented-main-ram-loader\n"
             "main_ram_loader_block_transfer logical_pc=3840 physical_pc=1f1840 operation=tii source=3c80 destination=2000 length=0020\n"
             "main_ram_loader_jsr logical_pc=3850 physical_pc=1f1850 target=2000 a=00 x=00 y=00\n"
             "main_ram_loader_rts logical_pc=2010 physical_pc=1f2010\n"
             "main_ram_loader_post_rts source_logical_pc=2010 source_physical_pc=1f2010 logical_pc=3853 physical_pc=1f1853 opcode=20\n"
             "main_ram_loader_jsr logical_pc=3853 physical_pc=1f1853 target=2100 a=00 x=00 y=00\n"
             "main_ram_loader_rts logical_pc=2110 physical_pc=1f2110\n"
             "main_ram_loader_post_rts source_logical_pc=2110 source_physical_pc=1f2110 logical_pc=3856 physical_pc=1f1856 opcode=ea\n"
             "main_ram_loader_jsr logical_pc=3858 physical_pc=1f1858 target=2200 a=00 x=00 y=00\n"
             "main_ram_loader_call_entry caller_logical_pc=3858 caller_physical_pc=1f1858 target=2200 logical_pc=2200 physical_pc=1f2200 opcode=ea\n"
             "main_ram_loader_entry_next entry_logical_pc=2200 entry_physical_pc=1f2200 logical_pc=2201 physical_pc=1f2201 opcode=73\n"
             "main_ram_loader_block_transfer logical_pc=2201 physical_pc=1f2201 operation=tii source=2008 destination=2400 length=0010\n"
             "main_ram_loader_jsr logical_pc=2204 physical_pc=1f2204 target=2400 a=00 x=00 y=00\n"
             "main_ram_loader_call_entry caller_logical_pc=2204 caller_physical_pc=1f2204 target=2400 logical_pc=2400 physical_pc=1f2400 opcode=%02x\n"
             "main_ram_loader_bra logical_pc=2400 physical_pc=1f2400 target=%04x displacement=%02x\n"
             "main_ram_loader_bra_target source_logical_pc=2400 source_physical_pc=1f2400 target=%04x logical_pc=%04x physical_pc=%06x opcode=ea\n"
             "main_ram_loader_bra_target_jsr branch_target=%04x branch_target_physical_pc=%06x logical_pc=%04x physical_pc=%06x target=%04x\n",
             transfer_destination_entry_opcode,
             transfer_destination_branch_target,
             transfer_destination_branch_displacement,
             transfer_destination_branch_target,
             transfer_destination_branch_target,
             0x1f0000u + transfer_destination_branch_target,
             transfer_destination_branch_target,
             0x1f0000u + transfer_destination_branch_target,
             transfer_destination_branch_target_jsr_pc,
             0x1f0000u + transfer_destination_branch_target_jsr_pc,
             transfer_destination_branch_target_jsr_target);
    {
        size_t capture_length = strlen(game_payload_capture);
        snprintf(game_payload_capture + capture_length,
                 sizeof(game_payload_capture) - capture_length,
                 "pce_cd_register_write cpu_pc=%04x physical=00001801 data=08\n"
                 "scsi_read_command generation=17 opcode=08 cdb=08%02x%02x%02x0100 start_lba=%u sector_count=1\n"
                 "pce_cd_fifo_origin_main_ram_receipt generation=17 source_lba=%u source_offset=%u fifo_sequence=23 reader_pc=e981 logical_destination=3000 physical_destination=1f1000 writer_pc=1840 writer_physical_pc=1f1840 value=%02x\n",
                 transfer_destination_branch_target_jsr_target,
                 (transfer_destination_branch_target_lba >> 16) & 0x1fu,
                 (transfer_destination_branch_target_lba >> 8) & 0xffu,
                 transfer_destination_branch_target_lba & 0xffu,
                 transfer_destination_branch_target_lba,
                 transfer_destination_branch_target_lba,
                 transfer_destination_branch_target_source_offset,
                 transfer_destination_branch_target_source_byte);
    }
    if (!theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry_branch(
            &handoff, game_payload_capture,
            &continuation_caller_next_transfer_call_entry_branch) ||
        !continuation_caller_next_transfer_call_entry_branch.valid ||
        !continuation_caller_next_transfer_call_entry_branch.copied_entry_branch_proven ||
        continuation_caller_next_transfer_call_entry_branch.level_or_object_semantics_proven ||
        continuation_caller_next_transfer_call_entry_branch.target_pc !=
            transfer_destination_branch_target ||
        continuation_caller_next_transfer_call_entry_branch.displacement !=
            transfer_destination_branch_displacement ||
        continuation_caller_next_transfer_call_entry_branch.original_displacement_address !=
            0x3c89u) {
        free(raw);
        printf("FAIL: copied destination BRA was not source-bound\n");
        return 1;
    }
    ++handoff.loader_post_envelope.bytes[9u];
    if (theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry_branch(
            &handoff, game_payload_capture,
            &continuation_caller_next_transfer_call_entry_branch)) {
        free(raw);
        printf("FAIL: altered copied BRA displacement reached receipt\n");
        return 1;
    }
    --handoff.loader_post_envelope.bytes[9u];
    if (!theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry_branch_target_jsr_cd(
            &handoff, game_payload_capture, raw, raw_size, md5,
            &continuation_caller_next_transfer_call_entry_branch_target_jsr_cd) ||
        !continuation_caller_next_transfer_call_entry_branch_target_jsr_cd.valid ||
        !continuation_caller_next_transfer_call_entry_branch_target_jsr_cd.jsr_cd_register_write_observed ||
        !continuation_caller_next_transfer_call_entry_branch_target_jsr_cd.read6_record_source_verified ||
        continuation_caller_next_transfer_call_entry_branch_target_jsr_cd.level_or_object_semantics_proven ||
        continuation_caller_next_transfer_call_entry_branch_target_jsr_cd.track02_record !=
            handoff.observed_track02_record ||
        continuation_caller_next_transfer_call_entry_branch_target_jsr_cd.scsi_lba !=
            transfer_destination_branch_target_lba ||
        continuation_caller_next_transfer_call_entry_branch_target_jsr_cd.source_offset !=
            transfer_destination_branch_target_source_offset ||
        continuation_caller_next_transfer_call_entry_branch_target_jsr_cd.source_byte !=
            transfer_destination_branch_target_source_byte) {
        free(raw);
        printf("FAIL: post-BRA JSR CD read did not bind a Track 02 record\n");
        return 1;
    }
    ++handoff.loader_post_envelope.bytes[9u];
    if (theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry_branch_target_jsr_cd(
            &handoff, game_payload_capture, raw, raw_size, md5,
            &continuation_caller_next_transfer_call_entry_branch_target_jsr_cd)) {
        free(raw);
        printf("FAIL: altered copied BRA target reached CD record receipt\n");
        return 1;
    }
    --handoff.loader_post_envelope.bytes[9u];
    /* The loader's later consumer read of the exact joined FIFO byte, and its
     * first observed control transfer after that read. Both remain opaque
     * byte/control observations with no record or payload semantics. */
    {
        size_t capture_length = strlen(game_payload_capture);
        snprintf(game_payload_capture + capture_length,
                 sizeof(game_payload_capture) - capture_length,
                 "pce_cd_fifo_origin_main_ram_consumer sequence=0 generation=17 source_lba=%u source_offset=%u fifo_sequence=23 logical_address=3000 physical_address=1f1000 value=%02x reader_pc=2450 reader_physical_pc=1f2450\n"
                 "main_ram_loader_jsr logical_pc=2453 physical_pc=1f2453 target=2600 a=00 x=00 y=00\n",
                 transfer_destination_branch_target_lba,
                 transfer_destination_branch_target_source_offset,
                 transfer_destination_branch_target_source_byte);
    }
    if (!theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer(
            &handoff, game_payload_capture, raw, raw_size, md5,
            &continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer) ||
        !continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer.valid ||
        !continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer.loader_consumer_read_proven ||
        continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer.level_or_object_semantics_proven ||
        continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer.consumer_generation != 17u ||
        continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer.consumer_lba !=
            transfer_destination_branch_target_lba ||
        continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer.source_offset !=
            transfer_destination_branch_target_source_offset ||
        continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer.source_byte !=
            transfer_destination_branch_target_source_byte ||
        continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer.consumer_physical_address !=
            0x1f1000u ||
        continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer.consumer_reader_pc !=
            0x2450u ||
        continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer.consumer_reader_physical_pc !=
            0x1f2450u) {
        free(raw);
        printf("FAIL: loader consumer read of the joined FIFO byte was not bound\n");
        return 1;
    }
    ++handoff.loader_post_envelope.bytes[9u];
    if (theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer(
            &handoff, game_payload_capture, raw, raw_size, md5,
            &continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer)) {
        free(raw);
        printf("FAIL: altered copied byte reached the consumer receipt\n");
        return 1;
    }
    --handoff.loader_post_envelope.bytes[9u];
    {
        char *consumer_row = strstr(game_payload_capture,
                                    "pce_cd_fifo_origin_main_ram_consumer ");
        char *value_field;
        char saved_value[3];
        if (!consumer_row) {
            free(raw);
            printf("FAIL: consumer row missing from the synthetic capture\n");
            return 1;
        }
        value_field = strstr(consumer_row, "value=");
        if (!value_field) {
            free(raw);
            printf("FAIL: consumer row value missing from the synthetic capture\n");
            return 1;
        }
        saved_value[0] = value_field[6];
        saved_value[1] = value_field[7];
        saved_value[2] = '\0';
        value_field[6] = transfer_destination_branch_target_source_byte != 0xffu ? 'f' : '0';
        value_field[7] = transfer_destination_branch_target_source_byte != 0xffu ? 'f' : '0';
        if (theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer(
                &handoff, game_payload_capture, raw, raw_size, md5,
                &continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer)) {
            free(raw);
            printf("FAIL: consumer of a different FIFO byte reached the consumer receipt\n");
            return 1;
        }
        value_field[6] = saved_value[0];
        value_field[7] = saved_value[1];
    }
    {
        char *reader_field = strstr(game_payload_capture,
                                    "reader_physical_pc=1f2450");
        if (!reader_field) {
            free(raw);
            printf("FAIL: consumer reader missing from the synthetic capture\n");
            return 1;
        }
        memcpy(reader_field, "reader_physical_pc=000a52", 25u);
        if (theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer(
                &handoff, game_payload_capture, raw, raw_size, md5,
                &continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer)) {
            free(raw);
            printf("FAIL: System Card consumer reader reached the consumer receipt\n");
            return 1;
        }
        memcpy(reader_field, "reader_physical_pc=1f2450", 25u);
    }
    if (!theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control(
            &handoff, game_payload_capture, raw, raw_size, md5,
            &continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control) ||
        !continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control.valid ||
        !continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control.consumer_control_transfer_proven ||
        continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control.level_or_object_semantics_proven ||
        continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control.control_pc != 0x2453u ||
        continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control.control_physical_pc !=
            0x1f2453u ||
        continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control.control_target != 0x2600u ||
        continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control.consumer.consumer_reader_pc !=
            0x2450u) {
        free(raw);
        printf("FAIL: loader control transfer after the consumer read was not bound\n");
        return 1;
    }
    ++handoff.loader_post_envelope.bytes[9u];
    if (theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control(
            &handoff, game_payload_capture, raw, raw_size, md5,
            &continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control)) {
        free(raw);
        printf("FAIL: altered copied byte reached the control-transfer receipt\n");
        return 1;
    }
    --handoff.loader_post_envelope.bytes[9u];
    {
        char *control_row = strstr(game_payload_capture,
                                   "main_ram_loader_jsr logical_pc=2453 ");
        char saved_char;
        if (!control_row) {
            free(raw);
            printf("FAIL: control row missing from the synthetic capture\n");
            return 1;
        }
        saved_char = *control_row;
        *control_row = '\0';
        if (theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control(
                &handoff, game_payload_capture, raw, raw_size, md5,
                &continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control)) {
            free(raw);
            printf("FAIL: capture without a control transfer reached the control receipt\n");
            return 1;
        }
        *control_row = saved_char;
    }
    {
        char *physical_field = strstr(game_payload_capture,
                                      "main_ram_loader_jsr logical_pc=2453 physical_pc=1f2453");
        if (!physical_field) {
            free(raw);
            printf("FAIL: control physical row missing from the synthetic capture\n");
            return 1;
        }
        memcpy(physical_field + strlen("main_ram_loader_jsr logical_pc=2453 physical_pc="),
               "000a53", 6u);
        if (theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control(
                &handoff, game_payload_capture, raw, raw_size, md5,
                &continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control)) {
            free(raw);
            printf("FAIL: non-main-RAM control transfer reached the control receipt\n");
            return 1;
        }
        memcpy(physical_field + strlen("main_ram_loader_jsr logical_pc=2453 physical_pc="),
               "1f2453", 6u);
    }
    /* The control routine's observed execution entry, the next main-RAM
     * instruction after that entry, and the bounded return that resumes the
     * consumer continuation at the exact call return address. All remain
     * opaque execution facts with no record or payload semantics. */
    {
        size_t capture_length = strlen(game_payload_capture);
        snprintf(game_payload_capture + capture_length,
                 sizeof(game_payload_capture) - capture_length,
                 "main_ram_loader_call_entry caller_logical_pc=2453 caller_physical_pc=1f2453 target=2600 logical_pc=2600 physical_pc=1f2600 opcode=20\n"
                 "main_ram_loader_entry_next entry_logical_pc=2600 entry_physical_pc=1f2600 logical_pc=2601 physical_pc=1f2601 opcode=a9\n"
                 "main_ram_loader_rts logical_pc=2610 physical_pc=1f2610\n"
                 "main_ram_loader_post_rts source_logical_pc=2610 source_physical_pc=1f2610 logical_pc=2456 physical_pc=1f2456 opcode=d8\n");
    }
    if (!theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_entry(
            &handoff, game_payload_capture, raw, raw_size, md5,
            &continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_entry) ||
        !continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_entry.valid ||
        !continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_entry.consumer_control_entry_proven ||
        continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_entry.level_or_object_semantics_proven ||
        continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_entry.entry_pc != 0x2600u ||
        continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_entry.entry_physical_pc != 0x1f2600u ||
        continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_entry.entry_opcode != 0x20u ||
        continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_entry.control.control_pc != 0x2453u) {
        free(raw);
        printf("FAIL: control transfer target execution entry was not bound\n");
        return 1;
    }
    ++handoff.loader_post_envelope.bytes[9u];
    if (theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_entry(
            &handoff, game_payload_capture, raw, raw_size, md5,
            &continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_entry)) {
        free(raw);
        printf("FAIL: altered copied byte reached the control entry receipt\n");
        return 1;
    }
    --handoff.loader_post_envelope.bytes[9u];
    {
        char *entry_row = strstr(game_payload_capture,
                                 "main_ram_loader_call_entry caller_logical_pc=2453 ");
        char saved_char;
        if (!entry_row) {
            free(raw);
            printf("FAIL: control entry row missing from the synthetic capture\n");
            return 1;
        }
        saved_char = *entry_row;
        *entry_row = '\0';
        if (theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_entry(
                &handoff, game_payload_capture, raw, raw_size, md5,
                &continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_entry)) {
            free(raw);
            printf("FAIL: capture without a control entry reached the entry receipt\n");
            return 1;
        }
        *entry_row = saved_char;
    }
    {
        char *entry_physical_field = strstr(game_payload_capture,
                                            "target=2600 logical_pc=2600 physical_pc=1f2600");
        if (!entry_physical_field) {
            free(raw);
            printf("FAIL: control entry physical row missing from the synthetic capture\n");
            return 1;
        }
        memcpy(entry_physical_field + strlen("target=2600 logical_pc=2600 physical_pc="),
               "000a00", 6u);
        if (theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_entry(
                &handoff, game_payload_capture, raw, raw_size, md5,
                &continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_entry)) {
            free(raw);
            printf("FAIL: non-main-RAM control entry reached the entry receipt\n");
            return 1;
        }
        memcpy(entry_physical_field + strlen("target=2600 logical_pc=2600 physical_pc="),
               "1f2600", 6u);
    }
    if (!theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_entry_next(
            &handoff, game_payload_capture, raw, raw_size, md5,
            &continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_entry_next) ||
        !continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_entry_next.valid ||
        !continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_entry_next.consumer_control_entry_next_proven ||
        continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_entry_next.level_or_object_semantics_proven ||
        continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_entry_next.next_pc != 0x2601u ||
        continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_entry_next.next_physical_pc != 0x1f2601u ||
        continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_entry_next.next_opcode != 0xa9u ||
        continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_entry_next.entry.entry_pc != 0x2600u) {
        free(raw);
        printf("FAIL: control entry successor instruction was not bound\n");
        return 1;
    }
    ++handoff.loader_post_envelope.bytes[9u];
    if (theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_entry_next(
            &handoff, game_payload_capture, raw, raw_size, md5,
            &continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_entry_next)) {
        free(raw);
        printf("FAIL: altered copied byte reached the control entry-next receipt\n");
        return 1;
    }
    --handoff.loader_post_envelope.bytes[9u];
    if (!theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return(
            &handoff, game_payload_capture, raw, raw_size, md5,
            &continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return) ||
        !continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return.valid ||
        !continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return.consumer_control_return_proven ||
        continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return.level_or_object_semantics_proven ||
        continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return.return_instruction_pc != 0x2610u ||
        continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return.return_instruction_physical_pc != 0x1f2610u ||
        continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return.post_return_pc != 0x2456u ||
        continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return.post_return_physical_pc != 0x1f2456u ||
        continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return.post_return_opcode != 0xd8u ||
        continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return.next.next_pc != 0x2601u) {
        free(raw);
        printf("FAIL: control routine bounded return was not bound\n");
        return 1;
    }
    ++handoff.loader_post_envelope.bytes[9u];
    if (theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return(
            &handoff, game_payload_capture, raw, raw_size, md5,
            &continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return)) {
        free(raw);
        printf("FAIL: altered copied byte reached the control return receipt\n");
        return 1;
    }
    --handoff.loader_post_envelope.bytes[9u];
    {
        char *resume_field = strstr(game_payload_capture,
                                    "post_rts source_logical_pc=2610 source_physical_pc=1f2610 logical_pc=2456");
        if (!resume_field) {
            free(raw);
            printf("FAIL: control resume row missing from the synthetic capture\n");
            return 1;
        }
        memcpy(resume_field + strlen("post_rts source_logical_pc=2610 source_physical_pc=1f2610 logical_pc="),
               "2457", 4u);
        if (theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return(
                &handoff, game_payload_capture, raw, raw_size, md5,
                &continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return)) {
            free(raw);
            printf("FAIL: off-target control resume reached the return receipt\n");
            return 1;
        }
        memcpy(resume_field + strlen("post_rts source_logical_pc=2610 source_physical_pc=1f2610 logical_pc="),
               "2456", 4u);
    }
    /* The resumed loader path: the consumer read of the FIFO byte adjacent
     * to the first bound consumer byte, observed after the bounded control
     * return, plus its own following control transfer and that transfer's
     * observed execution entry. All remain opaque byte/control facts. */
    {
        uint16_t resumed_source_offset =
            (uint16_t)(transfer_destination_branch_target_source_offset + 1u);
        uint8_t resumed_source_byte = raw[
            (size_t)handoff.observed_track02_record * THERON_TRACK02_RAW_SECTOR_BYTES +
            resumed_source_offset];
        size_t capture_length = strlen(game_payload_capture);
        snprintf(game_payload_capture + capture_length,
                 sizeof(game_payload_capture) - capture_length,
                 "pce_cd_fifo_origin_main_ram_receipt generation=17 source_lba=%u source_offset=%u fifo_sequence=24 reader_pc=e981 logical_destination=3001 physical_destination=1f1001 writer_pc=1840 writer_physical_pc=1f1840 value=%02x\n"
                 "pce_cd_fifo_origin_main_ram_consumer sequence=1 generation=17 source_lba=%u source_offset=%u fifo_sequence=24 logical_address=3001 physical_address=1f1001 value=%02x reader_pc=2456 reader_physical_pc=1f2456\n"
                 "main_ram_loader_jsr logical_pc=2459 physical_pc=1f2459 target=2700 a=00 x=00 y=00\n"
                 "main_ram_loader_call_entry caller_logical_pc=2459 caller_physical_pc=1f2459 target=2700 logical_pc=2700 physical_pc=1f2700 opcode=4c\n",
                 transfer_destination_branch_target_lba, resumed_source_offset,
                 resumed_source_byte, transfer_destination_branch_target_lba,
                 resumed_source_offset, resumed_source_byte);
        if (!theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer(
                &handoff, game_payload_capture, raw, raw_size, md5,
                &continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer) ||
            !continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer.valid ||
            !continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer.resumed_loader_consumer_read_proven ||
            continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer.level_or_object_semantics_proven ||
            continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer.consumer_generation != 17u ||
            continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer.consumer_lba !=
                transfer_destination_branch_target_lba ||
            continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer.track02_record !=
                handoff.observed_track02_record ||
            continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer.source_offset !=
                resumed_source_offset ||
            continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer.source_byte !=
                resumed_source_byte ||
            continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer.consumer_physical_address !=
                0x1f1001u ||
            continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer.consumer_reader_pc !=
                0x2456u ||
            continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer.consumer_reader_physical_pc !=
                0x1f2456u) {
            free(raw);
            printf("FAIL: resumed loader consumer read was not bound\n");
            return 1;
        }
        ++handoff.loader_post_envelope.bytes[9u];
        if (theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer(
                &handoff, game_payload_capture, raw, raw_size, md5,
                &continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer)) {
            free(raw);
            printf("FAIL: altered copied byte reached the resumed consumer receipt\n");
            return 1;
        }
        --handoff.loader_post_envelope.bytes[9u];
        {
            char *resumed_reader_field = strstr(game_payload_capture,
                                                "reader_physical_pc=1f2456");
            if (!resumed_reader_field) {
                free(raw);
                printf("FAIL: resumed consumer reader missing from the synthetic capture\n");
                return 1;
            }
            memcpy(resumed_reader_field, "reader_physical_pc=000a56", 25u);
            if (theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer(
                    &handoff, game_payload_capture, raw, raw_size, md5,
                    &continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer)) {
                free(raw);
                printf("FAIL: System Card resumed reader reached the resumed consumer receipt\n");
                return 1;
            }
            memcpy(resumed_reader_field, "reader_physical_pc=1f2456", 25u);
        }
        if (!theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control(
                &handoff, game_payload_capture, raw, raw_size, md5,
                &continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control) ||
            !continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control.valid ||
            !continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control.resumed_consumer_control_transfer_proven ||
            continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control.level_or_object_semantics_proven ||
            continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control.control_pc != 0x2459u ||
            continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control.control_physical_pc != 0x1f2459u ||
            continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control.control_target != 0x2700u ||
            continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control.consumer.consumer_reader_pc != 0x2456u) {
            free(raw);
            printf("FAIL: resumed loader control transfer was not bound\n");
            return 1;
        }
        ++handoff.loader_post_envelope.bytes[9u];
        if (theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control(
                &handoff, game_payload_capture, raw, raw_size, md5,
                &continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control)) {
            free(raw);
            printf("FAIL: altered copied byte reached the resumed control receipt\n");
            return 1;
        }
        --handoff.loader_post_envelope.bytes[9u];
        {
            char *resumed_control_row = strstr(game_payload_capture,
                                               "main_ram_loader_jsr logical_pc=2459 ");
            char saved_char;
            if (!resumed_control_row) {
                free(raw);
                printf("FAIL: resumed control row missing from the synthetic capture\n");
                return 1;
            }
            saved_char = *resumed_control_row;
            *resumed_control_row = '\0';
            if (theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control(
                    &handoff, game_payload_capture, raw, raw_size, md5,
                    &continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control)) {
                free(raw);
                printf("FAIL: capture without a resumed control transfer reached the resumed control receipt\n");
                return 1;
            }
            *resumed_control_row = saved_char;
        }
        if (!theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control_entry(
                &handoff, game_payload_capture, raw, raw_size, md5,
                &continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control_entry) ||
            !continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control_entry.valid ||
            !continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control_entry.resumed_control_entry_proven ||
            continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control_entry.level_or_object_semantics_proven ||
            continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control_entry.entry_pc != 0x2700u ||
            continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control_entry.entry_physical_pc != 0x1f2700u ||
            continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control_entry.entry_opcode != 0x4cu ||
            continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control_entry.control.control_pc != 0x2459u) {
            free(raw);
            printf("FAIL: resumed control target execution entry was not bound\n");
            return 1;
        }
        ++handoff.loader_post_envelope.bytes[9u];
        if (theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control_entry(
                &handoff, game_payload_capture, raw, raw_size, md5,
                &continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control_entry)) {
            free(raw);
            printf("FAIL: altered copied byte reached the resumed control entry receipt\n");
            return 1;
        }
        --handoff.loader_post_envelope.bytes[9u];
        {
            char *resumed_entry_physical_field = strstr(game_payload_capture,
                                                        "target=2700 logical_pc=2700 physical_pc=1f2700");
            if (!resumed_entry_physical_field) {
                free(raw);
                printf("FAIL: resumed entry physical row missing from the synthetic capture\n");
                return 1;
            }
            memcpy(resumed_entry_physical_field + strlen("target=2700 logical_pc=2700 physical_pc="),
                   "000a00", 6u);
            if (theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control_entry(
                    &handoff, game_payload_capture, raw, raw_size, md5,
                    &continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control_entry)) {
                free(raw);
                printf("FAIL: non-main-RAM resumed control entry reached the resumed entry receipt\n");
                return 1;
            }
            memcpy(resumed_entry_physical_field + strlen("target=2700 logical_pc=2700 physical_pc="),
                   "1f2700", 6u);
        }
    }
    /* The resumed control routine's own bounded window and the loader path's
     * second resumption: the next main-RAM instruction after the resumed
     * entry, exactly one RTS resuming at the resumed control call return
     * address, and the twice-resumed consumer read of the second adjacent
     * FIFO byte. All remain opaque byte/control facts. */
    {
        uint16_t twice_resumed_source_offset =
            (uint16_t)(transfer_destination_branch_target_source_offset + 2u);
        uint8_t twice_resumed_source_byte = raw[
            (size_t)handoff.observed_track02_record * THERON_TRACK02_RAW_SECTOR_BYTES +
            twice_resumed_source_offset];
        size_t capture_length = strlen(game_payload_capture);
        snprintf(game_payload_capture + capture_length,
                 sizeof(game_payload_capture) - capture_length,
                 "main_ram_loader_entry_next entry_logical_pc=2700 entry_physical_pc=1f2700 logical_pc=2701 physical_pc=1f2701 opcode=8d\n"
                 "main_ram_loader_rts logical_pc=2710 physical_pc=1f2710\n"
                 "main_ram_loader_post_rts source_logical_pc=2710 source_physical_pc=1f2710 logical_pc=245c physical_pc=1f245c opcode=60\n"
                 "pce_cd_fifo_origin_main_ram_receipt generation=17 source_lba=%u source_offset=%u fifo_sequence=25 reader_pc=e981 logical_destination=3002 physical_destination=1f1002 writer_pc=1840 writer_physical_pc=1f1840 value=%02x\n"
                 "pce_cd_fifo_origin_main_ram_consumer sequence=2 generation=17 source_lba=%u source_offset=%u fifo_sequence=25 logical_address=3002 physical_address=1f1002 value=%02x reader_pc=245c reader_physical_pc=1f245c\n",
                 transfer_destination_branch_target_lba,
                 twice_resumed_source_offset, twice_resumed_source_byte,
                 transfer_destination_branch_target_lba,
                 twice_resumed_source_offset, twice_resumed_source_byte);
        if (!theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control_entry_next(
                &handoff, game_payload_capture, raw, raw_size, md5,
                &continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control_entry_next) ||
            !continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control_entry_next.valid ||
            !continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control_entry_next.resumed_control_entry_next_proven ||
            continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control_entry_next.level_or_object_semantics_proven ||
            continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control_entry_next.next_pc != 0x2701u ||
            continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control_entry_next.next_physical_pc != 0x1f2701u ||
            continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control_entry_next.next_opcode != 0x8du ||
            continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control_entry_next.entry.entry_pc != 0x2700u) {
            free(raw);
            printf("FAIL: resumed control entry successor instruction was not bound\n");
            return 1;
        }
        ++handoff.loader_post_envelope.bytes[9u];
        if (theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control_entry_next(
                &handoff, game_payload_capture, raw, raw_size, md5,
                &continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control_entry_next)) {
            free(raw);
            printf("FAIL: altered copied byte reached the resumed entry-next receipt\n");
            return 1;
        }
        --handoff.loader_post_envelope.bytes[9u];
        if (!theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control_return(
                &handoff, game_payload_capture, raw, raw_size, md5,
                &continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control_return) ||
            !continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control_return.valid ||
            !continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control_return.resumed_control_return_proven ||
            continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control_return.level_or_object_semantics_proven ||
            continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control_return.return_instruction_pc != 0x2710u ||
            continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control_return.return_instruction_physical_pc != 0x1f2710u ||
            continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control_return.post_return_pc != 0x245cu ||
            continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control_return.post_return_physical_pc != 0x1f245cu ||
            continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control_return.post_return_opcode != 0x60u) {
            free(raw);
            printf("FAIL: resumed control routine bounded return was not bound\n");
            return 1;
        }
        ++handoff.loader_post_envelope.bytes[9u];
        if (theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control_return(
                &handoff, game_payload_capture, raw, raw_size, md5,
                &continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control_return)) {
            free(raw);
            printf("FAIL: altered copied byte reached the resumed control return receipt\n");
            return 1;
        }
        --handoff.loader_post_envelope.bytes[9u];
        {
            char *resumed_resume_field = strstr(game_payload_capture,
                                                "post_rts source_logical_pc=2710 source_physical_pc=1f2710 logical_pc=245c");
            if (!resumed_resume_field) {
                free(raw);
                printf("FAIL: resumed control resume row missing from the synthetic capture\n");
                return 1;
            }
            memcpy(resumed_resume_field + strlen("post_rts source_logical_pc=2710 source_physical_pc=1f2710 logical_pc="),
                   "245d", 4u);
            if (theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control_return(
                    &handoff, game_payload_capture, raw, raw_size, md5,
                    &continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control_return)) {
                free(raw);
                printf("FAIL: off-target resumed control resume reached the resumed return receipt\n");
                return 1;
            }
            memcpy(resumed_resume_field + strlen("post_rts source_logical_pc=2710 source_physical_pc=1f2710 logical_pc="),
                   "245c", 4u);
        }
        if (!theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control_return_consumer(
                &handoff, game_payload_capture, raw, raw_size, md5,
                &continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control_return_consumer) ||
            !continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control_return_consumer.valid ||
            !continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control_return_consumer.twice_resumed_loader_consumer_read_proven ||
            continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control_return_consumer.level_or_object_semantics_proven ||
            continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control_return_consumer.consumer_generation != 17u ||
            continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control_return_consumer.consumer_lba !=
                transfer_destination_branch_target_lba ||
            continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control_return_consumer.track02_record !=
                handoff.observed_track02_record ||
            continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control_return_consumer.source_offset !=
                twice_resumed_source_offset ||
            continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control_return_consumer.source_byte !=
                twice_resumed_source_byte ||
            continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control_return_consumer.consumer_physical_address !=
                0x1f1002u ||
            continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control_return_consumer.consumer_reader_pc !=
                0x245cu ||
            continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control_return_consumer.consumer_reader_physical_pc !=
                0x1f245cu) {
            free(raw);
            printf("FAIL: twice-resumed loader consumer read was not bound\n");
            return 1;
        }
        ++handoff.loader_post_envelope.bytes[9u];
        if (theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control_return_consumer(
                &handoff, game_payload_capture, raw, raw_size, md5,
                &continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control_return_consumer)) {
            free(raw);
            printf("FAIL: altered copied byte reached the twice-resumed consumer receipt\n");
            return 1;
        }
        --handoff.loader_post_envelope.bytes[9u];
        {
            char *twice_resumed_reader_field = strstr(game_payload_capture,
                                                      "reader_physical_pc=1f245c");
            if (!twice_resumed_reader_field) {
                free(raw);
                printf("FAIL: twice-resumed consumer reader missing from the synthetic capture\n");
                return 1;
            }
            memcpy(twice_resumed_reader_field, "reader_physical_pc=000a5c", 25u);
            if (theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control_return_consumer(
                    &handoff, game_payload_capture, raw, raw_size, md5,
                    &continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control_return_consumer)) {
                free(raw);
                printf("FAIL: System Card twice-resumed reader reached the twice-resumed consumer receipt\n");
                return 1;
            }
            memcpy(twice_resumed_reader_field, "reader_physical_pc=1f245c", 25u);
        }
    }
    /* The generalized loop continuation: two further per-byte
     * consume/dispatch iterations after the twice-resumed consumer read.
     * Each iteration binds an opaque control transfer, its adjacent fetched
     * window, exactly one bounded return resuming at the call return
     * address, and the next source-adjacent FIFO byte's consumer read joined
     * to its media-re-verified receipt, with the reader equal to the resumed
     * return address. All remain opaque byte/control facts; the loop's
     * termination or dispatch into a record consumer stays unproven. */
    {
        uint16_t loop_source_offset3 =
            (uint16_t)(transfer_destination_branch_target_source_offset + 3u);
        uint16_t loop_source_offset4 =
            (uint16_t)(transfer_destination_branch_target_source_offset + 4u);
        uint8_t loop_source_byte3 = raw[
            (size_t)handoff.observed_track02_record * THERON_TRACK02_RAW_SECTOR_BYTES +
            loop_source_offset3];
        uint8_t loop_source_byte4 = raw[
            (size_t)handoff.observed_track02_record * THERON_TRACK02_RAW_SECTOR_BYTES +
            loop_source_offset4];
        size_t capture_length = strlen(game_payload_capture);
        snprintf(game_payload_capture + capture_length,
                 sizeof(game_payload_capture) - capture_length,
                 "main_ram_loader_jsr logical_pc=245f physical_pc=1f245f target=2800 a=00 x=00 y=00\n"
                 "main_ram_loader_call_entry caller_logical_pc=245f caller_physical_pc=1f245f target=2800 logical_pc=2800 physical_pc=1f2800 opcode=a9\n"
                 "main_ram_loader_entry_next entry_logical_pc=2800 entry_physical_pc=1f2800 logical_pc=2801 physical_pc=1f2801 opcode=85\n"
                 "main_ram_loader_rts logical_pc=2810 physical_pc=1f2810\n"
                 "main_ram_loader_post_rts source_logical_pc=2810 source_physical_pc=1f2810 logical_pc=2462 physical_pc=1f2462 opcode=48\n"
                 "pce_cd_fifo_origin_main_ram_receipt generation=17 source_lba=%u source_offset=%u fifo_sequence=26 reader_pc=e981 logical_destination=3003 physical_destination=1f1003 writer_pc=1840 writer_physical_pc=1f1840 value=%02x\n"
                 "pce_cd_fifo_origin_main_ram_consumer sequence=3 generation=17 source_lba=%u source_offset=%u fifo_sequence=26 logical_address=3003 physical_address=1f1003 value=%02x reader_pc=2462 reader_physical_pc=1f2462\n"
                 "main_ram_loader_jsr logical_pc=2465 physical_pc=1f2465 target=2900 a=00 x=00 y=00\n"
                 "main_ram_loader_call_entry caller_logical_pc=2465 caller_physical_pc=1f2465 target=2900 logical_pc=2900 physical_pc=1f2900 opcode=ad\n"
                 "main_ram_loader_entry_next entry_logical_pc=2900 entry_physical_pc=1f2900 logical_pc=2901 physical_pc=1f2901 opcode=9d\n"
                 "main_ram_loader_rts logical_pc=2910 physical_pc=1f2910\n"
                 "main_ram_loader_post_rts source_logical_pc=2910 source_physical_pc=1f2910 logical_pc=2468 physical_pc=1f2468 opcode=e8\n"
                 "pce_cd_fifo_origin_main_ram_receipt generation=17 source_lba=%u source_offset=%u fifo_sequence=27 reader_pc=e981 logical_destination=3004 physical_destination=1f1004 writer_pc=1840 writer_physical_pc=1f1840 value=%02x\n"
                 "pce_cd_fifo_origin_main_ram_consumer sequence=4 generation=17 source_lba=%u source_offset=%u fifo_sequence=27 logical_address=3004 physical_address=1f1004 value=%02x reader_pc=2468 reader_physical_pc=1f2468\n",
                 transfer_destination_branch_target_lba,
                 loop_source_offset3, loop_source_byte3,
                 transfer_destination_branch_target_lba,
                 loop_source_offset3, loop_source_byte3,
                 transfer_destination_branch_target_lba,
                 loop_source_offset4, loop_source_byte4,
                 transfer_destination_branch_target_lba,
                 loop_source_offset4, loop_source_byte4);
        if (!theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control_return_consumer_loop_continuation(
                &handoff, game_payload_capture, raw, raw_size, md5,
                &continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control_return_consumer_loop_continuation) ||
            !continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control_return_consumer_loop_continuation.valid ||
            !continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control_return_consumer_loop_continuation.loop_continuation_proven ||
            continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control_return_consumer_loop_continuation.level_or_object_semantics_proven ||
            continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control_return_consumer_loop_continuation.consumer_generation != 17u ||
            continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control_return_consumer_loop_continuation.consumer_lba !=
                transfer_destination_branch_target_lba ||
            continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control_return_consumer_loop_continuation.first_source_offset !=
                transfer_destination_branch_target_source_offset ||
            continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control_return_consumer_loop_continuation.iterations[0].control_pc != 0x245fu ||
            continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control_return_consumer_loop_continuation.iterations[0].control_target != 0x2800u ||
            continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control_return_consumer_loop_continuation.iterations[0].post_return_pc != 0x2462u ||
            continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control_return_consumer_loop_continuation.iterations[0].source_offset !=
                loop_source_offset3 ||
            continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control_return_consumer_loop_continuation.iterations[0].source_byte !=
                loop_source_byte3 ||
            continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control_return_consumer_loop_continuation.iterations[0].track02_record !=
                handoff.observed_track02_record ||
            continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control_return_consumer_loop_continuation.iterations[0].consumer_physical_address != 0x1f1003u ||
            continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control_return_consumer_loop_continuation.iterations[0].consumer_reader_pc != 0x2462u ||
            continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control_return_consumer_loop_continuation.iterations[1].control_pc != 0x2465u ||
            continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control_return_consumer_loop_continuation.iterations[1].control_target != 0x2900u ||
            continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control_return_consumer_loop_continuation.iterations[1].post_return_pc != 0x2468u ||
            continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control_return_consumer_loop_continuation.iterations[1].source_offset !=
                loop_source_offset4 ||
            continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control_return_consumer_loop_continuation.iterations[1].source_byte !=
                loop_source_byte4 ||
            continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control_return_consumer_loop_continuation.iterations[1].track02_record !=
                handoff.observed_track02_record ||
            continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control_return_consumer_loop_continuation.iterations[1].consumer_physical_address != 0x1f1004u ||
            continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control_return_consumer_loop_continuation.iterations[1].consumer_reader_pc != 0x2468u) {
            free(raw);
            printf("FAIL: loader loop continuation was not bound\n");
            return 1;
        }
        ++handoff.loader_post_envelope.bytes[9u];
        if (theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control_return_consumer_loop_continuation(
                &handoff, game_payload_capture, raw, raw_size, md5,
                &continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control_return_consumer_loop_continuation)) {
            free(raw);
            printf("FAIL: altered copied byte reached the loop continuation receipt\n");
            return 1;
        }
        --handoff.loader_post_envelope.bytes[9u];
        {
            char *loop_resume_field = strstr(game_payload_capture,
                                             "post_rts source_logical_pc=2810 source_physical_pc=1f2810 logical_pc=2462");
            if (!loop_resume_field) {
                free(raw);
                printf("FAIL: loop resume row missing from the synthetic capture\n");
                return 1;
            }
            memcpy(loop_resume_field + strlen("post_rts source_logical_pc=2810 source_physical_pc=1f2810 logical_pc=246"),
                   "3", 1u);
            if (theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control_return_consumer_loop_continuation(
                    &handoff, game_payload_capture, raw, raw_size, md5,
                    &continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control_return_consumer_loop_continuation)) {
                free(raw);
                printf("FAIL: off-target loop resume reached the loop continuation receipt\n");
                return 1;
            }
            memcpy(loop_resume_field + strlen("post_rts source_logical_pc=2810 source_physical_pc=1f2810 logical_pc=246"),
                   "2", 1u);
        }
        {
            char *loop_reader_field = strstr(game_payload_capture,
                                             "reader_physical_pc=1f2468");
            if (!loop_reader_field) {
                free(raw);
                printf("FAIL: loop consumer reader missing from the synthetic capture\n");
                return 1;
            }
            memcpy(loop_reader_field, "reader_physical_pc=000a68", 25u);
            if (theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control_return_consumer_loop_continuation(
                    &handoff, game_payload_capture, raw, raw_size, md5,
                    &continuation_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control_return_consumer_loop_continuation)) {
                free(raw);
                printf("FAIL: System Card loop reader reached the loop continuation receipt\n");
                return 1;
            }
            memcpy(loop_reader_field, "reader_physical_pc=1f2468", 25u);
        }
    }
    if (!theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry_branch_target_jsr(
            &handoff, game_payload_capture,
            &continuation_caller_next_transfer_call_entry_branch_target_jsr) ||
        !continuation_caller_next_transfer_call_entry_branch_target_jsr.valid ||
        !continuation_caller_next_transfer_call_entry_branch_target_jsr.copied_entry_branch_target_jsr_proven ||
        continuation_caller_next_transfer_call_entry_branch_target_jsr.level_or_object_semantics_proven ||
        continuation_caller_next_transfer_call_entry_branch_target_jsr.control_pc !=
            transfer_destination_branch_target_jsr_pc ||
        continuation_caller_next_transfer_call_entry_branch_target_jsr.control_physical_pc !=
            0x1f0000u + transfer_destination_branch_target_jsr_pc ||
        continuation_caller_next_transfer_call_entry_branch_target_jsr.jsr_target !=
            transfer_destination_branch_target_jsr_target) {
        free(raw);
        printf("FAIL: copied destination BRA target JSR was not source-bound\n");
        return 1;
    }
    ++handoff.loader_post_envelope.bytes[9u];
    if (theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry_branch_target_jsr(
            &handoff, game_payload_capture,
            &continuation_caller_next_transfer_call_entry_branch_target_jsr)) {
        free(raw);
        printf("FAIL: altered copied BRA target reached JSR receipt\n");
        return 1;
    }
    --handoff.loader_post_envelope.bytes[9u];
    if (!theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry_branch_target(
            &handoff, game_payload_capture,
            &continuation_caller_next_transfer_call_entry_branch_target) ||
        !continuation_caller_next_transfer_call_entry_branch_target.valid ||
        !continuation_caller_next_transfer_call_entry_branch_target.copied_entry_branch_target_executed ||
        continuation_caller_next_transfer_call_entry_branch_target.level_or_object_semantics_proven ||
        continuation_caller_next_transfer_call_entry_branch_target.target_pc !=
            transfer_destination_branch_target ||
        continuation_caller_next_transfer_call_entry_branch_target.target_physical_pc !=
            0x1f0000u + transfer_destination_branch_target ||
        continuation_caller_next_transfer_call_entry_branch_target.target_opcode != 0xeau) {
        free(raw);
        printf("FAIL: copied destination BRA target execution was not source-bound\n");
        return 1;
    }
    ++handoff.loader_post_envelope.bytes[9u];
    if (theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry_branch_target(
            &handoff, game_payload_capture,
            &continuation_caller_next_transfer_call_entry_branch_target)) {
        free(raw);
        printf("FAIL: altered copied BRA target reached execution receipt\n");
        return 1;
    }
    --handoff.loader_post_envelope.bytes[9u];
    ++manifest.trace_md5[0];
    if (theron_v1_raw_loader_trace_bind_capture_manifest_to_initial_level_handoff(
            &handoff, &manifest, manifest.track02_path, md5,
            manifest.system_card_path, system_card_md5, manifest.trace_path,
            trace_md5, &handoff)) {
        free(raw);
        printf("FAIL: altered manifest trace identity reached the handoff\n");
        return 1;
    }
    --manifest.trace_md5[0];
    memset(&profile, 0, sizeof(profile));
    snprintf(profile.graphics_md5, sizeof(profile.graphics_md5), "%s", md5);
    profile.track02_initial_level_handoff = handoff;
    if (!theron_v1_startup_runtime_consume_boot_profile_initial_payload(
            &profile, raw, raw_size, &payload_receipt) ||
        !payload_receipt.consumed || !payload_receipt.no_fallback ||
        payload_receipt.record != handoff.observed_track02_record ||
        payload_receipt.payload_checksum != handoff.complete_payload_checksum ||
        payload_receipt.raw_track02_offset !=
            (uint64_t)handoff.initial_level_boundary.level_first_raw_sector *
                THERON_TRACK02_RAW_SECTOR_BYTES +
                THERON_TRACK02_RAW_USER_DATA_OFFSET) {
        free(raw);
        printf("FAIL: boot receipt did not retain the exact runtime payload\n");
        return 1;
    }
    theron_v1_world_init(&world);
    if (theron_v1_startup_runtime_receive_boot_profile_initial_route(
            &profile, &world, raw, raw_size,
            THERON_DUNGEON_1_AKUTUBA, &route_receipt)) {
        free(raw);
        printf("FAIL: loader-executed sector reached runtime as an unproven level\n");
        return 1;
    }
    if (!route_receipt.loader_record_received ||
        route_receipt.loader_record != handoff.observed_track02_record ||
        route_receipt.loader_record_raw_user_data_offset !=
            payload_receipt.raw_track02_offset ||
        route_receipt.loader_record_payload_checksum !=
            payload_receipt.payload_checksum ||
        route_receipt.level_envelope_offset !=
            handoff.loader_level_envelope.record_user_data_offset ||
        route_receipt.level_envelope_bytes !=
            handoff.loader_level_envelope.envelope_bytes ||
        route_receipt.level_envelope_checksum !=
            handoff.loader_level_envelope.envelope_checksum ||
        !world.runtime_media.loader_record.ready ||
        !world.runtime_media.loader_record.raw_source_verified ||
        !world.runtime_media.loader_record.no_semantic_promotion ||
        strcmp(world.runtime_media.loader_record.track02_md5, md5) != 0 ||
        world.runtime_media.loader_record.record != handoff.observed_track02_record ||
        world.runtime_media.loader_record.payload_checksum !=
            handoff.complete_payload_checksum ||
        !world.runtime_media.loader_record.level_envelope_bound ||
        world.runtime_media.loader_record.level_envelope_offset !=
            handoff.loader_level_envelope.record_user_data_offset ||
        world.runtime_media.loader_record.level_envelope_bytes !=
            handoff.loader_level_envelope.envelope_bytes ||
        world.runtime_media.loader_record.level_envelope_checksum !=
            handoff.loader_level_envelope.envelope_checksum ||
        world.runtime_media.loader_record.post_envelope_offset !=
            world.runtime_media.loader_record.level_envelope_offset +
                world.runtime_media.loader_record.level_envelope_bytes ||
        world.runtime_media.loader_record.post_envelope_checksum !=
            handoff.loader_post_envelope.checksum) {
        free(raw);
        printf("FAIL: authenticated loader record was not retained as opaque runtime provenance\n");
        return 1;
    }
    ++profile.track02_initial_level_handoff.initial_level_route.route_hash;
    if (theron_v1_startup_runtime_receive_boot_profile_initial_route(
            &profile, &world, raw, raw_size,
            THERON_DUNGEON_1_AKUTUBA, &route_receipt)) {
        free(raw);
        printf("FAIL: altered initial-level route reached runtime\n");
        return 1;
    }
    --profile.track02_initial_level_handoff.initial_level_route.route_hash;
    ++profile.track02_initial_level_handoff.loader_level_envelope.envelope[0];
    if (theron_v1_startup_runtime_receive_boot_profile_initial_route(
            &profile, &world, raw, raw_size,
            THERON_DUNGEON_1_AKUTUBA, &route_receipt)) {
        free(raw);
        printf("FAIL: altered record envelope reached runtime\n");
        return 1;
    }
    --profile.track02_initial_level_handoff.loader_level_envelope.envelope[0];
    ++profile.track02_initial_level_handoff.loader_post_envelope.bytes[0];
    if (theron_v1_startup_runtime_receive_boot_profile_initial_route(
            &profile, &world, raw, raw_size,
            THERON_DUNGEON_1_AKUTUBA, &route_receipt)) {
        free(raw);
        printf("FAIL: altered post-envelope source bytes reached runtime\n");
        return 1;
    }
    --profile.track02_initial_level_handoff.loader_post_envelope.bytes[0];
    ++raw[payload_receipt.raw_track02_offset];
    if (theron_v1_startup_runtime_consume_boot_profile_initial_payload(
            &profile, raw, raw_size, &payload_receipt)) {
        free(raw);
        printf("FAIL: altered original payload reached the runtime handoff\n");
        return 1;
    }
    --raw[payload_receipt.raw_track02_offset];
    ++raw[payload_receipt.raw_track02_offset +
          handoff.loader_post_envelope.record_user_data_offset];
    if (theron_v1_startup_runtime_consume_boot_profile_initial_payload(
            &profile, raw, raw_size, &payload_receipt)) {
        free(raw);
        printf("FAIL: altered raw post-envelope bytes reached runtime\n");
        return 1;
    }
    --raw[payload_receipt.raw_track02_offset +
          handoff.loader_post_envelope.record_user_data_offset];
    coalesced.stage3_post_irq2_resume_verified = 0;
    if (theron_v1_raw_loader_trace_bind_initial_level_handoff(
            &coalesced, raw, raw_size, md5, &handoff)) {
        free(raw);
        printf("FAIL: receipt without the Stage 3 IRQ2 resume admitted the level route\n");
        return 1;
    }
    coalesced.stage3_post_irq2_resume_verified = 1;
    coalesced.stage3_continuation_pc = 0x3803u;
    if (theron_v1_raw_loader_trace_bind_initial_level_handoff(
            &coalesced, raw, raw_size, md5, &handoff)) {
        free(raw);
        printf("FAIL: receipt with a non-original Stage 3 continuation admitted the level route\n");
        return 1;
    }
    coalesced.stage3_continuation_pc = 0x3802u;
    coalesced.later_post_return_step_verified = 0;
    if (theron_v1_raw_loader_trace_bind_initial_level_handoff(
            &coalesced, raw, raw_size, md5, &handoff)) {
        free(raw);
        printf("FAIL: receipt without the post-return control step admitted the level route\n");
        return 1;
    }
    coalesced.later_post_return_step_verified = 1;
    coalesced.later_post_return_resume_pc = 0xea04u;
    if (theron_v1_raw_loader_trace_bind_initial_level_handoff(
            &coalesced, raw, raw_size, md5, &handoff)) {
        free(raw);
        printf("FAIL: receipt without the returned-PC control edge admitted the level route\n");
        return 1;
    }
    coalesced.later_post_return_resume_pc = coalesced.return_pc;
    coalesced.later_caller_control_verified = 0;
    if (theron_v1_raw_loader_trace_bind_initial_level_handoff(
            &coalesced, raw, raw_size, md5, &handoff)) {
        free(raw);
        printf("FAIL: receipt without the observed e009 caller control admitted the level route\n");
        return 1;
    }
    coalesced.later_caller_control_verified = 1;
    coalesced.later_caller_target = 0xe00cu;
    if (theron_v1_raw_loader_trace_bind_initial_level_handoff(
            &coalesced, raw, raw_size, md5, &handoff)) {
        free(raw);
        printf("FAIL: receipt with a non-e009 caller target admitted the level route\n");
        return 1;
    }
    coalesced.later_caller_target = 0xe009u;

    ++coalesced.later_destination_span_checksum;
    if (theron_v1_raw_loader_trace_bind_initial_level_handoff(
            &coalesced, raw, raw_size, md5, &handoff)) {
        free(raw);
        printf("FAIL: receipt with a mismatched source span admitted the level route\n");
        return 1;
    }
    --coalesced.later_destination_span_checksum;
    coalesced.later_destination_payload_verified = 0;
    if (theron_v1_raw_loader_trace_bind_initial_level_handoff(
            &coalesced, raw, raw_size, md5, &handoff)) {
        free(raw);
        printf("FAIL: receipt without the complete local payload admitted the level route\n");
        return 1;
    }
    coalesced.later_destination_payload_verified = 1;
    ++coalesced.later_destination_payload_checksum;
    if (theron_v1_raw_loader_trace_bind_initial_level_handoff(
            &coalesced, raw, raw_size, md5, &handoff)) {
        free(raw);
        printf("FAIL: receipt with a mismatched complete payload admitted the level route\n");
        return 1;
    }
    --coalesced.later_destination_payload_checksum;
    coalesced.later_local_destination = 0x3900u;
    if (theron_v1_raw_loader_trace_bind_initial_level_handoff(
            &coalesced, raw, raw_size, md5, &handoff)) {
        free(raw);
        printf("FAIL: receipt with a non-loader-intake destination admitted the level route\n");
        return 1;
    }
    coalesced.later_local_destination = 0x3800u;
    coalesced.later_track02_record = 0x0b53u;
    if (theron_v1_raw_loader_trace_bind_initial_level_handoff(
            &coalesced, raw, raw_size, md5, &handoff)) {
        free(raw);
        printf("FAIL: adjacent CD record unexpectedly admitted the level route\n");
        return 1;
    }
    printf("PASS: source-bound receipt composes only record 0x0b52; no object or visual semantics promoted\n");

    if (trace_path) {
        uint8_t *trace;
        size_t trace_size;
        char *trace_text;

        trace = read_file(trace_path, &trace_size);
        trace_text = (char *)trace;
        if (!trace || memchr(trace_text, '\0', trace_size) != NULL) {
            free(raw);
            free(trace);
            printf("FAIL: supplied coalesced trace is not bounded text\n");
            return 1;
        }
        trace_text = (char *)realloc(trace, trace_size + 1u);
        if (!trace_text) {
            free(raw);
            free(trace);
            return 1;
        }
        trace_text[trace_size] = '\0';
        if (!theron_v1_raw_loader_trace_bind_coalesced_later_e009_raw_sector(
                trace_text, raw, raw_size, md5, &coalesced) ||
            !theron_v1_raw_loader_trace_bind_initial_level_handoff(
                &coalesced, raw, raw_size, md5, &handoff)) {
            free(trace_text);
            free(raw);
            printf("FAIL: supplied coalesced capture does not prove the initial-level CD handoff\n");
            return 1;
        }
        free(trace_text);
        printf("PASS: authenticated coalesced capture observes the source-locked initial-level record\n");
    } else {
        printf("SKIP: set FIRESTAFF_THERON_COALESCED_LOADER_TRACE for an authenticated capture result\n");
    }
    free(raw);
    return 0;
}
