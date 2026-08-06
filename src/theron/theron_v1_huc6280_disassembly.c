#include "theron_v1_huc6280_disassembly.h"

#include "asset_status_m12.h"
#include "theron_v1_track02.h"

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#if defined(_WIN32)
#include <io.h>
#define THERON_ACCESS _access
#define THERON_F_OK 0
#else
#include <unistd.h>
#define THERON_ACCESS access
#define THERON_F_OK F_OK
#endif

#define THERON_BANK1F_FILE_OFFSET 0x1f0000u
#define THERON_FRAGMENT_OFFSET 0x243eu
#define THERON_FRAGMENT_BYTES 134u
#define THERON_LEVEL_DECOMPRESSOR_OFFSET 0x23adu
#define THERON_LEVEL_DECOMPRESSOR_BYTES 382u
#define THERON_LEVEL_DECOMPRESSOR_FNV1A 0x3056f96cu
#define THERON_LEVEL_DECOMPRESSOR_CALLER_OFFSET 0x2386u
#define THERON_LEVEL_DECOMPRESSOR_CALLER_BYTES 30u
#define THERON_LEVEL_DECOMPRESSOR_CALLER_FNV1A 0x699e8da1u
#define THERON_STAGE2_RESOURCE_HANDLER_BANK_OFFSET 0x443fu
#define THERON_STAGE2_RESOURCE_HANDLER_ADDRESS 0x4c3fu
#define THERON_STAGE2_RESOURCE_HANDLER_BYTES 162u
#define THERON_STAGE2_RESOURCE_HANDLER_FNV1A 0x46360d97u
#define THERON_VCE_PALETTE_CONSUMER_BANK_OFFSET 0x9e15u
#define THERON_VCE_PALETTE_CONSUMER_ADDRESS 0x96a5u
#define THERON_VCE_PALETTE_CONSUMER_BYTES 37u
#define THERON_US_BIN_BANK_FILE_OFFSET 0x2bb200u
#define THERON_JP_BIN_BANK_FILE_OFFSET 0x2ba8d0u
#define THERON_US_BIN_SIZE 8104992u
#define THERON_JP_BIN_SIZE 8102640u
#define THERON_US_BIN_STAGE2_FNV1A 0x58cd4b73u
#define THERON_JP_BIN_STAGE2_FNV1A 0x788df8e7u

static const uint8_t g_fragment[THERON_FRAGMENT_BYTES] = {
    0xb2, 0x2e, 0x85, 0x0e, 0xe6, 0x2e, 0xd0, 0x02, 0xe6, 0x2f, 0x86, 0x0f,
    0xa5, 0x12, 0xd0, 0x02, 0xc6, 0x13, 0xc6, 0x12, 0xd0, 0xe0, 0xa5, 0x13,
    0xd0, 0xdc, 0x60, 0xad, 0x7e, 0x3b, 0x53, 0x08, 0xad, 0x7f, 0x3b, 0x53,
    0x10, 0xad, 0x80, 0x3b, 0x53, 0x20, 0xad, 0x81, 0x3b, 0x53, 0x40, 0x60,
    0xad, 0x82, 0x3b, 0x53, 0x08, 0xad, 0x83, 0x3b, 0x53, 0x10, 0xad, 0x84,
    0x3b, 0x53, 0x20, 0xad, 0x85, 0x3b, 0x53, 0x40, 0x60, 0xa5, 0x11, 0xf0,
    0x02, 0x10, 0x0d, 0x44, 0xe3, 0xa5, 0x10, 0x92, 0x30, 0xe6, 0x30, 0xd0,
    0x02, 0xe6, 0x31, 0x60, 0x38, 0xa5, 0x10, 0xe9, 0x01, 0x85, 0x10, 0xa5,
    0x11, 0xe9, 0x01, 0x85, 0x11, 0x06, 0x10, 0x26, 0x11, 0x18, 0xa5, 0x10,
    0x65, 0x32, 0x85, 0x10, 0xa5, 0x11, 0x65, 0x33, 0x85, 0x11, 0x85, 0x37,
    0xa5, 0x10, 0x85, 0x36, 0x44, 0x9d, 0xc2, 0xb1, 0x36, 0x85, 0x02, 0xc8,
    0xb1, 0x36
};

/* THQUEST.ASM stage-2 labels L96A5/L96C2. This exact 37-byte span contains
 * the VCE consumer entry and its inline TIA helper, byte-identical in the
 * authenticated US and JP raw Track 02 BINs. */
static const uint8_t g_vce_palette_consumer[
    THERON_VCE_PALETTE_CONSUMER_BYTES] = {
    0xad, 0xc2, 0x27, 0x8d, 0x02, 0x04, 0xad, 0xc3, 0x27,
    0x8d, 0x03, 0x04, 0x44, 0x17, 0xad, 0xc4, 0x27, 0x8d,
    0xc3, 0x56, 0xad, 0xc5, 0x27, 0x8d, 0xc4, 0x56, 0x44,
    0x01, 0x60, 0xe3, 0x00, 0x00, 0x04, 0x04, 0x20, 0x00,
    0x60
};

