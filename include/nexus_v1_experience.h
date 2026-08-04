
#ifndef NEXUS_V1_EXPERIENCE_H
#define NEXUS_V1_EXPERIENCE_H

#include "nexus_v1_champions.h"

/* DM.BIN 0x0604B5DC: 6-byte xp_thresholds, each byte << 8 (SHLL8).
 * Stat cap per level: 500 (DM.BIN 0x029FF6).
 * Stat XP cap: 32000 (DM.BIN 0x02A114). */
#define NEXUS_MAX_CLASS_LEVEL        6
#define NEXUS_STAT_GAIN_PER_LEVEL    2
#define NEXUS_STAT_CAP_PER_LEVEL   500  /* DM.BIN 0x029FF6: 0x01F4 */
#define NEXUS_STAT_XP_CAP        32000  /* DM.BIN 0x02A114: 0x7D00 */

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

/* DM.BIN 0x029F76: skill level from raw XP index (0-31) and class (0-3). */
int nexus_v1_skill_level_for_index(int class_index, int xp_index);

/* DM.BIN 0x03B8CC: initial stat value for champion rank (0-14). */
int nexus_v1_stat_init_value(int rank);

int nexus_v1_experience_check_levelup(Nexus_V1_ExperienceState *state,
                                      Nexus_V1_Champion *champion,
                                      int champion_index);

#endif
