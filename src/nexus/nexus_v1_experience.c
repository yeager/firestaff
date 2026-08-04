
#include "nexus_v1_experience.h"
#include <string.h>

void nexus_v1_experience_init(Nexus_V1_ExperienceState *state)
{
    if (!state) return;
    memset(state, 0, sizeof(*state));
}

/* DM.BIN 0x0604B5DC: 6-byte table, each byte << 8. Linear 10240/level. */
static const int nexus_xp_thresholds[NEXUS_MAX_CLASS_LEVEL + 1] = {
    0, 10240, 20480, 30720, 40960, 51200, 61440
};

/* DM.BIN 0x03A260: skill progression tables (4 classes × 32 bytes).
 * Maps raw XP index (0-31) to effective skill level for each class.
 * Fighter peaks at 16, ninja at 14, priest at 12, wizard at 10. */
static const int nexus_skill_table[4][32] = {
    {0,0,0,0,0,1,1,1,1,1,2,2,2,2,3,3,3,4,4,5,5,6,7,8,9,10,11,12,13,14,15,16},
    {0,0,0,0,0,0,0,0,1,1,1,1,1,1,2,2,2,3,3,3,3,4,5,6,7,8,9,10,11,12,13,14},
    {0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,2,2,2,3,3,3,4,5,5,6,7,8,9,10,11,12},
    {0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,2,2,2,3,3,4,5,5,5,6,7,8,9,10},
};

/* DM.BIN 0x03B8CC: stat init table — 15 BE16 values + 0xFFFF sentinel.
 * Purpose: initial stat values for champions (indexed by champion rank). */
static const int nexus_stat_init_table[15] = {
    1800,1500,1200,1200,900,900,900,600,600,600,480,480,480,360,360
};

/* DM.BIN 0x029F76: compute skill level from raw XP index using class table. */
int nexus_v1_skill_level_for_index(int class_index, int xp_index)
{
    if (class_index < 0 || class_index >= 4) return 0;
    if (xp_index < 0) return 0;
    if (xp_index > 31) xp_index = 31;
    return nexus_skill_table[class_index][xp_index];
}

int nexus_v1_stat_init_value(int rank)
{
    if (rank < 0 || rank >= 15) return 360;
    return nexus_stat_init_table[rank];
}

int nexus_v1_experience_level_for_xp(int xp)
{
    int level;
    if (xp < 0) return 0;
    for (level = NEXUS_MAX_CLASS_LEVEL; level > 0; level--) {
        if (xp >= nexus_xp_thresholds[level]) return level;
    }
    return 0;
}

void nexus_v1_experience_award_combat(Nexus_V1_ExperienceState *state,
                                      Nexus_V1_Champion *champion,
                                      int champion_index,
                                      int damage_dealt)
{
    if (!state || !champion || champion_index < 0 ||
        champion_index >= NEXUS_MAX_CHAMPIONS || damage_dealt <= 0)
        return;

    switch (champion->primary_class) {
        case NEXUS_CLASS_FIGHTER:
            state->xp[champion_index].fighter_xp += damage_dealt;
            break;
        case NEXUS_CLASS_NINJA:
            state->xp[champion_index].ninja_xp += damage_dealt;
            break;
        default:
            state->xp[champion_index].fighter_xp += damage_dealt;
            break;
    }
}

void nexus_v1_experience_award_spell(Nexus_V1_ExperienceState *state,
                                     Nexus_V1_Champion *champion,
                                     int champion_index,
                                     int spell_power)
{
    if (!state || !champion || champion_index < 0 ||
        champion_index >= NEXUS_MAX_CHAMPIONS || spell_power <= 0)
        return;

    switch (champion->primary_class) {
        case NEXUS_CLASS_PRIEST:
            state->xp[champion_index].priest_xp += spell_power;
            break;
        case NEXUS_CLASS_WIZARD:
            state->xp[champion_index].wizard_xp += spell_power;
            break;
        default:
            state->xp[champion_index].wizard_xp += spell_power;
            break;
    }
}

void nexus_v1_experience_award_kill(Nexus_V1_ExperienceState *state,
                                    Nexus_V1_Champion *champion,
                                    int champion_index,
                                    int creature_xp_value)
{
    if (!state || !champion || champion_index < 0 ||
        champion_index >= NEXUS_MAX_CHAMPIONS || creature_xp_value <= 0)
        return;
    state->xp[champion_index].fighter_xp += creature_xp_value;
    state->xp[champion_index].ninja_xp += creature_xp_value / 2;
}

int nexus_v1_experience_check_levelup(Nexus_V1_ExperienceState *state,
                                      Nexus_V1_Champion *champion,
                                      int champion_index)
{
    int new_level, leveled_up = 0;
    Nexus_V1_ChampionXP *xp;

    if (!state || !champion || champion_index < 0 ||
        champion_index >= NEXUS_MAX_CHAMPIONS)
        return 0;

    xp = &state->xp[champion_index];

    new_level = nexus_v1_experience_level_for_xp(xp->fighter_xp);
    if (new_level > champion->fighter_level) {
        champion->strength += NEXUS_STAT_GAIN_PER_LEVEL *
                              (new_level - champion->fighter_level);
        champion->max_health += 5 * (new_level - champion->fighter_level);
        champion->fighter_level = new_level;
        leveled_up = 1;
    }

    new_level = nexus_v1_experience_level_for_xp(xp->ninja_xp);
    if (new_level > champion->ninja_level) {
        champion->dexterity += NEXUS_STAT_GAIN_PER_LEVEL *
                               (new_level - champion->ninja_level);
        champion->max_stamina += 5 * (new_level - champion->ninja_level);
        champion->ninja_level = new_level;
        leveled_up = 1;
    }

    new_level = nexus_v1_experience_level_for_xp(xp->priest_xp);
    if (new_level > champion->priest_level) {
        champion->wisdom += NEXUS_STAT_GAIN_PER_LEVEL *
                            (new_level - champion->priest_level);
        champion->max_mana += 5 * (new_level - champion->priest_level);
        champion->priest_level = new_level;
        leveled_up = 1;
    }

    new_level = nexus_v1_experience_level_for_xp(xp->wizard_xp);
    if (new_level > champion->wizard_level) {
        champion->wisdom += NEXUS_STAT_GAIN_PER_LEVEL *
                            (new_level - champion->wizard_level);
        champion->max_mana += 5 * (new_level - champion->wizard_level);
        champion->wizard_level = new_level;
        leveled_up = 1;
    }

    return leveled_up;
}
