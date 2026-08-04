
#ifndef DM2_V1_GAME_H
#define DM2_V1_GAME_H
#include <stdint.h>

/* Dungeon Master II: The Legend of Skullkeep V1 game logic.
 * DM2 has a different engine with outdoor areas, shops, NPCs.
 * GRAPHICS.DAT is 8.6 MB (vs DM1's 363 KB) — much more art.
 * DUNGEON.DAT is 39 KB (vs DM1's 33 KB). */

/* Three-tier glob var storage (bits/bytes/words) embedded directly
 * to avoid pulling in the full runtime_parity header. Layout matches
 * skproject ddat.v1e0104 / ddat.globalb / ddat.v1e000c. */
typedef struct {
    uint8_t bit_vars[8];
    uint8_t byte_vars[64];
    int16_t word_vars[192];
} DM2_V1_GameGlobVars;

typedef struct DM2_V1_GameState {
    int party_x, party_y, party_dir;
    int current_level;
    int outdoor;            /* DM2 has outdoor areas */
    int gold;               /* DM2 has currency/shops */
    int reputation;         /* NPC interaction */
    int time_of_day;        /* day/night cycle */
    const char *data_dir;
    DM2_V1_GameGlobVars glob_vars;
} DM2_V1_GameState;

/* Actuator callback adapters — pass DM2_V1_GameState* as ctx */
int dm2_v1_game_get_glob_var(void *ctx, uint16_t index);
void dm2_v1_game_update_glob_var(void *ctx, uint16_t index, int op, uint16_t value);

void dm2_v1_init(DM2_V1_GameState *state, const char *data_dir);
/* Legacy shim. Always fails: only dm2_v1_boot_enter_game() may publish a
 * hash-verified, parsed DM2 dungeon and its record/map ownership. */
int dm2_v1_load_dungeon(DM2_V1_GameState *state);
/* Always fails until the caller supplies the original shop-glass actuator,
 * WALL_GFX/GDAT material and record-owned transaction state. */
int dm2_v1_enter_shop(DM2_V1_GameState *state);
int dm2_v1_is_outdoor(const DM2_V1_GameState *state);

#endif
