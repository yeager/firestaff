#include "theron_v1_mednafen_main_ram_consumer_trace.h"

#include "asset_status_m12.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#if defined(_WIN32)
#define THERON_V1_CONSUMER_LSTAT(path, info) stat((path), (info))
#else
#include <unistd.h>
#define THERON_V1_CONSUMER_LSTAT lstat
#endif

#define THERON_V1_CONSUMER_TRACE_MAX_BYTES (4u * 1024u * 1024u)

static int read_line(FILE *file, char *line, size_t capacity) {
    size_t length;
    if (!fgets(line, capacity, file)) return 0;
    length = strlen(line);
    while (length && (line[length - 1] == '\n' || line[length - 1] == '\r'))
        line[--length] = '\0';
    return length > 0;
}

static int reject(Theron_V1MednafenMainRamConsumerTraceReceipt *receipt) {
    receipt->status = THERON_V1_MEDNAFEN_MAIN_RAM_CONSUMER_TRACE_REJECTED;
    receipt->semantic_publication_allowed = 0;
    return 1;
}

int theron_v1_mednafen_main_ram_consumer_trace_parse_file(
    const char *path,
    Theron_V1MednafenMainRamConsumerTraceReceipt *out)
{
    Theron_V1MednafenMainRamConsumerTraceReceipt receipt = {0};
    struct stat info;
    FILE *file = NULL;
    char line[512];
    char md5[33];
    long file_size;
    int first_line = 1;

    if (!out) return 0;
    *out = receipt;
    if (!path || !path[0] || THERON_V1_CONSUMER_LSTAT(path, &info) != 0) {
        receipt.status = THERON_V1_MEDNAFEN_MAIN_RAM_CONSUMER_TRACE_UNAVAILABLE;
        *out = receipt;
        return 1;
    }
#if !defined(_WIN32)
    if (S_ISLNK(info.st_mode)) return reject(&receipt);
#endif
    if (!S_ISREG(info.st_mode) ||
        !(file = fopen(path, "rb")) || fseek(file, 0L, SEEK_END) != 0 ||
        (file_size = ftell(file)) <= 0 ||
        (unsigned long)file_size > THERON_V1_CONSUMER_TRACE_MAX_BYTES ||
        fseek(file, 0L, SEEK_SET) != 0 || !m12_file_md5_hex(path, md5)) {
        if (file) fclose(file);
        receipt.status = THERON_V1_MEDNAFEN_MAIN_RAM_CONSUMER_TRACE_UNAVAILABLE;
        *out = receipt;
        return 1;
    }

    while (read_line(file, line, sizeof(line))) {
        int consumed = 0;
        unsigned logical_address, physical_address, value, reader_pc;
        unsigned reader_physical_pc;
        unsigned sequence;

        if (first_line) {
            first_line = 0;
            if (strcmp(line, "source=mednafen-pce-instrumented-main-ram-consumer")) {
                fclose(file);
                return reject(&receipt);
            }
            receipt.source_header_verified = 1;
            continue;
        }
        if (sscanf(line,
                   "main_ram_consumer_read sequence=%u logical_address=%x "
                   "physical_address=%x value=%x reader_pc=%x "
                   "reader_physical_pc=%x%n",
                   &sequence, &logical_address, &physical_address, &value,
                   &reader_pc, &reader_physical_pc, &consumed) != 6 ||
            line[consumed] != '\0' || sequence != receipt.read_count ||
            logical_address > 0xffffu || physical_address < 0x1f0000u ||
            physical_address >= 0x1f8000u || value > 0xffu ||
            reader_pc > 0xffffu || reader_physical_pc < 0x1f0000u ||
            reader_physical_pc >= 0x1f8000u) {
            fclose(file);
            return reject(&receipt);
        }
        if (!receipt.read_count) {
            receipt.first_logical_address = logical_address;
            receipt.first_physical_address = physical_address;
            receipt.first_reader_pc = reader_pc;
            receipt.first_reader_physical_pc = reader_physical_pc;
        }
        receipt.last_logical_address = logical_address;
        receipt.last_physical_address = physical_address;
        receipt.last_reader_pc = reader_pc;
        receipt.last_reader_physical_pc = reader_physical_pc;
        receipt.read_count++;
    }
    fclose(file);

    if (first_line || !receipt.source_header_verified || !receipt.read_count ||
        !m12_file_md5_hex(path, md5))
        return reject(&receipt);

    receipt.status = THERON_V1_MEDNAFEN_MAIN_RAM_CONSUMER_TRACE_READY;
    receipt.source_trace_md5_verified = 1;
    receipt.bank_coordinates_verified = 1;
    receipt.target_2600_bytes_present = 0;
    receipt.semantic_publication_allowed = 0;
    snprintf(receipt.source_trace_path, sizeof(receipt.source_trace_path), "%s", path);
    snprintf(receipt.source_trace_md5, sizeof(receipt.source_trace_md5), "%s", md5);
    *out = receipt;
    return 1;
}

int theron_v1_mednafen_main_ram_consumer_trace_verify_code_window(
    const char *path,
    uint16_t start_pc,
    const uint8_t *expected_bytes,
    size_t expected_count)
{
    Theron_V1MednafenMainRamConsumerTraceReceipt receipt;
    FILE *file;
    char line[512];
    uint8_t *seen;
    size_t seen_count = 0u;

    if (!path || !path[0] || !expected_bytes || expected_count == 0u ||
        expected_count > 0x10000u ||
        (size_t)start_pc + expected_count > 0x10000u ||
        !theron_v1_mednafen_main_ram_consumer_trace_parse_file(path,
                                                                &receipt) ||
        receipt.status != THERON_V1_MEDNAFEN_MAIN_RAM_CONSUMER_TRACE_READY) {
        return 0;
    }
    seen = (uint8_t *)calloc(expected_count, sizeof(*seen));
    if (!seen || !(file = fopen(path, "rb"))) {
        free(seen);
        return 0;
    }
    while (fgets(line, sizeof(line), file)) {
        unsigned logical_address, physical_address, value, reader_pc;
        unsigned reader_physical_pc, sequence;
        int consumed = 0;
        size_t offset;

        if (strncmp(line, "main_ram_consumer_read ", 23) != 0 ||
            sscanf(line,
                   "main_ram_consumer_read sequence=%u logical_address=%x "
                   "physical_address=%x value=%x reader_pc=%x "
                   "reader_physical_pc=%x%n",
                   &sequence, &logical_address, &physical_address, &value,
                   &reader_pc, &reader_physical_pc, &consumed) != 6) {
            continue;
        }
        (void)sequence;
        (void)physical_address;
        if (line[consumed] != '\n' && line[consumed] != '\r' &&
            line[consumed] != '\0') continue;
        if (logical_address != reader_pc ||
            physical_address != reader_physical_pc ||
            reader_pc < start_pc ||
            reader_pc >= (unsigned)start_pc + expected_count ||
            value > 0xffu) continue;
        offset = (size_t)(reader_pc - start_pc);
        if ((uint8_t)value != expected_bytes[offset]) {
            fclose(file);
            free(seen);
            return 0;
        }
        if (!seen[offset]) {
            seen[offset] = 1u;
            ++seen_count;
        }
    }
    fclose(file);
    free(seen);
    return seen_count == expected_count;
}
