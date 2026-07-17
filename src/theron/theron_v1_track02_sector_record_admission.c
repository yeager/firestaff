#include "theron_v1_track02_sector_record_admission.h"

#include "asset_status_m12.h"
#include "theron_v1_raw_loader_trace.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define THERON_V1_TRACK02_SECTOR_RECORD_TRACE_MAX_BYTES (1024u * 1024u)

static unsigned char *read_regular_file(const char *path, size_t maximum,
                                        size_t *out_size)
{
    FILE *file;
    long length;
    unsigned char *bytes = NULL;

    if (!path || !path[0] || !out_size || !(file = fopen(path, "rb"))) {
        return NULL;
    }
    if (fseek(file, 0L, SEEK_END) != 0 || (length = ftell(file)) <= 0 ||
        (size_t)length > maximum || fseek(file, 0L, SEEK_SET) != 0 ||
        !(bytes = (unsigned char *)malloc((size_t)length + 1u)) ||
        fread(bytes, 1u, (size_t)length, file) != (size_t)length) {
        fclose(file);
        free(bytes);
        return NULL;
    }
    fclose(file);
    bytes[length] = '\0';
    *out_size = (size_t)length;
    return bytes;
}

int theron_v1_track02_sector_record_admit_from_trace(
    const Theron_V1Track02RawMediaIntakeReceipt *intake,
    const char *coalesced_trace_path,
    Theron_V1Track02SectorRecordAdmissionReceipt *out)
{
    Theron_V1Track02SectorRecordAdmissionReceipt receipt = {0};
    Theron_V1RawLoaderTraceCoalescedLaterReceipt coalesced;
    unsigned char *track = NULL;
    unsigned char *trace = NULL;
    size_t track_size = 0u;
    size_t trace_size = 0u;
    char observed_md5[33];

    if (!out) return 0;
    *out = receipt;
    if (!intake || !coalesced_trace_path || !coalesced_trace_path[0] ||
        intake->status == THERON_V1_TRACK02_MEDIA_INTAKE_UNAVAILABLE) {
        receipt.status = THERON_V1_TRACK02_SECTOR_RECORD_UNAVAILABLE;
        *out = receipt;
        return 1;
    }
    if (intake->status != THERON_V1_TRACK02_MEDIA_INTAKE_READY ||
        !intake->cue_consumed || !intake->mode1_2352 ||
        !intake->raw_trace_preparation_allowed ||
        (intake->variant != THERON_TRACK02_VARIANT_JP_BIN &&
         intake->variant != THERON_TRACK02_VARIANT_US_BIN) ||
        !intake->payload_path[0] || !intake->track02_md5[0] ||
        intake->payload_bytes == 0u || intake->payload_bytes % 2352u != 0u ||
        intake->sector_count != intake->payload_bytes / 2352u ||
        !m12_file_md5_hex(intake->payload_path, observed_md5) ||
        strcmp(observed_md5, intake->track02_md5) != 0 ||
        theron_v1_track02_variant_for_md5(observed_md5) != intake->variant ||
        !(track = read_regular_file(intake->payload_path, intake->payload_bytes,
                                    &track_size)) ||
        track_size != intake->payload_bytes ||
        !(trace = read_regular_file(coalesced_trace_path,
                                    THERON_V1_TRACK02_SECTOR_RECORD_TRACE_MAX_BYTES,
                                    &trace_size)) ||
        memchr(trace, '\0', trace_size) != NULL ||
        !theron_v1_raw_loader_trace_bind_coalesced_later_e009_raw_sector(
            (const char *)trace, track, track_size, observed_md5, &coalesced) ||
        !coalesced.valid || !coalesced.observation_order_verified ||
        !coalesced.stage3_post_irq2_resume_verified ||
        !coalesced.descriptor_row_media_bound ||
        coalesced.descriptor_semantics_proven ||
        !coalesced.descriptor_source_bytes_proven ||
        coalesced.descriptor_source_bytes != 6u ||
        !coalesced.descriptor_selector_aliases_proven ||
        !coalesced.selector_sector_bytes_verified ||
        !coalesced.later_destination_media_span_verified ||
        !coalesced.later_destination_payload_verified ||
        coalesced.variant != intake->variant ||
        strcmp(coalesced.track02_md5, observed_md5) != 0 ||
        coalesced.later_track02_record <= coalesced.stage3_track02_record ||
        coalesced.later_track02_record >= intake->sector_count ||
        (size_t)coalesced.later_track02_record > SIZE_MAX / 2352u ||
        coalesced.descriptor_selector == 0u ||
        coalesced.descriptor_source_raw_offset >= track_size ||
        coalesced.descriptor_source_hash == 0u ||
        coalesced.descriptor_record_user_data_hash == 0u ||
        !coalesced.later_local_destination ||
        !coalesced.later_destination_span_bytes ||
        !coalesced.later_destination_span_checksum ||
        !coalesced.later_destination_payload_bytes ||
        !coalesced.later_destination_payload_checksum ||
        !coalesced.later_caller_control_verified ||
        coalesced.later_caller_opcode != 0x20u ||
        coalesced.later_caller_target != 0xe009u ||
        !coalesced.sector_count ||
        coalesced.later_track02_record !=
            ((uint32_t)coalesced.later_record_cl |
             ((uint32_t)coalesced.later_record_dl << 8) |
             ((uint32_t)coalesced.later_record_ch << 16)) ||
        !coalesced.later_post_return_step_verified ||
        coalesced.later_post_return_resume_pc != coalesced.return_pc) {
        receipt.status = THERON_V1_TRACK02_SECTOR_RECORD_REJECTED;
        goto done;
    }

    receipt.status = THERON_V1_TRACK02_SECTOR_RECORD_READY;
    receipt.raw_cue_bin_identity_consumed = 1;
    receipt.stage3_directory_consumed = 1;
    receipt.observed_later_loader_consumed = 1;
    receipt.nonstartup_record_consumed = 1;
    receipt.track02_variant = coalesced.variant;
    snprintf(receipt.track02_md5, sizeof(receipt.track02_md5), "%s", observed_md5);
    receipt.stage3_track02_record = coalesced.stage3_track02_record;
    receipt.descriptor_ordinal = coalesced.descriptor_selector_ordinal;
    receipt.descriptor_word0 = coalesced.descriptor_word0;
    receipt.descriptor_word1 = coalesced.descriptor_word1;
    receipt.descriptor_selector = coalesced.descriptor_selector;
    receipt.descriptor_source_raw_offset = coalesced.descriptor_source_raw_offset;
    receipt.descriptor_source_hash = coalesced.descriptor_source_hash;
    receipt.resolved_track02_record = coalesced.later_track02_record;
    receipt.record_raw_offset = (size_t)coalesced.later_track02_record * 2352u;
    if (receipt.record_raw_offset > track_size ||
        track_size - receipt.record_raw_offset < 2352u) {
        receipt.status = THERON_V1_TRACK02_SECTOR_RECORD_REJECTED;
        goto done;
    }
    receipt.record_user_data_offset = receipt.record_raw_offset + 16u;
    receipt.record_user_data_bytes = 2048u;
    receipt.record_user_data_hash = coalesced.descriptor_record_user_data_hash;
    receipt.loader_caller_pc = coalesced.caller_pc;
    receipt.loader_return_pc = coalesced.return_pc;
    receipt.loader_caller_opcode = coalesced.later_caller_opcode;
    receipt.loader_caller_target = coalesced.later_caller_target;
    receipt.loader_post_return_pc = coalesced.later_post_return_resume_pc;
    receipt.loader_post_return_next_pc = coalesced.later_post_return_next_pc;
    receipt.loader_record_cl = coalesced.later_record_cl;
    receipt.loader_record_dl = coalesced.later_record_dl;
    receipt.loader_record_ch = coalesced.later_record_ch;
    receipt.loader_sector_count = coalesced.sector_count;
    receipt.loader_observed_raw_sector_lba = coalesced.observed_raw_sector_lba;
    receipt.loader_callsite_context_verified = 1;
    receipt.loader_destination = coalesced.later_local_destination;
    receipt.loader_destination_span_bytes = coalesced.later_destination_span_bytes;
    receipt.loader_destination_span_checksum = coalesced.later_destination_span_checksum;
    receipt.loader_destination_payload_bytes =
        coalesced.later_destination_payload_bytes;
    receipt.loader_destination_payload_checksum =
        coalesced.later_destination_payload_checksum;
    receipt.observed_raw_sector_checksum = coalesced.observed_raw_sector_checksum;
    *out = receipt;

done:
    free(track);
    free(trace);
    if (receipt.status != THERON_V1_TRACK02_SECTOR_RECORD_READY) *out = receipt;
    return 1;
}
