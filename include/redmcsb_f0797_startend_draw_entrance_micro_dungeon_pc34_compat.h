/*
 * ReDMCSB ENTRANCE.C F0797_STARTEND_DrawEntranceMicroDungeon, PC 3.4 route.
 */
#ifndef FIRESTAFF_REDMCSB_F0797_STARTEND_DRAW_ENTRANCE_MICRO_DUNGEON_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0797_STARTEND_DRAW_ENTRANCE_MICRO_DUNGEON_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    REDMCSB_F0797_MAP_INDEX_ENTRANCE_PC34 = 255,
    REDMCSB_F0797_MICRO_DUNGEON_WIDTH_PC34 = 5,
    REDMCSB_F0797_MICRO_DUNGEON_HEIGHT_PC34 = 5,
    REDMCSB_F0797_MICRO_DUNGEON_SQUARE_COUNT_PC34 = 25,
    REDMCSB_F0797_DIRECTION_SOUTH_PC34 = 2,
    REDMCSB_F0797_SQUARE_WALL_PC34 = 0,
    REDMCSB_F0797_SQUARE_CORRIDOR_PC34 = 32
};

typedef struct RedmcsbF0797EntranceDrawStatePc34 {
    int16_t party_map_index;
    int draw_floor_and_ceiling_requested;
    int16_t current_map_width;
    int16_t current_map_height;
} RedmcsbF0797EntranceDrawStatePc34;

typedef void (*RedmcsbF0797DrawDungeonViewPc34)(
    void *context,
    int16_t direction,
    int16_t party_x,
    int16_t party_y,
    const uint8_t *const map_columns[REDMCSB_F0797_MICRO_DUNGEON_WIDTH_PC34]);

/*
 * F0797's caller owns the actual dungeon-view renderer (F0128).  This adapter
 * does not synthesize pixels; its callback receives the exact temporary map
 * layout while the source's stack-local map data is live.
 */
void redmcsb_f0797_startend_draw_entrance_micro_dungeon_pc34_compat(
    RedmcsbF0797EntranceDrawStatePc34 *state,
    RedmcsbF0797DrawDungeonViewPc34 draw_dungeon_view,
    void *context);

const char *redmcsb_f0797_startend_draw_entrance_micro_dungeon_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