static uint32_t fnv1a(const uint8_t *bytes, size_t count) {
    uint32_t hash = 2166136261u;
    for (size_t i = 0u; i < count; ++i) {
        hash ^= bytes[i];
        hash *= 16777619u;
    }
    return hash;
}

static int expected_source(int variant,
                           uint32_t *size,
                           const char **md5,
                           uint32_t *bank_file_offset,
                           uint32_t *stage2_fnv1a) {
    if (variant == THERON_TRACK02_VARIANT_US_BIN) {
        /* Authentic raw Track 02 BIN: the MODE1/2352 framing places the
         * source bank-$1f window at this file offset. */
        *size = THERON_US_BIN_SIZE;
        *md5 = THERON_TRACK02_MD5_US_BIN;
        *bank_file_offset = THERON_US_BIN_BANK_FILE_OFFSET;
        *stage2_fnv1a = THERON_US_BIN_STAGE2_FNV1A;
        return 1;
    }
    if (variant == THERON_TRACK02_VARIANT_JP_BIN) {
        *size = THERON_JP_BIN_SIZE;
        *md5 = THERON_TRACK02_MD5_JP_BIN;
        *bank_file_offset = THERON_JP_BIN_BANK_FILE_OFFSET;
        *stage2_fnv1a = THERON_JP_BIN_STAGE2_FNV1A;
        return 1;
    }
    if (variant == THERON_TRACK02_VARIANT_US_ISO) {
        *size = 5984256u;
        *md5 = "51b40a17b92a30339957ba564aa0015c";
        *bank_file_offset = THERON_BANK1F_FILE_OFFSET;
        *stage2_fnv1a = THERON_STAGE2_RESOURCE_HANDLER_FNV1A;
        return 1;
    }
    if (variant == THERON_TRACK02_VARIANT_JP_REV1_ISO) {
        *size = 6291456u;
        *md5 = "f9f069a5e489b91207f3156059b756f1";
        *bank_file_offset = THERON_BANK1F_FILE_OFFSET;
        *stage2_fnv1a = THERON_STAGE2_RESOURCE_HANDLER_FNV1A;
        return 1;
    }
    return 0;
}

