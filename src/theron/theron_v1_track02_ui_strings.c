#include "theron_v1_track02_ui_strings.h"

/* Source: US Track 02 BIN, hash-verified MD5 f23601102138f87c33025877767ebf76.
 * Evidence: parity-evidence/pass215_theron_track02_binary_analysis.md */

/* UD 0x1C9C5B: flask/container charge states */
static const char *const g_flask_states[THERON_TRACK02_FLASK_STATE_COUNT] = {
    "(EMPTY)",        /* 0 */
    "(ALMOST EMPTY)", /* 1 */
    "(ALMOST FULL)",  /* 2 */
    "(FULL)",         /* 3 */
};

/* UD 0x1C9C8C: compass direction names */
static const char *const g_directions[THERON_TRACK02_DIRECTION_COUNT] = {
    "NORTH", "EAST", "SOUTH", "WEST",
};

/* UD 0x1C65D7: item condition statuses */
static const char *const g_item_statuses[THERON_TRACK02_ITEM_STATUS_COUNT] = {
    "POISONED", "BROKEN", "CURSED",
};

const char *theron_v1_track02_us_flask_state(unsigned int index) {
    if (index >= THERON_TRACK02_FLASK_STATE_COUNT) return NULL;
    return g_flask_states[index];
}

const char *theron_v1_track02_us_direction_name(unsigned int index) {
    if (index >= THERON_TRACK02_DIRECTION_COUNT) return NULL;
    return g_directions[index];
}

/* UD 0x1C9C7F */
const char *theron_v1_track02_us_party_facing(void) { return "PARTY FACING"; }

/* UD 0x1C9CA2 */
const char *theron_v1_track02_us_weighs(void) { return "WEIGHS"; }

/* UD 0x1C9CA9 */
const char *theron_v1_track02_us_kg_suffix(void) { return " KG."; }

/* UD 0x1C9CAE */
const char *theron_v1_track02_us_burnt_out(void) { return "(BURNT OUT)"; }

/* UD 0x1C65D4 */
const char *theron_v1_track02_us_consumable(void) { return "CONSUMABLE"; }

const char *theron_v1_track02_us_item_status(unsigned int index) {
    if (index >= THERON_TRACK02_ITEM_STATUS_COUNT) return NULL;
    return g_item_statuses[index];
}

/* UD 0x1C65F8 */
const char *theron_v1_track02_us_status_separator(void) { return ", "; }

/* UD 0x1C65FB */
const char *theron_v1_track02_us_status_and(void) { return " AND "; }

/* UD 0x1C2E1D */
const char *theron_v1_track02_us_wake_up(void) { return "WAKE UP"; }

/* UD 0x1C2E25 */
const char *theron_v1_track02_us_game_frozen(void) { return "GAME FROZEN"; }

/* UD 0x1C6E72 */
const char *theron_v1_track02_us_resurrected(void) { return "RESURRECTED."; }
