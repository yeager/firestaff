/* Nexus V1 production experience boundary.
 *
 * nexus_v1_experience.c contains a DM.BIN-shaped XP/level model, but the
 * supplied Saturn corpus does not bind the actor-death event, class-XP
 * producer, champion writeback or HUD level-up consumer. Keep the ABI
 * available to the engine while every production operation remains
 * state-preserving until an original execution trace admits it.
 */

#include "nexus_v1_experience.h"

void nexus_v1_experience_init(Nexus_V1_ExperienceState *state)
{
    (void)state;
}

void nexus_v1_experience_award_combat(Nexus_V1_ExperienceState *state,
                                      Nexus_V1_Champion *champion,
                                      int champion_index,
                                      int damage_dealt)
{
    (void)state; (void)champion; (void)champion_index; (void)damage_dealt;
}

void nexus_v1_experience_award_spell(Nexus_V1_ExperienceState *state,
                                     Nexus_V1_Champion *champion,
                                     int champion_index,
                                     int spell_power)
{
    (void)state; (void)champion; (void)champion_index; (void)spell_power;
}

void nexus_v1_experience_award_kill(Nexus_V1_ExperienceState *state,
                                    Nexus_V1_Champion *champion,
                                    int champion_index,
                                    int creature_xp_value)
{
    (void)state; (void)champion; (void)champion_index;
    (void)creature_xp_value;
}

int nexus_v1_experience_level_for_xp(int xp)
{
    (void)xp;
    return 0;
}

int nexus_v1_stat_level_for_xp(int xp)
{
    (void)xp;
    return 0;
}

int nexus_v1_skill_level_for_index(int class_index, int xp_index)
{
    (void)class_index; (void)xp_index;
    return 0;
}

int nexus_v1_stat_init_value(int rank)
{
    (void)rank;
    return 0;
}

int nexus_v1_experience_check_levelup(Nexus_V1_ExperienceState *state,
                                      Nexus_V1_Champion *champion,
                                      int champion_index)
{
    (void)state; (void)champion; (void)champion_index;
    return 0;
}
