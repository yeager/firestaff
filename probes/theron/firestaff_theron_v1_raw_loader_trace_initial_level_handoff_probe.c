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

    if (theron_v1_track02_capture_initial_level_object_boundary(
            raw, raw_size, md5, &boundary) != THERON_TRACK02_SIGNAL_OK ||
        !boundary.valid || boundary.following_user_data_bytes_in_record != 0x380u ||
        !boundary.following_user_data_hash ||
        boundary.object_boundary_raw_offset >= raw_size) {
        free(raw);
        printf("FAIL: raw Track 02 did not retain the opaque post-level witness\n");
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
