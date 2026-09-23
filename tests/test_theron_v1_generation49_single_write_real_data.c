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
        free(bytes);
        fclose(file);
        return NULL;
    }
    fclose(file);
    bytes[length] = 0;
    *size = (size_t)length;
    return bytes;
}

int main(void)
{
    const char *media_path = getenv("FIRESTAFF_THERON_US_TRACK02_BIN");
    const char *cd_path = getenv("FIRESTAFF_THERON_GENERATION49_CD_TRACE");
    const char *vdc_path = getenv("FIRESTAFF_THERON_GENERATION49_VDC_TRACE");
    unsigned char *media, *cd, *vdc;
    size_t media_size, cd_size, vdc_size;
    Theron_V1RawLoaderTraceGameE009VdcPresentationReceipt prerequisite;
    Theron_V1RawLoaderTraceGameGeneration49GraphicsReceipt receipt;

    if (!media_path || !cd_path || !vdc_path) {
        puts("SKIP: authentic generation-49 single-write inputs are not configured");
        return 77;
    }
    media = read_file(media_path, &media_size);
    cd = read_file(cd_path, &cd_size);
    vdc = read_file(vdc_path, &vdc_size);
    if (!media || !cd || !vdc || !cd_size || !vdc_size) {
        free(media); free(cd); free(vdc);
        fputs("FAIL: could not read generation-49 inputs\n", stderr);
        return 1;
    }
    memset(&prerequisite, 0, sizeof(prerequisite));
    prerequisite.valid = 1;
    prerequisite.variant = THERON_TRACK02_VARIANT_US_BIN;
    memcpy(prerequisite.track02_md5, THERON_TRACK02_MD5_US_BIN, 33u);
    prerequisite.vdc_replay_verified = 1;
    prerequisite.stage2_l466b_verified = 1;
    if (!theron_v1_raw_loader_trace_bind_game_generation49_graphics(
            &prerequisite, (const char *)cd, (const char *)vdc,
            media, media_size, THERON_TRACK02_MD5_US_BIN, &receipt) ||
        !receipt.valid || receipt.scsi_generation != 49u ||
        receipt.scsi_lba != 4622u || receipt.scsi_sector_count != 12u ||
        receipt.first_raw_track02_record != 0x64du ||
        receipt.payload_bytes != 24576u ||
        receipt.payload_checksum != 0x01551f76u ||
        receipt.vdc_rows != 24580u || receipt.vdc_payload_rows != 24576u ||
        receipt.first_vram_word != 0x1000u ||
        receipt.last_vram_word != 0x6fffu ||
        receipt.repeated_writes_verified || !receipt.single_writes_verified ||
        !receipt.media_bytes_verified || receipt.graphics_semantics_proven) {
        free(media); free(cd); free(vdc);
        fputs("FAIL: generation-49 single-write transport rejected\n", stderr);
        return 1;
    }
    media[(size_t)0x64du * 2352u + 16u] ^= 1u;
    if (theron_v1_raw_loader_trace_bind_game_generation49_graphics(
            &prerequisite, (const char *)cd, (const char *)vdc,
            media, media_size, THERON_TRACK02_MD5_US_BIN, &receipt)) {
        free(media); free(cd); free(vdc);
        fputs("FAIL: corrupted generation-49 media accepted\n", stderr);
        return 1;
    }
    free(media); free(cd); free(vdc);
    puts("PASS: authentic generation-49 transport uses one hardware write per byte");
    return 0;
}
