#include "theron_v1_mednafen_main_ram_trace.h"

#include "asset_status_m12.h"

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#if defined(_WIN32)
#define THERON_V1_MAIN_RAM_LSTAT(path, info) stat((path), (info))
#else
#include <unistd.h>
#define THERON_V1_MAIN_RAM_LSTAT lstat
#endif

#define THERON_V1_MAIN_RAM_TRACE_MAX_BYTES (256u * 1024u)

static int read_line(FILE *file, char *line, size_t capacity) {
    size_t length;
    if (!fgets(line, capacity, file)) return 0;
    length = strlen(line);
    while (length && (line[length - 1] == '\n' || line[length - 1] == '\r'))
        line[--length] = '\0';
    return length > 0;
}

static int reject(Theron_V1MednafenMainRamTraceReceipt *receipt) {
    receipt->status = THERON_V1_MEDNAFEN_MAIN_RAM_TRACE_REJECTED;
    receipt->semantic_publication_allowed = 0;
    return 1;
}

int theron_v1_mednafen_main_ram_trace_parse_file(
    const char *path,
    Theron_V1MednafenMainRamTraceReceipt *out)
{
    Theron_V1MednafenMainRamTraceReceipt receipt = {0};
    struct stat info;
    FILE *file = NULL;
    char line[512];
    char md5[33];
    long file_size;
    int first_line = 1;
    int saw_post_rts = 0;

    if (!out) return 0;
    *out = receipt;
    if (!path || !path[0] || THERON_V1_MAIN_RAM_LSTAT(path, &info) != 0) {
        receipt.status = THERON_V1_MEDNAFEN_MAIN_RAM_TRACE_UNAVAILABLE;
        *out = receipt;
        return 1;
    }
#if !defined(_WIN32)
    if (S_ISLNK(info.st_mode)) return reject(&receipt);
#endif
    if (!S_ISREG(info.st_mode) ||
        !(file = fopen(path, "rb")) || fseek(file, 0L, SEEK_END) != 0 ||
        (file_size = ftell(file)) <= 0 ||
        (unsigned long)file_size > THERON_V1_MAIN_RAM_TRACE_MAX_BYTES ||
        fseek(file, 0L, SEEK_SET) != 0 || !m12_file_md5_hex(path, md5)) {
        if (file) fclose(file);
        receipt.status = THERON_V1_MEDNAFEN_MAIN_RAM_TRACE_UNAVAILABLE;
        *out = receipt;
        return 1;
    }

    while (read_line(file, line, sizeof(line))) {
        int consumed = 0;
        uint32_t logical_pc, physical_pc, source, destination, length;
        uint32_t rts_logical_pc, rts_physical_pc;
        uint32_t post_source_logical_pc, post_source_physical_pc;
        uint32_t post_logical_pc, post_physical_pc, opcode;
        char operation[16];

        if (first_line) {
            first_line = 0;
            if (strcmp(line, "source=mednafen-pce-instrumented-main-ram-loader")) {
                fclose(file);
                return reject(&receipt);
            }
            receipt.source_header_verified = 1;
            continue;
        }
        if (sscanf(line,
                   "main_ram_loader_block_transfer logical_pc=%x physical_pc=%x "
                   "operation=%15s source=%x destination=%x length=%x%n",
                   &logical_pc, &physical_pc, operation, &source, &destination,
                   &length, &consumed) == 6 && line[consumed] == '\0') {
            if (strcmp(operation, "tia") ||
                !receipt.block_transfer_count ||
                receipt.first_logical_pc != logical_pc ||
                receipt.first_physical_pc != physical_pc ||
                receipt.first_source != source ||
                receipt.first_destination != destination ||
                receipt.first_length != length || strcmp(operation, "tia")) {
                if (!receipt.block_transfer_count) {
                    receipt.first_logical_pc = logical_pc;
                    receipt.first_physical_pc = physical_pc;
                    receipt.first_source = source;
                    receipt.first_destination = destination;
                    receipt.first_length = length;
                } else {
                    fclose(file);
                    return reject(&receipt);
                }
            }
            receipt.block_transfer_count++;
            continue;
        }
        if (sscanf(line, "main_ram_loader_rts logical_pc=%x physical_pc=%x%n",
                   &rts_logical_pc, &rts_physical_pc, &consumed) == 2 &&
            line[consumed] == '\0') {
            if (rts_logical_pc != 0x228du || rts_physical_pc != 0x1f028du) {
                fclose(file);
                return reject(&receipt);
            }
            receipt.rts_count++;
            continue;
        }
        if (sscanf(line,
                   "main_ram_loader_post_rts source_logical_pc=%x "
                   "source_physical_pc=%x logical_pc=%x physical_pc=%x opcode=%x%n",
                   &post_source_logical_pc, &post_source_physical_pc,
                   &post_logical_pc, &post_physical_pc, &opcode, &consumed) == 5 &&
            line[consumed] == '\0') {
            if (saw_post_rts || post_source_logical_pc != 0x228du ||
                post_source_physical_pc != 0x1f028du ||
                post_logical_pc != 0x2286u || post_physical_pc != 0x1f0286u ||
                opcode != 0xe3u) {
                fclose(file);
                return reject(&receipt);
            }
            saw_post_rts = 1;
            receipt.post_rts_count++;
            continue;
        }
        fclose(file);
        return reject(&receipt);
    }
    fclose(file);

    if (first_line || !receipt.source_header_verified ||
        receipt.block_transfer_count < 1u || receipt.rts_count < 1u ||
        receipt.post_rts_count != 1u || !saw_post_rts ||
        !m12_file_md5_hex(path, md5))
        return reject(&receipt);

    receipt.status = THERON_V1_MEDNAFEN_MAIN_RAM_TRACE_READY;
    receipt.source_trace_md5_verified = 1;
    receipt.transfer_coordinates_verified = 1;
    receipt.target_2600_bytes_present = 0;
    receipt.semantic_publication_allowed = 0;
    snprintf(receipt.source_trace_path, sizeof(receipt.source_trace_path), "%s", path);
    snprintf(receipt.source_trace_md5, sizeof(receipt.source_trace_md5), "%s", md5);
    *out = receipt;
    return 1;
}
