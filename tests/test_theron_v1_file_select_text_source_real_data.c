#include "theron_v1_raw_loader_trace.h"

#include <stdio.h>
#include <stdlib.h>

static unsigned char *read_file(const char *path, size_t *out_size)
{
    FILE *file;
    long length;
    unsigned char *bytes;

    *out_size = 0u;
    file = path ? fopen(path, "rb") : NULL;
    if (!file || fseek(file, 0, SEEK_END) != 0 ||
        (length = ftell(file)) <= 0 || fseek(file, 0, SEEK_SET) != 0) {
        if (file) fclose(file);
        return NULL;
    }
    bytes = (unsigned char *)malloc((size_t)length);
    if (!bytes || fread(bytes, 1u, (size_t)length, file) != (size_t)length) {
        free(bytes);
        fclose(file);
        return NULL;
    }
    fclose(file);
    *out_size = (size_t)length;
    return bytes;
}

int main(void)
{
    const char *path = getenv("FIRESTAFF_THERON_US_TRACK02_BIN");
    unsigned char *track02;
    size_t track02_size;
    Theron_V1RawLoaderTraceFileSelectTextSourceReceipt receipt;

    if (!path || !path[0]) {
        puts("SKIP: FIRESTAFF_THERON_US_TRACK02_BIN is not configured");
        return 77;
    }
    track02 = read_file(path, &track02_size);
    if (!track02) {
        fputs("FAIL: could not read authentic US Track 02\n", stderr);
        return 1;
    }
    if (!theron_v1_raw_loader_trace_bind_file_select_text_source(
            track02, track02_size, THERON_TRACK02_MD5_US_BIN, &receipt) ||
        !receipt.valid || receipt.variant != THERON_TRACK02_VARIANT_US_BIN ||
        receipt.occurrence_count != 3u ||
        receipt.raw_track02_record[0] != 0x4eau ||
        receipt.raw_track02_record[1] != 0x4ecu ||
        receipt.raw_track02_record[2] != 0x4eeu ||
        receipt.raw_sector_offset[0] != 0x1aeu ||
        receipt.raw_sector_offset[1] != 0x0fau ||
        receipt.raw_sector_offset[2] != 0x0fau ||
        receipt.play_prompt_checksum != 0xef1550adu ||
        receipt.load_prompt_checksum != 0xaa654403u ||
        !receipt.mode1_coordinates_verified || !receipt.source_text_verified ||
        receipt.screen_consumer_proven) {
        free(track02);
        fputs("FAIL: authentic file-select text source receipt rejected\n", stderr);
        return 1;
    }
    track02[2959246u] ^= 1u;
    if (theron_v1_raw_loader_trace_bind_file_select_text_source(
            track02, track02_size, THERON_TRACK02_MD5_US_BIN, &receipt)) {
        free(track02);
        fputs("FAIL: corrupted authentic prompt copy was accepted\n", stderr);
        return 1;
    }
    track02[2959246u] ^= 1u;
    if (theron_v1_raw_loader_trace_bind_file_select_text_source(
            track02, track02_size, "00000000000000000000000000000000",
            &receipt)) {
        free(track02);
        fputs("FAIL: wrong Track 02 identity was accepted\n", stderr);
        return 1;
    }
    free(track02);
    puts("PASS: authentic Theron file-select text source is media-bound");
    return 0;
}
