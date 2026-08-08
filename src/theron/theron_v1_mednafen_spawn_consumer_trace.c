#include "theron_v1_mednafen_spawn_consumer_trace.h"

#include <stdio.h>
#include <string.h>

static int main_ram_address(uint32_t address) {
    return address >= 0x1f0000u && address < 0x1f8000u;
}

static int c96b_window(uint32_t pc) {
    return pc >= 0xc96bu && pc <= 0xca69u;
}

static int cc4c_window(uint32_t pc) {
    return pc >= 0xcc4cu && pc <= 0xcd13u;
}

static int read_line(FILE *file, char *line, size_t capacity) {
    size_t length;
    if (!fgets(line, capacity, file)) return 0;
    length = strlen(line);
    if (length == 0u || line[length - 1u] != '\n') return 0;
    line[length - 1u] = '\0';
    return 1;
}

int theron_v1_mednafen_spawn_consumer_trace_parse_file(
    const char *path, Theron_V1SpawnConsumerTraceReceipt *out) {
    FILE *file;
    char line[512];
    unsigned int expected_sequence = 0u;
    int saw_record = 0;

    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    out->status = THERON_V1_SPAWN_CONSUMER_TRACE_UNAVAILABLE;
    out->semantic_publication_allowed = 0;
    if (!path || !path[0]) return 0;
    snprintf(out->source_trace_path, sizeof(out->source_trace_path), "%s", path);
    file = fopen(path, "rb");
    if (!file) return 0;
    if (!read_line(file, line, sizeof(line)) ||
        strcmp(line, "source=mednafen-pce-instrumented-spawn-consumer") != 0) {
        fclose(file);
        out->status = THERON_V1_SPAWN_CONSUMER_TRACE_REJECTED;
        return 0;
    }
    out->source_header_verified = 1;
    out->sequence_verified = 1;
    out->bank_coordinates_verified = 1;
    out->boundary_flags_verified = 1;
    while (read_line(file, line, sizeof(line))) {
        unsigned int sequence, value;
        unsigned int logical, physical, reader, reader_physical;
        unsigned int target_5d64, target_5d6a, c96b, cc4c;
        int consumed = 0;
        int target64, target6a, c96b_expected, cc4c_expected;
        if (sscanf(line,
                   "spawn_consumer_read sequence=%u logical_address=%x physical_address=%x value=%x reader_pc=%x reader_physical_pc=%x target_5d64=%u target_5d6a=%u c96b_window=%u cc4c_window=%u%n",
                   &sequence, &logical, &physical, &value, &reader,
                   &reader_physical, &target_5d64, &target_5d6a, &c96b,
                   &cc4c, &consumed) != 10 || line[consumed] != '\0' ||
            value > 0xffu || logical > 0xffffu || reader > 0xffffu) {
            out->status = THERON_V1_SPAWN_CONSUMER_TRACE_REJECTED;
            fclose(file);
            return 0;
        }
        target64 = logical == 0x5d64u;
        target6a = logical == 0x5d6au;
        c96b_expected = c96b_window(reader);
        cc4c_expected = cc4c_window(reader);
        if (sequence != expected_sequence || !main_ram_address(physical) ||
            !main_ram_address(reader_physical) ||
            (target_5d64 != (unsigned int)target64) ||
            (target_5d6a != (unsigned int)target6a) ||
            (c96b != (unsigned int)c96b_expected) ||
            (cc4c != (unsigned int)cc4c_expected) ||
            !(target64 || target6a || c96b_expected || cc4c_expected)) {
            out->sequence_verified = sequence == expected_sequence;
            out->bank_coordinates_verified =
                main_ram_address(physical) && main_ram_address(reader_physical);
            out->boundary_flags_verified =
                target_5d64 == (unsigned int)target64 &&
                target_5d6a == (unsigned int)target6a &&
                c96b == (unsigned int)c96b_expected &&
                cc4c == (unsigned int)cc4c_expected;
            out->status = THERON_V1_SPAWN_CONSUMER_TRACE_REJECTED;
            fclose(file);
            return 0;
        }
        if (!saw_record) {
            out->first_logical_address = logical;
            out->first_physical_address = physical;
            out->first_reader_pc = reader;
            out->first_reader_physical_pc = reader_physical;
            saw_record = 1;
        }
        out->last_logical_address = logical;
        out->last_physical_address = physical;
        out->last_reader_pc = reader;
        out->last_reader_physical_pc = reader_physical;
        out->read_count++;
        out->target_5d64_seen |= target64;
        out->target_5d6a_seen |= target6a;
        out->c96b_window_seen |= c96b_expected;
        out->cc4c_window_seen |= cc4c_expected;
        expected_sequence++;
    }
    fclose(file);
    if (!saw_record) {
        out->status = THERON_V1_SPAWN_CONSUMER_TRACE_REJECTED;
        return 0;
    }
    out->status = THERON_V1_SPAWN_CONSUMER_TRACE_READY;
    return 1;
}
