#include "theron_v1_original_command_capture.h"

#include "asset_status_m12.h"

#include <stdio.h>
#include <string.h>

enum { COMMAND_WRITE_LIMIT = 65536u };

static int file_size_is(const char *path, long expected) {
    FILE *file = NULL;
    long size;
    if (!path || !(file = fopen(path, "rb")) ||
        fseek(file, 0, SEEK_END) != 0 || (size = ftell(file)) < 0) {
        if (file) fclose(file);
        return 0;
    }
    fclose(file);
    return size == expected;
}

static int receipt_identity_matches(
    const Theron_V1OriginalCommandCaptureRequest *request) {
    FILE *file;
    char line[1024];
    int source = 0, mednafen = 0, track02 = 0, system = 0, state = 0;
    if (!request->transition_receipt_path ||
        !(file = fopen(request->transition_receipt_path, "rb"))) return 0;
    while (fgets(line, sizeof(line), file)) {
        size_t n = strlen(line);
        while (n && (line[n - 1] == '\n' || line[n - 1] == '\r'))
            line[--n] = '\0';
        if (!strcmp(line, "source=authentic-mednafen-transition-receipt"))
            source = 1;
        else if (!strncmp(line, "mednafen_binary_md5=", 20))
            mednafen = request->expected_mednafen_md5 &&
                !strcmp(line + 20, request->expected_mednafen_md5);
        else if (!strncmp(line, "track02_md5=", 12))
            track02 = request->expected_track02_md5 &&
                !strcmp(line + 12, request->expected_track02_md5);
        else if (!strncmp(line, "system_card_md5=", 16))
            system = request->expected_system_card_md5 &&
                !strcmp(line + 16, request->expected_system_card_md5);
        else if (!strncmp(line, "autoload_state_md5=", 19))
            state = request->expected_autoload_state_md5 &&
                !strcmp(line + 19, request->expected_autoload_state_md5);
    }
    fclose(file);
    return source && mednafen && track02 && system && state;
}

static int verify_consumer_window(
    const char *path, uint8_t command_type, uint32_t *source_read_count) {
    FILE *file;
    char line[512];
    uint32_t expected_sequence = 0u;
    int header = 0, active = 0, completed = 0;
    if (!path || !source_read_count || !(file = fopen(path, "rb"))) return 0;
    *source_read_count = 0u;
    while (fgets(line, sizeof(line), file)) {
        unsigned int sequence, logical, physical, value, pc, physical_pc;
        unsigned int a, x, y, sp, p;
        int consumed = 0;
        size_t n = strlen(line);
        while (n && (line[n - 1] == '\n' || line[n - 1] == '\r'))
            line[--n] = '\0';
        if (!header) {
            if (strcmp(line,
                       "source=mednafen-pce-instrumented-main-ram-consumer"))
                goto reject;
            header = 1;
            continue;
        }
        if (sscanf(line,
                   "main_ram_consumer_read sequence=%u logical_address=%x physical_address=%x value=%x reader_pc=%x reader_physical_pc=%x a=%x x=%x y=%x sp=%x p=%x%n",
                   &sequence, &logical, &physical, &value, &pc, &physical_pc,
                   &a, &x, &y, &sp, &p, &consumed) != 11 ||
            line[consumed] != '\0' || sequence != expected_sequence++ ||
            logical > 0xffffu || physical > 0xffffffu || value > 0xffu ||
            pc > 0xffffu || physical_pc > 0xffffffu || a > 0xffu ||
            x > 0xffu || y > 0xffu || sp > 0xffu || p > 0xffu)
            goto reject;
        if (!active && !completed && logical == 0x2905u &&
            value == command_type && pc == 0xd34du &&
            physical_pc == 0x0db34du) {
            active = 1;
            continue;
        }
        if (active && logical >= 0x2600u && logical < 0x2800u)
            ++*source_read_count;
        if (active && logical == 0x2905u && value == 0u &&
            pc == 0xd34du && physical_pc == 0x0db34du) {
            active = 0;
            completed = 1;
        }
    }
    fclose(file);
    return header && completed && !active;
reject:
    fclose(file);
    return 0;
}

