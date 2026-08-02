#ifndef THERON_V1_TRACK02_CHAMPION_ROSTER_H
#define THERON_V1_TRACK02_CHAMPION_ROSTER_H

#include <stddef.h>
#include <stdint.h>

#define THERON_TRACK02_CHAMPION_COUNT 8u

/* DM1 skill level tiers (Neophyte=0 .. Arch Master=15) */
#define THERON_SKILL_NONE        0u
#define THERON_SKILL_NEOPHYTE    0u
#define THERON_SKILL_NOVICE      1u
#define THERON_SKILL_APPRENTICE  2u
#define THERON_SKILL_JOURNEYMAN  3u
#define THERON_SKILL_ARTISAN     4u
#define THERON_SKILL_ADEPT       5u
#define THERON_SKILL_EXPERT      6u
#define THERON_SKILL_MASTER      7u

typedef struct {
    const char *name;
    const char *title;
    char sex;         /* 'M' or 'F' */
    uint16_t hp;
    uint16_t stamina;
    uint16_t mana;
    uint8_t  luck;
    uint8_t  strength;
    uint8_t  dexterity;
    uint8_t  wisdom;
    uint8_t  vitality;
    uint8_t  anti_magic;
    uint8_t  anti_fire;
    /* Skill sub-levels: Fighter(Swing,Thrust,Club,Parry),
     * Ninja(Steal,Fight,Throw,Shoot), Priest(Identify,Heal,Influence,Defend),
     * Wizard(Fire,Air,Earth,Water). Source: DMWeb encyclopaedia. */
    uint8_t  fighter_skills[4]; /* Swing, Thrust, Club, Parry */
    uint8_t  ninja_skills[4];  /* Steal, Fight, Throw, Shoot */
    uint8_t  priest_skills[4]; /* Identify, Heal, Influence, Defend */
    uint8_t  wizard_skills[4]; /* Fire, Air, Earth, Water */
    /* Starting equipment: track02 item indices + equip slot assignments.
     * Source: DMWeb encyclopaedia (dmweb.free.fr/?q=node/201).
     * -1 terminates the list. */
    int8_t   start_equip_slot[12];  /* equip slot (Theron_EquipSlot) or -1 for inventory */
    int8_t   start_equip_item[12];  /* track02 item index, -1 = end sentinel */
    uint8_t  start_equip_count;     /* number of valid entries */
} Theron_ChampionRecord;

const Theron_ChampionRecord *theron_v1_track02_us_champion(unsigned int index);
size_t theron_v1_track02_us_champion_count(void);

#endif /* THERON_V1_TRACK02_CHAMPION_ROSTER_H */
