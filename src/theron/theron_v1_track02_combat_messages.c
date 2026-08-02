#include "theron_v1_track02_combat_messages.h"
#include <stddef.h>

/* Source: US Track 02 BIN (MD5 f23601102138f87c33025877767ebf76). */

/* Item condition adjectives from UD 0x0865D4 */
static const char *g_item_conditions[THERON_TRACK02_ITEM_CONDITION_COUNT] = {
    "CONSUMABLE",
    "POISONED",
    "BROKEN",
    "CURSED",
    ", ",
    " AND ",
    ")",
};

const char *theron_v1_track02_item_condition(unsigned int index) {
    if (index >= THERON_TRACK02_ITEM_CONDITION_COUNT) return NULL;
    return g_item_conditions[index];
}

/* Combat feedback from UD 0x089A32 region (after class names) */
static const char *g_combat_feedback[THERON_TRACK02_COMBAT_FEEDBACK_COUNT] = {
    "IT COMES UP ",
    "HEADS.",
    "TAILS.",
    "CAN'T REACH",
    "NEED AMMO",
};

const char *theron_v1_track02_combat_feedback(unsigned int index) {
    if (index >= THERON_TRACK02_COMBAT_FEEDBACK_COUNT) return NULL;
    return g_combat_feedback[index];
}

/* System messages from UD 0x082E13 region */
const char *theron_v1_track02_system_wake_up(void) { return "WAKE UP"; }
const char *theron_v1_track02_system_game_frozen(void) { return "GAME FROZEN"; }

/* Resurrection from UD 0x086E70 */
const char *theron_v1_track02_system_resurrected(void) { return "RESURRECTED."; }

/* Turn pass from UD 0x088DCE */
const char *theron_v1_track02_system_pass(void) { return "PASS"; }

/* Theron-specific from UD 0x08BBBC region */
const char *theron_v1_track02_resurrect_theron(void) {
    return "GO AWAY AND RESURRECT THERON";
}

/* File select from UD 0x27519B */
const char *theron_v1_track02_file_select_play(void) {
    return "WHICH FILE DO YOU PLAY?";
}

/* Super CD-ROM2 requirement from UD 0x26C39D */
const char *theron_v1_track02_cdrom2_requirement(void) {
    return "the SUPER CD-ROM2 SYSTEM.";
}
