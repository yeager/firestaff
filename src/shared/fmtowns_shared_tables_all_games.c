#include "fmtowns_shared_tables_all_games.h"
#include <stddef.h>
#include <string.h>

const fmtowns_shared_tables_vaddrs_t
fmtowns_shared_tables_all_games[FMTOWNS_SHARED_TABLES_ALL_GAMES_COUNT] = {
    { "DM1", 0x24388u, 0x243a0u, 0x291b8u, 0x28f44u },
    { "CSB", 0x29f64u, 0x29f7cu, 0x2d164u, 0x2cd8au },
    { "DM2", 0x03bb0u, 0x03bc8u, 0x00000u, 0x00000u }
};

const fmtowns_shared_tables_vaddrs_t *
fmtowns_shared_tables_vaddrs_for_game_pc34(const char *game) {
    unsigned int i;
    if (!game) return NULL;
    for (i = 0; i < FMTOWNS_SHARED_TABLES_ALL_GAMES_COUNT; ++i) {
        if (strcmp(fmtowns_shared_tables_all_games[i].game, game) == 0) {
            return &fmtowns_shared_tables_all_games[i];
        }
    }
    return NULL;
}
