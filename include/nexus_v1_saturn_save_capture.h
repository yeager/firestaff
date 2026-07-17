#ifndef NEXUS_V1_SATURN_SAVE_CAPTURE_H
#define NEXUS_V1_SATURN_SAVE_CAPTURE_H

#include <stddef.h>
#include <stdint.h>

#define NEXUS_V1_SATURN_SAVE_IMAGE_BYTES 8192U
#define NEXUS_V1_SATURN_SAVE_BLOCK_BYTES 512U
#define NEXUS_V1_SATURN_SAVE_BLOCK_COUNT 16U

typedef enum {
    NEXUS_V1_SATURN_SAVE_CAPTURE_BLOCKED = 0,
    NEXUS_V1_SATURN_SAVE_CAPTURE_ADMITTED_OPAQUE = 1
} Nexus_V1_SaturnSaveCaptureStatus;

typedef struct {
    const uint8_t *image;
    size_t image_size;
    uint64_t expected_image_fnv1a64;
    int original_saturn_capture_authenticated;
    int title_route_active;
    int champion_route_active;
    int native_save_fallback_permitted;
} Nexus_V1_SaturnSaveCaptureInput;

typedef struct Nexus_V1_SaturnSaveCaptureReceipt {
    Nexus_V1_SaturnSaveCaptureStatus status;
    int valid;
    int original_saturn_capture_authenticated;
    int title_route_bound;
    int champion_route_bound;
    size_t image_bytes;
    uint32_t block_bytes;
    uint32_t block_count;
    uint64_t image_fnv1a64;
    int opaque_only;
    int native_save_fallback_permitted;
} Nexus_V1_SaturnSaveCaptureReceipt;

int nexus_v1_saturn_save_capture_admit(
    const Nexus_V1_SaturnSaveCaptureInput *input,
    Nexus_V1_SaturnSaveCaptureReceipt *out_receipt);

#endif
