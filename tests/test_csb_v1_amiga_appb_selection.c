/* Real-data regression for A31M APPB.FTL's source-owned language picker. */
#include "asset_find_by_hash.h"
#include "csb_v1_amiga_titl_dat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int load_file(const char *path, uint8_t **bytes, size_t *size)
{
    FILE *file = fopen(path, "rb");
    long length;
    if (!file || fseek(file, 0, SEEK_END) != 0 || (length = ftell(file)) <= 0 ||
        fseek(file, 0, SEEK_SET) != 0) {
        if (file) fclose(file);
        return 0;
    }
    *bytes = malloc((size_t)length);
    if (!*bytes || fread(*bytes, 1u, (size_t)length, file) != (size_t)length) {
        free(*bytes);
        *bytes = NULL;
        fclose(file);
        return 0;
    }
    fclose(file);
    *size = (size_t)length;
    return 1;
}

int main(void)
{
    const char *path = getenv("FIRESTAFF_CSB_AMIGA_APPB");
    const char *a35m_path = getenv("FIRESTAFF_CSB_AMIGA35M_APPB");
    CSB_V1_AmigaAppbSelectionReceipt receipt;
    uint8_t *bytes = NULL;
    uint8_t *pixels = NULL;
    size_t size = 0u;
    char md5[33];
    size_t pixel;
    uint64_t hash = UINT64_C(1469598103934665603);

    if (!path || !*path) return 77;
    if (!asset_file_md5_hex(path, md5) ||
        strcmp(md5, "35987d3f0278c6036fcc24786d4a75d7") != 0 ||
        !load_file(path, &bytes, &size)) {
        fputs("FAIL: APPB fixture is not original A31M media\n", stderr);
        return 1;
    }
    pixels = malloc(320u * 200u);
    if (!pixels || !csb_v1_amiga_appb_decode_language_selection(
            bytes, size, pixels, 320u * 200u, &receipt)) {
        fputs("FAIL: cannot decode original A31M APPB selection\n", stderr);
        free(pixels);
        free(bytes);
        return 1;
    }
    for (pixel = 0u; pixel < 320u * 200u; ++pixel) {
        hash ^= pixels[pixel];
        hash *= UINT64_C(1099511628211);
    }
    if (receipt.width != 320u || receipt.height != 200u ||
        receipt.rgb4[1][2] != 5u || receipt.rgb4[8][0] != 15u ||
        hash == UINT64_C(1469598103934665603)) {
        fputs("FAIL: APPB selection receipt is incomplete\n", stderr);
        free(pixels);
        free(bytes);
        return 1;
    }
    printf("ok: original A31M APPB selection hash %016llx\n",
           (unsigned long long)hash);
    free(pixels);
    free(bytes);
    if (!a35m_path || !*a35m_path) return 0;
    if (!asset_file_md5_hex(a35m_path, md5) ||
        strcmp(md5, "1533410beaeea4fa614ae0f0142e0861") != 0 ||
        !load_file(a35m_path, &bytes, &size)) {
        fputs("FAIL: APPB fixture is not original A35M media\n", stderr);
        return 1;
    }
    pixels = malloc(320u * 200u);
    if (!pixels || !csb_v1_amiga_a35m_appb_decode_language_selection(
            bytes, size, pixels, 320u * 200u, &receipt)) {
        fputs("FAIL: cannot decode original A35M APPB selection\n", stderr);
        free(pixels);
        free(bytes);
        return 1;
    }
    hash = UINT64_C(1469598103934665603);
    for (pixel = 0u; pixel < 320u * 200u; ++pixel) {
        hash ^= pixels[pixel];
        hash *= UINT64_C(1099511628211);
    }
    if (receipt.width != 320u || receipt.height != 200u ||
        receipt.rgb4[0][0] != 7u || receipt.rgb4[0][1] != 1u ||
        receipt.rgb4[0][2] != 7u ||
        hash != UINT64_C(0x8039bb203b9dfd26)) {
        fputs("FAIL: A35M APPB selection receipt is incomplete\n", stderr);
        free(pixels);
        free(bytes);
        return 1;
    }
    printf("ok: original A35M APPB selection hash %016llx\n",
           (unsigned long long)hash);
    free(pixels);
    free(bytes);
    return 0;
}
