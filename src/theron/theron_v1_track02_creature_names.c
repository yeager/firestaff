/*
 * theron_v1_track02_creature_names.c — legacy API for TQ dungeon labels
 *
 * Binary-verified from hash-locked US Track 02 BIN
 * (MD5: f23601102138f87c33025877767ebf76).
 *
 * Dungeon/region label table: UD 0x2741EF (7 entries, 8-byte stride,
 * 7-char fixed-width ASCII padded with spaces, 0x01 separator,
 * final entry terminated with 0x00).
 *
 * "GAME SPEED" options menu label: UD 0x274228.
 */

#include "theron_v1_track02_creature_names.h"
#include <stddef.h>

static const char *const g_creature_names[THERON_TRACK02_CREATURE_TYPE_COUNT] = {
    "AKUTUBA",     /* dungeon 0 — UD 0x2741EF */
    "DRATOR",      /* dungeon 1 — UD 0x2741F7 */
    "FORMIC",      /* dungeon 2 — UD 0x2741FF */
    "SARMON",      /* dungeon 3 — UD 0x274207 */
    "SHADO",       /* dungeon 4 — UD 0x27420F */
    "THIEF",       /* dungeon 5 — UD 0x274217 */
    "DEMON",       /* dungeon 6 — UD 0x27421F */
};

const char *theron_v1_track02_us_creature_name(unsigned int index)
{
    if (index >= THERON_TRACK02_CREATURE_TYPE_COUNT) return NULL;
    return g_creature_names[index];
}

const char *theron_v1_track02_us_game_speed_label(void)
{
    return "GAME SPEED";
}