int theron_v1_huc6280_disassembly_read_file(
    const char *path,
    int track02_variant,
    Theron_V1Huc6280DisassemblyReceipt *out) {
    Theron_V1Huc6280DisassemblyReceipt receipt = {0};
    FILE *file = NULL;
    uint8_t bytes[THERON_FRAGMENT_BYTES];
    uint8_t decompressor[THERON_LEVEL_DECOMPRESSOR_BYTES];
    uint8_t decompressor_caller[THERON_LEVEL_DECOMPRESSOR_CALLER_BYTES];
    uint8_t stage2_resource_handler[THERON_STAGE2_RESOURCE_HANDLER_BYTES];
    uint8_t vce_palette_consumer[THERON_VCE_PALETTE_CONSUMER_BYTES];
    int raw_bin_variant;
    uint32_t expected_size;
    uint32_t bank_file_offset;
    uint32_t expected_stage2_fnv1a;
    const char *expected_md5;
    long file_size;
    struct stat path_stat;

    if (!out) return 0;
    *out = receipt;
    if (!path || !path[0] || !expected_source(track02_variant,
                                               &expected_size, &expected_md5,
                                               &bank_file_offset,
                                               &expected_stage2_fnv1a)) {
        receipt.status = THERON_V1_HUC6280_DISASSEMBLY_REJECTED;
        *out = receipt;
        return 1;
    }
    raw_bin_variant = track02_variant == THERON_TRACK02_VARIANT_US_BIN ||
        track02_variant == THERON_TRACK02_VARIANT_JP_BIN;
    if (THERON_ACCESS(path, THERON_F_OK) != 0) {
        receipt.status = THERON_V1_HUC6280_DISASSEMBLY_UNAVAILABLE;
        *out = receipt;
        return 1;
    }
    if (stat(path, &path_stat) != 0 || !S_ISREG(path_stat.st_mode)) {
        receipt.status = THERON_V1_HUC6280_DISASSEMBLY_REJECTED;
        *out = receipt;
        return 1;
    }
    if (!(file = fopen(path, "rb")) || fseek(file, 0L, SEEK_END) != 0 ||
        (file_size = ftell(file)) <= 0 || (uint32_t)file_size != expected_size ||
        fseek(file, (long)(bank_file_offset + THERON_FRAGMENT_OFFSET),
              SEEK_SET) != 0 || fread(bytes, 1u, sizeof(bytes), file) != sizeof(bytes) ||
        fseek(file, (long)(bank_file_offset +
                           THERON_LEVEL_DECOMPRESSOR_OFFSET), SEEK_SET) != 0 ||
        fread(decompressor, 1u, sizeof(decompressor), file) != sizeof(decompressor) ||
        fseek(file, (long)(bank_file_offset +
                           THERON_LEVEL_DECOMPRESSOR_CALLER_OFFSET),
              SEEK_SET) != 0 ||
        fread(decompressor_caller, 1u, sizeof(decompressor_caller), file) !=
            sizeof(decompressor_caller) ||
        fseek(file, (long)(bank_file_offset +
                           THERON_STAGE2_RESOURCE_HANDLER_BANK_OFFSET),
              SEEK_SET) != 0 ||
        fread(stage2_resource_handler, 1u, sizeof(stage2_resource_handler),
              file) != sizeof(stage2_resource_handler) ||
        (raw_bin_variant &&
         (fseek(file, (long)(bank_file_offset +
                             THERON_VCE_PALETTE_CONSUMER_BANK_OFFSET),
                  SEEK_SET) != 0 ||
          fread(vce_palette_consumer, 1u, sizeof(vce_palette_consumer),
                file) != sizeof(vce_palette_consumer))) ||
        !m12_file_md5_hex(path, receipt.source_md5)) {
        if (file) fclose(file);
        receipt.status = THERON_V1_HUC6280_DISASSEMBLY_REJECTED;
        *out = receipt;
        return 1;
    }
    fclose(file);

    if (strcmp(receipt.source_md5, expected_md5) != 0 ||
        memcmp(bytes, g_fragment, sizeof(bytes)) != 0 ||
        fnv1a(decompressor, sizeof(decompressor)) !=
            THERON_LEVEL_DECOMPRESSOR_FNV1A ||
        fnv1a(decompressor_caller, sizeof(decompressor_caller)) !=
            THERON_LEVEL_DECOMPRESSOR_CALLER_FNV1A ||
        fnv1a(stage2_resource_handler, sizeof(stage2_resource_handler)) !=
            expected_stage2_fnv1a ||
        (raw_bin_variant &&
         memcmp(vce_palette_consumer, g_vce_palette_consumer,
                sizeof(vce_palette_consumer)) != 0) ||
        decompressor[0] != 0xa5u || decompressor[1] != 0x2eu ||
        decompressor[2] != 0x85u || decompressor[3] != 0x32u ||
        decompressor[sizeof(decompressor) - 1u] != 0x60u) {
        receipt.status = THERON_V1_HUC6280_DISASSEMBLY_REJECTED;
        *out = receipt;
        return 1;
    }
    receipt.status = THERON_V1_HUC6280_DISASSEMBLY_READY;
    receipt.source_file_identity_verified = 1;
    receipt.bank_window_verified = 1;
    receipt.forward_byte_step_verified = 1;
    receipt.bank_switch_table_verified = 1;
    receipt.reverse_byte_read_verified = 1;
    receipt.level_decompressor_fragment_verified = 1;
    receipt.level_decompressor_caller_verified = 1;
    receipt.stage2_resource_handler_verified = 1;
    receipt.stage2_resource_bank_table_population_verified = 1;
    receipt.stage2_resource_destination_registers_verified = 1;
    receipt.semantic_publication_allowed = 0;
    receipt.source_file_size = expected_size;
    receipt.bank_file_offset = bank_file_offset;
    receipt.fragment_address = THERON_FRAGMENT_OFFSET;
    receipt.fragment_bytes = THERON_FRAGMENT_BYTES;
    receipt.fragment_fnv1a = fnv1a(bytes, sizeof(bytes));
    receipt.level_decompressor_address = THERON_LEVEL_DECOMPRESSOR_OFFSET;
    receipt.level_decompressor_bytes = THERON_LEVEL_DECOMPRESSOR_BYTES;
    receipt.level_decompressor_fnv1a = fnv1a(decompressor, sizeof(decompressor));
    receipt.level_decompressor_caller_address = THERON_LEVEL_DECOMPRESSOR_CALLER_OFFSET;
    receipt.level_decompressor_caller_bytes = THERON_LEVEL_DECOMPRESSOR_CALLER_BYTES;
    receipt.level_decompressor_caller_fnv1a = fnv1a(
        decompressor_caller, sizeof(decompressor_caller));
    receipt.stage2_resource_handler_address =
        THERON_STAGE2_RESOURCE_HANDLER_ADDRESS;
    receipt.stage2_resource_handler_bytes =
        THERON_STAGE2_RESOURCE_HANDLER_BYTES;
    receipt.stage2_resource_handler_fnv1a = fnv1a(
        stage2_resource_handler, sizeof(stage2_resource_handler));
    receipt.vce_palette_consumer_verified = raw_bin_variant;
    if (raw_bin_variant) {
        receipt.vce_palette_consumer_address =
            THERON_VCE_PALETTE_CONSUMER_ADDRESS;
        receipt.vce_palette_consumer_bytes = THERON_VCE_PALETTE_CONSUMER_BYTES;
        receipt.vce_palette_consumer_file_offset = bank_file_offset +
            THERON_VCE_PALETTE_CONSUMER_BANK_OFFSET;
        receipt.vce_palette_consumer_fnv1a = fnv1a(
            vce_palette_consumer, sizeof(vce_palette_consumer));
    }
    *out = receipt;
    return 1;
}