int theron_v1_original_command_capture_admit(
    const Theron_V1OriginalCommandCaptureRequest *request,
    Theron_V1OriginalCommandCaptureReceipt *out_receipt) {
    Theron_V1OriginalCommandCaptureReceipt receipt = {0};
    FILE *file;
    char line[512];
    uint32_t expected_sequence = 0u;
    int header = 0, source = 0, boundary = 0, input_edge = 0;
    int command_type_seen = 0, click_x_seen = 0, click_y_seen = 0;
    int completion_seen = 0;

    if (out_receipt) *out_receipt = receipt;
    if (!request || !out_receipt || !request->command_trace_path ||
        !receipt_identity_matches(request) ||
        !(file = fopen(request->command_trace_path, "rb"))) return 0;
    while (fgets(line, sizeof(line), file)) {
        size_t n = strlen(line);
        unsigned int sequence, logical, physical, value, pc, physical_pc;
        int consumed = 0;
        while (n && (line[n - 1] == '\n' || line[n - 1] == '\r'))
            line[--n] = '\0';
        if (!header) {
            if (strcmp(line, "FIRESTAFF_THERON_COMMAND_RAM_TRACE_V1")) goto reject;
            header = 1;
            continue;
        }
        if (!source) {
            if (strcmp(line, "source=mednafen-pce-instrumented-command-ram"))
                goto reject;
            source = 1;
            continue;
        }
        if (sscanf(line,
                   "input_buffer_write sequence=%u logical_address=%x value=%x writer_pc=%x writer_physical_pc=%x%n",
                   &sequence, &logical, &value, &pc, &physical_pc,
                   &consumed) == 5 && line[consumed] == '\0') {
            if ((logical != 0x28b8u && logical != 0x28b9u) || value > 0xffu ||
                pc > 0xffffu || physical_pc > 0xffffffu) goto reject;
            if (logical == 0x28b8u && value == 0x01u && pc == 0x44e5u &&
                physical_pc == 0x0d04e5u) input_edge = 1;
            continue;
        }
        if (sscanf(line,
                   "command_ram_write sequence=%u logical_address=%x physical_address=%x value=%x writer_pc=%x writer_physical_pc=%x%n",
                   &sequence, &logical, &physical, &value, &pc,
                   &physical_pc, &consumed) == 6 && line[consumed] == '\0') {
            if (boundary || sequence != expected_sequence ||
                logical > 0xffffu || physical < 0x1f0000u ||
                physical >= 0x1f2000u ||
                physical != 0x1f0000u + (logical & 0x1fffu) ||
                value > 0xffu || pc > 0xffffu || physical_pc > 0xffffffu)
                goto reject;
            if (sequence == 0u &&
                (logical != 0x28b8u || value != 0x01u || pc != 0x44e5u ||
                 physical_pc != 0x0d04e5u)) goto reject;
            if (!command_type_seen && logical == 0x2905u && value != 0u) {
                if (pc != 0xccdbu || physical_pc != 0x0dacdbu) goto reject;
                receipt.command_type = (uint8_t)value;
                command_type_seen = 1;
            } else if (!click_x_seen && logical == 0x28fdu) {
                receipt.click_x = (uint8_t)value;
                click_x_seen = 1;
            } else if (!click_y_seen && logical == 0x2901u) {
                receipt.click_y = (uint8_t)value;
                click_y_seen = 1;
            }
            if (!completion_seen && command_type_seen &&
                logical == 0x2905u && value == 0u) {
                if (pc != 0xd3a0u || physical_pc != 0x0db3a0u) goto reject;
                receipt.completion_sequence = sequence;
                completion_seen = 1;
            }
            ++expected_sequence;
            continue;
        }
        if (!strcmp(line, "command_ram_boundary sequence=65536")) {
            if (expected_sequence != COMMAND_WRITE_LIMIT) goto reject;
            boundary = 1;
            continue;
        }
        goto reject;
    }
    fclose(file);
    if (!header || !source || !boundary || !input_edge ||
        !command_type_seen || !click_x_seen || !click_y_seen ||
        !completion_seen || expected_sequence != COMMAND_WRITE_LIMIT ||
        !file_size_is(request->command_code_path, 65536L) ||
        !file_size_is(request->command_before_ram_path, 8192L) ||
        !file_size_is(request->command_after_ram_path, 8192L) ||
        !verify_consumer_window(request->main_ram_consumer_trace_path,
                                receipt.command_type,
                                &receipt.consumer_source_read_count) ||
        !m12_file_md5_hex(request->command_trace_path, receipt.trace_md5) ||
        !m12_file_md5_hex(request->command_code_path, receipt.code_md5) ||
        !m12_file_md5_hex(request->command_before_ram_path,
                          receipt.before_ram_md5) ||
        !m12_file_md5_hex(request->command_after_ram_path,
                          receipt.after_ram_md5) ||
        !m12_file_md5_hex(request->main_ram_consumer_trace_path,
                          receipt.consumer_trace_md5)) return 0;
    receipt.admitted = 1;
    receipt.source_identity_verified = 1;
    receipt.input_edge_verified = 1;
    receipt.command_window_verified = 1;
    receipt.consumer_window_verified = 1;
    receipt.snapshots_verified = 1;
    receipt.semantic_publication_allowed = 0;
    receipt.command_write_count = expected_sequence;
    *out_receipt = receipt;
    return 1;

reject:
    fclose(file);
    return 0;
}
