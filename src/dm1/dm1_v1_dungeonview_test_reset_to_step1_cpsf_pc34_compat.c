#include "firestaff/dm1/v1/dungeonview_test_reset_to_step1_cpsf_pc34_compat.h"

static const char s_source_evidence[] =
    "ReDMCSB WIP20210206 DUNVIEW.C "
    "F0106_DUNGEONVIEW_TestResetToStep1_CPSF:3478-3498; "
    "MEDIA007:3485 uses retry limit 5; MEDIA423:3488 uses retry limit 6; "
    "G0315/G0316 MASK0x0001_CPS_FUZZY_SECTOR; G0069=15; G0078=C1_TRUE.";

void F0106_DUNGEONVIEW_TestResetToStep1_CPSF_Compat(
    DM1_V1_DungeonviewTestResetToStep1CpsfStatePc34Compat *state,
    uint16_t graceReadRetryLimit)
{
    if (!state ||
        state->graceReadRetryCountCpsf >= graceReadRetryLimit ||
        state->fuzzyBitFoundCpsf) {
        return;
    }

    if (!state->floppyDriveDmaTimeoutCpsdf &&
        !(state->sectorsReadFailureCpsdf &
          DM1_V1_DUNGEONVIEW_CPS_FUZZY_SECTOR_MASK_PC34)) {
        state->sectorsReadFailureCpsdf |=
            DM1_V1_DUNGEONVIEW_CPS_FUZZY_SECTOR_MASK_PC34;
        ++state->graceReadRetryCountCpsf;
    }

    state->sectorsReadPreviousFailureCpsdf &=
        (uint16_t)~DM1_V1_DUNGEONVIEW_CPS_FUZZY_SECTOR_MASK_PC34;
    state->stateCpsf = DM1_V1_DUNGEONVIEW_CPSF_STATE_STEP1_PC34;
    state->fuzzyBitFoundCpsf = 1;
}

const char *dm1_v1_dungeonview_test_reset_to_step1_cpsf_source_evidence_pc34(void)
{
    return s_source_evidence;
}
