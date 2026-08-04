#include "dm2_v1_skill_query_pc34_compat.h"

/*
 * Source: skgdtqdb.cpp:584-633 DM2_QUERY_PLAYER_SKILL_LV
 *
 * The skill level computation:
 *   1. If override_mode (ddat.v1e0238 != 0), return 1
 *   2. Start with raw skill XP at skill[skill_index]
 *   3. For sub-skills (index >= 4):
 *      - Compute parent class index: (index - 4) / 4
 *      - If use_bonus: multiplier = sbonus[parent] + 1
 *        else: multiplier = 1
 *      - Add parent_skill_xp * multiplier
 *      - Right-shift result by 1
 *   4. Convert XP to level: count how many times XP >= 0x200,
 *      halving each time. Level starts at 1.
 *   5. If use_bonus: add sbonus[skill_index] to level
 *   6. Clamp result to minimum 1
 */

static int16_t dm2_max_i16(int16_t a, int16_t b)
{
    return a > b ? a : b;
}

int16_t dm2_v1_query_player_skill_lv(
    const DM2_V1_HeroSkills *hero,
    int16_t skill_index,
    int use_bonus,
    int override_mode)
{
    if (!hero)
        return 1;

    if (override_mode)
        return 1;

    if (skill_index < 0 || skill_index >= DM2_V1_SKILL_ARRAY_SIZE)
        return 1;

    uint16_t xp = (uint16_t)hero->skill[skill_index];

    if (skill_index >= 4) {
        int16_t parent_class;
        int16_t multiplier;

        if (use_bonus) {
            parent_class = (skill_index - 4) >> 2;
            multiplier = (int16_t)(hero->sbonus[parent_class] + 1);
        } else {
            multiplier = 1;
        }

        int16_t parent_skill_idx = skill_index / 4 - 1;
        xp += (uint16_t)(hero->skill[parent_skill_idx] * multiplier);
        xp >>= 1;
    }

    int16_t level = 1;
    while (xp >= 0x200) {
        xp >>= 1;
        level++;
    }

    if (use_bonus) {
        level += hero->sbonus[skill_index];
        level = dm2_max_i16(1, level);
    }

    return level;
}
