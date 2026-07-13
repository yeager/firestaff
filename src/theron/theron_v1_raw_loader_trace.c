#include "theron_v1_raw_loader_trace.h"

#include "theron_v1_irq2_live_trace_gate.h"

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

int theron_v1_raw_loader_trace_ingest_mednafen_capture(
    const char *capture,
    const char *track02_md5,
    Theron_V1RawLoaderTraceReceipt *out)
{
    Theron_V1Irq2LiveTrace live_trace;
    const char *cursor;
    const char *line;
    size_t length;
    const char *dynamic_read;
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

    dynamic_read = strstr(capture, "dynamic_cd_read_transaction ");
    if (!dynamic_read) return 0;
    cursor = dynamic_read;
    out->palette_word_checksum = 2166136261u;
    while (tqr_trace_next_line(&cursor, &line, &length)) {
        if (length >= strlen("dynamic_cd_read_destination_span ") &&
            memcmp(line, "dynamic_cd_read_destination_span ",
                   strlen("dynamic_cd_read_destination_span ")) == 0) {
            if (out->dynamic_cd_read_destination_span_verified ||
                !tqr_trace_parse_cd_read_destination_span(
                    line, length, &destination_span_pc,
                    &destination_span_destination, &destination_span_bytes,
                    &destination_span_checksum)) return 0;
            out->dynamic_cd_read_destination_span_bytes =
                destination_span_bytes;
            out->dynamic_cd_read_destination_span_checksum =
                destination_span_checksum;
            out->dynamic_cd_read_destination_span_verified = 1;
        } else if (length >= strlen("dynamic_huc6260_palette_store ") &&
            memcmp(line, "dynamic_huc6260_palette_store ",
                   strlen("dynamic_huc6260_palette_store ")) == 0) {
            if (!tqr_trace_parse_palette_store(line, length, &pc, &address,
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
            if (!tqr_trace_parse_palette_word(line, length, &palette_index,
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
        !out->dynamic_cd_read_destination_span_verified) return 0;

    out->valid = 1;
    out->variant = live_trace.variant;
    out->dynamic_cd_read_record = live_trace.stage3_track02_record;
    out->dynamic_cd_read_destination = 0x3800u;
    out->dynamic_cd_read_verified = 1;
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

int theron_v1_raw_loader_trace_final_bind(
    const Theron_V1RawLoaderTraceReceipt *trace,
    const Theron_StartupMediaStateReceipt *media,
    Theron_V1RawLoaderTraceReceipt *out)
{
    if (out) memset(out, 0, sizeof(*out));
    if (!trace || !media || !out || !trace->valid ||
        !trace->dynamic_cd_read_verified ||
        !trace->dynamic_cd_read_destination_span_verified ||
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
    *out = *trace;
    out->bitmap_route_mask = media->startup_bitmap_raw_route_mask;
    out->bitmap_atlas_checksum = media->startup_bitmap_atlas_checksum;
    return 1;
}
