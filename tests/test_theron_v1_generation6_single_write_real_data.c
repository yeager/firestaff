#include "theron_v1_raw_loader_trace.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    bytes = (unsigned char *)malloc((size_t)length + 1u);
    if (!bytes || fread(bytes, 1u, (size_t)length, file) != (size_t)length) {
        free(bytes); fclose(file); return NULL;
    }
    fclose(file);
    bytes[length] = 0;
    *out_size = (size_t)length;
    return bytes;
}

int main(void)
{
    const char *media_path = getenv("FIRESTAFF_THERON_US_TRACK02_BIN");
    const char *cd_path = getenv("FIRESTAFF_THERON_GENERATION6_CD_TRACE");
    const char *vdc_path = getenv("FIRESTAFF_THERON_GENERATION6_VDC_TRACE");
    const char *vram_path = getenv("FIRESTAFF_THERON_GENERATION6_VRAM");
    unsigned char *media, *cd, *vdc, *vram;
    size_t media_size, cd_size, vdc_size, vram_size;
    Theron_V1RawLoaderTraceGameE009NextParametersReceipt parameters;
    Theron_V1RawLoaderTraceGameE009VdcReceipt receipt;

    if (!media_path || !cd_path || !vdc_path || !vram_path) {
        puts("SKIP: authentic generation-6 single-write inputs are not configured");
        return 77;
    }
    media = read_file(media_path, &media_size);
    cd = read_file(cd_path, &cd_size);
    vdc = read_file(vdc_path, &vdc_size);
    vram = read_file(vram_path, &vram_size);
    if (!media || !cd || !vdc || !vram || !cd_size || !vdc_size ||
        vram_size != 65536u) {
        free(media); free(cd); free(vdc); free(vram);
        fputs("FAIL: could not read generation-6 inputs\n", stderr);
        return 1;
    }
    memset(&parameters, 0, sizeof(parameters));
    parameters.valid = 1;
    parameters.variant = THERON_TRACK02_VARIANT_US_BIN;
    memcpy(parameters.track02_md5, THERON_TRACK02_MD5_US_BIN, 33u);
    memcpy(parameters.next_parameters,
        "\x00\x20\x00\x10\x00\x06\xf8\xfe", 8u);
    parameters.consumer_output_writes_verified = 1;
    parameters.tii_verified = 1;
    parameters.next_parameters_verified = 1;
    if (!theron_v1_raw_loader_trace_bind_game_e009_vdc_payload(
            &parameters, (const char *)cd, (const char *)vdc,
            media, media_size, vram, vram_size,
            THERON_TRACK02_MD5_US_BIN, &receipt) || !receipt.valid ||
        receipt.vdc_payload_writes != 8192u ||
        receipt.first_vram_word != 0x1000u ||
        receipt.last_vram_word != 0x1fffu ||
        receipt.vram_word_count != 4096u ||
        receipt.vram_snapshot_checksum != 0x4859675du ||
        receipt.full_vram_snapshot_checksum != 0xedfc7797u ||
        receipt.repeated_word_writes_verified ||
        !receipt.single_word_writes_verified ||
        !receipt.mode1_payload_verified || receipt.payload_semantics_proven) {
        free(media); free(cd); free(vdc); free(vram);
        fputs("FAIL: generation-6 single-write receipt rejected\n", stderr);
        return 1;
    }
    vram[0x2000u] ^= 1u;
    if (theron_v1_raw_loader_trace_bind_game_e009_vdc_payload(
            &parameters, (const char *)cd, (const char *)vdc,
            media, media_size, vram, vram_size,
            THERON_TRACK02_MD5_US_BIN, &receipt)) {
        free(media); free(cd); free(vdc); free(vram);
        fputs("FAIL: corrupted generation-6 VRAM accepted\n", stderr);
        return 1;
    }
    vram[0x2000u] ^= 1u;
    media[0x7d9u * 2352u + 16u] ^= 1u;
    if (theron_v1_raw_loader_trace_bind_game_e009_vdc_payload(
            &parameters, (const char *)cd, (const char *)vdc,
            media, media_size, vram, vram_size,
            THERON_TRACK02_MD5_US_BIN, &receipt)) {
        free(media); free(cd); free(vdc); free(vram);
        fputs("FAIL: corrupted generation-6 media accepted\n", stderr);
        return 1;
    }
    free(media); free(cd); free(vdc); free(vram);
    puts("PASS: authentic generation-6 VDC transport uses one hardware write");
    return 0;
}
