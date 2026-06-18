#ifndef FIRESTAFF_DM1_V1_WOUND_PROBABILITY_INDEX_TO_MASK_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_WOUND_PROBABILITY_INDEX_TO_MASK_PC34_COMPAT_H

typedef struct DM1_V1_WoundProbabilityIndexToMaskResultPc34 {
    int accepted;
    int assertionCount;
    unsigned char tableEntries[4];
    int allUnique;
    int correctOrdering;
    int allMasksInDefs;
    int lookupBranchCorrect;
    int fallbackBranchCorrect;
    int lookupBranchGuardCorrect;
    int declarationMatchesInit;
} DM1_V1_WoundProbabilityIndexToMaskResultPc34;

const unsigned char *
dm1_v1_wound_probability_index_to_mask_table_pc34(void);

int
dm1_v1_wound_probability_index_to_mask_size_pc34(void);

unsigned int
dm1_v1_wound_probability_index_to_mask_pc34(int index);

int
dm1_v1_wound_probability_test_branch_pc34(unsigned int wound_test);

unsigned int
dm1_v1_wound_probability_test_mask_pc34(void);

unsigned int
dm1_v1_wound_probability_ready_hand_mask_pc34(void);

unsigned int
dm1_v1_wound_probability_index_count_pc34(void);

int
dm1_v1_wound_probability_index_to_mask_run_pc34(
    DM1_V1_WoundProbabilityIndexToMaskResultPc34 *out);

#endif
