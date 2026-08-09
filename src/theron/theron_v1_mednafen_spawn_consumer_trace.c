#include "theron_v1_mednafen_spawn_consumer_trace.h"

#include <stdio.h>
#include <string.h>

static int main_ram_address(uint32_t address) {
    return address >= 0x1f0000u && address < 0x1f8000u;
}

/* HuC6280 physical code can execute from any 8-bit MPR bank.  The
 * $1fxxxx-only check belongs to data reads in the game main-RAM window, not
 * to the register sidecar's disassembly PC. */
static int huc6280_physical_address(uint32_t address) {
    return address <= 0x1fffffu;
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

static int parse_spawn_register_trace_file(
    const char *path, Theron_V1SpawnRegisterTraceReceipt *out,
    int require_runtime_edges) {
    FILE *file;
    char line[768];
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
        strcmp(line, "source=mednafen-pce-instrumented-spawn-registers-v3") != 0) {
        fclose(file);
        out->status = THERON_V1_SPAWN_CONSUMER_TRACE_REJECTED;
        return 0;
    }
    out->source_header_verified = 1;
    out->sequence_verified = 1;
    out->bank_coordinates_verified = 1;
    out->boundary_flags_verified = 1;

    while (read_line(file, line, sizeof(line))) {
        unsigned int sequence, pc, physical_pc;
        unsigned int a, x, y, sp, p, mpr0, mpr_pc;
        unsigned int b3, b4, b5, b6, b8, ba, bb;
        unsigned int c96b, cc4c, preconsumer, helper, spawn_entry;
        int consumed = 0;
        int expected_c96b, expected_cc4c, expected_spawn_entry;
        int expected_preconsumer, expected_helper;

        if (sscanf(line,
                   "spawn_consumer_registers sequence=%u pc=%x physical_pc=%x a=%x x=%x y=%x sp=%x p=%x mpr0=%x mpr_pc=%x b3=%x b4=%x b5=%x b6=%x b8=%x ba=%x bb=%x c96b_window=%u cc4c_window=%u preconsumer_4644=%u helper_4667=%u spawn_entry_b0e5=%u%n",
                   &sequence, &pc, &physical_pc, &a, &x, &y, &sp, &p,
                   &mpr0, &mpr_pc, &b3, &b4, &b5, &b6, &b8, &ba, &bb, &c96b,
                   &cc4c, &preconsumer, &helper, &spawn_entry, &consumed) != 22 ||
            line[consumed] != '\0' || pc > 0xffffu ||
            physical_pc > 0x1fffffu || a > 0xffu || x > 0xffu ||
            y > 0xffu || sp > 0xffu || p > 0xffu || mpr0 > 0xffu ||
            mpr_pc > 0xffu ||
            b3 > 0xffu || b4 > 0xffu || b5 > 0xffu || b6 > 0xffu ||
            b8 > 0xffu || ba > 0xffu || bb > 0xffu) {
            out->status = THERON_V1_SPAWN_CONSUMER_TRACE_REJECTED;
            fclose(file);
            return 0;
        }
        expected_c96b = c96b_window(pc);
        expected_cc4c = cc4c_window(pc);
        /* THQUEST.ASM: regular spawn entry LB0E5 at PCE $B0E5. */
        expected_spawn_entry = pc == 0xb0e5u;
        expected_preconsumer = pc == 0x4644u;
        expected_helper = pc == 0x4667u;
        if (sequence != expected_sequence ||
            !huc6280_physical_address(physical_pc) ||
            physical_pc != ((mpr_pc << 13u) | (pc & 0x1fffu)) ||
            c96b != (unsigned int)expected_c96b ||
            cc4c != (unsigned int)expected_cc4c ||
            preconsumer != (unsigned int)expected_preconsumer ||
            helper != (unsigned int)expected_helper ||
            spawn_entry != (unsigned int)expected_spawn_entry ||
            !(expected_c96b || expected_cc4c || expected_preconsumer ||
              expected_helper || expected_spawn_entry)) {
            out->sequence_verified = sequence == expected_sequence;
            out->bank_coordinates_verified =
                huc6280_physical_address(physical_pc);
            out->boundary_flags_verified =
                c96b == (unsigned int)expected_c96b &&
                cc4c == (unsigned int)expected_cc4c &&
                preconsumer == (unsigned int)expected_preconsumer &&
                helper == (unsigned int)expected_helper &&
                spawn_entry == (unsigned int)expected_spawn_entry;
            out->status = THERON_V1_SPAWN_CONSUMER_TRACE_REJECTED;
            fclose(file);
            return 0;
        }
        if (!saw_record) {
            out->first_pc = pc;
            out->first_physical_pc = physical_pc;
            saw_record = 1;
        }
        out->last_pc = pc;
        out->last_physical_pc = physical_pc;
        out->last_mpr_pc = (uint8_t)mpr_pc;
        out->last_a = (uint8_t)a;
        out->last_x = (uint8_t)x;
        out->last_y = (uint8_t)y;
        out->last_sp = (uint8_t)sp;
        out->last_p = (uint8_t)p;
        out->last_mpr0 = (uint8_t)mpr0;
        out->last_b3 = (uint8_t)b3;
        out->last_b4 = (uint8_t)b4;
        out->last_b5 = (uint8_t)b5;
        out->last_b6 = (uint8_t)b6;
        out->last_b8 = (uint8_t)b8;
        out->last_ba = (uint8_t)ba;
        out->last_bb = (uint8_t)bb;
        out->sample_count++;
        out->c96b_window_seen |= expected_c96b;
        out->cc4c_window_seen |= expected_cc4c;
        out->preconsumer_4644_seen |= expected_preconsumer;
        out->helper_4667_seen |= expected_helper;
        out->spawn_entry_b0e5_seen |= expected_spawn_entry;
        expected_sequence++;
    }
    fclose(file);
    if (!saw_record || !out->c96b_window_seen || !out->cc4c_window_seen ||
        (require_runtime_edges &&
         (!out->spawn_entry_b0e5_seen ||
          !out->preconsumer_4644_seen || !out->helper_4667_seen))) {
        out->status = THERON_V1_SPAWN_CONSUMER_TRACE_REJECTED;
        return 0;
    }
    out->status = THERON_V1_SPAWN_CONSUMER_TRACE_READY;
    return 1;
}

