#include "theron_v1_raw_loader_trace.h"

#include "theron_v1_irq2_live_trace_gate.h"
#include "theron_v1_stage3_manifest_evidence.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define THERON_V1_RAW_LOADER_TRACE_MAX_BYTES (1024u * 1024u)

static uint32_t tqr_trace_fnv1a_u16(uint32_t hash, uint16_t value)
{
    hash ^= (uint8_t)(value & 0xffu);
    hash *= 16777619u;
    hash ^= (uint8_t)(value >> 8);
    return hash * 16777619u;
}

static uint32_t tqr_trace_fnv1a_bytes(const uint8_t *bytes, size_t byte_count)
{
    uint32_t hash = 2166136261u;
    size_t i;

    if (!bytes || !byte_count) return 0u;
    for (i = 0u; i < byte_count; ++i) {
        hash ^= bytes[i];
        hash *= 16777619u;
    }
    return hash;
}

static int tqr_trace_next_line(const char **cursor,
                               const char **out_line,
                               size_t *out_length)
{
    const char *line;
    const char *end;

    if (!cursor || !*cursor || !out_line || !out_length) return 0;
    line = *cursor;
    if (!line[0]) return 0;
    end = strchr(line, '\n');
    *out_line = line;
    *out_length = end ? (size_t)(end - line) : strlen(line);
    *cursor = end ? end + 1 : line + *out_length;
    return 1;
}

static int tqr_trace_parse_palette_store(const char *line, size_t length,
                                         unsigned int *out_pc,
                                         unsigned int *out_address,
                                         unsigned int *out_accumulator)
{
    int consumed = 0;
    unsigned int physical_pc;
    unsigned int opcode;

    if (!line || !out_pc || !out_address || !out_accumulator) return 0;
    return sscanf(line,
                  "dynamic_huc6260_palette_store pc=%x physical_pc=%x opcode=%x address=%x accumulator=%x%n",
                  out_pc, &physical_pc, &opcode, out_address, out_accumulator,
                  &consumed) == 5 &&
           consumed == (int)length && *out_pc <= 0xffffu &&
           physical_pc <= 0xffffffffu && opcode == 0x8du &&
           *out_address >= 0x0402u && *out_address <= 0x0405u &&
           *out_accumulator <= 0xffu;
}

static int tqr_trace_parse_palette_word(const char *line, size_t length,
                                        unsigned int *out_index,
                                        unsigned int *out_word)
{
    int consumed = 0;

    if (!line || !out_index || !out_word) return 0;
    return sscanf(line,
                  "dynamic_huc6260_palette_word index=%x word=%x%n",
                  out_index, out_word, &consumed) == 2 &&
           consumed == (int)length && *out_index <= 0x1ffu &&
           *out_word <= 0x1ffu;
}

static int tqr_trace_parse_cd_read_destination_span(
    const char *line, size_t length, unsigned int *out_pc,
    unsigned int *out_destination, unsigned int *out_bytes,
    unsigned int *out_checksum)
{
    int consumed = 0;

    if (!line || !out_pc || !out_destination || !out_bytes ||
        !out_checksum) return 0;
    return sscanf(line,
                  "dynamic_cd_read_destination_span pc=%x destination=%x bytes=%u fnv1a=%x%n",
                  out_pc, out_destination, out_bytes, out_checksum,
                  &consumed) == 4 && consumed == (int)length &&
           *out_pc == 0x4093u && *out_destination == 0x3800u &&
           *out_bytes == 32u && *out_checksum != 0u;
}

static int tqr_trace_parse_later_e009_dispatch(
    const char *line, size_t length, unsigned int *out_caller_pc,
    unsigned int *out_return_pc, unsigned int *out_sector_count,
    unsigned int *out_cl, unsigned int *out_dl, unsigned int *out_ch,
    unsigned int *out_record, unsigned int *out_caller_opcode,
    unsigned int *out_caller_target)
{
    int consumed = 0;

    if (!line || !out_caller_pc || !out_return_pc || !out_sector_count ||
        !out_cl || !out_dl || !out_ch || !out_record || !out_caller_opcode ||
        !out_caller_target) return 0;
    return sscanf(line,
                  "later_system_card_e009_dispatch caller_pc=%x return_pc=%x caller_opcode=%x caller_target=%x sector_count=%x record_cl=%x record_dl=%x record_ch=%x record=%x%n",
                  out_caller_pc, out_return_pc, out_caller_opcode,
                  out_caller_target, out_sector_count, out_cl, out_dl,
                  out_ch, out_record, &consumed) == 9 &&
           consumed == (int)length && *out_caller_pc <= 0xffffu &&
           *out_return_pc <= 0xffffu && *out_sector_count > 0u &&
           *out_sector_count <= 0xffu && *out_cl <= 0xffu &&
           *out_dl <= 0xffu && *out_ch <= 0xffu && *out_record <= 0xffffffu;
}

static int tqr_trace_parse_later_e009_return(const char *line, size_t length,
                                              unsigned int *out_caller_pc,
                                              unsigned int *out_return_pc,
                                              unsigned int *out_record)
{
    int consumed = 0;

    if (!line || !out_caller_pc || !out_return_pc || !out_record) return 0;
    return sscanf(line,
                  "later_system_card_e009_return caller_pc=%x return_pc=%x record=%x%n",
                  out_caller_pc, out_return_pc, out_record, &consumed) == 3 &&
           consumed == (int)length && *out_caller_pc <= 0xffffu &&
           *out_return_pc <= 0xffffu && *out_record <= 0xffffffu;
}

static int tqr_trace_parse_later_e009_destination_span(
    const char *line, size_t length, unsigned int *out_caller_pc,
    unsigned int *out_return_pc, unsigned int *out_record,
    unsigned int *out_destination, unsigned int *out_bytes,
    unsigned int *out_checksum)
{
    int consumed = 0;

    if (!line || !out_caller_pc || !out_return_pc || !out_record ||
        !out_destination || !out_bytes || !out_checksum) return 0;
    return sscanf(line,
                  "later_system_card_e009_destination_span caller_pc=%x return_pc=%x record=%x destination=%x bytes=%u fnv1a=%x%n",
                  out_caller_pc, out_return_pc, out_record, out_destination,
                  out_bytes, out_checksum, &consumed) == 6 &&
           consumed == (int)length && *out_caller_pc <= 0xffffu &&
           *out_return_pc <= 0xffffu && *out_record <= 0xffffffu &&
           *out_destination <= 0xffffu && *out_bytes == 32u &&
           *out_checksum != 0u;
}

static int tqr_trace_parse_later_e009_destination_payload(
    const char *line, size_t length, unsigned int *out_caller_pc,
    unsigned int *out_return_pc, unsigned int *out_record,
    unsigned int *out_destination, unsigned int *out_bytes,
    unsigned int *out_checksum)
{
    int consumed = 0;

    if (!line || !out_caller_pc || !out_return_pc || !out_record ||
        !out_destination || !out_bytes || !out_checksum) return 0;
    return sscanf(line,
                  "later_system_card_e009_destination_payload caller_pc=%x return_pc=%x record=%x destination=%x bytes=%u fnv1a=%x%n",
                  out_caller_pc, out_return_pc, out_record, out_destination,
                  out_bytes, out_checksum, &consumed) == 6 &&
           consumed == (int)length && *out_caller_pc <= 0xffffu &&
           *out_return_pc <= 0xffffu && *out_record <= 0xffffffu &&
           *out_destination <= 0xffffu &&
           *out_bytes == THERON_TRACK02_RAW_USER_DATA_BYTES &&
           *out_checksum != 0u;
}

