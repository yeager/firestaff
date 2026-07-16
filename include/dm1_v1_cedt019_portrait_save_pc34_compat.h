#ifndef FIRESTAFF_DM1_V1_CEDT019_PORTRAIT_SAVE_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CEDT019_PORTRAIT_SAVE_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#include "dm1_v1_portrait_panel_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_V1_CEDT019_PORTRAIT_COUNT_PC34 = DM1_MAX_CHAMPIONS,
    DM1_V1_CEDT019_PORTRAIT_PLANAR_BYTES_PC34 =
        DM1_PORTRAIT_PLANAR_BYTES,
    DM1_V1_CEDT019_PORTRAIT_CHUNKY_BYTES_PC34 =
        DM1_PORTRAIT_CHUNKY_BYTES
};

typedef struct {
    int present;
    int portraitBytesProven;
    const uint8_t *planarSource;
    size_t planarSourceSize;
    uint8_t *planarDestination;
    size_t planarDestinationSize;
    const uint8_t *chunkySource;
    size_t chunkySourceSize;
    uint8_t *chunkyDestination;
    size_t chunkyDestinationSize;
} DM1_V1_CEDT019_PortraitSlotPc34;

typedef struct {
    int valid;
    int convertedPortraitCount;
    int rejectedPortraitCount;
    int requiredPortraitCount;
    int requiredPlanarBytes;
    int requiredChunkyBytes;
    int sourceLineStart;
    int sourceLineEnd;
} DM1_V1_CEDT019_PortraitBatchReceiptPc34;

int F2122_DecodeAllPortraitsWhileLoading(
    DM1_V1_CEDT019_PortraitSlotPc34 *portraits,
    size_t portraitCount,
    DM1_V1_CEDT019_PortraitBatchReceiptPc34 *out);

int F2123_EncodeAllPortraitsBeforeSaving(
    DM1_V1_CEDT019_PortraitSlotPc34 *portraits,
    size_t portraitCount,
    DM1_V1_CEDT019_PortraitBatchReceiptPc34 *out);

int F2124_DecodeAllPortraitsAfterSaving(
    DM1_V1_CEDT019_PortraitSlotPc34 *portraits,
    size_t portraitCount,
    DM1_V1_CEDT019_PortraitBatchReceiptPc34 *out);

const char *F2122_F2123_F2124_CEDT019_SourceEvidencePc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_CEDT019_PORTRAIT_SAVE_PC34_COMPAT_H */
