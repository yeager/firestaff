#ifndef FIRESTAFF_DM1_V1_CHAMPION_STATS_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHAMPION_STATS_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_STAT_HEALTH = 0,
    DM1_STAT_STAMINA,
    DM1_STAT_MANA,
    DM1_STAT_STRENGTH,
    DM1_STAT_DEXTERITY,
    DM1_STAT_WISDOM,
    DM1_STAT_VITALITY,
    DM1_STAT_ANTIFIRE,
    DM1_STAT_ANTIMAGIC,
    DM1_STAT_LUCK,
    DM1_STAT_COUNT
};

enum {
    DM1_SKILL_FIGHTER = 0,
    DM1_SKILL_NINJA,
    DM1_SKILL_PRIEST,
    DM1_SKILL_WIZARD,
    DM1_SKILL_COUNT
};

enum {
    DM1_WOUND_LEGS = 0x0010,
    DM1_WOUND_FEET = 0x0020
};

enum {
    DM1_ICON_ARMOUR_ELVEN_BOOTS = 119,
    DM1_ICON_ARMOUR_BOOT_OF_SPEED = 194
};

typedef struct {
    char name[16];
    int stats[DM1_STAT_COUNT];
    int maxStats[DM1_STAT_COUNT];
    int skills[DM1_SKILL_COUNT];
    int skillXP[DM1_SKILL_COUNT];
    int load;
    int wounds;
    int feetIconIndex;
    int level;
    int food;
    int water;
    int alive;
    int poisoned;
    int poisonAmount;
} M11_ChampionStats;

#define M11_MAX_CHAMPIONS 4

typedef struct {
    M11_ChampionStats champions[M11_MAX_CHAMPIONS];
    int count;
    int leader;
    int staminaTick;
} M11_ChampionStatsState;

void m11_stats_init(M11_ChampionStatsState* s);
int m11_stats_add_champion(M11_ChampionStatsState* s, const char* name);
int m11_stats_get(const M11_ChampionStatsState* s, int champ, int stat);
void m11_stats_set(M11_ChampionStatsState* s, int champ, int stat, int val);
void m11_stats_modify(M11_ChampionStatsState* s, int champ, int stat, int delta);
void m11_stats_tick(M11_ChampionStatsState* s);
int m11_stats_is_alive(const M11_ChampionStatsState* s, int champ);
void m11_stats_kill(M11_ChampionStatsState* s, int champ);
void m11_stats_resurrect(M11_ChampionStatsState* s, int champ, int hp);
const char* m11_stat_name(int stat);
const char* m11_skill_name(int skill);
int dm1_stats_stamina_adjusted_value_pc34(int currentStamina,
                                          int maximumStamina,
                                          int value);
int m11_stats_maximum_load_pc34(const M11_ChampionStats* champion);
int m11_stats_movement_ticks_pc34(const M11_ChampionStats* champion);
int m11_stats_movement_stamina_cost_pc34(const M11_ChampionStats* champion);

#ifdef __cplusplus
}
#endif

#endif // FIRESTAFF_DM1_V1_CHAMPION_STATS_PC34_COMPAT_H