static int tqr_trace_parse_later_e009_post_return_step(
    const char *line, size_t length, unsigned int *out_caller_pc,
    unsigned int *out_return_pc, unsigned int *out_record,
    unsigned int *out_resume_pc, unsigned int *out_next_pc)
{
    int consumed = 0;

    if (!line || !out_caller_pc || !out_return_pc || !out_record ||
        !out_resume_pc || !out_next_pc) return 0;
    return sscanf(line,
                  "later_system_card_e009_post_return_step caller_pc=%x return_pc=%x record=%x resume_pc=%x next_pc=%x%n",
                  out_caller_pc, out_return_pc, out_record, out_resume_pc,
                  out_next_pc, &consumed) == 5 && consumed == (int)length &&
           *out_caller_pc <= 0xffffu && *out_return_pc <= 0xffffu &&
           *out_record <= 0xffffffu && *out_resume_pc <= 0xffffu &&
           *out_next_pc <= 0xffffu;
}

static int tqr_trace_parse_raw_sector_span(const char *line, size_t length,
                                           unsigned int *out_lba,
                                           unsigned int *out_bytes,
                                           unsigned int *out_span_offset,
                                           unsigned int *out_span_bytes,
                                           unsigned int *out_span_checksum,
                                           unsigned int *out_sector_checksum)
{
    int consumed = 0;

    if (!line || !out_lba || !out_bytes || !out_span_offset ||
        !out_span_bytes || !out_span_checksum || !out_sector_checksum) return 0;
    return sscanf(line,
                  "cd_interface_raw_sector_read lba=%u bytes=%u sector_fnv1a=%x span_offset=%u span_bytes=%u span_fnv1a=%x%n",
                  out_lba, out_bytes, out_sector_checksum, out_span_offset,
                  out_span_bytes, out_span_checksum, &consumed) == 6 &&
           consumed == (int)length &&
           *out_bytes == THERON_TRACK02_RAW_SECTOR_BYTES &&
           *out_span_offset == 0u && *out_span_bytes == 32u &&
           *out_span_checksum != 0u && *out_sector_checksum != 0u;
}

static uint32_t tqr_trace_fnv1a_user_data_range(const uint8_t *track02_data,
                                                 size_t first_raw_sector,
                                                 size_t sector_count)
{
    uint32_t hash = 2166136261u;
    size_t sector;
    size_t byte;

    for (sector = 0u; sector < sector_count; ++sector) {
        const uint8_t *user_data = track02_data +
            (first_raw_sector + sector) * THERON_TRACK02_RAW_SECTOR_BYTES +
            THERON_TRACK02_RAW_USER_DATA_OFFSET;
        for (byte = 0u; byte < THERON_TRACK02_RAW_USER_DATA_BYTES; ++byte) {
            hash ^= user_data[byte];
            hash *= 16777619u;
        }
    }
    return hash;
}

int theron_v1_raw_loader_trace_ingest_mednafen_capture(
    const char *capture,
    const char *track02_md5,
    Theron_V1RawLoaderTraceReceipt *out)
{
    Theron_V1Irq2LiveTrace live_trace;
    const char *cursor;
    const char *line;
    size_t length;
    int dynamic_read_seen = 0;
    int destination_span_seen = 0;
    int controller_state_seen = 0;
    unsigned int pc;
    unsigned int address;
    unsigned int accumulator;
    unsigned int palette_index;
    unsigned int palette_word;
    unsigned int destination_span_pc;
    unsigned int destination_span_destination;
    unsigned int destination_span_bytes;
    unsigned int destination_span_checksum;

    if (out) memset(out, 0, sizeof(*out));
    if (!capture || !track02_md5 || !out ||
        !theron_v1_irq2_live_trace_from_mednafen_capture(capture,
                                                          &live_trace)) {
        return 0;
    }
    if ((live_trace.variant == THERON_TRACK02_VARIANT_JP_BIN &&
         strcmp(track02_md5, THERON_TRACK02_MD5_JP_BIN) != 0) ||
        (live_trace.variant == THERON_TRACK02_VARIANT_US_BIN &&
         strcmp(track02_md5, THERON_TRACK02_MD5_US_BIN) != 0)) {
        return 0;
    }

    cursor = capture;
    out->palette_word_checksum = 2166136261u;
    while (tqr_trace_next_line(&cursor, &line, &length)) {
        if (length >= strlen("dynamic_cd_read_transaction ") &&
            memcmp(line, "dynamic_cd_read_transaction ",
                   strlen("dynamic_cd_read_transaction ")) == 0) {
            if (dynamic_read_seen || destination_span_seen ||
                controller_state_seen || out->palette_store_count ||
                out->palette_word_count) return 0;
            dynamic_read_seen = 1;
        } else if (length >= strlen("dynamic_cd_read_destination_span ") &&
            memcmp(line, "dynamic_cd_read_destination_span ",
                   strlen("dynamic_cd_read_destination_span ")) == 0) {
            if (!dynamic_read_seen || destination_span_seen ||
                controller_state_seen || out->dynamic_cd_read_destination_span_verified ||
                !tqr_trace_parse_cd_read_destination_span(
                    line, length, &destination_span_pc,
                    &destination_span_destination, &destination_span_bytes,
                    &destination_span_checksum)) return 0;
            out->dynamic_cd_read_destination_span_bytes =
                destination_span_bytes;
            out->dynamic_cd_read_destination_span_checksum =
                destination_span_checksum;
            out->dynamic_cd_read_destination_span_verified = 1;
            destination_span_seen = 1;
        } else if (length >= strlen("dynamic_cd_read_controller_state ") &&
                   memcmp(line, "dynamic_cd_read_controller_state ",
                          strlen("dynamic_cd_read_controller_state ")) == 0) {
            if (!destination_span_seen || controller_state_seen ||
                out->palette_store_count || out->palette_word_count) return 0;
            controller_state_seen = 1;
        } else if (length >= strlen("dynamic_huc6260_palette_store ") &&
            memcmp(line, "dynamic_huc6260_palette_store ",
                   strlen("dynamic_huc6260_palette_store ")) == 0) {
            if (!controller_state_seen ||
                !tqr_trace_parse_palette_store(line, length, &pc, &address,
                                               &accumulator)) {
                return 0;
            }
            ++out->palette_store_count;
            out->palette_register_mask |= 1u << (address - 0x0402u);
            if (out->palette_store_count == 1u) {
                out->first_palette_store_pc = (uint16_t)pc;
                out->first_palette_store_accumulator = (uint8_t)accumulator;
            }
        } else if (length >= strlen("dynamic_huc6260_palette_word ") &&
                   memcmp(line, "dynamic_huc6260_palette_word ",
                          strlen("dynamic_huc6260_palette_word ")) == 0) {
            if (!controller_state_seen ||
                !tqr_trace_parse_palette_word(line, length, &palette_index,
                                              &palette_word)) {
                return 0;
            }
            ++out->palette_word_count;
            if (out->palette_word_count == 1u) {
                out->first_palette_word_index = (uint16_t)palette_index;
                out->first_palette_word_value = (uint16_t)palette_word;
            }
            out->palette_word_checksum = tqr_trace_fnv1a_u16(
                tqr_trace_fnv1a_u16(out->palette_word_checksum,
                                    (uint16_t)palette_index),
                (uint16_t)palette_word);
        }
    }
    if (out->palette_store_count == 0u ||
        !out->dynamic_cd_read_destination_span_verified ||
        !controller_state_seen) return 0;

    out->valid = 1;
    out->variant = live_trace.variant;
    out->dynamic_cd_read_record = live_trace.stage3_track02_record;
    out->dynamic_cd_read_record_cl = live_trace.cd_read_record_cl;
    out->dynamic_cd_read_record_dl = live_trace.cd_read_record_dl;
    out->dynamic_cd_read_record_ch = live_trace.cd_read_record_ch;
    out->dynamic_cd_read_destination = 0x3800u;
    out->dynamic_cd_read_verified = 1;
    out->dynamic_cd_read_registers_verified = 1;
    out->palette_store_observed_after_dynamic_read = 1;
    /* The current emulator receipt has no source-byte provenance. */
    out->palette_descriptor_relation_verified = 0;
    snprintf(out->track02_md5, sizeof(out->track02_md5), "%s", track02_md5);
    return 1;
}

