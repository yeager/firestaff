
#ifndef NEXUS_V1_CHAMPIONS_H
#define NEXUS_V1_CHAMPIONS_H
#include <stdint.h>

/* DM Nexus has the same champion system as DM1 but with Japanese names.
 * 24 champions in the Hall of Champions, 4 active in party.
 * Stats, skills, spells identical to DM1 engine. */

#define NEXUS_MAX_CHAMPIONS 24
#define NEXUS_MAX_PARTY 4

typedef enum {
    NEXUS_CLASS_FIGHTER = 0,
    NEXUS_CLASS_NINJA,
    NEXUS_CLASS_PRIEST,
    NEXUS_CLASS_WIZARD,
    NEXUS_CLASS_COUNT
} Nexus_ChampionClass;

typedef struct {
    char name_ascii[32];      /* ASCII romanized name */
    char name_jp[64];         /* UTF-8 Japanese name */
    Nexus_ChampionClass primary_class;
    int health, max_health;
    int stamina, max_stamina;
    int mana, max_mana;
    int strength, dexterity, wisdom, vitality, anti_magic, anti_fire;
    int fighter_level, ninja_level, priest_level, wizard_level;
    int food, water;
    int alive;
    int portrait_index;       /* CG texture index for portrait */
    uint8_t inventory[30];    /* item indices */
} Nexus_V1_Champion;

typedef struct {
    Nexus_V1_Champion champions[NEXUS_MAX_CHAMPIONS];
    int champion_count;
    int party[NEXUS_MAX_PARTY];
    int party_count;
    int leader_index;
} Nexus_V1_ChampionPool;

void nexus_v1_champions_init(Nexus_V1_ChampionPool *pool);
int nexus_v1_champion_recruit(Nexus_V1_ChampionPool *pool, int mirror_index);
int nexus_v1_champion_resurrect(Nexus_V1_ChampionPool *pool, int party_slot);

#endif

