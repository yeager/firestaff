/* DM1 HoC C127 must never fill a non-PC34 middle party slot. */

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

static void add_valid_catalog_record(M11_GameViewState* state) {
    struct ChampionState_Compat champion;

    F0600_CHAMPION_InitEmpty_Compat(&champion);
    CHECK(F0606_CHAMPION_ParseMirrorTextIdentity_Compat(
              "HALK|THE BRAVE||M|AAGEAAHIAABJAAAA|AABOCACCCECGCIAAAA|"
              "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", &champion) == 1,
          "fixture mirror record parses");
    state->mirrorCatalogAvailable = 1;
    state->mirrorCatalog.count = 1;
    state->mirrorCatalog.records[0].textStringIndex = 7;
    state->mirrorCatalog.records[0].mirrorOrdinal = 3;
    state->mirrorCatalog.records[0].champion = champion;
    (void)F0628_CHAMPION_UnpackName_Compat(
        &champion, state->mirrorCatalog.records[0].nameText,
        sizeof(state->mirrorCatalog.records[0].nameText));
    (void)F0629_CHAMPION_UnpackTitle_Compat(
        &champion, state->mirrorCatalog.records[0].titleText,
        sizeof(state->mirrorCatalog.records[0].titleText));
}

int main(void) {
    M11_GameViewState state;

    memset(&state, 0, sizeof(state));
    M11_GameView_Init(&state);
    state.active = 1;
    state.world.party.activeChampionIndex = 0;
    state.world.party.championCount = 4;
    state.world.party.champions[0].present = 1;
    state.world.party.champions[2].present = 1;
    state.world.party.champions[3].present = 1;
    add_valid_catalog_record(&state);

    /* ReDMCSB REVIVE.C F0280:124-133 rejects G0305 == 4 before taking
     * M516[G0305]. F0280:272-277 appends at that count, never at a free
     * interior record. A missing middle M11 slot is malformed state and
     * must fail closed rather than turning C127 into a hole-filling route. */
    CHECK(M11_GameView_RecruitChampionByMirrorOrdinal(&state, 3) == 0,
          "C127 host recruit rejects full party with empty middle slot");
    CHECK(state.world.party.championCount == 4,
          "rejected C127 preserves source-owned party count");
    CHECK(state.world.party.activeChampionIndex == 0,
          "rejected C127 preserves existing leader");
    CHECK(state.world.party.champions[0].present == 1 &&
          state.world.party.champions[1].present == 0 &&
          state.world.party.champions[2].present == 1 &&
          state.world.party.champions[3].present == 1,
          "rejected C127 does not fill or reorder the middle slot");

    state.world.party.champions[1].present = 1;
    state.world.party.champions[1].hp.current = 0;
    state.world.party.champions[1].hp.maximum = 100;
    CHECK(M11_GameView_RecruitChampionByMirrorOrdinal(&state, 3) == 0,
          "dead but present middle champion still leaves original party full");
    CHECK(state.world.party.championCount == 4 &&
          state.world.party.activeChampionIndex == 0,
          "dead full-party rejection preserves count and leader");

    if (failures == 0) {
        puts("ok: DM1 HoC C127 rejects noncompact or full party state");
    }
    return failures == 0 ? 0 : 1;
}