int theron_v1_raw_loader_trace_import_mednafen_capture_file(
    const char *path,
    const char *track02_md5,
    Theron_V1RawLoaderTraceReceipt *out)
{
    FILE *file;
    long size;
    char *capture;
    int result;

    if (out) memset(out, 0, sizeof(*out));
    if (!path || !track02_md5 || !out || !(file = fopen(path, "rb"))) {
        return 0;
    }
    if (fseek(file, 0L, SEEK_END) != 0 || (size = ftell(file)) <= 0 ||
        (size_t)size > THERON_V1_RAW_LOADER_TRACE_MAX_BYTES ||
        fseek(file, 0L, SEEK_SET) != 0 ||
        !(capture = (char *)malloc((size_t)size + 1u))) {
        fclose(file);
        return 0;
    }
    if (fread(capture, 1u, (size_t)size, file) != (size_t)size) {
        fclose(file);
        free(capture);
        return 0;
    }
    fclose(file);
    capture[size] = '\0';
    result = theron_v1_raw_loader_trace_ingest_mednafen_capture(
        capture, track02_md5, out);
    free(capture);
    return result;
}

int theron_v1_raw_loader_trace_bind_track02_destination_span(
    const Theron_V1RawLoaderTraceReceipt *trace,
    const uint8_t *track02_data,
    size_t track02_size,
    const char *track02_md5,
    Theron_V1RawLoaderTraceReceipt *out)
{
    Theron_Track02Stage2DynamicPayloadReceipt payload;
    Theron_Track02SignalStatus status;

    if (out) memset(out, 0, sizeof(*out));
    if (!trace || !track02_data || !track02_md5 || !out || !trace->valid ||
        !trace->dynamic_cd_read_verified ||
        !trace->dynamic_cd_read_registers_verified ||
        !trace->dynamic_cd_read_destination_span_verified ||
        !trace->dynamic_cd_read_destination_span_bytes ||
        !trace->dynamic_cd_read_destination_span_checksum ||
        trace->dynamic_cd_read_record !=
            ((uint32_t)trace->dynamic_cd_read_record_cl |
             ((uint32_t)trace->dynamic_cd_read_record_dl << 8) |
             ((uint32_t)trace->dynamic_cd_read_record_ch << 16)) ||
        strcmp(trace->track02_md5, track02_md5) != 0) {
        return 0;
    }

    status = theron_v1_track02_inspect_stage2_dynamic_payload(
        track02_data, track02_size, track02_md5, &payload);
    if (status != THERON_TRACK02_SIGNAL_OK || !payload.valid ||
        payload.variant != trace->variant ||
        payload.track02_record != trace->dynamic_cd_read_record ||
        trace->dynamic_cd_read_destination_span_bytes > payload.user_data_bytes ||
        payload.user_data_offset > track02_size ||
        trace->dynamic_cd_read_destination_span_bytes >
            track02_size - payload.user_data_offset ||
        tqr_trace_fnv1a_bytes(
            track02_data + payload.user_data_offset,
            trace->dynamic_cd_read_destination_span_bytes) !=
            trace->dynamic_cd_read_destination_span_checksum) {
        return 0;
    }

    *out = *trace;
    out->dynamic_cd_read_raw_sector = payload.raw_sector;
    out->dynamic_cd_read_raw_offset = payload.raw_offset;
    out->dynamic_cd_read_user_data_offset = payload.user_data_offset;
    out->stage2_dynamic_payload_verified = 1;
    out->stage2_dynamic_payload_bytes = payload.user_data_bytes;
    out->stage2_dynamic_payload_checksum = payload.user_data_hash;
    out->dynamic_cd_read_media_span_verified = 1;
    return 1;
}

int theron_v1_raw_loader_trace_stage3_sector_receipt_from_bound_span(
    const Theron_V1RawLoaderTraceReceipt *trace,
    const Theron_Track02Stage2DynamicPayloadReceipt *payload,
    Theron_V1RawLoaderTraceStage3SectorReceipt *out)
{
    const char *expected_md5;

    if (out) memset(out, 0, sizeof(*out));
    if (!trace || !payload || !out || !trace->valid || !payload->valid ||
        !trace->dynamic_cd_read_verified ||
        !trace->dynamic_cd_read_registers_verified ||
        !trace->dynamic_cd_read_destination_span_verified ||
        !trace->dynamic_cd_read_media_span_verified ||
        trace->dynamic_cd_read_destination != 0x3800u ||
        !trace->dynamic_cd_read_destination_span_bytes ||
        !trace->dynamic_cd_read_destination_span_checksum ||
        (payload->variant != THERON_TRACK02_VARIANT_JP_BIN &&
         payload->variant != THERON_TRACK02_VARIANT_US_BIN) ||
        trace->variant != payload->variant ||
        trace->dynamic_cd_read_record != payload->track02_record ||
        trace->dynamic_cd_read_raw_sector != payload->raw_sector ||
        trace->dynamic_cd_read_raw_offset != payload->raw_offset ||
        trace->dynamic_cd_read_user_data_offset != payload->user_data_offset ||
        trace->dynamic_cd_read_destination_span_bytes >
            payload->user_data_bytes ||
        payload->user_data_bytes !=
            THERON_TRACK02_IPL_STAGE2_DYNAMIC_PAYLOAD_BYTES ||
        payload->raw_sector > SIZE_MAX / THERON_TRACK02_RAW_SECTOR_BYTES ||
        payload->raw_offset !=
            payload->raw_sector * THERON_TRACK02_RAW_SECTOR_BYTES ||
        payload->raw_offset > SIZE_MAX - 16u ||
        payload->user_data_offset != payload->raw_offset + 16u ||
        !payload->user_data_hash) {
        return 0;
    }

    expected_md5 = payload->variant == THERON_TRACK02_VARIANT_JP_BIN
        ? THERON_TRACK02_MD5_JP_BIN : THERON_TRACK02_MD5_US_BIN;
    if (strcmp(trace->track02_md5, expected_md5) != 0) return 0;

    out->valid = 1;
    out->variant = payload->variant;
    snprintf(out->track02_md5, sizeof(out->track02_md5), "%s",
             trace->track02_md5);
    out->stage3_track02_record = payload->track02_record;
    out->stage3_raw_sector = payload->raw_sector;
    out->stage3_raw_offset = payload->raw_offset;
    out->stage3_user_data_offset = payload->user_data_offset;
    out->stage3_user_data_bytes = payload->user_data_bytes;
    out->stage3_user_data_hash = payload->user_data_hash;
    out->observed_destination_span_bytes =
        trace->dynamic_cd_read_destination_span_bytes;
    out->observed_destination_span_checksum =
        trace->dynamic_cd_read_destination_span_checksum;
    out->observed_cd_read_to_media_span_verified = 1;
    out->stage3_handoff_record_proven = 1;
    return 1;
}

