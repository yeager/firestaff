#include "nexus_v1_font256_s2d_section_witness.h"
#include "nexus_v1_test_retail_member.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static char real_sha256[65];

static uint64_t fnv1a64(const uint8_t *bytes, size_t size)
{
    uint64_t value = UINT64_C(1469598103934665603);
    size_t index;
    for (index = 0; index < size; ++index) { value ^= bytes[index]; value *= UINT64_C(1099511628211); }
    return value;
}

static uint8_t *read_file(const char *path, size_t *out_size)
{
    FILE *file; long length; uint8_t *bytes;
    if (strstr(path, "::")) return nexus_v1_test_read_retail_member(path,out_size,real_sha256);
    *out_size = 0; file = fopen(path, "rb");
    if (!file || fseek(file, 0, SEEK_END) != 0 || (length = ftell(file)) <= 0 || fseek(file, 0, SEEK_SET) != 0) { if (file) fclose(file); return NULL; }
    bytes = (uint8_t *)malloc((size_t)length);
    if (!bytes || fread(bytes, 1, (size_t)length, file) != (size_t)length) { free(bytes); fclose(file); return NULL; }
    fclose(file); *out_size = (size_t)length; return bytes;
}

int main(int argc, char **argv)
{
    const char *path = argc == 2 ? argv[1] : getenv("FIRESTAFF_NEXUS_FONT256_PATH");
    Nexus_V1_Font256S2DSourceIdentity identity;
    Nexus_V1_Font256S2DAdmissionReceipt admission;
    Nexus_V1_Font256S2DSectionWitnessReceipt witness;
    uint8_t *bytes; size_t size; uint8_t original;
    if (!path || !*path || !(bytes = read_file(path, &size))) return 77;
    memset(&identity, 0, sizeof(identity));
    identity.sha256_verified = 1; identity.sha256_hex = real_sha256[0] ? real_sha256 : NEXUS_V1_FONT256_S2D_SHA256;
    identity.source_fnv1a64 = fnv1a64(bytes, size);
    if (!nexus_v1_font256_s2d_admit(bytes, size, &identity, &admission) ||
        !nexus_v1_font256_s2d_first_section_witness(bytes, size, &admission, &witness) ||
        !witness.valid || !witness.preamble_capture_required ||
        witness.glyph_layout_proven || witness.palette_proven ||
        witness.pixel_decode_permitted || witness.draw_permitted ||
        witness.section_offset != 0x120U || witness.section_length != 0x2010U ||
        witness.preamble_length != 16U || !witness.preamble_fnv1a64) { free(bytes); return 1; }
    original = bytes[witness.preamble_offset]; bytes[witness.preamble_offset] ^= 1U;
    if (nexus_v1_font256_s2d_first_section_witness(bytes, size, &admission, &witness)) { free(bytes); return 1; }
    bytes[witness.preamble_offset] = original;
    admission.sections[0].size = 8U;
    if (nexus_v1_font256_s2d_first_section_witness(bytes, size, &admission, &witness)) { free(bytes); return 1; }
    free(bytes); puts("FONT256.S2D first-section witness: PASS"); return 0;
}
