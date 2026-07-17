#include "nexus_v1_warning_dgt2_m11_presentation.h"

#include "nexus_v1_warning_dgt2_pp_execution.h"

#include <string.h>

typedef struct {
    uint8_t *framebuffer;
    uint8_t (*rgb6)[3];
    int called;
} Nexus_V1_WarningM11PresentContext;

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

static uint8_t expand_5_to_6(uint16_t value)
{
    return (uint8_t)((value << 1) | (value >> 4));
}

static int nexus_v1_warning_m11_present_callback(
    void *userdata, const uint8_t *pixels, uint16_t width, uint16_t height,
    uint32_t stride, const uint16_t *bgr555_words, uint32_t word_count)
{
    Nexus_V1_WarningM11PresentContext *context = userdata;
    uint8_t rgb6[256][3];
    uint32_t entry;
    uint16_t word;
    uint16_t red;
    uint16_t green;
    uint16_t blue;
    uint16_t row;

    if (!context || !context->framebuffer || !context->rgb6 || !pixels ||
        !bgr555_words || width != 240U || height != 96U || stride != 240U ||
        word_count != 256U) {
        return 0;
    }
    for (entry = 0U; entry < 256U; ++entry) {
        word = bgr555_words[entry];
        /* ST-124 section 6: B4..B0, G4..G0, R4..R0, high to low. */
        red = (uint16_t)(word & 0x1fU);
        green = (uint16_t)((word >> 5) & 0x1fU);
        blue = (uint16_t)((word >> 10) & 0x1fU);
        rgb6[entry][0] = expand_5_to_6(red);
        rgb6[entry][1] = expand_5_to_6(green);
        rgb6[entry][2] = expand_5_to_6(blue);
    }
    for (row = 0U; row < height; ++row) {
        memcpy(context->framebuffer + (size_t)row * NEXUS_V1_WARNING_M11_WIDTH,
               pixels + (size_t)row * stride, width);
    }
    memcpy(context->rgb6, rgb6, sizeof(rgb6));
    context->called = 1;
    return 1;
}

int nexus_v1_warning_dgt2_m11_present(
    const uint8_t *source_bytes, size_t source_size, uint8_t *m11_framebuffer,
    size_t m11_framebuffer_size, uint8_t out_rgb6[256][3],
    Nexus_V1_WarningDgt2M11PresentationReceipt *out_receipt)
{
    Nexus_V1_WarningDgt2SourceIdentity identity;
    Nexus_V1_WarningDgt2SourceAdmissionReceipt source;
    Nexus_V1_WarningDgt2DescriptorAdmissionReceipt descriptor;
    Nexus_V1_WarningDgt2PpPayloadAdmissionReceipt payload;
    Nexus_V1_WarningDgt2PpExecutionReceipt execution;
    Nexus_V1_WarningDgt2M11PresentationReceipt receipt;
    Nexus_V1_WarningM11PresentContext context;
    uint8_t indexed_pixels[240U * 96U];
    uint16_t bgr555_words[256];

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    if (!source_bytes || !m11_framebuffer || !out_rgb6 ||
        m11_framebuffer_size !=
            (size_t)NEXUS_V1_WARNING_M11_WIDTH * NEXUS_V1_WARNING_M11_HEIGHT) {
        *out_receipt = receipt;
        return 0;
    }
    memset(&identity, 0, sizeof(identity));
    identity.canonical_fnv1a64_verified = 1;
    identity.source_fnv1a64 = fnv1a64(source_bytes, source_size);
    if (!nexus_v1_warning_dgt2_source_admit(source_bytes, source_size,
            &identity, &source) ||
        !nexus_v1_warning_dgt2_descriptor_admit(source_bytes, source_size,
            &source, &descriptor) ||
        !nexus_v1_warning_dgt2_pp_payload_admit(source_bytes, source_size,
            &descriptor, &payload)) {
        *out_receipt = receipt;
        return 0;
    }
    memset(&context, 0, sizeof(context));
    context.framebuffer = m11_framebuffer;
    context.rgb6 = out_rgb6;
    if (!nexus_v1_warning_dgt2_pp_execute(source_bytes, source_size, &payload,
            indexed_pixels, sizeof(indexed_pixels), bgr555_words,
            nexus_v1_warning_m11_present_callback, &context, &execution) ||
        !context.called) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.valid = receipt.canonical_source_bound = receipt.pp_execution_bound = 1;
    receipt.host_surface_written = receipt.bgr555_to_rgb6_exact = 1;
    receipt.source_fnv1a64 = execution.source_fnv1a64;
    receipt.index_plane_fnv1a64 = execution.index_plane_fnv1a64;
    receipt.bgr555_words_fnv1a64 = execution.bgr555_words_fnv1a64;
    receipt.host_palette_rgb6_fnv1a64 = fnv1a64(&out_rgb6[0][0], 256U * 3U);
    receipt.width = execution.width;
    receipt.height = execution.height;
    receipt.stride = execution.stride;
    *out_receipt = receipt;
    return 1;
}