int theron_v1_raw_loader_trace_bind_later_e009_sector(
    const Theron_V1RawLoaderTraceReceipt *trace,
    const char *capture,
    const uint8_t *track02_data,
    size_t track02_size,
    const char *track02_md5,
    Theron_V1RawLoaderTraceLaterSectorReceipt *out)
{
    const char *cursor;
    const char *line;
    size_t length;
    unsigned int caller_pc = 0u;
    unsigned int return_pc = 0u;
    unsigned int sector_count = 0u;
    unsigned int cl = 0u;
    unsigned int dl = 0u;
    unsigned int ch = 0u;
    unsigned int record = 0u;
    unsigned int caller_opcode = 0u;
    unsigned int caller_target = 0u;
    unsigned int returned_caller_pc = 0u;
    unsigned int returned_pc = 0u;
    unsigned int returned_record = 0u;
    size_t source_count = 0u;
    size_t dispatch_count = 0u;
    size_t returned_count = 0u;
    size_t raw_sector_count;
    size_t first_raw_offset;
    size_t selector_ordinal;
    const char *expected_md5;
    Theron_Track02Stage2DynamicPayloadReceipt payload;
    Theron_V1Stage3ManifestEvidence manifest;
    uint32_t derived_record_base;
    uint32_t selector;

    if (out) memset(out, 0, sizeof(*out));
    if (!trace || !capture || !track02_data || !track02_md5 || !out ||
        !trace->valid || !trace->dynamic_cd_read_verified ||
        !trace->dynamic_cd_read_registers_verified ||
        !trace->dynamic_cd_read_destination_span_verified ||
        !trace->dynamic_cd_read_media_span_verified ||
        !trace->stage2_dynamic_payload_verified ||
        trace->dynamic_cd_read_destination != 0x3800u ||
        trace->dynamic_cd_read_destination_span_bytes != 32u ||
        strcmp(trace->track02_md5, track02_md5) != 0 ||
        (trace->variant != THERON_TRACK02_VARIANT_JP_BIN &&
         trace->variant != THERON_TRACK02_VARIANT_US_BIN) ||
        track02_size % THERON_TRACK02_RAW_SECTOR_BYTES != 0u) return 0;

    expected_md5 = trace->variant == THERON_TRACK02_VARIANT_JP_BIN
        ? THERON_TRACK02_MD5_JP_BIN : THERON_TRACK02_MD5_US_BIN;
    if (strcmp(track02_md5, expected_md5) != 0) return 0;
    if (theron_v1_track02_inspect_stage2_dynamic_payload(
            track02_data, track02_size, track02_md5, &payload) !=
            THERON_TRACK02_SIGNAL_OK ||
        !theron_v1_stage3_manifest_evidence_from_payload(
            track02_data, track02_size, &payload, &manifest) ||
        manifest.variant != trace->variant ||
        manifest.track02_record != trace->dynamic_cd_read_record ||
        manifest.first_descriptor.word2 == 0u ||
        manifest.track02_record < manifest.first_descriptor.word2) return 0;

    cursor = capture;
    while (tqr_trace_next_line(&cursor, &line, &length)) {
        if (length == strlen("source=mednafen-pce-instrumented") &&
            memcmp(line, "source=mednafen-pce-instrumented", length) == 0) {
            ++source_count;
        } else if (length >= strlen("later_system_card_e009_dispatch ") &&
                   memcmp(line, "later_system_card_e009_dispatch ",
                          strlen("later_system_card_e009_dispatch ")) == 0) {
            if (++dispatch_count != 1u || !tqr_trace_parse_later_e009_dispatch(
                    line, length, &caller_pc, &return_pc, &sector_count,
                    &cl, &dl, &ch, &record, &caller_opcode,
                    &caller_target)) return 0;
        } else if (length >= strlen("later_system_card_e009_return ") &&
                   memcmp(line, "later_system_card_e009_return ",
                          strlen("later_system_card_e009_return ")) == 0) {
            if (++returned_count != 1u || !tqr_trace_parse_later_e009_return(
                    line, length, &returned_caller_pc, &returned_pc,
                    &returned_record)) return 0;
        }
    }
    if (source_count != 1u || dispatch_count != 1u || returned_count != 1u ||
        return_pc != caller_pc + 3u || caller_opcode != 0x20u ||
        caller_target != 0xe009u || returned_caller_pc != caller_pc ||
        returned_pc != return_pc || returned_record != record ||
        record != (cl | (dl << 8) | (ch << 16)) ||
        record <= trace->dynamic_cd_read_record) return 0;

    raw_sector_count = track02_size / THERON_TRACK02_RAW_SECTOR_BYTES;
    if (record >= raw_sector_count || sector_count > raw_sector_count - record)
        return 0;
    derived_record_base = manifest.track02_record -
        manifest.first_descriptor.word2;
    if (record < derived_record_base ||
        record - derived_record_base > UINT16_MAX) return 0;
    selector = record - derived_record_base;
    for (selector_ordinal = 0u;
         selector_ordinal < manifest.descriptor_count;
         ++selector_ordinal) {
        if (manifest.descriptors[selector_ordinal].word2 == selector) break;
    }
    if (selector_ordinal == manifest.descriptor_count) return 0;
    first_raw_offset = (size_t)record * THERON_TRACK02_RAW_SECTOR_BYTES;

    out->valid = 1;
    out->variant = trace->variant;
    snprintf(out->track02_md5, sizeof(out->track02_md5), "%s", track02_md5);
    out->stage3_track02_record = trace->dynamic_cd_read_record;
    out->later_track02_record = record;
    out->descriptor_selector = (uint16_t)selector;
    out->descriptor_selector_ordinal = selector_ordinal;
    out->caller_pc = (uint16_t)caller_pc;
    out->return_pc = (uint16_t)return_pc;
    out->sector_count = (uint8_t)sector_count;
    out->first_raw_sector = record;
    out->first_raw_offset = first_raw_offset;
    out->first_user_data_offset = first_raw_offset +
        THERON_TRACK02_RAW_USER_DATA_OFFSET;
    out->user_data_bytes = (size_t)sector_count *
        THERON_TRACK02_RAW_USER_DATA_BYTES;
    out->user_data_hash = tqr_trace_fnv1a_user_data_range(
        track02_data, record, sector_count);
    out->later_e009_return_verified = 1;
    out->later_cd_read_to_media_verified = 1;
    out->descriptor_selector_bound = 1;
    return 1;
}

