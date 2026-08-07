#ifndef FMTOWNS_SHARED_TABLES_ALL_GAMES_H
#define FMTOWNS_SHARED_TABLES_ALL_GAMES_H

#include <stdint.h>
#include "dm1_v1_fmtowns_music_tables.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Source-locked cross-game shared game tables.
 *
 * Byte-verified 2026-08-07 via pattern fingerprint search: several
 * DM1 EDM.EXP game tables are byte-identical in CSB CHTWE.EXP and
 * DM2 SKULL.EXP at per-game vaddrs — the games share the same
 * spell / player / icon constants at the data level even though
 * their surrounding code differs.
 *
 * Shared payload (byte-identical across games where matched):
 *
 *   SPELL_COSTS  (32 bytes)  DM1 @ 0x24388, CSB @ 0x29f64, DM2 @ 0x3bb0
 *   SPELL_MULT   (8 bytes)   DM1 @ 0x243a0, CSB @ 0x29f7c, DM2 @ 0x3bc8
 *   PLAYER_COLOR (8 bytes)   DM1 @ 0x291b8, CSB @ 0x2d164
 *   ICON_PAL     (6 bytes)   DM1 @ 0x28f44, CSB @ 0x2cd8a
 *
 * NOT shared (each game has its own values):
 *   LEVEL_SONGS  (music per game)
 *   DOOR_PAL     (per-game door palettes)
 *   DM_MUSIC     (per-game defaults)
 *
 * This module ships per-game vaddrs and defers to the DM1 modules
 * for the actual byte values (no duplicate storage).
 */

typedef struct {
    const char *game;
    uint32_t    spell_costs_vaddr;   /* 0 if not present */
    uint32_t    spell_mult_vaddr;
    uint32_t    player_color_vaddr;
    uint32_t    icon_pal_vaddr;
} fmtowns_shared_tables_vaddrs_t;

#define FMTOWNS_SHARED_TABLES_ALL_GAMES_COUNT  3U

extern const fmtowns_shared_tables_vaddrs_t
    fmtowns_shared_tables_all_games[FMTOWNS_SHARED_TABLES_ALL_GAMES_COUNT];

const fmtowns_shared_tables_vaddrs_t *
fmtowns_shared_tables_vaddrs_for_game_pc34(const char *game);

#ifdef __cplusplus
}
#endif

#endif /* FMTOWNS_SHARED_TABLES_ALL_GAMES_H */
