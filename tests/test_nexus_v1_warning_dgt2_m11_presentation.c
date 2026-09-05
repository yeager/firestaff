#include "nexus_v1_warning_dgt2_m11_presentation.h"
#include "nexus_v1_warning_dgt2_pp_payload_admission.h"
#include "nexus_v1_test_retail_member.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint8_t *read_file(const char *path, size_t *size)
{
    FILE *file;
    long length;
    uint8_t *bytes;
    char hash[65];
    if (strstr(path, "::"))
        return nexus_v1_test_read_retail_member(path, size, hash);
    *size = 0U;
    file = fopen(path, "rb");
    if (!file || fseek(file, 0, SEEK_END) || (length = ftell(file)) <= 0 ||
        fseek(file, 0, SEEK_SET)) { if (file) fclose(file); return NULL; }
    bytes = malloc((size_t)length);
    if (!bytes || fread(bytes, 1, (size_t)length, file) != (size_t)length) {
        free(bytes); fclose(file); return NULL;
    }
    fclose(file);
    *size = (size_t)length;
    return bytes;
}

static uint8_t expand_5_to_6(uint16_t value)
{
    return (uint8_t)((value << 1) | (value >> 4));
}

int main(int argc, char **argv)
{
    const char *path = argc == 2 ? argv[1] : getenv("FIRESTAFF_NEXUS_WARNING_PATH");
    Nexus_V1_WarningDgt2M11PresentationReceipt receipt;
    uint8_t *bytes;
    uint8_t framebuffer[320U * 200U];
    uint8_t before[320U * 200U];
    uint8_t rgb6[256][3];
    size_t size;
    uint32_t index;
    uint16_t word;

    if (!path || !*path || !(bytes = read_file(path, &size))) return 77;
    memset(framebuffer, 0xa5, sizeof(framebuffer));
    if (!nexus_v1_warning_dgt2_m11_present(bytes, size, framebuffer,
            sizeof(framebuffer), rgb6, &receipt) || !receipt.valid ||
        !receipt.canonical_source_bound || !receipt.pp_execution_bound ||
        !receipt.host_surface_written || !receipt.bgr555_to_rgb6_exact ||
        receipt.fallback_permitted || receipt.width != 240U ||
        receipt.height != 96U || receipt.stride != 240U) {
        free(bytes); return 1;
    }
    for (index = 0U; index < 96U; ++index) {
        if (memcmp(framebuffer + index * 320U, bytes + 0x256U + index * 240U,
                   240U) || framebuffer[index * 320U + 240U] != 0xa5U) {
            free(bytes); return 1;
        }
    }
    if (framebuffer[96U * 320U] != 0xa5U) { free(bytes); return 1; }
    for (index = 0U; index < 256U; ++index) {
        word = (uint16_t)((uint16_t)bytes[0x56U + index * 2U] << 8 |
                          bytes[0x57U + index * 2U]);
        if (rgb6[index][0] != expand_5_to_6((uint16_t)(word & 0x1fU)) ||
            rgb6[index][1] != expand_5_to_6((uint16_t)((word >> 5) & 0x1fU)) ||
            rgb6[index][2] != expand_5_to_6((uint16_t)((word >> 10) & 0x1fU))) {
            free(bytes); return 1;
        }
    }
    memcpy(before, framebuffer, sizeof(before));
    bytes[0x256U] ^= 1U;
    if (nexus_v1_warning_dgt2_m11_present(bytes, size, framebuffer,
            sizeof(framebuffer), rgb6, &receipt) || receipt.valid ||
        memcmp(framebuffer, before, sizeof(before))) { free(bytes); return 1; }
    bytes[0x256U] ^= 1U;
    if (nexus_v1_warning_dgt2_m11_present(bytes, size, framebuffer,
            sizeof(framebuffer) - 1U, rgb6, &receipt) || receipt.valid ||
        memcmp(framebuffer, before, sizeof(before))) { free(bytes); return 1; }
    free(bytes);
    puts("WARNING.BIN DGT2 PP M11 presentation: PASS");
    return 0;
}
