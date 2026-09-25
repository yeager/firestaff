#include "theron_v1_raw_loader_trace.h"
#include "theron_v1_track02.h"

#include "theron_v1_irq2_live_trace_gate.h"
#include "theron_v1_later_record_correlation.h"
#include "theron_v1_stage3_irq2_dispatch.h"
#include "theron_v1_stage3_manifest_evidence.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define THERON_V1_RAW_LOADER_TRACE_MAX_BYTES (1024u * 1024u)
#define TQR_TRACE_TRACK02_LBA_BASE 3009u

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

int theron_v1_raw_loader_trace_track02_byte_for_scsi_source(
    const uint8_t *track02_data, size_t track02_size, const char *track02_md5,
    uint32_t source_lba, uint32_t source_offset,
    uint32_t *out_track02_record, uint8_t *out_byte)
{
    uint32_t record;
    size_t byte_offset;

    if (out_track02_record) *out_track02_record = 0u;
    if (out_byte) *out_byte = 0u;
    if (!track02_data || !track02_md5 || !out_track02_record || !out_byte ||
        source_lba < TQR_TRACE_TRACK02_LBA_BASE) {
        return 0;
    }
    record = source_lba - TQR_TRACE_TRACK02_LBA_BASE;

    if (strcmp(track02_md5, THERON_TRACK02_MD5_US_BIN) == 0 ||
        strcmp(track02_md5, THERON_TRACK02_MD5_JP_BIN) == 0) {
        if (track02_size % THERON_TRACK02_RAW_SECTOR_BYTES != 0u ||
            source_offset < THERON_TRACK02_RAW_USER_DATA_OFFSET ||
            source_offset >= THERON_TRACK02_RAW_USER_DATA_OFFSET +
                THERON_TRACK02_RAW_USER_DATA_BYTES ||
            (size_t)record >= track02_size / THERON_TRACK02_RAW_SECTOR_BYTES) {
            return 0;
        }
        byte_offset = (size_t)record * THERON_TRACK02_RAW_SECTOR_BYTES +
            source_offset;
    } else if (strcmp(track02_md5, THERON_TRACK02_MD5_US_ISO) == 0 ||
               strcmp(track02_md5, THERON_TRACK02_MD5_JP_REV1_ISO) == 0 ||
               strcmp(track02_md5, THERON_TRACK02_MD5_JP_ISO) == 0) {
        if (track02_size % THERON_TRACK02_RAW_USER_DATA_BYTES != 0u ||
            source_offset >= THERON_TRACK02_RAW_USER_DATA_BYTES ||
            (size_t)record >= track02_size / THERON_TRACK02_RAW_USER_DATA_BYTES) {
            return 0;
        }
        byte_offset = (size_t)record * THERON_TRACK02_RAW_USER_DATA_BYTES +
            source_offset;
    } else {
        return 0;
    }

    if (byte_offset >= track02_size) return 0;
    *out_track02_record = record;
    *out_byte = track02_data[byte_offset];
    return 1;
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

static int tqr_trace_parse_stage3_irq2_resume(
    const char *line, size_t length, unsigned int *out_entry_pc,
    unsigned int *out_selector, unsigned int *out_continuation_pc,
    unsigned int *out_resumed_pc, unsigned int *out_next_pc)
{
    int consumed = 0;

    if (!line || !out_entry_pc || !out_selector || !out_continuation_pc ||
        !out_resumed_pc || !out_next_pc) return 0;
    return sscanf(line,
                  "stage3_irq2_resume entry_pc=%x selector=%x continuation_pc=%x resumed_pc=%x next_pc=%x%n",
                  out_entry_pc, out_selector, out_continuation_pc,
                  out_resumed_pc, out_next_pc, &consumed) == 5 &&
           consumed == (int)length && *out_entry_pc <= 0xffffu &&
           *out_selector <= 0xffu && *out_continuation_pc <= 0xffffu &&
           *out_resumed_pc <= 0xffffu && *out_next_pc <= 0xffffu;
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

static int tqr_trace_hex_byte(const char *text, uint8_t *out)
{
    unsigned int value = 0u;
    char pair[3];

    if (!text || !out) return 0;
    pair[0] = text[0];
    pair[1] = text[1];
    pair[2] = '\0';
    if (sscanf(pair, "%2x", &value) != 1 || value > 0xffu) return 0;
    *out = (uint8_t)value;
    return 1;
}

static int tqr_trace_parse_scsi_read6(const char *line, size_t length,
                                       unsigned int *out_generation,
                                       unsigned int *out_lba,
                                       unsigned int *out_sector_count,
                                       uint8_t out_cdb[6])
{
    int consumed = 0;
    unsigned int opcode = 0u;
    unsigned int lba = 0u;
    unsigned int sector_count = 0u;
    char cdb[13] = {0};
    uint8_t bytes[6];
    size_t index;
    unsigned int decoded_lba;
    unsigned int decoded_count;

    if (!line || !out_generation || !out_lba || !out_sector_count ||
        !out_cdb) return 0;
    if (sscanf(line,
               "scsi_read_command generation=%u opcode=%x cdb=%12[0-9a-f] start_lba=%u sector_count=%u%n",
               out_generation, &opcode, cdb, &lba, &sector_count,
               &consumed) != 5 || consumed != (int)length || opcode != 0x08u ||
        !sector_count) return 0;
    for (index = 0u; index < 6u; ++index) {
        if (!tqr_trace_hex_byte(cdb + index * 2u, &bytes[index])) return 0;
    }
    decoded_lba = ((unsigned int)(bytes[1] & 0x1fu) << 16) |
        ((unsigned int)bytes[2] << 8) | bytes[3];
    decoded_count = bytes[4] ? bytes[4] : 256u;
    if (bytes[0] != 0x08u || bytes[5] != 0u || decoded_lba != lba ||
        decoded_count != sector_count) return 0;
    memcpy(out_cdb, bytes, sizeof(bytes));
    *out_lba = lba;
    *out_sector_count = sector_count;
    return 1;
}

static int tqr_trace_parse_fifo_origin_main_ram(
    const char *line, size_t length, unsigned int *out_generation,
    unsigned int *out_lba, unsigned int *out_offset,
    unsigned long long *out_fifo_sequence, unsigned int *out_destination,
    unsigned int *out_value, unsigned int *out_writer_physical_pc)
{
    int consumed = 0;
    unsigned int reader_pc = 0u;
    unsigned int logical_destination = 0u;
    unsigned int writer_pc = 0u;

    if (!line || !out_generation || !out_lba || !out_offset ||
        !out_fifo_sequence || !out_destination || !out_value ||
        !out_writer_physical_pc) return 0;
    return sscanf(line,
                  "pce_cd_fifo_origin_main_ram_receipt generation=%u source_lba=%u source_offset=%u fifo_sequence=%llu reader_pc=%x logical_destination=%x physical_destination=%x writer_pc=%x writer_physical_pc=%x value=%x%n",
                  out_generation, out_lba, out_offset, out_fifo_sequence,
                  &reader_pc, &logical_destination, out_destination,
                  &writer_pc, out_writer_physical_pc, out_value,
                  &consumed) == 10 && consumed == (int)length &&
           *out_offset < THERON_TRACK02_RAW_SECTOR_BYTES &&
           *out_destination >= 0x1f0000u && *out_destination < 0x1f8000u &&
           *out_value <= 0xffu && *out_writer_physical_pc >= 0x1f0000u &&
           *out_writer_physical_pc < 0x1f8000u;
}

static int tqr_trace_parse_fifo_origin_main_ram_consumer(
    const char *line, size_t length, unsigned int *out_generation,
    unsigned int *out_lba, unsigned int *out_offset,
    unsigned long long *out_fifo_sequence, unsigned int *out_physical,
    unsigned int *out_value, unsigned int *out_reader_physical_pc)
{
    int consumed = 0;
    unsigned int sequence = 0u;
    unsigned int logical_address = 0u;
    unsigned int reader_pc = 0u;

    if (!line || !out_generation || !out_lba || !out_offset ||
        !out_fifo_sequence || !out_physical || !out_value ||
        !out_reader_physical_pc) return 0;
    return sscanf(line,
                  "pce_cd_fifo_origin_main_ram_consumer sequence=%u generation=%u source_lba=%u source_offset=%u fifo_sequence=%llu logical_address=%x physical_address=%x value=%x reader_pc=%x reader_physical_pc=%x%n",
                  &sequence, out_generation, out_lba, out_offset,
                  out_fifo_sequence, &logical_address, out_physical,
                  out_value, &reader_pc, out_reader_physical_pc,
                  &consumed) == 10 && consumed == (int)length &&
           *out_offset < THERON_TRACK02_RAW_SECTOR_BYTES &&
           *out_physical >= 0x1f0000u && *out_physical < 0x1f8000u &&
           *out_value <= 0xffu && *out_reader_physical_pc >= 0x1f0000u &&
           *out_reader_physical_pc < 0x1f8000u;
}

int theron_v1_raw_loader_trace_bind_game_owned_fifo_payload(
    const char *capture, const uint8_t *track02_data, size_t track02_size,
    const char *track02_md5, Theron_V1RawLoaderTraceGamePayloadReceipt *out)
{
    const char *cursor;
    const char *line;
    size_t length;
    size_t line_number = 0u;
    size_t source_line = 0u;
    size_t dispatch_line = 0u;
    size_t first_cdb_line = 0u;
    size_t scsi_line = 0u;
    size_t origin_line = 0u;
    size_t consumer_line = 0u;
    unsigned int source_count = 0u;
    unsigned int dispatch_count = 0u;
    unsigned int cdb_count = 0u;
    unsigned int scsi_count = 0u;
    unsigned int origin_count = 0u;
    unsigned int consumer_count = 0u;
    unsigned int dispatch_sequence = 0u;
    unsigned int dispatch_logical_pc = 0u;
    unsigned int dispatch_physical_pc = 0u;
    unsigned int dispatch_a = 0u;
    unsigned int dispatch_x = 0u;
    unsigned int dispatch_y = 0u;
    unsigned int scsi_generation = 0u;
    unsigned int scsi_lba = 0u;
    unsigned int scsi_sector_count = 0u;
    unsigned int origin_generation = 0u;
    unsigned int origin_lba = 0u;
    unsigned int origin_offset = 0u;
    unsigned long long origin_fifo_sequence = 0u;
    unsigned int origin_destination = 0u;
    unsigned int origin_value = 0u;
    unsigned int origin_writer_physical_pc = 0u;
    unsigned int consumer_generation = 0u;
    unsigned int consumer_lba = 0u;
    unsigned int consumer_offset = 0u;
    unsigned long long consumer_fifo_sequence = 0u;
    unsigned int consumer_physical = 0u;
    unsigned int consumer_value = 0u;
    unsigned int consumer_reader_physical_pc = 0u;
    uint8_t observed_cdb[7] = {0};
    uint8_t decoded_cdb[6] = {0};
    uint32_t raw_record;
    Theron_Track02Variant variant;
    int consumed = 0;

    if (out) memset(out, 0, sizeof(*out));
    if (!capture || !track02_data || !track02_md5 || !out ||
        track02_size % THERON_TRACK02_RAW_SECTOR_BYTES != 0u ||
        strcmp(track02_md5, THERON_TRACK02_MD5_US_BIN) != 0 ||
        (variant = theron_v1_track02_variant_for_md5(track02_md5)) !=
            THERON_TRACK02_VARIANT_US_BIN) return 0;

    cursor = capture;
    while (tqr_trace_next_line(&cursor, &line, &length)) {
        ++line_number;
        if (length == strlen("source=mednafen-pce-instrumented-cd") &&
            memcmp(line, "source=mednafen-pce-instrumented-cd", length) == 0) {
            ++source_count;
            source_line = line_number;
        } else if (length >= strlen("main_ram_loader_e009_dispatch ") &&
                   memcmp(line, "main_ram_loader_e009_dispatch ",
                          strlen("main_ram_loader_e009_dispatch ")) == 0) {
            if (++dispatch_count != 1u ||
                sscanf(line,
                       "main_ram_loader_e009_dispatch sequence=%u logical_pc=%x physical_pc=%x a=%x x=%x y=%x%n",
                       &dispatch_sequence, &dispatch_logical_pc,
                       &dispatch_physical_pc, &dispatch_a, &dispatch_x,
                       &dispatch_y, &consumed) != 6 ||
                consumed != (int)length) return 0;
            dispatch_line = line_number;
        } else if (length >= strlen("pce_cd_register_write ") &&
                   memcmp(line, "pce_cd_register_write ",
                          strlen("pce_cd_register_write ")) == 0) {
            unsigned int cpu_pc = 0u;
            unsigned int physical = 0u;
            unsigned int data = 0u;
            if (sscanf(line,
                       "pce_cd_register_write cpu_pc=%x physical=%x data=%x%n",
                       &cpu_pc, &physical, &data, &consumed) != 3 ||
                consumed != (int)length || physical != 0x1801u ||
                data > 0xffu) return 0;
            if (dispatch_count == 1u && !scsi_count) {
                if (!first_cdb_line) first_cdb_line = line_number;
                if (cdb_count >= sizeof(observed_cdb) ||
                    (cdb_count == 0u && cpu_pc != 0xe90du) ||
                    (cdb_count > 0u && cpu_pc != 0xe981u)) return 0;
                observed_cdb[cdb_count] = (uint8_t)data;
                ++cdb_count;
            }
        } else if (length >= strlen("scsi_read_command ") &&
                   memcmp(line, "scsi_read_command ",
                          strlen("scsi_read_command ")) == 0) {
            if (++scsi_count != 1u || !tqr_trace_parse_scsi_read6(
                    line, length, &scsi_generation, &scsi_lba,
                    &scsi_sector_count, decoded_cdb)) return 0;
            scsi_line = line_number;
        } else if (length >= strlen("pce_cd_fifo_origin_main_ram_receipt ") &&
                   memcmp(line, "pce_cd_fifo_origin_main_ram_receipt ",
                          strlen("pce_cd_fifo_origin_main_ram_receipt ")) == 0) {
            if (++origin_count != 1u || !tqr_trace_parse_fifo_origin_main_ram(
                    line, length, &origin_generation, &origin_lba,
                    &origin_offset, &origin_fifo_sequence, &origin_destination,
                    &origin_value, &origin_writer_physical_pc)) return 0;
            origin_line = line_number;
        } else if (length >= strlen("pce_cd_fifo_origin_main_ram_consumer ") &&
                   memcmp(line, "pce_cd_fifo_origin_main_ram_consumer ",
                          strlen("pce_cd_fifo_origin_main_ram_consumer ")) == 0) {
            if (++consumer_count != 1u ||
                !tqr_trace_parse_fifo_origin_main_ram_consumer(
                    line, length, &consumer_generation, &consumer_lba,
                    &consumer_offset, &consumer_fifo_sequence,
                    &consumer_physical, &consumer_value,
                    &consumer_reader_physical_pc)) return 0;
            consumer_line = line_number;
        }
    }

    if (source_count != 1u || dispatch_count != 1u || cdb_count != 7u ||
        scsi_count != 1u || origin_count != 1u || consumer_count != 1u ||
        !(source_line < dispatch_line && dispatch_line < first_cdb_line &&
          first_cdb_line < scsi_line && scsi_line < origin_line &&
          origin_line < consumer_line) || dispatch_logical_pc != 0x3840u ||
        dispatch_physical_pc != 0x1f1840u || dispatch_a != 0x20u ||
        dispatch_x > 0xffu || dispatch_y > 0xffu ||
        observed_cdb[0] != 0x81u ||
        memcmp(observed_cdb + 1u, decoded_cdb, sizeof(decoded_cdb)) != 0 ||
        origin_generation != scsi_generation || origin_lba != scsi_lba ||
        origin_lba < 3009u || origin_lba >= scsi_lba + scsi_sector_count ||
        consumer_generation != origin_generation || consumer_lba != origin_lba ||
        consumer_offset != origin_offset ||
        consumer_fifo_sequence != origin_fifo_sequence ||
        consumer_physical != origin_destination || consumer_value != origin_value ||
        origin_writer_physical_pc < 0x1f0000u ||
        origin_writer_physical_pc >= 0x1f8000u ||
        consumer_reader_physical_pc < 0x1f0000u ||
        consumer_reader_physical_pc >= 0x1f8000u) return 0;

    raw_record = origin_lba - 3009u;
    if ((size_t)raw_record >= track02_size / THERON_TRACK02_RAW_SECTOR_BYTES ||
        track02_data[(size_t)raw_record * THERON_TRACK02_RAW_SECTOR_BYTES +
                     origin_offset] != (uint8_t)origin_value) return 0;

    out->valid = 1;
    out->variant = variant;
    snprintf(out->track02_md5, sizeof(out->track02_md5), "%s", track02_md5);
    out->dispatch_sequence = dispatch_sequence;
    out->dispatch_logical_pc = (uint16_t)dispatch_logical_pc;
    out->dispatch_physical_pc = dispatch_physical_pc;
    out->scsi_generation = scsi_generation;
    out->scsi_lba = scsi_lba;
    out->scsi_sector_count = scsi_sector_count;
    out->raw_track02_record = raw_record;
    out->source_offset = origin_offset;
    out->fifo_sequence = origin_fifo_sequence;
    out->physical_destination = origin_destination;
    out->reader_physical_pc = consumer_reader_physical_pc;
    out->source_byte = (uint8_t)origin_value;
    out->cdb_read6_verified = 1;
    out->fifo_to_game_ram_verified = 1;
    out->game_ram_consumer_verified = 1;
    out->payload_semantics_proven = 0;
    return 1;
}

int theron_v1_raw_loader_trace_bind_game_e009_destination(
    const char *capture, const char *cd_capture,
    const uint8_t *track02_data, size_t track02_size,
    const char *track02_md5,
    Theron_V1RawLoaderTraceGameE009DestinationReceipt *out)
{
    static const uint8_t expected_parameters[8] = {
        0x01u, 0x00u, 0x00u, 0x28u, 0x00u, 0x03u, 0xffu, 0x01u
    };
    const char *cursor;
    const char *line;
    size_t length;
    size_t line_number = 0u;
    size_t source_count = 0u;
    size_t dispatch2_count = 0u;
    size_t dispatch3_count = 0u;
    size_t receipt_count = 0u;
    size_t dispatch2_line = 0u;
    size_t receipt_line = 0u;
    size_t dispatch3_line = 0u;
    size_t cd_source_count = 0u;
    size_t enter1_count = 0u;
    size_t scsi_count = 0u;
    size_t return_count = 0u;
    size_t enter2_count = 0u;
    size_t enter1_line = 0u;
    size_t scsi_line = 0u;
    size_t return_line = 0u;
    size_t enter2_line = 0u;
    unsigned int sequence = 0u;
    unsigned int logical_pc = 0u;
    unsigned int physical_pc = 0u;
    unsigned int a = 0u;
    unsigned int x = 0u;
    unsigned int y = 0u;
    unsigned int caller_pc = 0u;
    unsigned int caller_physical_pc = 0u;
    unsigned int completion_pc = 0u;
    unsigned int completion_physical_pc = 0u;
    unsigned int parameters[8] = {0u};
    unsigned int destination = 0u;
    unsigned int call_mpr = 0u;
    unsigned int completion_mpr = 0u;
    unsigned int destination_physical = 0u;
    unsigned int payload_bytes = 0u;
    unsigned int bounded = 0u;
    unsigned int span_bytes = 0u;
    unsigned int span_checksum = 0u;
    unsigned int payload_checksum = 0u;
    unsigned int scsi_generation = 0u;
    unsigned int scsi_lba = 0u;
    unsigned int scsi_sector_count = 0u;
    unsigned int enter_return_pc = 0u;
    unsigned int expected_return_pc = 0u;
    unsigned int return_logical_pc = 0u;
    unsigned int return_physical_pc = 0u;
    unsigned int return_matched = 0u;
    uint8_t cdb[6] = {0};
    uint32_t raw_record;
    size_t user_offset;
    size_t i;
    int consumed = 0;

    if (out) memset(out, 0, sizeof(*out));
    if (!capture || !cd_capture || !track02_data || !track02_md5 || !out ||
        strcmp(track02_md5, THERON_TRACK02_MD5_US_BIN) != 0 ||
        track02_size % THERON_TRACK02_RAW_SECTOR_BYTES != 0u) return 0;

    cursor = capture;
    while (tqr_trace_next_line(&cursor, &line, &length)) {
        ++line_number;
        if (length == strlen("source=mednafen-pce-instrumented") &&
            memcmp(line, "source=mednafen-pce-instrumented", length) == 0) {
            ++source_count;
        } else if (sscanf(line,
                   "game_main_ram_e009_dispatch sequence=%u logical_pc=%x physical_pc=%x target=e009 a=%x x=%x y=%x%n",
                   &sequence, &logical_pc, &physical_pc, &a, &x, &y,
                   &consumed) == 6 && consumed == (int)length &&
                   logical_pc == 0x3840u && physical_pc == 0x1f1840u) {
            if (sequence == 2u) {
                if (a != 0x20u || x != 0x03u || y != 0x02u) return 0;
                ++dispatch2_count;
                dispatch2_line = line_number;
            } else if (sequence == 3u) {
                if (a != 0x20u || x != 0x00u || y != 0x04u) return 0;
                ++dispatch3_count;
                dispatch3_line = line_number;
            }
        } else if (length >= strlen("game_main_ram_e009_destination_receipt ") &&
                   memcmp(line, "game_main_ram_e009_destination_receipt ",
                          strlen("game_main_ram_e009_destination_receipt ")) == 0) {
            consumed = 0;
            if (++receipt_count != 1u || sscanf(line,
                "game_main_ram_e009_destination_receipt caller_pc=%x caller_physical_pc=%x completion_pc=%x completion_physical_pc=%x f8=%x f9=%x fa=%x fb=%x fc=%x fd=%x fe=%x ff=%x destination=%x destination_call_mpr=%x destination_completion_mpr=%x destination_physical=%x bytes=%u bounded=%u span_bytes=%u span_fnv1a=%x payload_fnv1a=%x%n",
                &caller_pc, &caller_physical_pc, &completion_pc,
                &completion_physical_pc, &parameters[0], &parameters[1],
                &parameters[2], &parameters[3], &parameters[4], &parameters[5],
                &parameters[6], &parameters[7], &destination, &call_mpr,
                &completion_mpr, &destination_physical, &payload_bytes,
                &bounded, &span_bytes, &span_checksum, &payload_checksum,
                &consumed) != 21 || consumed != (int)length) return 0;
            receipt_line = line_number;
        }
    }

    line_number = 0u;
    cursor = cd_capture;
    while (tqr_trace_next_line(&cursor, &line, &length)) {
        ++line_number;
        if (length == strlen("source=mednafen-pce-instrumented-cd-state") &&
            memcmp(line, "source=mednafen-pce-instrumented-cd-state",
                   length) == 0) {
            ++cd_source_count;
        } else if (sscanf(line,
                   "main_ram_e009_enter sequence=%u logical_pc=%x physical_pc=%x return_pc=%x a=%x x=%x y=%x%n",
                   &sequence, &logical_pc, &physical_pc, &enter_return_pc,
                   &a, &x, &y, &consumed) == 7 && consumed == (int)length &&
                   logical_pc == 0x3840u && physical_pc == 0x1f1840u) {
            if (sequence == 1u) {
                if (a != 0x20u || x != 0x03u || y != 0x02u ||
                    enter_return_pc != 0x3843u) return 0;
                ++enter1_count;
                enter1_line = line_number;
            } else if (sequence == 2u) {
                if (a != 0x20u || x != 0x00u || y != 0x04u ||
                    enter_return_pc != 0x3843u) return 0;
                ++enter2_count;
                enter2_line = line_number;
            }
        } else if (length >= strlen("scsi_read_command ") &&
                   memcmp(line, "scsi_read_command ",
                          strlen("scsi_read_command ")) == 0) {
            unsigned int parsed_generation = 0u;
            unsigned int parsed_lba = 0u;
            unsigned int parsed_count = 0u;
            uint8_t parsed_cdb[6];
            if (tqr_trace_parse_scsi_read6(line, length, &parsed_generation,
                    &parsed_lba, &parsed_count, parsed_cdb) &&
                parsed_generation == 5u) {
                if (++scsi_count != 1u) return 0;
                scsi_generation = parsed_generation;
                scsi_lba = parsed_lba;
                scsi_sector_count = parsed_count;
                memcpy(cdb, parsed_cdb, sizeof(cdb));
                scsi_line = line_number;
            }
        } else if (sscanf(line,
                   "main_ram_e009_return sequence=%u expected_return_pc=%x logical_pc=%x physical_pc=%x matched=%u%n",
                   &sequence, &expected_return_pc, &return_logical_pc,
                   &return_physical_pc, &return_matched, &consumed) == 5 &&
                   consumed == (int)length && sequence == 1u) {
            ++return_count;
            return_line = line_number;
        }
    }

    if (source_count != 1u || dispatch2_count != 1u || dispatch3_count != 1u ||
        receipt_count != 1u || !(dispatch2_line < receipt_line &&
        receipt_line < dispatch3_line) || cd_source_count != 1u ||
        enter1_count != 1u || scsi_count != 1u || return_count != 1u ||
        enter2_count != 1u || !(enter1_line < scsi_line &&
        scsi_line < return_line && return_line < enter2_line) ||
        caller_pc != 0x3840u || caller_physical_pc != 0x1f1840u ||
        completion_pc != 0x3840u || completion_physical_pc != 0x1f1840u ||
        destination != 0x2800u || call_mpr != 0xf8u ||
        completion_mpr != 0xf8u || destination_physical != 0x1f0800u ||
        payload_bytes != THERON_TRACK02_RAW_USER_DATA_BYTES || bounded != 1u ||
        span_bytes != 32u || expected_return_pc != 0x3843u ||
        return_logical_pc != 0x3b36u || return_physical_pc != 0x1f1b36u ||
        return_matched != 0u || scsi_sector_count != 1u ||
        scsi_lba < TQR_TRACE_TRACK02_LBA_BASE) return 0;
    for (i = 0u; i < sizeof(expected_parameters); ++i) {
        if (parameters[i] > 0xffu ||
            parameters[i] != expected_parameters[i]) return 0;
    }
    raw_record = scsi_lba - TQR_TRACE_TRACK02_LBA_BASE;
    if (raw_record != THERON_TRACK02_IPL_STAGE2_CD_READ_RECORD_US ||
        (size_t)raw_record >= track02_size / THERON_TRACK02_RAW_SECTOR_BYTES ||
        cdb[0] != 0x08u) return 0;
    user_offset = (size_t)raw_record * THERON_TRACK02_RAW_SECTOR_BYTES +
        THERON_TRACK02_RAW_USER_DATA_OFFSET;
    if (span_checksum != tqr_trace_fnv1a_bytes(
            track02_data + user_offset, span_bytes) ||
        payload_checksum != tqr_trace_fnv1a_bytes(
            track02_data + user_offset, payload_bytes)) return 0;

    out->valid = 1;
    out->variant = THERON_TRACK02_VARIANT_US_BIN;
    snprintf(out->track02_md5, sizeof(out->track02_md5), "%s", track02_md5);
    out->caller_pc = (uint16_t)caller_pc;
    out->caller_physical_pc = caller_physical_pc;
    out->completion_pc = (uint16_t)completion_pc;
    out->completion_physical_pc = completion_physical_pc;
    for (i = 0u; i < sizeof(out->parameters); ++i)
        out->parameters[i] = (uint8_t)parameters[i];
    out->destination = (uint16_t)destination;
    out->destination_physical = destination_physical;
    out->payload_bytes = payload_bytes;
    out->payload_span_checksum = span_checksum;
    out->payload_checksum = payload_checksum;
    out->scsi_generation = scsi_generation;
    out->scsi_lba = scsi_lba;
    out->scsi_sector_count = scsi_sector_count;
    out->raw_track02_record = raw_record;
    out->asynchronous_resume_observed = 1;
    out->next_dispatch_completion_observed = 1;
    out->mode1_payload_verified = 1;
    out->payload_semantics_proven = 0;
    return 1;
}

int theron_v1_raw_loader_trace_bind_game_e009_consumer(
    const Theron_V1RawLoaderTraceGameE009DestinationReceipt *destination,
    const char *consumer_capture, const uint8_t *track02_data,
    size_t track02_size, const char *track02_md5,
    Theron_V1RawLoaderTraceGameE009ConsumerReceipt *out)
{
    static const unsigned int expected_logical[5] = {
        0x2d13u, 0x2d14u, 0x2d15u, 0x2d16u, 0x2d17u
    };
    static const unsigned int expected_reader_pc[5] = {
        0x37e2u, 0x37e9u, 0x37f7u, 0x37fcu, 0x3802u
    };
    static const uint8_t expected_value[5] = {
        0xf9u, 0x02u, 0x04u, 0x00u, 0x20u
    };
    const char *cursor;
    const char *line;
    size_t length;
    size_t source_count = 0u;
    size_t enter1_count = 0u;
    size_t resume1_count = 0u;
    size_t enter2_count = 0u;
    size_t read_count = 0u;
    size_t code_count = 0u;
    size_t phase = 0u;
    size_t user_offset;
    size_t i;
    unsigned int sequence = 0u;
    unsigned int logical_pc = 0u;
    unsigned int physical_pc = 0u;
    unsigned int expected_pc = 0u;
    unsigned int a = 0u, x = 0u, y = 0u;
    unsigned int matched = 0u;
    unsigned int logical_address = 0u, physical_address = 0u, value = 0u;
    unsigned int reader_pc = 0u, reader_physical_pc = 0u;
    unsigned int sp = 0u, p = 0u;
    unsigned int system_card_copy = 0u, debugger_completion_hash = 0u;
    unsigned int provenance_valid = 0u, provenance_value_match = 0u;
    unsigned int source_lba = 0u, source_offset = 0u;
    unsigned long long fifo_sequence = 0u;
    int consumed = 0;

    if (out) memset(out, 0, sizeof(*out));
    if (!destination || !destination->valid || !consumer_capture ||
        !track02_data || !track02_md5 || !out ||
        destination->variant != THERON_TRACK02_VARIANT_US_BIN ||
        strcmp(destination->track02_md5, THERON_TRACK02_MD5_US_BIN) != 0 ||
        strcmp(track02_md5, THERON_TRACK02_MD5_US_BIN) != 0 ||
        destination->raw_track02_record !=
            THERON_TRACK02_IPL_STAGE2_CD_READ_RECORD_US ||
        destination->destination != 0x2800u ||
        destination->destination_physical != 0x1f0800u ||
        destination->payload_bytes != THERON_TRACK02_RAW_USER_DATA_BYTES ||
        destination->payload_checksum != 0x33a90342u ||
        !destination->asynchronous_resume_observed ||
        !destination->next_dispatch_completion_observed ||
        !destination->mode1_payload_verified ||
        destination->payload_semantics_proven ||
        track02_size % THERON_TRACK02_RAW_SECTOR_BYTES != 0u ||
        (size_t)destination->raw_track02_record >=
            track02_size / THERON_TRACK02_RAW_SECTOR_BYTES) return 0;

    user_offset = (size_t)destination->raw_track02_record *
        THERON_TRACK02_RAW_SECTOR_BYTES + THERON_TRACK02_RAW_USER_DATA_OFFSET;
    if (tqr_trace_fnv1a_bytes(track02_data + user_offset,
            THERON_TRACK02_RAW_USER_DATA_BYTES) !=
        destination->payload_checksum) return 0;

    cursor = consumer_capture;
    while (tqr_trace_next_line(&cursor, &line, &length)) {
        consumed = 0;
        if (length == strlen("source=mednafen-pce-instrumented-e009-destination-consumer") &&
            memcmp(line, "source=mednafen-pce-instrumented-e009-destination-consumer",
                   length) == 0) {
            if (++source_count != 1u || phase != 0u) return 0;
        } else if (sscanf(line,
                   "e009_destination_boundary kind=enter sequence=%u logical_pc=%x physical_pc=%x a=%x x=%x y=%x%n",
                   &sequence, &logical_pc, &physical_pc, &a, &x, &y,
                   &consumed) == 6 && consumed == (int)length) {
            if (sequence == 1u) {
                if (++enter1_count != 1u || phase != 0u ||
                    logical_pc != 0x3840u || physical_pc != 0x1f1840u ||
                    a != 0x20u || x != 0x03u || y != 0x02u) return 0;
                phase = 1u;
            } else if (sequence == 2u) {
                if (++enter2_count != 1u || phase != 2u || read_count != 5u ||
                    logical_pc != 0x3840u || physical_pc != 0x1f1840u ||
                    a != 0x20u || x != 0x00u || y != 0x04u) return 0;
                phase = 3u;
            }
        } else if (sscanf(line,
                   "e009_destination_boundary kind=resume sequence=%u expected_pc=%x logical_pc=%x physical_pc=%x matched=%u%n",
                   &sequence, &expected_pc, &logical_pc, &physical_pc,
                   &matched, &consumed) == 5 && consumed == (int)length &&
                   sequence == 1u) {
            if (++resume1_count != 1u || phase != 1u ||
                expected_pc != 0x3843u || logical_pc != 0x3b36u ||
                physical_pc != 0x1f1b36u || matched != 0u) return 0;
            phase = 2u;
        } else if (sscanf(line,
                   "e009_consumer_code_byte logical_address=%x physical_address=%x value=%x boundary_sequence=%u%n",
                   &logical_address, &physical_address, &value, &sequence,
                   &consumed) == 4 && consumed == (int)length && phase == 2u) {
            if (code_count >= sizeof(out->code_bytes) || sequence != 1u ||
                logical_address != 0x37c8u + code_count ||
                physical_address != 0x1f17c8u + code_count || value > 0xffu)
                return 0;
            out->code_bytes[code_count++] = (uint8_t)value;
        } else if (sscanf(line,
                   "e009_destination_read sequence=%u logical_address=%x physical_address=%x value=%x reader_pc=%x reader_physical_pc=%x a=%x x=%x y=%x sp=%x p=%x system_card_copy=%u debugger_completion_hash=%u provenance_valid=%u provenance_value_match=%u source_lba=%u source_offset=%u fifo_sequence=%llu%n",
                   &sequence, &logical_address, &physical_address, &value,
                   &reader_pc, &reader_physical_pc, &a, &x, &y, &sp, &p,
                   &system_card_copy, &debugger_completion_hash,
                   &provenance_valid, &provenance_value_match, &source_lba,
                   &source_offset, &fifo_sequence, &consumed) == 18 &&
                   consumed == (int)length && phase == 2u) {
            if (code_count != sizeof(out->code_bytes) || read_count >= 5u ||
                system_card_copy != 0u ||
                debugger_completion_hash != 0u ||
                logical_address != expected_logical[read_count] ||
                physical_address != 0x1f0000u + expected_logical[read_count] -
                    0x2000u || value != expected_value[read_count] ||
                reader_pc != expected_reader_pc[read_count] ||
                reader_physical_pc != 0x1f0000u +
                    expected_reader_pc[read_count] - 0x2000u) return 0;
            ++read_count;
        } else if (phase == 2u &&
                   length >= strlen("e009_destination_read ") &&
                   memcmp(line, "e009_destination_read ",
                          strlen("e009_destination_read ")) == 0) {
            return 0;
        }
    }

    if (source_count != 1u || enter1_count != 1u || resume1_count != 1u ||
        enter2_count != 1u || read_count != 5u ||
        code_count != sizeof(out->code_bytes) || phase != 3u) return 0;

    /* The captured main-RAM range crosses a raw-sector boundary.  Compare it
     * as two MODE1 user-data fragments; raw sync/header/ECC bytes are never
     * mistaken for loaded program bytes. */
    if ((size_t)0x4c5u >= track02_size / THERON_TRACK02_RAW_SECTOR_BYTES ||
        memcmp(out->code_bytes,
            track02_data + (size_t)0x4c4u * THERON_TRACK02_RAW_SECTOR_BYTES +
                THERON_TRACK02_RAW_USER_DATA_OFFSET + 0x7c8u,
            56u) != 0 ||
        memcmp(out->code_bytes + 56u,
            track02_data + (size_t)0x4c5u * THERON_TRACK02_RAW_SECTOR_BYTES +
                THERON_TRACK02_RAW_USER_DATA_OFFSET,
            64u) != 0) return 0;

    /* Source-bound HuC6280 opcodes at $3806 load base $2803, multiply an
     * opaque byte by six with a three-step shift/add loop, and use the result
     * as the ($00),Y source.  The first observed address closes the relation;
     * the byte is not assigned a gameplay meaning. */
    if (out->code_bytes[0x3eu] != 0xadu ||
        out->code_bytes[0x3fu] != 0xd6u ||
        out->code_bytes[0x40u] != 0x37u ||
        out->code_bytes[0x41u] != 0x85u ||
        out->code_bytes[0x42u] != 0x00u ||
        out->code_bytes[0x43u] != 0xadu ||
        out->code_bytes[0x44u] != 0xd7u ||
        out->code_bytes[0x45u] != 0x37u ||
        out->code_bytes[0x46u] != 0x85u ||
        out->code_bytes[0x47u] != 0x01u ||
        out->code_bytes[0x4fu] != 0xa9u ||
        out->code_bytes[0x50u] != 0x06u ||
        out->code_bytes[0x53u] != 0xa2u ||
        out->code_bytes[0x54u] != 0x03u ||
        (uint16_t)(0x2803u + 6u * 0xd8u) != expected_logical[0]) return 0;
    for (i = 0u; i < 5u; ++i) {
        size_t media_offset = 0x513u + i;
        if (track02_data[user_offset + media_offset] != expected_value[i])
            return 0;
        out->logical_addresses[i] = (uint16_t)expected_logical[i];
        out->physical_addresses[i] = 0x1f0000u + expected_logical[i] - 0x2000u;
        out->reader_pcs[i] = (uint16_t)expected_reader_pc[i];
        out->reader_physical_pcs[i] = 0x1f0000u + expected_reader_pc[i] - 0x2000u;
        out->media_offsets[i] = media_offset;
        out->values[i] = expected_value[i];
    }
    out->valid = 1;
    out->variant = THERON_TRACK02_VARIANT_US_BIN;
    snprintf(out->track02_md5, sizeof(out->track02_md5), "%s", track02_md5);
    out->raw_track02_record = destination->raw_track02_record;
    out->code_checksum = tqr_trace_fnv1a_bytes(out->code_bytes,
        sizeof(out->code_bytes));
    out->code_first_record = 0x4c4u;
    out->code_first_user_offset = 0x7c8u;
    out->code_second_record = 0x4c5u;
    out->code_second_user_offset = 0u;
    out->pointer_base = 0x2803u;
    out->pointer_stride = 6u;
    out->resolved_index = 0xd8u;
    out->resolved_pointer = 0x2d13u;
    out->code_media_verified = 1;
    out->address_construction_verified = 1;
    out->source_bytes_verified = 1;
    out->read_order_verified = 1;
    out->next_dispatch_observed = 1;
    out->field_semantics_proven = 0;
    return 1;
}

int theron_v1_raw_loader_trace_bind_game_e009_next_parameters(
    const Theron_V1RawLoaderTraceGameE009ConsumerReceipt *consumer,
    const char *main_ram_loader_capture,
    Theron_V1RawLoaderTraceGameE009NextParametersReceipt *out)
{
    static const unsigned int offsets[] = {
        0u, 1u, 2u, 3u, 4u, 5u, 6u, 7u, 8u, 9u, 10u, 12u,
        26u, 27u, 28u, 29u, 30u, 31u, 32u, 33u
    };
    static const unsigned int addresses[] = {
        0x2025u, 0x2024u, 0x2023u, 0x2022u, 0x201eu, 0x37d0u,
        0x37d1u, 0x2025u, 0x2020u, 0x2021u, 0x201eu, 0x201fu,
        0x20f8u, 0x20f9u, 0x20fau, 0x20fbu, 0x20fcu, 0x20fdu,
        0x20feu, 0x20ffu
    };
    static const unsigned int values[] = {
        0x01u, 0xf8u, 0x06u, 0x00u, 0x04u, 0x00u, 0x20u, 0xfeu,
        0x00u, 0x10u, 0x00u, 0x20u, 0x00u, 0x20u, 0x00u, 0x10u,
        0x00u, 0x06u, 0xf8u, 0xfeu
    };
    static const unsigned int writer_pcs[] = {
        0x37dfu, 0x37e7u, 0x37eeu, 0x37f4u, 0x37f9u, 0x37ffu,
        0x3805u, 0x36d9u, 0x36deu, 0x36e3u, 0x36e8u, 0x36f0u,
        0x383du, 0x383du, 0x383du, 0x383du, 0x383du, 0x383du,
        0x383du, 0x383du
    };
    const char *cursor;
    const char *line;
    size_t length;
    size_t source_count = 0u, tii_count = 0u, matched_count = 0u;
    unsigned int base_sequence = 0u;
    unsigned int sequence, address, physical, value, writer_pc, writer_physical;
    unsigned int logical_pc, physical_pc, source, destination, transfer_length;
    int consumed;

    if (out) memset(out, 0, sizeof(*out));
    if (!consumer || !consumer->valid || !main_ram_loader_capture || !out ||
        consumer->variant != THERON_TRACK02_VARIANT_US_BIN ||
        strcmp(consumer->track02_md5, THERON_TRACK02_MD5_US_BIN) != 0 ||
        !consumer->code_media_verified ||
        !consumer->address_construction_verified ||
        !consumer->source_bytes_verified || !consumer->read_order_verified ||
        consumer->field_semantics_proven) return 0;

    cursor = main_ram_loader_capture;
    while (tqr_trace_next_line(&cursor, &line, &length)) {
        consumed = 0;
        if (length == strlen("source=mednafen-pce-instrumented-main-ram-loader") &&
            memcmp(line, "source=mednafen-pce-instrumented-main-ram-loader",
                   length) == 0) {
            if (++source_count != 1u) return 0;
        } else if (sscanf(line,
                   "main_ram_loader_block_transfer logical_pc=%x physical_pc=%x operation=tii source=%x destination=%x length=%x%n",
                   &logical_pc, &physical_pc, &source, &destination,
                   &transfer_length, &consumed) == 5 &&
                   consumed == (int)length && logical_pc == 0x3836u) {
            if (++tii_count != 1u || physical_pc != 0x1f1836u ||
                source != 0x201eu || destination != 0x20f8u ||
                transfer_length != 8u) return 0;
        } else if (sscanf(line,
                   "main_ram_loader_write sequence=%u dispatch_sequence=unbound logical_destination=%x physical_destination=%x value=%x writer_pc=%x writer_physical_pc=%x%n",
                   &sequence, &address, &physical, &value, &writer_pc,
                   &writer_physical, &consumed) == 6 &&
                   consumed == (int)length) {
            if (matched_count == 0u && address == addresses[0] &&
                value == values[0] && writer_pc == writer_pcs[0])
                base_sequence = sequence;
            if (matched_count < sizeof(offsets) / sizeof(offsets[0]) &&
                base_sequence != 0u &&
                sequence == base_sequence + offsets[matched_count]) {
                if (address != addresses[matched_count] ||
                    physical != 0x1f0000u + address - 0x2000u ||
                    value != values[matched_count] ||
                    writer_pc != writer_pcs[matched_count] ||
                    writer_physical != 0x1f0000u + writer_pc - 0x2000u)
                    return 0;
                ++matched_count;
            }
        }
    }
    if (source_count != 1u || tii_count != 1u ||
        matched_count != sizeof(offsets) / sizeof(offsets[0])) return 0;

    out->valid = 1;
    out->variant = consumer->variant;
    snprintf(out->track02_md5, sizeof(out->track02_md5), "%s",
        consumer->track02_md5);
    {
        size_t i;
        for (i = 0u; i < sizeof(out->consumer_outputs); ++i)
            out->consumer_outputs[i] = (uint8_t)values[i];
        for (i = 0u; i < sizeof(out->next_parameters); ++i)
            out->next_parameters[i] = (uint8_t)values[12u + i];
    }
    out->first_write_sequence = base_sequence;
    out->tii_pc = 0x3836u;
    out->tii_source = 0x201eu;
    out->tii_destination = 0x20f8u;
    out->tii_length = 8u;
    out->consumer_output_writes_verified = 1;
    out->tii_verified = 1;
    out->next_parameters_verified = 1;
    out->parameter_semantics_proven = 0;
    return 1;
}

int theron_v1_raw_loader_trace_bind_game_e009_vdc_payload(
    const Theron_V1RawLoaderTraceGameE009NextParametersReceipt *parameters,
    const char *cd_capture, const char *vdc_capture,
    const uint8_t *track02_data, size_t track02_size,
    const uint8_t *vram_snapshot, size_t vram_snapshot_size,
    const char *track02_md5,
    Theron_V1RawLoaderTraceGameE009VdcReceipt *out)
{
    static const uint8_t expected_parameters[8] = {
        0x00u, 0x20u, 0x00u, 0x10u, 0x00u, 0x06u, 0xf8u, 0xfeu
    };
    static const unsigned int setup_address[4] = {0u, 2u, 3u, 0u};
    static const unsigned int setup_value[4] = {0u, 0u, 0x10u, 2u};
    static const unsigned int setup_pc[4] = {0xecd4u, 0xf341u, 0xf346u, 0xecdeu};
    const char *cursor, *line;
    size_t length, cd_source_count = 0u, command_count = 0u;
    size_t vdc_source_count = 0u, generation_rows = 0u, unique_rows = 0u;
    size_t boundary_count = 0u;
    size_t payload_index = 0u, i;
    unsigned int generation = 0u, lba = 0u, sector_count = 0u;
    unsigned int sequence, row_generation, timestamp, address, physical;
    unsigned int value, writer_pc, writer_physical;
    unsigned int boundary_generation, boundary_rows, boundary_sequence;
    unsigned int boundary_timestamp, boundary_first, boundary_last;
    unsigned int boundary_words, boundary_hash;
    unsigned int previous_sequence = 0u;
    uint8_t cdb[6] = {0}, parsed_cdb[6] = {0};
    uint32_t first_record;
    uint32_t hash = 2166136261u;
    int consumed = 0;

    if (out) memset(out, 0, sizeof(*out));
    if (!parameters || !parameters->valid || !cd_capture || !vdc_capture ||
        !track02_data || !vram_snapshot || !track02_md5 || !out ||
        parameters->variant != THERON_TRACK02_VARIANT_US_BIN ||
        strcmp(parameters->track02_md5, THERON_TRACK02_MD5_US_BIN) != 0 ||
        strcmp(track02_md5, THERON_TRACK02_MD5_US_BIN) != 0 ||
        memcmp(parameters->next_parameters, expected_parameters,
            sizeof(expected_parameters)) != 0 ||
        !parameters->consumer_output_writes_verified ||
        !parameters->tii_verified || !parameters->next_parameters_verified ||
        parameters->parameter_semantics_proven ||
        track02_size % THERON_TRACK02_RAW_SECTOR_BYTES != 0u ||
        vram_snapshot_size != 65536u) return 0;

    cursor = cd_capture;
    while (tqr_trace_next_line(&cursor, &line, &length)) {
        if (length == strlen("source=mednafen-pce-instrumented-cd-state") &&
            memcmp(line, "source=mednafen-pce-instrumented-cd-state",
                length) == 0) {
            ++cd_source_count;
        } else if (length >= strlen("scsi_read_command ") &&
                   memcmp(line, "scsi_read_command ",
                       strlen("scsi_read_command ")) == 0) {
            unsigned int parsed_generation = 0u, parsed_lba = 0u;
            unsigned int parsed_sector_count = 0u;
            if (tqr_trace_parse_scsi_read6(line, length, &parsed_generation,
                    &parsed_lba, &parsed_sector_count, parsed_cdb) &&
                parsed_generation == 6u) {
                if (++command_count != 1u) return 0;
                generation = parsed_generation;
                lba = parsed_lba;
                sector_count = parsed_sector_count;
                memcpy(cdb, parsed_cdb, sizeof(cdb));
            }
        }
    }
    if (cd_source_count != 1u || command_count != 1u || lba != 5018u ||
        sector_count != 4u || cdb[0] != 0x08u ||
        lba < TQR_TRACE_TRACK02_LBA_BASE) return 0;
    first_record = lba - TQR_TRACE_TRACK02_LBA_BASE;
    if (first_record != 0x7d9u ||
        (size_t)first_record + sector_count >
            track02_size / THERON_TRACK02_RAW_SECTOR_BYTES) return 0;

    cursor = vdc_capture;
    while (tqr_trace_next_line(&cursor, &line, &length)) {
        consumed = 0;
        if (length == strlen("source=mednafen-pce-instrumented-scsi-generation-vdc") &&
            memcmp(line, "source=mednafen-pce-instrumented-scsi-generation-vdc",
                length) == 0) {
            ++vdc_source_count;
        } else if (sscanf(line,
                   "scsi_generation_vdc_write sequence=%u scsi_generation=%u timestamp=%u logical_address=%x physical_address=%x value=%x writer_pc=%x writer_physical_pc=%x%n",
                   &sequence, &row_generation, &timestamp, &address, &physical,
                   &value, &writer_pc, &writer_physical, &consumed) == 8 &&
                   consumed == (int)length && row_generation == 6u) {
            ++generation_rows;
            if (value > 0xffu || physical != 0x1fe000u + address ||
                writer_pc < 0xe000u || writer_physical != writer_pc - 0xe000u ||
                (generation_rows > 1u && sequence != previous_sequence + 1u))
                return 0;
            previous_sequence = sequence;
            if (generation_rows <= 4u) {
                size_t setup_index = generation_rows - 1u;
                if (address != setup_address[setup_index] ||
                    value != setup_value[setup_index] ||
                    writer_pc != setup_pc[setup_index]) return 0;
            } else {
                size_t record = first_record + payload_index /
                    THERON_TRACK02_RAW_USER_DATA_BYTES;
                size_t offset = payload_index %
                    THERON_TRACK02_RAW_USER_DATA_BYTES;
                uint8_t media_byte;
                if (payload_index >= 4u * THERON_TRACK02_RAW_USER_DATA_BYTES ||
                    address != 2u + (payload_index & 1u) ||
                    writer_pc != 0xeb35u) return 0;
                media_byte = track02_data[record *
                    THERON_TRACK02_RAW_SECTOR_BYTES +
                    THERON_TRACK02_RAW_USER_DATA_OFFSET + offset];
                if (value != media_byte) return 0;
                hash ^= media_byte;
                hash *= 16777619u;
                ++payload_index;
            }
            ++unique_rows;
        } else if (sscanf(line,
                   "scsi_generation_vdc_snapshot_boundary scsi_generation=%u generation_rows=%u sequence=%u timestamp=%u first_vram_word=%x last_vram_word=%x vram_words=%u vram_fnv1a=%x%n",
                   &boundary_generation, &boundary_rows, &boundary_sequence,
                   &boundary_timestamp, &boundary_first, &boundary_last,
                   &boundary_words, &boundary_hash, &consumed) == 8 &&
                   consumed == (int)length) {
            if (++boundary_count != 1u || boundary_generation != 6u ||
                boundary_rows != 8196u || boundary_sequence != 8198u ||
                boundary_first != 0x1000u || boundary_last != 0x2fffu ||
                boundary_words != 8192u || boundary_hash != 0xa0e05797u)
                return 0;
        }
    }
    if (vdc_source_count != 1u || boundary_count != 1u ||
        generation_rows != 8196u ||
        unique_rows != 8196u || payload_index != 8192u ||
        hash != 0x4859675du) return 0;
    for (i = 0u; i < 8192u; ++i) {
        size_t record = first_record + i /
            THERON_TRACK02_RAW_USER_DATA_BYTES;
        size_t offset = i % THERON_TRACK02_RAW_USER_DATA_BYTES;
        uint8_t media_byte = track02_data[record *
            THERON_TRACK02_RAW_SECTOR_BYTES +
            THERON_TRACK02_RAW_USER_DATA_OFFSET + offset];
        if (vram_snapshot[0x2000u + i] != media_byte) return 0;
    }
    if (tqr_trace_fnv1a_bytes(vram_snapshot + 0x2000u, 0x2000u) !=
            0x4859675du ||
        tqr_trace_fnv1a_bytes(vram_snapshot + 0x2000u, 0x4000u) !=
            0xa0e05797u ||
        tqr_trace_fnv1a_bytes(vram_snapshot, vram_snapshot_size) !=
            0xedfc7797u) return 0;

    out->valid = 1;
    out->variant = THERON_TRACK02_VARIANT_US_BIN;
    snprintf(out->track02_md5, sizeof(out->track02_md5), "%s", track02_md5);
    out->scsi_generation = generation;
    out->scsi_lba = lba;
    out->scsi_sector_count = sector_count;
    out->first_raw_track02_record = first_record;
    out->payload_bytes = payload_index;
    out->payload_checksum = hash;
    out->vdc_write_pc = 0xeb35u;
    out->vdc_write_physical_pc = 0x000b35u;
    out->vdc_payload_writes = generation_rows - 4u;
    out->first_vram_word = 0x1000u;
    out->last_vram_word = 0x1fffu;
    out->vram_word_count = 4096u;
    out->vram_snapshot_checksum = 0x4859675du;
    out->full_vram_snapshot_checksum =
        tqr_trace_fnv1a_bytes(vram_snapshot, vram_snapshot_size);
    out->read6_verified = 1;
    out->vdc_setup_verified = 1;
    out->repeated_word_writes_verified = 0;
    out->single_word_writes_verified = 1;
    out->vram_destination_verified = 1;
    out->vram_snapshot_verified = 1;
    out->mode1_payload_verified = 1;
    out->payload_semantics_proven = 0;
    return 1;
}

int theron_v1_raw_loader_trace_bind_game_e009_vdc_presentation(
    const Theron_V1RawLoaderTraceGameE009VdcReceipt *payload,
    const char *vdc_capture, const char *vdc_state_capture,
    const uint8_t *pre_vram_snapshot, size_t pre_vram_snapshot_size,
    const uint8_t *post_vram_snapshot, size_t post_vram_snapshot_size,
    const uint8_t *vce_snapshot, size_t vce_snapshot_size,
    const uint8_t *track02_data, size_t track02_size,
    const char *track02_md5,
    Theron_V1RawLoaderTraceGameE009VdcPresentationReceipt *out)
{
    const char *cursor, *line;
    size_t length, source_count = 0u, row_count = 0u, boundary_count = 0u;
    size_t state_source_count = 0u, state_count = 0u, commits = 0u, i;
    size_t code_count = 0u;
    unsigned int sequence, generation, timestamp, logical, physical, value;
    unsigned int writer_pc, writer_physical, previous_sequence = 0u;
    unsigned int boundary_generation, boundary_rows, boundary_sequence;
    unsigned int boundary_timestamp, boundary_first, boundary_last;
    unsigned int boundary_words, boundary_hash;
    unsigned int vdc, bxr, byr, mwr, hsr, hdr, vsr, vdr, vcr, cr;
    unsigned int code_generation, code_physical, code_value;
    uint16_t bat[2048];
    uint8_t selected_register = 0u, vwr_low = 0u;
    uint16_t mawr = 0u;
    unsigned int have_vwr_low = 0u;
    uint8_t seen_tiles[0x200] = {0u};
    uint8_t code[512];
    size_t source_cells = 0u, unique_tiles = 0u;
    size_t background_pixels = 0u, nonzero_pixels = 0u;
    uint32_t background_hash = 2166136261u;
    uint32_t color_hash = 2166136261u;
    uint16_t first_tile = 0xffffu, last_tile = 0u;
    Theron_Track02Stage2Enclosing45xxCalleesReceipt stage2_callees;
    int consumed = 0;

    if (out) memset(out, 0, sizeof(*out));
    if (!payload || !payload->valid || !vdc_capture || !vdc_state_capture ||
        !pre_vram_snapshot || !post_vram_snapshot || !vce_snapshot ||
        !track02_data || !track02_md5 || !out ||
        payload->variant != THERON_TRACK02_VARIANT_US_BIN ||
        strcmp(payload->track02_md5, THERON_TRACK02_MD5_US_BIN) != 0 ||
        !payload->read6_verified || !payload->vdc_setup_verified ||
        payload->repeated_word_writes_verified ||
        !payload->single_word_writes_verified ||
        !payload->vram_destination_verified || !payload->vram_snapshot_verified ||
        payload->payload_semantics_proven || pre_vram_snapshot_size != 65536u ||
        post_vram_snapshot_size != 65536u || vce_snapshot_size != 1024u ||
        strcmp(track02_md5, THERON_TRACK02_MD5_US_BIN) != 0 ||
        track02_size % THERON_TRACK02_RAW_SECTOR_BYTES != 0u ||
        tqr_trace_fnv1a_bytes(pre_vram_snapshot, pre_vram_snapshot_size) !=
            0xedfc7797u || payload->full_vram_snapshot_checksum != 0xedfc7797u)
        return 0;
    if (theron_v1_track02_verify_stage2_enclosing_45xx_callees(
            track02_data, track02_size, track02_md5, &stage2_callees) !=
            THERON_TRACK02_SIGNAL_OK || !stage2_callees.valid ||
        !stage2_callees.l466b_proven) return 0;
    for (i = 0u; i < 2048u; ++i)
        bat[i] = (uint16_t)(pre_vram_snapshot[i * 2u] |
            ((uint16_t)pre_vram_snapshot[i * 2u + 1u] << 8));

    cursor = vdc_capture;
    while (tqr_trace_next_line(&cursor, &line, &length)) {
        consumed = 0;
        if (length == strlen("source=mednafen-pce-instrumented-scsi-generation-vdc") &&
            memcmp(line, "source=mednafen-pce-instrumented-scsi-generation-vdc",
                length) == 0) {
            ++source_count;
        } else if (sscanf(line,
                   "scsi_generation_vdc_write sequence=%u scsi_generation=%u timestamp=%u logical_address=%x physical_address=%x value=%x writer_pc=%x writer_physical_pc=%x%n",
                   &sequence, &generation, &timestamp, &logical, &physical,
                   &value, &writer_pc, &writer_physical, &consumed) == 8 &&
                   consumed == (int)length && generation == 7u) {
            uint32_t normalized = physical & 0x7fffffffu;
            unsigned int port;
            if (row_count == 0u) {
                if (sequence != 8198u) return 0;
            } else if (sequence != previous_sequence + 1u) return 0;
            previous_sequence = sequence;
            ++row_count;
            if (value > 0xffu || normalized < 0x1fe000u ||
                normalized > 0x1fe003u) return 0;
            port = normalized - 0x1fe000u;
            if (port == 0u) {
                selected_register = (uint8_t)(value & 0x1fu);
            } else if (port == 2u) {
                if (selected_register == 0u)
                    mawr = (uint16_t)((mawr & 0xff00u) | value);
                else if (selected_register == 2u) {
                    vwr_low = (uint8_t)value;
                    have_vwr_low = 1u;
                }
            } else if (port == 3u) {
                if (selected_register == 0u)
                    mawr = (uint16_t)((mawr & 0x00ffu) | (value << 8));
                else if (selected_register == 2u && have_vwr_low) {
                    if (mawr < 2048u)
                        bat[mawr] = (uint16_t)(vwr_low | (value << 8));
                    ++mawr;
                    ++commits;
                }
            }
        } else if (sscanf(line,
                   "scsi_generation_vdc_snapshot_boundary scsi_generation=%u generation_rows=%u sequence=%u timestamp=%u first_vram_word=%x last_vram_word=%x vram_words=%u vram_fnv1a=%x%n",
                   &boundary_generation, &boundary_rows, &boundary_sequence,
                   &boundary_timestamp, &boundary_first, &boundary_last,
                   &boundary_words, &boundary_hash, &consumed) == 8 &&
                   consumed == (int)length && boundary_generation == 7u) {
            if (++boundary_count != 1u || boundary_rows != 2187u ||
                boundary_sequence != 10385u || boundary_first != 0x1000u ||
                boundary_last != 0x2fffu || boundary_words != 8192u ||
                boundary_hash != 0xa0e05797u) return 0;
        } else if (sscanf(line,
                   "scsi_generation_vdc_code_byte scsi_generation=%u physical_address=%x value=%x%n",
                   &code_generation, &code_physical, &code_value,
                   &consumed) == 3 && consumed == (int)length &&
                   code_generation == 7u) {
            if (code_count >= sizeof(code) ||
                code_physical != 0x104600u + code_count || code_value > 0xffu)
                return 0;
            code[code_count++] = (uint8_t)code_value;
        }
    }
    if (source_count != 1u || row_count != 2187u || boundary_count != 1u ||
        commits != 1024u || code_count != sizeof(code)) return 0;
    for (i = 0u; i < 2048u; ++i) {
        uint16_t post_word = (uint16_t)(post_vram_snapshot[i * 2u] |
            ((uint16_t)post_vram_snapshot[i * 2u + 1u] << 8));
        if (bat[i] != post_word) return 0;
    }
    if (tqr_trace_fnv1a_bytes(post_vram_snapshot, post_vram_snapshot_size) !=
            0xd9d48117u ||
        tqr_trace_fnv1a_bytes(post_vram_snapshot, 4096u) != 0x4740a645u ||
        tqr_trace_fnv1a_bytes(post_vram_snapshot + 0x2000u, 0x2000u) !=
            payload->vram_snapshot_checksum) return 0;
    {
        size_t code_media_offset = (size_t)0x4d0u *
            THERON_TRACK02_RAW_SECTOR_BYTES +
            THERON_TRACK02_RAW_USER_DATA_OFFSET + 0x600u;
        if ((size_t)0x4d0u >=
                track02_size / THERON_TRACK02_RAW_SECTOR_BYTES ||
            code_media_offset + sizeof(code) > track02_size ||
            memcmp(code, track02_data + code_media_offset, 0x8du) != 0 ||
            memcmp(code + 0x92u, track02_data + code_media_offset + 0x92u,
                0x126u) != 0 ||
            tqr_trace_fnv1a_bytes(code, sizeof(code)) != 0x3e3745f7u ||
            tqr_trace_fnv1a_bytes(code, 0x8du) != 0xe8f39f3cu ||
            tqr_trace_fnv1a_bytes(code + 0x92u, 0x126u) != 0xd156f430u)
            return 0;
    }
    if (code[0x6fu] != 0xa5u || code[0x70u] != 0x02u ||
        code[0x71u] != 0x8du || code[0x72u] != 0x02u ||
        code[0x73u] != 0x00u || code[0x74u] != 0xa5u ||
        code[0x75u] != 0x03u || code[0x76u] != 0x8du ||
        code[0x77u] != 0x03u || code[0x78u] != 0x00u ||
        code[0x79u] != 0x03u || code[0x7au] != 0x02u ||
        code[0x8cu] != 0xe3u || code[0x8du] != 0xe0u ||
        code[0x8eu] != 0x47u || code[0x8fu] != 0x02u ||
        code[0x90u] != 0x00u || code[0x91u] != 0x40u ||
        code[0x92u] != 0x00u ||
        tqr_trace_fnv1a_bytes(code + 0x8cu, 7u) != 0x37013231u ||
        tqr_trace_fnv1a_bytes(code + 0x1e0u, 32u) != 0xda633f05u)
        return 0;

    cursor = vdc_state_capture;
    while (tqr_trace_next_line(&cursor, &line, &length)) {
        consumed = 0;
        if (length == strlen("FIRESTAFF_THERON_VDC_STATE_V1") &&
            memcmp(line, "FIRESTAFF_THERON_VDC_STATE_V1", length) == 0)
            ++state_source_count;
        else if (sscanf(line,
                    "vdc=%u bxr=%x byr=%x mwr=%x hsr=%x hdr=%x vsr=%x vdr=%x vcr=%x cr=%x%n",
                    &vdc, &bxr, &byr, &mwr, &hsr, &hdr, &vsr, &vdr, &vcr,
                    &cr, &consumed) == 10 && consumed == (int)length)
            ++state_count;
    }
    if (state_source_count != 1u || state_count != 1u || vdc != 0u ||
        bxr != 0u || byr != 0u || mwr != 0x10u || hdr != 0x041fu ||
        vdr != 0x00efu || cr != 0x0088u) return 0;

    for (i = 0u; i < 30u * 32u; ++i) {
        size_t x = i % 32u, y = i / 32u;
        uint16_t tile = (uint16_t)(bat[y * 64u + x] & 0x0fffu);
        if (tile < 0x100u || tile > 0x2ffu) return 0;
        ++source_cells;
        if (tile < first_tile) first_tile = tile;
        if (tile > last_tile) last_tile = tile;
        if (!seen_tiles[tile - 0x100u]) {
            seen_tiles[tile - 0x100u] = 1u;
            ++unique_tiles;
        }
    }
    if (source_cells != 960u || unique_tiles != 124u ||
        first_tile != 0x110u || last_tile != 0x18fu) return 0;
    for (i = 0u; i < 30u * 8u; ++i) {
        size_t tile_y = i / 8u, row = i % 8u, tile_x, pixel_x;
        for (tile_x = 0u; tile_x < 32u; ++tile_x) {
            uint16_t tile = (uint16_t)(bat[tile_y * 64u + tile_x] & 0x0fffu);
            const uint8_t *bytes = post_vram_snapshot + (size_t)tile * 32u;
            uint8_t planes[4];
            planes[0] = bytes[row * 2u];
            planes[1] = bytes[row * 2u + 1u];
            planes[2] = bytes[16u + row * 2u];
            planes[3] = bytes[16u + row * 2u + 1u];
            for (pixel_x = 0u; pixel_x < 8u; ++pixel_x) {
                unsigned int bit = 7u - (unsigned int)pixel_x;
                uint8_t pixel = (uint8_t)(((planes[0] >> bit) & 1u) |
                    (((planes[1] >> bit) & 1u) << 1) |
                    (((planes[2] >> bit) & 1u) << 2) |
                    (((planes[3] >> bit) & 1u) << 3));
                background_hash ^= pixel;
                background_hash *= 16777619u;
                color_hash ^= vce_snapshot[(size_t)pixel * 2u];
                color_hash *= 16777619u;
                color_hash ^= vce_snapshot[(size_t)pixel * 2u + 1u];
                color_hash *= 16777619u;
                ++background_pixels;
                if (pixel != 0u) ++nonzero_pixels;
            }
        }
    }
    if (background_pixels != 61440u || nonzero_pixels != 3373u ||
        background_hash != 0x2c2cfb4du || color_hash != 0x3fde1dc5u ||
        tqr_trace_fnv1a_bytes(vce_snapshot, vce_snapshot_size) != 0x1f116dc5u ||
        tqr_trace_fnv1a_bytes(vce_snapshot, 32u) != 0x0b2ae445u) return 0;

    out->valid = 1;
    out->variant = payload->variant;
    snprintf(out->track02_md5, sizeof(out->track02_md5), "%s",
        payload->track02_md5);
    out->scsi_generation = 7u;
    out->vdc_rows = row_count;
    out->vwr_commits = commits;
    out->pre_vram_checksum = 0xedfc7797u;
    out->post_vram_checksum = 0xd9d48117u;
    out->bat_checksum = 0x4740a645u;
    out->first_source_tile = first_tile;
    out->last_source_tile = last_tile;
    out->active_bat_cells = 960u;
    out->source_backed_active_cells = source_cells;
    out->unique_source_tiles = unique_tiles;
    out->background_index_pixels = background_pixels;
    out->nonzero_background_pixels = nonzero_pixels;
    out->background_index_checksum = background_hash;
    out->vce_checksum = 0x1f116dc5u;
    out->palette_zero_checksum = 0x0b2ae445u;
    out->background_color_checksum = color_hash;
    out->code_snapshot_checksum = 0x3e3745f7u;
    out->code_track02_record = 0x4d0u;
    out->code_track02_user_offset = 0x600u;
    out->source_code_bytes = 0x8du + 0x126u;
    out->tia_pc = 0x468cu;
    out->tia_source = 0x47e0u;
    out->tia_destination = 0x0002u;
    out->tia_length = 0x0040u;
    out->generated_bat_row_checksum = 0xda633f05u;
    out->vdc_replay_verified = 1;
    out->background_enabled = 1;
    out->active_bat_source_verified = 1;
    out->background_pixels_verified = 1;
    out->vce_palette_verified = 1;
    out->code_media_verified = 1;
    out->stage2_l466b_verified = 1;
    out->self_modifying_tia_verified = 1;
    out->tile_semantics_proven = 0;
    return 1;
}

int theron_v1_raw_loader_trace_bind_game_generation49_graphics(
    const Theron_V1RawLoaderTraceGameE009VdcPresentationReceipt *presentation,
    const char *cd_capture, const char *vdc_capture,
    const uint8_t *track02_data, size_t track02_size,
    const char *track02_md5,
    Theron_V1RawLoaderTraceGameGeneration49GraphicsReceipt *out)
{
    static const unsigned int setup_address[4] = {0u, 2u, 3u, 0u};
    static const unsigned int setup_value[4] = {0u, 0u, 0x10u, 2u};
    static const unsigned int setup_pc[4] = {0xecd4u, 0xf341u, 0xf346u, 0xecdeu};
    const char *cursor, *line;
    size_t length, cd_sources = 0u, commands = 0u, vdc_sources = 0u;
    size_t rows = 0u, payload_index = 0u, boundaries = 0u;
    unsigned int generation = 0u, lba = 0u, sectors = 0u;
    unsigned int sequence, row_generation, timestamp, address, physical;
    unsigned int value, writer_pc, writer_physical, previous_sequence = 0u;
    unsigned int boundary_generation, boundary_rows, boundary_sequence;
    unsigned int boundary_timestamp, boundary_first, boundary_last;
    unsigned int boundary_words, boundary_hash;
    uint8_t cdb[6] = {0}, parsed_cdb[6] = {0};
    uint32_t first_record, hash = 2166136261u;
    int consumed = 0;

    if (out) memset(out, 0, sizeof(*out));
    if (!presentation || !presentation->valid || !cd_capture || !vdc_capture ||
        !track02_data || !track02_md5 || !out ||
        presentation->variant != THERON_TRACK02_VARIANT_US_BIN ||
        !presentation->vdc_replay_verified ||
        !presentation->stage2_l466b_verified ||
        presentation->tile_semantics_proven ||
        strcmp(presentation->track02_md5, THERON_TRACK02_MD5_US_BIN) != 0 ||
        strcmp(track02_md5, THERON_TRACK02_MD5_US_BIN) != 0 ||
        track02_size % THERON_TRACK02_RAW_SECTOR_BYTES != 0u) return 0;

    cursor = cd_capture;
    while (tqr_trace_next_line(&cursor, &line, &length)) {
        if (length == strlen("source=mednafen-pce-instrumented-cd-state") &&
            memcmp(line, "source=mednafen-pce-instrumented-cd-state", length) == 0)
            ++cd_sources;
        else if (length >= strlen("scsi_read_command ") &&
                 memcmp(line, "scsi_read_command ", strlen("scsi_read_command ")) == 0) {
            unsigned int parsed_generation, parsed_lba, parsed_sectors;
            if (tqr_trace_parse_scsi_read6(line, length, &parsed_generation,
                    &parsed_lba, &parsed_sectors, parsed_cdb) &&
                parsed_generation == 49u) {
                if (++commands != 1u) return 0;
                generation = parsed_generation; lba = parsed_lba;
                sectors = parsed_sectors;
                memcpy(cdb, parsed_cdb, sizeof(cdb));
            }
        }
    }
    if (cd_sources != 1u || commands != 1u || generation != 49u ||
        lba != 4622u || sectors != 12u || cdb[0] != 0x08u ||
        lba < TQR_TRACE_TRACK02_LBA_BASE) return 0;
    first_record = lba - TQR_TRACE_TRACK02_LBA_BASE;
    if (first_record != 0x64du ||
        (size_t)first_record + sectors >
            track02_size / THERON_TRACK02_RAW_SECTOR_BYTES) return 0;

    cursor = vdc_capture;
    while (tqr_trace_next_line(&cursor, &line, &length)) {
        consumed = 0;
        if (length == strlen("source=mednafen-pce-instrumented-scsi-generation-vdc") &&
            memcmp(line, "source=mednafen-pce-instrumented-scsi-generation-vdc",
                length) == 0) ++vdc_sources;
        else if (sscanf(line,
                "scsi_generation_vdc_write sequence=%u scsi_generation=%u timestamp=%u logical_address=%x physical_address=%x value=%x writer_pc=%x writer_physical_pc=%x%n",
                &sequence, &row_generation, &timestamp, &address, &physical,
                &value, &writer_pc, &writer_physical, &consumed) == 8 &&
                consumed == (int)length && row_generation == 49u) {
            uint32_t normalized = physical & 0x7fffffffu;
            ++rows;
            if (value > 0xffu || normalized != 0x1fe000u + address ||
                writer_pc < 0xe000u || writer_physical != writer_pc - 0xe000u ||
                (rows == 1u && sequence != 15006u) ||
                (rows > 1u && sequence != previous_sequence + 1u)) return 0;
            previous_sequence = sequence;
            if (rows <= 4u) {
                size_t setup_index = rows - 1u;
                if (address != setup_address[setup_index] ||
                    value != setup_value[setup_index] ||
                    writer_pc != setup_pc[setup_index]) return 0;
            } else {
                size_t record = first_record + payload_index /
                    THERON_TRACK02_RAW_USER_DATA_BYTES;
                size_t offset = payload_index % THERON_TRACK02_RAW_USER_DATA_BYTES;
                uint8_t media_byte;
                if (payload_index >= 12u * THERON_TRACK02_RAW_USER_DATA_BYTES ||
                    address != 2u + (payload_index & 1u) ||
                    writer_pc != 0xeb35u) return 0;
                media_byte = track02_data[record * THERON_TRACK02_RAW_SECTOR_BYTES +
                    THERON_TRACK02_RAW_USER_DATA_OFFSET + offset];
                if (value != media_byte) return 0;
                hash ^= media_byte; hash *= 16777619u;
                ++payload_index;
            }
        } else if (sscanf(line,
                "scsi_generation_vdc_snapshot_boundary scsi_generation=%u generation_rows=%u sequence=%u timestamp=%u first_vram_word=%x last_vram_word=%x vram_words=%u vram_fnv1a=%x%n",
                &boundary_generation, &boundary_rows, &boundary_sequence,
                &boundary_timestamp, &boundary_first, &boundary_last,
                &boundary_words, &boundary_hash, &consumed) == 8 &&
                consumed == (int)length && boundary_generation == 49u) {
            if (++boundaries != 1u || boundary_rows != 24580u ||
                boundary_sequence != 39586u || boundary_first != 0x1000u ||
                boundary_last != 0x2fffu || boundary_words != 8192u ||
                boundary_hash != 0xc94298deu) return 0;
        }
    }
    if (vdc_sources != 1u || boundaries != 1u || rows != 24580u ||
        payload_index != 24576u ||
        hash != 0x01551f76u) return 0;

    out->valid = 1;
    out->variant = THERON_TRACK02_VARIANT_US_BIN;
    snprintf(out->track02_md5, sizeof(out->track02_md5), "%s", track02_md5);
    out->scsi_generation = generation; out->scsi_lba = lba;
    out->scsi_sector_count = sectors; out->first_raw_track02_record = first_record;
    out->payload_bytes = payload_index; out->payload_checksum = hash;
    out->vdc_rows = rows; out->vdc_payload_rows = rows - 4u;
    out->first_vram_word = 0x1000u; out->last_vram_word = 0x6fffu;
    out->read6_verified = 1; out->vdc_setup_verified = 1;
    out->repeated_writes_verified = 0; out->single_writes_verified = 1;
    out->media_bytes_verified = 1;
    out->graphics_semantics_proven = 0;
    return 1;
}

int theron_v1_raw_loader_trace_bind_game_generation51_frame(
    const Theron_V1RawLoaderTraceGameGeneration49GraphicsReceipt *graphics,
    const char *vdc_capture, const char *vdc_state_capture,
    const uint8_t *vram_snapshot, size_t vram_snapshot_size,
    const uint8_t *vce_snapshot, size_t vce_snapshot_size,
    const uint8_t *sat_snapshot, size_t sat_snapshot_size,
    const uint8_t *track02_data, size_t track02_size,
    const char *track02_md5,
    Theron_V1RawLoaderTraceGameGeneration51FrameReceipt *out)
{
    const char *cursor, *line;
    size_t length, sources = 0u, rows = 0u, boundaries = 0u;
    size_t l466b = 0u, l4943 = 0u, l50f1 = 0u, l5111 = 0u;
    size_t state_sources = 0u, states = 0u;
    unsigned int sequence, generation, timestamp, logical, physical, value;
    unsigned int pc, physical_pc, previous_sequence = 0u;
    unsigned int bg, br, bs, bt, bf, bl, bw, bh;
    unsigned int vdc, bxr, byr, mwr, hsr, hdr, vsr, vdr, vcr, cr;
    Theron_Track02Stage2Enclosing45xxCalleesReceipt stage2;
    int consumed = 0;

    if (out) memset(out, 0, sizeof(*out));
    if (!graphics || !graphics->valid || !vdc_capture || !vdc_state_capture ||
        !vram_snapshot || !vce_snapshot || !sat_snapshot || !track02_data ||
        !track02_md5 || !out || !graphics->media_bytes_verified ||
        graphics->repeated_writes_verified || !graphics->single_writes_verified ||
        graphics->graphics_semantics_proven ||
        strcmp(graphics->track02_md5, THERON_TRACK02_MD5_US_BIN) != 0 ||
        strcmp(track02_md5, THERON_TRACK02_MD5_US_BIN) != 0 ||
        vram_snapshot_size != 65536u || vce_snapshot_size != 1024u ||
        sat_snapshot_size != 512u ||
        tqr_trace_fnv1a_bytes(vram_snapshot, vram_snapshot_size) != 0xde27fc7eu ||
        tqr_trace_fnv1a_bytes(vce_snapshot, vce_snapshot_size) != 0x88629e93u ||
        tqr_trace_fnv1a_bytes(sat_snapshot, sat_snapshot_size) != 0x4d7705c5u)
        return 0;
    if (theron_v1_track02_verify_stage2_enclosing_45xx_callees(
            track02_data, track02_size, track02_md5, &stage2) !=
            THERON_TRACK02_SIGNAL_OK || !stage2.valid || !stage2.l466b_proven ||
        !stage2.l4943_proven || !stage2.l50f1_proven || !stage2.l5111_proven ||
        !stage2.l50f1_vdc_transfer_proven || !stage2.l5111_command_table_proven)
        return 0;

    cursor = vdc_capture;
    while (tqr_trace_next_line(&cursor, &line, &length)) {
        consumed = 0;
        if (length == strlen("source=mednafen-pce-instrumented-scsi-generation-vdc") &&
            memcmp(line, "source=mednafen-pce-instrumented-scsi-generation-vdc",
                length) == 0) ++sources;
        else if (sscanf(line,
                "scsi_generation_vdc_write sequence=%u scsi_generation=%u timestamp=%u logical_address=%x physical_address=%x value=%x writer_pc=%x writer_physical_pc=%x%n",
                &sequence, &generation, &timestamp, &logical, &physical, &value,
                &pc, &physical_pc, &consumed) == 8 && consumed == (int)length &&
                generation == 51u && boundaries == 0u) {
            if ((rows == 0u && sequence != 39592u) ||
                (rows > 0u && sequence != previous_sequence + 1u)) return 0;
            previous_sequence = sequence; ++rows;
            if (pc >= 0x466fu && pc <= 0x4693u) ++l466b;
            if (pc >= 0x4934u && pc <= 0x4942u) ++l4943;
            if (pc >= 0x50f1u && pc <= 0x5110u) ++l50f1;
            if (pc == 0x5110u) ++l5111;
        } else if (sscanf(line,
                "scsi_generation_vdc_snapshot_boundary scsi_generation=%u generation_rows=%u sequence=%u timestamp=%u first_vram_word=%x last_vram_word=%x vram_words=%u vram_fnv1a=%x%n",
                &bg, &br, &bs, &bt, &bf, &bl, &bw, &bh, &consumed) == 8 &&
                consumed == (int)length && bg == 51u) {
            if (++boundaries != 1u || br != 54842u || bs != 94434u ||
                bf != 0x1000u || bl != 0x2fffu || bw != 8192u ||
                bh != 0xc94298deu || rows != br) return 0;
            break;
        }
    }
    if (sources != 1u || boundaries != 1u || rows != 54842u ||
        l466b != 2176u || l4943 != 15u || l50f1 != 2064u || l5111 != 2048u)
        return 0;

    cursor = vdc_state_capture;
    while (tqr_trace_next_line(&cursor, &line, &length)) {
        consumed = 0;
        if (length == strlen("FIRESTAFF_THERON_VDC_STATE_V1") &&
            memcmp(line, "FIRESTAFF_THERON_VDC_STATE_V1", length) == 0)
            ++state_sources;
        else if (sscanf(line,
                "vdc=%u bxr=%x byr=%x mwr=%x hsr=%x hdr=%x vsr=%x vdr=%x vcr=%x cr=%x%n",
                &vdc, &bxr, &byr, &mwr, &hsr, &hdr, &vsr, &vdr, &vcr, &cr,
                &consumed) == 10 && consumed == (int)length) ++states;
    }
    if (state_sources != 1u || states != 1u || vdc != 0u || bxr != 0u ||
        byr != 0u || mwr != 0x10u || hdr != 0x041fu || vdr != 0x00efu ||
        cr != 0x0048u) return 0;

    out->valid = 1; out->variant = THERON_TRACK02_VARIANT_US_BIN;
    snprintf(out->track02_md5, sizeof(out->track02_md5), "%s", track02_md5);
    out->scsi_generation = 51u; out->generation_rows = rows;
    out->boundary_sequence = 94434u; out->vram_checksum = 0xde27fc7eu;
    out->vce_checksum = 0x88629e93u; out->sat_checksum = 0x4d7705c5u;
    out->l466b_writer_rows = l466b; out->l4943_writer_rows = l4943;
    out->l50f1_writer_rows = l50f1; out->l5111_writer_rows = l5111;
    out->atomic_snapshot_verified = 1; out->stage2_control_flow_verified = 1;
    out->stable_loop_boundary_verified = 1; out->screen_semantics_proven = 0;
    return 1;
}

int theron_v1_raw_loader_trace_bind_file_select_text_source(
    const uint8_t *track02_data, size_t track02_size,
    const char *track02_md5,
    Theron_V1RawLoaderTraceFileSelectTextSourceReceipt *out)
{
    static const size_t play_raw_offsets[3] = {2959246u, 2963770u, 2968474u};
    static const size_t load_raw_offsets[3] = {2959274u, 2963795u, 2968499u};
    static const uint32_t records[3] = {0x4eau, 0x4ecu, 0x4eeu};
    static const uint16_t sector_offsets[3] = {0x1aeu, 0x0fau, 0x0fau};
    size_t i;

    if (out) memset(out, 0, sizeof(*out));
    if (!track02_data || !track02_md5 || !out ||
        strcmp(track02_md5, THERON_TRACK02_MD5_US_BIN) != 0)
        return 0;
    for (i = 0u; i < 3u; ++i) {
        size_t play = play_raw_offsets[i], load = load_raw_offsets[i];
        size_t expected_user = (size_t)records[i] * 2048u +
            (size_t)sector_offsets[i] - 16u;
        size_t converted_user = 0u;
        if (play > track02_size || 23u > track02_size - play ||
            load > track02_size || 23u > track02_size - load ||
            play / 2352u != records[i] || play % 2352u != sector_offsets[i] ||
            theron_v1_track02_raw_offset_to_user_offset(play, track02_size,
                track02_md5, &converted_user) != THERON_TRACK02_SIGNAL_OK ||
            converted_user != expected_user ||
            tqr_trace_fnv1a_bytes(track02_data + play, 23u) != 0xef1550adu ||
            tqr_trace_fnv1a_bytes(track02_data + load, 23u) != 0xaa654403u)
            return 0;
        out->raw_track02_record[i] = records[i];
        out->raw_sector_offset[i] = sector_offsets[i];
        out->user_data_offset[i] = converted_user;
    }
    out->valid = 1;
    out->variant = THERON_TRACK02_VARIANT_US_BIN;
    snprintf(out->track02_md5, sizeof(out->track02_md5), "%s", track02_md5);
    out->occurrence_count = 3u;
    out->play_prompt_checksum = 0xef1550adu;
    out->load_prompt_checksum = 0xaa654403u;
    out->mode1_coordinates_verified = 1;
    out->source_text_verified = 1;
    out->screen_consumer_proven = 0;
    return 1;
}

#define TQR_FILE_SELECT_ENCODED_BYTES 195u

static int tqr_trace_find_file_select_ram_span(
    const char *capture, const char *row_name, unsigned int expected_pc,
    uint32_t first_physical, const uint8_t *expected,
    unsigned int *out_frame, uint32_t *out_physical_pc)
{
    const char *cursor = capture, *line;
    size_t length, index = 0u;
    unsigned int matches = 0u, frame = 0u, row_frame, sequence;
    unsigned int logical, physical, value, pc, physical_pc;
    uint32_t candidate_physical_pc = 0u;
    int consumed;

    if (!capture || !row_name || !expected || !out_frame || !out_physical_pc)
        return 0;
    while (tqr_trace_next_line(&cursor, &line, &length)) {
        consumed = 0;
        if (strcmp(row_name, "file_select_source_write") == 0) {
            if (sscanf(line,
                    "file_select_source_write sequence=%u frame=%u logical_address=%x physical_address=%x value=%x writer_pc=%x writer_physical_pc=%x%n",
                    &sequence, &row_frame, &logical, &physical, &value, &pc,
                    &physical_pc, &consumed) != 7 || consumed != (int)length)
                continue;
        } else {
            if (sscanf(line,
                    "file_select_read sequence=%u frame=%u logical_address=%x physical_address=%x value=%x reader_pc=%x reader_physical_pc=%x%n",
                    &sequence, &row_frame, &logical, &physical, &value, &pc,
                    &physical_pc, &consumed) != 7 || consumed != (int)length)
                continue;
        }
        if (pc == expected_pc && physical == first_physical + index &&
            value == expected[index] &&
            (index == 0u || (row_frame == frame &&
             physical_pc == candidate_physical_pc))) {
            if (index == 0u) {
                frame = row_frame;
                candidate_physical_pc = physical_pc;
            }
            if (++index == TQR_FILE_SELECT_ENCODED_BYTES) {
                ++matches;
                *out_frame = frame;
                *out_physical_pc = candidate_physical_pc;
                index = 0u;
            }
        } else if (pc == expected_pc && physical == first_physical) {
            index = value == expected[0] ? 1u : 0u;
            frame = row_frame;
            candidate_physical_pc = physical_pc;
        }
    }
    return matches == 1u;
}

int theron_v1_raw_loader_trace_bind_file_select_encoded_transport(
    const char *cd_capture, const char *source_write_capture,
    const char *source_read_capture, const char *consumer_read_capture,
    const char *vdc_capture, const uint8_t *track02_data,
    size_t track02_size, const char *track02_md5,
    Theron_V1RawLoaderTraceFileSelectEncodedTransportReceipt *out)
{
    static const unsigned int setup_address[13] = {
        0u, 2u, 3u, 0u, 2u, 3u, 0u, 2u, 3u, 0u, 2u, 3u, 0u
    };
    static const unsigned int setup_value[13] = {
        0x08u, 0xe8u, 0x00u, 0x07u, 0x00u, 0x00u, 0x05u,
        0xc8u, 0x00u, 0x00u, 0x00u, 0x08u, 0x02u
    };
    static const unsigned int setup_pc[13] = {
        0x4993u, 0x4999u, 0x499fu, 0x49a6u, 0x49acu, 0x49b2u,
        0x4934u, 0x4939u, 0x4942u, 0x50fbu, 0x5101u, 0x5107u,
        0x5109u
    };
    const size_t raw_sector = 0x67bu;
    const size_t raw_offset = raw_sector * THERON_TRACK02_RAW_SECTOR_BYTES;
    const uint8_t *payload;
    const char *cursor, *line;
    size_t length;
    unsigned int generation, opcode, lba, count;
    unsigned int observed_lba, bytes, sector_hash, span_offset, span_bytes;
    unsigned int span_hash, read6_count = 0u, sector_count = 0u;
    unsigned int loader_frame = 0u, transfer_frame = 0u, consumer_frame = 0u;
    unsigned int presentation_frame = 0u, presentation_reads = 0u;
    unsigned int vdc_frame = 0u, vdc_rows = 0u, vdc_total_rows = 0u;
    unsigned int vdc_sources = 0u;
    unsigned int parsed_frame = 0u;
    uint32_t loader_physical_pc = 0u, transfer_physical_pc = 0u;
    uint32_t consumer_physical_pc = 0u;
    unsigned int sequence, logical, physical, value, pc, physical_pc;
    uint8_t vdc_payload[512];
    uint8_t selected_register = 0u;
    uint16_t mawr = 0u, first_vram_word = 0xffffu, last_vram_word = 0u;
    size_t vdc_payload_index = 0u, vram_words = 0u;
    unsigned int have_vwr_low = 0u;
    int consumed;

    if (out) memset(out, 0, sizeof(*out));
    if (!out || !cd_capture || !source_write_capture || !source_read_capture ||
        !consumer_read_capture || !vdc_capture ||
        !track02_data || !track02_md5 ||
        strcmp(track02_md5, THERON_TRACK02_MD5_US_BIN) != 0 ||
        raw_offset > track02_size || THERON_TRACK02_RAW_SECTOR_BYTES >
            track02_size - raw_offset)
        return 0;
    payload = track02_data + raw_offset + THERON_TRACK02_RAW_USER_DATA_OFFSET;
    if (tqr_trace_fnv1a_bytes(track02_data + raw_offset,
            THERON_TRACK02_RAW_SECTOR_BYTES) != 0xfcc73c77u ||
        tqr_trace_fnv1a_bytes(payload, TQR_FILE_SELECT_ENCODED_BYTES) !=
            0xefad54b3u)
        return 0;

    cursor = cd_capture;
    while (tqr_trace_next_line(&cursor, &line, &length)) {
        consumed = 0;
        if (sscanf(line,
                "scsi_read_command generation=%u opcode=%x cdb=%*12s start_lba=%u sector_count=%u%n",
                &generation, &opcode, &lba, &count, &consumed) == 4 &&
            consumed == (int)length && generation == 12u) {
            if (opcode != 8u || lba != 4668u || count != 1u) return 0;
            ++read6_count;
        } else if (sscanf(line,
                "cd_interface_raw_sector_read lba=%u bytes=%u sector_fnv1a=%x span_offset=%u span_bytes=%u span_fnv1a=%x%n",
                &observed_lba, &bytes, &sector_hash, &span_offset, &span_bytes,
                &span_hash, &consumed) == 6 && consumed == (int)length &&
                observed_lba == 4668u) {
            if (bytes != 2352u || sector_hash != 0xfcc73c77u ||
                span_offset != 0u || span_bytes != 32u ||
                span_hash != 0xf58a8575u) return 0;
            ++sector_count;
        }
    }
    cursor = consumer_read_capture;
    while (tqr_trace_next_line(&cursor, &line, &length)) {
        consumed = 0;
        if (sscanf(line,
                "file_select_read sequence=%u frame=%u logical_address=%x physical_address=%x value=%x reader_pc=%x reader_physical_pc=%x%n",
                &sequence, &parsed_frame, &logical, &physical, &value,
                &pc, &physical_pc, &consumed) == 7 &&
            consumed == (int)length && logical == 0x7d58u &&
            physical == 0x0d1d58u && value == 0x02u && pc == 0x514bu &&
            physical_pc == 0x10514bu) {
            presentation_frame = parsed_frame;
            ++presentation_reads;
        }
    }
    cursor = vdc_capture;
    while (tqr_trace_next_line(&cursor, &line, &length)) {
        consumed = 0;
        if (length == strlen("source=mednafen-pce-instrumented-file-select-vdc") &&
            memcmp(line, "source=mednafen-pce-instrumented-file-select-vdc",
                length) == 0) {
            ++vdc_sources;
        } else if (sscanf(line,
                "file_select_vdc_write sequence=%u frame=%u logical_address=%x physical_address=%x value=%x writer_pc=%x writer_physical_pc=%x%*[^\n]%n",
                &sequence, &vdc_frame, &logical, &physical, &value, &pc,
                &physical_pc, &consumed) >= 7) {
            uint32_t normalized = physical & 0x7fffffffu;
            unsigned int port;
            if (sequence != vdc_total_rows || vdc_frame != 8580u ||
                value > 0xffu ||
                normalized < 0x1fe000u || normalized > 0x1fe003u)
                return 0;
            port = normalized - 0x1fe000u;
            if (vdc_total_rows < 13u) {
                if (port != setup_address[vdc_total_rows] ||
                    value != setup_value[vdc_total_rows] ||
                    pc != setup_pc[vdc_total_rows] ||
                    physical_pc != 0x100000u + pc) return 0;
            } else {
                if (pc != 0x5110u || physical_pc != 0x105110u ||
                    port != 2u + (vdc_payload_index & 1u) ||
                    vdc_payload_index >= sizeof(vdc_payload)) return 0;
                vdc_payload[vdc_payload_index++] = (uint8_t)value;
                ++vdc_rows;
            }
            if (port == 0u) selected_register = (uint8_t)(value & 0x1fu);
            else if (port == 2u) {
                if (selected_register == 0u)
                    mawr = (uint16_t)((mawr & 0xff00u) | value);
                else if (selected_register == 2u) {
                    have_vwr_low = 1u;
                }
            } else if (port == 3u) {
                if (selected_register == 0u)
                    mawr = (uint16_t)((mawr & 0x00ffu) | (value << 8));
                else if (selected_register == 2u && have_vwr_low) {
                    if (vram_words == 0u) first_vram_word = mawr;
                    last_vram_word = mawr++;
                    ++vram_words;
                    have_vwr_low = 0u;
                }
            }
            ++vdc_total_rows;
        }
    }
    if (read6_count != 1u || sector_count != 1u ||
        !tqr_trace_find_file_select_ram_span(source_write_capture,
            "file_select_source_write", 0xea9eu, 0x0ddc5bu, payload,
            &loader_frame, &loader_physical_pc) ||
        !tqr_trace_find_file_select_ram_span(source_write_capture,
            "file_select_source_write", 0x3446u, 0x0d1d3du, payload,
            &transfer_frame, &transfer_physical_pc) ||
        !tqr_trace_find_file_select_ram_span(source_read_capture,
            "file_select_read", 0x3446u, 0x0ddc5bu, payload,
            &consumer_frame, &consumer_physical_pc) ||
        loader_physical_pc != 0x000a9eu ||
        transfer_physical_pc != 0x1f1446u ||
        consumer_physical_pc != 0x1f1446u ||
        presentation_reads != 1u || vdc_sources != 1u ||
        vdc_total_rows != 525u || vdc_rows != 512u ||
        vdc_payload_index != sizeof(vdc_payload) ||
        tqr_trace_fnv1a_bytes(vdc_payload, sizeof(vdc_payload)) != 0xa8007f15u ||
        vram_words != 256u || first_vram_word != 0x0800u ||
        last_vram_word != 0x08ffu ||
        presentation_frame != vdc_frame ||
        !(loader_frame < transfer_frame && transfer_frame == consumer_frame &&
          transfer_frame < presentation_frame))
        return 0;

    out->valid = 1;
    out->variant = THERON_TRACK02_VARIANT_US_BIN;
    snprintf(out->track02_md5, sizeof(out->track02_md5), "%s", track02_md5);
    out->scsi_generation = 12u;
    out->scsi_lba = 4668u;
    out->raw_track02_record = (uint32_t)raw_sector;
    out->raw_user_data_offset = raw_offset +
        THERON_TRACK02_RAW_USER_DATA_OFFSET;
    out->byte_count = TQR_FILE_SELECT_ENCODED_BYTES;
    out->raw_sector_checksum = 0xfcc73c77u;
    out->payload_checksum = 0xefad54b3u;
    out->loader_frame = loader_frame;
    out->loader_pc = 0xea9eu;
    out->loader_physical_pc = loader_physical_pc;
    out->source_physical_first = 0x0ddc5bu;
    out->transfer_frame = transfer_frame;
    out->transfer_pc = 0x3446u;
    out->transfer_physical_pc = transfer_physical_pc;
    out->destination_physical_first = 0x0d1d3du;
    out->presentation_frame = presentation_frame;
    out->presentation_source_reader_pc = 0x514bu;
    out->presentation_source_reader_physical_pc = 0x10514bu;
    out->vdc_writer_pc = 0x5110u;
    out->vdc_writer_physical_pc = 0x105110u;
    out->vdc_setup_rows = 13u;
    out->vdc_payload_rows = 512u;
    out->vdc_payload_checksum = 0xa8007f15u;
    out->first_vram_word = 0x0800u;
    out->last_vram_word = 0x08ffu;
    out->vram_word_count = 256u;
    out->read6_verified = 1;
    out->media_to_source_ram_verified = 1;
    out->source_to_destination_ram_verified = 1;
    out->destination_consumer_verified = 1;
    out->destination_to_vdc_verified = 1;
    out->vdc_destination_replay_verified = 1;
    out->text_or_glyph_semantics_proven = 0;
    return 1;
}

int theron_v1_raw_loader_trace_bind_file_select_sat_frame(
    const Theron_V1RawLoaderTraceFileSelectEncodedTransportReceipt *transport,
    const char *vdc_state_capture,
    const uint8_t *vram_snapshot, size_t vram_snapshot_size,
    const uint8_t *vce_snapshot, size_t vce_snapshot_size,
    const uint8_t *sat_snapshot, size_t sat_snapshot_size,
    Theron_V1RawLoaderTraceFileSelectSatFrameReceipt *out)
{
    const char *cursor, *line;
    size_t length, state_sources = 0u, states = 0u, i;
    size_t nonzero_sat_entries = 0u, visible_sat_entries = 0u;
    size_t bat_references = 0u;
    size_t nonzero_frame_pixels = 0u, background_source_pixels = 0u;
    size_t sprite_source_pixels = 0u, unique_source_indices = 0u;
    uint16_t *frame = NULL;
    uint8_t source_seen[512] = {0};
    uint32_t frame_source_checksum = 2166136261u;
    uint32_t frame_color_checksum = 2166136261u;
    uint16_t sprite_min_x = 256u, sprite_max_x = 0u;
    uint16_t sprite_min_y = 240u, sprite_max_y = 0u;
    unsigned int vdc, bxr, byr, mwr, hsr, hdr, vsr, vdr, vcr, cr;
    int consumed = 0;

    if (out) memset(out, 0, sizeof(*out));
    if (!transport || !transport->valid || !vdc_state_capture ||
        !vram_snapshot || !vce_snapshot || !sat_snapshot || !out ||
        transport->variant != THERON_TRACK02_VARIANT_US_BIN ||
        strcmp(transport->track02_md5, THERON_TRACK02_MD5_US_BIN) != 0 ||
        transport->presentation_frame != 8580u ||
        !transport->destination_to_vdc_verified ||
        !transport->vdc_destination_replay_verified ||
        transport->vdc_payload_checksum != 0xa8007f15u ||
        transport->first_vram_word != 0x0800u ||
        transport->last_vram_word != 0x08ffu ||
        transport->text_or_glyph_semantics_proven ||
        vram_snapshot_size != 65536u || vce_snapshot_size != 1024u ||
        sat_snapshot_size != 512u ||
        tqr_trace_fnv1a_bytes(vram_snapshot, vram_snapshot_size) != 0x832b4d13u ||
        tqr_trace_fnv1a_bytes(vce_snapshot, vce_snapshot_size) != 0x5376a91bu ||
        tqr_trace_fnv1a_bytes(sat_snapshot, sat_snapshot_size) != 0xa8007f15u ||
        tqr_trace_fnv1a_bytes(vram_snapshot + 0x1000u, 512u) != 0xa8007f15u ||
        memcmp(vram_snapshot + 0x1000u, sat_snapshot, 512u) != 0)
        return 0;

    cursor = vdc_state_capture;
    while (tqr_trace_next_line(&cursor, &line, &length)) {
        consumed = 0;
        if (length == strlen("FIRESTAFF_THERON_VDC_STATE_V1") &&
            memcmp(line, "FIRESTAFF_THERON_VDC_STATE_V1", length) == 0)
            ++state_sources;
        else if (sscanf(line,
                "vdc=%u bxr=%x byr=%x mwr=%x hsr=%x hdr=%x vsr=%x vdr=%x vcr=%x cr=%x%n",
                &vdc, &bxr, &byr, &mwr, &hsr, &hdr, &vsr, &vdr, &vcr, &cr,
                &consumed) == 10 && consumed == (int)length) ++states;
    }
    if (state_sources != 1u || states != 1u || vdc != 0u || bxr != 0u ||
        byr != 0x00e8u || mwr != 0x0010u || hdr != 0x041fu ||
        vdr != 0x00efu || cr != 0x00c8u) return 0;

    for (i = 0u; i < 64u; ++i) {
        size_t byte_index;
        int nonzero = 0;
        for (byte_index = 0u; byte_index < 8u; ++byte_index)
            if (sat_snapshot[i * 8u + byte_index] != 0u) nonzero = 1;
        if (nonzero) {
            uint16_t syw, sxw, pn, flags;
            static const uint16_t expected_y[3] = {0x0130u, 0x0040u, 0x00f0u};
            static const uint16_t expected_x[6] = {
                0x00e0u, 0x00c0u, 0x00a0u, 0x0080u, 0x0060u, 0x0040u
            };
            if (i >= 18u) return 0;
            syw = (uint16_t)(sat_snapshot[i * 8u] |
                ((uint16_t)sat_snapshot[i * 8u + 1u] << 8));
            sxw = (uint16_t)(sat_snapshot[i * 8u + 2u] |
                ((uint16_t)sat_snapshot[i * 8u + 3u] << 8));
            pn = (uint16_t)(sat_snapshot[i * 8u + 4u] |
                ((uint16_t)sat_snapshot[i * 8u + 5u] << 8));
            flags = (uint16_t)(sat_snapshot[i * 8u + 6u] |
                ((uint16_t)sat_snapshot[i * 8u + 7u] << 8));
            if (syw != expected_y[i / 6u] || sxw != expected_x[i % 6u] ||
                pn != 0x0210u || flags != (i < 6u ? 0x0180u : 0x3180u))
                return 0;
            ++nonzero_sat_entries;
            if (i >= 6u) ++visible_sat_entries;
        } else if (i < 18u) return 0;
    }
    for (i = 0u; i < 2048u; ++i) {
        uint16_t word = (uint16_t)(vram_snapshot[i * 2u] |
            ((uint16_t)vram_snapshot[i * 2u + 1u] << 8));
        uint16_t index = (uint16_t)(word & 0x0fffu);
        if (index >= 0x0080u && index <= 0x008fu) ++bat_references;
    }
    if (nonzero_sat_entries != 18u || visible_sat_entries != 12u ||
        bat_references != 0u ||
        tqr_trace_fnv1a_bytes(vram_snapshot + 0x8400u, 1024u) != 0xba5526c5u)
        return 0;

    frame = (uint16_t *)calloc(256u * 240u, sizeof(*frame));
    if (!frame) return 0;
    for (i = 0u; i < 256u * 240u; ++i) {
        unsigned int x = (unsigned int)(i % 256u);
        unsigned int y = (unsigned int)(i / 256u);
        unsigned int sx = (bxr + x) & 0x01ffu;
        unsigned int sy = (byr + y) & 0x00ffu;
        size_t bat_byte = ((size_t)(sy >> 3) * 64u + (sx >> 3)) * 2u;
        uint16_t bat = (uint16_t)vram_snapshot[bat_byte] |
            ((uint16_t)vram_snapshot[bat_byte + 1u] << 8);
        size_t pattern_byte = (size_t)(bat & 0x0fffu) * 32u +
            (size_t)(sy & 7u) * 2u;
        unsigned int bit = 7u - (sx & 7u);
        unsigned int pixel =
            ((vram_snapshot[pattern_byte] >> bit) & 1u) |
            (((vram_snapshot[pattern_byte + 1u] >> bit) & 1u) << 1) |
            (((vram_snapshot[pattern_byte + 16u] >> bit) & 1u) << 2) |
            (((vram_snapshot[pattern_byte + 17u] >> bit) & 1u) << 3);
        if (pixel) frame[i] = (uint16_t)(((bat >> 12) << 4) | pixel);
    }
    for (i = 0u; i < 240u; ++i) {
        struct TqrFileSelectSpritePiece {
            uint16_t flags, pattern, palette;
            int x, row;
        } active[16];
        int active_count = 0;
        size_t sprite;
        for (sprite = 0u; sprite < 64u && active_count < 16; ++sprite) {
            const uint8_t *s = sat_snapshot + sprite * 8u;
            uint16_t syw = (uint16_t)s[0] | ((uint16_t)s[1] << 8);
            uint16_t sxw = (uint16_t)s[2] | ((uint16_t)s[3] << 8);
            uint16_t pn = (uint16_t)s[4] | ((uint16_t)s[5] << 8);
            uint16_t flags = (uint16_t)s[6] | ((uint16_t)s[7] << 8);
            static const int heights[4] = {16, 32, 64, 64};
            static const unsigned int masks[4] = {~0u, ~2u, ~6u, ~6u};
            int sy = (int)(syw & 0x03ffu) - 0x40;
            int height = heights[(flags >> 12) & 3u];
            int width = (flags & 0x0100u) ? 32 : 16;
            int row, half;
            unsigned int base;
            if ((int)i < sy || (int)i >= sy + height) continue;
            row = (int)i - sy;
            if (flags & 0x8000u) row = height - 1 - row;
            base = ((pn >> 1) & 0x03ffu) & masks[(flags >> 12) & 3u];
            base |= (unsigned int)(row & 0x30) >> 3;
            if (width == 32) base &= ~1u;
            for (half = 0; half < width / 16 && active_count < 16; ++half) {
                unsigned int pattern = base | (unsigned int)half;
                if ((flags & 0x0800u) && width == 32) pattern ^= 1u;
                active[active_count].flags = flags;
                active[active_count].pattern = (uint16_t)pattern;
                active[active_count].palette = (uint16_t)((flags & 0xfu) << 4);
                active[active_count].x =
                    (int)(sxw & 0x03ffu) - 0x20 + half * 16;
                active[active_count].row = row & 15;
                ++active_count;
            }
        }
        while (active_count-- > 0) {
            const struct TqrFileSelectSpritePiece *sp = &active[active_count];
            size_t word_base = (size_t)sp->pattern * 64u + (size_t)sp->row;
            uint16_t planes[4];
            int plane, px;
            if (word_base + 48u >= vram_snapshot_size / 2u) continue;
            for (plane = 0; plane < 4; ++plane) {
                size_t byte = (word_base + (size_t)plane * 16u) * 2u;
                planes[plane] = (uint16_t)vram_snapshot[byte] |
                    ((uint16_t)vram_snapshot[byte + 1u] << 8);
            }
            for (px = 0; px < 16; ++px) {
                int bit = (sp->flags & 0x0800u) ? px : 15 - px;
                int dx = sp->x + px;
                unsigned int pixel = 0u;
                uint16_t *destination;
                if (dx < 0 || dx >= 256) continue;
                for (plane = 0; plane < 4; ++plane)
                    pixel |= ((planes[plane] >> bit) & 1u) << plane;
                if (!pixel) continue;
                destination = &frame[i * 256u + (size_t)dx];
                if ((*destination & 0x0fu) == 0u || (sp->flags & 0x0080u))
                    *destination = (uint16_t)(0x100u | sp->palette | pixel);
            }
        }
    }
    for (i = 0u; i < 256u * 240u; ++i) {
        uint16_t source = frame[i] & 0x01ffu;
        size_t color_byte = (size_t)source * 2u;
        frame_source_checksum = tqr_trace_fnv1a_u16(
            frame_source_checksum, source);
        frame_color_checksum ^= vce_snapshot[color_byte];
        frame_color_checksum *= 16777619u;
        frame_color_checksum ^= vce_snapshot[color_byte + 1u];
        frame_color_checksum *= 16777619u;
        if (!source_seen[source]) {
            source_seen[source] = 1u;
            ++unique_source_indices;
        }
        if (source) ++nonzero_frame_pixels;
        if (source > 0u && source < 0x100u) ++background_source_pixels;
        if (source >= 0x100u) {
            uint16_t x = (uint16_t)(i % 256u);
            uint16_t y = (uint16_t)(i / 256u);
            ++sprite_source_pixels;
            if (x < sprite_min_x) sprite_min_x = x;
            if (x > sprite_max_x) sprite_max_x = x;
            if (y < sprite_min_y) sprite_min_y = y;
            if (y > sprite_max_y) sprite_max_y = y;
        }
    }
    free(frame);
    if (nonzero_frame_pixels != 43087u ||
        background_source_pixels != 18511u ||
        sprite_source_pixels != 24576u || unique_source_indices != 40u ||
        frame_source_checksum != 0x7622aee1u ||
        frame_color_checksum != 0x8f1cf573u ||
        sprite_min_x != 32u || sprite_max_x != 223u ||
        sprite_min_y != 0u || sprite_max_y != 239u ||
        vce_snapshot[514u] != 0u || vce_snapshot[515u] != 0u)
        return 0;

    out->valid = 1;
    out->variant = transport->variant;
    snprintf(out->track02_md5, sizeof(out->track02_md5), "%s",
        transport->track02_md5);
    out->frame = 8580u;
    out->first_vram_word = 0x0800u;
    out->last_vram_word = 0x08ffu;
    out->byte_count = 512u;
    out->staging_checksum = 0xa8007f15u;
    out->vram_checksum = 0x832b4d13u;
    out->vce_checksum = 0x5376a91bu;
    out->sat_checksum = 0xa8007f15u;
    out->nonzero_sat_entries = nonzero_sat_entries;
    out->visible_sat_entries = visible_sat_entries;
    out->first_sprite_pattern = 0x0108u;
    out->last_sprite_pattern = 0x010fu;
    out->sprite_pattern_bytes = 1024u;
    out->sprite_pattern_checksum = 0xba5526c5u;
    out->bat_reference_count = bat_references;
    out->frame_pixels = 256u * 240u;
    out->nonzero_frame_pixels = nonzero_frame_pixels;
    out->background_source_pixels = background_source_pixels;
    out->sprite_source_pixels = sprite_source_pixels;
    out->unique_source_indices = unique_source_indices;
    out->frame_source_checksum = frame_source_checksum;
    out->frame_color_checksum = frame_color_checksum;
    out->sprite_source_index = 0x0101u;
    out->sprite_color = 0x0000u;
    out->sprite_min_x = sprite_min_x;
    out->sprite_max_x = sprite_max_x;
    out->sprite_min_y = sprite_min_y;
    out->sprite_max_y = sprite_max_y;
    out->byr = (uint16_t)byr;
    out->mwr = (uint16_t)mwr;
    out->cr = (uint16_t)cr;
    out->atomic_snapshot_verified = 1;
    out->vram_sat_identity_verified = 1;
    out->sat_record_layout_verified = 1;
    out->sprite_pattern_range_verified = 1;
    out->frame_composition_verified = 1;
    out->vce_color_composition_verified = 1;
    out->sprite_or_screen_semantics_proven = 0;
    return 1;
}

static int tqr_trace_file_select_frame_end_state(
    const char *capture, unsigned int expected_frame, uint16_t expected_byr)
{
    const char *cursor = capture, *line;
    size_t length, sources = 0u, states = 0u, markers = 0u;
    unsigned int vdc, bxr, byr, mwr, hsr, hdr, vsr, vdr, vcr, cr, frame;
    int consumed;

    if (!capture) return 0;
    while (tqr_trace_next_line(&cursor, &line, &length)) {
        consumed = 0;
        if (length == strlen("FIRESTAFF_THERON_VDC_STATE_V1") &&
            memcmp(line, "FIRESTAFF_THERON_VDC_STATE_V1", length) == 0)
            ++sources;
        else if (sscanf(line,
                "vdc=%u bxr=%x byr=%x mwr=%x hsr=%x hdr=%x vsr=%x vdr=%x vcr=%x cr=%x%n",
                &vdc, &bxr, &byr, &mwr, &hsr, &hdr, &vsr, &vdr, &vcr, &cr,
                &consumed) == 10 && consumed == (int)length) {
            if (vdc != 0u || bxr != 0u || byr != expected_byr ||
                mwr != 0x0010u || hsr != 0x0202u || hdr != 0x041fu ||
                vsr != 0x0f02u || vdr != 0x00efu || vcr != 0x0004u ||
                cr != 0x00c8u) return 0;
            ++states;
        } else if (sscanf(line, "snapshot_frame_end=%u%n", &frame,
                &consumed) == 1 && consumed == (int)length) {
            if (frame != expected_frame) return 0;
            ++markers;
        } else return 0;
    }
    return sources == 1u && states == 1u && markers == 1u;
}

static int tqr_trace_compose_file_select_source_frame(
    const uint8_t *vram, size_t vram_size,
    const uint8_t *sat, size_t sat_size, uint16_t bxr, uint16_t byr,
    uint16_t *frame)
{
    size_t i;
    if (!vram || vram_size != 65536u || !sat || sat_size != 512u ||
        !frame) return 0;
    memset(frame, 0, 256u * 240u * sizeof(*frame));
    for (i = 0u; i < 256u * 240u; ++i) {
        unsigned int x = (unsigned int)(i % 256u);
        unsigned int y = (unsigned int)(i / 256u);
        unsigned int sx = (bxr + x) & 0x01ffu;
        unsigned int sy = (byr + y) & 0x00ffu;
        size_t bat_byte = ((size_t)(sy >> 3) * 64u + (sx >> 3)) * 2u;
        uint16_t bat = (uint16_t)vram[bat_byte] |
            ((uint16_t)vram[bat_byte + 1u] << 8);
        size_t pattern_byte = (size_t)(bat & 0x0fffu) * 32u +
            (size_t)(sy & 7u) * 2u;
        unsigned int bit = 7u - (sx & 7u);
        unsigned int pixel =
            ((vram[pattern_byte] >> bit) & 1u) |
            (((vram[pattern_byte + 1u] >> bit) & 1u) << 1) |
            (((vram[pattern_byte + 16u] >> bit) & 1u) << 2) |
            (((vram[pattern_byte + 17u] >> bit) & 1u) << 3);
        if (pixel) frame[i] = (uint16_t)(((bat >> 12) << 4) | pixel);
    }
    for (i = 0u; i < 240u; ++i) {
        struct TqrScrollSpritePiece {
            uint16_t flags, pattern, palette;
            int x, row;
        } active[16];
        int active_count = 0;
        size_t sprite;
        for (sprite = 0u; sprite < 64u && active_count < 16; ++sprite) {
            const uint8_t *s = sat + sprite * 8u;
            uint16_t syw = (uint16_t)s[0] | ((uint16_t)s[1] << 8);
            uint16_t sxw = (uint16_t)s[2] | ((uint16_t)s[3] << 8);
            uint16_t pn = (uint16_t)s[4] | ((uint16_t)s[5] << 8);
            uint16_t flags = (uint16_t)s[6] | ((uint16_t)s[7] << 8);
            static const int heights[4] = {16, 32, 64, 64};
            static const unsigned int masks[4] = {~0u, ~2u, ~6u, ~6u};
            int sy = (int)(syw & 0x03ffu) - 0x40;
            int height = heights[(flags >> 12) & 3u];
            int width = (flags & 0x0100u) ? 32 : 16;
            int row, half;
            unsigned int base;
            if ((int)i < sy || (int)i >= sy + height) continue;
            row = (int)i - sy;
            if (flags & 0x8000u) row = height - 1 - row;
            base = ((pn >> 1) & 0x03ffu) & masks[(flags >> 12) & 3u];
            base |= (unsigned int)(row & 0x30) >> 3;
            if (width == 32) base &= ~1u;
            for (half = 0; half < width / 16 && active_count < 16; ++half) {
                unsigned int pattern = base | (unsigned int)half;
                if ((flags & 0x0800u) && width == 32) pattern ^= 1u;
                active[active_count].flags = flags;
                active[active_count].pattern = (uint16_t)pattern;
                active[active_count].palette = (uint16_t)((flags & 0xfu) << 4);
                active[active_count].x =
                    (int)(sxw & 0x03ffu) - 0x20 + half * 16;
                active[active_count].row = row & 15;
                ++active_count;
            }
        }
        while (active_count-- > 0) {
            const struct TqrScrollSpritePiece *sp = &active[active_count];
            size_t word_base = (size_t)sp->pattern * 64u + (size_t)sp->row;
            uint16_t planes[4];
            int plane, px;
            if (word_base + 48u >= vram_size / 2u) continue;
            for (plane = 0; plane < 4; ++plane) {
                size_t byte = (word_base + (size_t)plane * 16u) * 2u;
                planes[plane] = (uint16_t)vram[byte] |
                    ((uint16_t)vram[byte + 1u] << 8);
            }
            for (px = 0; px < 16; ++px) {
                int bit = (sp->flags & 0x0800u) ? px : 15 - px;
                int dx = sp->x + px;
                unsigned int pixel = 0u;
                uint16_t *destination;
                if (dx < 0 || dx >= 256) continue;
                for (plane = 0; plane < 4; ++plane)
                    pixel |= ((planes[plane] >> bit) & 1u) << plane;
                if (!pixel) continue;
                destination = &frame[i * 256u + (size_t)dx];
                if ((*destination & 0x0fu) == 0u || (sp->flags & 0x0080u))
                    *destination = (uint16_t)(0x100u | sp->palette | pixel);
            }
        }
    }
    return 1;
}

int theron_v1_raw_loader_trace_bind_file_select_scroll_transition(
    const Theron_V1RawLoaderTraceFileSelectEncodedTransportReceipt *transport,
    const char *vdc_capture, const char *pre_vdc_state_capture,
    const uint8_t *pre_vram, size_t pre_vram_size,
    const uint8_t *pre_vce, size_t pre_vce_size,
    const uint8_t *pre_sat, size_t pre_sat_size,
    const char *post_vdc_state_capture,
    const uint8_t *post_vram, size_t post_vram_size,
    const uint8_t *post_vce, size_t post_vce_size,
    const uint8_t *post_sat, size_t post_sat_size,
    Theron_V1RawLoaderTraceFileSelectScrollReceipt *out)
{
    const char *cursor, *line;
    size_t length, rows = 0u, sources = 0u, i;
    uint16_t *pre_frame = NULL, *post_frame = NULL;
    uint32_t pre_source = 2166136261u, post_source = 2166136261u;
    uint32_t pre_color = 2166136261u, post_color = 2166136261u;
    size_t pre_nonzero = 0u, post_nonzero = 0u, sprite_pixels = 0u;
    size_t changed = 0u, changed_sprite = 0u;
    uint16_t min_x = 256u, max_x = 0u, min_y = 240u, max_y = 0u;
    unsigned int sequence, frame, logical, physical, value, pc, physical_pc;
    unsigned int a, x, y;
    int consumed;

    if (out) memset(out, 0, sizeof(*out));
    if (!transport || !transport->valid || !vdc_capture || !out ||
        transport->variant != THERON_TRACK02_VARIANT_US_BIN ||
        strcmp(transport->track02_md5, THERON_TRACK02_MD5_US_BIN) != 0 ||
        transport->presentation_frame != 8580u ||
        !transport->destination_to_vdc_verified ||
        !transport->vdc_destination_replay_verified ||
        !tqr_trace_file_select_frame_end_state(
            pre_vdc_state_capture, 8579u, 0x00e9u) ||
        !tqr_trace_file_select_frame_end_state(
            post_vdc_state_capture, 8581u, 0x00e8u) ||
        pre_vram_size != 65536u || post_vram_size != 65536u ||
        pre_vce_size != 1024u || post_vce_size != 1024u ||
        pre_sat_size != 512u || post_sat_size != 512u ||
        memcmp(pre_vram, post_vram, pre_vram_size) != 0 ||
        memcmp(pre_vce, post_vce, pre_vce_size) != 0 ||
        memcmp(pre_sat, post_sat, pre_sat_size) != 0 ||
        tqr_trace_fnv1a_bytes(pre_vram, pre_vram_size) != 0x832b4d13u ||
        tqr_trace_fnv1a_bytes(pre_vce, pre_vce_size) != 0x5376a91bu ||
        tqr_trace_fnv1a_bytes(pre_sat, pre_sat_size) != 0xa8007f15u)
        return 0;

    cursor = vdc_capture;
    while (tqr_trace_next_line(&cursor, &line, &length)) {
        consumed = 0;
        if (length == strlen("source=mednafen-pce-instrumented-file-select-vdc") &&
            memcmp(line, "source=mednafen-pce-instrumented-file-select-vdc",
                length) == 0) {
            ++sources;
            continue;
        }
        if (sscanf(line,
                "file_select_vdc_write sequence=%u frame=%u logical_address=%x physical_address=%x value=%x writer_pc=%x writer_physical_pc=%x a=%x x=%x y=%x%n",
                &sequence, &frame, &logical, &physical, &value, &pc,
                &physical_pc, &a, &x, &y, &consumed) != 10 ||
            consumed != (int)length || sequence != rows || frame != 8580u)
            return 0;
        if ((rows == 0u && (logical != 0x21ddu || value != 0x08u ||
                pc != 0x4993u || physical_pc != 0x104993u)) ||
            (rows == 1u && (logical != 0x0002u || value != 0xe8u ||
                pc != 0x4999u || physical_pc != 0x104999u)) ||
            (rows == 2u && (logical != 0x0003u || value != 0x00u ||
                pc != 0x499fu || physical_pc != 0x10499fu))) return 0;
        ++rows;
    }
    if (sources != 1u || rows != 525u) return 0;

    pre_frame = (uint16_t *)malloc(256u * 240u * sizeof(*pre_frame));
    post_frame = (uint16_t *)malloc(256u * 240u * sizeof(*post_frame));
    if (!pre_frame || !post_frame ||
        !tqr_trace_compose_file_select_source_frame(pre_vram, pre_vram_size,
            pre_sat, pre_sat_size, 0u, 0x00e9u, pre_frame) ||
        !tqr_trace_compose_file_select_source_frame(post_vram, post_vram_size,
            post_sat, post_sat_size, 0u, 0x00e8u, post_frame)) {
        free(pre_frame); free(post_frame); return 0;
    }
    for (i = 0u; i < 256u * 240u; ++i) {
        uint16_t before = pre_frame[i] & 0x01ffu;
        uint16_t after = post_frame[i] & 0x01ffu;
        size_t before_color = (size_t)before * 2u;
        size_t after_color = (size_t)after * 2u;
        pre_source = tqr_trace_fnv1a_u16(pre_source, before);
        post_source = tqr_trace_fnv1a_u16(post_source, after);
        pre_color ^= pre_vce[before_color]; pre_color *= 16777619u;
        pre_color ^= pre_vce[before_color + 1u]; pre_color *= 16777619u;
        post_color ^= post_vce[after_color]; post_color *= 16777619u;
        post_color ^= post_vce[after_color + 1u]; post_color *= 16777619u;
        if (before) ++pre_nonzero;
        if (after) ++post_nonzero;
        if (before >= 0x100u) ++sprite_pixels;
        if (before != after) {
            uint16_t px = (uint16_t)(i % 256u);
            uint16_t py = (uint16_t)(i / 256u);
            ++changed;
            if (before >= 0x100u || after >= 0x100u) ++changed_sprite;
            if (px < min_x) min_x = px;
            if (px > max_x) max_x = px;
            if (py < min_y) min_y = py;
            if (py > max_y) max_y = py;
        }
    }
    free(pre_frame); free(post_frame);
    if (pre_source != 0xaf183e0du || post_source != 0x7622aee1u ||
        pre_color != 0x68fe4a69u || post_color != 0x8f1cf573u ||
        pre_nonzero != 43047u || post_nonzero != 43087u ||
        sprite_pixels != 24576u || changed != 12644u || changed_sprite != 0u ||
        min_x != 32u || max_x != 223u || min_y != 64u || max_y != 175u)
        return 0;

    out->valid = 1;
    out->variant = transport->variant;
    snprintf(out->track02_md5, sizeof(out->track02_md5), "%s",
        transport->track02_md5);
    out->pre_frame = 8579u; out->update_frame = 8580u; out->post_frame = 8581u;
    out->pre_byr = 0x00e9u; out->post_byr = 0x00e8u;
    out->register_select_pc = 0x4993u;
    out->low_byte_writer_pc = 0x4999u;
    out->high_byte_writer_pc = 0x499fu;
    out->vram_checksum = 0x832b4d13u;
    out->vce_checksum = 0x5376a91bu;
    out->sat_checksum = 0xa8007f15u;
    out->pre_source_checksum = pre_source; out->post_source_checksum = post_source;
    out->pre_color_checksum = pre_color; out->post_color_checksum = post_color;
    out->pre_nonzero_pixels = pre_nonzero; out->post_nonzero_pixels = post_nonzero;
    out->sprite_pixels = sprite_pixels; out->changed_pixels = changed;
    out->changed_sprite_pixels = changed_sprite;
    out->changed_min_x = min_x; out->changed_max_x = max_x;
    out->changed_min_y = min_y; out->changed_max_y = max_y;
    out->frame_end_markers_verified = 1;
    out->graphics_snapshots_identical = 1;
    out->game_byr_write_verified = 1;
    out->composition_delta_verified = 1;
    out->sprite_mask_static_verified = 1;
    out->screen_semantics_proven = 0;
    return 1;
}

int theron_v1_raw_loader_trace_bind_file_select_scroll_driver(
    const Theron_V1RawLoaderTraceFileSelectScrollReceipt *scroll,
    const uint8_t *code_snapshot, size_t code_snapshot_size,
    const uint8_t *ram_snapshot, size_t ram_snapshot_size,
    const char *scroll_ram_capture,
    const char *scroll_control_capture,
    Theron_V1RawLoaderTraceFileSelectScrollDriverReceipt *out)
{
    static const uint8_t signed_loop[] = {
        0xa5,0x51,0x05,0x50,0xf0,0x26,0xa5,0x51,0x18,0x6d,0x10,0x22,
        0x8d,0x10,0x22,0xa0,0x69,0xa5,0x50,0x10,0x02,0xa0,0xe9,0x8c,
        0x99,0x41,0x18,0x6d,0x0c,0x22,0x8d,0x0c,0x22,0xad,0x0d,0x22,
        0x69,0x00,0x8d,0x0d,0x22,0x20,0x15,0x42,0x60
    };
    static const uint8_t byr_load[] = {
        0xad,0x10,0x22,0x8d,0x02,0x00,0xad,0x11,0x22,0x8d,0x03,0x00
    };
    static const uint8_t next_control_loop[] = {
        0xc6,0x38,0xd0,0x06,0xa5,0x39,0xd0,0x02,0x64,0x50,0xa5,0x38,
        0x8d,0xba,0x47,0xa5,0x39,0x8d,0xbb,0x47,0x9c,0xd4,0x47,0x60
    };
    const char *cursor, *line;
    size_t length, rows, sources;
    unsigned int sequence, frame, logical, physical, old_value, value;
    unsigned int pc, physical_pc, a, x, y, group, slot;
    int consumed, ram_boundary = 0, control_boundary = 0;
    int next_phase_setup = 0, next_phase_stop = 0, next_phase_reset = 0;

    if (out) memset(out, 0, sizeof(*out));
    if (!scroll || !scroll->valid || !out || !code_snapshot || !ram_snapshot ||
        !scroll_ram_capture || !scroll_control_capture ||
        scroll->variant != THERON_TRACK02_VARIANT_US_BIN ||
        strcmp(scroll->track02_md5, THERON_TRACK02_MD5_US_BIN) != 0 ||
        scroll->update_frame != 8580u || !scroll->game_byr_write_verified ||
        code_snapshot_size != 16384u || ram_snapshot_size != 8192u ||
        tqr_trace_fnv1a_bytes(code_snapshot, code_snapshot_size) != 0x0408d000u ||
        tqr_trace_fnv1a_bytes(ram_snapshot, ram_snapshot_size) != 0x6908b113u ||
        memcmp(code_snapshot + 0x0175u, signed_loop, sizeof(signed_loop)) != 0 ||
        memcmp(code_snapshot + 0x0993u, byr_load, sizeof(byr_load)) != 0 ||
        memcmp(code_snapshot + 0x0b0cu, next_control_loop,
            sizeof(next_control_loop)) != 0 ||
        ram_snapshot[0x020cu] != 0u || ram_snapshot[0x020du] != 0u ||
        ram_snapshot[0x0210u] != 0xe8u || ram_snapshot[0x0211u] != 0u)
        return 0;

    cursor = scroll_ram_capture; rows = 0u; sources = 0u;
    while (tqr_trace_next_line(&cursor, &line, &length)) {
        if (length == strlen("source=mednafen-pce-instrumented-file-select-scroll-ram") &&
            memcmp(line, "source=mednafen-pce-instrumented-file-select-scroll-ram", length) == 0) {
            ++sources; continue;
        }
        consumed = 0;
        if (sscanf(line,
                "file_select_scroll_ram_write sequence=%u frame=%u logical_address=%x physical_address=%x old=%x value=%x writer_pc=%x writer_physical_pc=%x a=%x x=%x y=%x%n",
                &sequence, &frame, &logical, &physical, &old_value, &value,
                &pc, &physical_pc, &a, &x, &y, &consumed) != 11 ||
            consumed != (int)length || sequence != rows) return 0;
        if (sequence < 432u) {
            group = sequence / 3u; slot = sequence % 3u;
            if (frame != 8522u + group * 8u) return 0;
            if (slot == 0u) {
                if (logical != 0x2210u || physical != 0x1f0210u ||
                    old_value != 0xf0u - group || value != 0xefu - group ||
                    pc != 0x4184u || physical_pc != 0x104184u) return 0;
            } else if (slot == 1u) {
                if (logical != 0x220cu || physical != 0x1f020cu ||
                    old_value != 0u || value != 0u || pc != 0x4196u ||
                    physical_pc != 0x104196u) return 0;
            } else if (logical != 0x220du || physical != 0x1f020du ||
                       old_value != 0u || value != 0u || pc != 0x419eu ||
                       physical_pc != 0x10419eu) return 0;
        } else if (sequence == 432u) {
            if (frame != 10310u || logical != 0x220cu || old_value != 0u ||
                value != 0u || pc != 0x4005u) return 0;
            ram_boundary = 1;
        }
        ++rows;
    }
    if (sources != 1u || rows < 433u || !ram_boundary) return 0;

    cursor = scroll_control_capture; rows = 0u; sources = 0u;
    while (tqr_trace_next_line(&cursor, &line, &length)) {
        if (length == strlen("source=mednafen-pce-instrumented-file-select-scroll-control") &&
            memcmp(line, "source=mednafen-pce-instrumented-file-select-scroll-control", length) == 0) {
            ++sources; continue;
        }
        consumed = 0;
        if (sscanf(line,
                "file_select_scroll_control_write sequence=%u frame=%u logical_address=%x physical_address=%x old=%x value=%x writer_pc=%x writer_physical_pc=%x a=%x x=%x y=%x%n",
                &sequence, &frame, &logical, &physical, &old_value, &value,
                &pc, &physical_pc, &a, &x, &y, &consumed) != 11 ||
            consumed != (int)length || sequence != rows) return 0;
        if (sequence < 288u) {
            group = sequence / 2u; slot = sequence % 2u;
            if (frame != 8524u + group * 8u) return 0;
            if (slot == 0u) {
                if (logical != 0x47bcu || physical != 0x1047bcu ||
                    old_value != 0x90u - group || value != 0x8fu - group ||
                    pc != 0x4a7bu || physical_pc != 0x104a7bu) return 0;
            } else if (logical != 0x47bdu || physical != 0x1047bdu ||
                       old_value != 0u || value != 0u || pc != 0x4a80u ||
                       physical_pc != 0x104a80u) return 0;
        } else if (sequence == 288u) {
            if (frame != 10310u || logical != 0x47bau || old_value != 0u ||
                value != 0u || pc != 0x45b0u) return 0;
            control_boundary = 1;
        } else if (sequence >= 2340u && sequence <= 2345u) {
            static const unsigned int setup_address[6] = {
                0x47bcu, 0x47bau, 0x47bdu, 0x47bbu, 0x47bcu, 0x47bdu
            };
            static const unsigned int setup_old[6] = { 0u, 0u, 0u, 0u, 0x40u, 0u };
            static const unsigned int setup_value[6] = { 0x40u, 0x40u, 0u, 0u, 0u, 0u };
            static const unsigned int setup_pc[6] = {
                0x40f9u, 0x40fcu, 0x4101u, 0x4104u, 0x413fu, 0x4142u
            };
            slot = sequence - 2340u;
            if (frame != 10632u || logical != setup_address[slot] ||
                old_value != setup_old[slot] || value != setup_value[slot] ||
                pc != setup_pc[slot] || physical_pc != 0x100000u + setup_pc[slot])
                return 0;
            if (sequence == 2345u) next_phase_setup = 1;
        } else if (sequence >= 2346u && sequence <= 2473u) {
            group = (sequence - 2346u) / 2u;
            slot = (sequence - 2346u) % 2u;
            if (frame != 10644u + group * 10u) return 0;
            if (slot == 0u) {
                if (logical != 0x47bau || physical != 0x1047bau ||
                    old_value != 0x40u - group || value != 0x3fu - group ||
                    pc != 0x4b1bu || physical_pc != 0x104b1bu) return 0;
            } else if (logical != 0x47bbu || physical != 0x1047bbu ||
                       old_value != 0u || value != 0u || pc != 0x4b20u ||
                       physical_pc != 0x104b20u) return 0;
            if (sequence == 2473u) next_phase_stop = 1;
        } else if (sequence >= 2474u && sequence <= 2477u) {
            if (frame != 11578u || logical != 0x47bau + sequence - 2474u ||
                old_value != 0u || value != 0u || pc != 0x45b0u ||
                physical_pc != 0x1045b0u) return 0;
            if (sequence == 2477u) next_phase_reset = 1;
        }
        ++rows;
    }
    if (sources != 1u || rows < 2478u || !control_boundary ||
        !next_phase_setup || !next_phase_stop || !next_phase_reset) return 0;

    out->valid = 1; out->variant = scroll->variant;
    snprintf(out->track02_md5, sizeof(out->track02_md5), "%s", scroll->track02_md5);
    out->code_checksum = 0x0408d000u; out->ram_checksum = 0x6908b113u;
    out->byr_source_address = 0x2210u; out->byr_load_pc = 0x4993u;
    out->scroll_writer_pc = 0x4184u;
    out->countdown_low_writer_pc = 0x4a7bu;
    out->countdown_high_writer_pc = 0x4a80u;
    out->first_scroll_frame = 8522u; out->presented_scroll_frame = 8580u;
    out->final_scroll_frame = 9666u; out->final_countdown_frame = 9668u;
    out->update_interval_frames = 8u; out->scroll_updates = 144u;
    out->countdown_updates = 144u; out->initial_byr = 0x00f0u;
    out->final_byr = 0x0060u; out->initial_countdown = 0x0090u;
    out->final_countdown = 0u;
    out->next_phase_start_frame = 10632u;
    out->next_phase_final_frame = 11274u;
    out->next_phase_reset_frame = 11578u;
    out->next_phase_interval_frames = 10u;
    out->next_phase_updates = 64u;
    out->next_phase_initial_countdown = 0x0040u;
    out->next_phase_final_countdown = 0u;
    out->next_phase_low_writer_pc = 0x4b1bu;
    out->next_phase_high_writer_pc = 0x4b20u;
    out->code_path_verified = 1;
    out->ram_snapshot_verified = 1; out->signed_scroll_loop_verified = 1;
    out->zero_stop_verified = 1; out->next_control_phase_verified = 1;
    out->screen_semantics_proven = 0;
    return 1;
}

int theron_v1_raw_loader_trace_import_game_owned_fifo_payload_file(
    const char *path, const uint8_t *track02_data, size_t track02_size,
    const char *track02_md5, Theron_V1RawLoaderTraceGamePayloadReceipt *out)
{
    FILE *file;
    long size;
    char *capture;
    int result;

    if (out) memset(out, 0, sizeof(*out));
    if (!path || !track02_data || !track02_md5 || !out ||
        !(file = fopen(path, "rb"))) {
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
    result = theron_v1_raw_loader_trace_bind_game_owned_fifo_payload(
        capture, track02_data, track02_size, track02_md5, out);
    free(capture);
    return result;
}

int theron_v1_raw_loader_trace_correlate_game_payload_initial_envelope(
    const Theron_V1RawLoaderTraceGamePayloadReceipt *payload,
    const uint8_t *track02_data, size_t track02_size,
    const char *track02_md5,
    Theron_V1RawLoaderTraceInitialEnvelopeByteReceipt *out)
{
    Theron_Track02InitialLevelObjectBoundaryReceipt boundary;
    size_t envelope_first_offset;
    size_t envelope_end_offset;

    if (out) memset(out, 0, sizeof(*out));
    if (!payload || !track02_data || !track02_md5 || !out || !payload->valid ||
        !payload->cdb_read6_verified || !payload->fifo_to_game_ram_verified ||
        !payload->game_ram_consumer_verified || payload->payload_semantics_proven ||
        strcmp(payload->track02_md5, track02_md5) != 0 ||
        track02_size % THERON_TRACK02_RAW_SECTOR_BYTES != 0u ||
        theron_v1_track02_capture_initial_level_object_boundary(
            track02_data, track02_size, track02_md5, &boundary) !=
            THERON_TRACK02_SIGNAL_OK || !boundary.valid ||
        boundary.object_table_parsed || boundary.object_table_semantics_proven ||
        !boundary.promotion_blocked || payload->variant != boundary.variant ||
        payload->raw_track02_record != boundary.level_first_raw_sector) {
        return 0;
    }

    envelope_first_offset = THERON_TRACK02_RAW_USER_DATA_OFFSET +
        boundary.level_user_data_offset_in_record;
    if (boundary.level_byte_count > SIZE_MAX - envelope_first_offset) return 0;
    envelope_end_offset = envelope_first_offset + boundary.level_byte_count;
    if (payload->source_offset < envelope_first_offset ||
        payload->source_offset >= envelope_end_offset ||
        payload->raw_track02_record >=
            track02_size / THERON_TRACK02_RAW_SECTOR_BYTES ||
        track02_data[(size_t)payload->raw_track02_record *
                         THERON_TRACK02_RAW_SECTOR_BYTES +
                     payload->source_offset] != payload->source_byte) {
        return 0;
    }

    out->valid = 1;
    out->variant = boundary.variant;
    snprintf(out->track02_md5, sizeof(out->track02_md5), "%s", track02_md5);
    out->track02_record = boundary.track02_record;
    out->raw_sector = boundary.level_first_raw_sector;
    out->raw_sector_offset = payload->source_offset;
    out->envelope_offset = payload->source_offset - envelope_first_offset;
    out->source_byte = payload->source_byte;
    out->game_payload_chain_verified = 1;
    out->source_envelope_overlap_verified = 1;
    out->level_semantics_proven = 0;
    return 1;
}

int theron_v1_raw_loader_trace_correlate_game_payload_initial_post_envelope(
    const Theron_V1RawLoaderTraceGamePayloadReceipt *payload,
    const uint8_t *track02_data, size_t track02_size,
    const char *track02_md5,
    Theron_V1RawLoaderTraceInitialPostEnvelopeByteReceipt *out)
{
    Theron_Track02InitialLevelObjectBoundaryReceipt boundary;
    size_t continuation_first_offset;
    size_t continuation_end_offset;

    if (out) memset(out, 0, sizeof(*out));
    if (!payload || !track02_data || !track02_md5 || !out || !payload->valid ||
        !payload->cdb_read6_verified || !payload->fifo_to_game_ram_verified ||
        !payload->game_ram_consumer_verified || payload->payload_semantics_proven ||
        strcmp(payload->track02_md5, track02_md5) != 0 ||
        track02_size % THERON_TRACK02_RAW_SECTOR_BYTES != 0u ||
        theron_v1_track02_capture_initial_level_object_boundary(
            track02_data, track02_size, track02_md5, &boundary) !=
            THERON_TRACK02_SIGNAL_OK || !boundary.valid ||
        boundary.object_table_parsed || boundary.object_table_semantics_proven ||
        !boundary.promotion_blocked || payload->variant != boundary.variant ||
        payload->raw_track02_record != boundary.level_first_raw_sector) {
        return 0;
    }

    continuation_first_offset = THERON_TRACK02_RAW_USER_DATA_OFFSET +
        boundary.object_boundary_user_data_offset_in_record;
    if (boundary.following_user_data_bytes_in_record >
        SIZE_MAX - continuation_first_offset) {
        return 0;
    }
    continuation_end_offset = continuation_first_offset +
        boundary.following_user_data_bytes_in_record;
    if (payload->source_offset < continuation_first_offset ||
        payload->source_offset >= continuation_end_offset ||
        payload->raw_track02_record >=
            track02_size / THERON_TRACK02_RAW_SECTOR_BYTES ||
        track02_data[(size_t)payload->raw_track02_record *
                         THERON_TRACK02_RAW_SECTOR_BYTES +
                     payload->source_offset] != payload->source_byte) {
        return 0;
    }

    out->valid = 1;
    out->variant = boundary.variant;
    snprintf(out->track02_md5, sizeof(out->track02_md5), "%s", track02_md5);
    out->track02_record = boundary.track02_record;
    out->raw_sector = boundary.level_first_raw_sector;
    out->raw_sector_offset = payload->source_offset;
    out->continuation_offset = payload->source_offset - continuation_first_offset;
    out->source_byte = payload->source_byte;
    out->game_payload_chain_verified = 1;
    out->source_continuation_overlap_verified = 1;
    out->object_table_semantics_proven = 0;
    return 1;
}

int theron_v1_raw_loader_trace_correlate_game_payload_initial_post_envelope_prefix(
    const Theron_V1RawLoaderTraceGamePayloadReceipt *payloads,
    size_t payload_count, const uint8_t *track02_data, size_t track02_size,
    const char *track02_md5,
    Theron_V1RawLoaderTraceInitialPostEnvelopePrefixReceipt *out)
{
    Theron_V1RawLoaderTraceInitialPostEnvelopeByteReceipt byte_receipt;
    size_t index;

    if (out) memset(out, 0, sizeof(*out));
    if (!payloads || !track02_data || !track02_md5 || !out ||
        payload_count != THERON_V1_RAW_LOADER_INITIAL_POST_ENVELOPE_PREFIX_BYTES) {
        return 0;
    }
    for (index = 0u; index < payload_count; ++index) {
        if (!theron_v1_raw_loader_trace_correlate_game_payload_initial_post_envelope(
                &payloads[index], track02_data, track02_size, track02_md5,
                &byte_receipt) || !byte_receipt.valid ||
            byte_receipt.continuation_offset != index) {
            return 0;
        }
        if (index == 0u) {
            out->variant = byte_receipt.variant;
            out->track02_record = byte_receipt.track02_record;
            out->raw_sector = byte_receipt.raw_sector;
            out->dispatch_sequence = payloads[index].dispatch_sequence;
            out->scsi_generation = payloads[index].scsi_generation;
            out->scsi_lba = payloads[index].scsi_lba;
            out->scsi_sector_count = payloads[index].scsi_sector_count;
        } else if (payloads[index].dispatch_sequence !=
                       out->dispatch_sequence ||
                   payloads[index].scsi_generation != out->scsi_generation ||
                   payloads[index].scsi_lba != out->scsi_lba ||
                   payloads[index].scsi_sector_count !=
                       out->scsi_sector_count) {
            return 0;
        }
        out->bytes[index] = byte_receipt.source_byte;
    }
    snprintf(out->track02_md5, sizeof(out->track02_md5), "%s", track02_md5);
    out->bytes_hash = tqr_trace_fnv1a_bytes(out->bytes, sizeof(out->bytes));
    if (!out->bytes_hash) return 0;
    out->valid = 1;
    out->contiguous_capture_chain_verified = 1;
    out->object_table_semantics_proven = 0;
    return 1;
}

int theron_v1_raw_loader_trace_bind_initial_post_envelope_tii_transfer(
    const Theron_V1RawLoaderTraceInitialLevelHandoffReceipt *handoff,
    const char *capture,
    Theron_V1RawLoaderTraceInitialPostEnvelopeTransferReceipt *out)
{
    const char *cursor;
    const char *line;
    size_t length;
    unsigned int transfer_pc;
    unsigned int transfer_physical_pc;
    unsigned int source;
    unsigned int destination;
    unsigned int byte_count;
    unsigned int matched = 0u;
    unsigned int matched_pc = 0u;
    unsigned int matched_physical_pc = 0u;
    unsigned int matched_destination = 0u;
    unsigned int matched_byte_count = 0u;
    int source_marker_seen = 0;
    int consumed;
    uint16_t expected_source;

    if (out) memset(out, 0, sizeof(*out));
    if (!handoff || !capture || !out ||
        !theron_v1_raw_loader_trace_manifest_initial_level_handoff_is_complete(
            handoff) ||
        handoff->loader_payload.destination > UINT16_MAX -
            handoff->loader_post_envelope.record_user_data_offset ||
        handoff->loader_post_envelope.byte_count == 0u) {
        return 0;
    }
    expected_source = (uint16_t)(handoff->loader_payload.destination +
        handoff->loader_post_envelope.record_user_data_offset);
    cursor = capture;
    while (tqr_trace_next_line(&cursor, &line, &length)) {
        if (length == strlen("source=mednafen-pce-instrumented-main-ram-loader") &&
            memcmp(line, "source=mednafen-pce-instrumented-main-ram-loader",
                   length) == 0) {
            source_marker_seen = 1;
            continue;
        }
        consumed = 0;
        if (sscanf(line,
                   "main_ram_loader_block_transfer logical_pc=%x physical_pc=%x operation=tii source=%x destination=%x length=%x%n",
                   &transfer_pc, &transfer_physical_pc, &source, &destination,
                   &byte_count, &consumed) != 5 || consumed != (int)length) {
            continue;
        }
        if (source != expected_source) continue;
        if (++matched != 1u || transfer_pc > UINT16_MAX ||
            transfer_physical_pc < 0x1f0000u ||
            transfer_physical_pc >= 0x1f8000u ||
            destination > UINT16_MAX || byte_count == 0u ||
            byte_count > handoff->loader_post_envelope.byte_count) {
            return 0;
        }
        /* Snapshot the accepted row: later parsed-but-rejected rows must not
         * overwrite the values reported in the receipt. */
        matched_pc = transfer_pc;
        matched_physical_pc = transfer_physical_pc;
        matched_destination = destination;
        matched_byte_count = byte_count;
    }
    if (!source_marker_seen || matched != 1u) return 0;

    out->valid = 1;
    out->variant = handoff->variant;
    snprintf(out->track02_md5, sizeof(out->track02_md5), "%s",
             handoff->track02_md5);
    out->track02_record = handoff->observed_track02_record;
    out->transfer_pc = (uint16_t)matched_pc;
    out->transfer_physical_pc = matched_physical_pc;
    out->source_address = expected_source;
    out->destination_address = (uint16_t)matched_destination;
    out->byte_count = matched_byte_count;
    out->source_checksum = tqr_trace_fnv1a_bytes(
        handoff->loader_post_envelope.bytes, matched_byte_count);
    if (!out->source_checksum) {
        memset(out, 0, sizeof(*out));
        return 0;
    }
    out->manifest_bound = 1;
    out->source_continuation_transfer_verified = 1;
    out->object_table_semantics_proven = 0;
    return 1;
}

int theron_v1_raw_loader_trace_import_initial_post_envelope_tii_transfer_file(
    const Theron_V1RawLoaderTraceInitialLevelHandoffReceipt *handoff,
    const char *path,
    Theron_V1RawLoaderTraceInitialPostEnvelopeTransferReceipt *out)
{
    FILE *file;
    long size;
    char *capture;
    int result;

    if (out) memset(out, 0, sizeof(*out));
    if (!handoff || !path || !out || !(file = fopen(path, "rb"))) {
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
    result = theron_v1_raw_loader_trace_bind_initial_post_envelope_tii_transfer(
        handoff, capture, out);
    free(capture);
    return result;
}

int theron_v1_raw_loader_trace_bind_initial_post_envelope_execution(
    const Theron_V1RawLoaderTraceInitialLevelHandoffReceipt *handoff,
    const char *capture,
    Theron_V1RawLoaderTraceInitialPostEnvelopeExecutionReceipt *out)
{
    Theron_V1RawLoaderTraceInitialPostEnvelopeTransferReceipt transfer;
    const char *cursor;
    const char *line;
    size_t length;
    unsigned int logical_pc;
    unsigned int physical_pc;
    unsigned int target;
    unsigned int return_instruction_pc;
    unsigned int return_instruction_physical_pc;
    unsigned int post_return_source_pc;
    unsigned int post_return_source_physical_pc;
    unsigned int post_return_pc;
    unsigned int post_return_physical_pc;
    unsigned int post_return_opcode;
    int consumed;
    int transfer_seen = 0;
    unsigned int matching_call_count = 0u;
    unsigned int matching_return_count = 0u;
    unsigned int matching_post_return_count = 0u;

    if (out) memset(out, 0, sizeof(*out));
    if (!handoff || !capture || !out ||
        !theron_v1_raw_loader_trace_bind_initial_post_envelope_tii_transfer(
            handoff, capture, &transfer)) {
        return 0;
    }
    cursor = capture;
    while (tqr_trace_next_line(&cursor, &line, &length)) {
        {
            unsigned int source;
            unsigned int destination;
            unsigned int byte_count;
            consumed = 0;
            if (sscanf(line,
                       "main_ram_loader_block_transfer logical_pc=%x physical_pc=%x operation=tii source=%x destination=%x length=%x%n",
                       &logical_pc, &physical_pc, &source, &destination,
                       &byte_count, &consumed) == 5 &&
                consumed == (int)length && source == transfer.source_address &&
                destination == transfer.destination_address &&
                byte_count == transfer.byte_count) {
                transfer_seen = 1;
                continue;
            }
        }
        if (!transfer_seen) continue;
        consumed = 0;
        if (sscanf(line,
                   "main_ram_loader_jsr logical_pc=%x physical_pc=%x target=%x a=%*x x=%*x y=%*x%n",
                   &logical_pc, &physical_pc, &target, &consumed) == 3 &&
            consumed == (int)length && target == transfer.destination_address) {
            if (++matching_call_count != 1u || logical_pc > UINT16_MAX ||
                physical_pc < 0x1f0000u || physical_pc >= 0x1f8000u) {
                return 0;
            }
            out->call_pc = (uint16_t)logical_pc;
            out->call_physical_pc = physical_pc;
            out->call_target = (uint16_t)target;
            continue;
        }
        if (matching_call_count == 0u) continue;
        consumed = 0;
        if (sscanf(line,
                   "main_ram_loader_rts logical_pc=%x physical_pc=%x%n",
                   &return_instruction_pc, &return_instruction_physical_pc,
                   &consumed) == 2 && consumed == (int)length) {
            if (return_instruction_pc > UINT16_MAX ||
                return_instruction_physical_pc < 0x1f0000u ||
                return_instruction_physical_pc >= 0x1f8000u) {
                return 0;
            }
            if (return_instruction_pc < transfer.destination_address ||
                (size_t)(return_instruction_pc - transfer.destination_address) >=
                    transfer.byte_count) {
                /* An RTS outside the copied continuation span belongs to a
                 * later routine; it is not the continuation terminator and
                 * remains an opaque row. */
                continue;
            }
            if (++matching_return_count != 1u) {
                return 0;
            }
            out->return_instruction_pc = (uint16_t)return_instruction_pc;
            out->return_instruction_physical_pc = return_instruction_physical_pc;
            continue;
        }
        if (matching_return_count == 0u) continue;
        consumed = 0;
        if (sscanf(line,
                   "main_ram_loader_post_rts source_logical_pc=%x source_physical_pc=%x logical_pc=%x physical_pc=%x opcode=%x%n",
                   &post_return_source_pc, &post_return_source_physical_pc,
                   &post_return_pc, &post_return_physical_pc,
                   &post_return_opcode, &consumed) != 5 ||
            consumed != (int)length) {
            continue;
        }
        if (post_return_source_pc != out->return_instruction_pc ||
            post_return_source_physical_pc != out->return_instruction_physical_pc) {
            /* A post-RTS resume of a different routine is not the
             * continuation return and remains an opaque row. */
            continue;
        }
        if (++matching_post_return_count != 1u ||
            post_return_pc != (unsigned int)out->call_pc + 3u ||
            post_return_pc > UINT16_MAX ||
            post_return_physical_pc < 0x1f0000u ||
            post_return_physical_pc >= 0x1f8000u ||
            post_return_opcode > UINT8_MAX) {
            return 0;
        }
        out->post_return_pc = (uint16_t)post_return_pc;
        out->post_return_physical_pc = post_return_physical_pc;
        out->post_return_opcode = (uint8_t)post_return_opcode;
    }
    if (!transfer_seen || matching_call_count != 1u ||
        matching_return_count != 1u || matching_post_return_count != 1u) {
        return 0;
    }
    out->valid = 1;
    out->transfer = transfer;
    out->continuation_execution_proven = 1;
    out->continuation_termination_instruction_proven = 1;
    out->continuation_post_return_target_proven = 1;
    out->level_or_object_semantics_proven = 0;
    return 1;
}

int theron_v1_raw_loader_trace_bind_initial_post_envelope_post_return_call(
    const Theron_V1RawLoaderTraceInitialLevelHandoffReceipt *handoff,
    const char *capture,
    Theron_V1RawLoaderTraceInitialPostEnvelopePostReturnCallReceipt *out)
{
    Theron_V1RawLoaderTraceInitialPostEnvelopeExecutionReceipt execution;
    const char *cursor;
    const char *line;
    size_t length;
    unsigned int source_pc;
    unsigned int source_physical_pc;
    unsigned int logical_pc;
    unsigned int physical_pc;
    unsigned int opcode;
    unsigned int target;
    unsigned int post_return_count = 0u;
    int consumed;

    if (out) memset(out, 0, sizeof(*out));
    if (!handoff || !capture || !out ||
        !theron_v1_raw_loader_trace_bind_initial_post_envelope_execution(
            handoff, capture, &execution) ||
        !execution.valid || !execution.continuation_execution_proven ||
        !execution.continuation_post_return_target_proven ||
        execution.post_return_opcode != 0x20u) {
        return 0;
    }
    cursor = capture;
    while (tqr_trace_next_line(&cursor, &line, &length)) {
        consumed = 0;
        if (sscanf(line,
                   "main_ram_loader_post_rts source_logical_pc=%x source_physical_pc=%x logical_pc=%x physical_pc=%x opcode=%x%n",
                   &source_pc, &source_physical_pc, &logical_pc, &physical_pc,
                   &opcode, &consumed) == 5 && consumed == (int)length &&
            source_pc == execution.return_instruction_pc &&
            source_physical_pc == execution.return_instruction_physical_pc &&
            logical_pc == execution.post_return_pc &&
            physical_pc == execution.post_return_physical_pc &&
            opcode == execution.post_return_opcode) {
            if (++post_return_count != 1u ||
                !tqr_trace_next_line(&cursor, &line, &length)) {
                return 0;
            }
            consumed = 0;
            if (sscanf(line,
                       "main_ram_loader_jsr logical_pc=%x physical_pc=%x target=%x a=%*x x=%*x y=%*x%n",
                       &logical_pc, &physical_pc, &target, &consumed) != 3 ||
                consumed != (int)length ||
                logical_pc != execution.post_return_pc ||
                physical_pc != execution.post_return_physical_pc ||
                target > UINT16_MAX) {
                return 0;
            }
            out->valid = 1;
            out->execution = execution;
            out->call_pc = (uint16_t)logical_pc;
            out->call_physical_pc = physical_pc;
            out->call_target = (uint16_t)target;
            out->post_return_call_proven = 1;
            out->level_or_object_semantics_proven = 0;
            return 1;
        }
    }
    return 0;
}

int theron_v1_raw_loader_trace_bind_initial_post_envelope_post_return_call_termination(
    const Theron_V1RawLoaderTraceInitialLevelHandoffReceipt *handoff,
    const char *capture,
    Theron_V1RawLoaderTraceInitialPostEnvelopePostReturnCallTerminationReceipt *out)
{
    Theron_V1RawLoaderTraceInitialPostEnvelopePostReturnCallReceipt call;
    const char *cursor;
    const char *line;
    size_t length;
    unsigned int logical_pc;
    unsigned int physical_pc;
    unsigned int target;
    unsigned int return_instruction_pc = 0u;
    unsigned int return_instruction_physical_pc = 0u;
    unsigned int source_pc;
    unsigned int source_physical_pc;
    unsigned int opcode;
    unsigned int matching_return_count = 0u;
    int call_seen = 0;
    int return_pending = 0;
    int consumed;

    if (out) memset(out, 0, sizeof(*out));
    if (!handoff || !capture || !out ||
        !theron_v1_raw_loader_trace_bind_initial_post_envelope_post_return_call(
            handoff, capture, &call) || !call.valid ||
        !call.post_return_call_proven) {
        return 0;
    }
    cursor = capture;
    while (tqr_trace_next_line(&cursor, &line, &length)) {
        if (!call_seen) {
            consumed = 0;
            if (sscanf(line,
                       "main_ram_loader_jsr logical_pc=%x physical_pc=%x target=%x a=%*x x=%*x y=%*x%n",
                       &logical_pc, &physical_pc, &target, &consumed) == 3 &&
                consumed == (int)length && logical_pc == call.call_pc &&
                physical_pc == call.call_physical_pc && target == call.call_target) {
                call_seen = 1;
            }
            continue;
        }
        if (return_pending) {
            consumed = 0;
            if (sscanf(line,
                       "main_ram_loader_post_rts source_logical_pc=%x source_physical_pc=%x logical_pc=%x physical_pc=%x opcode=%x%n",
                       &source_pc, &source_physical_pc, &logical_pc, &physical_pc,
                       &opcode, &consumed) == 5 && consumed == (int)length &&
                source_pc == return_instruction_pc &&
                source_physical_pc == return_instruction_physical_pc) {
                return_pending = 0;
                if (logical_pc == (unsigned int)call.call_pc + 3u &&
                    logical_pc <= UINT16_MAX && physical_pc >= 0x1f0000u &&
                    physical_pc < 0x1f8000u && opcode <= UINT8_MAX) {
                    if (++matching_return_count != 1u) return 0;
                    out->return_instruction_pc =
                        (uint16_t)return_instruction_pc;
                    out->return_instruction_physical_pc =
                        return_instruction_physical_pc;
                    out->post_return_pc = (uint16_t)logical_pc;
                    out->post_return_physical_pc = physical_pc;
                    out->post_return_opcode = (uint8_t)opcode;
                }
                continue;
            }
            return_pending = 0;
        }
        consumed = 0;
        if (sscanf(line,
                   "main_ram_loader_rts logical_pc=%x physical_pc=%x%n",
                   &return_instruction_pc, &return_instruction_physical_pc,
                   &consumed) == 2 && consumed == (int)length &&
            return_instruction_pc <= UINT16_MAX &&
            return_instruction_physical_pc >= 0x1f0000u &&
            return_instruction_physical_pc < 0x1f8000u) {
            return_pending = 1;
        }
    }
    if (!call_seen || matching_return_count != 1u) return 0;
    out->valid = 1;
    out->call = call;
    out->post_return_call_termination_proven = 1;
    out->post_return_call_return_proven = 1;
    out->level_or_object_semantics_proven = 0;
    return 1;
}

int theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_call(
    const Theron_V1RawLoaderTraceInitialLevelHandoffReceipt *handoff,
    const char *capture,
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextCallReceipt *out)
{
    Theron_V1RawLoaderTraceInitialPostEnvelopePostReturnCallTerminationReceipt
        termination;
    const char *cursor;
    const char *line;
    size_t length;
    unsigned int source_pc;
    unsigned int source_physical_pc;
    unsigned int logical_pc;
    unsigned int physical_pc;
    unsigned int opcode;
    unsigned int target;
    int caller_resumed = 0;
    int consumed;

    if (out) memset(out, 0, sizeof(*out));
    if (!handoff || !capture || !out ||
        !theron_v1_raw_loader_trace_bind_initial_post_envelope_post_return_call_termination(
            handoff, capture, &termination) || !termination.valid ||
        !termination.post_return_call_return_proven) {
        return 0;
    }
    cursor = capture;
    while (tqr_trace_next_line(&cursor, &line, &length)) {
        if (!caller_resumed) {
            consumed = 0;
            if (sscanf(line,
                       "main_ram_loader_post_rts source_logical_pc=%x source_physical_pc=%x logical_pc=%x physical_pc=%x opcode=%x%n",
                       &source_pc, &source_physical_pc, &logical_pc, &physical_pc,
                       &opcode, &consumed) == 5 && consumed == (int)length &&
                source_pc == termination.return_instruction_pc &&
                source_physical_pc == termination.return_instruction_physical_pc &&
                logical_pc == termination.post_return_pc &&
                physical_pc == termination.post_return_physical_pc &&
                opcode == termination.post_return_opcode) {
                caller_resumed = 1;
            }
            continue;
        }
        consumed = 0;
        if (sscanf(line,
                   "main_ram_loader_jsr logical_pc=%x physical_pc=%x target=%x a=%*x x=%*x y=%*x%n",
                   &logical_pc, &physical_pc, &target, &consumed) != 3 ||
            consumed != (int)length) {
            continue;
        }
        if (logical_pc > UINT16_MAX || physical_pc < 0x1f0000u ||
            physical_pc >= 0x1f8000u || target > UINT16_MAX) {
            return 0;
        }
        out->valid = 1;
        out->termination = termination;
        out->call_pc = (uint16_t)logical_pc;
        out->call_physical_pc = physical_pc;
        out->call_target = (uint16_t)target;
        out->caller_next_call_proven = 1;
        out->level_or_object_semantics_proven = 0;
        return 1;
    }
    return 0;
}

int theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_call_entry(
    const Theron_V1RawLoaderTraceInitialLevelHandoffReceipt *handoff,
    const char *capture,
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextCallEntryReceipt *out)
{
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextCallReceipt call;
    const char *cursor;
    const char *line;
    size_t length;
    unsigned int caller_pc;
    unsigned int caller_physical_pc;
    unsigned int target;
    unsigned int entry_pc;
    unsigned int entry_physical_pc;
    unsigned int entry_opcode;
    int call_seen = 0;
    int consumed;

    if (out) memset(out, 0, sizeof(*out));
    if (!handoff || !capture || !out ||
        !theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_call(
            handoff, capture, &call) || !call.valid ||
        !call.caller_next_call_proven) {
        return 0;
    }
    cursor = capture;
    while (tqr_trace_next_line(&cursor, &line, &length)) {
        if (!call_seen) {
            consumed = 0;
            if (sscanf(line,
                       "main_ram_loader_jsr logical_pc=%x physical_pc=%x target=%x a=%*x x=%*x y=%*x%n",
                       &caller_pc, &caller_physical_pc, &target, &consumed) == 3 &&
                consumed == (int)length && caller_pc == call.call_pc &&
                caller_physical_pc == call.call_physical_pc &&
                target == call.call_target) {
                call_seen = 1;
            }
            continue;
        }
        consumed = 0;
        if (sscanf(line,
                   "main_ram_loader_call_entry caller_logical_pc=%x caller_physical_pc=%x target=%x logical_pc=%x physical_pc=%x opcode=%x%n",
                   &caller_pc, &caller_physical_pc, &target, &entry_pc,
                   &entry_physical_pc, &entry_opcode, &consumed) != 6 ||
            consumed != (int)length || caller_pc != call.call_pc ||
            caller_physical_pc != call.call_physical_pc ||
            target != call.call_target || entry_pc != call.call_target ||
            entry_pc > UINT16_MAX || entry_physical_pc < 0x1f0000u ||
            entry_physical_pc >= 0x1f8000u || entry_opcode > UINT8_MAX) {
            return 0;
        }
        out->valid = 1;
        out->call = call;
        out->entry_pc = (uint16_t)entry_pc;
        out->entry_physical_pc = entry_physical_pc;
        out->entry_opcode = (uint8_t)entry_opcode;
        out->caller_next_call_entry_proven = 1;
        out->level_or_object_semantics_proven = 0;
        return 1;
    }
    return 0;
}

int theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_entry_next(
    const Theron_V1RawLoaderTraceInitialLevelHandoffReceipt *handoff,
    const char *capture,
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextEntryNextReceipt *out)
{
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextCallEntryReceipt entry;
    const char *cursor;
    const char *line;
    size_t length;
    unsigned int caller_pc;
    unsigned int caller_physical_pc;
    unsigned int target;
    unsigned int entry_pc;
    unsigned int entry_physical_pc;
    unsigned int entry_opcode;
    unsigned int next_pc;
    unsigned int next_physical_pc;
    unsigned int next_opcode;
    int entry_seen = 0;
    int consumed;

    if (out) memset(out, 0, sizeof(*out));
    if (!handoff || !capture || !out ||
        !theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_call_entry(
            handoff, capture, &entry) || !entry.valid ||
        !entry.caller_next_call_entry_proven) {
        return 0;
    }
    cursor = capture;
    while (tqr_trace_next_line(&cursor, &line, &length)) {
        if (!entry_seen) {
            consumed = 0;
            if (sscanf(line,
                       "main_ram_loader_call_entry caller_logical_pc=%x caller_physical_pc=%x target=%x logical_pc=%x physical_pc=%x opcode=%x%n",
                       &caller_pc, &caller_physical_pc, &target, &entry_pc,
                       &entry_physical_pc, &entry_opcode, &consumed) == 6 &&
                consumed == (int)length &&
                caller_pc == entry.call.call_pc &&
                caller_physical_pc == entry.call.call_physical_pc &&
                target == entry.call.call_target && entry_pc == entry.entry_pc &&
                entry_physical_pc == entry.entry_physical_pc &&
                entry_opcode == entry.entry_opcode) {
                entry_seen = 1;
            }
            continue;
        }
        consumed = 0;
        if (sscanf(line,
                   "main_ram_loader_entry_next entry_logical_pc=%x entry_physical_pc=%x logical_pc=%x physical_pc=%x opcode=%x%n",
                   &entry_pc, &entry_physical_pc, &next_pc, &next_physical_pc,
                   &next_opcode, &consumed) != 5 || consumed != (int)length ||
            entry_pc != entry.entry_pc ||
            entry_physical_pc != entry.entry_physical_pc ||
            next_pc > UINT16_MAX || next_physical_pc < 0x1f0000u ||
            next_physical_pc >= 0x1f8000u || next_opcode > UINT8_MAX) {
            return 0;
        }
        out->valid = 1;
        out->entry = entry;
        out->next_pc = (uint16_t)next_pc;
        out->next_physical_pc = next_physical_pc;
        out->next_opcode = (uint8_t)next_opcode;
        out->caller_next_entry_next_instruction_proven = 1;
        out->level_or_object_semantics_proven = 0;
        return 1;
    }
    return 0;
}

int theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer(
    const Theron_V1RawLoaderTraceInitialLevelHandoffReceipt *handoff,
    const char *capture,
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferReceipt *out)
{
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextEntryNextReceipt next;
    const char *cursor;
    const char *line;
    size_t length;
    unsigned int entry_pc;
    unsigned int entry_physical_pc;
    unsigned int next_pc;
    unsigned int next_physical_pc;
    unsigned int next_opcode;
    unsigned int transfer_pc;
    unsigned int transfer_physical_pc;
    unsigned int source;
    unsigned int destination;
    unsigned int byte_count;
    size_t source_offset;
    int successor_seen = 0;
    int consumed;

    if (out) memset(out, 0, sizeof(*out));
    if (!handoff || !capture || !out ||
        !theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_entry_next(
            handoff, capture, &next) || !next.valid ||
        !next.caller_next_entry_next_instruction_proven ||
        next.next_opcode != 0x73u) {
        return 0;
    }
    cursor = capture;
    while (tqr_trace_next_line(&cursor, &line, &length)) {
        if (!successor_seen) {
            consumed = 0;
            if (sscanf(line,
                       "main_ram_loader_entry_next entry_logical_pc=%x entry_physical_pc=%x logical_pc=%x physical_pc=%x opcode=%x%n",
                       &entry_pc, &entry_physical_pc, &next_pc, &next_physical_pc,
                       &next_opcode, &consumed) == 5 && consumed == (int)length &&
                entry_pc == next.entry.entry_pc &&
                entry_physical_pc == next.entry.entry_physical_pc &&
                next_pc == next.next_pc &&
                next_physical_pc == next.next_physical_pc &&
                next_opcode == next.next_opcode) {
                successor_seen = 1;
            }
            continue;
        }
        consumed = 0;
        if (sscanf(line,
                   "main_ram_loader_block_transfer logical_pc=%x physical_pc=%x operation=tii source=%x destination=%x length=%x%n",
                   &transfer_pc, &transfer_physical_pc, &source, &destination,
                   &byte_count, &consumed) != 5 || consumed != (int)length ||
            transfer_pc != next.next_pc ||
            transfer_physical_pc != next.next_physical_pc ||
            source > UINT16_MAX || destination > UINT16_MAX || byte_count == 0u ||
            transfer_physical_pc < 0x1f0000u ||
            transfer_physical_pc >= 0x1f8000u ||
            source < next.entry.call.termination.call.execution.transfer.destination_address) {
            return 0;
        }
        source_offset = source -
            next.entry.call.termination.call.execution.transfer.destination_address;
        if (source_offset >= next.entry.call.termination.call.execution.transfer.byte_count ||
            byte_count > next.entry.call.termination.call.execution.transfer.byte_count -
                source_offset ||
            next.entry.call.termination.call.execution.transfer.source_address >
                UINT16_MAX - source_offset) {
            return 0;
        }
        out->valid = 1;
        out->next = next;
        out->transfer_pc = (uint16_t)transfer_pc;
        out->transfer_physical_pc = transfer_physical_pc;
        out->source_address = (uint16_t)source;
        out->destination_address = (uint16_t)destination;
        out->byte_count = byte_count;
        out->original_source_address = (uint16_t)(
            next.entry.call.termination.call.execution.transfer.source_address +
            source_offset);
        out->source_checksum = tqr_trace_fnv1a_bytes(
            handoff->loader_post_envelope.bytes + source_offset, byte_count);
        if (!out->source_checksum) {
            memset(out, 0, sizeof(*out));
            return 0;
        }
        out->source_track02_bytes_proven = 1;
        out->level_or_object_semantics_proven = 0;
        return 1;
    }
    return 0;
}

int theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call(
    const Theron_V1RawLoaderTraceInitialLevelHandoffReceipt *handoff,
    const char *capture,
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallReceipt *out)
{
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferReceipt transfer;
    const char *cursor;
    const char *line;
    size_t length;
    unsigned int transfer_pc;
    unsigned int transfer_physical_pc;
    unsigned int source;
    unsigned int destination;
    unsigned int byte_count;
    unsigned int call_pc;
    unsigned int call_physical_pc;
    unsigned int call_target;
    int transfer_seen = 0;
    int consumed;

    if (out) memset(out, 0, sizeof(*out));
    if (!handoff || !capture || !out ||
        !theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer(
            handoff, capture, &transfer) || !transfer.valid ||
        !transfer.source_track02_bytes_proven) {
        return 0;
    }
    cursor = capture;
    while (tqr_trace_next_line(&cursor, &line, &length)) {
        if (!transfer_seen) {
            consumed = 0;
            if (sscanf(line,
                       "main_ram_loader_block_transfer logical_pc=%x physical_pc=%x operation=tii source=%x destination=%x length=%x%n",
                       &transfer_pc, &transfer_physical_pc, &source, &destination,
                       &byte_count, &consumed) == 5 && consumed == (int)length &&
                transfer_pc == transfer.transfer_pc &&
                transfer_physical_pc == transfer.transfer_physical_pc &&
                source == transfer.source_address &&
                destination == transfer.destination_address &&
                byte_count == transfer.byte_count) {
                transfer_seen = 1;
            }
            continue;
        }
        consumed = 0;
        if (sscanf(line,
                   "main_ram_loader_jsr logical_pc=%x physical_pc=%x target=%x a=%*x x=%*x y=%*x%n",
                   &call_pc, &call_physical_pc, &call_target, &consumed) != 3 ||
            consumed != (int)length) {
            continue;
        }
        if (call_target != transfer.destination_address || call_pc > UINT16_MAX ||
            call_physical_pc < 0x1f0000u || call_physical_pc >= 0x1f8000u) {
            return 0;
        }
        out->valid = 1;
        out->transfer = transfer;
        out->call_pc = (uint16_t)call_pc;
        out->call_physical_pc = call_physical_pc;
        out->call_target = (uint16_t)call_target;
        out->transfer_destination_call_proven = 1;
        out->level_or_object_semantics_proven = 0;
        return 1;
    }
    return 0;
}

int theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry(
    const Theron_V1RawLoaderTraceInitialLevelHandoffReceipt *handoff,
    const char *capture,
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryReceipt *out)
{
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallReceipt
        call;
    const char *cursor;
    const char *line;
    size_t length;
    unsigned int caller_pc;
    unsigned int caller_physical_pc;
    unsigned int target;
    unsigned int entry_pc;
    unsigned int entry_physical_pc;
    unsigned int entry_opcode;
    int call_seen = 0;
    int consumed;

    if (out) memset(out, 0, sizeof(*out));
    if (!handoff || !capture || !out ||
        !theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call(
            handoff, capture, &call) || !call.valid ||
        !call.transfer_destination_call_proven) {
        return 0;
    }
    cursor = capture;
    while (tqr_trace_next_line(&cursor, &line, &length)) {
        if (!call_seen) {
            consumed = 0;
            if (sscanf(line,
                       "main_ram_loader_jsr logical_pc=%x physical_pc=%x target=%x a=%*x x=%*x y=%*x%n",
                       &caller_pc, &caller_physical_pc, &target, &consumed) == 3 &&
                consumed == (int)length && caller_pc == call.call_pc &&
                caller_physical_pc == call.call_physical_pc &&
                target == call.call_target) {
                call_seen = 1;
            }
            continue;
        }
        consumed = 0;
        if (sscanf(line,
                   "main_ram_loader_call_entry caller_logical_pc=%x caller_physical_pc=%x target=%x logical_pc=%x physical_pc=%x opcode=%x%n",
                   &caller_pc, &caller_physical_pc, &target, &entry_pc,
                   &entry_physical_pc, &entry_opcode, &consumed) != 6 ||
            consumed != (int)length || caller_pc != call.call_pc ||
            caller_physical_pc != call.call_physical_pc ||
            target != call.call_target || entry_pc != call.call_target ||
            entry_pc > UINT16_MAX || entry_physical_pc < 0x1f0000u ||
            entry_physical_pc >= 0x1f8000u || entry_opcode > UINT8_MAX) {
            return 0;
        }
        out->valid = 1;
        out->call = call;
        out->entry_pc = (uint16_t)entry_pc;
        out->entry_physical_pc = entry_physical_pc;
        out->entry_opcode = (uint8_t)entry_opcode;
        out->transfer_destination_call_entry_proven = 1;
        out->level_or_object_semantics_proven = 0;
        return 1;
    }
    return 0;
}

int theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry_copy(
    const Theron_V1RawLoaderTraceInitialLevelHandoffReceipt *handoff,
    const char *capture,
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryCopyReceipt *out)
{
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryReceipt
        entry;
    const Theron_V1RawLoaderTraceInitialPostEnvelopeTransferReceipt *parent;
    size_t source_offset;
    uint8_t source_byte;

    if (out) memset(out, 0, sizeof(*out));
    if (!handoff || !capture || !out ||
        !theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry(
            handoff, capture, &entry) || !entry.valid ||
        !entry.transfer_destination_call_entry_proven ||
        !entry.call.transfer.source_track02_bytes_proven ||
        entry.entry_pc != entry.call.transfer.destination_address ||
        entry.call.transfer.byte_count == 0u) {
        return 0;
    }
    parent = &entry.call.transfer.next.entry.call.termination.call.execution.transfer;
    if (entry.call.transfer.source_address < parent->destination_address) {
        return 0;
    }
    source_offset = entry.call.transfer.source_address - parent->destination_address;
    if (source_offset >= parent->byte_count ||
        entry.call.transfer.original_source_address !=
            parent->source_address + source_offset) {
        return 0;
    }
    source_byte = handoff->loader_post_envelope.bytes[source_offset];
    if (entry.entry_opcode != source_byte) {
        return 0;
    }
    out->valid = 1;
    out->entry = entry;
    out->copied_source_address = entry.call.transfer.source_address;
    out->original_source_address = entry.call.transfer.original_source_address;
    out->copied_source_byte = source_byte;
    out->copied_source_byte_proven = 1;
    out->level_or_object_semantics_proven = 0;
    return 1;
}

int theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry_copy_next(
    const Theron_V1RawLoaderTraceInitialLevelHandoffReceipt *handoff,
    const char *capture,
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryCopyNextReceipt *out)
{
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryCopyReceipt
        entry_copy;
    const Theron_V1RawLoaderTraceInitialPostEnvelopeTransferReceipt *parent;
    const char *cursor;
    const char *line;
    size_t length;
    size_t copy_source_offset;
    size_t next_copy_offset;
    size_t source_offset;
    unsigned int entry_pc;
    unsigned int entry_physical_pc;
    unsigned int next_pc;
    unsigned int next_physical_pc;
    unsigned int next_opcode;
    int consumed;

    if (out) memset(out, 0, sizeof(*out));
    if (!handoff || !capture || !out ||
        !theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry_copy(
            handoff, capture, &entry_copy) || !entry_copy.valid ||
        !entry_copy.copied_source_byte_proven) {
        return 0;
    }
    cursor = capture;
    while (tqr_trace_next_line(&cursor, &line, &length)) {
        consumed = 0;
        if (sscanf(line,
                   "main_ram_loader_entry_next entry_logical_pc=%x entry_physical_pc=%x logical_pc=%x physical_pc=%x opcode=%x%n",
                   &entry_pc, &entry_physical_pc, &next_pc, &next_physical_pc,
                   &next_opcode, &consumed) != 5 || consumed != (int)length ||
            entry_pc != entry_copy.entry.entry_pc ||
            entry_physical_pc != entry_copy.entry.entry_physical_pc) {
            continue;
        }
        if (next_pc > UINT16_MAX || next_opcode > UINT8_MAX ||
            next_physical_pc < 0x1f0000u || next_physical_pc >= 0x1f8000u ||
            entry_copy.entry.call.transfer.destination_address >
                UINT16_MAX - entry_copy.entry.call.transfer.byte_count ||
            next_pc < entry_copy.entry.call.transfer.destination_address ||
            next_pc >= entry_copy.entry.call.transfer.destination_address +
                entry_copy.entry.call.transfer.byte_count) {
            return 0;
        }
        parent = &entry_copy.entry.call.transfer.next.entry.call.termination.call.execution.transfer;
        if (entry_copy.entry.call.transfer.source_address <
            parent->destination_address) {
            return 0;
        }
        copy_source_offset = entry_copy.entry.call.transfer.source_address -
            parent->destination_address;
        next_copy_offset = next_pc -
            entry_copy.entry.call.transfer.destination_address;
        source_offset = copy_source_offset + next_copy_offset;
        if (copy_source_offset >= parent->byte_count ||
            next_copy_offset >= entry_copy.entry.call.transfer.byte_count ||
            source_offset >= parent->byte_count ||
            parent->source_address > UINT16_MAX - source_offset ||
            next_opcode != handoff->loader_post_envelope.bytes[source_offset]) {
            return 0;
        }
        out->valid = 1;
        out->entry_copy = entry_copy;
        out->next_pc = (uint16_t)next_pc;
        out->next_physical_pc = next_physical_pc;
        out->original_source_address = (uint16_t)(parent->source_address +
                                                  source_offset);
        out->next_source_byte = (uint8_t)next_opcode;
        out->copied_successor_byte_proven = 1;
        out->level_or_object_semantics_proven = 0;
        return 1;
    }
    return 0;
}

int theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry_copy_successor(
    const Theron_V1RawLoaderTraceInitialLevelHandoffReceipt *handoff,
    const char *capture,
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryCopySuccessorReceipt *out)
{
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryCopyNextReceipt
        successor;
    const Theron_V1RawLoaderTraceInitialPostEnvelopeTransferReceipt *parent;
    const char *cursor;
    const char *line;
    size_t length;
    size_t copy_source_offset;
    size_t next_copy_offset;
    size_t source_offset;
    unsigned int successor_pc;
    unsigned int successor_physical_pc;
    unsigned int next_pc;
    unsigned int next_physical_pc;
    unsigned int next_opcode;
    int consumed;

    if (out) memset(out, 0, sizeof(*out));
    if (!handoff || !capture || !out ||
        !theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry_copy_next(
            handoff, capture, &successor) || !successor.valid ||
        !successor.copied_successor_byte_proven) {
        return 0;
    }
    cursor = capture;
    while (tqr_trace_next_line(&cursor, &line, &length)) {
        consumed = 0;
        if (sscanf(line,
                   "main_ram_loader_entry_successor_next successor_logical_pc=%x successor_physical_pc=%x logical_pc=%x physical_pc=%x opcode=%x%n",
                   &successor_pc, &successor_physical_pc, &next_pc,
                   &next_physical_pc, &next_opcode, &consumed) != 5 ||
            consumed != (int)length || successor_pc != successor.next_pc ||
            successor_physical_pc != successor.next_physical_pc) {
            continue;
        }
        if (next_pc > UINT16_MAX || next_opcode > UINT8_MAX ||
            next_physical_pc < 0x1f0000u || next_physical_pc >= 0x1f8000u ||
            successor.entry_copy.entry.call.transfer.destination_address >
                UINT16_MAX - successor.entry_copy.entry.call.transfer.byte_count ||
            next_pc < successor.entry_copy.entry.call.transfer.destination_address ||
            next_pc >= successor.entry_copy.entry.call.transfer.destination_address +
                successor.entry_copy.entry.call.transfer.byte_count) {
            return 0;
        }
        parent = &successor.entry_copy.entry.call.transfer.next.entry.call.termination.call.execution.transfer;
        if (successor.entry_copy.entry.call.transfer.source_address <
            parent->destination_address) {
            return 0;
        }
        copy_source_offset =
            successor.entry_copy.entry.call.transfer.source_address -
            parent->destination_address;
        next_copy_offset = next_pc -
            successor.entry_copy.entry.call.transfer.destination_address;
        if (copy_source_offset >= parent->byte_count ||
            next_copy_offset >= successor.entry_copy.entry.call.transfer.byte_count ||
            copy_source_offset > parent->byte_count - next_copy_offset) {
            return 0;
        }
        source_offset = copy_source_offset + next_copy_offset;
        if (source_offset >= parent->byte_count ||
            parent->source_address > UINT16_MAX - source_offset ||
            next_opcode != handoff->loader_post_envelope.bytes[source_offset]) {
            return 0;
        }
        out->valid = 1;
        out->successor = successor;
        out->next_pc = (uint16_t)next_pc;
        out->next_physical_pc = next_physical_pc;
        out->original_source_address = (uint16_t)(parent->source_address +
                                                  source_offset);
        out->next_source_byte = (uint8_t)next_opcode;
        out->copied_successor_next_byte_proven = 1;
        out->level_or_object_semantics_proven = 0;
        return 1;
    }
    return 0;
}

int theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry_branch(
    const Theron_V1RawLoaderTraceInitialLevelHandoffReceipt *handoff,
    const char *capture,
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryBranchReceipt *out)
{
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryCopyReceipt
        entry_copy;
    const Theron_V1RawLoaderTraceInitialPostEnvelopeTransferReceipt *parent;
    const char *cursor;
    const char *line;
    size_t length;
    size_t source_offset;
    unsigned int branch_pc;
    unsigned int branch_physical_pc;
    unsigned int target_pc;
    unsigned int displacement;
    int consumed;

    if (out) memset(out, 0, sizeof(*out));
    if (!handoff || !capture || !out ||
        !theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry_copy(
            handoff, capture, &entry_copy) || !entry_copy.valid ||
        !entry_copy.copied_source_byte_proven || entry_copy.entry.entry_opcode != 0x80u ||
        entry_copy.entry.call.transfer.byte_count < 2u) {
        return 0;
    }
    parent = &entry_copy.entry.call.transfer.next.entry.call.termination.call.execution.transfer;
    if (entry_copy.entry.call.transfer.source_address < parent->destination_address) {
        return 0;
    }
    source_offset = entry_copy.entry.call.transfer.source_address -
        parent->destination_address;
    if (source_offset >= parent->byte_count || source_offset + 1u >= parent->byte_count ||
        parent->source_address > UINT16_MAX - (source_offset + 1u)) {
        return 0;
    }
    cursor = capture;
    while (tqr_trace_next_line(&cursor, &line, &length)) {
        consumed = 0;
        if (sscanf(line,
                   "main_ram_loader_bra logical_pc=%x physical_pc=%x target=%x displacement=%x%n",
                   &branch_pc, &branch_physical_pc, &target_pc, &displacement,
                   &consumed) != 4 || consumed != (int)length ||
            branch_pc != entry_copy.entry.entry_pc ||
            branch_physical_pc != entry_copy.entry.entry_physical_pc) {
            continue;
        }
        if (target_pc > UINT16_MAX || displacement > UINT8_MAX ||
            displacement != handoff->loader_post_envelope.bytes[source_offset + 1u] ||
            target_pc != (uint16_t)(entry_copy.entry.entry_pc + 2u +
                                    (int8_t)displacement)) {
            return 0;
        }
        out->valid = 1;
        out->entry_copy = entry_copy;
        out->target_pc = (uint16_t)target_pc;
        out->displacement = (uint8_t)displacement;
        out->original_displacement_address = (uint16_t)(parent->source_address +
                                                        source_offset + 1u);
        out->copied_entry_branch_proven = 1;
        out->level_or_object_semantics_proven = 0;
        return 1;
    }
    return 0;
}

int theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry_branch_target(
    const Theron_V1RawLoaderTraceInitialLevelHandoffReceipt *handoff,
    const char *capture,
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryBranchTargetReceipt *out)
{
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryBranchReceipt branch;
    const char *cursor;
    const char *line;
    size_t length;
    unsigned int source_pc;
    unsigned int source_physical_pc;
    unsigned int target_pc;
    unsigned int observed_pc;
    unsigned int observed_physical_pc;
    unsigned int opcode;
    unsigned int branch_target_pc;
    unsigned int branch_displacement;
    int consumed;
    int branch_seen = 0;

    if (out) memset(out, 0, sizeof(*out));
    if (!handoff || !capture || !out ||
        !theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry_branch(
            handoff, capture, &branch) || !branch.valid) {
        return 0;
    }
    cursor = capture;
    while (tqr_trace_next_line(&cursor, &line, &length)) {
        consumed = 0;
        if (sscanf(line,
                   "main_ram_loader_bra logical_pc=%x physical_pc=%x target=%x displacement=%x%n",
                   &source_pc, &source_physical_pc, &branch_target_pc,
                   &branch_displacement, &consumed) == 4 && consumed == (int)length &&
            source_pc == branch.entry_copy.entry.entry_pc &&
            source_physical_pc == branch.entry_copy.entry.entry_physical_pc &&
            branch_target_pc == branch.target_pc &&
            branch_displacement == branch.displacement) {
            branch_seen = 1;
            continue;
        }
        consumed = 0;
        if (sscanf(line,
                   "main_ram_loader_bra_target source_logical_pc=%x source_physical_pc=%x target=%x logical_pc=%x physical_pc=%x opcode=%x%n",
                   &source_pc, &source_physical_pc, &target_pc, &observed_pc,
                   &observed_physical_pc, &opcode, &consumed) != 6 ||
            consumed != (int)length || source_pc != branch.entry_copy.entry.entry_pc ||
            source_physical_pc != branch.entry_copy.entry.entry_physical_pc ||
            target_pc != branch.target_pc || observed_pc != branch.target_pc || !branch_seen) {
            continue;
        }
        if (observed_physical_pc < 0x1f0000u || observed_physical_pc >= 0x1f8000u ||
            opcode > UINT8_MAX) {
            return 0;
        }
        out->valid = 1;
        out->branch = branch;
        out->target_pc = (uint16_t)target_pc;
        out->target_physical_pc = observed_physical_pc;
        out->target_opcode = (uint8_t)opcode;
        out->copied_entry_branch_target_executed = 1;
        out->level_or_object_semantics_proven = 0;
        return 1;
    }
    return 0;
}

int theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry_branch_target_jsr(
    const Theron_V1RawLoaderTraceInitialLevelHandoffReceipt *handoff,
    const char *capture,
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryBranchTargetJsrReceipt *out)
{
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryBranchTargetReceipt
        branch_target;
    const char *cursor;
    const char *line;
    size_t length;
    unsigned int source_target;
    unsigned int source_target_physical_pc;
    unsigned int target_pc;
    unsigned int target_physical_pc;
    unsigned int target_opcode;
    unsigned int control_pc;
    unsigned int control_physical_pc;
    unsigned int jsr_target;
    int consumed;
    int target_seen = 0;

    if (out) memset(out, 0, sizeof(*out));
    if (!handoff || !capture || !out ||
        !theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry_branch_target(
            handoff, capture, &branch_target) || !branch_target.valid) {
        return 0;
    }
    cursor = capture;
    while (tqr_trace_next_line(&cursor, &line, &length)) {
        consumed = 0;
        if (sscanf(line,
                   "main_ram_loader_bra_target source_logical_pc=%x source_physical_pc=%x target=%x logical_pc=%x physical_pc=%x opcode=%x%n",
                   &source_target, &source_target_physical_pc, &target_pc,
                   &control_pc, &target_physical_pc, &target_opcode,
                   &consumed) == 6 && consumed == (int)length &&
            source_target == branch_target.branch.entry_copy.entry.entry_pc &&
            source_target_physical_pc == branch_target.branch.entry_copy.entry.entry_physical_pc &&
            target_pc == branch_target.target_pc && control_pc == branch_target.target_pc &&
            target_physical_pc == branch_target.target_physical_pc &&
            target_opcode == branch_target.target_opcode) {
            target_seen = 1;
            continue;
        }
        consumed = 0;
        if (sscanf(line,
                   "main_ram_loader_bra_target_jsr branch_target=%x branch_target_physical_pc=%x logical_pc=%x physical_pc=%x target=%x%n",
                   &source_target, &source_target_physical_pc, &control_pc,
                   &control_physical_pc, &jsr_target, &consumed) != 5 ||
            consumed != (int)length || !target_seen ||
            source_target != branch_target.target_pc ||
            source_target_physical_pc != branch_target.target_physical_pc ||
            control_pc > UINT16_MAX ||
            control_physical_pc < 0x1f0000u || control_physical_pc >= 0x1f8000u ||
            jsr_target > UINT16_MAX) {
            continue;
        }
        out->valid = 1;
        out->branch_target = branch_target;
        out->control_pc = (uint16_t)control_pc;
        out->control_physical_pc = control_physical_pc;
        out->jsr_target = (uint16_t)jsr_target;
        out->copied_entry_branch_target_jsr_proven = 1;
        out->level_or_object_semantics_proven = 0;
        return 1;
    }
    return 0;
}

int theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry_branch_target_jsr_cd(
    const Theron_V1RawLoaderTraceInitialLevelHandoffReceipt *handoff,
    const char *capture, const uint8_t *track02_data, size_t track02_size,
    const char *track02_md5,
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryBranchTargetJsrCdReceipt *out)
{
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryBranchTargetJsrReceipt
        branch_target_jsr;
    const char *cursor;
    const char *line;
    size_t length;
    unsigned int cpu_pc;
    unsigned int physical;
    unsigned int data;
    unsigned int scsi_generation;
    unsigned int scsi_lba;
    unsigned int scsi_sector_count;
    unsigned int origin_generation;
    unsigned int origin_lba;
    unsigned int origin_offset;
    unsigned long long fifo_sequence;
    unsigned int reader_pc;
    unsigned int logical_destination;
    unsigned int physical_destination;
    unsigned int writer_pc;
    unsigned int writer_physical_pc;
    unsigned int value;
    uint8_t cdb[6] = {0};
    int consumed;
    int jsr_seen = 0;
    int register_seen = 0;
    int scsi_seen = 0;

    if (out) memset(out, 0, sizeof(*out));
    if (!handoff || !capture || !track02_data || !track02_md5 || !out ||
        !theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry_branch_target_jsr(
            handoff, capture, &branch_target_jsr) || !branch_target_jsr.valid) {
        return 0;
    }
    cursor = capture;
    while (tqr_trace_next_line(&cursor, &line, &length)) {
        consumed = 0;
        if (sscanf(line,
                   "main_ram_loader_bra_target_jsr branch_target=%x branch_target_physical_pc=%x logical_pc=%x physical_pc=%x target=%x%n",
                   &cpu_pc, &physical, &reader_pc, &writer_physical_pc,
                   &scsi_lba, &consumed) == 5 && consumed == (int)length &&
            cpu_pc == branch_target_jsr.branch_target.target_pc &&
            physical == branch_target_jsr.branch_target.target_physical_pc &&
            reader_pc == branch_target_jsr.control_pc &&
            writer_physical_pc == branch_target_jsr.control_physical_pc &&
            scsi_lba == branch_target_jsr.jsr_target) {
            jsr_seen = 1;
            continue;
        }
        consumed = 0;
        if (sscanf(line,
                   "pce_cd_register_write cpu_pc=%x physical=%x data=%x%n",
                   &cpu_pc, &physical, &data, &consumed) == 3 &&
            consumed == (int)length && jsr_seen && !scsi_seen) {
            if (cpu_pc != branch_target_jsr.jsr_target || physical != 0x1801u ||
                data > UINT8_MAX) {
                return 0;
            }
            register_seen = 1;
            continue;
        }
        if (!scsi_seen && register_seen &&
            tqr_trace_parse_scsi_read6(line, length, &scsi_generation,
                                        &scsi_lba, &scsi_sector_count, cdb)) {
            if (!scsi_sector_count) return 0;
            scsi_seen = 1;
            continue;
        }
        consumed = 0;
        if (sscanf(line,
                   "pce_cd_fifo_origin_main_ram_receipt generation=%u source_lba=%u source_offset=%u fifo_sequence=%llu reader_pc=%x logical_destination=%x physical_destination=%x writer_pc=%x writer_physical_pc=%x value=%x%n",
                   &origin_generation, &origin_lba, &origin_offset, &fifo_sequence,
                   &reader_pc, &logical_destination, &physical_destination,
                   &writer_pc, &writer_physical_pc, &value, &consumed) != 10 ||
            consumed != (int)length || !scsi_seen ||
            origin_generation != scsi_generation || origin_lba != scsi_lba ||
            origin_lba < 3009u || origin_lba >= scsi_lba + scsi_sector_count ||
            origin_offset >= THERON_TRACK02_RAW_SECTOR_BYTES || value > UINT8_MAX) {
            continue;
        }
        if (!theron_v1_raw_loader_trace_track02_byte_for_scsi_source(
                track02_data, track02_size, track02_md5, origin_lba,
                origin_offset, &out->track02_record, &out->source_byte) ||
            out->source_byte != (uint8_t)value) {
            return 0;
        }
        out->valid = 1;
        out->branch_target_jsr = branch_target_jsr;
        out->cd_register_value = (uint8_t)data;
        out->scsi_generation = scsi_generation;
        out->scsi_lba = scsi_lba;
        out->source_offset = (uint16_t)origin_offset;
        out->jsr_cd_register_write_observed = 1;
        out->read6_record_source_verified = 1;
        out->level_or_object_semantics_proven = 0;
        return 1;
    }
    return 0;
}

int theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer(
    const Theron_V1RawLoaderTraceInitialLevelHandoffReceipt *handoff,
    const char *capture, const uint8_t *track02_data, size_t track02_size,
    const char *track02_md5,
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryBranchTargetJsrCdConsumerReceipt *out)
{
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryBranchTargetJsrCdReceipt
        jsr_cd;
    const char *cursor;
    const char *line;
    size_t length;
    unsigned int origin_generation;
    unsigned int origin_lba;
    unsigned int origin_offset;
    unsigned long long origin_fifo_sequence = 0u;
    unsigned int origin_destination = 0u;
    unsigned int origin_value;
    unsigned int origin_writer_physical_pc;
    unsigned int consumer_sequence;
    unsigned int consumer_generation;
    unsigned int consumer_lba;
    unsigned int consumer_offset;
    unsigned long long consumer_fifo_sequence;
    unsigned int consumer_logical_address;
    unsigned int consumer_physical;
    unsigned int consumer_value;
    unsigned int consumer_reader_pc;
    unsigned int consumer_reader_physical_pc;
    int origin_seen = 0;
    int consumed;

    if (out) memset(out, 0, sizeof(*out));
    if (!handoff || !capture || !track02_data || !track02_md5 || !out ||
        !theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry_branch_target_jsr_cd(
            handoff, capture, track02_data, track02_size, track02_md5,
            &jsr_cd) || !jsr_cd.valid ||
        !jsr_cd.read6_record_source_verified) {
        return 0;
    }
    cursor = capture;
    while (tqr_trace_next_line(&cursor, &line, &length)) {
        if (!origin_seen) {
            if (!tqr_trace_parse_fifo_origin_main_ram(
                    line, length, &origin_generation, &origin_lba,
                    &origin_offset, &origin_fifo_sequence, &origin_destination,
                    &origin_value, &origin_writer_physical_pc)) {
                continue;
            }
            if (origin_generation != jsr_cd.scsi_generation ||
                origin_lba != jsr_cd.scsi_lba ||
                origin_offset != jsr_cd.source_offset ||
                origin_value != jsr_cd.source_byte) {
                continue;
            }
            origin_seen = 1;
            continue;
        }
        consumed = 0;
        if (sscanf(line,
                   "pce_cd_fifo_origin_main_ram_consumer sequence=%u generation=%u source_lba=%u source_offset=%u fifo_sequence=%llu logical_address=%x physical_address=%x value=%x reader_pc=%x reader_physical_pc=%x%n",
                   &consumer_sequence, &consumer_generation, &consumer_lba,
                   &consumer_offset, &consumer_fifo_sequence,
                   &consumer_logical_address, &consumer_physical,
                   &consumer_value, &consumer_reader_pc,
                   &consumer_reader_physical_pc, &consumed) != 10 ||
            consumed != (int)length) {
            continue;
        }
        if (consumer_generation != jsr_cd.scsi_generation ||
            consumer_lba != jsr_cd.scsi_lba ||
            consumer_offset != jsr_cd.source_offset ||
            consumer_value != jsr_cd.source_byte) {
            /* A consumer read of a different FIFO byte is not this byte's
             * consumer and neither proves nor contradicts the joined read. */
            continue;
        }
        if (consumer_fifo_sequence != origin_fifo_sequence ||
            consumer_physical != origin_destination ||
            consumer_offset >= THERON_TRACK02_RAW_SECTOR_BYTES ||
            consumer_physical < 0x1f0000u || consumer_physical >= 0x1f8000u ||
            consumer_value > UINT8_MAX || consumer_reader_pc > UINT16_MAX ||
            consumer_reader_physical_pc < 0x1f0000u ||
            consumer_reader_physical_pc >= 0x1f8000u) {
            /* The exact joined byte was consumed, but by a different
             * transfer, destination, or a non-main-RAM (System Card) reader.
             * That contradicts a game-owned consumer read; fail closed. */
            return 0;
        }
        out->valid = 1;
        out->jsr_cd = jsr_cd;
        out->consumer_generation = consumer_generation;
        out->consumer_lba = consumer_lba;
        out->consumer_physical_address = consumer_physical;
        out->consumer_reader_pc = (uint16_t)consumer_reader_pc;
        out->consumer_reader_physical_pc = consumer_reader_physical_pc;
        out->source_offset = jsr_cd.source_offset;
        out->source_byte = jsr_cd.source_byte;
        out->loader_consumer_read_proven = 1;
        out->level_or_object_semantics_proven = 0;
        return 1;
    }
    return 0;
}

int theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control(
    const Theron_V1RawLoaderTraceInitialLevelHandoffReceipt *handoff,
    const char *capture, const uint8_t *track02_data, size_t track02_size,
    const char *track02_md5,
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryBranchTargetJsrCdConsumerControlReceipt *out)
{
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryBranchTargetJsrCdConsumerReceipt
        consumer;
    const char *cursor;
    const char *line;
    size_t length;
    unsigned int consumer_sequence;
    unsigned int consumer_generation;
    unsigned int consumer_lba;
    unsigned int consumer_offset;
    unsigned long long consumer_fifo_sequence;
    unsigned int consumer_logical_address;
    unsigned int consumer_physical;
    unsigned int consumer_value;
    unsigned int consumer_reader_pc;
    unsigned int consumer_reader_physical_pc;
    unsigned int logical_pc;
    unsigned int physical_pc;
    unsigned int target;
    int consumer_seen = 0;
    int consumed;

    if (out) memset(out, 0, sizeof(*out));
    if (!handoff || !capture || !track02_data || !track02_md5 || !out ||
        !theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer(
            handoff, capture, track02_data, track02_size, track02_md5,
            &consumer) || !consumer.valid ||
        !consumer.loader_consumer_read_proven) {
        return 0;
    }
    cursor = capture;
    while (tqr_trace_next_line(&cursor, &line, &length)) {
        if (!consumer_seen) {
            consumed = 0;
            if (sscanf(line,
                       "pce_cd_fifo_origin_main_ram_consumer sequence=%u generation=%u source_lba=%u source_offset=%u fifo_sequence=%llu logical_address=%x physical_address=%x value=%x reader_pc=%x reader_physical_pc=%x%n",
                       &consumer_sequence, &consumer_generation, &consumer_lba,
                       &consumer_offset, &consumer_fifo_sequence,
                       &consumer_logical_address, &consumer_physical,
                       &consumer_value, &consumer_reader_pc,
                       &consumer_reader_physical_pc, &consumed) == 10 &&
                consumed == (int)length &&
                consumer_generation == consumer.consumer_generation &&
                consumer_lba == consumer.consumer_lba &&
                consumer_offset == consumer.source_offset &&
                consumer_physical == consumer.consumer_physical_address &&
                consumer_value == consumer.source_byte &&
                consumer_reader_pc == consumer.consumer_reader_pc &&
                consumer_reader_physical_pc ==
                    consumer.consumer_reader_physical_pc) {
                consumer_seen = 1;
            }
            continue;
        }
        consumed = 0;
        if (sscanf(line,
                   "main_ram_loader_jsr logical_pc=%x physical_pc=%x target=%x a=%*x x=%*x y=%*x%n",
                   &logical_pc, &physical_pc, &target, &consumed) != 3 ||
            consumed != (int)length) {
            continue;
        }
        if (logical_pc > UINT16_MAX || physical_pc < 0x1f0000u ||
            physical_pc >= 0x1f8000u || target > UINT16_MAX) {
            return 0;
        }
        out->valid = 1;
        out->consumer = consumer;
        out->control_pc = (uint16_t)logical_pc;
        out->control_physical_pc = physical_pc;
        out->control_target = (uint16_t)target;
        out->consumer_control_transfer_proven = 1;
        out->level_or_object_semantics_proven = 0;
        return 1;
    }
    return 0;
}

int theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_entry(
    const Theron_V1RawLoaderTraceInitialLevelHandoffReceipt *handoff,
    const char *capture, const uint8_t *track02_data, size_t track02_size,
    const char *track02_md5,
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryBranchTargetJsrCdConsumerControlEntryReceipt *out)
{
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryBranchTargetJsrCdConsumerControlReceipt
        control;
    const char *cursor;
    const char *line;
    size_t length;
    unsigned int caller_pc;
    unsigned int caller_physical_pc;
    unsigned int target;
    unsigned int entry_pc;
    unsigned int entry_physical_pc;
    unsigned int entry_opcode;
    int control_seen = 0;
    int consumed;

    if (out) memset(out, 0, sizeof(*out));
    if (!handoff || !capture || !track02_data || !track02_md5 || !out ||
        !theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control(
            handoff, capture, track02_data, track02_size, track02_md5,
            &control) || !control.valid ||
        !control.consumer_control_transfer_proven) {
        return 0;
    }
    cursor = capture;
    while (tqr_trace_next_line(&cursor, &line, &length)) {
        if (!control_seen) {
            consumed = 0;
            if (sscanf(line,
                       "main_ram_loader_jsr logical_pc=%x physical_pc=%x target=%x a=%*x x=%*x y=%*x%n",
                       &caller_pc, &caller_physical_pc, &target,
                       &consumed) == 3 && consumed == (int)length &&
                caller_pc == control.control_pc &&
                caller_physical_pc == control.control_physical_pc &&
                target == control.control_target) {
                control_seen = 1;
            }
            continue;
        }
        /* The control entry row must be adjacent to its call row: the
         * producer records a call entry only when the target is executed
         * immediately after the call. Any other row fails closed. */
        consumed = 0;
        if (sscanf(line,
                   "main_ram_loader_call_entry caller_logical_pc=%x caller_physical_pc=%x target=%x logical_pc=%x physical_pc=%x opcode=%x%n",
                   &caller_pc, &caller_physical_pc, &target, &entry_pc,
                   &entry_physical_pc, &entry_opcode, &consumed) != 6 ||
            consumed != (int)length || caller_pc != control.control_pc ||
            caller_physical_pc != control.control_physical_pc ||
            target != control.control_target ||
            entry_pc != control.control_target || entry_pc > UINT16_MAX ||
            entry_physical_pc < 0x1f0000u ||
            entry_physical_pc >= 0x1f8000u || entry_opcode > UINT8_MAX) {
            return 0;
        }
        out->valid = 1;
        out->control = control;
        out->entry_pc = (uint16_t)entry_pc;
        out->entry_physical_pc = entry_physical_pc;
        out->entry_opcode = (uint8_t)entry_opcode;
        out->consumer_control_entry_proven = 1;
        out->level_or_object_semantics_proven = 0;
        return 1;
    }
    return 0;
}

int theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_entry_next(
    const Theron_V1RawLoaderTraceInitialLevelHandoffReceipt *handoff,
    const char *capture, const uint8_t *track02_data, size_t track02_size,
    const char *track02_md5,
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryBranchTargetJsrCdConsumerControlEntryNextReceipt *out)
{
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryBranchTargetJsrCdConsumerControlEntryReceipt
        entry;
    const char *cursor;
    const char *line;
    size_t length;
    unsigned int caller_pc;
    unsigned int caller_physical_pc;
    unsigned int target;
    unsigned int entry_pc;
    unsigned int entry_physical_pc;
    unsigned int entry_opcode;
    unsigned int next_pc;
    unsigned int next_physical_pc;
    unsigned int next_opcode;
    int entry_seen = 0;
    int consumed;

    if (out) memset(out, 0, sizeof(*out));
    if (!handoff || !capture || !track02_data || !track02_md5 || !out ||
        !theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_entry(
            handoff, capture, track02_data, track02_size, track02_md5,
            &entry) || !entry.valid || !entry.consumer_control_entry_proven) {
        return 0;
    }
    cursor = capture;
    while (tqr_trace_next_line(&cursor, &line, &length)) {
        if (!entry_seen) {
            consumed = 0;
            if (sscanf(line,
                       "main_ram_loader_call_entry caller_logical_pc=%x caller_physical_pc=%x target=%x logical_pc=%x physical_pc=%x opcode=%x%n",
                       &caller_pc, &caller_physical_pc, &target, &entry_pc,
                       &entry_physical_pc, &entry_opcode, &consumed) == 6 &&
                consumed == (int)length &&
                caller_pc == entry.control.control_pc &&
                caller_physical_pc == entry.control.control_physical_pc &&
                target == entry.control.control_target &&
                entry_pc == entry.entry_pc &&
                entry_physical_pc == entry.entry_physical_pc &&
                entry_opcode == entry.entry_opcode) {
                entry_seen = 1;
            }
            continue;
        }
        /* The next-instruction row must be adjacent to the control entry
         * row. Any other row fails closed. */
        consumed = 0;
        if (sscanf(line,
                   "main_ram_loader_entry_next entry_logical_pc=%x entry_physical_pc=%x logical_pc=%x physical_pc=%x opcode=%x%n",
                   &entry_pc, &entry_physical_pc, &next_pc,
                   &next_physical_pc, &next_opcode, &consumed) != 5 ||
            consumed != (int)length || entry_pc != entry.entry_pc ||
            entry_physical_pc != entry.entry_physical_pc ||
            next_pc > UINT16_MAX || next_physical_pc < 0x1f0000u ||
            next_physical_pc >= 0x1f8000u || next_opcode > UINT8_MAX) {
            return 0;
        }
        out->valid = 1;
        out->entry = entry;
        out->next_pc = (uint16_t)next_pc;
        out->next_physical_pc = next_physical_pc;
        out->next_opcode = (uint8_t)next_opcode;
        out->consumer_control_entry_next_proven = 1;
        out->level_or_object_semantics_proven = 0;
        return 1;
    }
    return 0;
}

int theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return(
    const Theron_V1RawLoaderTraceInitialLevelHandoffReceipt *handoff,
    const char *capture, const uint8_t *track02_data, size_t track02_size,
    const char *track02_md5,
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryBranchTargetJsrCdConsumerControlReturnReceipt *out)
{
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryBranchTargetJsrCdConsumerControlEntryNextReceipt
        next;
    const char *cursor;
    const char *line;
    size_t length;
    unsigned int entry_pc;
    unsigned int entry_physical_pc;
    unsigned int next_pc;
    unsigned int next_physical_pc;
    unsigned int next_opcode;
    unsigned int logical_pc;
    unsigned int physical_pc;
    unsigned int source_pc;
    unsigned int source_physical_pc;
    unsigned int opcode;
    unsigned int return_instruction_pc = 0u;
    unsigned int return_instruction_physical_pc = 0u;
    unsigned int matching_return_count = 0u;
    int next_seen = 0;
    int return_pending = 0;
    int consumed;

    if (out) memset(out, 0, sizeof(*out));
    if (!handoff || !capture || !track02_data || !track02_md5 || !out ||
        !theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_entry_next(
            handoff, capture, track02_data, track02_size, track02_md5,
            &next) || !next.valid ||
        !next.consumer_control_entry_next_proven) {
        return 0;
    }
    cursor = capture;
    while (tqr_trace_next_line(&cursor, &line, &length)) {
        if (!next_seen) {
            consumed = 0;
            if (sscanf(line,
                       "main_ram_loader_entry_next entry_logical_pc=%x entry_physical_pc=%x logical_pc=%x physical_pc=%x opcode=%x%n",
                       &entry_pc, &entry_physical_pc, &next_pc,
                       &next_physical_pc, &next_opcode, &consumed) == 5 &&
                consumed == (int)length &&
                entry_pc == next.entry.entry_pc &&
                entry_physical_pc == next.entry.entry_physical_pc &&
                next_pc == next.next_pc &&
                next_physical_pc == next.next_physical_pc &&
                next_opcode == next.next_opcode) {
                next_seen = 1;
            }
            continue;
        }
        if (return_pending) {
            consumed = 0;
            if (sscanf(line,
                       "main_ram_loader_post_rts source_logical_pc=%x source_physical_pc=%x logical_pc=%x physical_pc=%x opcode=%x%n",
                       &source_pc, &source_physical_pc, &logical_pc,
                       &physical_pc, &opcode, &consumed) == 5 &&
                consumed == (int)length &&
                source_pc == return_instruction_pc &&
                source_physical_pc == return_instruction_physical_pc) {
                return_pending = 0;
                if (logical_pc ==
                        (unsigned int)next.entry.control.control_pc + 3u &&
                    logical_pc <= UINT16_MAX &&
                    physical_pc >= 0x1f0000u && physical_pc < 0x1f8000u &&
                    opcode <= UINT8_MAX) {
                    if (++matching_return_count != 1u) return 0;
                    out->return_instruction_pc =
                        (uint16_t)return_instruction_pc;
                    out->return_instruction_physical_pc =
                        return_instruction_physical_pc;
                    out->post_return_pc = (uint16_t)logical_pc;
                    out->post_return_physical_pc = physical_pc;
                    out->post_return_opcode = (uint8_t)opcode;
                }
                /* A resume of the just-observed RTS that lands anywhere
                 * else is another routine's or another path's resume and
                 * neither proves nor contradicts the bounded return. */
                continue;
            }
            /* A non-resume row directly after an RTS clears the pending
             * window; that RTS stays an opaque row. */
            return_pending = 0;
        }
        consumed = 0;
        if (sscanf(line,
                   "main_ram_loader_rts logical_pc=%x physical_pc=%x%n",
                   &return_instruction_pc, &return_instruction_physical_pc,
                   &consumed) == 2 && consumed == (int)length &&
            return_instruction_pc <= UINT16_MAX &&
            return_instruction_physical_pc >= 0x1f0000u &&
            return_instruction_physical_pc < 0x1f8000u) {
            return_pending = 1;
        }
    }
    if (!next_seen || matching_return_count != 1u) return 0;
    out->valid = 1;
    out->next = next;
    out->consumer_control_return_proven = 1;
    out->level_or_object_semantics_proven = 0;
    return 1;
}

int theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer(
    const Theron_V1RawLoaderTraceInitialLevelHandoffReceipt *handoff,
    const char *capture, const uint8_t *track02_data, size_t track02_size,
    const char *track02_md5,
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryBranchTargetJsrCdConsumerControlReturnConsumerReceipt *out)
{
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryBranchTargetJsrCdConsumerControlReturnReceipt
        control_return;
    const char *cursor;
    const char *line;
    size_t length;
    unsigned int source_pc;
    unsigned int source_physical_pc;
    unsigned int logical_pc;
    unsigned int physical_pc;
    unsigned int opcode;
    unsigned int origin_generation;
    unsigned int origin_lba;
    unsigned int origin_offset;
    unsigned long long origin_fifo_sequence = 0u;
    unsigned int origin_reader_pc;
    unsigned int origin_logical_destination;
    unsigned int origin_destination = 0u;
    unsigned int origin_writer_pc;
    unsigned int origin_writer_physical_pc;
    unsigned int origin_value;
    unsigned int consumer_sequence;
    unsigned int consumer_generation;
    unsigned int consumer_lba;
    unsigned int consumer_offset;
    unsigned long long consumer_fifo_sequence;
    unsigned int consumer_logical_address;
    unsigned int consumer_physical;
    unsigned int consumer_value;
    unsigned int consumer_reader_pc;
    unsigned int consumer_reader_physical_pc;
    uint32_t source_record = 0u;
    uint8_t source_byte = 0u;
    unsigned int first_offset;
    int resume_seen = 0;
    int receipt_seen = 0;
    int consumed;

    if (out) memset(out, 0, sizeof(*out));
    if (!handoff || !capture || !track02_data || !track02_md5 || !out ||
        !theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return(
            handoff, capture, track02_data, track02_size, track02_md5,
            &control_return) || !control_return.valid ||
        !control_return.consumer_control_return_proven) {
        return 0;
    }
    first_offset =
        control_return.next.entry.control.consumer.source_offset;
    cursor = capture;
    while (tqr_trace_next_line(&cursor, &line, &length)) {
        if (!resume_seen) {
            consumed = 0;
            if (sscanf(line,
                       "main_ram_loader_post_rts source_logical_pc=%x source_physical_pc=%x logical_pc=%x physical_pc=%x opcode=%x%n",
                       &source_pc, &source_physical_pc, &logical_pc,
                       &physical_pc, &opcode, &consumed) == 5 &&
                consumed == (int)length &&
                source_pc == control_return.return_instruction_pc &&
                source_physical_pc ==
                    control_return.return_instruction_physical_pc &&
                logical_pc == control_return.post_return_pc &&
                physical_pc == control_return.post_return_physical_pc &&
                opcode == control_return.post_return_opcode) {
                resume_seen = 1;
            }
            continue;
        }
        if (!receipt_seen) {
            consumed = 0;
            if (sscanf(line,
                       "pce_cd_fifo_origin_main_ram_receipt generation=%u source_lba=%u source_offset=%u fifo_sequence=%llu reader_pc=%x logical_destination=%x physical_destination=%x writer_pc=%x writer_physical_pc=%x value=%x%n",
                       &origin_generation, &origin_lba, &origin_offset,
                       &origin_fifo_sequence, &origin_reader_pc,
                       &origin_logical_destination, &origin_destination,
                       &origin_writer_pc, &origin_writer_physical_pc,
                       &origin_value, &consumed) != 10 ||
                consumed != (int)length) {
                continue;
            }
            if (origin_generation !=
                    control_return.next.entry.control.consumer.consumer_generation ||
                origin_lba !=
                    control_return.next.entry.control.consumer.consumer_lba ||
                origin_offset != first_offset + 1u) {
                /* A FIFO transfer of any other byte is not the adjacent
                 * byte's receipt and remains an opaque row. */
                continue;
            }
            if (origin_value > UINT8_MAX ||
                !theron_v1_raw_loader_trace_track02_byte_for_scsi_source(
                    track02_data, track02_size, track02_md5, origin_lba,
                    origin_offset, &source_record, &source_byte) ||
                source_byte != (uint8_t)origin_value) {
                return 0;
            }
            receipt_seen = 1;
            continue;
        }
        consumed = 0;
        if (sscanf(line,
                   "pce_cd_fifo_origin_main_ram_consumer sequence=%u generation=%u source_lba=%u source_offset=%u fifo_sequence=%llu logical_address=%x physical_address=%x value=%x reader_pc=%x reader_physical_pc=%x%n",
                   &consumer_sequence, &consumer_generation, &consumer_lba,
                   &consumer_offset, &consumer_fifo_sequence,
                   &consumer_logical_address, &consumer_physical,
                   &consumer_value, &consumer_reader_pc,
                   &consumer_reader_physical_pc, &consumed) != 10 ||
            consumed != (int)length) {
            continue;
        }
        if (consumer_generation !=
                control_return.next.entry.control.consumer.consumer_generation ||
            consumer_lba !=
                control_return.next.entry.control.consumer.consumer_lba ||
            consumer_offset != first_offset + 1u ||
            consumer_value != source_byte) {
            /* A consumer read of a different FIFO byte is not this byte's
             * consumer and neither proves nor contradicts the joined read. */
            continue;
        }
        if (consumer_sequence != 1u ||
            consumer_fifo_sequence != origin_fifo_sequence ||
            consumer_physical != origin_destination ||
            consumer_physical < 0x1f0000u ||
            consumer_physical >= 0x1f8000u ||
            consumer_reader_pc > UINT16_MAX ||
            consumer_reader_physical_pc < 0x1f0000u ||
            consumer_reader_physical_pc >= 0x1f8000u) {
            /* The exact adjacent byte was consumed, but out of order, by a
             * different transfer or destination, or by a non-main-RAM
             * (System Card) reader. That contradicts a resumed game-owned
             * consumer read; fail closed. */
            return 0;
        }
        out->valid = 1;
        out->control_return = control_return;
        out->consumer_generation = consumer_generation;
        out->consumer_lba = consumer_lba;
        out->track02_record = source_record;
        out->consumer_physical_address = consumer_physical;
        out->consumer_reader_pc = (uint16_t)consumer_reader_pc;
        out->consumer_reader_physical_pc = consumer_reader_physical_pc;
        out->source_offset = (uint16_t)consumer_offset;
        out->source_byte = source_byte;
        out->resumed_loader_consumer_read_proven = 1;
        out->level_or_object_semantics_proven = 0;
        return 1;
    }
    return 0;
}

int theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control(
    const Theron_V1RawLoaderTraceInitialLevelHandoffReceipt *handoff,
    const char *capture, const uint8_t *track02_data, size_t track02_size,
    const char *track02_md5,
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryBranchTargetJsrCdConsumerControlReturnConsumerControlReceipt *out)
{
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryBranchTargetJsrCdConsumerControlReturnConsumerReceipt
        consumer;
    const char *cursor;
    const char *line;
    size_t length;
    unsigned int consumer_sequence;
    unsigned int consumer_generation;
    unsigned int consumer_lba;
    unsigned int consumer_offset;
    unsigned long long consumer_fifo_sequence;
    unsigned int consumer_logical_address;
    unsigned int consumer_physical;
    unsigned int consumer_value;
    unsigned int consumer_reader_pc;
    unsigned int consumer_reader_physical_pc;
    unsigned int logical_pc;
    unsigned int physical_pc;
    unsigned int target;
    int consumer_seen = 0;
    int consumed;

    if (out) memset(out, 0, sizeof(*out));
    if (!handoff || !capture || !track02_data || !track02_md5 || !out ||
        !theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer(
            handoff, capture, track02_data, track02_size, track02_md5,
            &consumer) || !consumer.valid ||
        !consumer.resumed_loader_consumer_read_proven) {
        return 0;
    }
    cursor = capture;
    while (tqr_trace_next_line(&cursor, &line, &length)) {
        if (!consumer_seen) {
            consumed = 0;
            if (sscanf(line,
                       "pce_cd_fifo_origin_main_ram_consumer sequence=%u generation=%u source_lba=%u source_offset=%u fifo_sequence=%llu logical_address=%x physical_address=%x value=%x reader_pc=%x reader_physical_pc=%x%n",
                       &consumer_sequence, &consumer_generation,
                       &consumer_lba, &consumer_offset,
                       &consumer_fifo_sequence, &consumer_logical_address,
                       &consumer_physical, &consumer_value,
                       &consumer_reader_pc, &consumer_reader_physical_pc,
                       &consumed) == 10 && consumed == (int)length &&
                consumer_sequence == 1u &&
                consumer_generation == consumer.consumer_generation &&
                consumer_lba == consumer.consumer_lba &&
                consumer_offset == consumer.source_offset &&
                consumer_physical == consumer.consumer_physical_address &&
                consumer_value == consumer.source_byte &&
                consumer_reader_pc == consumer.consumer_reader_pc &&
                consumer_reader_physical_pc ==
                    consumer.consumer_reader_physical_pc) {
                consumer_seen = 1;
            }
            continue;
        }
        consumed = 0;
        if (sscanf(line,
                   "main_ram_loader_jsr logical_pc=%x physical_pc=%x target=%x a=%*x x=%*x y=%*x%n",
                   &logical_pc, &physical_pc, &target, &consumed) != 3 ||
            consumed != (int)length) {
            continue;
        }
        if (logical_pc > UINT16_MAX || physical_pc < 0x1f0000u ||
            physical_pc >= 0x1f8000u || target > UINT16_MAX) {
            return 0;
        }
        out->valid = 1;
        out->consumer = consumer;
        out->control_pc = (uint16_t)logical_pc;
        out->control_physical_pc = physical_pc;
        out->control_target = (uint16_t)target;
        out->resumed_consumer_control_transfer_proven = 1;
        out->level_or_object_semantics_proven = 0;
        return 1;
    }
    return 0;
}

int theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control_entry(
    const Theron_V1RawLoaderTraceInitialLevelHandoffReceipt *handoff,
    const char *capture, const uint8_t *track02_data, size_t track02_size,
    const char *track02_md5,
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryBranchTargetJsrCdConsumerControlReturnConsumerControlEntryReceipt *out)
{
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryBranchTargetJsrCdConsumerControlReturnConsumerControlReceipt
        control;
    const char *cursor;
    const char *line;
    size_t length;
    unsigned int caller_pc;
    unsigned int caller_physical_pc;
    unsigned int target;
    unsigned int entry_pc;
    unsigned int entry_physical_pc;
    unsigned int entry_opcode;
    int control_seen = 0;
    int consumed;

    if (out) memset(out, 0, sizeof(*out));
    if (!handoff || !capture || !track02_data || !track02_md5 || !out ||
        !theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control(
            handoff, capture, track02_data, track02_size, track02_md5,
            &control) || !control.valid ||
        !control.resumed_consumer_control_transfer_proven) {
        return 0;
    }
    cursor = capture;
    while (tqr_trace_next_line(&cursor, &line, &length)) {
        if (!control_seen) {
            consumed = 0;
            if (sscanf(line,
                       "main_ram_loader_jsr logical_pc=%x physical_pc=%x target=%x a=%*x x=%*x y=%*x%n",
                       &caller_pc, &caller_physical_pc, &target,
                       &consumed) == 3 && consumed == (int)length &&
                caller_pc == control.control_pc &&
                caller_physical_pc == control.control_physical_pc &&
                target == control.control_target) {
                control_seen = 1;
            }
            continue;
        }
        /* The control entry row must be adjacent to its call row: the
         * producer records a call entry only when the target is executed
         * immediately after the call. Any other row fails closed. */
        consumed = 0;
        if (sscanf(line,
                   "main_ram_loader_call_entry caller_logical_pc=%x caller_physical_pc=%x target=%x logical_pc=%x physical_pc=%x opcode=%x%n",
                   &caller_pc, &caller_physical_pc, &target, &entry_pc,
                   &entry_physical_pc, &entry_opcode, &consumed) != 6 ||
            consumed != (int)length || caller_pc != control.control_pc ||
            caller_physical_pc != control.control_physical_pc ||
            target != control.control_target ||
            entry_pc != control.control_target || entry_pc > UINT16_MAX ||
            entry_physical_pc < 0x1f0000u ||
            entry_physical_pc >= 0x1f8000u || entry_opcode > UINT8_MAX) {
            return 0;
        }
        out->valid = 1;
        out->control = control;
        out->entry_pc = (uint16_t)entry_pc;
        out->entry_physical_pc = entry_physical_pc;
        out->entry_opcode = (uint8_t)entry_opcode;
        out->resumed_control_entry_proven = 1;
        out->level_or_object_semantics_proven = 0;
        return 1;
    }
    return 0;
}

int theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control_entry_next(
    const Theron_V1RawLoaderTraceInitialLevelHandoffReceipt *handoff,
    const char *capture, const uint8_t *track02_data, size_t track02_size,
    const char *track02_md5,
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryBranchTargetJsrCdConsumerControlReturnConsumerControlEntryNextReceipt *out)
{
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryBranchTargetJsrCdConsumerControlReturnConsumerControlEntryReceipt
        entry;
    const char *cursor;
    const char *line;
    size_t length;
    unsigned int caller_pc;
    unsigned int caller_physical_pc;
    unsigned int target;
    unsigned int entry_pc;
    unsigned int entry_physical_pc;
    unsigned int entry_opcode;
    unsigned int next_pc;
    unsigned int next_physical_pc;
    unsigned int next_opcode;
    int entry_seen = 0;
    int consumed;

    if (out) memset(out, 0, sizeof(*out));
    if (!handoff || !capture || !track02_data || !track02_md5 || !out ||
        !theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control_entry(
            handoff, capture, track02_data, track02_size, track02_md5,
            &entry) || !entry.valid || !entry.resumed_control_entry_proven) {
        return 0;
    }
    cursor = capture;
    while (tqr_trace_next_line(&cursor, &line, &length)) {
        if (!entry_seen) {
            consumed = 0;
            if (sscanf(line,
                       "main_ram_loader_call_entry caller_logical_pc=%x caller_physical_pc=%x target=%x logical_pc=%x physical_pc=%x opcode=%x%n",
                       &caller_pc, &caller_physical_pc, &target, &entry_pc,
                       &entry_physical_pc, &entry_opcode, &consumed) == 6 &&
                consumed == (int)length &&
                caller_pc == entry.control.control_pc &&
                caller_physical_pc == entry.control.control_physical_pc &&
                target == entry.control.control_target &&
                entry_pc == entry.entry_pc &&
                entry_physical_pc == entry.entry_physical_pc &&
                entry_opcode == entry.entry_opcode) {
                entry_seen = 1;
            }
            continue;
        }
        /* The next-instruction row must be adjacent to the resumed control
         * entry row. Any other row fails closed. */
        consumed = 0;
        if (sscanf(line,
                   "main_ram_loader_entry_next entry_logical_pc=%x entry_physical_pc=%x logical_pc=%x physical_pc=%x opcode=%x%n",
                   &entry_pc, &entry_physical_pc, &next_pc,
                   &next_physical_pc, &next_opcode, &consumed) != 5 ||
            consumed != (int)length || entry_pc != entry.entry_pc ||
            entry_physical_pc != entry.entry_physical_pc ||
            next_pc > UINT16_MAX || next_physical_pc < 0x1f0000u ||
            next_physical_pc >= 0x1f8000u || next_opcode > UINT8_MAX) {
            return 0;
        }
        out->valid = 1;
        out->entry = entry;
        out->next_pc = (uint16_t)next_pc;
        out->next_physical_pc = next_physical_pc;
        out->next_opcode = (uint8_t)next_opcode;
        out->resumed_control_entry_next_proven = 1;
        out->level_or_object_semantics_proven = 0;
        return 1;
    }
    return 0;
}

int theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control_return(
    const Theron_V1RawLoaderTraceInitialLevelHandoffReceipt *handoff,
    const char *capture, const uint8_t *track02_data, size_t track02_size,
    const char *track02_md5,
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryBranchTargetJsrCdConsumerControlReturnConsumerControlReturnReceipt *out)
{
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryBranchTargetJsrCdConsumerControlReturnConsumerControlEntryNextReceipt
        next;
    const char *cursor;
    const char *line;
    size_t length;
    unsigned int entry_pc;
    unsigned int entry_physical_pc;
    unsigned int next_pc;
    unsigned int next_physical_pc;
    unsigned int next_opcode;
    unsigned int logical_pc;
    unsigned int physical_pc;
    unsigned int source_pc;
    unsigned int source_physical_pc;
    unsigned int opcode;
    unsigned int return_instruction_pc = 0u;
    unsigned int return_instruction_physical_pc = 0u;
    unsigned int matching_return_count = 0u;
    int next_seen = 0;
    int return_pending = 0;
    int consumed;

    if (out) memset(out, 0, sizeof(*out));
    if (!handoff || !capture || !track02_data || !track02_md5 || !out ||
        !theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control_entry_next(
            handoff, capture, track02_data, track02_size, track02_md5,
            &next) || !next.valid ||
        !next.resumed_control_entry_next_proven) {
        return 0;
    }
    cursor = capture;
    while (tqr_trace_next_line(&cursor, &line, &length)) {
        if (!next_seen) {
            consumed = 0;
            if (sscanf(line,
                       "main_ram_loader_entry_next entry_logical_pc=%x entry_physical_pc=%x logical_pc=%x physical_pc=%x opcode=%x%n",
                       &entry_pc, &entry_physical_pc, &next_pc,
                       &next_physical_pc, &next_opcode, &consumed) == 5 &&
                consumed == (int)length &&
                entry_pc == next.entry.entry_pc &&
                entry_physical_pc == next.entry.entry_physical_pc &&
                next_pc == next.next_pc &&
                next_physical_pc == next.next_physical_pc &&
                next_opcode == next.next_opcode) {
                next_seen = 1;
            }
            continue;
        }
        if (return_pending) {
            consumed = 0;
            if (sscanf(line,
                       "main_ram_loader_post_rts source_logical_pc=%x source_physical_pc=%x logical_pc=%x physical_pc=%x opcode=%x%n",
                       &source_pc, &source_physical_pc, &logical_pc,
                       &physical_pc, &opcode, &consumed) == 5 &&
                consumed == (int)length &&
                source_pc == return_instruction_pc &&
                source_physical_pc == return_instruction_physical_pc) {
                return_pending = 0;
                if (logical_pc ==
                        (unsigned int)next.entry.control.control_pc + 3u &&
                    logical_pc <= UINT16_MAX &&
                    physical_pc >= 0x1f0000u && physical_pc < 0x1f8000u &&
                    opcode <= UINT8_MAX) {
                    if (++matching_return_count != 1u) return 0;
                    out->return_instruction_pc =
                        (uint16_t)return_instruction_pc;
                    out->return_instruction_physical_pc =
                        return_instruction_physical_pc;
                    out->post_return_pc = (uint16_t)logical_pc;
                    out->post_return_physical_pc = physical_pc;
                    out->post_return_opcode = (uint8_t)opcode;
                }
                /* A resume of the just-observed RTS that lands anywhere
                 * else is another routine's or another path's resume and
                 * neither proves nor contradicts the bounded return. */
                continue;
            }
            /* A non-resume row directly after an RTS clears the pending
             * window; that RTS stays an opaque row. */
            return_pending = 0;
        }
        consumed = 0;
        if (sscanf(line,
                   "main_ram_loader_rts logical_pc=%x physical_pc=%x%n",
                   &return_instruction_pc, &return_instruction_physical_pc,
                   &consumed) == 2 && consumed == (int)length &&
            return_instruction_pc <= UINT16_MAX &&
            return_instruction_physical_pc >= 0x1f0000u &&
            return_instruction_physical_pc < 0x1f8000u) {
            return_pending = 1;
        }
    }
    if (!next_seen || matching_return_count != 1u) return 0;
    out->valid = 1;
    out->next = next;
    out->resumed_control_return_proven = 1;
    out->level_or_object_semantics_proven = 0;
    return 1;
}

int theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control_return_consumer(
    const Theron_V1RawLoaderTraceInitialLevelHandoffReceipt *handoff,
    const char *capture, const uint8_t *track02_data, size_t track02_size,
    const char *track02_md5,
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryBranchTargetJsrCdConsumerControlReturnConsumerControlReturnConsumerReceipt *out)
{
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryBranchTargetJsrCdConsumerControlReturnConsumerControlReturnReceipt
        control_return;
    const char *cursor;
    const char *line;
    size_t length;
    unsigned int source_pc;
    unsigned int source_physical_pc;
    unsigned int logical_pc;
    unsigned int physical_pc;
    unsigned int opcode;
    unsigned int origin_generation;
    unsigned int origin_lba;
    unsigned int origin_offset;
    unsigned long long origin_fifo_sequence = 0u;
    unsigned int origin_reader_pc;
    unsigned int origin_logical_destination;
    unsigned int origin_destination = 0u;
    unsigned int origin_writer_pc;
    unsigned int origin_writer_physical_pc;
    unsigned int origin_value;
    unsigned int consumer_sequence;
    unsigned int consumer_generation;
    unsigned int consumer_lba;
    unsigned int consumer_offset;
    unsigned long long consumer_fifo_sequence;
    unsigned int consumer_logical_address;
    unsigned int consumer_physical;
    unsigned int consumer_value;
    unsigned int consumer_reader_pc;
    unsigned int consumer_reader_physical_pc;
    uint32_t source_record = 0u;
    uint8_t source_byte = 0u;
    unsigned int first_offset;
    int resume_seen = 0;
    int receipt_seen = 0;
    int consumed;

    if (out) memset(out, 0, sizeof(*out));
    if (!handoff || !capture || !track02_data || !track02_md5 || !out ||
        !theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control_return(
            handoff, capture, track02_data, track02_size, track02_md5,
            &control_return) || !control_return.valid ||
        !control_return.resumed_control_return_proven) {
        return 0;
    }
    first_offset =
        control_return.next.entry.control.consumer.control_return.next.entry
            .control.consumer.source_offset;
    cursor = capture;
    while (tqr_trace_next_line(&cursor, &line, &length)) {
        if (!resume_seen) {
            consumed = 0;
            if (sscanf(line,
                       "main_ram_loader_post_rts source_logical_pc=%x source_physical_pc=%x logical_pc=%x physical_pc=%x opcode=%x%n",
                       &source_pc, &source_physical_pc, &logical_pc,
                       &physical_pc, &opcode, &consumed) == 5 &&
                consumed == (int)length &&
                source_pc == control_return.return_instruction_pc &&
                source_physical_pc ==
                    control_return.return_instruction_physical_pc &&
                logical_pc == control_return.post_return_pc &&
                physical_pc == control_return.post_return_physical_pc &&
                opcode == control_return.post_return_opcode) {
                resume_seen = 1;
            }
            continue;
        }
        if (!receipt_seen) {
            consumed = 0;
            if (sscanf(line,
                       "pce_cd_fifo_origin_main_ram_receipt generation=%u source_lba=%u source_offset=%u fifo_sequence=%llu reader_pc=%x logical_destination=%x physical_destination=%x writer_pc=%x writer_physical_pc=%x value=%x%n",
                       &origin_generation, &origin_lba, &origin_offset,
                       &origin_fifo_sequence, &origin_reader_pc,
                       &origin_logical_destination, &origin_destination,
                       &origin_writer_pc, &origin_writer_physical_pc,
                       &origin_value, &consumed) != 10 ||
                consumed != (int)length) {
                continue;
            }
            if (origin_generation !=
                    control_return.next.entry.control.consumer.consumer_generation ||
                origin_lba !=
                    control_return.next.entry.control.consumer.consumer_lba ||
                origin_offset != first_offset + 2u) {
                /* A FIFO transfer of any other byte is not the second
                 * adjacent byte's receipt and remains an opaque row. */
                continue;
            }
            if (origin_value > UINT8_MAX ||
                !theron_v1_raw_loader_trace_track02_byte_for_scsi_source(
                    track02_data, track02_size, track02_md5, origin_lba,
                    origin_offset, &source_record, &source_byte) ||
                source_byte != (uint8_t)origin_value) {
                return 0;
            }
            receipt_seen = 1;
            continue;
        }
        consumed = 0;
        if (sscanf(line,
                   "pce_cd_fifo_origin_main_ram_consumer sequence=%u generation=%u source_lba=%u source_offset=%u fifo_sequence=%llu logical_address=%x physical_address=%x value=%x reader_pc=%x reader_physical_pc=%x%n",
                   &consumer_sequence, &consumer_generation, &consumer_lba,
                   &consumer_offset, &consumer_fifo_sequence,
                   &consumer_logical_address, &consumer_physical,
                   &consumer_value, &consumer_reader_pc,
                   &consumer_reader_physical_pc, &consumed) != 10 ||
            consumed != (int)length) {
            continue;
        }
        if (consumer_generation !=
                control_return.next.entry.control.consumer.consumer_generation ||
            consumer_lba !=
                control_return.next.entry.control.consumer.consumer_lba ||
            consumer_offset != first_offset + 2u ||
            consumer_value != source_byte) {
            /* A consumer read of a different FIFO byte is not this byte's
             * consumer and neither proves nor contradicts the joined read. */
            continue;
        }
        if (consumer_sequence != 2u ||
            consumer_fifo_sequence != origin_fifo_sequence ||
            consumer_physical != origin_destination ||
            consumer_physical < 0x1f0000u ||
            consumer_physical >= 0x1f8000u ||
            consumer_reader_pc > UINT16_MAX ||
            consumer_reader_physical_pc < 0x1f0000u ||
            consumer_reader_physical_pc >= 0x1f8000u) {
            /* The exact second adjacent byte was consumed, but out of
             * order, by a different transfer or destination, or by a
             * non-main-RAM (System Card) reader. That contradicts a
             * twice-resumed game-owned consumer read; fail closed. */
            return 0;
        }
        out->valid = 1;
        out->control_return = control_return;
        out->consumer_generation = consumer_generation;
        out->consumer_lba = consumer_lba;
        out->track02_record = source_record;
        out->consumer_physical_address = consumer_physical;
        out->consumer_reader_pc = (uint16_t)consumer_reader_pc;
        out->consumer_reader_physical_pc = consumer_reader_physical_pc;
        out->source_offset = (uint16_t)consumer_offset;
        out->source_byte = source_byte;
        out->twice_resumed_loader_consumer_read_proven = 1;
        out->level_or_object_semantics_proven = 0;
        return 1;
    }
    return 0;
}

int theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control_return_consumer_loop_continuation(
    const Theron_V1RawLoaderTraceInitialLevelHandoffReceipt *handoff,
    const char *capture, const uint8_t *track02_data, size_t track02_size,
    const char *track02_md5,
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryBranchTargetJsrCdConsumerControlReturnConsumerControlReturnConsumerLoopContinuationReceipt *out)
{
    Theron_V1RawLoaderTraceInitialPostEnvelopeCallerNextTransferCallEntryBranchTargetJsrCdConsumerControlReturnConsumerControlReturnConsumerReceipt
        third;
    const char *cursor;
    const char *continuation;
    const char *line;
    size_t length;
    size_t iteration;
    unsigned int consumer_sequence;
    unsigned int consumer_generation;
    unsigned int consumer_lba;
    unsigned int consumer_offset;
    unsigned long long consumer_fifo_sequence;
    unsigned int consumer_logical_address;
    unsigned int consumer_physical;
    unsigned int consumer_value;
    unsigned int consumer_reader_pc;
    unsigned int consumer_reader_physical_pc;
    unsigned int logical_pc;
    unsigned int physical_pc;
    unsigned int target;
    unsigned int caller_pc;
    unsigned int caller_physical_pc;
    unsigned int entry_pc;
    unsigned int entry_physical_pc;
    unsigned int entry_opcode;
    unsigned int next_pc;
    unsigned int next_physical_pc;
    unsigned int next_opcode;
    unsigned int source_pc;
    unsigned int source_physical_pc;
    unsigned int opcode;
    unsigned int return_instruction_pc;
    unsigned int return_instruction_physical_pc;
    unsigned int matching_return_count;
    unsigned int origin_generation;
    unsigned int origin_lba;
    unsigned int origin_offset;
    unsigned long long origin_fifo_sequence = 0u;
    unsigned int origin_reader_pc;
    unsigned int origin_logical_destination;
    unsigned int origin_destination = 0u;
    unsigned int origin_writer_pc;
    unsigned int origin_writer_physical_pc;
    unsigned int origin_value;
    uint32_t source_record = 0u;
    uint8_t source_byte = 0u;
    unsigned int first_offset;
    int anchor_seen;
    int return_pending;
    int consumed;

    if (out) memset(out, 0, sizeof(*out));
    if (!handoff || !capture || !track02_data || !track02_md5 || !out ||
        !theron_v1_raw_loader_trace_bind_initial_post_envelope_caller_next_transfer_call_entry_branch_target_jsr_cd_consumer_control_return_consumer_control_return_consumer(
            handoff, capture, track02_data, track02_size, track02_md5,
            &third) || !third.valid ||
        !third.twice_resumed_loader_consumer_read_proven) {
        return 0;
    }
    first_offset =
        third.control_return.next.entry.control.consumer.control_return.next
            .entry.control.consumer.source_offset;

    /* The exact twice-resumed consumer row anchors the continuation: the
     * generalized loop may only extend a capture that already proved the
     * first three consume/dispatch steps. */
    cursor = capture;
    anchor_seen = 0;
    while (tqr_trace_next_line(&cursor, &line, &length)) {
        consumed = 0;
        if (sscanf(line,
                   "pce_cd_fifo_origin_main_ram_consumer sequence=%u generation=%u source_lba=%u source_offset=%u fifo_sequence=%llu logical_address=%x physical_address=%x value=%x reader_pc=%x reader_physical_pc=%x%n",
                   &consumer_sequence, &consumer_generation, &consumer_lba,
                   &consumer_offset, &consumer_fifo_sequence,
                   &consumer_logical_address, &consumer_physical,
                   &consumer_value, &consumer_reader_pc,
                   &consumer_reader_physical_pc, &consumed) == 10 &&
            consumed == (int)length && consumer_sequence == 2u &&
            consumer_generation == third.consumer_generation &&
            consumer_lba == third.consumer_lba &&
            consumer_offset == third.source_offset &&
            consumer_physical == third.consumer_physical_address &&
            consumer_value == third.source_byte &&
            consumer_reader_pc == third.consumer_reader_pc &&
            consumer_reader_physical_pc == third.consumer_reader_physical_pc) {
            anchor_seen = 1;
            break;
        }
    }
    if (!anchor_seen) return 0;
    continuation = cursor;

    for (iteration = 0u;
         iteration < THERON_V1_RAW_LOADER_LOOP_CONTINUATION_ITERATIONS;
         ++iteration) {
        Theron_V1RawLoaderTraceLoopContinuationIterationReceipt current;
        unsigned int expected_sequence = 3u + (unsigned int)iteration;
        unsigned int expected_offset =
            first_offset + 3u + (unsigned int)iteration;
        int jsr_seen = 0;
        int entry_seen = 0;
        int next_seen = 0;
        int receipt_seen = 0;
        int consumer_seen = 0;

        memset(&current, 0, sizeof(current));

        /* Pass A: the first main-RAM control transfer after the previous
         * consumer read, its adjacent call-entry row, and the adjacent
         * next-instruction row. The target stays opaque. */
        cursor = continuation;
        while (tqr_trace_next_line(&cursor, &line, &length)) {
            if (!jsr_seen) {
                consumed = 0;
                if (sscanf(line,
                           "main_ram_loader_jsr logical_pc=%x physical_pc=%x target=%x a=%*x x=%*x y=%*x%n",
                           &logical_pc, &physical_pc, &target,
                           &consumed) == 3 && consumed == (int)length) {
                    if (logical_pc > UINT16_MAX ||
                        physical_pc < 0x1f0000u ||
                        physical_pc >= 0x1f8000u || target > UINT16_MAX) {
                        return 0;
                    }
                    current.control_pc = (uint16_t)logical_pc;
                    current.control_physical_pc = physical_pc;
                    current.control_target = (uint16_t)target;
                    jsr_seen = 1;
                }
                continue;
            }
            if (!entry_seen) {
                /* The call-entry row must be adjacent to the control
                 * transfer; any other row fails closed. */
                consumed = 0;
                if (sscanf(line,
                           "main_ram_loader_call_entry caller_logical_pc=%x caller_physical_pc=%x target=%x logical_pc=%x physical_pc=%x opcode=%x%n",
                           &caller_pc, &caller_physical_pc, &target,
                           &entry_pc, &entry_physical_pc, &entry_opcode,
                           &consumed) != 6 || consumed != (int)length ||
                    caller_pc != current.control_pc ||
                    caller_physical_pc != current.control_physical_pc ||
                    target != current.control_target ||
                    entry_pc != current.control_target ||
                    entry_pc > UINT16_MAX ||
                    entry_physical_pc < 0x1f0000u ||
                    entry_physical_pc >= 0x1f8000u ||
                    entry_opcode > UINT8_MAX) {
                    return 0;
                }
                current.entry_pc = (uint16_t)entry_pc;
                current.entry_physical_pc = entry_physical_pc;
                current.entry_opcode = (uint8_t)entry_opcode;
                entry_seen = 1;
                continue;
            }
            /* The next-instruction row must be adjacent to the call-entry
             * row; any other row fails closed. */
            consumed = 0;
            if (sscanf(line,
                       "main_ram_loader_entry_next entry_logical_pc=%x entry_physical_pc=%x logical_pc=%x physical_pc=%x opcode=%x%n",
                       &entry_pc, &entry_physical_pc, &next_pc,
                       &next_physical_pc, &next_opcode, &consumed) != 5 ||
                consumed != (int)length || entry_pc != current.entry_pc ||
                entry_physical_pc != current.entry_physical_pc ||
                next_pc > UINT16_MAX || next_physical_pc < 0x1f0000u ||
                next_physical_pc >= 0x1f8000u || next_opcode > UINT8_MAX) {
                return 0;
            }
            current.next_pc = (uint16_t)next_pc;
            current.next_physical_pc = next_physical_pc;
            current.next_opcode = (uint8_t)next_opcode;
            next_seen = 1;
            break;
        }
        if (!next_seen) return 0;

        /* Pass B: exactly one main-RAM RTS anywhere after that fetched
         * window whose linked post-RTS row resumes at the exact control
         * call return address. Zero or two qualifying resumes fail closed;
         * other routines' RTS/post-RTS rows remain opaque. */
        cursor = capture;
        anchor_seen = 0;
        return_pending = 0;
        matching_return_count = 0u;
        return_instruction_pc = 0u;
        return_instruction_physical_pc = 0u;
        while (tqr_trace_next_line(&cursor, &line, &length)) {
            if (!anchor_seen) {
                consumed = 0;
                if (sscanf(line,
                           "main_ram_loader_entry_next entry_logical_pc=%x entry_physical_pc=%x logical_pc=%x physical_pc=%x opcode=%x%n",
                           &entry_pc, &entry_physical_pc, &next_pc,
                           &next_physical_pc, &next_opcode,
                           &consumed) == 5 && consumed == (int)length &&
                    entry_pc == current.entry_pc &&
                    entry_physical_pc == current.entry_physical_pc &&
                    next_pc == current.next_pc &&
                    next_physical_pc == current.next_physical_pc &&
                    next_opcode == current.next_opcode) {
                    anchor_seen = 1;
                }
                continue;
            }
            if (return_pending) {
                consumed = 0;
                if (sscanf(line,
                           "main_ram_loader_post_rts source_logical_pc=%x source_physical_pc=%x logical_pc=%x physical_pc=%x opcode=%x%n",
                           &source_pc, &source_physical_pc, &logical_pc,
                           &physical_pc, &opcode, &consumed) == 5 &&
                    consumed == (int)length &&
                    source_pc == return_instruction_pc &&
                    source_physical_pc == return_instruction_physical_pc) {
                    return_pending = 0;
                    if (logical_pc ==
                            (unsigned int)current.control_pc + 3u &&
                        logical_pc <= UINT16_MAX &&
                        physical_pc >= 0x1f0000u &&
                        physical_pc < 0x1f8000u && opcode <= UINT8_MAX) {
                        if (++matching_return_count != 1u) return 0;
                        current.return_instruction_pc =
                            (uint16_t)return_instruction_pc;
                        current.return_instruction_physical_pc =
                            return_instruction_physical_pc;
                        current.post_return_pc = (uint16_t)logical_pc;
                        current.post_return_physical_pc = physical_pc;
                        current.post_return_opcode = (uint8_t)opcode;
                    }
                    /* A resume of the just-observed RTS that lands
                     * anywhere else is another path's resume and neither
                     * proves nor contradicts this bounded return. */
                    continue;
                }
                /* A non-resume row directly after an RTS clears the
                 * pending window; that RTS stays an opaque row. */
                return_pending = 0;
            }
            consumed = 0;
            if (sscanf(line,
                       "main_ram_loader_rts logical_pc=%x physical_pc=%x%n",
                       &return_instruction_pc,
                       &return_instruction_physical_pc, &consumed) == 2 &&
                consumed == (int)length &&
                return_instruction_pc <= UINT16_MAX &&
                return_instruction_physical_pc >= 0x1f0000u &&
                return_instruction_physical_pc < 0x1f8000u) {
                return_pending = 1;
            }
        }
        if (!anchor_seen || matching_return_count != 1u) return 0;

        /* Pass C: after the exact resume row, the next source-adjacent
         * FIFO byte's receipt row must re-verify against the hash-verified
         * media, and its consumer row must carry the expected loop
         * sequence joined to that receipt's fifo_sequence and main-RAM
         * destination. The reader must be the resumed loader path itself:
         * a read anywhere else is not this loop iteration's read. */
        cursor = capture;
        anchor_seen = 0;
        while (tqr_trace_next_line(&cursor, &line, &length)) {
            if (!anchor_seen) {
                consumed = 0;
                if (sscanf(line,
                           "main_ram_loader_post_rts source_logical_pc=%x source_physical_pc=%x logical_pc=%x physical_pc=%x opcode=%x%n",
                           &source_pc, &source_physical_pc, &logical_pc,
                           &physical_pc, &opcode, &consumed) == 5 &&
                    consumed == (int)length &&
                    source_pc == current.return_instruction_pc &&
                    source_physical_pc ==
                        current.return_instruction_physical_pc &&
                    logical_pc == current.post_return_pc &&
                    physical_pc == current.post_return_physical_pc &&
                    opcode == current.post_return_opcode) {
                    anchor_seen = 1;
                }
                continue;
            }
            if (!receipt_seen) {
                consumed = 0;
                if (sscanf(line,
                           "pce_cd_fifo_origin_main_ram_receipt generation=%u source_lba=%u source_offset=%u fifo_sequence=%llu reader_pc=%x logical_destination=%x physical_destination=%x writer_pc=%x writer_physical_pc=%x value=%x%n",
                           &origin_generation, &origin_lba, &origin_offset,
                           &origin_fifo_sequence, &origin_reader_pc,
                           &origin_logical_destination, &origin_destination,
                           &origin_writer_pc, &origin_writer_physical_pc,
                           &origin_value, &consumed) != 10 ||
                    consumed != (int)length) {
                    continue;
                }
                if (origin_generation != third.consumer_generation ||
                    origin_lba != third.consumer_lba ||
                    origin_offset != expected_offset) {
                    /* A FIFO transfer of any other byte is not this loop
                     * iteration's receipt and remains an opaque row. */
                    continue;
                }
                if (origin_value > UINT8_MAX ||
                    !theron_v1_raw_loader_trace_track02_byte_for_scsi_source(
                        track02_data, track02_size, track02_md5, origin_lba,
                        origin_offset, &source_record, &source_byte) ||
                    source_byte != (uint8_t)origin_value) {
                    return 0;
                }
                receipt_seen = 1;
                continue;
            }
            consumed = 0;
            if (sscanf(line,
                       "pce_cd_fifo_origin_main_ram_consumer sequence=%u generation=%u source_lba=%u source_offset=%u fifo_sequence=%llu logical_address=%x physical_address=%x value=%x reader_pc=%x reader_physical_pc=%x%n",
                       &consumer_sequence, &consumer_generation,
                       &consumer_lba, &consumer_offset,
                       &consumer_fifo_sequence, &consumer_logical_address,
                       &consumer_physical, &consumer_value,
                       &consumer_reader_pc, &consumer_reader_physical_pc,
                       &consumed) != 10 || consumed != (int)length) {
                continue;
            }
            if (consumer_generation != third.consumer_generation ||
                consumer_lba != third.consumer_lba ||
                consumer_offset != expected_offset ||
                consumer_value != source_byte) {
                /* A consumer read of a different FIFO byte is not this
                 * byte's consumer and neither proves nor contradicts the
                 * joined read. */
                continue;
            }
            if (consumer_sequence != expected_sequence ||
                consumer_fifo_sequence != origin_fifo_sequence ||
                consumer_physical != origin_destination ||
                consumer_physical < 0x1f0000u ||
                consumer_physical >= 0x1f8000u ||
                consumer_reader_pc > UINT16_MAX ||
                consumer_reader_physical_pc < 0x1f0000u ||
                consumer_reader_physical_pc >= 0x1f8000u) {
                /* The exact loop byte was consumed, but out of order, by
                 * a different transfer or destination, or by a
                 * non-main-RAM (System Card) reader. That contradicts a
                 * loop-continuation consumer read; fail closed. */
                return 0;
            }
            if (consumer_reader_pc != current.post_return_pc) {
                /* The byte reached main RAM but a reader other than the
                 * resumed loader path consumed it; the loop back-edge is
                 * contradicted, so fail closed. */
                return 0;
            }
            current.track02_record = source_record;
            current.source_offset = (uint16_t)consumer_offset;
            current.source_byte = source_byte;
            current.consumer_physical_address = consumer_physical;
            current.consumer_reader_pc = (uint16_t)consumer_reader_pc;
            current.consumer_reader_physical_pc =
                consumer_reader_physical_pc;
            consumer_seen = 1;
            break;
        }
        if (!anchor_seen || !receipt_seen || !consumer_seen) return 0;
        out->iterations[iteration] = current;
        continuation = cursor;
    }

    out->valid = 1;
    out->consumer = third;
    out->consumer_generation = third.consumer_generation;
    out->consumer_lba = third.consumer_lba;
    out->first_source_offset = (uint16_t)first_offset;
    out->loop_continuation_proven = 1;
    out->level_or_object_semantics_proven = 0;
    return 1;
}

int theron_v1_raw_loader_trace_correlate_game_payload_initial_envelope_header(
    const Theron_V1RawLoaderTraceGamePayloadReceipt *payloads,
    size_t payload_count, const uint8_t *track02_data, size_t track02_size,
    const char *track02_md5,
    Theron_V1RawLoaderTraceInitialEnvelopeHeaderReceipt *out)
{
    Theron_V1RawLoaderTraceInitialEnvelopeByteReceipt byte_receipt;
    size_t index;

    if (out) memset(out, 0, sizeof(*out));
    if (!payloads || !track02_data || !track02_md5 || !out ||
        payload_count != THERON_V1_RAW_LOADER_INITIAL_ENVELOPE_HEADER_BYTES) {
        return 0;
    }
    for (index = 0u; index < payload_count; ++index) {
        if (!theron_v1_raw_loader_trace_correlate_game_payload_initial_envelope(
                &payloads[index], track02_data, track02_size, track02_md5,
                &byte_receipt) || !byte_receipt.valid ||
            byte_receipt.envelope_offset != index) {
            return 0;
        }
        if (index == 0u) {
            out->variant = byte_receipt.variant;
            out->track02_record = byte_receipt.track02_record;
            out->raw_sector = byte_receipt.raw_sector;
            out->dispatch_sequence = payloads[index].dispatch_sequence;
            out->scsi_generation = payloads[index].scsi_generation;
            out->scsi_lba = payloads[index].scsi_lba;
            out->scsi_sector_count = payloads[index].scsi_sector_count;
        } else if (payloads[index].dispatch_sequence !=
                       out->dispatch_sequence ||
                   payloads[index].scsi_generation != out->scsi_generation ||
                   payloads[index].scsi_lba != out->scsi_lba ||
                   payloads[index].scsi_sector_count !=
                       out->scsi_sector_count) {
            return 0;
        }
        out->bytes[index] = byte_receipt.source_byte;
    }
    snprintf(out->track02_md5, sizeof(out->track02_md5), "%s", track02_md5);
    out->bytes_hash = tqr_trace_fnv1a_bytes(out->bytes, sizeof(out->bytes));
    if (!out->bytes_hash) return 0;
    out->valid = 1;
    out->contiguous_capture_chain_verified = 1;
    out->header_semantics_proven = 0;
    return 1;
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
    Theron_V1Stage3Irq2DispatchReceipt stage3_dispatch;
    Theron_V1Stage3ManifestEvidence manifest;
    Theron_V1Stage3DescriptorRecordBoundary descriptor_boundary;
    const char *cursor;
    const char *line;
    size_t length;
    size_t source_count = 0u;
    size_t dynamic_count = 0u;
    size_t stage3_resume_count = 0u;
    size_t dispatch_count = 0u;
    size_t sector_count = 0u;
    size_t destination_count = 0u;
    size_t destination_payload_count = 0u;
    size_t return_count = 0u;
    size_t post_return_count = 0u;
    size_t dynamic_line = 0u;
    size_t stage3_resume_line = 0u;
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
    unsigned int stage3_entry_pc = 0u;
    unsigned int stage3_selector = 0u;
    unsigned int stage3_continuation_pc = 0u;
    unsigned int stage3_resumed_pc = 0u;
    unsigned int stage3_next_pc = 0u;
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
        !theron_v1_stage3_irq2_dispatch_from_original_media(
            track02_data, track02_size, &payload, &stage3_dispatch) ||
        !theron_v1_stage3_manifest_evidence_from_payload(
            track02_data, track02_size, &payload, &manifest)) return 0;

    memset(&descriptor_boundary, 0, sizeof(descriptor_boundary));
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
        } else if (length >= strlen("stage3_irq2_resume ") &&
                   memcmp(line, "stage3_irq2_resume ",
                          strlen("stage3_irq2_resume ")) == 0) {
            if (++stage3_resume_count != 1u ||
                !tqr_trace_parse_stage3_irq2_resume(
                    line, length, &stage3_entry_pc, &stage3_selector,
                    &stage3_continuation_pc, &stage3_resumed_pc,
                    &stage3_next_pc)) return 0;
            stage3_resume_line = line_number;
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
    if (source_count != 1u || dynamic_count != 1u ||
        stage3_resume_count != 1u || dispatch_count != 1u ||
        sector_count != 1u || destination_count != 1u ||
        destination_payload_count != 1u || return_count != 1u ||
        post_return_count != 1u ||
        !(dynamic_line < stage3_resume_line &&
          stage3_resume_line < dispatch_line && dispatch_line < sector_line &&
          sector_line < destination_line &&
          destination_line < destination_payload_line &&
          destination_payload_line < return_line &&
          return_line < post_return_line) ||
        return_pc != caller_pc + 3u || caller_opcode != 0x20u ||
        caller_target != 0xe009u ||
        return_caller_pc != caller_pc || return_return_pc != return_pc ||
        return_record != record || destination_caller_pc != caller_pc ||
        stage3_entry_pc != stage3_dispatch.entry_address ||
        stage3_selector != stage3_dispatch.irq2_selector ||
        stage3_continuation_pc != stage3_dispatch.continuation_address ||
        stage3_resumed_pc != stage3_dispatch.continuation_address ||
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
    if (ordinal == manifest.descriptor_count ||
        !theron_v1_stage3_descriptor_record_boundary_from_manifest(
            track02_data, track02_size, &manifest, ordinal,
            &descriptor_boundary) ||
        !descriptor_boundary.valid ||
        !descriptor_boundary.record_coordinate_proven ||
        !descriptor_boundary.mode1_user_data_proven ||
        !descriptor_boundary.descriptor_source_bytes_proven ||
        !descriptor_boundary.selector_aliases_proven ||
        descriptor_boundary.descriptor_semantics_proven ||
        descriptor_boundary.descriptor.word2 != selector ||
        descriptor_boundary.resolved_track02_record != record) return 0;

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
    out->stage3_entry_pc = (uint16_t)stage3_entry_pc;
    out->stage3_irq2_selector = (uint8_t)stage3_selector;
    out->stage3_continuation_pc = (uint16_t)stage3_continuation_pc;
    out->stage3_post_irq2_next_pc = (uint16_t)stage3_next_pc;
    out->stage3_post_irq2_resume_verified = 1;
    out->later_track02_record = record;
    out->descriptor_selector = (uint16_t)selector;
    out->descriptor_selector_ordinal = ordinal;
    out->descriptor_word0 = descriptor_boundary.descriptor.word0;
    out->descriptor_word1 = descriptor_boundary.descriptor.word1;
    out->descriptor_record_user_data_hash =
        descriptor_boundary.user_data_hash;
    out->descriptor_row_media_bound = 1;
    out->descriptor_semantics_proven = 0;
    out->descriptor_source_raw_offset =
        descriptor_boundary.descriptor_source_raw_offset;
    out->descriptor_source_bytes = descriptor_boundary.descriptor_source_bytes;
    out->descriptor_source_hash = descriptor_boundary.descriptor_source_hash;
    out->descriptor_source_bytes_proven = 1;
    out->descriptor_selector_occurrence_count =
        descriptor_boundary.selector_occurrence_count;
    out->descriptor_selector_first_ordinal =
        descriptor_boundary.selector_first_ordinal;
    out->descriptor_selector_last_ordinal =
        descriptor_boundary.selector_last_ordinal;
    out->descriptor_selector_row_hash = descriptor_boundary.selector_row_hash;
    out->descriptor_selector_aliases_proven = 1;
    out->caller_pc = (uint16_t)caller_pc;
    out->return_pc = (uint16_t)return_pc;
    out->later_caller_opcode = (uint8_t)caller_opcode;
    out->later_caller_target = (uint16_t)caller_target;
    out->later_caller_control_verified = 1;
    out->later_record_cl = (uint8_t)cl;
    out->later_record_dl = (uint8_t)dl;
    out->later_record_ch = (uint8_t)ch;
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
    hash ^= receipt->descriptor_word0;
    hash *= 16777619u;
    hash ^= receipt->descriptor_word1;
    hash *= 16777619u;
    hash ^= receipt->descriptor_record_user_data_hash;
    hash *= 16777619u;
    hash ^= receipt->descriptor_row_media_bound ? 1u : 0u;
    hash *= 16777619u;
    hash ^= receipt->descriptor_semantics_proven ? 1u : 0u;
    hash *= 16777619u;
    hash ^= (uint32_t)receipt->descriptor_source_raw_offset;
    hash *= 16777619u;
    hash ^= (uint32_t)receipt->descriptor_source_bytes;
    hash *= 16777619u;
    hash ^= receipt->descriptor_source_hash;
    hash *= 16777619u;
    hash ^= receipt->descriptor_source_bytes_proven ? 1u : 0u;
    hash *= 16777619u;
    hash ^= (uint32_t)receipt->descriptor_selector_occurrence_count;
    hash *= 16777619u;
    hash ^= (uint32_t)receipt->descriptor_selector_first_ordinal;
    hash *= 16777619u;
    hash ^= (uint32_t)receipt->descriptor_selector_last_ordinal;
    hash *= 16777619u;
    hash ^= receipt->descriptor_selector_row_hash;
    hash *= 16777619u;
    hash ^= receipt->descriptor_selector_aliases_proven ? 1u : 0u;
    hash *= 16777619u;
    hash ^= (uint32_t)receipt->complete_payload_bytes;
    hash *= 16777619u;
    hash ^= receipt->complete_payload_checksum;
    hash *= 16777619u;
    hash ^= receipt->complete_payload_witness_proven ? 1u : 0u;
    hash *= 16777619u;
    hash ^= receipt->initial_level_semantics_proven ? 1u : 0u;
    hash *= 16777619u;
    hash ^= receipt->loader_intake.observed ? 1u : 0u;
    hash *= 16777619u;
    hash ^= receipt->loader_intake.payload_intake_admitted ? 1u : 0u;
    hash *= 16777619u;
    hash ^= (uint32_t)receipt->loader_intake.track02_variant;
    hash *= 16777619u;
    hash ^= receipt->loader_intake.record;
    hash *= 16777619u;
    hash ^= receipt->loader_intake.record_user_data_offset;
    hash *= 16777619u;
    hash ^= receipt->loader_intake.observed_destination;
    hash *= 16777619u;
    hash ^= receipt->loader_intake.observed_byte_count;
    hash *= 16777619u;
    hash ^= receipt->loader_intake.observed_payload_checksum;
    hash *= 16777619u;
    hash ^= receipt->loader_payload.handed_off ? 1u : 0u;
    hash *= 16777619u;
    hash ^= receipt->loader_payload.no_fallback ? 1u : 0u;
    hash *= 16777619u;
    hash ^= (uint32_t)receipt->loader_payload.track02_variant;
    hash *= 16777619u;
    hash ^= receipt->loader_payload.record;
    hash *= 16777619u;
    hash ^= receipt->loader_payload.record_user_data_offset;
    hash *= 16777619u;
    hash ^= receipt->loader_payload.destination;
    hash *= 16777619u;
    hash ^= receipt->loader_payload.payload_bytes;
    hash *= 16777619u;
    hash ^= receipt->loader_payload.payload_checksum;
    hash *= 16777619u;
    hash ^= (uint32_t)receipt->loader_level_envelope.track02_variant;
    hash *= 16777619u;
    hash ^= receipt->loader_level_envelope.record_user_data_offset;
    hash *= 16777619u;
    hash ^= receipt->loader_level_envelope.envelope_bytes;
    hash *= 16777619u;
    hash ^= receipt->loader_level_envelope.envelope_checksum;
    hash *= 16777619u;
    hash ^= receipt->loader_post_envelope.handed_off ? 1u : 0u;
    hash *= 16777619u;
    hash ^= receipt->loader_post_envelope.no_fallback ? 1u : 0u;
    hash *= 16777619u;
    hash ^= (uint32_t)receipt->loader_post_envelope.track02_variant;
    hash *= 16777619u;
    hash ^= receipt->loader_post_envelope.record;
    hash *= 16777619u;
    hash ^= receipt->loader_post_envelope.record_user_data_offset;
    hash *= 16777619u;
    hash ^= receipt->loader_post_envelope.byte_count;
    hash *= 16777619u;
    hash ^= receipt->loader_post_envelope.checksum;
    hash *= 16777619u;
    hash ^= receipt->loader_semantic_gate.valid ? 1u : 0u;
    hash *= 16777619u;
    hash ^= receipt->loader_semantic_gate.no_fallback ? 1u : 0u;
    hash *= 16777619u;
    hash ^= receipt->loader_semantic_gate.real_payload_available ? 1u : 0u;
    hash *= 16777619u;
    hash ^= receipt->loader_semantic_gate.level_envelope_available ? 1u : 0u;
    hash *= 16777619u;
    hash ^= receipt->loader_semantic_gate.post_envelope_available ? 1u : 0u;
    hash *= 16777619u;
    hash ^= (uint32_t)receipt->loader_semantic_gate.track02_variant;
    hash *= 16777619u;
    hash ^= receipt->loader_semantic_gate.record;
    hash *= 16777619u;
    hash ^= receipt->loader_semantic_gate.payload_checksum;
    hash *= 16777619u;
    hash ^= receipt->loader_semantic_gate.level_envelope_checksum;
    hash *= 16777619u;
    hash ^= receipt->loader_semantic_gate.post_envelope_checksum;
    hash *= 16777619u;
    hash ^= receipt->loader_semantic_gate.dungeon_record_semantics_proven ? 1u : 0u;
    hash *= 16777619u;
    hash ^= receipt->loader_semantic_gate.object_table_semantics_proven ? 1u : 0u;
    hash *= 16777619u;
    hash ^= receipt->loader_semantic_gate.bitmap_route_bound ? 1u : 0u;
    hash *= 16777619u;
    hash ^= receipt->loader_semantic_gate.palette_binding_verified ? 1u : 0u;
    hash *= 16777619u;
    hash ^= receipt->loader_semantic_gate.rgba_output_allowed ? 1u : 0u;
    hash *= 16777619u;
    hash ^= receipt->loader_semantic_gate.fallback_visuals_allowed ? 1u : 0u;
    hash *= 16777619u;
    hash ^= receipt->capture_manifest_bound ? 1u : 0u;
    hash *= 16777619u;
    hash ^= tqr_trace_fnv1a_bytes(
        (const uint8_t *)receipt->capture_manifest_system_card_md5,
        strlen(receipt->capture_manifest_system_card_md5));
    hash *= 16777619u;
    hash ^= tqr_trace_fnv1a_bytes(
        (const uint8_t *)receipt->capture_manifest_trace_md5,
        strlen(receipt->capture_manifest_trace_md5));
    hash *= 16777619u;
    hash ^= receipt->capture_manifest_binding_hash;
    hash *= 16777619u;
    return hash;
}

static int tqr_trace_is_lower_md5(const char *value) {
    size_t i;

    if (!value || strlen(value) != 32u) return 0;
    for (i = 0u; i < 32u; ++i) {
        if (!((value[i] >= '0' && value[i] <= '9') ||
              (value[i] >= 'a' && value[i] <= 'f'))) {
            return 0;
        }
    }
    return 1;
}

int theron_v1_raw_loader_trace_initial_level_handoff_is_complete(
    const Theron_V1RawLoaderTraceInitialLevelHandoffReceipt *receipt)
{
    return receipt && receipt->valid &&
           receipt->coalesced_loader_cd_receipt_proven &&
           receipt->initial_level_record_proven &&
           receipt->complete_initial_level_envelope_proven &&
           !receipt->initial_level_semantics_proven &&
           receipt->descriptor_row_media_bound &&
           !receipt->descriptor_semantics_proven &&
           receipt->descriptor_source_bytes_proven &&
           receipt->descriptor_source_bytes == 6u &&
           receipt->descriptor_source_hash != 0u &&
           receipt->descriptor_selector_aliases_proven &&
           receipt->descriptor_selector_occurrence_count != 0u &&
           receipt->descriptor_selector_first_ordinal <=
               receipt->descriptor_selector_ordinal &&
           receipt->descriptor_selector_ordinal <=
               receipt->descriptor_selector_last_ordinal &&
           receipt->descriptor_selector_row_hash != 0u &&
           receipt->descriptor_record_user_data_hash ==
               receipt->complete_payload_checksum &&
           receipt->complete_payload_witness_proven &&
           receipt->complete_payload_bytes == THERON_TRACK02_RAW_USER_DATA_BYTES &&
           receipt->complete_payload_checksum != 0u &&
           receipt->loader_intake.observed &&
           !receipt->loader_intake.payload_intake_admitted &&
           receipt->loader_intake.track02_variant == receipt->variant &&
           (receipt->loader_intake.track02_variant == THERON_TRACK02_VARIANT_JP_BIN ||
            receipt->loader_intake.track02_variant == THERON_TRACK02_VARIANT_US_BIN) &&
           receipt->loader_intake.record == receipt->observed_track02_record &&
           receipt->loader_intake.record_user_data_offset ==
               THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET &&
           receipt->loader_intake.observed_destination ==
               THERON_V1_INITIAL_ENVELOPE_DESTINATION &&
           receipt->loader_intake.observed_byte_count ==
               THERON_V1_INITIAL_ENVELOPE_PAYLOAD_BYTES &&
           receipt->loader_intake.observed_payload_checksum ==
               receipt->complete_payload_checksum &&
           receipt->loader_payload.handed_off &&
           receipt->loader_payload.no_fallback &&
           receipt->loader_payload.track02_variant == receipt->loader_intake.track02_variant &&
           receipt->loader_payload.record == receipt->loader_intake.record &&
           receipt->loader_payload.record_user_data_offset ==
               receipt->loader_intake.record_user_data_offset &&
           receipt->loader_payload.destination ==
               receipt->loader_intake.observed_destination &&
           receipt->loader_payload.payload_bytes == receipt->complete_payload_bytes &&
           receipt->loader_payload.payload_checksum ==
               receipt->complete_payload_checksum &&
           receipt->loader_level_envelope.handed_off &&
           receipt->loader_level_envelope.no_fallback &&
           receipt->loader_level_envelope.track02_variant ==
               receipt->loader_payload.track02_variant &&
           receipt->loader_level_envelope.record == receipt->loader_payload.record &&
           receipt->loader_level_envelope.record_user_data_offset ==
               receipt->initial_level_boundary.level_user_data_offset_in_record &&
           receipt->loader_level_envelope.envelope_bytes ==
               receipt->initial_level_boundary.level_byte_count &&
           receipt->loader_level_envelope.envelope_checksum ==
               receipt->initial_level_boundary.level_payload_hash &&
           receipt->loader_post_envelope.handed_off &&
           receipt->loader_post_envelope.no_fallback &&
           receipt->loader_post_envelope.track02_variant ==
               receipt->loader_payload.track02_variant &&
           receipt->loader_post_envelope.record == receipt->loader_payload.record &&
           receipt->loader_post_envelope.record_user_data_offset ==
               receipt->initial_level_boundary.object_boundary_user_data_offset_in_record &&
           receipt->loader_post_envelope.byte_count ==
               receipt->initial_level_boundary.following_user_data_bytes_in_record &&
           receipt->loader_post_envelope.byte_count ==
               THERON_V1_INITIAL_LEVEL_POST_ENVELOPE_BYTES &&
           receipt->loader_post_envelope.checksum ==
               receipt->initial_level_boundary.following_user_data_hash &&
           tqr_trace_fnv1a_bytes(receipt->loader_post_envelope.bytes,
                                 receipt->loader_post_envelope.byte_count) ==
               receipt->loader_post_envelope.checksum &&
           receipt->loader_semantic_gate.valid &&
           receipt->loader_semantic_gate.no_fallback &&
           receipt->loader_semantic_gate.real_payload_available &&
           receipt->loader_semantic_gate.level_envelope_available &&
           receipt->loader_semantic_gate.post_envelope_available &&
           receipt->loader_semantic_gate.track02_variant == receipt->variant &&
           receipt->loader_semantic_gate.record == receipt->observed_track02_record &&
           receipt->loader_semantic_gate.payload_checksum ==
               receipt->complete_payload_checksum &&
           receipt->loader_semantic_gate.level_envelope_checksum ==
               receipt->loader_level_envelope.envelope_checksum &&
           receipt->loader_semantic_gate.post_envelope_checksum ==
               receipt->loader_post_envelope.checksum &&
           !receipt->loader_semantic_gate.dungeon_record_semantics_proven &&
           !receipt->loader_semantic_gate.object_table_semantics_proven &&
           !receipt->loader_semantic_gate.bitmap_route_bound &&
           !receipt->loader_semantic_gate.palette_binding_verified &&
           !receipt->loader_semantic_gate.rgba_output_allowed &&
           !receipt->loader_semantic_gate.fallback_visuals_allowed &&
           receipt->receipt_hash != 0u &&
           receipt->receipt_hash == tqr_trace_initial_level_handoff_hash(receipt);
}

int theron_v1_raw_loader_trace_manifest_initial_level_handoff_is_complete(
    const Theron_V1RawLoaderTraceInitialLevelHandoffReceipt *receipt) {
    return theron_v1_raw_loader_trace_initial_level_handoff_is_complete(receipt) &&
           receipt->capture_manifest_bound &&
           strcmp(receipt->capture_manifest_system_card_md5,
                  "ff1a674273fe3540ccef576376407d1d") == 0 &&
           tqr_trace_is_lower_md5(receipt->capture_manifest_trace_md5) &&
           receipt->capture_manifest_binding_hash != 0u;
}

int theron_v1_raw_loader_trace_bind_capture_manifest_to_initial_level_handoff(
    const Theron_V1RawLoaderTraceInitialLevelHandoffReceipt *source,
    const Theron_V1CaptureManifest *manifest,
    const char *track02_path,
    const char *track02_md5,
    const char *system_card_path,
    const char *system_card_md5,
    const char *trace_path,
    const char *trace_md5,
    Theron_V1RawLoaderTraceInitialLevelHandoffReceipt *out) {
    Theron_V1RawLoaderTraceInitialLevelHandoffReceipt source_copy;
    Theron_V1RawLoaderTraceInitialLevelHandoffReceipt receipt;
    uint32_t binding_hash = 2166136261u;

    if (!source || !manifest || !out) return 0;
    /* Callers may atomically enrich a receipt in place. Preserve the source
     * before clearing `out`, otherwise source == out destroys the evidence
     * that this function is meant to bind. */
    source_copy = *source;
    if (source != out) memset(out, 0, sizeof(*out));
    if (!theron_v1_raw_loader_trace_initial_level_handoff_is_complete(
            &source_copy) ||
        !theron_v1_raw_loader_trace_capture_manifest_matches(
            manifest, track02_path, track02_md5, system_card_path,
            system_card_md5, trace_path, trace_md5) ||
        strcmp(source_copy.track02_md5, track02_md5) != 0) {
        return 0;
    }

    receipt = source_copy;
    receipt.capture_manifest_bound = 1;
    snprintf(receipt.capture_manifest_system_card_md5,
             sizeof(receipt.capture_manifest_system_card_md5), "%s",
             system_card_md5);
    snprintf(receipt.capture_manifest_trace_md5,
             sizeof(receipt.capture_manifest_trace_md5), "%s", trace_md5);
    binding_hash ^= source_copy.receipt_hash;
    binding_hash *= 16777619u;
    binding_hash ^= tqr_trace_fnv1a_bytes((const uint8_t *)track02_md5,
                                          strlen(track02_md5));
    binding_hash *= 16777619u;
    binding_hash ^= tqr_trace_fnv1a_bytes((const uint8_t *)system_card_md5,
                                          strlen(system_card_md5));
    binding_hash *= 16777619u;
    binding_hash ^= tqr_trace_fnv1a_bytes((const uint8_t *)trace_md5,
                                          strlen(trace_md5));
    binding_hash *= 16777619u;
    if (binding_hash == 0u) return 0;
    receipt.capture_manifest_binding_hash = binding_hash;
    receipt.receipt_hash = tqr_trace_initial_level_handoff_hash(&receipt);
    if (!theron_v1_raw_loader_trace_manifest_initial_level_handoff_is_complete(
            &receipt)) {
        return 0;
    }
    *out = receipt;
    return 1;
}

int theron_v1_raw_loader_trace_bind_initial_level_handoff(
    const Theron_V1RawLoaderTraceCoalescedLaterReceipt *coalesced_receipt,
    const uint8_t *track02_data,
    size_t track02_size,
    const char *track02_md5,
    Theron_V1RawLoaderTraceInitialLevelHandoffReceipt *out)
{
    Theron_Track02InitialLevelObjectBoundaryReceipt boundary;
    Theron_V1Track02LoaderReadFacts loader_facts;
    Theron_V1Track02LoaderIntakeReceipt loader_intake;
    Theron_V1Track02LoaderPayloadReceipt loader_payload;
    Theron_V1Track02LoaderLevelEnvelopeReceipt loader_level_envelope;
    Theron_V1Track02LoaderPostEnvelopeReceipt loader_post_envelope;
    Theron_V1Track02LoaderSemanticGateReceipt loader_semantic_gate;
    Theron_Track02Stage2DynamicPayloadReceipt stage3_payload;
    Theron_V1Stage3ManifestEvidence stage3_manifest;
    Theron_V1Stage3DescriptorRecordBoundary descriptor_boundary;

    if (out) memset(out, 0, sizeof(*out));
    memset(&stage3_payload, 0, sizeof(stage3_payload));
    memset(&stage3_manifest, 0, sizeof(stage3_manifest));
    memset(&descriptor_boundary, 0, sizeof(descriptor_boundary));
    if (!coalesced_receipt || !track02_data || !track02_md5 || !out ||
        !coalesced_receipt->valid ||
        !coalesced_receipt->stage3_post_irq2_resume_verified ||
        coalesced_receipt->stage3_entry_pc != 0x3800u ||
        coalesced_receipt->stage3_irq2_selector != 0xffu ||
        coalesced_receipt->stage3_continuation_pc != 0x3802u ||
        !coalesced_receipt->observation_order_verified ||
        !coalesced_receipt->selector_sector_bytes_verified ||
        !coalesced_receipt->descriptor_row_media_bound ||
        coalesced_receipt->descriptor_semantics_proven ||
        !coalesced_receipt->descriptor_source_bytes_proven ||
        coalesced_receipt->descriptor_source_bytes != 6u ||
        coalesced_receipt->descriptor_source_hash == 0u ||
        !coalesced_receipt->descriptor_selector_aliases_proven ||
        coalesced_receipt->descriptor_selector_occurrence_count == 0u ||
        coalesced_receipt->descriptor_selector_first_ordinal >
            coalesced_receipt->descriptor_selector_ordinal ||
        coalesced_receipt->descriptor_selector_ordinal >
            coalesced_receipt->descriptor_selector_last_ordinal ||
        coalesced_receipt->descriptor_selector_row_hash == 0u ||
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
        theron_v1_track02_inspect_stage2_dynamic_payload(
            track02_data, track02_size, track02_md5, &stage3_payload) !=
            THERON_TRACK02_SIGNAL_OK ||
        !theron_v1_stage3_manifest_evidence_from_payload(
            track02_data, track02_size, &stage3_payload, &stage3_manifest) ||
        !theron_v1_stage3_descriptor_record_boundary_from_manifest(
            track02_data, track02_size, &stage3_manifest,
            coalesced_receipt->descriptor_selector_ordinal,
            &descriptor_boundary) ||
        !descriptor_boundary.valid ||
        !descriptor_boundary.record_coordinate_proven ||
        !descriptor_boundary.mode1_user_data_proven ||
        !descriptor_boundary.descriptor_source_bytes_proven ||
        !descriptor_boundary.selector_aliases_proven ||
        descriptor_boundary.descriptor_semantics_proven ||
        descriptor_boundary.descriptor.word2 !=
            coalesced_receipt->descriptor_selector ||
        descriptor_boundary.descriptor.word0 !=
            coalesced_receipt->descriptor_word0 ||
        descriptor_boundary.descriptor.word1 !=
            coalesced_receipt->descriptor_word1 ||
        descriptor_boundary.resolved_track02_record !=
            coalesced_receipt->later_track02_record ||
        descriptor_boundary.user_data_hash !=
            coalesced_receipt->descriptor_record_user_data_hash ||
        descriptor_boundary.descriptor_source_raw_offset !=
            coalesced_receipt->descriptor_source_raw_offset ||
        descriptor_boundary.descriptor_source_bytes !=
            coalesced_receipt->descriptor_source_bytes ||
        descriptor_boundary.descriptor_source_hash !=
            coalesced_receipt->descriptor_source_hash ||
        descriptor_boundary.selector_occurrence_count !=
            coalesced_receipt->descriptor_selector_occurrence_count ||
        descriptor_boundary.selector_first_ordinal !=
            coalesced_receipt->descriptor_selector_first_ordinal ||
        descriptor_boundary.selector_last_ordinal !=
            coalesced_receipt->descriptor_selector_last_ordinal ||
        descriptor_boundary.selector_row_hash !=
            coalesced_receipt->descriptor_selector_row_hash ||
        theron_v1_track02_capture_initial_level_object_boundary(
            track02_data, track02_size, track02_md5, &boundary) !=
            THERON_TRACK02_SIGNAL_OK ||
        !boundary.valid ||
        !boundary.promotion_blocked || boundary.object_table_parsed ||
        boundary.object_table_semantics_proven ||
        coalesced_receipt->later_track02_record != boundary.track02_record) {
        return 0;
    }

    memset(&loader_facts, 0, sizeof(loader_facts));
    memset(&loader_intake, 0, sizeof(loader_intake));
    memset(&loader_payload, 0, sizeof(loader_payload));
    memset(&loader_level_envelope, 0, sizeof(loader_level_envelope));
    memset(&loader_post_envelope, 0, sizeof(loader_post_envelope));
    memset(&loader_semantic_gate, 0, sizeof(loader_semantic_gate));
    loader_facts.authenticated_original_trace = 1;
    loader_facts.later_than_stage2_transfer = 1;
    loader_facts.track02_variant = coalesced_receipt->variant;
    loader_facts.track02_record = coalesced_receipt->later_track02_record;
    /* `0x114` is the source-locked byte coordinate within record 0x0b52,
     * retained by the raw-media boundary. Do not turn it into a global file
     * offset: the intake contract describes the loader's selected sector. */
    if (boundary.level_user_data_offset_in_record > UINT32_MAX) {
        return 0;
    }
    loader_facts.record_user_data_offset =
        (uint32_t)boundary.level_user_data_offset_in_record;
    loader_facts.destination = coalesced_receipt->later_local_destination;
    loader_facts.byte_count =
        (uint32_t)coalesced_receipt->later_destination_payload_bytes;
    loader_facts.complete_payload_witness_verified =
        coalesced_receipt->later_destination_payload_verified;
    loader_facts.complete_payload_checksum =
        coalesced_receipt->later_destination_payload_checksum;
    if (!theron_v1_track02_loader_intake_observe(&loader_facts,
                                                 &loader_intake)) {
        return 0;
    }

    /* CD_READ records are INDEX 01-relative.  The authenticated IPL receipt
     * supplies the corresponding physical sector in the raw BIN; using the
     * logical record as a file-sector offset silently reads the pregap on
     * discs that retain it. */
    if (boundary.level_first_raw_sector >
            track02_size / THERON_TRACK02_RAW_SECTOR_BYTES ||
        boundary.level_first_raw_sector * THERON_TRACK02_RAW_SECTOR_BYTES >
            track02_size ||
        THERON_TRACK02_RAW_USER_DATA_OFFSET >
            track02_size - boundary.level_first_raw_sector *
                THERON_TRACK02_RAW_SECTOR_BYTES ||
        coalesced_receipt->later_destination_span_bytes >
            track02_size - (boundary.level_first_raw_sector *
                THERON_TRACK02_RAW_SECTOR_BYTES +
                THERON_TRACK02_RAW_USER_DATA_OFFSET) ||
        tqr_trace_fnv1a_bytes(track02_data +
            boundary.level_first_raw_sector * THERON_TRACK02_RAW_SECTOR_BYTES +
            THERON_TRACK02_RAW_USER_DATA_OFFSET,
            coalesced_receipt->later_destination_span_bytes) !=
            coalesced_receipt->later_destination_span_checksum ||
        tqr_trace_fnv1a_bytes(track02_data +
            boundary.level_first_raw_sector * THERON_TRACK02_RAW_SECTOR_BYTES +
            THERON_TRACK02_RAW_USER_DATA_OFFSET,
            coalesced_receipt->later_destination_payload_bytes) !=
            coalesced_receipt->later_destination_payload_checksum) {
        return 0;
    }
    if (!theron_v1_track02_loader_intake_handoff_complete_payload(
            &loader_intake,
            track02_data + boundary.level_first_raw_sector *
                THERON_TRACK02_RAW_SECTOR_BYTES +
                THERON_TRACK02_RAW_USER_DATA_OFFSET,
            coalesced_receipt->later_destination_payload_bytes,
            &loader_payload)) {
        return 0;
    }
    if (boundary.level_user_data_offset_in_record > UINT32_MAX ||
        boundary.level_byte_count > UINT32_MAX ||
        !theron_v1_track02_loader_intake_handoff_level_envelope(
            &loader_payload, (uint32_t)boundary.level_user_data_offset_in_record,
            (uint32_t)boundary.level_byte_count, boundary.level_payload_hash,
            &loader_level_envelope)) {
        return 0;
    }
    if (boundary.object_boundary_user_data_offset_in_record !=
            THERON_V1_INITIAL_LEVEL_POST_ENVELOPE_OFFSET ||
        boundary.following_user_data_bytes_in_record !=
            THERON_V1_INITIAL_LEVEL_POST_ENVELOPE_BYTES ||
        !theron_v1_track02_loader_intake_handoff_initial_level_post_envelope(
            &loader_payload, boundary.following_user_data_hash,
            &loader_post_envelope)) {
        return 0;
    }
    if (!theron_v1_track02_loader_intake_semantic_gate(
            &loader_payload, &loader_level_envelope, &loader_post_envelope,
            &loader_semantic_gate)) {
        return 0;
    }

    out->valid = 1;
    out->variant = coalesced_receipt->variant;
    snprintf(out->track02_md5, sizeof(out->track02_md5), "%s", track02_md5);
    out->observed_track02_record = coalesced_receipt->later_track02_record;
    out->descriptor_selector = coalesced_receipt->descriptor_selector;
    out->descriptor_selector_ordinal =
        coalesced_receipt->descriptor_selector_ordinal;
    out->descriptor_word0 = coalesced_receipt->descriptor_word0;
    out->descriptor_word1 = coalesced_receipt->descriptor_word1;
    out->descriptor_record_user_data_hash =
        coalesced_receipt->descriptor_record_user_data_hash;
    out->descriptor_row_media_bound = 1;
    out->descriptor_semantics_proven = 0;
    out->descriptor_source_raw_offset =
        coalesced_receipt->descriptor_source_raw_offset;
    out->descriptor_source_bytes = coalesced_receipt->descriptor_source_bytes;
    out->descriptor_source_hash = coalesced_receipt->descriptor_source_hash;
    out->descriptor_source_bytes_proven = 1;
    out->descriptor_selector_occurrence_count =
        coalesced_receipt->descriptor_selector_occurrence_count;
    out->descriptor_selector_first_ordinal =
        coalesced_receipt->descriptor_selector_first_ordinal;
    out->descriptor_selector_last_ordinal =
        coalesced_receipt->descriptor_selector_last_ordinal;
    out->descriptor_selector_row_hash =
        coalesced_receipt->descriptor_selector_row_hash;
    out->descriptor_selector_aliases_proven = 1;
    out->coalesced_loader_cd_receipt_proven = 1;
    out->initial_level_record_proven = 1;
    out->complete_initial_level_envelope_proven = 1;
    out->initial_level_semantics_proven = 0;
    out->complete_payload_bytes =
        coalesced_receipt->later_destination_payload_bytes;
    out->complete_payload_checksum =
        coalesced_receipt->later_destination_payload_checksum;
    out->complete_payload_witness_proven = 1;
    out->loader_intake = loader_intake;
    out->loader_payload = loader_payload;
    out->loader_level_envelope = loader_level_envelope;
    out->loader_post_envelope = loader_post_envelope;
    out->loader_semantic_gate = loader_semantic_gate;
    out->initial_level_boundary = boundary;
    /* The source-bound `$3800` transfer is a loader/media fact. Keep the
     * historical route member zeroed until a captured game consumer proves
     * that any bytes in this sector belong to a dungeon record. */
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
        !media->startup_bitmap_raw_atlas_tile_count ||
        !media->startup_bitmap_atlas_nonzero_pixel_count ||
        !media->startup_bitmap_atlas_checksum) {
        return 0;
    }
    if (!media->startup_bitmap_soul_room_route_ready ||
        !(media->startup_bitmap_raw_route_mask &
          THERON_TRACK02_STARTUP_BITMAP_ROUTE_SOUL_ROOM) ||
        media->startup_bitmap_soul_room_atlas_tile_count == 0u ||
        media->startup_bitmap_soul_room_nonzero_pixel_count == 0u ||
        media->startup_bitmap_soul_room_checksum == 0u ||
        media->startup_bitmap_soul_room_first_raw_offset >
            media->startup_bitmap_soul_room_last_raw_offset ||
        media->startup_bitmap_soul_room_last_raw_offset >
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
    soul_room_end_raw_offset = media->startup_bitmap_soul_room_last_raw_offset +
        THERON_TRACK02_STARTUP_BITMAP_TILE_BYTES;
    if (!(dynamic_span_end_raw_offset <=
              media->startup_bitmap_soul_room_first_raw_offset ||
          soul_room_end_raw_offset <= dynamic_span_first_raw_offset)) {
        return 0;
    }
    *out = *trace;
    out->soul_room_raw_route_verified = 1;
    out->soul_room_first_raw_offset =
        media->startup_bitmap_soul_room_first_raw_offset;
    out->soul_room_last_raw_offset =
        media->startup_bitmap_soul_room_last_raw_offset;
    out->soul_room_checksum = media->startup_bitmap_soul_room_checksum;
    out->soul_room_route_disjoint_from_dynamic_span = 1;
    out->bitmap_route_mask = media->startup_bitmap_raw_route_mask;
    out->bitmap_atlas_checksum = media->startup_bitmap_atlas_checksum;
    return 1;
}
