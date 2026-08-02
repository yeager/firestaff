#ifndef THERON_V1_TRACK02_SPELL_ACTION_NAMES_H
#define THERON_V1_TRACK02_SPELL_ACTION_NAMES_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Source: US Track 02 BIN, UD 0x09EEA3.
 * Combined action + spell name table, null-terminated ASCII strings.
 * Indices 0-19: combat actions (N, BLOCK, CHOP, ..., BERZERK).
 * Indices 20-40: spells (FIREBALL, DISPELL, ..., THROW).
 * "X" entries (indices 3, 26) are unused placeholders. */

#define THERON_TRACK02_ACTION_SPELL_COUNT  41u
#define THERON_TRACK02_FIRST_SPELL_INDEX   20u

const char *theron_v1_track02_us_action_spell_name(unsigned int index);
size_t theron_v1_track02_us_action_spell_count(void);

/* Skill level names from UD 0x089B6B.
 * 16 levels: NEOPHYTE through ARCHMASTER. */
#define THERON_TRACK02_SKILL_LEVEL_COUNT  15u

const char *theron_v1_track02_us_skill_level_name(unsigned int index);
size_t theron_v1_track02_us_skill_level_count(void);

#ifdef __cplusplus
}
#endif

#endif /* THERON_V1_TRACK02_SPELL_ACTION_NAMES_H */