int theron_v1_mednafen_spawn_register_trace_parse_file(
    const char *path, Theron_V1SpawnRegisterTraceReceipt *out) {
    return parse_spawn_register_trace_file(path, out, 1);
}

int theron_v1_mednafen_spawn_register_trace_parse_execution_window_file(
    const char *path, Theron_V1SpawnRegisterTraceReceipt *out) {
    return parse_spawn_register_trace_file(path, out, 0);
}

int theron_v1_mednafen_spawn_capture_correlate_files(
    const char *consumer_path,
    const char *register_path,
    Theron_V1SpawnCaptureCorrelationReceipt *out) {
    Theron_V1SpawnConsumerTraceReceipt consumer;
    Theron_V1SpawnRegisterTraceReceipt registers;

    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    out->semantic_publication_allowed = 0;
    out->dynamic_return_contract_verified = 0;
    out->consumer_ready =
        theron_v1_mednafen_spawn_consumer_trace_parse_file(
            consumer_path, &consumer);
    out->registers_ready =
        theron_v1_mednafen_spawn_register_trace_parse_file(
            register_path, &registers);
    if (out->consumer_ready) {
        out->consumer_read_count = consumer.read_count;
    }
    if (out->registers_ready) {
        out->register_sample_count = registers.sample_count;
    }
    out->source_windows_paired =
        out->consumer_ready && out->registers_ready &&
        consumer.source_header_verified && registers.source_header_verified &&
        consumer.sequence_verified && registers.sequence_verified &&
        consumer.bank_coordinates_verified &&
        registers.bank_coordinates_verified &&
        consumer.boundary_flags_verified && registers.boundary_flags_verified &&
        consumer.c96b_window_seen && consumer.cc4c_window_seen &&
        registers.c96b_window_seen && registers.cc4c_window_seen &&
        registers.spawn_entry_b0e5_seen &&
        registers.preconsumer_4644_seen && registers.helper_4667_seen;
    out->ready = out->source_windows_paired;
    return out->ready;
}
