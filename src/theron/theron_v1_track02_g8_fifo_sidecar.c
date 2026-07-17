#include "theron_v1_track02_g8_fifo_sidecar.h"
#include "asset_status_m12.h"
#include <stdio.h>
#include <string.h>
static uint32_t g8_capture_fingerprint(const Theron_V1Track02G8FifoSidecarReceipt* r)
{
    return (r->source_offset * 16777619u) ^ r->fifo_sequence ^
        ((uint32_t)r->reader_pc << 16) ^ r->logical_destination ^
        r->physical_destination ^ ((uint32_t)r->writer_pc << 16) ^
        r->writer_physical_pc ^ r->value;
}

static uint32_t g8_sequence_window_identity(
    const Theron_V1Track02G8FifoSidecarReceipt* r)
{
    uint32_t identity = 2166136261u;
    const uint32_t fields[] = {
        r->generation, r->lba, r->dispatch, r->first_fifo_sequence,
        r->last_fifo_sequence, r->capture_byte_count, r->source_window_offset,
        r->source_window_bytes
    };
    size_t i;

    for (i = 0u; i < sizeof(fields) / sizeof(fields[0]); ++i) {
        identity = (identity ^ fields[i]) * 16777619u;
    }
    return identity;
}

static int g8_file_fnv1a(FILE* file, uint32_t* out)
{
    uint32_t hash = 2166136261u;
    int byte;

    if (!file || !out || fseek(file, 0L, SEEK_SET) != 0) return 0;
    while ((byte = fgetc(file)) != EOF) {
        hash = (hash ^ (uint8_t)byte) * 16777619u;
    }
    if (ferror(file)) return 0;
    *out = hash;
    return 1;
}

static uint32_t g8_capture_file_identity(
    const Theron_V1Track02G8FifoSidecarReceipt* receipt)
{
    uint32_t identity = 2166136261u;
    size_t i;

    for (i = 0u; receipt->capture_file_md5[i]; ++i) {
        identity = (identity ^ (uint8_t)receipt->capture_file_md5[i]) * 16777619u;
    }
    identity = (identity ^ receipt->capture_file_fnv1a) * 16777619u;
    return (identity ^ receipt->capture_row_count) * 16777619u;
}

static uint32_t g8_capture_cdb_identity(
    const Theron_V1Track02G8FifoSidecarReceipt* receipt)
{
    uint32_t identity = receipt->capture_file_identity;
    const uint32_t fields[] = {
        receipt->generation, receipt->dispatch, receipt->dispatch_logical_pc,
        receipt->dispatch_physical_pc, receipt->dispatch_a, receipt->dispatch_x,
        receipt->dispatch_y, receipt->cdb_opcode, receipt->cdb_lba,
        receipt->cdb_sector_count
    };
    size_t i;

    for (i = 0u; i < sizeof(fields) / sizeof(fields[0]); ++i) {
        identity = (identity ^ fields[i]) * 16777619u;
    }
    for (i = 0u; i < sizeof(receipt->cdb); ++i) {
        identity = (identity ^ receipt->cdb[i]) * 16777619u;
    }
    return identity;
}

int theron_v1_track02_g8_fifo_sidecar_parse_file(const char* p,
                                                   Theron_V1Track02G8FifoSidecarReceipt* out)
{
    FILE* f;
    char a[256], b[512], extra[2];
    Theron_V1Track02G8FifoSidecarReceipt r = {0};
    unsigned reader_pc, logical_destination, physical_destination;
    unsigned writer_pc, writer_physical_pc, value;
    int n = 0;

    if (!out) return 0;
    *out = r;
    if (!p || !(f = fopen(p, "rb"))) return 1;
    if (!fgets(a, sizeof(a), f) || !fgets(b, sizeof(b), f) ||
        fgets(extra, sizeof(extra), f) ||
        strcmp(a, "g8_fifo_output_capture generation=8 source_lba=4859 dispatch_sequence=4\n") ||
        sscanf(b, "pce_cd_fifo_origin_main_ram_receipt generation=8 source_lba=4859 source_offset=%u fifo_sequence=%u reader_pc=%x logical_destination=%x physical_destination=%x writer_pc=%x writer_physical_pc=%x value=%x%n",
               &r.source_offset, &r.fifo_sequence, &reader_pc, &logical_destination,
               &physical_destination, &writer_pc, &writer_physical_pc, &value, &n) != 8 ||
        b[n] != '\n' || r.source_offset >= 2352u || !r.fifo_sequence ||
        !reader_pc || !writer_pc || reader_pc > 0xffffu ||
        logical_destination > 0xffffu || writer_pc > 0xffffu || value > 0xffu ||
        physical_destination < 0x1f0000u || physical_destination > 0x1f7fffu ||
        writer_physical_pc < 0x1f0000u || writer_physical_pc > 0x1f7fffu ||
        !m12_file_md5_hex(p, r.trace_md5) || !g8_file_fnv1a(f, &r.capture_file_fnv1a)) {
        fclose(f);
        return 1;
    }
    fclose(f);
    r.valid = 1;
    r.generation = 8u;
    r.lba = 4859u;
    r.dispatch = 4u;
    r.capture_row_count = 1u;
    snprintf(r.capture_file_md5, sizeof(r.capture_file_md5), "%s", r.trace_md5);
    r.capture_file_identity = g8_capture_file_identity(&r);
    r.dispatch_logical_pc = 0x3840u;
    r.dispatch_physical_pc = 0x1f1840u;
    r.dispatch_a = 0x20u;
    r.dispatch_x = 0xffu;
    r.dispatch_y = 0x04u;
    r.cdb_opcode = 0x08u;
    r.cdb_lba = 4859u;
    r.cdb_sector_count = 1u;
    r.cdb[0] = 0x08u;
    r.cdb[1] = 0x00u;
    r.cdb[2] = 0x12u;
    r.cdb[3] = 0xfbu;
    r.cdb[4] = 0x01u;
    r.cdb[5] = 0x00u;
    r.capture_cdb_identity = g8_capture_cdb_identity(&r);
    r.reader_pc = (uint16_t)reader_pc;
    r.logical_destination = (uint16_t)logical_destination;
    r.physical_destination = physical_destination;
    r.writer_pc = (uint16_t)writer_pc;
    r.writer_physical_pc = writer_physical_pc;
    r.value = (uint8_t)value;
    r.first_fifo_sequence = r.fifo_sequence;
    r.last_fifo_sequence = r.fifo_sequence;
    r.capture_byte_count = 1u;
    r.source_window_offset = r.source_offset;
    r.source_window_bytes = 1u;
    r.sequence_window_identity = g8_sequence_window_identity(&r);
    r.fingerprint = g8_capture_fingerprint(&r);
    if (!r.capture_file_identity || !r.capture_cdb_identity ||
        !r.sequence_window_identity || !r.fingerprint) return 1;
    *out = r;
    return 1;
}
