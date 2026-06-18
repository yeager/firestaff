#ifndef FIRESTAFF_DM1_V1_STEAL_FROM_SLOT_INDICES_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_STEAL_FROM_SLOT_INDICES_PC34_COMPAT_H

typedef struct DM1_V1_StealFromSlotIndicesResultPc34 {
    int accepted;
    int assertionCount;
    unsigned char tableEntries[8];
    int tableSize;
    int tableMatchesDeclaration;
    int counterModEightLoopCorrect;
    int allWithinSlotRange;
    int backpackSlotsUseRandom;
    int nonBackpackSlotsPassthrough;
    int initialCounterRandomInRange;
} DM1_V1_StealFromSlotIndicesResultPc34;

const unsigned char *
dm1_v1_steal_from_slot_indices_table_pc34(void);

int
dm1_v1_steal_from_slot_indices_size_pc34(void);

unsigned int
dm1_v1_steal_from_slot_indices_pc34(int counter);

int
dm1_v1_steal_from_slot_indices_is_backpack_pc34(unsigned int slot_index);

unsigned int
dm1_v1_steal_from_slot_indices_backpack_random_range_pc34(void);

unsigned int
dm1_v1_steal_from_slot_indices_counter_mod_mask_pc34(void);

int
dm1_v1_steal_from_slot_indices_run_pc34(
    DM1_V1_StealFromSlotIndicesResultPc34 *out);

#endif
