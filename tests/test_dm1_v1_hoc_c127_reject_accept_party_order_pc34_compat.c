/* DM1 HoC C127/C162/C160 candidate-slot reuse regression. */

#include "dm1_v1_resurrection_pc34_compat.h"
#include "m11_game_view.h"

#include <stdio.h>
#include <string.h>

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

static int failures;

#define CHECK(condition, message) do { \
    if (!(condition)) { \
        fprintf(stderr, "FAIL: %s\n", (message)); \
        ++failures; \
    } \
} while (0)

static CandidateChampionAddResult_Compat route_c127(uint16_t partyCount)
{
    ChampionPortraitClickInput_Compat in;

    memset(&in, 0, sizeof(in));
    in.command = DM1_COMMAND_CLICK_IN_DUNGEON_VIEW;
    in.leaderEmptyHanded = 1;
    in.leaderIndex = 0;
    in.frontWallOrnamentHit = 1;
    in.frontSquareInBounds = 1;
    in.sensorType = DM1_SENSOR_WALL_CHAMPION_PORTRAIT;
    in.sensorData = 7;
    in.sensorCell = 2;
    in.clickedWallCell = 2;
    in.partyChampionCount = partyCount;
    return F0866_RESURRECTION_RouteChampionPortraitClick_Compat(&in);
}

static CandidatePanelResult_Compat panel_command(
    const CandidateChampionAddResult_Compat* add, int command)
{
    CandidatePanelState_Compat state;

    state.partyChampionCount = add->nextPartyChampionCount;
    state.candidateChampionOrdinal = add->candidateChampionOrdinal;
    return F0867_RESURRECTION_ProcessCandidatePanelCommand_Compat(state, command);
}

int main(void)
{
    M11_GameViewState state;
    CandidateChampionAddResult_Compat rejectedAdd;
    CandidateChampionAddResult_Compat acceptedAdd;
    CandidatePanelResult_Compat rejected;
    CandidatePanelResult_Compat accepted;

    memset(&state, 0, sizeof(state));
    M11_GameView_Init(&state);
    state.active = 1;
    state.world.party.championCount = 1;
    state.world.party.activeChampionIndex = 0;
    state.world.party.champions[0].present = 1;

    /* ReDMCSB REVIVE.C F0280:272-283 appends the C127 candidate at G0305.
     * F0282:745-783 C162 clears G0299 and decrements G0305, so the next
     * C127 attempt must reuse the same last slot before C160 finalizes it. */
    rejectedAdd = route_c127(1);
    rejected = panel_command(&rejectedAdd, DM1_COMMAND_CANCEL);
    CHECK(rejectedAdd.triggersCandidateAdd == 1 &&
          rejectedAdd.candidateChampionIndex == 1 &&
          rejectedAdd.candidateChampionOrdinal == 2,
          "first C127 appends slot 1 / ordinal 2");
    CHECK(rejected.valid == 1 && rejected.cancelled == 1 &&
          rejected.nextPartyChampionCount == 1 &&
          rejected.nextCandidateChampionOrdinal == 0,
          "C162 restores G0305/G0299 to the pre-candidate state");

    state.world.party.championCount = (int)rejectedAdd.nextPartyChampionCount;
    state.world.party.champions[1].present = 1;
    state.candidateMirrorOrdinal = 7;
    state.candidateMirrorPartyIndex = rejectedAdd.candidateChampionIndex;
    state.candidateMirrorPanelActive = 1;
    state.inventoryPanelActive = 1;
    CHECK(M11_GameView_CancelMirrorCandidate(&state) == 1,
          "runtime C162 clears the live candidate");
    CHECK(state.world.party.championCount == 1 &&
          state.world.party.activeChampionIndex == 0 &&
          state.world.party.champions[0].present &&
          !state.world.party.champions[1].present,
          "C162 preserves leader and clears only appended slot 1");

    acceptedAdd = route_c127((uint16_t)state.world.party.championCount);
    accepted = panel_command(&acceptedAdd, DM1_COMMAND_RESURRECT);
    CHECK(acceptedAdd.triggersCandidateAdd == 1 &&
          acceptedAdd.candidateChampionIndex == 1 &&
          acceptedAdd.candidateChampionOrdinal == 2,
          "reselected C127 reuses slot 1 / ordinal 2");
    CHECK(accepted.valid == 1 && accepted.resurrected == 1 &&
          accepted.candidateChampionIndex == 1 &&
          accepted.nextPartyChampionCount == 2,
          "C160 finalizes the re-accepted candidate in slot 1");

    state.world.party.championCount = (int)accepted.nextPartyChampionCount;
    state.world.party.champions[1].present = 1;
    state.world.party.champions[1].hp.current = 100;
    state.world.party.champions[1].hp.maximum = 100;
    state.candidateMirrorOrdinal = 7;
    state.candidateMirrorPartyIndex = accepted.candidateChampionIndex;
    state.candidateMirrorPanelActive = 1;
    state.inventoryPanelActive = 1;
    CHECK(M11_GameView_ConfirmMirrorCandidate(&state, 0) == 1,
          "runtime C160 confirms the re-accepted candidate");
    CHECK(state.world.party.championCount == 2 &&
          state.world.party.activeChampionIndex == 0 &&
          state.world.party.champions[0].present &&
          state.world.party.champions[1].present,
          "C160 keeps leader 0 and append-only order after C162 reuse");
    CHECK(state.candidateMirrorPanelActive == 0 &&
          state.candidateMirrorOrdinal == -1 &&
          state.candidateMirrorPartyIndex == -1,
          "C160 clears the re-accepted candidate state");

    if (failures == 0) {
        puts("ok: DM1 HoC rejected C127 candidate reuses its slot and keeps leader");
    }
    return failures == 0 ? 0 : 1;
}
