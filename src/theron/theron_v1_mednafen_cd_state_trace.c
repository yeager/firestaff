#include "theron_v1_mednafen_cd_state_trace.h"

#include "asset_status_m12.h"

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#if defined(_WIN32)
#include <io.h>
#define THERON_V1_CD_TRACE_FILENO _fileno
#define THERON_V1_CD_TRACE_LSTAT(path, info) stat((path), (info))
#else
#include <unistd.h>
#define THERON_V1_CD_TRACE_FILENO fileno
#define THERON_V1_CD_TRACE_LSTAT lstat
#endif

#define THERON_V1_MEDNAFEN_CD_TRACE_MAX_BYTES (4u * 1024u * 1024u)
#define THERON_V1_RAW_MODE1_2352_BYTES 2352u

typedef struct {
    uint32_t generation;
    uint32_t start_lba;
    uint32_t sector_count;
    uint32_t next_sector_index;
    int active;
} Theron_V1PendingCdRead;

typedef struct {
    uint32_t start_lba;
    uint32_t sector_count;
} Theron_V1CdRange;

static int source_lba_in_range(const Theron_V1CdRange *ranges,
                               size_t range_count,
                               uint32_t source_lba) {
    size_t i;
    for (i = 0; i < range_count; ++i) {
        if (source_lba >= ranges[i].start_lba &&
            source_lba - ranges[i].start_lba < ranges[i].sector_count)
            return 1;
    }
    return 0;
}

static int trace_line(FILE *file, char *line, size_t capacity) {
    size_t length;
    if (!fgets(line, capacity, file)) return 0;
    length = strlen(line);
    while (length && (line[length - 1] == '\n' || line[length - 1] == '\r'))
        line[--length] = '\0';
    return length > 0;
}

static int known_source_marker(const char *line) {
    static const char *const markers[] = {
        "source=mednafen-pce-instrumented-cd-state",
        "source=mednafen-pce-instrumented-cd",
        "source=mednafen-pce-instrumented-cd-register",
        "source=mednafen-pce-instrumented-cd-transfer",
        "source=mednafen-pce-instrumented-scsi-read",
    };
    size_t i;
    for (i = 0; i < sizeof(markers) / sizeof(markers[0]); ++i)
        if (!strcmp(line, markers[i])) return 1;
    return 0;
}

static int known_observation_row(const char *line, const char *prefix) {
    return strncmp(line, prefix, strlen(prefix)) == 0;
}

static int reject(Theron_V1MednafenCdStateTraceReceipt *receipt) {
    receipt->status = THERON_V1_MEDNAFEN_CD_STATE_TRACE_REJECTED;
    receipt->semantic_publication_allowed = 0;
    return 1;
}

