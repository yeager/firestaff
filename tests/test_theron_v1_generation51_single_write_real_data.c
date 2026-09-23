#include "theron_v1_raw_loader_trace.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned char *read_file(const char *path, size_t *size)
{
    FILE *file = path ? fopen(path, "rb") : NULL;
    long length;
    unsigned char *bytes;
    *size = 0u;
    if (!file || fseek(file, 0, SEEK_END) != 0 ||
        (length = ftell(file)) <= 0 || fseek(file, 0, SEEK_SET) != 0) {
        if (file) fclose(file);
        return NULL;
    }
    bytes = (unsigned char *)malloc((size_t)length + 1u);
    if (!bytes || fread(bytes, 1u, (size_t)length, file) != (size_t)length) {
        free(bytes); fclose(file); return NULL;
    }
    fclose(file);
    bytes[length] = 0;
    *size = (size_t)length;
    return bytes;
}

int main(void)
{
    const char *paths[6] = {
        getenv("FIRESTAFF_THERON_US_TRACK02_BIN"),
        getenv("FIRESTAFF_THERON_GENERATION51_VDC_TRACE"),
        getenv("FIRESTAFF_THERON_GENERATION51_VDC_STATE"),
        getenv("FIRESTAFF_THERON_GENERATION51_VRAM"),
        getenv("FIRESTAFF_THERON_GENERATION51_VCE"),
        getenv("FIRESTAFF_THERON_GENERATION51_SAT")
    };
    unsigned char *data[6] = {0};
    size_t sizes[6] = {0}, i;
    Theron_V1RawLoaderTraceGameGeneration49GraphicsReceipt graphics;
    Theron_V1RawLoaderTraceGameGeneration51FrameReceipt receipt;

    for (i = 0u; i < 6u; ++i) if (!paths[i]) {
        puts("SKIP: authentic generation-51 single-write inputs are not configured");
        return 77;
    }
    for (i = 0u; i < 6u; ++i) if (!(data[i] = read_file(paths[i], &sizes[i]))) {
        fputs("FAIL: could not read generation-51 inputs\n", stderr);
        while (i) free(data[--i]);
        return 1;
    }
    memset(&graphics, 0, sizeof(graphics));
    graphics.valid = 1;
    graphics.variant = THERON_TRACK02_VARIANT_US_BIN;
    memcpy(graphics.track02_md5, THERON_TRACK02_MD5_US_BIN, 33u);
    graphics.media_bytes_verified = 1;
    graphics.single_writes_verified = 1;
    if (!theron_v1_raw_loader_trace_bind_game_generation51_frame(
            &graphics, (const char *)data[1], (const char *)data[2],
            data[3], sizes[3], data[4], sizes[4], data[5], sizes[5],
            data[0], sizes[0], THERON_TRACK02_MD5_US_BIN, &receipt) ||
        !receipt.valid || receipt.generation_rows != 54842u ||
        receipt.boundary_sequence != 94434u ||
        receipt.vram_checksum != 0xde27fc7eu ||
        receipt.vce_checksum != 0x88629e93u ||
        receipt.sat_checksum != 0x4d7705c5u ||
        receipt.l466b_writer_rows != 2176u ||
        receipt.l4943_writer_rows != 15u ||
        receipt.l50f1_writer_rows != 2064u ||
        receipt.l5111_writer_rows != 2048u ||
        !receipt.atomic_snapshot_verified ||
        !receipt.stable_loop_boundary_verified || receipt.screen_semantics_proven) {
        fputs("FAIL: generation-51 single-write frame rejected\n", stderr);
        for (i = 0u; i < 6u; ++i) free(data[i]);
        return 1;
    }
    data[4][0] ^= 1u;
    if (theron_v1_raw_loader_trace_bind_game_generation51_frame(
            &graphics, (const char *)data[1], (const char *)data[2],
            data[3], sizes[3], data[4], sizes[4], data[5], sizes[5],
            data[0], sizes[0], THERON_TRACK02_MD5_US_BIN, &receipt)) {
        fputs("FAIL: corrupted generation-51 palette accepted\n", stderr);
        for (i = 0u; i < 6u; ++i) free(data[i]);
        return 1;
    }
    for (i = 0u; i < 6u; ++i) free(data[i]);
    puts("PASS: authentic generation-51 frame uses the single-write boundary");
    return 0;
}
