#ifndef FIRESTAFF_DM1_V1_DUNGEON_LIGHT_ADMISSION_F0337_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_DUNGEON_LIGHT_ADMISSION_F0337_PC34_COMPAT_H

#include <stdint.h>

#include "memory_dungeon_dat_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

enum { DM1_V1_F0337_HAND_COUNT_PC34 = 8 };

typedef struct {
    int valid;
    int torchLightPower[DM1_V1_F0337_HAND_COUNT_PC34];
    uint32_t rawFingerprint;
    const char *sourceAnchor;
} DM1_V1_DungeonLightReceiptF0337Pc34;

/* `handThings` is action, ready for each of the four PC34 champion records.
 * The result is calculation-only: it never selects or renders a palette. */
int dm1_v1_dungeon_light_admit_f0337_pc34(
    const struct DungeonThings_Compat *things,
    const unsigned short handThings[DM1_V1_F0337_HAND_COUNT_PC34],
    DM1_V1_DungeonLightReceiptF0337Pc34 *outReceipt);

#ifdef __cplusplus
}
#endif

#endif
