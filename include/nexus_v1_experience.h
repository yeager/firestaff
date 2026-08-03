
#ifndef NEXUS_V1_EXPERIENCE_H
#define NEXUS_V1_EXPERIENCE_H

#include "nexus_v1_champions.h"

#define NEXUS_XP_PER_LEVEL         500
#define NEXUS_MAX_CLASS_LEVEL       16
#define NEXUS_STAT_GAIN_PER_LEVEL    2

typedef struct {
    int fighter_xp;
    int ninja_xp;
    int priest_xp;
    int wizard_xp;
} Nexus_V1_ChampionXP;

typedef struct {
    Nexus_V1_ChampionXP xp[NEXUS_MAX_CHAMPIONS];
} Nexus_V1_ExperienceState;

void nexus_v1_experience_init(Nexus_V1_ExperienceState *state);

void nexus_v1_experience_award_combat(Nexus_V1_ExperienceState *state,
                                      Nexus_V1_Champion *champion,
                                      int champion_index,
                                      int damage_dealt);

void nexus_v1_experience_award_spell(Nexus_V1_ExperienceState *state,
                                     Nexus_V1_Champion *champion,
                                     int champion_index,
                                     int spell_power);

void nexus_v1_experience_award_kill(Nexus_V1_ExperienceState *state,
                                    Nexus_V1_Champion *champion,
                                    int champion_index,
                                    int creature_xp_value);

int nexus_v1_experience_level_for_xp(int xp);

int nexus_v1_experience_check_levelup(Nexus_V1_ExperienceState *state,
                                      Nexus_V1_Champion *champion,
                                      int champion_index);

#endif
