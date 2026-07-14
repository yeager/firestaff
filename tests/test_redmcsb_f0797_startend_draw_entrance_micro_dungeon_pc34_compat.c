#include <stdint.h>
#include <string.h>

#include "redmcsb_f0797_startend_draw_entrance_micro_dungeon_pc34_compat.h"

typedef struct TestState {
    int calls;
    int16_t direction;
    int16_t party_x;
    int16_t party_y;
    uint8_t squares[REDMCSB_F0797_MICRO_DUNGEON_SQUARE_COUNT_PC34];
} TestState;

static void draw_dungeon_view(
    void *context,
    int16_t direction,
    int16_t party_x,
    int16_t party_y,
    const uint8_t *const map_columns[REDMCSB_F0797_MICRO_DUNGEON_WIDTH_PC34])
{
    TestState *state = context;
    int16_t column;
    int16_t row;

    state->calls++;
    state->direction = direction;
    state->party_x = party_x;
    state->party_y = party_y;
    for (column = 0; column < REDMCSB_F0797_MICRO_DUNGEON_WIDTH_PC34; ++column) {
        for (row = 0; row < REDMCSB_F0797_MICRO_DUNGEON_HEIGHT_PC34; ++row) {
            state->squares[column * REDMCSB_F0797_MICRO_DUNGEON_WIDTH_PC34 + row] =
                map_columns[column][row];
        }
    }
}

int main(void)
{
    RedmcsbF0797EntranceDrawStatePc34 entrance_state = {0};
    TestState test_state = {0};
    int16_t square_index;

    redmcsb_f0797_startend_draw_entrance_micro_dungeon_pc34_compat(
        &entrance_state, draw_dungeon_view, &test_state);

    if (entrance_state.party_map_index != REDMCSB_F0797_MAP_INDEX_ENTRANCE_PC34 ||
        entrance_state.draw_floor_and_ceiling_requested != 1 ||
        entrance_state.current_map_width != REDMCSB_F0797_MICRO_DUNGEON_WIDTH_PC34 ||
        entrance_state.current_map_height != REDMCSB_F0797_MICRO_DUNGEON_HEIGHT_PC34 ||
        test_state.calls != 1 ||
        test_state.direction != REDMCSB_F0797_DIRECTION_SOUTH_PC34 ||
        test_state.party_x != 2 || test_state.party_y != 0) {
        return 1;
    }

    for (square_index = 0;
         square_index < REDMCSB_F0797_MICRO_DUNGEON_SQUARE_COUNT_PC34;
         ++square_index) {
        const uint8_t expected =
            ((square_index >= 10 && square_index <= 14) || square_index == 7)
                ? REDMCSB_F0797_SQUARE_CORRIDOR_PC34
                : REDMCSB_F0797_SQUARE_WALL_PC34;
        if (test_state.squares[square_index] != expected) {
            return 1;
        }
    }

    if (strcmp(
            redmcsb_f0797_startend_draw_entrance_micro_dungeon_source_evidence_pc34(),
            "ReDMCSB ENTRANCE.C:58-81; C255 5x5 temporary map, F0128 south view from (2,0)") !=
        0) {
        return 1;
    }

    return 0;
}
