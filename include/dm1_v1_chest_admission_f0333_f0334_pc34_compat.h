#ifndef FIRESTAFF_DM1_V1_CHEST_ADMISSION_F0333_F0334_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHEST_ADMISSION_F0333_F0334_PC34_COMPAT_H

#include <stdint.h>

#include "memory_dungeon_dat_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

enum { DM1_CHEST_VISIBLE_SLOT_COUNT_F0333_F0334_PC34 = 8 };

typedef struct {
    int valid;
    unsigned short containerThing;
    unsigned short slots[DM1_CHEST_VISIBLE_SLOT_COUNT_F0333_F0334_PC34];
    int slotCount;
    uint32_t rawContainerFNV1a;
    uint32_t rawChainFNV1a;
    const char *sourceAnchor;
} DM1_ChestAdmissionReceiptF0333F0334Pc34;

int dm1_v1_chest_open_admit_f0333_pc34(
    const struct DungeonThings_Compat *things,
    unsigned short containerThing,
    DM1_ChestAdmissionReceiptF0333F0334Pc34 *outReceipt);

int dm1_v1_chest_close_admit_f0334_pc34(
    const struct DungeonThings_Compat *things,
    unsigned short containerThing,
    const unsigned short slots[DM1_CHEST_VISIBLE_SLOT_COUNT_F0333_F0334_PC34],
    DM1_ChestAdmissionReceiptF0333F0334Pc34 *outReceipt);

/* Commits only a previously admissible C537..C544 visible chest list.  Both
 * the decoded mirrors and source PC34 next/slot bytes change together. */
int dm1_v1_chest_close_commit_f0334_pc34(
    struct DungeonThings_Compat *things,
    unsigned short containerThing,
    const unsigned short slots[DM1_CHEST_VISIBLE_SLOT_COUNT_F0333_F0334_PC34],
    DM1_ChestAdmissionReceiptF0333F0334Pc34 *outReceipt);

#ifdef __cplusplus
}
#endif

#endif
