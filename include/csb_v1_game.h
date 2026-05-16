
#ifndef CSB_V1_GAME_H
#define CSB_V1_GAME_H
#include <stdint.h>

/* Chaos Strikes Back V1 game logic.
 * CSB shares DM1 engine but adds:
 *   - New dungeon (DUNGEON.DAT is 2098 bytes — very small, 1 level?)
 *   - Import DM1 champions
 *   - New spells and creatures
 *   - Harder difficulty */

typedef struct {
    int party_x, party_y, party_dir;
    int current_level;
    int dm1_import_done;
    int difficulty;          /* CSB is harder: 1.5x creature stats */
    const char *data_dir;
} CSB_V1_GameState;

void csb_v1_init(CSB_V1_GameState *state, const char *data_dir);
int csb_v1_load_dungeon(CSB_V1_GameState *state);
int csb_v1_import_dm1_save(CSB_V1_GameState *state, const char *dm1_save_path);

#endif

