/*
 * Bounded integration check for the future positive Track 02 loader/CD
 * handoff. A fixture receipt only exercises composition; it is not presented
 * as an original capture. A real positive result requires the operator to
 * supply a coalesced Mednafen transcript that the production parser accepts.
 */
#include "asset_status_m12.h"
#include "theron_v1_boot.h"
#include "theron_v1_raw_loader_trace.h"
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
    size_t raw_offset;

    memset(out, 0, sizeof(*out));
    if (theron_v1_track02_capture_initial_level_object_boundary(
            raw, raw_size, md5, &boundary) != THERON_TRACK02_SIGNAL_OK ||
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
    char game_payload_capture[2048];
    uint8_t *raw;
    size_t raw_size;
    Theron_V1RawLoaderTraceCoalescedLaterReceipt coalesced;
    Theron_V1RawLoaderTraceInitialLevelHandoffReceipt handoff;
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
    Theron_V1RawLoaderTraceGamePayloadReceipt continuation_payloads[
        THERON_V1_RAW_LOADER_INITIAL_POST_ENVELOPE_PREFIX_BYTES];
    Theron_V1RawLoaderTraceInitialEnvelopeHeaderReceipt envelope_header;
    size_t header_index;
    const char *system_card_md5 = "ff1a674273fe3540ccef576376407d1d";
    const char *trace_md5 = "0123456789abcdef0123456789abcdef";

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
        handoff.object_tail_semantics_proven || handoff.fallback_visuals_allowed) {
        free(raw);
        printf("FAIL: fixture composition did not preserve the bounded handoff contract\n");
        return 1;
    }

    {
        Theron_Track02InitialLevelLoaderSemanticReceipt direct_semantics;
        Theron_Track02InitialLevelLoaderRoute direct_route;

        if (theron_v1_track02_decode_initial_level_loader_semantics(
                raw, raw_size, md5, &direct_semantics) !=
                THERON_TRACK02_SIGNAL_NOT_FOUND ||
            direct_semantics.valid ||
            theron_v1_track02_load_initial_level_loader_route(
                raw, raw_size, md5, THERON_DUNGEON_1_HALL_OF_RECORDS, 0,
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
             "main_ram_loader_jsr logical_pc=3850 physical_pc=1f1850 target=2000 a=00 x=00 y=00\n");
    if (!theron_v1_raw_loader_trace_bind_initial_post_envelope_execution(
            &handoff, game_payload_capture, &continuation_execution) ||
        !continuation_execution.valid ||
        !continuation_execution.continuation_execution_proven ||
        continuation_execution.level_or_object_semantics_proven ||
        continuation_execution.call_pc != 0x3850u ||
        continuation_execution.call_physical_pc != 0x1f1850u ||
        continuation_execution.call_target != 0x2000u ||
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
            THERON_DUNGEON_1_HALL_OF_RECORDS, &route_receipt)) {
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
            THERON_DUNGEON_1_HALL_OF_RECORDS, &route_receipt)) {
        free(raw);
        printf("FAIL: altered initial-level route reached runtime\n");
        return 1;
    }
    --profile.track02_initial_level_handoff.initial_level_route.route_hash;
    ++profile.track02_initial_level_handoff.loader_level_envelope.envelope[0];
    if (theron_v1_startup_runtime_receive_boot_profile_initial_route(
            &profile, &world, raw, raw_size,
            THERON_DUNGEON_1_HALL_OF_RECORDS, &route_receipt)) {
        free(raw);
        printf("FAIL: altered record envelope reached runtime\n");
        return 1;
    }
    --profile.track02_initial_level_handoff.loader_level_envelope.envelope[0];
    ++profile.track02_initial_level_handoff.loader_post_envelope.bytes[0];
    if (theron_v1_startup_runtime_receive_boot_profile_initial_route(
            &profile, &world, raw, raw_size,
            THERON_DUNGEON_1_HALL_OF_RECORDS, &route_receipt)) {
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
