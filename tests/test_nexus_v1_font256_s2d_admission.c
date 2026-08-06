#include "nexus_v1_font256_s2d_admission.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint64_t fnv1a64(const uint8_t *bytes, size_t size)
{
    uint64_t value = UINT64_C(1469598103934665603);
    size_t index;
    for (index = 0; index < size; ++index) {
        value ^= bytes[index];
        value *= UINT64_C(1099511628211);
    }
    return value;
}

static uint8_t *read_file(const char *path, size_t *out_size)
{
    FILE *file;
    long length;
    uint8_t *bytes;
    *out_size = 0;
    file = fopen(path, "rb");
    if (!file || fseek(file, 0, SEEK_END) != 0 || (length = ftell(file)) <= 0 ||
        fseek(file, 0, SEEK_SET) != 0) {
        if (file) fclose(file);
        return NULL;
    }
    bytes = (uint8_t *)malloc((size_t)length);
    if (!bytes || fread(bytes, 1, (size_t)length, file) != (size_t)length) {
        free(bytes);
        fclose(file);
        return NULL;
    }
    fclose(file);
    *out_size = (size_t)length;
    return bytes;
}

int main(int argc, char **argv)
{
    const char *path = argc == 2 ? argv[1] : getenv("FIRESTAFF_NEXUS_FONT256_PATH");
    Nexus_V1_Font256S2DSourceIdentity identity;
    Nexus_V1_Font256S2DAdmissionReceipt receipt;
    uint8_t *bytes;
    size_t size;
    uint8_t original;

    if (!path || !*path) {
        puts("FONT256.S2D admission: SKIP (no local path)");
        return 77;
    }
    bytes = read_file(path, &size);
    if (!bytes) {
        puts("FONT256.S2D admission: SKIP (file unavailable)");
        return 77;
    }
    memset(&identity, 0, sizeof(identity));
    identity.sha256_verified = 1;
    identity.sha256_hex = getenv("FIRESTAFF_NEXUS_FONT256_SHA256");
    if (!identity.sha256_hex) {
        identity.sha256_hex = NEXUS_V1_FONT256_S2D_SHA256;
    }
    identity.source_fnv1a64 = fnv1a64(bytes, size);
    if (!nexus_v1_font256_s2d_admit(bytes, size, &identity, &receipt) ||
        !receipt.valid || !receipt.source_identity_bound ||
        !receipt.scr_header_bound || !receipt.section_table_bound ||
        receipt.glyph_layout_proven || receipt.pixel_decode_permitted ||
        receipt.draw_permitted || receipt.section_count != 4U) {
        free(bytes);
        return 1;
    }
    if (!receipt.section_table_fnv1a64 || !receipt.section_fnv1a64[0] ||
        !receipt.section_fnv1a64[1] || !receipt.section_fnv1a64[2] ||
        !receipt.section_fnv1a64[3]) {
        free(bytes);
        return 1;
    }

    identity.sha256_verified = 0;
    if (nexus_v1_font256_s2d_admit(bytes, size, &identity, &receipt)) {
        free(bytes);
        return 1;
    }
    identity.sha256_verified = 1;
    original = bytes[0x20];
    bytes[0x20] ^= 1U;
    if (nexus_v1_font256_s2d_admit(bytes, size, &identity, &receipt)) {
        free(bytes);
        return 1;
    }
    bytes[0x20] = original;
    if (nexus_v1_font256_s2d_admit(bytes, size - 1U, &identity, &receipt)) {
        free(bytes);
        return 1;
    }
    free(bytes);
    puts("FONT256.S2D admission: PASS");
    return 0;
}
