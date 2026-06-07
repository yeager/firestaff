#include "dm1_v1_champion_mirror_click_closed_pc34_compat.h"

#include <stdio.h>

static int gTests;
static int gPasses;

#define CHECK_REDMCSB(cond, msg, anchor) do { \
    ++gTests; \
    if (cond) { \
        ++gPasses; \
    } else { \
        printf("FAIL: %s [%s]\n", msg, anchor); \
    } \
} while (0)

static void test_c040_closed_c159_click_switches_leader_and_keeps_mirror(void)
{
    Dm1V1MirrorClickClosedStatePc34Compat state;
    Dm1V1MirrorClickClosedResultPc34Compat result;
    int changed;

    DM1_V1_MirrorClickClosed_InitPc34Compat(&state);
    state.partyChampionCount = 4;
    state.candidateChampionOrdinal = 0u;
    state.inventoryChampionOrdinal = 2u;
    state.leaderIndex = 1;
    state.frontD1cMirrorChampionOrdinal = 1;
    state.champions[0].portraitOrdinal = 7;
    state.champions[1].portraitOrdinal = 11;

    changed = DM1_V1_MirrorClickClosed_ProcessStatusBoxClickPc34Compat(
        &state,
        DM1_V1_MIRROR_CLICK_CLOSED_STATUS_BOX_0_PC34_COMPAT,
        1,
        5,
        DM1_V1_MIRROR_CLICK_CLOSED_MOUSE_LEFT_PC34_COMPAT,
        &result);

    CHECK_REDMCSB(result.candidateChampionOrdinalBefore == 0u,
                  "test starts with C040 closed and no pending G0299 candidate",
                  "COMMAND.C:2158-2162");
    CHECK_REDMCSB(result.dispatchedStatusBoxClick == 1,
                  "C012 status-box owner dispatches while G0299 is clear",
                  "COMMAND.C:2158-2162");
    CHECK_REDMCSB(result.scannedChampionNameRows == 1,
                  "non-inventory status-box click scans G0455 name rows",
                  "CLIKCHAM.C:24-30; COMMAND.C:484-488");
    CHECK_REDMCSB(result.nestedCommand ==
                      DM1_V1_MIRROR_CLICK_CLOSED_SET_LEADER_0_PC34_COMPAT,
                  "x=1,y=5 in C159 maps to C016 set-leader champion 0",
                  "COMMAND.C:484-488");
    CHECK_REDMCSB(result.targetLeaderIndex == 0,
                  "C016 target is champion index 0 for F0368",
                  "CLIKCHAM.C:27-30");
    CHECK_REDMCSB(result.previousLeaderIndex == 1 &&
                      result.oldLeaderDetached == 1,
                  "F0368 detaches the previous G0411 leader before switching",
                  "CLIKCHAM.C:54-59");
    CHECK_REDMCSB(changed == 1 && result.leaderChanged == 1,
                  "F0368 accepts a live non-current target",
                  "CLIKCHAM.C:51-72; CHAMPION.C:1573-1574");
    CHECK_REDMCSB(state.leaderIndex == 0 && result.newLeaderIndex == 0,
                  "F0368 assigns champion 0 as the new G0411 leader",
                  "CLIKCHAM.C:66-72");
    CHECK_REDMCSB(result.frontD1cPortraitIndex == 7,
                  "front D1C mirror draws the new champion portrait ordinal",
                  "DUNGEON.C:2608-2612; DUNVIEW.C:3913-3928");
    CHECK_REDMCSB(state.candidateChampionOrdinal == 0u &&
                      result.candidateChampionOrdinalAfter == 0u,
                  "closed C040 pending state remains untouched after the click",
                  "COMMAND.C:2158-2162; CLIKCHAM.C:69-72");
}

int main(void)
{
    test_c040_closed_c159_click_switches_leader_and_keeps_mirror();

    CHECK_REDMCSB(
        DM1_V1_MirrorClickClosed_SourceEvidencePc34Compat() != NULL,
        "source evidence string is available",
        "COMMAND.C:484-488; COMMAND.C:2158-2162; CLIKCHAM.C:24-72");

    printf("PASS dm1_v1_champion_mirror_click_closed_pc34_compat %d/%d assertions\n",
           gPasses, gTests);
    return gPasses == gTests ? 0 : 1;
}
