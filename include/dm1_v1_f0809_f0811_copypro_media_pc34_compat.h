#ifndef FIRESTAFF_DM1_V1_F0809_F0811_COPYPRO_MEDIA_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_F0809_F0811_COPYPRO_MEDIA_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

enum {
    DM1_V1_F0809_COPYPRO_DIAGNOSTIC_BYTES_PC34 = 1088,
    DM1_V1_F0810_COPYPRO_DIAGNOSTIC_BYTES_PC34 = 5840,
    DM1_V1_F0810_COPYPRO_CHECKED_BYTES_PC34 = 5836
};

typedef enum DM1_V1_CopyProtectionMediaKindPc34 {
    DM1_V1_COPYPRO_MEDIA_NONE_PC34 = 0,
    DM1_V1_COPYPRO_MEDIA_INSTALL_FILES_PC34,
    DM1_V1_COPYPRO_MEDIA_SECTOR_IMAGE_PC34,
    DM1_V1_COPYPRO_MEDIA_AUTHENTICATED_FLUX_CAPTURE_PC34
} DM1_V1_CopyProtectionMediaKindPc34;

typedef struct DM1_V1_F0811ReadIdPc34 {
    uint8_t dl;
    uint8_t cl;
    uint8_t dh;
    uint8_t ch;
} DM1_V1_F0811ReadIdPc34;

typedef struct DM1_V1_CopyProtectionCapturePc34 {
    DM1_V1_CopyProtectionMediaKindPc34 mediaKind;
    int mediaHashVerified;
    const unsigned char* track0Diagnostic;
    size_t track0DiagnosticBytes;
    const unsigned char* track2Diagnostic;
    size_t track2DiagnosticBytes;
    DM1_V1_F0811ReadIdPc34 track0ReadId;
} DM1_V1_CopyProtectionCapturePc34;

typedef struct DM1_V1_CopyProtectionReceiptPc34 {
    int valid;
    int suppressSyntheticDiskResponse;
    int functionId;
    int expectedCylinder;
    size_t requiredBytes;
} DM1_V1_CopyProtectionReceiptPc34;

/* IO.C F0809/F0810/F0811 consume controller-captured diagnostic bytes only.
 * A mounted install, an HDM/FDI sector dump, or caller-invented BIOS status
 * never counts as the intentionally unreadable CPSX protection track. */
int dm1_v1_f0809_f0811_verify_copy_protection_capture_pc34(
    int functionId,
    const DM1_V1_CopyProtectionCapturePc34* capture,
    DM1_V1_CopyProtectionReceiptPc34* outReceipt);

const char* dm1_v1_f0809_f0811_copypro_source_evidence_pc34(void);

#endif
