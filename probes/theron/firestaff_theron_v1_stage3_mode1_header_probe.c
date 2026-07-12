#include "theron_v1_stage3_mode1_header.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int g_fail;

static void check(int condition, const char *name) {
    if (!condition) {
        ++g_fail;
        printf("[FAIL] %s\n", name);
    } else {
        printf("[PASS] %s\n", name);
    }
}

static uint8_t *read_file_bytes(const char *path, size_t *out_size) {
    FILE *file = NULL;
    long size;
    uint8_t *bytes = NULL;

    if (!path || !out_size || !(file = fopen(path, "rb")) ||
        fseek(file, 0L, SEEK_END) != 0 || (size = ftell(file)) <= 0 ||
        fseek(file, 0L, SEEK_SET) != 0 ||
        !(bytes = (uint8_t *)malloc((size_t)size)) ||
        fread(bytes, 1u, (size_t)size, file) != (size_t)size) {
        if (file) fclose(file);
        free(bytes);
        return NULL;
    }
    fclose(file);
    *out_size = (size_t)size;
    return bytes;
}

static Theron_Track02Stage2DynamicPayloadReceipt payload(
    Theron_Track02Variant variant,
    uint32_t record) {
    Theron_Track02Stage2DynamicPayloadReceipt receipt;

    memset(&receipt, 0, sizeof(receipt));
    receipt.valid = 1;
    receipt.variant = variant;
    receipt.track02_record = record;
    receipt.raw_sector = record;
    receipt.user_data_offset = (size_t)record * 2352u + 16u;
    receipt.user_data_bytes = 2048u;
    return receipt;
}

int main(int argc, char **argv) {
    Theron_V1Stage3Mode1HeaderReceipt jp;
    Theron_V1Stage3Mode1HeaderReceipt us;
    Theron_Track02Stage2DynamicPayloadReceipt jp_payload;
    Theron_Track02Stage2DynamicPayloadReceipt us_payload;
    uint8_t *jp_bytes;
    uint8_t *us_bytes;
    size_t jp_size;
    size_t us_size;

    if (argc != 3) {
        printf("[FAIL] expected JP and US raw Track02 paths\n");
        return 1;
    }
    jp_bytes = read_file_bytes(argv[1], &jp_size);
    us_bytes = read_file_bytes(argv[2], &us_size);
    check(jp_bytes && us_bytes, "raw JP/US Track02 files read");
    jp_payload = payload(THERON_TRACK02_VARIANT_JP_BIN, 0x0004dfu);
    us_payload = payload(THERON_TRACK02_VARIANT_US_BIN, 0x0004e0u);
    check(jp_bytes && theron_v1_stage3_mode1_header_from_original_media(
              jp_bytes, jp_size, &jp_payload, &jp),
          "JP stage-three sector has a strict MODE1 envelope");
    check(us_bytes && theron_v1_stage3_mode1_header_from_original_media(
              us_bytes, us_size, &us_payload, &us),
          "US stage-three sector has a strict MODE1 envelope");
    check(jp.valid && us.valid && jp.minute_bcd == 0x01u &&
              jp.second_bcd == 0x03u && jp.frame_bcd == 0x38u &&
              us.minute_bcd == 0x00u && us.second_bcd == 0x58u &&
              us.frame_bcd == 0x57u && jp.mode == 1u && us.mode == 1u &&
              jp.user_data_offset == (size_t)0x4dfu * 2352u + 16u &&
              us.user_data_offset == (size_t)0x4e0u * 2352u + 16u,
          "JP/US physical sector provenance remains byte-stable");
    free(jp_bytes);
    free(us_bytes);
    printf("--- %d failed ---\n", g_fail);
    return g_fail ? 1 : 0;
}
