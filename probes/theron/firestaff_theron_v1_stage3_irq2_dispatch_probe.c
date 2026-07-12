#include "asset_status_m12.h"
#include "theron_v1_stage3_irq2_dispatch.h"
#include "theron_v1_track02.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int g_fail;
static int g_skip;

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

static int inspect(const char *path,
                   const char *md5_hex,
                   Theron_V1Stage3Irq2DispatchReceipt *out_receipt) {
    Theron_Track02Stage2DynamicPayloadReceipt payload;
    uint8_t *bytes;
    size_t size;
    char actual_md5[33];
    int ok;

    bytes = read_file_bytes(path, &size);
    if (!bytes) return 0;
    ok = m12_file_md5_hex(path, actual_md5) &&
        strcmp(actual_md5, md5_hex) == 0 &&
        theron_v1_track02_inspect_stage2_dynamic_payload(
            bytes, size, md5_hex, &payload) == THERON_TRACK02_SIGNAL_OK &&
        theron_v1_stage3_irq2_dispatch_from_dynamic_payload(&payload,
                                                             out_receipt);
    free(bytes);
    return ok;
}

static void check_variant(const char *path,
                          const char *md5_hex,
                          uint32_t expected_record,
                          const char *name) {
    Theron_V1Stage3Irq2DispatchReceipt receipt;

    check(inspect(path, md5_hex, &receipt), name);
    check(receipt.valid && receipt.track02_record == expected_record &&
              receipt.entry_address == 0x3800u && receipt.opcode == 0x00u &&
              receipt.irq2_selector == 0xffu &&
              receipt.continuation_address == 0x3802u &&
              receipt.irq2_vector_address == 0xfff6u &&
              receipt.irq2_dispatch_proven && receipt.manifest_not_linear_cpu_code,
          "stage-three entry is BRK IRQ2 dispatch, not linear manifest code");
}

int main(void) {
    const char *jp_path = getenv("FIRESTAFF_THERON_TRACK02_JP_BIN");
    const char *us_path = getenv("FIRESTAFF_THERON_TRACK02_US_BIN");

    if (!jp_path || !us_path) {
        ++g_skip;
        printf("[SKIP] set FIRESTAFF_THERON_TRACK02_JP_BIN and FIRESTAFF_THERON_TRACK02_US_BIN\n");
        return 0;
    }
    check_variant(jp_path, THERON_TRACK02_MD5_JP_BIN, 0x0004dfu,
                  "JP raw Track02 has a hash-gated stage-three IRQ2 entry");
    check_variant(us_path, THERON_TRACK02_MD5_US_BIN, 0x0004e0u,
                  "US raw Track02 has a hash-gated stage-three IRQ2 entry");
    printf("--- %d failed, %d skipped ---\n", g_fail, g_skip);
    return g_fail ? 1 : 0;
}
