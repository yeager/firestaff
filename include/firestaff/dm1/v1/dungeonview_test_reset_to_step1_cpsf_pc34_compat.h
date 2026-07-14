#ifndef FIRESTAFF_DM1_V1_DUNGEONVIEW_TEST_RESET_TO_STEP1_CPSF_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_DUNGEONVIEW_TEST_RESET_TO_STEP1_CPSF_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_V1_DUNGEONVIEW_CPS_FUZZY_SECTOR_MASK_PC34 = 0x0001u,
    DM1_V1_DUNGEONVIEW_CPSF_STATE_STEP1_PC34 = 15u,
    DM1_V1_DUNGEONVIEW_CPSF_GRACE_RETRY_LIMIT_EARLY_PC34 = 5u,
    DM1_V1_DUNGEONVIEW_CPSF_GRACE_RETRY_LIMIT_LATE_PC34 = 6u
};

/* The source globals read and written by F0106. */
typedef struct DM1_V1_DungeonviewTestResetToStep1CpsfStatePc34Compat {
    uint16_t stateCpsf;
    uint16_t graceReadRetryCountCpsf;
    uint16_t sectorsReadFailureCpsdf;
    uint16_t sectorsReadPreviousFailureCpsdf;
    int16_t floppyDriveDmaTimeoutCpsdf;
    int fuzzyBitFoundCpsf;
} DM1_V1_DungeonviewTestResetToStep1CpsfStatePc34Compat;

/*
 * ReDMCSB DUNVIEW.C F0106:3485-3496.  The source selects the retry limit at
 * build time: MEDIA007 uses 5 and MEDIA423 uses 6.  Callers provide that
 * source-selected limit so this standalone transition preserves both bodies.
 */
void F0106_DUNGEONVIEW_TestResetToStep1_CPSF_Compat(
    DM1_V1_DungeonviewTestResetToStep1CpsfStatePc34Compat *state,
    uint16_t graceReadRetryLimit);

const char *dm1_v1_dungeonview_test_reset_to_step1_cpsf_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