int theron_v1_raw_loader_trace_witness_later_e009_raw_sector(
    const Theron_V1RawLoaderTraceLaterSectorReceipt *later_receipt,
    const char *cd_capture,
    const uint8_t *track02_data,
    size_t track02_size,
    const char *track02_md5,
    Theron_V1RawLoaderTraceLaterRawSectorWitness *out)
{
    const char *cursor;
    const char *line;
    size_t length;
    size_t source_count = 0u;
    size_t matching_span_count = 0u;
    unsigned int lba = 0u;
    unsigned int bytes = 0u;
    unsigned int span_offset = 0u;
    unsigned int span_bytes = 0u;
    unsigned int checksum = 0u;
    unsigned int sector_checksum = 0u;
    unsigned int matched_lba = 0u;
    uint32_t expected_span_checksum;
    uint32_t expected_sector_checksum;
    const char *expected_md5;

    if (out) memset(out, 0, sizeof(*out));
    if (!later_receipt || !cd_capture || !track02_data || !track02_md5 ||
        !out || !later_receipt->valid ||
        !later_receipt->later_e009_return_verified ||
        !later_receipt->later_cd_read_to_media_verified ||
        !later_receipt->descriptor_selector_bound ||
        strcmp(later_receipt->track02_md5, track02_md5) != 0 ||
        later_receipt->first_raw_offset > track02_size ||
        THERON_TRACK02_RAW_SECTOR_BYTES >
            track02_size - later_receipt->first_raw_offset ||
        (later_receipt->variant != THERON_TRACK02_VARIANT_JP_BIN &&
         later_receipt->variant != THERON_TRACK02_VARIANT_US_BIN)) {
        return 0;
    }
    expected_md5 = later_receipt->variant == THERON_TRACK02_VARIANT_JP_BIN
        ? THERON_TRACK02_MD5_JP_BIN : THERON_TRACK02_MD5_US_BIN;
    if (strcmp(track02_md5, expected_md5) != 0) return 0;

    expected_span_checksum = tqr_trace_fnv1a_bytes(
        track02_data + later_receipt->first_raw_offset, 32u);
    expected_sector_checksum = tqr_trace_fnv1a_bytes(
        track02_data + later_receipt->first_raw_offset,
        THERON_TRACK02_RAW_SECTOR_BYTES);
    cursor = cd_capture;
    while (tqr_trace_next_line(&cursor, &line, &length)) {
        if (length == strlen("source=mednafen-pce-instrumented-cd") &&
            memcmp(line, "source=mednafen-pce-instrumented-cd", length) == 0) {
            ++source_count;
        } else if (length >= strlen("cd_interface_raw_sector_read ") &&
                   memcmp(line, "cd_interface_raw_sector_read ",
                          strlen("cd_interface_raw_sector_read ")) == 0) {
            if (!tqr_trace_parse_raw_sector_span(line, length, &lba, &bytes,
                                                  &span_offset, &span_bytes,
                                                  &checksum, &sector_checksum)) return 0;
            if (checksum == expected_span_checksum &&
                sector_checksum == expected_sector_checksum) {
                ++matching_span_count;
                matched_lba = lba;
            }
        }
    }
    if (source_count != 1u || matching_span_count != 1u) return 0;

    out->valid = 1;
    out->variant = later_receipt->variant;
    snprintf(out->track02_md5, sizeof(out->track02_md5), "%s", track02_md5);
    out->later_track02_record = later_receipt->later_track02_record;
    out->descriptor_selector = later_receipt->descriptor_selector;
    out->descriptor_selector_ordinal = later_receipt->descriptor_selector_ordinal;
    out->observed_raw_sector_lba = (int)matched_lba;
    out->observed_raw_sector_bytes = THERON_TRACK02_RAW_SECTOR_BYTES;
    out->observed_raw_sector_checksum = expected_sector_checksum;
    out->observed_raw_sector_span_bytes = 32u;
    out->observed_raw_sector_span_checksum = expected_span_checksum;
    out->same_capture_raw_sector_span_verified = 1;
    return 1;
}

