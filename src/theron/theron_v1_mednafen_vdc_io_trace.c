#include "theron_v1_mednafen_vdc_io_trace.h"

#include <stdio.h>
#include <string.h>

static int read_line(FILE *file, char *line, size_t capacity) {
    size_t length;
    if (!fgets(line, capacity, file)) return 0;
    length = strlen(line);
    if (length == 0u || line[length - 1u] != '\n') return 0;
    line[length - 1u] = '\0';
    return 1;
}

static int reject(Theron_V1VdcIoTraceReceipt *receipt, FILE *file) {
    if (file) fclose(file);
    receipt->status = THERON_V1_VDC_IO_TRACE_REJECTED;
    receipt->semantic_publication_allowed = 0;
    return 0;
}

int theron_v1_mednafen_vdc_io_trace_parse_file(
    const char *path,
    Theron_V1VdcIoTraceReceipt *out) {
    FILE *file;
    char line[512];
    uint32_t expected_sequence = 0u;
    uint32_t previous_timestamp = 0u;
    int saw_record = 0;

    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    out->status = THERON_V1_VDC_IO_TRACE_UNAVAILABLE;
    out->semantic_publication_allowed = 0;
    if (!path || !path[0]) return 0;
    snprintf(out->source_trace_path, sizeof(out->source_trace_path), "%s", path);
    file = fopen(path, "rb");
    if (!file) return 0;

    if (!read_line(file, line, sizeof(line)) ||
        strcmp(line, "FIRESTAFF_THERON_VDC_IO_TRACE_V1") != 0 ||
        !read_line(file, line, sizeof(line)) ||
        strcmp(line, "source=mednafen-pce-instrumented-vdc-io") != 0) {
        return reject(out, file);
    }
    out->source_header_verified = 1;
    out->sequence_verified = 1;
    out->timestamp_verified = 1;
    out->address_bounds_verified = 1;
    out->register_bounds_verified = 1;

    while (1) {
        unsigned int sequence, timestamp, logical, physical, value;
        unsigned int writer_pc, writer_physical_pc;
        unsigned int a, x, y;
        int consumed = 0;
        int parsed;

        if (!fgets(line, sizeof(line), file)) break;
        size_t length = strlen(line);
        if (length == 0u || line[length - 1u] != '\n')
            return reject(out, file);
        line[length - 1u] = '\0';
        parsed = sscanf(
            line,
            "vdc_io_write sequence=%u timestamp=%u logical_address=%x physical_address=%x value=%x writer_pc=%x writer_physical_pc=%x a=%x x=%x y=%x%n",
            &sequence, &timestamp, &logical, &physical, &value, &writer_pc,
            &writer_physical_pc, &a, &x, &y, &consumed);
        if (parsed != 10 || line[consumed] != '\0' ||
            sequence >= THERON_V1_VDC_IO_TRACE_MAX_WRITES ||
            timestamp > UINT32_MAX || logical > 0xffffu ||
            physical > 0xffffffu || value > 0xffu || writer_pc > 0xffffu ||
            writer_physical_pc > 0x1fffffu || a > 0xffu || x > 0xffu ||
            y > 0xffu || sequence != expected_sequence ||
            (saw_record && timestamp < previous_timestamp)) {
            out->sequence_verified = sequence == expected_sequence;
            out->timestamp_verified = !saw_record || timestamp >= previous_timestamp;
            out->address_bounds_verified = logical <= 0xffffu &&
                                            physical <= 0xffffffu;
            out->register_bounds_verified = value <= 0xffu &&
                writer_pc <= 0xffffu && writer_physical_pc <= 0x1fffffu &&
                a <= 0xffu && x <= 0xffu && y <= 0xffu;
            return reject(out, file);
        }
        if (!saw_record) {
            out->first_timestamp = timestamp;
            out->first_logical_address = logical;
            out->first_physical_address = physical;
            out->first_writer_pc = (uint16_t)writer_pc;
            out->first_writer_physical_pc = writer_physical_pc;
            saw_record = 1;
        }
        out->last_timestamp = timestamp;
        out->last_logical_address = logical;
        out->last_physical_address = physical;
        out->last_writer_pc = (uint16_t)writer_pc;
        out->last_writer_physical_pc = writer_physical_pc;
        out->last_a = (uint8_t)a;
        out->last_x = (uint8_t)x;
        out->last_y = (uint8_t)y;
        out->write_count++;
        previous_timestamp = timestamp;
        expected_sequence++;
    }
    fclose(file);
    if (!saw_record) {
        out->status = THERON_V1_VDC_IO_TRACE_REJECTED;
        return 0;
    }
    out->status = THERON_V1_VDC_IO_TRACE_READY;
    return 1;
}
