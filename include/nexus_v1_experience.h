
#ifndef NEXUS_V1_EXPERIENCE_H
#define NEXUS_V1_EXPERIENCE_H

#include "nexus_v1_champions.h"

/* DM.BIN 0x029FFE: level = halving loop over XP with threshold 500.
 * level=1; while(xp>=500) { xp>>=1; level++ }
 * Thresholds: 500, 1000, 2000, 4000, 8000, 16000, 32000.
 * Stat cap per level: 500 (DM.BIN 0x029FF6: 0x01F4).
 * XP cap: 32000 (DM.BIN 0x02A114: 0x7D00).
 * Champion struct size: 316 bytes (DM.BIN 0x02A038).
 * Stat entries at struct offset +112, 8 bytes each. */
#define NEXUS_XP_LEVEL_THRESHOLD   500  /* DM.BIN 0x029FF6: halving threshold */
#define NEXUS_STAT_CAP_PER_LEVEL   500  /* DM.BIN 0x029FF6: 0x01F4 */
#define NEXUS_STAT_XP_CAP        32000  /* DM.BIN 0x02A114: 0x7D00 */
#define NEXUS_CHAMPION_STRUCT_SIZE 316  /* DM.BIN 0x02A038: 0x013C */
#define NEXUS_STAT_ENTRY_OFFSET    112  /* DM.BIN 0x029FB2: ADD #112 */
#define NEXUS_STAT_ENTRY_SIZE        8  /* DM.BIN 0x029FB6: SHLL2+SHLL */

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

/* DM.BIN 0x3B5DC: class level from XP (linear thresholds 10240/level). */
int nexus_v1_experience_level_for_xp(int xp);

/* DM.BIN 0x029FFE: stat level via halving loop (threshold 500).
 * level=1; while(xp>=500) { xp>>=1; level++ } */
int nexus_v1_stat_level_for_xp(int xp);

/* DM.BIN 0x029F76: skill level from raw XP index (0-31) and class (0-3). */
int nexus_v1_skill_level_for_index(int class_index, int xp_index);

/* DM.BIN 0x03B8CC: initial stat value for champion rank (0-14). */
int nexus_v1_stat_init_value(int rank);

int nexus_v1_experience_check_levelup(Nexus_V1_ExperienceState *state,
                                      Nexus_V1_Champion *champion,
                                      int champion_index);

#endif
