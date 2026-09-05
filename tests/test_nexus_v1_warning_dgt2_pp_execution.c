#include "nexus_v1_warning_dgt2_pp_execution.h"
#include "nexus_v1_test_retail_member.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int calls;
    int fail;
    const uint8_t *expected_pixels;
    const uint16_t *expected_palette;
} PresentState;

static uint64_t fnv1a64(const uint8_t *p, size_t n)
{
    uint64_t h = UINT64_C(1469598103934665603);
    size_t i;
    for (i = 0; i < n; ++i) { h ^= p[i]; h *= UINT64_C(1099511628211); }
    return h;
}

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

static int present(void *context, const uint8_t *pixels, uint16_t width,
                   uint16_t height, uint32_t stride, const uint16_t *palette,
                   uint32_t palette_count)
{
    PresentState *state = context;
    ++state->calls;
    return !state->fail && width == 240U && height == 96U && stride == 240U &&
        palette_count == 256U && pixels == state->expected_pixels &&
        palette == state->expected_palette;
}

int main(int argc, char **argv)
{
    const char *path = argc == 2 ? argv[1] : getenv("FIRESTAFF_NEXUS_WARNING_PATH");
    Nexus_V1_WarningDgt2SourceIdentity identity;
    Nexus_V1_WarningDgt2SourceAdmissionReceipt source;
    Nexus_V1_WarningDgt2DescriptorAdmissionReceipt descriptor;
    Nexus_V1_WarningDgt2PpPayloadAdmissionReceipt payload;
    Nexus_V1_WarningDgt2PpExecutionReceipt execution;
    PresentState state;
    uint8_t *bytes;
    uint8_t *pixels;
    uint16_t palette[256];
    size_t size;
    uint8_t saved;

    if (!path || !*path || !(bytes = read_file(path, &size))) return 77;
    memset(&identity, 0, sizeof(identity));
    identity.sha256_verified = 1;
    identity.sha256_hex = NEXUS_V1_WARNING_BIN_SHA256;
    identity.source_fnv1a64 = fnv1a64(bytes, size);
    if (!nexus_v1_warning_dgt2_source_admit(bytes, size, &identity, &source) ||
        !nexus_v1_warning_dgt2_descriptor_admit(bytes, size, &source, &descriptor) ||
        !nexus_v1_warning_dgt2_pp_payload_admit(bytes, size, &descriptor, &payload) ||
        !(pixels = malloc(payload.declared_body_length))) { free(bytes); return 1; }
    memset(&state, 0, sizeof(state));
    state.expected_pixels = pixels;
    state.expected_palette = palette;
    if (!nexus_v1_warning_dgt2_pp_execute(bytes, size, &payload, pixels,
            payload.declared_body_length, palette, present, &state, &execution) ||
        !execution.valid || !execution.pp_256_indexed_proven ||
        !execution.bgr555_clut_proven || !execution.stride_proven ||
        !execution.presented || execution.fallback_permitted || state.calls != 1 ||
        execution.width != 240U || execution.height != 96U || execution.stride != 240U ||
        execution.bgr555_word_count != 256U || execution.trailing_raw_bytes != 2U ||
        memcmp(pixels, bytes + payload.declared_body_offset, payload.declared_body_length) ||
        palette[0] != (uint16_t)((uint16_t)bytes[payload.post_header_prefix_offset] << 8 |
                                 bytes[payload.post_header_prefix_offset + 1U]) ||
        palette[255] != (uint16_t)((uint16_t)bytes[payload.post_header_prefix_offset + 510U] << 8 |
                                   bytes[payload.post_header_prefix_offset + 511U])) {
        free(pixels); free(bytes); return 1;
    }
    state.fail = 1;
    if (nexus_v1_warning_dgt2_pp_execute(bytes, size, &payload, pixels,
            payload.declared_body_length, palette, present, &state, &execution) || execution.valid) {
        free(pixels); free(bytes); return 1;
    }
    saved = bytes[payload.declared_body_offset];
    bytes[payload.declared_body_offset] ^= 1U;
    state.fail = 0;
    if (nexus_v1_warning_dgt2_pp_execute(bytes, size, &payload, pixels,
            payload.declared_body_length, palette, present, &state, &execution) || execution.valid) {
        free(pixels); free(bytes); return 1;
    }
    bytes[payload.declared_body_offset] = saved;
    if (nexus_v1_warning_dgt2_pp_execute(bytes, size, &payload, pixels,
            payload.declared_body_length, palette, NULL, &state, &execution) || execution.valid) {
        free(pixels); free(bytes); return 1;
    }
    free(pixels);
    free(bytes);
    puts("WARNING.BIN DGT2 PP execution: PASS");
    return 0;
}
