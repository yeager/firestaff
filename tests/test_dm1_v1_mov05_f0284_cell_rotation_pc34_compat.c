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
 * Pins the F0284 invariants:
 *  T1  No-op direction change is idempotent.
 *  T2  Per-present-cell Direction += delta (mod 4).
 *  T3  Present-list Cell rotation: champion at rank k moves to
 *      rank (k + delta) mod nPresent, with empty slots preserved.
 *  T4  activeChampionIndex follows the previously-active champion.
 *  T5  4-champion contiguous party: single right turn (delta=1)
 *      rotates to (champ 1, champ 2, champ 3, champ 0).
 *  T6  4-champion party: 3 right turns == 1 left turn (delta=1).
 *  T7  1-champion party: rotation is a no-op on cell order.
 *  T8  3-champion party with empty slot at index 0: rotation
 *      preserves the empty slot at index 0.
 *  T9  2-champion party: rotation swaps the two cell occupants.
 *  T10 Non-contiguous 3-of-4: empty slot stays empty.
 *  T11 All four cardinal directions reachable.
 *  T12 portraitIndex preserved through rotation (cell rotation
 *      doesn't change the champion's identity).
 *  T13 delta=2 (right+right): each champion moves 2 slots.
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

#define DIR_NORTH 0
#define DIR_EAST  1
#define DIR_SOUTH 2
#define DIR_WEST  3

static void setup_4_champions(struct PartyState_Compat* party) {
    int i;
    memset(party, 0, sizeof(*party));
    for (i = 0; i < 4; ++i) {
        party->champions[i].present = 1;
        party->champions[i].portraitIndex = 100 + i;
        party->champions[i].direction = DIR_NORTH;
    }
    party->championCount = 4;
    party->direction = DIR_NORTH;
    party->activeChampionIndex = 0;
}

static void setup_1_champion(struct PartyState_Compat* party) {
    memset(party, 0, sizeof(*party));
    party->champions[0].present = 1;
    party->champions[0].portraitIndex = 100;
    party->champions[0].direction = DIR_NORTH;
    party->championCount = 1;
    party->direction = DIR_NORTH;
    party->activeChampionIndex = 0;
}

