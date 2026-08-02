#include "theron_v1_track02_spell_action_names.h"
#include <stddef.h>

/* Source: US Track 02 BIN (MD5 f23601102138f87c33025877767ebf76).
 * Action/spell names from UD 0x09EEA3, null-terminated ASCII.
 * Skill level names from UD 0x089B6B. */

static const char *const g_action_spell_names[THERON_TRACK02_ACTION_SPELL_COUNT] = {
    /* 0-19: combat actions */
    "N", "BLOCK", "CHOP", "X", "BLOW HORN", "FLIP",
    "PUNCH", "KICK", "WAR CRY", "STAB", "CLIMB DOWN",
    "FREEZE LIFE", "HIT", "SWING", "STAB", "THRUST",
    "JAB", "PARRY", "HACK", "BERZERK",
    /* 20-40: spells */
    "FIREBALL", "DISPELL", "CONFUSE", "LIGHTNING", "DISRUPT",
    "MELEE", "X", "INVOKE", "SLASH", "CLEAVE",
    "BASH", "STUN", "SHOOT", "SPELLSHIELD", "FIRESHIELD",
    "HEAL", "CALM", "LIGHT", "SPIT", "BRANDISH", "THROW",
};

const char *theron_v1_track02_us_action_spell_name(unsigned int index) {
    if (index >= THERON_TRACK02_ACTION_SPELL_COUNT) return NULL;
    return g_action_spell_names[index];
}

size_t theron_v1_track02_us_action_spell_count(void) {
    return THERON_TRACK02_ACTION_SPELL_COUNT;
}

static const char *const g_skill_levels[THERON_TRACK02_SKILL_LEVEL_COUNT] = {
    "NEOPHYTE", "NOVICE", "APPRENTICE", "JOURNEYMAN",
    "CRAFTSMAN", "ARTISAN", "ADEPT", "EXPERT",
    "` MASTER", "a MASTER", "b MASTER", "c MASTER",
    "d MASTER", "e MASTER", "ARCHMASTER",
};

const char *theron_v1_track02_us_skill_level_name(unsigned int index) {
    if (index >= THERON_TRACK02_SKILL_LEVEL_COUNT) return NULL;
    return g_skill_levels[index];
}

size_t theron_v1_track02_us_skill_level_count(void) {
    return THERON_TRACK02_SKILL_LEVEL_COUNT;
}
