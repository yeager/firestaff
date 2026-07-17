#include "nexus_v1_saturn_save_capture.h"

#include <string.h>

static uint64_t fnv1a64(const uint8_t *data, size_t size)
{
    uint64_t value = UINT64_C(1469598103934665603); size_t i;
    for (i = 0U; i < size; ++i) { value ^= data[i]; value *= UINT64_C(1099511628211); }
    return value;
}
int main(void)
{
    uint8_t corpus[NEXUS_V1_SATURN_SAVE_IMAGE_BYTES];
    Nexus_V1_SaturnSaveCaptureInput input;
    Nexus_V1_SaturnSaveCaptureReceipt receipt;
    size_t i;
    for (i = 0U; i < sizeof(corpus); ++i) corpus[i] = (uint8_t)(i * 37U + 11U);
    memset(&input, 0, sizeof(input)); input.image = corpus; input.image_size = sizeof(corpus);
    input.expected_image_fnv1a64 = fnv1a64(corpus, sizeof(corpus));
    input.original_saturn_capture_authenticated = input.title_route_active = input.champion_route_active = 1;
    if (!nexus_v1_saturn_save_capture_admit(&input, &receipt) || !receipt.valid ||
        receipt.status != NEXUS_V1_SATURN_SAVE_CAPTURE_ADMITTED_OPAQUE ||
        !receipt.opaque_only || receipt.native_save_fallback_permitted ||
        receipt.block_bytes != 512U || receipt.block_count != 16U) return 1;
    corpus[17] ^= 1U;
    if (nexus_v1_saturn_save_capture_admit(&input, &receipt) || receipt.valid) return 1;
    corpus[17] ^= 1U; input.image_size--;
    if (nexus_v1_saturn_save_capture_admit(&input, &receipt)) return 1;
    input.image_size++; input.champion_route_active = 0;
    if (nexus_v1_saturn_save_capture_admit(&input, &receipt)) return 1;
    return 0;
}
