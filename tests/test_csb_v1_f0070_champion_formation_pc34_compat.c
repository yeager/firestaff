#include "csb_v1_f0070_champion_formation_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int passed;
static int failed;
#define CHECK(value, text) do { if (value) ++passed; else { ++failed; fprintf(stderr, "FAIL: %s\n", text); } } while (0)

static CsbV1F0070ChampionFormationStatePc34 three_champions(void)
{
    CsbV1F0070ChampionFormationStatePc34 state;
    memset(&state, 0, sizeof(state));
    state.champion_count = 3;
    state.party_direction = 1;
    state.cell[0] = 1;
    state.cell[1] = 2;
    state.cell[2] = 3;
    state.direction[0] = 0;
    state.direction[1] = 2;
    state.direction[2] = 3;
    return state;
}

int main(void)
{
    CsbV1F0070ChampionFormationStatePc34 state = three_champions();
    CsbV1F0070ChampionFormationReceiptPc34 receipt;

    /* icon 0 + party direction 1 selects champion 0 at cell 1. */
    CHECK(csb_v1_f0070_champion_formation_click_pc34(&state, 0, &receipt),
          "C125 picks up its occupied formation icon");
    CHECK(receipt.accepted && receipt.picked_up && receipt.source_icon_suppressed &&
          state.held_icon_ordinal == 1U && state.cell[0] == 1,
          "pickup changes only source-owned pointer ordinal, never inventory");

    /* icon 3 + direction 1 is empty cell 0: move rather than panel open. */
    CHECK(csb_v1_f0070_champion_formation_click_pc34(&state, 3, &receipt),
          "C128 releases formation icon into an empty cell");
    CHECK(receipt.released && receipt.moved_to_empty_cell && receipt.source_icon_cleared &&
          !receipt.swapped_with_occupant && state.held_icon_ordinal == 0U &&
          state.cell[0] == 0 && state.direction[0] == 1 &&
          (state.attributes[0] & CSB_V1_F0070_ATTRIBUTE_ICON_DIRTY_PC34),
          "empty-cell release is a durable move with source icon dirty flag");

    state = three_champions();
    CHECK(csb_v1_f0070_champion_formation_click_pc34(&state, 0, &receipt) &&
          csb_v1_f0070_champion_formation_click_pc34(&state, 1, &receipt),
          "two C125-C128 clicks admit a source formation swap");
    CHECK(receipt.swapped_with_occupant && state.cell[0] == 2 && state.cell[1] == 1 &&
          state.direction[0] == 1 &&
          (state.attributes[0] & CSB_V1_F0070_ATTRIBUTE_ICON_DIRTY_PC34) &&
          (state.attributes[1] & CSB_V1_F0070_ATTRIBUTE_ICON_DIRTY_PC34),
          "occupied release swaps cells and dirties both icon owners");

    state = three_champions();
    CHECK(!csb_v1_f0070_champion_formation_click_pc34(&state, 3, &receipt),
          "empty icon cannot be picked up");
    CHECK(state.held_icon_ordinal == 0U && state.cell[0] == 1,
          "rejected empty pickup leaves formation untouched");
    CHECK(strstr(csb_v1_f0070_champion_formation_source_evidence_pc34(),
                 "F0378 handles only C081") != NULL,
          "evidence explicitly separates formation from inventory panel route");

    printf("%d passed, %d failed\n", passed, failed);
    return failed != 0;
}
