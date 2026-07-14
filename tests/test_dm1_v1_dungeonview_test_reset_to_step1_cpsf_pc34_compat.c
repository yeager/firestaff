#include "firestaff/dm1/v1/dungeonview_test_reset_to_step1_cpsf_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int s_assertions;
static int s_failures;

#define CHECK(condition) do { \
    ++s_assertions; \
    if (!(condition)) { \
        ++s_failures; \
        fprintf(stderr, "%s:%d: %s\\n", __FILE__, __LINE__, #condition); \
    } \
} while (0)

static DM1_V1_DungeonviewTestResetToStep1CpsfStatePc34Compat fixture(void)
{
    DM1_V1_DungeonviewTestResetToStep1CpsfStatePc34Compat state;

    memset(&state, 0, sizeof(state));
    state.stateCpsf = 11u;
    state.sectorsReadFailureCpsdf = 0x4000u;
    state.sectorsReadPreviousFailureCpsdf = 0x4001u;
    return state;
}

int main(void)
{
    DM1_V1_DungeonviewTestResetToStep1CpsfStatePc34Compat state;
    DM1_V1_DungeonviewTestResetToStep1CpsfStatePc34Compat before;

    CHECK(strstr(dm1_v1_dungeonview_test_reset_to_step1_cpsf_source_evidence_pc34(),
                 "DUNVIEW.C F0106_DUNGEONVIEW_TestResetToStep1_CPSF:3478-3498") != NULL);

    state = fixture();
    F0106_DUNGEONVIEW_TestResetToStep1_CPSF_Compat(
        &state, DM1_V1_DUNGEONVIEW_CPSF_GRACE_RETRY_LIMIT_EARLY_PC34);
    CHECK(state.sectorsReadFailureCpsdf == 0x4001u);
    CHECK(state.graceReadRetryCountCpsf == 1u);
    CHECK(state.sectorsReadPreviousFailureCpsdf == 0x4000u);
    CHECK(state.stateCpsf == DM1_V1_DUNGEONVIEW_CPSF_STATE_STEP1_PC34);
    CHECK(state.fuzzyBitFoundCpsf == 1);

    state = fixture();
    state.floppyDriveDmaTimeoutCpsdf = 1;
    F0106_DUNGEONVIEW_TestResetToStep1_CPSF_Compat(
        &state, DM1_V1_DUNGEONVIEW_CPSF_GRACE_RETRY_LIMIT_EARLY_PC34);
    CHECK(state.sectorsReadFailureCpsdf == 0x4000u);
    CHECK(state.graceReadRetryCountCpsf == 0u);
    CHECK(state.sectorsReadPreviousFailureCpsdf == 0x4000u &&
          state.stateCpsf == DM1_V1_DUNGEONVIEW_CPSF_STATE_STEP1_PC34 &&
          state.fuzzyBitFoundCpsf == 1);

    state = fixture();
    state.sectorsReadFailureCpsdf |=
        DM1_V1_DUNGEONVIEW_CPS_FUZZY_SECTOR_MASK_PC34;
    F0106_DUNGEONVIEW_TestResetToStep1_CPSF_Compat(
        &state, DM1_V1_DUNGEONVIEW_CPSF_GRACE_RETRY_LIMIT_EARLY_PC34);
    CHECK(state.sectorsReadFailureCpsdf == 0x4001u &&
          state.graceReadRetryCountCpsf == 0u);
    CHECK(state.sectorsReadPreviousFailureCpsdf == 0x4000u &&
          state.stateCpsf == DM1_V1_DUNGEONVIEW_CPSF_STATE_STEP1_PC34 &&
          state.fuzzyBitFoundCpsf == 1);

    state = fixture();
    state.graceReadRetryCountCpsf =
        DM1_V1_DUNGEONVIEW_CPSF_GRACE_RETRY_LIMIT_EARLY_PC34;
    before = state;
    F0106_DUNGEONVIEW_TestResetToStep1_CPSF_Compat(
        &state, DM1_V1_DUNGEONVIEW_CPSF_GRACE_RETRY_LIMIT_EARLY_PC34);
    CHECK(memcmp(&state, &before, sizeof(state)) == 0);

    state = fixture();
    state.fuzzyBitFoundCpsf = 1;
    before = state;
    F0106_DUNGEONVIEW_TestResetToStep1_CPSF_Compat(
        &state, DM1_V1_DUNGEONVIEW_CPSF_GRACE_RETRY_LIMIT_LATE_PC34);
    CHECK(memcmp(&state, &before, sizeof(state)) == 0);

    state = fixture();
    state.graceReadRetryCountCpsf = 5u;
    F0106_DUNGEONVIEW_TestResetToStep1_CPSF_Compat(
        &state, DM1_V1_DUNGEONVIEW_CPSF_GRACE_RETRY_LIMIT_LATE_PC34);
    CHECK(state.graceReadRetryCountCpsf == 6u &&
          state.sectorsReadFailureCpsdf == 0x4001u &&
          state.stateCpsf == DM1_V1_DUNGEONVIEW_CPSF_STATE_STEP1_PC34 &&
          state.fuzzyBitFoundCpsf == 1);

    F0106_DUNGEONVIEW_TestResetToStep1_CPSF_Compat(
        NULL, DM1_V1_DUNGEONVIEW_CPSF_GRACE_RETRY_LIMIT_EARLY_PC34);
    CHECK(s_failures == 0);

    printf("test_dm1_v1_dungeonview_test_reset_to_step1_cpsf_pc34_compat: "
           "%d assertions, %d failures\\n", s_assertions, s_failures);
    return s_failures == 0 ? 0 : 1;
}
