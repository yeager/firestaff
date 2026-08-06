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

static uint32_t fnv1a(const uint8_t *bytes, size_t count) {
    uint32_t hash = 2166136261u;
    for (size_t i = 0u; i < count; ++i) {
        hash ^= bytes[i];
        hash *= 16777619u;
    }
    return hash;
}

static int expected_source(int variant, uint32_t *size, const char **md5) {
    if (variant == THERON_TRACK02_VARIANT_US_ISO) {
        *size = 5984256u;
        *md5 = "51b40a17b92a30339957ba564aa0015c";
        return 1;
    }
    if (variant == THERON_TRACK02_VARIANT_JP_REV1_ISO) {
        *size = 6291456u;
        *md5 = "f9f069a5e489b91207f3156059b756f1";
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
    uint32_t expected_size;
    const char *expected_md5;
    long file_size;
    struct stat path_stat;

    if (!out) return 0;
    *out = receipt;
    if (!path || !path[0] || !expected_source(track02_variant,
                                               &expected_size, &expected_md5)) {
        receipt.status = THERON_V1_HUC6280_DISASSEMBLY_REJECTED;
        *out = receipt;
        return 1;
    }
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
        fseek(file, (long)(THERON_BANK1F_FILE_OFFSET + THERON_FRAGMENT_OFFSET),
              SEEK_SET) != 0 || fread(bytes, 1u, sizeof(bytes), file) != sizeof(bytes) ||
        !m12_file_md5_hex(path, receipt.source_md5)) {
        if (file) fclose(file);
        receipt.status = THERON_V1_HUC6280_DISASSEMBLY_REJECTED;
        *out = receipt;
        return 1;
    }
    fclose(file);

    if (strcmp(receipt.source_md5, expected_md5) != 0 ||
        memcmp(bytes, g_fragment, sizeof(bytes)) != 0) {
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
    receipt.semantic_publication_allowed = 0;
    receipt.source_file_size = expected_size;
    receipt.bank_file_offset = THERON_BANK1F_FILE_OFFSET;
    receipt.fragment_address = THERON_FRAGMENT_OFFSET;
    receipt.fragment_bytes = THERON_FRAGMENT_BYTES;
    receipt.fragment_fnv1a = fnv1a(bytes, sizeof(bytes));
    *out = receipt;
    return 1;
}
