#include "redmcsb_f0797_startend_draw_entrance_micro_dungeon_pc34_compat.h"

void redmcsb_f0797_startend_draw_entrance_micro_dungeon_pc34_compat(
    RedmcsbF0797EntranceDrawStatePc34 *state,
    RedmcsbF0797DrawDungeonViewPc34 draw_dungeon_view,
    void *context)
{
    uint8_t micro_dungeon_squares[REDMCSB_F0797_MICRO_DUNGEON_SQUARE_COUNT_PC34];
    const uint8_t *micro_dungeon_current_map_data[REDMCSB_F0797_MICRO_DUNGEON_WIDTH_PC34];
    int16_t column_index;
    int16_t square_index;

    state->party_map_index = REDMCSB_F0797_MAP_INDEX_ENTRANCE_PC34;
    state->draw_floor_and_ceiling_requested = 1;
    state->current_map_width = REDMCSB_F0797_MICRO_DUNGEON_WIDTH_PC34;
    state->current_map_height = REDMCSB_F0797_MICRO_DUNGEON_HEIGHT_PC34;

    for (square_index = 0;
         square_index < REDMCSB_F0797_MICRO_DUNGEON_SQUARE_COUNT_PC34;
         ++square_index) {
        micro_dungeon_squares[square_index] = REDMCSB_F0797_SQUARE_WALL_PC34;
    }
    for (column_index = 0;
         column_index < REDMCSB_F0797_MICRO_DUNGEON_WIDTH_PC34;
         ++column_index) {
        micro_dungeon_current_map_data[column_index] =
            &micro_dungeon_squares[column_index * REDMCSB_F0797_MICRO_DUNGEON_WIDTH_PC34];
        micro_dungeon_squares[column_index + 10] =
            REDMCSB_F0797_SQUARE_CORRIDOR_PC34;
    }
    micro_dungeon_squares[7] = REDMCSB_F0797_SQUARE_CORRIDOR_PC34;

    draw_dungeon_view(
        context,
        REDMCSB_F0797_DIRECTION_SOUTH_PC34,
        2,
        0,
        micro_dungeon_current_map_data);
}

const char *redmcsb_f0797_startend_draw_entrance_micro_dungeon_source_evidence_pc34(void)
{
    return "ReDMCSB ENTRANCE.C:58-81; C255 5x5 temporary map, F0128 south view from (2,0)";
}
