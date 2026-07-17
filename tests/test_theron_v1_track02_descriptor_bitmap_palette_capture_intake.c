#include "theron_v1_track02_descriptor_bitmap_palette_capture_intake.h"

#include <stdio.h>

int main(void)
{
    Theron_V1Track02DescriptorBitmapPaletteCaptureIntakeReceipt receipt;
    Theron_V1Track02LevelObjectDescriptorCaptureIntakeReceipt descriptor = {0};
    Theron_V1Track02CaptureArtifactRuntimeAdmissionReceipt artifact = {0};

    if (!theron_v1_track02_descriptor_bitmap_palette_capture_intake_admit(
            NULL, NULL, NULL, NULL, NULL, NULL, 0u, 0u, &receipt) ||
        receipt.status != THERON_V1_TRACK02_DESCRIPTOR_BITMAP_PALETTE_CAPTURE_UNAVAILABLE ||
        receipt.opaque_presentation_only) return 1;
    descriptor.status = THERON_V1_TRACK02_LEVEL_OBJECT_DESCRIPTOR_CAPTURE_READY;
    artifact.status = THERON_V1_TRACK02_CAPTURE_ARTIFACT_READY;
    if (!theron_v1_track02_descriptor_bitmap_palette_capture_intake_admit(
            &descriptor, NULL, NULL, NULL, NULL, &artifact, 7u, 3u, &receipt) ||
        receipt.status != THERON_V1_TRACK02_DESCRIPTOR_BITMAP_PALETTE_CAPTURE_REJECTED ||
        receipt.opaque_presentation_only) return 2;
    puts("test_theron_v1_track02_descriptor_bitmap_palette_capture_intake: PASS (no local capture)");
    return 0;
}