int theron_v1_raw_loader_trace_bind_coalesced_later_e009_raw_sector(
    const char *capture,
    const uint8_t *track02_data,
    size_t track02_size,
    const char *track02_md5,
    Theron_V1RawLoaderTraceCoalescedLaterReceipt *out)
{
    Theron_Track02Stage2DynamicPayloadReceipt payload;
    Theron_V1Stage3ManifestEvidence manifest;
    const char *cursor;
    const char *line;
    size_t length;
    size_t source_count = 0u;
    size_t dynamic_count = 0u;
    size_t dispatch_count = 0u;
    size_t sector_count = 0u;
    size_t destination_count = 0u;
    size_t destination_payload_count = 0u;
    size_t return_count = 0u;
    size_t post_return_count = 0u;
    size_t dynamic_line = 0u;
    size_t dispatch_line = 0u;
    size_t sector_line = 0u;
    size_t destination_line = 0u;
    size_t destination_payload_line = 0u;
    size_t return_line = 0u;
    size_t post_return_line = 0u;
    size_t line_number = 0u;
    size_t ordinal;
    unsigned int caller_pc = 0u;
    unsigned int return_pc = 0u;
    unsigned int read_count = 0u;
    unsigned int cl = 0u;
    unsigned int dl = 0u;
    unsigned int ch = 0u;
    unsigned int record = 0u;
    unsigned int caller_opcode = 0u;
    unsigned int caller_target = 0u;
    unsigned int return_caller_pc = 0u;
    unsigned int return_return_pc = 0u;
    unsigned int return_record = 0u;
    unsigned int lba = 0u;
    unsigned int bytes = 0u;
    unsigned int span_offset = 0u;
    unsigned int span_bytes = 0u;
    unsigned int span_checksum = 0u;
    unsigned int sector_checksum = 0u;
    unsigned int destination_caller_pc = 0u;
    unsigned int destination_return_pc = 0u;
    unsigned int destination_record = 0u;
    unsigned int destination = 0u;
    unsigned int destination_bytes = 0u;
    unsigned int destination_checksum = 0u;
    unsigned int destination_payload_caller_pc = 0u;
    unsigned int destination_payload_return_pc = 0u;
    unsigned int destination_payload_record = 0u;
    unsigned int destination_payload_destination = 0u;
    unsigned int destination_payload_bytes = 0u;
    unsigned int destination_payload_checksum = 0u;
    unsigned int post_return_caller_pc = 0u;
    unsigned int post_return_return_pc = 0u;
    unsigned int post_return_record = 0u;
    unsigned int post_return_resume_pc = 0u;
    unsigned int post_return_next_pc = 0u;
    uint32_t expected_span_checksum;
    uint32_t expected_sector_checksum;
    uint32_t derived_base;
    uint32_t selector;
    const char *variant_name;
    char expected_dynamic[256];

    if (out) memset(out, 0, sizeof(*out));
    if (!capture || !track02_data || !track02_md5 || !out ||
        track02_size % THERON_TRACK02_RAW_SECTOR_BYTES != 0u ||
        theron_v1_track02_inspect_stage2_dynamic_payload(
            track02_data, track02_size, track02_md5, &payload) !=
            THERON_TRACK02_SIGNAL_OK ||
        !theron_v1_stage3_manifest_evidence_from_payload(
            track02_data, track02_size, &payload, &manifest)) return 0;

    if (manifest.variant == THERON_TRACK02_VARIANT_JP_BIN) {
        variant_name = "jp_bin";
    } else if (manifest.variant == THERON_TRACK02_VARIANT_US_BIN) {
        variant_name = "us_bin";
    } else {
        return 0;
    }
    snprintf(expected_dynamic, sizeof(expected_dynamic),
             "dynamic_cd_read_transaction pc=4090 return_pc=4093 "
             "sector_count=01 destination=3800 record_register_mask=07 "
             "record_cl=%02x record_dl=%02x record_ch=%02x variant=%s "
             "record=%x",
             payload.track02_record & 0xffu,
             (payload.track02_record >> 8) & 0xffu,
             (payload.track02_record >> 16) & 0xffu, variant_name,
             payload.track02_record);

    cursor = capture;
    while (tqr_trace_next_line(&cursor, &line, &length)) {
        ++line_number;
        if (length == strlen("source=mednafen-pce-instrumented-coalesced") &&
            memcmp(line, "source=mednafen-pce-instrumented-coalesced",
                   length) == 0) {
            ++source_count;
        } else if (length >= strlen("dynamic_cd_read_transaction ") &&
                   memcmp(line, "dynamic_cd_read_transaction ",
                          strlen("dynamic_cd_read_transaction ")) == 0) {
            if (++dynamic_count != 1u || length != strlen(expected_dynamic) ||
                memcmp(line, expected_dynamic, length) != 0) return 0;
            dynamic_line = line_number;
        } else if (length >= strlen("later_system_card_e009_dispatch ") &&
                   memcmp(line, "later_system_card_e009_dispatch ",
                          strlen("later_system_card_e009_dispatch ")) == 0) {
            if (++dispatch_count != 1u ||
                !tqr_trace_parse_later_e009_dispatch(
                    line, length, &caller_pc, &return_pc, &read_count, &cl,
                    &dl, &ch, &record, &caller_opcode, &caller_target)) return 0;
            dispatch_line = line_number;
        } else if (length >= strlen("cd_interface_raw_sector_read ") &&
                   memcmp(line, "cd_interface_raw_sector_read ",
                          strlen("cd_interface_raw_sector_read ")) == 0) {
            if (++sector_count != 1u || !tqr_trace_parse_raw_sector_span(
                    line, length, &lba, &bytes, &span_offset, &span_bytes,
                    &span_checksum, &sector_checksum)) return 0;
            sector_line = line_number;
        } else if (length >= strlen("later_system_card_e009_destination_span ") &&
                   memcmp(line, "later_system_card_e009_destination_span ",
                          strlen("later_system_card_e009_destination_span ")) == 0) {
            if (++destination_count != 1u ||
                !tqr_trace_parse_later_e009_destination_span(
                    line, length, &destination_caller_pc,
                    &destination_return_pc, &destination_record,
                    &destination, &destination_bytes,
                    &destination_checksum)) return 0;
            destination_line = line_number;
        } else if (length >= strlen("later_system_card_e009_destination_payload ") &&
                   memcmp(line, "later_system_card_e009_destination_payload ",
                          strlen("later_system_card_e009_destination_payload ")) == 0) {
            if (++destination_payload_count != 1u ||
                !tqr_trace_parse_later_e009_destination_payload(
                    line, length, &destination_payload_caller_pc,
                    &destination_payload_return_pc, &destination_payload_record,
                    &destination_payload_destination,
                    &destination_payload_bytes,
                    &destination_payload_checksum)) return 0;
            destination_payload_line = line_number;
        } else if (length >= strlen("later_system_card_e009_return ") &&
                   memcmp(line, "later_system_card_e009_return ",
                          strlen("later_system_card_e009_return ")) == 0) {
            if (++return_count != 1u || !tqr_trace_parse_later_e009_return(
                    line, length, &return_caller_pc, &return_return_pc,
                    &return_record)) return 0;
            return_line = line_number;
        } else if (length >= strlen("later_system_card_e009_post_return_step ") &&
                   memcmp(line, "later_system_card_e009_post_return_step ",
                          strlen("later_system_card_e009_post_return_step ")) == 0) {
            if (++post_return_count != 1u ||
                !tqr_trace_parse_later_e009_post_return_step(
                    line, length, &post_return_caller_pc,
                    &post_return_return_pc, &post_return_record,
                    &post_return_resume_pc,
                    &post_return_next_pc)) return 0;
            post_return_line = line_number;
        }
    }
    if (source_count != 1u || dynamic_count != 1u || dispatch_count != 1u ||
        sector_count != 1u || destination_count != 1u ||
        destination_payload_count != 1u || return_count != 1u ||
        post_return_count != 1u ||
        !(dynamic_line < dispatch_line && dispatch_line < sector_line &&
          sector_line < destination_line &&
          destination_line < destination_payload_line &&
          destination_payload_line < return_line &&
          return_line < post_return_line) ||
        return_pc != caller_pc + 3u || caller_opcode != 0x20u ||
        caller_target != 0xe009u ||
        return_caller_pc != caller_pc || return_return_pc != return_pc ||
        return_record != record || destination_caller_pc != caller_pc ||
        destination_return_pc != return_pc || destination_record != record ||
        destination_payload_caller_pc != caller_pc ||
        destination_payload_return_pc != return_pc ||
        destination_payload_record != record ||
        destination_payload_destination != destination ||
        post_return_caller_pc != caller_pc ||
        post_return_return_pc != return_pc || post_return_record != record ||
        post_return_resume_pc != return_pc ||
        record != (cl | (dl << 8) | (ch << 16)) ||
        read_count == 0u || record <= manifest.track02_record ||
        record >= track02_size / THERON_TRACK02_RAW_SECTOR_BYTES ||
        read_count > track02_size / THERON_TRACK02_RAW_SECTOR_BYTES - record ||
        manifest.first_descriptor.word2 == 0u ||
        record < manifest.track02_record - manifest.first_descriptor.word2) {
        return 0;
    }
    derived_base = manifest.track02_record - manifest.first_descriptor.word2;
    selector = record - derived_base;
    if (selector > UINT16_MAX) return 0;
    for (ordinal = 0u; ordinal < manifest.descriptor_count; ++ordinal) {
        if (manifest.descriptors[ordinal].word2 == (uint16_t)selector) break;
    }
    if (ordinal == manifest.descriptor_count) return 0;

    expected_span_checksum = tqr_trace_fnv1a_bytes(
        track02_data + (size_t)record * THERON_TRACK02_RAW_SECTOR_BYTES, 32u);
    expected_sector_checksum = tqr_trace_fnv1a_bytes(
        track02_data + (size_t)record * THERON_TRACK02_RAW_SECTOR_BYTES,
        THERON_TRACK02_RAW_SECTOR_BYTES);
    if (span_checksum != expected_span_checksum ||
        sector_checksum != expected_sector_checksum ||
        destination_checksum != tqr_trace_fnv1a_bytes(
            track02_data + (size_t)record * THERON_TRACK02_RAW_SECTOR_BYTES +
                THERON_TRACK02_RAW_USER_DATA_OFFSET,
            destination_bytes) ||
        destination_payload_checksum != tqr_trace_fnv1a_bytes(
            track02_data + (size_t)record * THERON_TRACK02_RAW_SECTOR_BYTES +
                THERON_TRACK02_RAW_USER_DATA_OFFSET,
            destination_payload_bytes)) return 0;

    out->valid = 1;
    out->variant = manifest.variant;
    snprintf(out->track02_md5, sizeof(out->track02_md5), "%s", track02_md5);
    out->stage3_track02_record = manifest.track02_record;
    out->later_track02_record = record;
    out->descriptor_selector = (uint16_t)selector;
    out->descriptor_selector_ordinal = ordinal;
    out->caller_pc = (uint16_t)caller_pc;
    out->return_pc = (uint16_t)return_pc;
    out->later_caller_opcode = (uint8_t)caller_opcode;
    out->later_caller_target = (uint16_t)caller_target;
    out->later_caller_control_verified = 1;
    out->sector_count = (uint8_t)read_count;
    out->observed_raw_sector_lba = (int)lba;
    out->observed_raw_sector_checksum = expected_sector_checksum;
    out->observed_raw_sector_span_checksum = expected_span_checksum;
    out->later_local_destination = (uint16_t)destination;
    out->later_destination_span_bytes = destination_bytes;
    out->later_destination_span_checksum = destination_checksum;
    out->later_destination_local_ram_verified = 1;
    out->later_destination_media_span_verified = 1;
    out->later_destination_payload_bytes = destination_payload_bytes;
    out->later_destination_payload_checksum = destination_payload_checksum;
    out->later_destination_payload_verified = 1;
    out->later_post_return_resume_pc = (uint16_t)post_return_resume_pc;
    out->later_post_return_next_pc = (uint16_t)post_return_next_pc;
    out->later_post_return_step_verified = 1;
    out->observation_order_verified = 1;
    out->selector_sector_bytes_verified = 1;
    return 1;
}

