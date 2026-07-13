/* DM1 HoC C160 party-leader regression, source-locked to REVIVE.C F0282. */

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

int main(void)
{
    M11_GameViewState state;
    ChampionPortraitClickInput_Compat click;
    CandidateChampionAddResult_Compat add;

    memset(&state, 0, sizeof(state));
    M11_GameView_Init(&state);
    state.active = 1;
    state.world.party.championCount = 2;
    state.world.party.activeChampionIndex = 0;
    state.world.party.champions[0].present = 1;
    state.world.party.champions[0].hp.current = 100;
    state.world.party.champions[0].hp.maximum = 100;
    state.world.party.champions[1].present = 1;
    state.world.party.champions[1].hp.current = 100;
    state.world.party.champions[1].hp.maximum = 100;
    state.candidateMirrorOrdinal = 1;
    state.candidateMirrorPartyIndex = 1;
    state.candidateMirrorPanelActive = 1;
    state.inventoryPanelActive = 1;

    /* ReDMCSB REVIVE.C F0282:837-845 calls SetLeader(C00) only when
     * G0305 == 1. C160 for a later mirror leaves the current leader intact. */
    CHECK(M11_GameView_ConfirmMirrorCandidate(&state, 0) == 1,
          "C160 confirms a live appended mirror candidate");
    CHECK(state.world.party.activeChampionIndex == 0,
          "later C160 preserves the existing party leader");
    CHECK(state.candidateMirrorPanelActive == 0,
          "C160 closes the candidate panel");
    CHECK(state.candidateMirrorPartyIndex == -1,
          "C160 clears the candidate party index");

    memset(&state, 0, sizeof(state));
    M11_GameView_Init(&state);
    state.active = 1;
    state.world.party.championCount = 3;
    state.world.party.activeChampionIndex = 1;
    state.world.party.champions[0].present = 1;
    state.world.party.champions[0].hp.current = 80;
    state.world.party.champions[0].hp.maximum = 80;
    state.world.party.champions[1].present = 1;
    state.world.party.champions[1].hp.current = 0;
    state.world.party.champions[1].hp.maximum = 100;
    state.world.party.champions[2].present = 1;
    state.world.party.champions[2].hp.current = 100;
    state.world.party.champions[2].hp.maximum = 100;
    state.candidateMirrorOrdinal = 4;
    state.candidateMirrorPartyIndex = 2;
    state.candidateMirrorPanelActive = 1;
    state.inventoryPanelActive = 1;

    /* ReDMCSB REVIVE.C F0280 appends the C127 candidate at the tail while
     * CHAMPION.C F0319:1662-1681 reassigns a dead leader to the first living
     * G0305 slot. The C160 finish must repair a stale restored leader index
     * without reordering either existing champion or the new candidate. */
    memset(&click, 0, sizeof(click));
    click.command = DM1_COMMAND_CLICK_IN_DUNGEON_VIEW;
    click.leaderEmptyHanded = 1;
    click.leaderIndex = 1;
    click.frontWallOrnamentHit = 1;
    click.frontSquareInBounds = 1;
    click.sensorType = DM1_SENSOR_WALL_CHAMPION_PORTRAIT;
    click.sensorData = 4;
    click.sensorCell = 2;
    click.clickedWallCell = 2;
    click.partyChampionCount = 2;
    add = F0866_RESURRECTION_RouteChampionPortraitClick_Compat(&click);
    CHECK(add.triggersCandidateAdd == 1 && add.candidateChampionIndex == 2 &&
          add.candidateChampionOrdinal == 3,
          "C127 still appends after a compact two-champion roster");
    CHECK(M11_GameView_ConfirmMirrorCandidate(&state, 0) == 1,
          "C160 confirms after stale dead-leader restoration");
    CHECK(state.world.party.activeChampionIndex == 0,
          "C160 repairs dead leader to first living source-order champion");
    CHECK(state.world.party.championCount == 3 &&
          state.world.party.champions[0].present &&
          state.world.party.champions[1].present &&
          state.world.party.champions[2].present,
          "C160 leader repair keeps compact party order intact");

    if (failures == 0) {
        puts("ok: DM1 HoC C160 preserves the leader for later mirror candidates");
    }
    return failures == 0 ? 0 : 1;
}
