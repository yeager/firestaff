#include "dm1_v1_amiga_swsh.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
    kSectorBytes = 512,
    kSectorCount = 11,
    kBootBytes = 1024,
    kAdfBytes = 901120
};

static void write_be32(uint8_t* p, uint32_t value) {
    p[0] = (uint8_t)(value >> 24);
    p[1] = (uint8_t)(value >> 16);
    p[2] = (uint8_t)(value >> 8);
    p[3] = (uint8_t)value;
}

static void test_sector_11_copyright(void) {
    static const char marker[] = "Copyright (C) 1988 Software Heaven, Inc.";
    uint8_t* disk = (uint8_t*)calloc(kAdfBytes, 1u);
    assert(disk != NULL);
    memcpy(disk + (size_t)kSectorBytes * kSectorCount,
           marker, sizeof(marker) - 1u);
    assert(dm1_v1_amiga_original_disk_check(disk, kAdfBytes, 0));
    assert(!dm1_v1_amiga_original_disk_check(
        disk, (size_t)kSectorBytes * kSectorCount, 1));
    free(disk);
}

static void test_boot_sector_fallback(void) {
    static const char copyright[] =
        "Copyright (C) 1988 Software Heaven Inc. / FTL Games";
    uint8_t* disk = (uint8_t*)calloc(kAdfBytes, 1u);
    size_t source = 0u;
    assert(disk != NULL);
    write_be32(disk + 4u, UINT32_C(0xe3a6ec5b));
    for (size_t offset = 52u; offset < kBootBytes; ++offset) {
        disk[offset] = (uint8_t)copyright[source++];
        if (copyright[source] == '\0') source = 0u;
    }
    assert(dm1_v1_amiga_original_disk_check(disk, kAdfBytes, 1));
    assert(!dm1_v1_amiga_original_disk_check(disk, kAdfBytes, 0));
    disk[400u] ^= 1u;
    assert(!dm1_v1_amiga_original_disk_check(disk, kAdfBytes, 1));
    disk[400u] ^= 1u;
    write_be32(disk + 4u, 0u);
    assert(!dm1_v1_amiga_original_disk_check(disk, kAdfBytes, 1));
    free(disk);
}

int main(void) {
    test_sector_11_copyright();
    test_boot_sector_fallback();
    puts("PASS: ReDMCSB F0906/F0907 Amiga original disk check");
    return 0;
}
