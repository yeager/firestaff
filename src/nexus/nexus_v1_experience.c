
#include "nexus_v1_experience.h"
#include <string.h>

void nexus_v1_experience_init(Nexus_V1_ExperienceState *state)
{
    if (!state) return;
    memset(state, 0, sizeof(*state));
}

int nexus_v1_experience_level_for_xp(int xp)
{
    int level;
    if (xp < 0) return 0;
    level = xp / NEXUS_XP_PER_LEVEL;
    if (level > NEXUS_MAX_CLASS_LEVEL) level = NEXUS_MAX_CLASS_LEVEL;
    return level;
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
