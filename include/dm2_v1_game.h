
#ifndef DM2_V1_GAME_H
#define DM2_V1_GAME_H
#include <stdint.h>

/* Dungeon Master II: The Legend of Skullkeep V1 game logic.
 * DM2 has a different engine with outdoor areas, shops, NPCs.
 * GRAPHICS.DAT is 8.6 MB (vs DM1's 363 KB) — much more art.
 * DUNGEON.DAT is 39 KB (vs DM1's 33 KB). */

typedef struct {
    int party_x, party_y, party_dir;
    int current_level;
    int outdoor;            /* DM2 has outdoor areas */
    int gold;               /* DM2 has currency/shops */
    int reputation;         /* NPC interaction */
    int time_of_day;        /* day/night cycle */
    const char *data_dir;
} DM2_V1_GameState;

void dm2_v1_init(DM2_V1_GameState *state, const char *data_dir);
int dm2_v1_load_dungeon(DM2_V1_GameState *state);
int dm2_v1_enter_shop(DM2_V1_GameState *state);
int dm2_v1_is_outdoor(const DM2_V1_GameState *state);

#endif

