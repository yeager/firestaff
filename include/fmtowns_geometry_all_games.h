#ifndef FMTOWNS_GEOMETRY_ALL_GAMES_H
#define FMTOWNS_GEOMETRY_ALL_GAMES_H

#include <stdint.h>
#include "dm1_v1_fmtowns_text_geometry.h"
#include "dm1_v1_fmtowns_icon_geometry.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Source-locked FM Towns cross-game text + icon geometry.
 *
 * Byte-verified via 14-byte pattern fingerprint search across all
 * three FM Towns Dungeon Master game binaries:
 *
 *   ICON geometry (SCR_X=320, ICON_SIZE=256, ICON_X=16, ICON_Y=16)
 *     encodes as bytes: 40 01 00 01 10 00 10 00 (starting at
 *     SCR_X_SIZE @ +0x00 in each game's data segment)
 *
 *   CHAR geometry (X=5, Y=6, X_SPC=1, Y_SPC=1, DESCENDER=1,
 *                   X_WID=6, Y_HYT=7)
 *     encodes as 14 bytes: 05 00 06 00 01 00 01 00 01 00 06 00 07 00
 *
 * BOTH patterns match byte-exact in all 3 game binaries at
 * per-game vaddrs. That means CSB and DM2 use the SAME geometry
 * constants as DM1 — the shared DM1_V1_FMTOWNS_CHAR_* and
 * DM1_V1_FMTOWNS_ICON_* macros are the source-locked values for
 * every game.
 *
 * Per-game vaddrs (for direct byte access in each executable):
 */

typedef struct {
    const char *game;
    const char *binary;
    uint32_t    icon_size_vaddr;     /* start of ICON constants (SCR_X_SIZE) */
    uint32_t    char_geometry_vaddr; /* start of CHAR_X_SIZE */
} fmtowns_geometry_vaddr_t;

#define FMTOWNS_GEOMETRY_ALL_GAMES_COUNT  3U

extern const fmtowns_geometry_vaddr_t
    fmtowns_geometry_all_games[FMTOWNS_GEOMETRY_ALL_GAMES_COUNT];

/* Return the per-game vaddr set, or NULL for unknown game name. */
const fmtowns_geometry_vaddr_t *
fmtowns_geometry_vaddrs_for_game_pc34(const char *game);

#ifdef __cplusplus
}
#endif

#endif /* FMTOWNS_GEOMETRY_ALL_GAMES_H */
