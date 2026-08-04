#ifndef FIRESTAFF_DM2_V1_SKILL_QUERY_PC34_COMPAT_H
#define FIRESTAFF_DM2_V1_SKILL_QUERY_PC34_COMPAT_H

/*
 * dm2_v1_skill_query_pc34_compat.h — DM2 skill level query.
 *
 * Ports DM2_QUERY_PLAYER_SKILL_LV from skproject skgdtqdb.cpp:584-633.
 * Maps a hero's raw skill experience to an effective skill level,
 * combining base skill, parent skill, and skill bonuses.
 *
 * The skill array is organized as:
 *   indices 0-3:  class skills (Fighter, Ninja, Priest, Wizard)
 *   indices 4-7:  Fighter sub-skills
 *   indices 8-11: Ninja sub-skills
 *   indices 12-15: Priest sub-skills
 *   indices 16-19: Wizard sub-skills
 *
 * For sub-skills (index >= 4), the effective level combines:
 *   raw sub-skill XP + (parent_class_level * bonus_multiplier)
 * then halved by right-shift.
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM2_V1_SKILL_ARRAY_SIZE 20
#define DM2_V1_SBONUS_ARRAY_SIZE 20

typedef struct {
    int16_t skill[DM2_V1_SKILL_ARRAY_SIZE];
    int16_t sbonus[DM2_V1_SBONUS_ARRAY_SIZE];
} DM2_V1_HeroSkills;

/* Query the effective skill level for a hero.
 *
 * skill_index: 0-19 (0-3 = class, 4-19 = sub-skills)
 * use_bonus: if nonzero, add sbonus to the result
 * override_mode: if nonzero, always returns 1 (training mode)
 *
 * Returns effective skill level (>= 1).
 *
 * Source: skgdtqdb.cpp:584-633 DM2_QUERY_PLAYER_SKILL_LV
 */
int16_t dm2_v1_query_player_skill_lv(
    const DM2_V1_HeroSkills *hero,
    int16_t skill_index,
    int use_bonus,
    int override_mode);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_SKILL_QUERY_PC34_COMPAT_H */
