/* DM1 HoC C160 party-leader regression, source-locked to REVIVE.C F0282. */

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

    memset(&state, 0, sizeof(state));
    M11_GameView_Init(&state);
    state.active = 1;
    state.world.party.championCount = 2;
    state.world.party.activeChampionIndex = 0;
    state.world.party.champions[0].present = 1;
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

    if (failures == 0) {
        puts("ok: DM1 HoC C160 preserves the leader for later mirror candidates");
    }
    return failures == 0 ? 0 : 1;
}
