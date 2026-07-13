/* DM1 HoC C127/C160 multi-confirmation party-order regression. */

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

static CandidateChampionAddResult_Compat route_c127_candidate(
    uint16_t partyChampionCount, uint16_t portraitOrdinal)
{
    ChampionPortraitClickInput_Compat in;

    memset(&in, 0, sizeof(in));
    in.command = DM1_COMMAND_CLICK_IN_DUNGEON_VIEW;
    in.leaderEmptyHanded = 1;
    in.leaderIndex = DM1_CHAMPION_NONE;
    in.frontWallOrnamentHit = 1;
    in.frontSquareInBounds = 1;
    in.sensorType = DM1_SENSOR_WALL_CHAMPION_PORTRAIT;
    in.sensorData = portraitOrdinal;
    in.sensorCell = 2;
    in.clickedWallCell = 2;
    in.partyChampionCount = partyChampionCount;
    return F0866_RESURRECTION_RouteChampionPortraitClick_Compat(&in);
}

static CandidatePanelResult_Compat finalize_c160(
    const CandidateChampionAddResult_Compat* add)
{
    CandidatePanelState_Compat panel;

    panel.partyChampionCount = add->nextPartyChampionCount;
    panel.candidateChampionOrdinal = add->candidateChampionOrdinal;
    return F0867_RESURRECTION_ProcessCandidatePanelCommand_Compat(
        panel, DM1_COMMAND_RESURRECT);
}

int main(void)
{
    M11_GameViewState state;
    CandidateChampionAddResult_Compat first;
    CandidateChampionAddResult_Compat second;
    CandidatePanelResult_Compat firstConfirm;
    CandidatePanelResult_Compat secondConfirm;

    memset(&state, 0, sizeof(state));
    M11_GameView_Init(&state);
    state.active = 1;
    state.world.party.championCount = 1;
    state.world.party.activeChampionIndex = 0;
    state.world.party.champions[0].present = 1;

    /* ReDMCSB REVIVE.C F0280:272-283 appends C127 candidates at G0305;
     * F0282:744-845 finalizes C160 at G0305-1 without reseating leader. */
    first = route_c127_candidate(1, 4);
    firstConfirm = finalize_c160(&first);
    CHECK(first.triggersCandidateAdd == 1, "first C127 route appends a candidate");
    CHECK(first.candidateChampionIndex == 1 &&
          first.candidateChampionOrdinal == 2,
          "first C127 candidate occupies party slot 1 / ordinal 2");
    CHECK(firstConfirm.valid == 1 && firstConfirm.candidateChampionIndex == 1 &&
          firstConfirm.nextPartyChampionCount == 2,
          "first C160 finalizes the appended slot without changing party count");

    state.world.party.championCount = (int)firstConfirm.nextPartyChampionCount;
    state.world.party.champions[1].present = 1;
    state.world.party.champions[1].hp.current = 100;
    state.world.party.champions[1].hp.maximum = 100;
    state.candidateMirrorOrdinal = 4;
    state.candidateMirrorPartyIndex = firstConfirm.candidateChampionIndex;
    state.candidateMirrorPanelActive = 1;
    state.inventoryPanelActive = 1;
    CHECK(M11_GameView_ConfirmMirrorCandidate(&state, 0) == 1,
          "first runtime C160 confirmation succeeds");
    CHECK(state.world.party.activeChampionIndex == 0,
          "first later-mirror C160 preserves the original leader");
    CHECK(state.world.party.championCount == 2 &&
          state.world.party.champions[0].present && state.world.party.champions[1].present,
          "first C160 retains party slot order");

    second = route_c127_candidate(2, 9);
    secondConfirm = finalize_c160(&second);
    CHECK(second.triggersCandidateAdd == 1, "second C127 route appends a candidate");
    CHECK(second.candidateChampionIndex == 2 &&
          second.candidateChampionOrdinal == 3,
          "second C127 candidate occupies party slot 2 / ordinal 3");
    CHECK(secondConfirm.valid == 1 && secondConfirm.candidateChampionIndex == 2 &&
          secondConfirm.nextPartyChampionCount == 3,
          "second C160 finalizes the next appended slot");

    state.world.party.championCount = (int)secondConfirm.nextPartyChampionCount;
    state.world.party.champions[2].present = 1;
    state.world.party.champions[2].hp.current = 100;
    state.world.party.champions[2].hp.maximum = 100;
    state.candidateMirrorOrdinal = 9;
    state.candidateMirrorPartyIndex = secondConfirm.candidateChampionIndex;
    state.candidateMirrorPanelActive = 1;
    state.inventoryPanelActive = 1;
    CHECK(M11_GameView_ConfirmMirrorCandidate(&state, 0) == 1,
          "second runtime C160 confirmation succeeds");
    CHECK(state.world.party.activeChampionIndex == 0,
          "second later-mirror C160 preserves the original leader");
    CHECK(state.world.party.championCount == 3 &&
          state.world.party.champions[0].present &&
          state.world.party.champions[1].present &&
          state.world.party.champions[2].present,
          "two C127/C160 confirmations preserve append-only party order");
    CHECK(state.candidateMirrorPanelActive == 0 &&
          state.candidateMirrorOrdinal == -1 &&
          state.candidateMirrorPartyIndex == -1,
          "second C160 clears only the live candidate state");

    if (failures == 0) {
        puts("ok: DM1 HoC C127/C160 confirmations retain leader and party order");
    }
    return failures == 0 ? 0 : 1;
}
