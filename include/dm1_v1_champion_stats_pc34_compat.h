#ifndef FIRESTAFF_DM1_V1_CHAMPION_STATS_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHAMPION_STATS_PC34_COMPAT_H

#include <stddef.h>
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

enum {
    DM1_LOAD_COLOR_RED = 8,
    DM1_LOAD_COLOR_YELLOW = 11,
    DM1_LOAD_COLOR_LIGHTEST_GRAY = 13
};

enum {
    DM1_STAT_COLOR_LIGHT_GREEN = 7,
    DM1_STAT_COLOR_RED = 8,
    DM1_STAT_COLOR_LIGHTEST_GRAY = 13
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
} DM1_V1_ChampionStatsPc34;

#define DM1_V1_MAX_CHAMPIONS_PC34 4

typedef struct {
    DM1_V1_ChampionStatsPc34 champions[DM1_V1_MAX_CHAMPIONS_PC34];
    int count;
    int leader;
    int staminaTick;
} DM1_V1_ChampionStatsStatePc34;

void DM1_V1_ChampionStats_InitPc34Compat(DM1_V1_ChampionStatsStatePc34* s);
int DM1_V1_ChampionStats_AddChampionPc34Compat(DM1_V1_ChampionStatsStatePc34* s, const char* name);
int DM1_V1_ChampionStats_GetPc34Compat(const DM1_V1_ChampionStatsStatePc34* s, int champ, int stat);
void DM1_V1_ChampionStats_SetPc34Compat(DM1_V1_ChampionStatsStatePc34* s, int champ, int stat, int val);
void DM1_V1_ChampionStats_ModifyPc34Compat(DM1_V1_ChampionStatsStatePc34* s, int champ, int stat, int delta);
void DM1_V1_ChampionStats_TickPc34Compat(DM1_V1_ChampionStatsStatePc34* s);
int DM1_V1_ChampionStats_IsAlivePc34Compat(const DM1_V1_ChampionStatsStatePc34* s, int champ);
void DM1_V1_ChampionStats_KillPc34Compat(DM1_V1_ChampionStatsStatePc34* s, int champ);
void DM1_V1_ChampionStats_ResurrectPc34Compat(DM1_V1_ChampionStatsStatePc34* s, int champ, int hp);
const char* DM1_V1_ChampionStats_StatNamePc34Compat(int stat);
const char* DM1_V1_ChampionStats_SkillNamePc34Compat(int skill);
int DM1_V1_ChampionStats_StaminaAdjustedValuePc34Compat(int currentStamina,
                                          int maximumStamina,
                                          int value);
int DM1_V1_ChampionStats_StatisticColorPc34Compat(int currentValue, int maximumValue);
int DM1_V1_ChampionStats_ChampionStatisticColorPc34Compat(const DM1_V1_ChampionStatsPc34* champion, int stat);
int DM1_V1_ChampionStats_MaximumLoadPc34Compat(const DM1_V1_ChampionStatsPc34* champion);
int DM1_V1_ChampionStats_MovementTicksPc34Compat(const DM1_V1_ChampionStatsPc34* champion);
int DM1_V1_ChampionStats_MovementStaminaCostPc34Compat(const DM1_V1_ChampionStatsPc34* champion);
int DM1_V1_ChampionStats_LoadColorPc34Compat(const DM1_V1_ChampionStatsPc34* champion);
int DM1_V1_ChampionStats_FormatLoadPc34Compat(const DM1_V1_ChampionStatsPc34* champion,
                               char* out,
                               size_t outSize);

#define m11_stat_name DM1_V1_ChampionStats_StatNamePc34Compat
#define m11_skill_name DM1_V1_ChampionStats_SkillNamePc34Compat
#define dm1_stats_stamina_adjusted_value_pc34 DM1_V1_ChampionStats_StaminaAdjustedValuePc34Compat

#ifdef __cplusplus
}
#endif

#endif // FIRESTAFF_DM1_V1_CHAMPION_STATS_PC34_COMPAT_H