int main(void) {
    struct PartyState_Compat party;
    int r;

    /* T1: No-op direction change is idempotent. */
    setup_4_champions(&party);
    r = F0284_CHAMPION_SetPartyDirection_Compat(&party, DIR_NORTH);
    CHECK(r == 0, "T1: no-op turn returns 0");
    CHECK(party.direction == DIR_NORTH, "T1: direction unchanged");

    /* T2: Per-present-cell Direction += delta (mod 4). */
    setup_4_champions(&party);
    /* Pre-set all directions to NORTH, then turn right (1). */
    F0284_CHAMPION_SetPartyDirection_Compat(&party, DIR_EAST);
    CHECK(party.champions[0].direction == DIR_EAST, "T2: champ[0] dir +1");
    CHECK(party.champions[3].direction == DIR_EAST, "T2: champ[3] dir +1");
    F0284_CHAMPION_SetPartyDirection_Compat(&party, DIR_SOUTH);
    CHECK(party.champions[0].direction == DIR_SOUTH, "T2: champ[0] dir +2");
    F0284_CHAMPION_SetPartyDirection_Compat(&party, DIR_WEST);
    CHECK(party.champions[0].direction == DIR_WEST, "T2: champ[0] dir +3");
    F0284_CHAMPION_SetPartyDirection_Compat(&party, DIR_NORTH);
    CHECK(party.champions[0].direction == DIR_NORTH, "T2: champ[0] dir wraps to 0");

    /* T3 + T5: 4-champion party, single right turn.
     * Champions at rank 0..3 move to rank 1..3,0 respectively.
     * Cell order: was [0,1,2,3] -> [3,0,1,2]. */
    setup_4_champions(&party);
    F0284_CHAMPION_SetPartyDirection_Compat(&party, DIR_EAST);
    /* Active index 0 (was at rank 0) now at rank 1, which is
     * array index of the champion that was at rank 1 (portrait 101). */
    CHECK(party.champions[1].portraitIndex == 100, "T5: rank-0 champ -> index 1");
    CHECK(party.champions[2].portraitIndex == 101, "T5: rank-1 champ -> index 2");
    CHECK(party.champions[3].portraitIndex == 102, "T5: rank-2 champ -> index 3");
    CHECK(party.champions[0].portraitIndex == 103, "T5: rank-3 champ -> index 0");
    CHECK(party.activeChampionIndex == 1, "T5: active moved from 0 to 1");

    /* T6: 3 right turns == 1 left turn (delta=1). */
    setup_4_champions(&party);
    F0284_CHAMPION_SetPartyDirection_Compat(&party, DIR_EAST);
    F0284_CHAMPION_SetPartyDirection_Compat(&party, DIR_SOUTH);
    F0284_CHAMPION_SetPartyDirection_Compat(&party, DIR_WEST);
    /* After 3 right turns, delta_wrapped = 3, equivalent to 1 left. */
    CHECK(party.champions[0].portraitIndex == 101, "T6: 3-right == 1-left, rank-0 -> 101");
    CHECK(party.champions[3].portraitIndex == 100, "T6: 3-right == 1-left, rank-3 -> 100");
    /* The cell rotation: in 3 right turns, slot i holds the
     * champion that was originally at slot (i - 3) mod 4 = (i+1) mod 4.
     * After 3 right turns the order is the same as 1 left turn. */

    /* T7: 1-champion party: rotation is a no-op on cell order. */
    setup_1_champion(&party);
    F0284_CHAMPION_SetPartyDirection_Compat(&party, DIR_EAST);
    CHECK(party.champions[0].portraitIndex == 100, "T7: 1-champ party: portrait 100 stays");
    CHECK(party.activeChampionIndex == 0, "T7: 1-champ party: active stays");

    /* T8: 3-champion party with empty slot at index 0.  The
     * approximation rotates present-list ranks; with the empty
     * slot at index 0 and 3 present champions, the present-list
     * is (rank 0=idx 1, rank 1=idx 2, rank 2=idx 3).  After right
     * turn: rank 0=idx 1 (still), but rank 0's champion moves to
     * rank 1's target slot.  Net: slot 0 stays empty (no
     * present-list element targets it after the shift), slot 1
     * gets rank 2's champion, slot 2 gets rank 0's champion, slot
     * 3 gets rank 1's champion.  Active follows the previously
     * active champion. */
    memset(&party, 0, sizeof(party));
    party.champions[0].present = 0; /* gap */
    party.champions[1].present = 1; party.champions[1].portraitIndex = 101; party.champions[1].direction = DIR_NORTH;
    party.champions[2].present = 1; party.champions[2].portraitIndex = 102; party.champions[2].direction = DIR_NORTH;
    party.champions[3].present = 1; party.champions[3].portraitIndex = 103; party.champions[3].direction = DIR_NORTH;
    party.championCount = 3;
    party.direction = DIR_NORTH;
    party.activeChampionIndex = 1;
    F0284_CHAMPION_SetPartyDirection_Compat(&party, DIR_EAST);
    /* Slot 0 stays empty (no present list rank maps to it). */
    CHECK(party.champions[0].present == 0, "T8: empty slot[0] preserved");
    /* Present ranks: 0=101, 1=102, 2=103. After right turn: rank 0 (101) -> rank 1's target (idx 2),
     * rank 1 (102) -> rank 2's target (idx 3), rank 2 (103) -> rank 0's target (idx 1).
     * So: idx 1=103, idx 2=101, idx 3=102. */
    CHECK(party.champions[1].portraitIndex == 103, "T8: idx 1 holds rank-2 champ (103)");
    CHECK(party.champions[2].portraitIndex == 101, "T8: idx 2 holds rank-0 champ (101)");
    CHECK(party.champions[3].portraitIndex == 102, "T8: idx 3 holds rank-1 champ (102)");
    /* Active was at idx 1 (rank 0). After rotation, rank 0 maps
     * to idx 2. Active follows. */
    CHECK(party.activeChampionIndex == 2, "T8: active moved from idx 1 to idx 2");

    /* T9: 2-champion party: rotation swaps the two. */
    memset(&party, 0, sizeof(party));
    party.champions[0].present = 1; party.champions[0].portraitIndex = 200; party.champions[0].direction = DIR_NORTH;
    party.champions[1].present = 1; party.champions[1].portraitIndex = 201; party.champions[1].direction = DIR_NORTH;
    party.champions[2].present = 0;
    party.champions[3].present = 0;
    party.championCount = 2;
    party.direction = DIR_NORTH;
    party.activeChampionIndex = 0;
    F0284_CHAMPION_SetPartyDirection_Compat(&party, DIR_EAST);
    /* Present ranks: 0=200, 1=201. After right: 0=201, 1=200. */
    CHECK(party.champions[0].portraitIndex == 201, "T9: rank-0 swapped (was 200, now 201)");
    CHECK(party.champions[1].portraitIndex == 200, "T9: rank-1 swapped (was 201, now 200)");
    CHECK(party.activeChampionIndex == 1, "T9: active moved to rank 1");

    /* T10: Non-contiguous 3-of-4 with empty slot at index 1.  The
     * approximation rotates the present-list.  Empty slot at
     * index 1 must stay empty.  Present ranks: 0=300 (idx 0),
     * 1=302 (idx 2), 2=303 (idx 3).  After right turn:
     * rank 0 -> rank 1's target = idx 2,
     * rank 1 -> rank 2's target = idx 3,
     * rank 2 -> rank 0's target = idx 0.
     * So: idx 0=303, idx 2=300, idx 3=302.  Empty slot at idx 1
     * stays empty. */
    memset(&party, 0, sizeof(party));
    party.champions[0].present = 1; party.champions[0].portraitIndex = 300;
    party.champions[1].present = 0; /* gap in the middle */
    party.champions[2].present = 1; party.champions[2].portraitIndex = 302;
    party.champions[3].present = 1; party.champions[3].portraitIndex = 303;
    party.championCount = 3;
    party.direction = DIR_NORTH;
    party.activeChampionIndex = 0;
    F0284_CHAMPION_SetPartyDirection_Compat(&party, DIR_EAST);
    CHECK(party.champions[1].present == 0, "T10: middle empty slot preserved");
    CHECK(party.champions[0].portraitIndex == 303, "T10: idx 0 holds rank-2 champ (303)");
    CHECK(party.champions[2].portraitIndex == 300, "T10: idx 2 holds rank-0 champ (300)");
    CHECK(party.champions[3].portraitIndex == 302, "T10: idx 3 holds rank-1 champ (302)");

    /* T11: All four cardinal directions reachable. */
    setup_4_champions(&party);
    F0284_CHAMPION_SetPartyDirection_Compat(&party, DIR_EAST);
    CHECK(party.direction == DIR_EAST, "T11: direction == EAST");
    F0284_CHAMPION_SetPartyDirection_Compat(&party, DIR_SOUTH);
    CHECK(party.direction == DIR_SOUTH, "T11: direction == SOUTH");
    F0284_CHAMPION_SetPartyDirection_Compat(&party, DIR_WEST);
    CHECK(party.direction == DIR_WEST, "T11: direction == WEST");
    F0284_CHAMPION_SetPartyDirection_Compat(&party, DIR_NORTH);
    CHECK(party.direction == DIR_NORTH, "T11: direction == NORTH");

    /* T12: portraitIndex preserved through rotation. */
    setup_4_champions(&party);
    F0284_CHAMPION_SetPartyDirection_Compat(&party, DIR_EAST);
    F0284_CHAMPION_SetPartyDirection_Compat(&party, DIR_SOUTH);
    F0284_CHAMPION_SetPartyDirection_Compat(&party, DIR_WEST);
    {
        int seen[4] = {0, 0, 0, 0};
        int i;
        for (i = 0; i < 4; ++i) {
            int p = party.champions[i].portraitIndex;
            if (p >= 100 && p < 104) seen[p - 100] = 1;
        }
        for (i = 0; i < 4; ++i) {
            CHECK(seen[i] == 1, "T12: all 4 portraits (100..103) still present after rotation");
        }
    }

    /* T13: delta=2 (right+right): each champion moves 2 slots. */
    setup_4_champions(&party);
    F0284_CHAMPION_SetPartyDirection_Compat(&party, DIR_EAST);
    F0284_CHAMPION_SetPartyDirection_Compat(&party, DIR_SOUTH);
    /* After 2 right turns, delta=2: rank 0 -> 2, 1 -> 3, 2 -> 0, 3 -> 1. */
    CHECK(party.champions[0].portraitIndex == 102, "T13: rank-0 (was 100) -> 102");
    CHECK(party.champions[1].portraitIndex == 103, "T13: rank-1 (was 101) -> 103");
    CHECK(party.champions[2].portraitIndex == 100, "T13: rank-2 (was 102) -> 100");
    CHECK(party.champions[3].portraitIndex == 101, "T13: rank-3 (was 103) -> 101");

    printf("PASS: MOV-05 F0284 cell-rotation invariants (13 scenarios)\n");
    return 0;
}
