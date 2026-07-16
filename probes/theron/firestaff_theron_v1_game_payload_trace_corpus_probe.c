#include "asset_status_m12.h"
#include "theron_v1_raw_loader_trace.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

static uint8_t *read_file(const char *path, size_t *out_size)
{
    FILE *file;
    long size;
    uint8_t *bytes;

    if (out_size) *out_size = 0u;
    if (!path || !out_size || !(file = fopen(path, "rb"))) return NULL;
    if (fseek(file, 0L, SEEK_END) != 0 || (size = ftell(file)) <= 0 ||
        fseek(file, 0L, SEEK_SET) != 0 ||
        !(bytes = (uint8_t *)malloc((size_t)size))) {
        fclose(file);
        return NULL;
    }
    if (fread(bytes, 1u, (size_t)size, file) != (size_t)size) {
        fclose(file);
        free(bytes);
        return NULL;
    }
    fclose(file);
    *out_size = (size_t)size;
    return bytes;
}

int main(void)
{
    const char *raw_path = getenv("FIRESTAFF_THERON_TRACK02_US_BIN");
    const char *trace_path =
        getenv("FIRESTAFF_THERON_GAME_PAYLOAD_TRACE");
    struct stat raw_stat;
    struct stat trace_stat;
    char md5[33];
    uint8_t *raw;
    size_t raw_size;
    Theron_V1RawLoaderTraceGamePayloadReceipt receipt;

    if (!raw_path || !trace_path) {
        printf("status=skip reason=staged_us_track02_and_game_payload_trace_required\n");
        return 0;
    }
    if (stat(raw_path, &raw_stat) != 0 || raw_stat.st_size <= 0 ||
        stat(trace_path, &trace_stat) != 0 || trace_stat.st_size <= 0) {
        printf("status=blocked reason=staged_consumer_corpus_missing\n");
        return 1;
    }
    if (!m12_file_md5_hex(raw_path, md5) ||
        strcmp(md5, THERON_TRACK02_MD5_US_BIN) != 0 ||
        (size_t)raw_stat.st_size % THERON_TRACK02_RAW_SECTOR_BYTES != 0u) {
        printf("status=blocked reason=us_raw_track02_unverified\n");
        return 1;
    }
    raw = read_file(raw_path, &raw_size);
    if (!raw) {
        printf("status=blocked reason=us_raw_track02_read_failed\n");
        return 1;
    }
    if (!theron_v1_raw_loader_trace_import_game_owned_fifo_payload_file(
            trace_path, raw, raw_size, md5, &receipt) ||
        !receipt.valid || receipt.variant != THERON_TRACK02_VARIANT_US_BIN ||
        strcmp(receipt.track02_md5, md5) != 0 ||
        !receipt.cdb_read6_verified || !receipt.fifo_to_game_ram_verified ||
        !receipt.game_ram_consumer_verified ||
        receipt.payload_semantics_proven ||
        receipt.raw_track02_record >=
            raw_size / THERON_TRACK02_RAW_SECTOR_BYTES ||
        receipt.source_offset >= THERON_TRACK02_RAW_SECTOR_BYTES ||
        raw[(size_t)receipt.raw_track02_record *
                THERON_TRACK02_RAW_SECTOR_BYTES +
            receipt.source_offset] != receipt.source_byte) {
        free(raw);
        printf("status=blocked reason=game_payload_trace_unproven\n");
        return 1;
    }
    raw[(size_t)receipt.raw_track02_record *
            THERON_TRACK02_RAW_SECTOR_BYTES +
        receipt.source_offset] ^= 0x01u;
    if (theron_v1_raw_loader_trace_import_game_owned_fifo_payload_file(
            trace_path, raw, raw_size, md5, &receipt)) {
        free(raw);
        printf("status=blocked reason=consumer_trace_accepted_mutated_media\n");
        return 1;
    }
    free(raw);
    printf("status=ready trace=validated_game_owned_fifo_payload_consumer semantics=opaque\n");
    return 0;
}
