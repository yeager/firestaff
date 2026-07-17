#include "nexus_v1_saturn_save_capture.h"

#include <string.h>

static uint64_t fnv1a64(const uint8_t *data, size_t size)
{
    uint64_t value = UINT64_C(1469598103934665603);
    size_t i;
    for (i = 0U; i < size; ++i) {
        value ^= data[i];
        value *= UINT64_C(1099511628211);
    }
    return value;
}

int nexus_v1_saturn_save_capture_admit(
    const Nexus_V1_SaturnSaveCaptureInput *input,
    Nexus_V1_SaturnSaveCaptureReceipt *out_receipt)
{
    uint64_t image_fnv;
    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    out_receipt->status = NEXUS_V1_SATURN_SAVE_CAPTURE_BLOCKED;
    out_receipt->opaque_only = 1;
    if (!input || !input->image ||
        input->image_size != NEXUS_V1_SATURN_SAVE_IMAGE_BYTES ||
        NEXUS_V1_SATURN_SAVE_BLOCK_COUNT * NEXUS_V1_SATURN_SAVE_BLOCK_BYTES !=
            NEXUS_V1_SATURN_SAVE_IMAGE_BYTES ||
        !input->expected_image_fnv1a64 ||
        !input->original_saturn_capture_authenticated ||
        !input->title_route_active || !input->champion_route_active ||
        input->native_save_fallback_permitted) return 0;
    image_fnv = fnv1a64(input->image, input->image_size);
    if (image_fnv != input->expected_image_fnv1a64) return 0;
    out_receipt->status = NEXUS_V1_SATURN_SAVE_CAPTURE_ADMITTED_OPAQUE;
    out_receipt->valid = 1;
    out_receipt->original_saturn_capture_authenticated = 1;
    out_receipt->title_route_bound = 1;
    out_receipt->champion_route_bound = 1;
    out_receipt->image_bytes = input->image_size;
    out_receipt->block_bytes = NEXUS_V1_SATURN_SAVE_BLOCK_BYTES;
    out_receipt->block_count = NEXUS_V1_SATURN_SAVE_BLOCK_COUNT;
    out_receipt->image_fnv1a64 = image_fnv;
    return 1;
}
