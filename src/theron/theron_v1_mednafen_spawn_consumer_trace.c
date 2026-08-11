#include "theron_v1_mednafen_spawn_consumer_trace.h"

#include <stdio.h>
#include <string.h>

#define THERON_US_TRACK02_BYTES 8104992u
#define THERON_US_RNG_CODE_OFFSET 0x975c4u
#define THERON_US_RNG_CODE_STRIDE 0x49800u
#define THERON_US_RNG_CODE_COPIES 7u
#define THERON_RNG_CODE_BYTES 256u
#define THERON_RNG_CODE_HEX_BYTES (THERON_RNG_CODE_BYTES * 2u)

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
        unsigned int return_pc, caller_pc;
        unsigned int c96b, cc4c, preconsumer, helper, spawn_entry;
        unsigned int caller_b07d = 0u;
        int consumed = 0;
        int caller_suffix_consumed = 0;
        int parsed_fields;
        int has_return_context = strstr(line, " return_pc=") != NULL;
        int has_caller_window = strstr(line, " caller_b07d_window=") != NULL;
        int expected_c96b, expected_cc4c, expected_spawn_address;
        int expected_spawn_entry, expected_caller_window;
        int expected_preconsumer, expected_helper;

        if (has_return_context) {
            parsed_fields = sscanf(
                line,
                "spawn_consumer_registers sequence=%u pc=%x physical_pc=%x a=%x x=%x y=%x sp=%x p=%x mpr0=%x mpr_pc=%x b3=%x b4=%x b5=%x b6=%x b8=%x ba=%x bb=%x return_pc=%x caller_pc=%x c96b_window=%u cc4c_window=%u preconsumer_4644=%u helper_4667=%u spawn_entry_b0e5=%u%n",
                &sequence, &pc, &physical_pc, &a, &x, &y, &sp, &p,
                &mpr0, &mpr_pc, &b3, &b4, &b5, &b6, &b8, &ba, &bb,
                &return_pc, &caller_pc, &c96b, &cc4c, &preconsumer, &helper,
                &spawn_entry, &consumed);
        } else {
            parsed_fields = sscanf(
                line,
                "spawn_consumer_registers sequence=%u pc=%x physical_pc=%x a=%x x=%x y=%x sp=%x p=%x mpr0=%x mpr_pc=%x b3=%x b4=%x b5=%x b6=%x b8=%x ba=%x bb=%x c96b_window=%u cc4c_window=%u preconsumer_4644=%u helper_4667=%u spawn_entry_b0e5=%u%n",
                &sequence, &pc, &physical_pc, &a, &x, &y, &sp, &p,
                &mpr0, &mpr_pc, &b3, &b4, &b5, &b6, &b8, &ba, &bb, &c96b,
                &cc4c, &preconsumer, &helper, &spawn_entry, &consumed);
        }
        if (parsed_fields != (has_return_context ? 24 : 22) ||
            (has_caller_window &&
             (sscanf(line + consumed, " caller_b07d_window=%u%n",
                     &caller_b07d, &caller_suffix_consumed) != 1 ||
              line[consumed + caller_suffix_consumed] != '\0')) ||
            (!has_caller_window && line[consumed] != '\0') ||
            pc > 0xffffu ||
            physical_pc > 0x1fffffu || a > 0xffu || x > 0xffu ||
            y > 0xffu || sp > 0xffu || p > 0xffu || mpr0 > 0xffu ||
            mpr_pc > 0xffu ||
            (has_return_context &&
             (return_pc > 0xffffu || caller_pc > 0xffffu)) ||
            b3 > 0xffu || b4 > 0xffu || b5 > 0xffu || b6 > 0xffu ||
            b8 > 0xffu || ba > 0xffu || bb > 0xffu) {
            out->status = THERON_V1_SPAWN_CONSUMER_TRACE_REJECTED;
            fclose(file);
            return 0;
        }
        expected_c96b = c96b_window(pc);
        expected_cc4c = cc4c_window(pc);
        /* THQUEST.ASM: regular spawn entry LB0E5 at PCE $B0E5. The
         * sidecar flag is an address hit; the disassembly dispatches only
         * categories 0..3 here. Keep those facts separate. */
        expected_spawn_address = pc == 0xb0e5u;
        expected_spawn_entry = expected_spawn_address && a <= 3u;
        expected_caller_window = pc >= 0xb07du && pc <= 0xb0e4u;
        expected_preconsumer = pc == 0x4644u;
        expected_helper = pc == 0x4667u;
        if (sequence != expected_sequence ||
            !huc6280_physical_address(physical_pc) ||
            physical_pc != ((mpr_pc << 13u) | (pc & 0x1fffu)) ||
            c96b != (unsigned int)expected_c96b ||
            cc4c != (unsigned int)expected_cc4c ||
            preconsumer != (unsigned int)expected_preconsumer ||
            helper != (unsigned int)expected_helper ||
            spawn_entry != (unsigned int)expected_spawn_address ||
            (has_caller_window &&
             caller_b07d != (unsigned int)expected_caller_window) ||
            !(expected_c96b || expected_cc4c || expected_preconsumer ||
              expected_helper || expected_spawn_address ||
              (has_caller_window && expected_caller_window))) {
            out->sequence_verified = sequence == expected_sequence;
            out->bank_coordinates_verified =
                huc6280_physical_address(physical_pc);
            out->boundary_flags_verified =
                c96b == (unsigned int)expected_c96b &&
                cc4c == (unsigned int)expected_cc4c &&
                preconsumer == (unsigned int)expected_preconsumer &&
                helper == (unsigned int)expected_helper &&
                spawn_entry == (unsigned int)expected_spawn_address &&
                (!has_caller_window ||
                 caller_b07d == (unsigned int)expected_caller_window);
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
        if (expected_caller_window) {
            out->caller_b07d_window_seen = 1;
            out->caller_b07d_window_samples++;
        }
        if (expected_helper && ((b3 & 0x07u) == 0x04u))
            out->helper_4667_special_branch_seen = 1;
        if (expected_spawn_address) {
            out->spawn_entry_b0e5_address_hits++;
            if (!expected_spawn_entry)
                out->spawn_entry_b0e5_invalid_category_samples++;
        }
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

int theron_v1_mednafen_rng_consumer_trace_parse_file(
    const char *path, Theron_V1RngConsumerTraceReceipt *out) {
    FILE *file;
    char line[768];
    char entry[16];
    unsigned int expected_step = 0u;
    unsigned int previous_sequence = 0u;
    unsigned int sample_limit = 0u;
    int sample_limit_consumed = 0;
    int saw_record = 0;
    int previous_window_complete = 0;

    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    out->status = THERON_V1_SPAWN_CONSUMER_TRACE_UNAVAILABLE;
    out->semantic_publication_allowed = 0;
    if (!path || !path[0]) return 0;
    snprintf(out->source_trace_path, sizeof(out->source_trace_path), "%s", path);
    file = fopen(path, "rb");
    if (!file) return 0;
    if (!read_line(file, line, sizeof(line)) ||
        strcmp(line, "source=mednafen-pce-instrumented-rng-consumer") != 0) {
        fclose(file);
        out->status = THERON_V1_SPAWN_CONSUMER_TRACE_REJECTED;
        return 0;
    }
    out->source_header_verified = 1;
    if (!read_line(file, line, sizeof(line)) ||
        sscanf(line, "rng_consumer_sample_limit=%u%n", &sample_limit,
               &sample_limit_consumed) != 1 ||
        line[sample_limit_consumed] != '\0' || sample_limit < 512u ||
        sample_limit > 65536u) {
        out->status = THERON_V1_SPAWN_CONSUMER_TRACE_REJECTED;
        fclose(file);
        return 0;
    }
    out->sample_limit = sample_limit;
    out->sequence_verified = 1;
    out->step_verified = 1;
    out->physical_pc_bounds_verified = 1;
    out->boundary_flags_verified = 1;

    while (read_line(file, line, sizeof(line))) {
        unsigned int sequence, step, pc, physical_pc;
        unsigned int a, x, y, sp, p, mpr0;
        unsigned int b3, b4, b5, b6, b8, ba, bb, entry_sp, return_pc;
        unsigned int return_boundary;
        int consumed = 0;
        int is_target_5d64, is_target_5d6a;
        int sequence_ok, step_ok, entry_ok;

        if (sscanf(line,
                   "rng_consumer_window sequence=%u step=%u pc=%x physical_pc=%x entry=%15s a=%x x=%x y=%x sp=%x p=%x mpr0=%x b3=%x b4=%x b5=%x b6=%x b8=%x ba=%x bb=%x entry_sp=%x return_pc=%x return_boundary=%u%n",
                   &sequence, &step, &pc, &physical_pc, entry, &a, &x, &y,
                   &sp, &p, &mpr0, &b3, &b4, &b5, &b6, &b8, &ba, &bb,
                   &entry_sp, &return_pc, &return_boundary, &consumed) != 21 ||
            line[consumed] != '\0' || sequence == 0u ||
            step >= out->sample_limit ||
            pc > 0xffffu || return_pc > 0xffffu || entry_sp > 0xffu ||
            return_boundary > 1u ||
            physical_pc > 0x1fffffu || a > 0xffu || x > 0xffu ||
            y > 0xffu || sp > 0xffu || p > 0xffu || mpr0 > 0xffu ||
            b3 > 0xffu || b4 > 0xffu || b5 > 0xffu || b6 > 0xffu ||
            b8 > 0xffu || ba > 0xffu || bb > 0xffu) {
            out->status = THERON_V1_SPAWN_CONSUMER_TRACE_REJECTED;
            fclose(file);
            return 0;
        }

        is_target_5d64 = pc == 0x5d64u;
        is_target_5d6a = pc == 0x5d6au;
        entry_ok = (is_target_5d64 && strcmp(entry, "5d64") == 0) ||
                   (is_target_5d6a && strcmp(entry, "5d6a") == 0) ||
                   (!is_target_5d64 && !is_target_5d6a &&
                    strcmp(entry, "window") == 0);
        if (!saw_record) {
            sequence_ok = sequence == 1u;
            step_ok = step == 0u;
        } else if (sequence == previous_sequence) {
            sequence_ok = 1;
            step_ok = step == expected_step;
        } else {
            sequence_ok = sequence == previous_sequence + 1u;
            step_ok = step == 0u && previous_window_complete;
        }
        if (!sequence_ok || !step_ok || !entry_ok) {
            out->sequence_verified &= sequence_ok;
            out->step_verified &= step_ok;
            out->boundary_flags_verified &= entry_ok;
            out->status = THERON_V1_SPAWN_CONSUMER_TRACE_REJECTED;
            fclose(file);
            return 0;
        }

        if (!saw_record) {
            out->first_sequence = sequence;
            out->first_step = step;
            out->first_pc = pc;
            out->first_physical_pc = physical_pc;
            out->window_count = 1u;
            saw_record = 1;
        } else if (sequence != previous_sequence) {
            if (previous_window_complete) out->complete_window_count++;
            out->window_count++;
        }
        out->last_sequence = sequence;
        out->last_step = step;
        out->last_pc = pc;
        out->last_physical_pc = physical_pc;
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
        out->last_entry_sp = (uint8_t)entry_sp;
        out->last_return_pc = (uint16_t)return_pc;
        out->sample_count++;
        out->target_5d64_seen |= is_target_5d64;
        out->target_5d6a_seen |= is_target_5d6a;
        expected_step = step + 1u;
        previous_sequence = sequence;
        previous_window_complete = step + 1u == out->sample_limit;
        out->complete_window_seen |= previous_window_complete;
        out->return_boundary_seen |= return_boundary != 0u;
    }
    fclose(file);
    if (previous_window_complete) out->complete_window_count++;
    if (!saw_record || (!out->target_5d64_seen && !out->target_5d6a_seen)) {
        out->status = THERON_V1_SPAWN_CONSUMER_TRACE_REJECTED;
        return 0;
    }
    out->status = THERON_V1_SPAWN_CONSUMER_TRACE_READY;
    return 1;
}

static int hex_digit(unsigned char value) {
    if (value >= (unsigned char)'0' && value <= (unsigned char)'9')
        return (int)(value - (unsigned char)'0');
    if (value >= (unsigned char)'a' && value <= (unsigned char)'f')
        return (int)(value - (unsigned char)'a') + 10;
    if (value >= (unsigned char)'A' && value <= (unsigned char)'F')
        return (int)(value - (unsigned char)'A') + 10;
    return -1;
}

static int parse_hex_window(const char *hex,
                            unsigned char out[THERON_RNG_CODE_BYTES]) {
    size_t i;
    if (!hex || strlen(hex) != THERON_RNG_CODE_HEX_BYTES) return 0;
    for (i = 0u; i < THERON_RNG_CODE_BYTES; ++i) {
        int high = hex_digit((unsigned char)hex[i * 2u]);
        int low = hex_digit((unsigned char)hex[i * 2u + 1u]);
        if (high < 0 || low < 0) return 0;
        out[i] = (unsigned char)((high << 4) | low);
    }
    return 1;
}

static int source_window_matches(
    FILE *track02, const unsigned char window[THERON_RNG_CODE_BYTES],
    uint32_t *out_offset) {
    unsigned int copy;
    unsigned char source[THERON_RNG_CODE_BYTES];
    for (copy = 0u; copy < THERON_US_RNG_CODE_COPIES; ++copy) {
        uint32_t offset = THERON_US_RNG_CODE_OFFSET +
                          (uint32_t)copy * THERON_US_RNG_CODE_STRIDE;
        if (fseek(track02, (long)offset, SEEK_SET) != 0 ||
            fread(source, 1u, sizeof(source), track02) != sizeof(source)) {
            return 0;
        }
        if (memcmp(source, window, sizeof(source)) == 0) {
            if (out_offset) *out_offset = offset;
            return 1;
        }
    }
    return 0;
}

int theron_v1_mednafen_rng_code_correlate_us_track02_file(
    const char *rng_code_trace_path,
    const char *track02_path,
    Theron_V1RngCodeSourceCorrelationReceipt *out) {
    FILE *trace = NULL;
    FILE *track02 = NULL;
    char line[1024];
    int saw_window = 0;

    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    out->status = THERON_V1_SPAWN_CONSUMER_TRACE_UNAVAILABLE;
    out->semantic_publication_allowed = 0;
    if (!rng_code_trace_path || !rng_code_trace_path[0] ||
        !track02_path || !track02_path[0]) return 0;
    snprintf(out->source_trace_path, sizeof(out->source_trace_path), "%s",
             rng_code_trace_path);
    snprintf(out->track02_path, sizeof(out->track02_path), "%s", track02_path);
    trace = fopen(rng_code_trace_path, "rb");
    track02 = fopen(track02_path, "rb");
    if (!trace || !track02) goto rejected;
    if (!read_line(trace, line, sizeof(line)) ||
        strcmp(line, "source=mednafen-pce-instrumented-rng-code-v1") != 0) {
        goto rejected;
    }
    out->source_header_verified = 1;
    out->format_verified = 1;
    if (fseek(track02, 0L, SEEK_END) != 0 ||
        ftell(track02) != (long)THERON_US_TRACK02_BYTES) goto rejected;
    out->track02_size_verified = 1;

    while (read_line(trace, line, sizeof(line))) {
        char entry[16];
        char hex[THERON_RNG_CODE_HEX_BYTES + 1u];
        unsigned int logical_pc, physical_pc, byte_count;
        unsigned char window[THERON_RNG_CODE_BYTES];
        uint32_t source_offset = 0u;
        int consumed = 0;
        int logical_ok;

        if (sscanf(line,
                   "rng_code_window entry=%15s logical_pc=%x physical_pc=%x bytes=%u hex=%512s%n",
                   entry, &logical_pc, &physical_pc, &byte_count, hex,
                   &consumed) != 5 || line[consumed] != '\0' ||
            logical_pc > 0xffffu || physical_pc > 0x1fffffu ||
            byte_count != THERON_RNG_CODE_BYTES ||
            !parse_hex_window(hex, window)) goto rejected;
        logical_ok = (logical_pc == 0x5d64u && strcmp(entry, "5d64") == 0) ||
                     (logical_pc == 0x5d6au && strcmp(entry, "5d6a") == 0);
        if (!logical_ok ||
            !source_window_matches(track02, window, &source_offset)) goto rejected;
        if (!saw_window) {
            out->first_source_offset = source_offset;
            out->first_logical_pc = logical_pc;
            out->first_physical_pc = physical_pc;
            saw_window = 1;
        }
        out->last_source_offset = source_offset;
        out->last_logical_pc = logical_pc;
        out->last_physical_pc = physical_pc;
        out->window_count++;
        out->source_match_count++;
        out->target_5d64_seen |= logical_pc == 0x5d64u;
        out->target_5d6a_seen |= logical_pc == 0x5d6au;
    }
    if (!saw_window) goto rejected;
    out->logical_pc_verified = 1;
    out->physical_pc_verified = 1;
    out->source_bytes_match_verified = out->source_match_count == out->window_count;
    out->status = THERON_V1_SPAWN_CONSUMER_TRACE_READY;
    fclose(trace);
    fclose(track02);
    return 1;

rejected:
    if (trace) fclose(trace);
    if (track02) fclose(track02);
    out->status = THERON_V1_SPAWN_CONSUMER_TRACE_REJECTED;
    return 0;
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
