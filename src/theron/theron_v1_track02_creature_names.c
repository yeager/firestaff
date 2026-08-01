/*
 * theron_v1_track02_creature_names.c — TQ US Track 02 creature type names
 *
 * Binary-verified from hash-locked US Track 02 BIN
 * (MD5: f23601102138f87c33025877767ebf76).
 *
 * Creature name table: UD 0x2741EF (7 entries, 8-byte stride,
 * 7-char fixed-width ASCII padded with spaces, 0x01 separator,
 * final entry terminated with 0x00).
 *
 * "GAME SPEED" options menu label: UD 0x274228.
 */

#include "theron_v1_track02_creature_names.h"
#include <stddef.h>

static const char *const g_creature_names[THERON_TRACK02_CREATURE_TYPE_COUNT] = {
    "AKUTUBA",     /* type 0 — UD 0x2741EF */
    "DRATOR",      /* type 1 — UD 0x2741F7 */
    "FORMIC",      /* type 2 — UD 0x2741FF */
    "SARMON",      /* type 3 — UD 0x274207 */
    "SHADO",       /* type 4 — UD 0x27420F */
    "THIEF",       /* type 5 — UD 0x274217 */
    "DEMON",       /* type 6 — UD 0x27421F */
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
