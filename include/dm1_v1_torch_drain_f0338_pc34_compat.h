#ifndef FIRESTAFF_DM1_V1_TORCH_DRAIN_F0338_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_TORCH_DRAIN_F0338_PC34_COMPAT_H

#include <stdint.h>

#include "memory_dungeon_dat_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_V1_F0338_HAND_COUNT_PC34 = 8,
    DM1_V1_F0338_TORCH_WEAPON_TYPE_PC34 = 2
};

typedef struct {
    int valid;
    int changedCount;
    unsigned short drainedThings[DM1_V1_F0338_HAND_COUNT_PC34];
    uint32_t rawFingerprint;
    const char *sourceAnchor;
} DM1_V1_TorchDrainReceiptF0338Pc34;

/* `handThings` is ordered action, ready for each active champion. The call
 * validates every referenced raw C05 weapon before changing any charge byte. */
int dm1_v1_torch_drain_f0338_pc34(
    struct DungeonThings_Compat *things,
    const unsigned short handThings[DM1_V1_F0338_HAND_COUNT_PC34],
    int championCount,
    DM1_V1_TorchDrainReceiptF0338Pc34 *outReceipt);

#ifdef __cplusplus
}
#endif

#endif
