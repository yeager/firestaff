/*
 * test_dm1_v1_mov05_f0284_cell_rotation_pc34_compat.c
 *
 * Source-locked to ReDMCSB CHAMPION.C:117-130,
 * F0284_CHAMPION_SetPartyDirection.
 *
 * MOV-05 (DM1 V1 functional-divergence-report.md):
 *   "F0284 rotates Direction but not Cell.  Cell rotation affects
 *    display ordering in the inventory panel; the inventory panel
 *    may not refresh correctly when the party turns direction
 *    while a candidate is present."
 *
 * ReDMCSB F0284 rotates each party champion's Cell and Direction
 * by the direction delta. It does not reorder M516_CHAMPIONS or
 * change the selected leader index.
 */

#include "memory_mov05_f0284_cell_rotation_pc34_compat.h"
#include "memory_champion_state_pc34_compat.h"

#include <stdio.h>
#include <string.h>

#define CHECK(cond, msg) do { \
    if (!(cond)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, msg); \
        return 1; \
    } \
} while (0)

static void setup_4_champions(struct PartyState_Compat* party)
{
    int i;
    memset(party, 0, sizeof(*party));
    for (i = 0; i < 4; ++i) {
        party->champions[i].present = 1;
        party->champions[i].portraitIndex = 100 + i;
        party->champions[i].cell = (unsigned char)i;
        party->champions[i].direction = 0;
    }
    party->championCount = 4;
    party->direction = 0;
    party->activeChampionIndex = 0;
}

static void setup_sparse_3_champions(struct PartyState_Compat* party)
{
    memset(party, 0, sizeof(*party));
    party->champions[0].present = 1;
    party->champions[0].portraitIndex = 300;
    party->champions[0].cell = 0;
    party->champions[0].direction = 1;

    party->champions[1].present = 0;

    party->champions[2].present = 1;
    party->champions[2].portraitIndex = 302;
    party->champions[2].cell = 2;
    party->champions[2].direction = 2;

    party->champions[3].present = 1;
    party->champions[3].portraitIndex = 303;
    party->champions[3].cell = 3;
    party->champions[3].direction = 3;

    party->championCount = 3;
    party->direction = 0;
    party->activeChampionIndex = 2;
}

int main(void)
{
    struct PartyState_Compat party;
    int r;
    int i;

    /* No-op direction change is idempotent. */
    setup_4_champions(&party);
    r = F0284_CHAMPION_SetPartyDirection_Compat(&party, 0);
    CHECK(r == 0, "no-op turn returns 0");
    CHECK(party.direction == 0, "party direction unchanged by no-op");
    for (i = 0; i < 4; ++i) {
        CHECK(party.champions[i].portraitIndex == 100 + i,
              "no-op preserves champion slot identity");
        CHECK(party.champions[i].cell == (unsigned char)i,
              "no-op preserves champion Cell");
        CHECK(party.champions[i].direction == 0,
              "no-op preserves champion Direction");
    }
    CHECK(party.activeChampionIndex == 0,
          "no-op preserves activeChampionIndex");

    /* NORTH -> EAST: delta=+1.  F0284 updates per-champion Cell and
     * Direction and leaves the champion array order untouched. */
    setup_4_champions(&party);
    r = F0284_CHAMPION_SetPartyDirection_Compat(&party, 1);
    CHECK(r == 1, "NORTH->EAST turn returns 1");
    CHECK(party.direction == 1, "party direction advances to EAST");
    for (i = 0; i < 4; ++i) {
        CHECK(party.champions[i].portraitIndex == 100 + i,
              "right turn preserves champion slot identity");
        CHECK(party.champions[i].cell == (unsigned char)((i + 1) & 3),
              "right turn rotates Cell by +1 mod 4");
        CHECK(party.champions[i].direction == 1,
              "right turn rotates Direction by +1 mod 4");
    }
    CHECK(party.activeChampionIndex == 0,
          "right turn preserves activeChampionIndex");

    /* EAST -> WEST: delta=+2. */
    r = F0284_CHAMPION_SetPartyDirection_Compat(&party, 3);
    CHECK(r == 1, "EAST->WEST turn returns 1");
    CHECK(party.direction == 3, "party direction advances to WEST");
    for (i = 0; i < 4; ++i) {
        CHECK(party.champions[i].portraitIndex == 100 + i,
              "delta=2 preserves champion slot identity");
        CHECK(party.champions[i].cell == (unsigned char)((i + 3) & 3),
              "delta=2 rotates Cell by +2 on top of prior +1");
        CHECK(party.champions[i].direction == 3,
              "delta=2 rotates Direction by +2 on top of prior +1");
    }
    CHECK(party.activeChampionIndex == 0,
          "delta=2 preserves activeChampionIndex");

    /* WEST -> SOUTH: target < current, so F0284 takes the delta += 4
     * branch and applies delta=+3. */
    r = F0284_CHAMPION_SetPartyDirection_Compat(&party, 2);
    CHECK(r == 1, "WEST->SOUTH turn returns 1");
    CHECK(party.direction == 2, "party direction advances to SOUTH");
    for (i = 0; i < 4; ++i) {
        CHECK(party.champions[i].portraitIndex == 100 + i,
              "wrapped delta preserves champion slot identity");
        CHECK(party.champions[i].cell == (unsigned char)((i + 2) & 3),
              "wrapped delta rotates Cell by +3 on top of prior +3");
        CHECK(party.champions[i].direction == 2,
              "wrapped delta rotates Direction by +3 on top of prior +3");
    }

    /* Sparse party: empty array slots stay empty because source F0284
     * does not move champion structs.  The occupied champions still
     * rotate their own Cell and Direction. */
    setup_sparse_3_champions(&party);
    r = F0284_CHAMPION_SetPartyDirection_Compat(&party, 1);
    CHECK(r == 1, "sparse NORTH->EAST turn returns 1");
    CHECK(party.champions[1].present == 0,
          "sparse party keeps empty array slot empty");
    CHECK(party.champions[0].portraitIndex == 300,
          "sparse party preserves slot 0 champion identity");
    CHECK(party.champions[2].portraitIndex == 302,
          "sparse party preserves slot 2 champion identity");
    CHECK(party.champions[3].portraitIndex == 303,
          "sparse party preserves slot 3 champion identity");
    CHECK(party.champions[0].cell == 1 &&
              party.champions[2].cell == 3 &&
              party.champions[3].cell == 0,
          "sparse party rotates present champion Cells by +1 mod 4");
    CHECK(party.champions[0].direction == 2 &&
              party.champions[2].direction == 3 &&
              party.champions[3].direction == 0,
          "sparse party rotates present champion Directions by +1 mod 4");
    CHECK(party.activeChampionIndex == 2,
          "sparse party preserves activeChampionIndex");

    printf("PASS: MOV-05 F0284 source-locked Cell/Direction rotation\n");
    return 0;
}
