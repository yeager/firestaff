#include "theron_v1_mednafen_vdc_io_trace.h"

#include <stdio.h>
#include <stdlib.h>
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

static int parse_file_internal(
    const char *path,
    Theron_V1VdcIoTraceReceipt *out,
    Theron_V1VdcIoWrite *writes) {
    FILE *file;
    char line[512];
    uint32_t expected_sequence = 0u;
    uint32_t previous_timestamp = 0u;
    int saw_record = 0;
    int saw_snapshot_boundary = 0;

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
    out->physical_bus_marker_verified = 1;
    out->register_bounds_verified = 1;

    while (1) {
        unsigned int sequence, timestamp, logical, physical, value;
        unsigned int writer_pc, writer_physical_pc;
        unsigned int a, x, y;
        int consumed = 0;
        int parsed;
        int timestamp_discontinuity;

        if (!fgets(line, sizeof(line), file)) break;
        size_t length = strlen(line);
        if (length == 0u || line[length - 1u] != '\n')
            return reject(out, file);
        line[length - 1u] = '\0';
        if (strncmp(line, "vdc_snapshot_boundary ", 22u) == 0) {
            unsigned int boundary_sequence, boundary_timestamp;
            int boundary_consumed = 0;
            if (saw_snapshot_boundary || !saw_record ||
                sscanf(line,
                       "vdc_snapshot_boundary sequence=%u timestamp=%u%n",
                       &boundary_sequence, &boundary_timestamp,
                       &boundary_consumed) != 2 ||
                line[boundary_consumed] != '\0' ||
                boundary_sequence != expected_sequence) {
                return reject(out, file);
            }
            saw_snapshot_boundary = 1;
            out->snapshot_boundary_verified = 1;
            out->snapshot_boundary_sequence = boundary_sequence;
            out->snapshot_boundary_timestamp = boundary_timestamp;
            continue;
        }
        if (saw_snapshot_boundary) return reject(out, file);
        parsed = sscanf(
            line,
            "vdc_io_write sequence=%u timestamp=%u logical_address=%x physical_address=%x value=%x writer_pc=%x writer_physical_pc=%x a=%x x=%x y=%x%n",
            &sequence, &timestamp, &logical, &physical, &value, &writer_pc,
            &writer_physical_pc, &a, &x, &y, &consumed);
        /* HuCPU.Timestamp() is a diagnostic, epoch-local clock.  A real
         * same-session capture proves that it can move backwards by a few
         * cycles as well as wrap to a new execution epoch.  Sequence is the
         * producer-order authority; retain and count timestamp regressions
         * without inventing a monotonicity contract. */
        timestamp_discontinuity = saw_record && timestamp < previous_timestamp;
        if (parsed != 10 || line[consumed] != '\0' ||
            sequence >= THERON_V1_VDC_IO_TRACE_MAX_WRITES ||
            timestamp > UINT32_MAX || logical > 0xffffu ||
            (physical & 0x7f000000u) != 0u ||
            (physical & 0x7fffffffu) > 0xffffffu ||
            value > 0xffu || writer_pc > 0xffffu ||
            writer_physical_pc > 0x1fffffu || a > 0xffu || x > 0xffu ||
            y > 0xffu || sequence != expected_sequence) {
            out->sequence_verified = sequence == expected_sequence;
            out->timestamp_verified = timestamp <= UINT32_MAX;
            out->address_bounds_verified = logical <= 0xffffu &&
                (physical & 0x7f000000u) == 0u &&
                (physical & 0x7fffffffu) <= 0xffffffu;
            out->physical_bus_marker_verified =
                (physical & 0x7f000000u) == 0u;
            out->register_bounds_verified = value <= 0xffu &&
                writer_pc <= 0xffffu && writer_physical_pc <= 0x1fffffu &&
                a <= 0xffu && x <= 0xffu && y <= 0xffu;
            return reject(out, file);
        }
        if (!saw_record) {
            out->first_timestamp = timestamp;
            out->first_logical_address = logical;
            out->first_physical_address = physical;
            out->first_normalized_physical_address =
                physical & 0x7fffffffu;
            out->first_writer_pc = (uint16_t)writer_pc;
            out->first_writer_physical_pc = writer_physical_pc;
            saw_record = 1;
            out->timestamp_epoch_count = 1u;
        }
        if (writes) {
            Theron_V1VdcIoWrite *write = &writes[sequence];
            write->sequence = sequence;
            write->timestamp = timestamp;
            write->logical_address = (uint16_t)logical;
            write->raw_physical_address = physical;
            write->normalized_physical_address = physical & 0x7fffffffu;
            write->value = (uint8_t)value;
            write->writer_pc = (uint16_t)writer_pc;
            write->writer_physical_pc = writer_physical_pc;
            write->a = (uint8_t)a;
            write->x = (uint8_t)x;
            write->y = (uint8_t)y;
        }
        if (timestamp_discontinuity) {
            out->timestamp_reset_count++;
            out->timestamp_epoch_count++;
        }
        out->last_timestamp = timestamp;
        out->last_logical_address = logical;
        out->last_physical_address = physical;
        out->last_normalized_physical_address = physical & 0x7fffffffu;
        if (physical & 0x80000000u) out->marked_physical_write_count++;
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

int theron_v1_mednafen_vdc_io_trace_parse_file(
    const char *path,
    Theron_V1VdcIoTraceReceipt *out) {
    return parse_file_internal(path, out, NULL);
}

int theron_v1_mednafen_vdc_io_trace_load_file(
    const char *path,
    Theron_V1VdcIoTrace *out) {
    Theron_V1VdcIoWrite *writes;
    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    writes = (Theron_V1VdcIoWrite *)calloc(
        THERON_V1_VDC_IO_TRACE_MAX_WRITES, sizeof(*writes));
    if (!writes) return 0;
    if (!parse_file_internal(path, &out->receipt, writes)) {
        free(writes);
        return 0;
    }
    out->writes = writes;
    out->write_count = out->receipt.write_count;
    return 1;
}

void theron_v1_mednafen_vdc_io_trace_free(Theron_V1VdcIoTrace *trace) {
    if (!trace) return;
    free(trace->writes);
    memset(trace, 0, sizeof(*trace));
}

int theron_v1_mednafen_vdc_io_verify_vram_snapshot(
    const Theron_V1VdcIoTrace *trace,
    const char *vram_path,
    Theron_V1VdcIoVramReplayReceipt *out) {
    static const uint16_t increments[4] = {1u, 32u, 64u, 128u};
    uint16_t registers[32] = {0};
    uint16_t replay[32768] = {0};
    uint8_t written[32768] = {0};
    uint8_t snapshot[65536];
    uint8_t selected = 0u;
    uint8_t vwr_low = 0u;
    FILE *file;

    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!trace || !trace->writes || !vram_path ||
        trace->write_count == 0u ||
        trace->write_count > THERON_V1_VDC_IO_TRACE_MAX_WRITES ||
        !trace->receipt.snapshot_boundary_verified ||
        trace->receipt.snapshot_boundary_sequence != trace->write_count ||
        !(file = fopen(vram_path, "rb"))) return 0;
    if (fread(snapshot, 1u, sizeof(snapshot), file) != sizeof(snapshot) ||
        fgetc(file) != EOF) {
        fclose(file);
        return 0;
    }
    fclose(file);

    for (size_t i = 0u; i < trace->write_count; ++i) {
        const Theron_V1VdcIoWrite *write = &trace->writes[i];
        uint32_t port = write->normalized_physical_address & 3u;
        if (port == 0u) {
            selected = write->value & 0x1fu;
        } else if (port == 2u) {
            if (selected == 2u) vwr_low = write->value;
            else registers[selected] =
                (uint16_t)((registers[selected] & 0xff00u) | write->value);
        } else if (port == 3u) {
            if (selected == 2u) {
                uint16_t address = registers[0] & 0x7fffu;
                replay[address] = (uint16_t)(vwr_low | (write->value << 8));
                written[address] = 1u;
                registers[0] = (uint16_t)(registers[0] +
                    increments[(registers[5] >> 11) & 3u]);
                out->vwr_commit_count++;
            } else {
                registers[selected] = (uint16_t)(
                    (registers[selected] & 0x00ffu) | (write->value << 8));
            }
        }
    }
    for (size_t address = 0u; address < 32768u; ++address) {
        uint16_t expected;
        if (!written[address]) continue;
        expected = (uint16_t)(snapshot[address * 2u] |
                              (snapshot[address * 2u + 1u] << 8));
        out->written_word_count++;
        if (replay[address] == expected) out->matched_word_count++;
        else out->mismatched_word_count++;
    }
    out->atomic_snapshot_verified = out->written_word_count > 0u &&
        out->mismatched_word_count == 0u;
    out->semantic_publication_allowed = 0;
    return out->atomic_snapshot_verified;
}
