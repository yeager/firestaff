#include "fmtowns_geometry_all_games.h"
#include <stddef.h>
#include <string.h>

/* Byte-verified 2026-08-07 via 14-byte fingerprint search on the
 * shipping FM Towns Dungeon Master discs. ICON pattern starts at
 * SCR_X_SIZE (word offset 0), CHAR starts at CHAR_X_SIZE. */
const fmtowns_geometry_vaddr_t
fmtowns_geometry_all_games[FMTOWNS_GEOMETRY_ALL_GAMES_COUNT] = {
    /* DM1: SCR_X_SIZE @ 0x26c68, CHAR_X_SIZE @ 0x26c8a
     * (delta = 0x22 = 34 bytes = 17 words of intervening data). */
    { "DM1", "EDM.EXP",   0x26c68u, 0x26c8au },

    /* CSB CHTWE.EXP: ICON @ 0x2c938, CHAR @ 0x2c94c
     * (delta = 0x14 = 20 bytes = 10 words). Different intervening
     * layout from DM1 but same 4-word ICON block + 7-word CHAR block. */
    { "CSB", "CHTWE.EXP", 0x2c938u, 0x2c94cu },

    /* DM2 SKULL.EXP: ICON @ 0x1de, CHAR @ 0x1f6
     * (delta = 0x18 = 24 bytes). Also different intervening layout
     * but identical CHAR/ICON constants. */
    { "DM2", "SKULL.EXP", 0x001deu, 0x001f6u }
};

const fmtowns_geometry_vaddr_t *
fmtowns_geometry_vaddrs_for_game_pc34(const char *game) {
    unsigned int i;
    if (!game) return NULL;
    for (i = 0; i < FMTOWNS_GEOMETRY_ALL_GAMES_COUNT; ++i) {
        if (strcmp(fmtowns_geometry_all_games[i].game, game) == 0) {
            return &fmtowns_geometry_all_games[i];
        }
    }
    return NULL;
}
