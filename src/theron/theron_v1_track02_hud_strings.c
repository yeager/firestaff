#include "theron_v1_track02_hud_strings.h"

/* Source: US Track 02 BIN (MD5 f23601102138f87c33025877767ebf76).
 * HUD display strings from UD 0x089A6E-0x089C50 region. */

static const char *const g_flask_levels[THERON_TRACK02_FLASK_LEVEL_COUNT] = {
    "(EMPTY)", "(ALMOST EMPTY)", "(ALMOST FULL)", "(FULL)",
};

static const char *const g_directions[THERON_TRACK02_DIRECTION_COUNT] = {
    "NORTH", "EAST", "SOUTH", "WEST",
};

static const char *const g_stat_names[THERON_TRACK02_STAT_NAME_COUNT] = {
    "STRENGTH", "DEXTERITY", "WISDOM",
    "VITALITY", "ANTI-MAGIC", "ANTI-FIRE",
};

static const char *const g_vital_names[THERON_TRACK02_VITAL_NAME_COUNT] = {
    "HEALTH", "STAMINA", "MANA",
};

static const char *const g_spell_messages[THERON_TRACK02_SPELL_MSG_COUNT] = {
    " NEEDS MORE PRACTICE WITH THIS ",
    " MUMBLES A MEANINGLESS SPELL.",
    " NEEDS AN EMPTY FLASK IN HAND FOR POTION.",
    " SPELL.",
    " JUST GAINED A ",
    " LEVEL!",
};

const char *theron_v1_track02_us_flask_level(unsigned int index) {
    if (index >= THERON_TRACK02_FLASK_LEVEL_COUNT) return NULL;
    return g_flask_levels[index];
}

const char *theron_v1_track02_us_direction_name(unsigned int index) {
    if (index >= THERON_TRACK02_DIRECTION_COUNT) return NULL;
    return g_directions[index];
}

const char *theron_v1_track02_us_stat_name(unsigned int index) {
    if (index >= THERON_TRACK02_STAT_NAME_COUNT) return NULL;
    return g_stat_names[index];
}

const char *theron_v1_track02_us_vital_name(unsigned int index) {
    if (index >= THERON_TRACK02_VITAL_NAME_COUNT) return NULL;
    return g_vital_names[index];
}

const char *theron_v1_track02_us_spell_message(unsigned int index) {
    if (index >= THERON_TRACK02_SPELL_MSG_COUNT) return NULL;
    return g_spell_messages[index];
}

const char *theron_v1_track02_us_weight_label(void) { return "WEIGHS"; }
const char *theron_v1_track02_us_weight_unit(void) { return " KG."; }
const char *theron_v1_track02_us_burnt_out_label(void) { return "(BURNT OUT)"; }
const char *theron_v1_track02_us_party_facing_label(void) { return "PARTY FACING"; }