static uint32_t tqr_trace_initial_level_handoff_hash(
    const Theron_V1RawLoaderTraceInitialLevelHandoffReceipt *receipt)
{
    uint32_t hash;

    if (!receipt) return 0u;
    hash = receipt->initial_level_boundary.receipt_hash;
    hash ^= receipt->initial_level_route.route_hash;
    hash *= 16777619u;
    hash ^= receipt->observed_track02_record;
    hash *= 16777619u;
    hash ^= receipt->descriptor_selector;
    hash *= 16777619u;
    hash ^= (uint32_t)receipt->descriptor_selector_ordinal;
    hash *= 16777619u;
    hash ^= (uint32_t)receipt->complete_payload_bytes;
    hash *= 16777619u;
    hash ^= receipt->complete_payload_checksum;
    hash *= 16777619u;
    hash ^= receipt->complete_payload_witness_proven ? 1u : 0u;
    hash *= 16777619u;
    return hash;
}

int theron_v1_raw_loader_trace_initial_level_handoff_is_complete(
    const Theron_V1RawLoaderTraceInitialLevelHandoffReceipt *receipt)
{
    return receipt && receipt->valid &&
           receipt->coalesced_loader_cd_receipt_proven &&
           receipt->initial_level_record_proven &&
           receipt->complete_initial_level_envelope_proven &&
           receipt->complete_payload_witness_proven &&
           receipt->complete_payload_bytes == THERON_TRACK02_RAW_USER_DATA_BYTES &&
           receipt->complete_payload_checksum != 0u &&
           receipt->receipt_hash != 0u &&
           receipt->receipt_hash == tqr_trace_initial_level_handoff_hash(receipt);
}

int theron_v1_raw_loader_trace_bind_initial_level_handoff(
    const Theron_V1RawLoaderTraceCoalescedLaterReceipt *coalesced_receipt,
    const uint8_t *track02_data,
    size_t track02_size,
    const char *track02_md5,
    Theron_V1RawLoaderTraceInitialLevelHandoffReceipt *out)
{
    Theron_Track02InitialLevelObjectBoundaryReceipt boundary;
    Theron_Track02InitialLevelLoaderRoute route;

    if (out) memset(out, 0, sizeof(*out));
    if (!coalesced_receipt || !track02_data || !track02_md5 || !out ||
        !coalesced_receipt->valid ||
        !coalesced_receipt->observation_order_verified ||
        !coalesced_receipt->selector_sector_bytes_verified ||
        !coalesced_receipt->later_destination_local_ram_verified ||
        !coalesced_receipt->later_destination_media_span_verified ||
        !coalesced_receipt->later_destination_payload_verified ||
        !coalesced_receipt->later_caller_control_verified ||
        coalesced_receipt->later_caller_opcode != 0x20u ||
        coalesced_receipt->later_caller_target != 0xe009u ||
        coalesced_receipt->later_destination_span_bytes != 32u ||
        coalesced_receipt->later_destination_payload_bytes !=
            THERON_TRACK02_RAW_USER_DATA_BYTES ||
        !coalesced_receipt->later_destination_span_checksum ||
        !coalesced_receipt->later_destination_payload_checksum ||
        !coalesced_receipt->later_post_return_step_verified ||
        coalesced_receipt->later_post_return_resume_pc !=
            coalesced_receipt->return_pc ||
        coalesced_receipt->sector_count != 1u ||
        strcmp(coalesced_receipt->track02_md5, track02_md5) != 0 ||
        (coalesced_receipt->variant != THERON_TRACK02_VARIANT_JP_BIN &&
         coalesced_receipt->variant != THERON_TRACK02_VARIANT_US_BIN) ||
        theron_v1_track02_variant_for_md5(track02_md5) !=
            coalesced_receipt->variant ||
        theron_v1_track02_capture_initial_level_object_boundary(
            track02_data, track02_size, track02_md5, &boundary) !=
            THERON_TRACK02_SIGNAL_OK ||
        !boundary.valid ||
        !boundary.promotion_blocked || boundary.object_table_parsed ||
        boundary.object_table_semantics_proven ||
        coalesced_receipt->later_track02_record != boundary.track02_record ||
        theron_v1_track02_load_initial_level_loader_route(
            track02_data, track02_size, track02_md5,
            THERON_DUNGEON_1_HALL_OF_RECORDS, 0, &route) !=
            THERON_TRACK02_SIGNAL_OK ||
        !route.valid || route.semantics.envelope.track02_record !=
            boundary.track02_record ||
        route.semantics.object_tail_semantics_proven ||
        route.fallback_visuals_allowed) {
        return 0;
    }

    /* Keep the final admission bound to the bytes that the trace claims
     * reached local RAM, rather than trusting a checksum copied from a
     * prior receipt. */
    if (coalesced_receipt->later_track02_record >
            track02_size / THERON_TRACK02_RAW_SECTOR_BYTES ||
        (size_t)coalesced_receipt->later_track02_record *
            THERON_TRACK02_RAW_SECTOR_BYTES > track02_size ||
        THERON_TRACK02_RAW_USER_DATA_OFFSET >
            track02_size - (size_t)coalesced_receipt->later_track02_record *
                THERON_TRACK02_RAW_SECTOR_BYTES ||
        coalesced_receipt->later_destination_span_bytes >
            track02_size - ((size_t)coalesced_receipt->later_track02_record *
                THERON_TRACK02_RAW_SECTOR_BYTES +
                THERON_TRACK02_RAW_USER_DATA_OFFSET) ||
        tqr_trace_fnv1a_bytes(track02_data +
            (size_t)coalesced_receipt->later_track02_record *
                THERON_TRACK02_RAW_SECTOR_BYTES +
            THERON_TRACK02_RAW_USER_DATA_OFFSET,
            coalesced_receipt->later_destination_span_bytes) !=
            coalesced_receipt->later_destination_span_checksum ||
        tqr_trace_fnv1a_bytes(track02_data +
            (size_t)coalesced_receipt->later_track02_record *
                THERON_TRACK02_RAW_SECTOR_BYTES +
            THERON_TRACK02_RAW_USER_DATA_OFFSET,
            coalesced_receipt->later_destination_payload_bytes) !=
            coalesced_receipt->later_destination_payload_checksum) {
        return 0;
    }

    out->valid = 1;
    out->variant = coalesced_receipt->variant;
    snprintf(out->track02_md5, sizeof(out->track02_md5), "%s", track02_md5);
    out->observed_track02_record = coalesced_receipt->later_track02_record;
    out->descriptor_selector = coalesced_receipt->descriptor_selector;
    out->descriptor_selector_ordinal =
        coalesced_receipt->descriptor_selector_ordinal;
    out->coalesced_loader_cd_receipt_proven = 1;
    out->initial_level_record_proven = 1;
    out->complete_initial_level_envelope_proven = 1;
    out->complete_payload_bytes =
        coalesced_receipt->later_destination_payload_bytes;
    out->complete_payload_checksum =
        coalesced_receipt->later_destination_payload_checksum;
    out->complete_payload_witness_proven = 1;
    out->initial_level_boundary = boundary;
    out->initial_level_route = route;
    out->object_tail_semantics_proven = 0;
    out->fallback_visuals_allowed = 0;

    out->receipt_hash = tqr_trace_initial_level_handoff_hash(out);
    return theron_v1_raw_loader_trace_initial_level_handoff_is_complete(out);
}

