#ifndef FIRESTAFF_DM1_V1_S0080_S0081_MEDIA_PLATFORM_BOUNDARY_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_S0080_S0081_MEDIA_PLATFORM_BOUNDARY_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct DM1_V1_S0080S0081MediaPlatformRequestPc34 {
    uint32_t raw_media_fingerprint;
    int original_pc34_media_verified;
    int no_dma_emulation;
    int no_floppy_emulation;
} DM1_V1_S0080S0081MediaPlatformRequestPc34;

typedef struct DM1_V1_S0080S0081MediaPlatformReceiptPc34 {
    int source_body_applicable;
    int execution_permitted;
    int fail_closed;
    int dma_completion_suppressed;
    int floppy_power_suppressed;
    const char *source_evidence;
} DM1_V1_S0080S0081MediaPlatformReceiptPc34;

int dm1_v1_s0080_check_dma_transfer_completion_boundary_pc34(
    const DM1_V1_S0080S0081MediaPlatformRequestPc34 *request,
    DM1_V1_S0080S0081MediaPlatformReceiptPc34 *out_receipt);
int dm1_v1_s0081_turn_off_floppy_drive_boundary_pc34(
    const DM1_V1_S0080S0081MediaPlatformRequestPc34 *request,
    DM1_V1_S0080S0081MediaPlatformReceiptPc34 *out_receipt);
const char *dm1_v1_s0080_s0081_media_platform_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_S0080_S0081_MEDIA_PLATFORM_BOUNDARY_PC34_COMPAT_H */