int theron_v1_mednafen_cd_state_trace_parse_file(
    const char *path,
    Theron_V1MednafenCdStateTraceReceipt *out)
{
    Theron_V1MednafenCdStateTraceReceipt receipt = {0};
    Theron_V1PendingCdRead pending = {0};
    Theron_V1CdRange ranges[64] = {{0}};
    size_t range_count = 0;
    struct stat info;
    FILE *file;
    char line[512];
    char md5[33];
    long file_size;
    int first_line = 1;

    if (!out) return 0;
    *out = receipt;
    if (!path || !path[0] || THERON_V1_CD_TRACE_LSTAT(path, &info) != 0) {
        receipt.status = THERON_V1_MEDNAFEN_CD_STATE_TRACE_UNAVAILABLE;
        *out = receipt;
        return 1;
    }
#if !defined(_WIN32)
    if (S_ISLNK(info.st_mode)) return reject(&receipt);
#endif
    if (!S_ISREG(info.st_mode)) {
        receipt.status = THERON_V1_MEDNAFEN_CD_STATE_TRACE_UNAVAILABLE;
        *out = receipt;
        return 1;
    }
    file = fopen(path, "rb");
    if (!file || fseek(file, 0L, SEEK_END) != 0 ||
        (file_size = ftell(file)) <= 0 ||
        (unsigned long)file_size > THERON_V1_MEDNAFEN_CD_TRACE_MAX_BYTES ||
        fseek(file, 0L, SEEK_SET) != 0 || !m12_file_md5_hex(path, md5)) {
        if (file) fclose(file);
        receipt.status = THERON_V1_MEDNAFEN_CD_STATE_TRACE_UNAVAILABLE;
        *out = receipt;
        return 1;
    }

    while (trace_line(file, line, sizeof(line))) {
        int consumed = 0;
        uint32_t generation, opcode, start_lba, sector_count;
        uint32_t lba, bytes, sector_fnv1a, span_offset, span_bytes, span_fnv1a;
        uint32_t binding_generation, binding_start_lba, binding_sector_count;
        uint32_t binding_lba, sector_index;
        uint32_t origin_lba, origin_offset, origin_reader_pc;
        uint32_t origin_reader_physical_pc, origin_writer_pc;
        uint32_t origin_writer_physical_pc, origin_destination;
        uint32_t origin_physical, origin_read_value, origin_stored_value;
        unsigned long long origin_fifo_sequence;
        char cdb[32];

        if (first_line) {
            first_line = 0;
            if (strcmp(line, "source=mednafen-pce-instrumented-cd-state")) {
                fclose(file);
                return reject(&receipt);
            }
            receipt.source_header_verified = 1;
            receipt.source_marker_rows++;
            continue;
        }
        if (known_source_marker(line)) {
            receipt.source_marker_rows++;
            continue;
        }
        if (known_observation_row(line, "pce_cd_irq ")) {
            receipt.cd_irq_count++;
            continue;
        }
        if (known_observation_row(line, "pce_cd_register_read ")) {
            receipt.register_read_count++;
            continue;
        }
        if (known_observation_row(line, "pce_cd_register_write ")) {
            receipt.register_write_count++;
            continue;
        }
        if (known_observation_row(line, "pce_cd_data_destination_candidate ")) {
            receipt.destination_candidate_count++;
            continue;
        }
        if (sscanf(line,
                   "pce_cd_origin_ram_receipt source_lba=%u source_offset=%u "
                   "fifo_sequence=%llu reader_pc=%x reader_physical_pc=%x "
                   "writer_pc=%x writer_physical_pc=%x logical_destination=%x "
                   "physical=%x read_value=%x stored_value=%x%n",
                   &origin_lba, &origin_offset, &origin_fifo_sequence,
                   &origin_reader_pc, &origin_reader_physical_pc,
                   &origin_writer_pc, &origin_writer_physical_pc,
                   &origin_destination, &origin_physical, &origin_read_value,
                   &origin_stored_value,
                   &consumed) == 11 && line[consumed] == '\0') {
            if (origin_offset >= THERON_V1_RAW_MODE1_2352_BYTES ||
                !source_lba_in_range(ranges, range_count, origin_lba) ||
                origin_reader_pc > 0xffffu ||
                origin_writer_pc > 0xffffu ||
                origin_reader_physical_pc > 0xffffffu ||
                origin_writer_physical_pc > 0xffffffu ||
                origin_destination > 0xffffu ||
                origin_physical < 0x1f0000u ||
                origin_physical >= 0x1f8000u ||
                origin_read_value > 0xffu || origin_stored_value > 0xffu ||
                origin_read_value != origin_stored_value) {
                fclose(file);
                return reject(&receipt);
            }
            receipt.origin_ram_receipt_count++;
            receipt.origin_ram_source_verified = 1;
            continue;
        }
        if (known_observation_row(line, "main_ram_e009_enter ") ||
            known_observation_row(line, "main_ram_e009_register_write ") ||
            known_observation_row(line, "main_ram_e009_return ")) {
            continue;
        }
        if (sscanf(line,
                   "scsi_read_command generation=%u opcode=%x cdb=%31s "
                   "start_lba=%u sector_count=%u%n",
                   &generation, &opcode, cdb, &start_lba, &sector_count,
                   &consumed) == 5 && line[consumed] == '\0') {
            if (pending.active || opcode != 0x08u || !sector_count ||
                generation != receipt.scsi_command_count + 1u ||
                strlen(cdb) != 12u) {
                fclose(file);
                return reject(&receipt);
            }
            pending.generation = generation;
            pending.start_lba = start_lba;
            pending.sector_count = sector_count;
            pending.next_sector_index = 0;
            pending.active = 1;
            receipt.scsi_command_count++;
            receipt.requested_sector_count += sector_count;
            if (range_count >= sizeof(ranges) / sizeof(ranges[0])) {
                fclose(file);
                return reject(&receipt);
            }
            ranges[range_count].start_lba = start_lba;
            ranges[range_count].sector_count = sector_count;
            range_count++;
            continue;
        }
        if (sscanf(line,
                   "cd_interface_raw_sector_read lba=%u bytes=%u "
                   "sector_fnv1a=%x span_offset=%u span_bytes=%u "
                   "span_fnv1a=%x%n",
                   &lba, &bytes, &sector_fnv1a, &span_offset, &span_bytes,
                   &span_fnv1a, &consumed) == 6 && line[consumed] == '\0') {
            if (!pending.active || bytes != THERON_V1_RAW_MODE1_2352_BYTES ||
                lba != pending.start_lba + pending.next_sector_index ||
                span_offset > bytes || span_bytes > bytes - span_offset) {
                fclose(file);
                return reject(&receipt);
            }
            if (!receipt.raw_sector_count) {
                receipt.first_lba = lba;
                receipt.first_sector_fnv1a = sector_fnv1a;
            }
            receipt.last_lba = lba;
            receipt.last_sector_fnv1a = sector_fnv1a;
            receipt.raw_sector_count++;
            receipt.raw_mode1_2352_verified = 1;
            continue;
        }
        if (sscanf(line,
                   "scsi_read_sector_binding generation=%u start_lba=%u "
                   "sector_count=%u lba=%u sector_index=%u%n",
                   &binding_generation, &binding_start_lba,
                   &binding_sector_count, &binding_lba, &sector_index,
                   &consumed) == 5 && line[consumed] == '\0') {
            if (!pending.active || binding_generation != pending.generation ||
                binding_start_lba != pending.start_lba ||
                binding_sector_count != pending.sector_count ||
                binding_lba != pending.start_lba + pending.next_sector_index ||
                sector_index != pending.next_sector_index) {
                fclose(file);
                return reject(&receipt);
            }
            pending.next_sector_index++;
            receipt.sector_binding_count++;
            if (pending.next_sector_index == pending.sector_count)
                pending.active = 0;
            continue;
        }
        fclose(file);
        return reject(&receipt);
    }
    fclose(file);

    if (first_line || pending.active || !receipt.source_header_verified ||
        receipt.source_marker_rows != 5u ||
        !receipt.scsi_command_count ||
        receipt.requested_sector_count != receipt.raw_sector_count ||
        receipt.raw_sector_count != receipt.sector_binding_count ||
        receipt.raw_sector_count == 0u || !m12_file_md5_hex(path, md5))
        return reject(&receipt);

    receipt.status = THERON_V1_MEDNAFEN_CD_STATE_TRACE_READY;
    receipt.source_trace_md5_verified = 1;
    receipt.command_sector_binding_verified = 1;
    receipt.semantic_publication_allowed = 0;
    snprintf(receipt.source_trace_path, sizeof(receipt.source_trace_path), "%s", path);
    snprintf(receipt.source_trace_md5, sizeof(receipt.source_trace_md5), "%s", md5);
    *out = receipt;
    return 1;
}