int theron_v1_raw_loader_trace_capture_manifest_matches(
    const Theron_V1CaptureManifest *manifest,
    const char *track02_path,
    const char *track02_md5,
    const char *system_card_path,
    const char *system_card_md5,
    const char *trace_path,
    const char *trace_md5)
{
    return track02_md5 &&
           (strcmp(track02_md5, THERON_TRACK02_MD5_JP_BIN) == 0 ||
            strcmp(track02_md5, THERON_TRACK02_MD5_US_BIN) == 0) &&
           system_card_md5 &&
           strcmp(system_card_md5,
                  "ff1a674273fe3540ccef576376407d1d") == 0 &&
           theron_v1_capture_manifest_matches_preflight_inputs(
               manifest, track02_path, track02_md5, system_card_path,
               system_card_md5, trace_path, trace_md5);
}

int theron_v1_raw_loader_trace_final_bind(
    const Theron_V1RawLoaderTraceReceipt *trace,
    const Theron_StartupMediaStateReceipt *media,
    Theron_V1RawLoaderTraceReceipt *out)
{
    Theron_StartupRawBitmapRouteReceipt soul_room;
    size_t dynamic_span_first_raw_offset;
    size_t dynamic_span_end_raw_offset;
    size_t soul_room_end_raw_offset;

    if (out) memset(out, 0, sizeof(*out));
    if (!trace || !media || !out || !trace->valid ||
        !trace->dynamic_cd_read_verified ||
        !trace->dynamic_cd_read_registers_verified ||
        !trace->dynamic_cd_read_destination_span_verified ||
        !trace->dynamic_cd_read_media_span_verified ||
        !trace->stage2_dynamic_payload_verified ||
        trace->stage2_dynamic_payload_bytes !=
            THERON_TRACK02_IPL_STAGE2_DYNAMIC_PAYLOAD_BYTES ||
        !trace->stage2_dynamic_payload_checksum ||
        trace->dynamic_cd_read_destination_span_bytes != 32u ||
        !trace->dynamic_cd_read_destination_span_checksum ||
        !trace->palette_store_observed_after_dynamic_read ||
        strcmp(trace->track02_md5, media->track02_md5) != 0 ||
        trace->variant != (Theron_Track02Variant)media->track02_variant ||
        !theron_v1_startup_media_state_receipt_has_complete_bitmap_routes(media) ||
        !media->startup_bitmap_raw_route_mask ||
        !media->startup_bitmap_atlas_checksum) {
        return 0;
    }
    if (!theron_v1_startup_media_consume_raw_bitmap_route(
            media, THERON_TRACK02_STARTUP_BITMAP_ROUTE_SOUL_ROOM,
            &soul_room) || !soul_room.valid || !soul_room.raw_source_verified ||
        soul_room.variant != trace->variant ||
        strcmp(soul_room.track02_md5, trace->track02_md5) != 0 ||
        soul_room.route_bit != THERON_TRACK02_STARTUP_BITMAP_ROUTE_SOUL_ROOM ||
        soul_room.tile_count == 0u || soul_room.checksum == 0u ||
        soul_room.first_raw_offset > soul_room.last_raw_offset ||
        soul_room.last_raw_offset >
            SIZE_MAX - THERON_TRACK02_STARTUP_BITMAP_TILE_BYTES ||
        trace->dynamic_cd_read_raw_offset >
            SIZE_MAX - THERON_TRACK02_RAW_USER_DATA_OFFSET ||
        trace->dynamic_cd_read_destination_span_bytes >
            SIZE_MAX - (trace->dynamic_cd_read_raw_offset +
                        THERON_TRACK02_RAW_USER_DATA_OFFSET)) {
        return 0;
    }
    dynamic_span_first_raw_offset = trace->dynamic_cd_read_raw_offset +
        THERON_TRACK02_RAW_USER_DATA_OFFSET;
    dynamic_span_end_raw_offset = dynamic_span_first_raw_offset +
        trace->dynamic_cd_read_destination_span_bytes;
    soul_room_end_raw_offset = soul_room.last_raw_offset +
        THERON_TRACK02_STARTUP_BITMAP_TILE_BYTES;
    if (!(dynamic_span_end_raw_offset <= soul_room.first_raw_offset ||
          soul_room_end_raw_offset <= dynamic_span_first_raw_offset)) {
        return 0;
    }
    *out = *trace;
    out->soul_room_raw_route_verified = 1;
    out->soul_room_first_raw_offset = soul_room.first_raw_offset;
    out->soul_room_last_raw_offset = soul_room.last_raw_offset;
    out->soul_room_checksum = soul_room.checksum;
    out->soul_room_route_disjoint_from_dynamic_span = 1;
    out->bitmap_route_mask = media->startup_bitmap_raw_route_mask;
    out->bitmap_atlas_checksum = media->startup_bitmap_atlas_checksum;
    return 1;
}
