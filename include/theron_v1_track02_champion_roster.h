#ifndef THERON_V1_TRACK02_CHAMPION_ROSTER_H
#define THERON_V1_TRACK02_CHAMPION_ROSTER_H

#include <stddef.h>
#include <stdint.h>

#define THERON_TRACK02_CHAMPION_COUNT 8u

typedef struct {
    const char *name;
    const char *title;
    char sex;         /* 'M' or 'F' */
    uint16_t hp;
    uint16_t stamina;
    uint16_t mana;
    uint8_t  strength;
    uint8_t  dexterity;
    uint8_t  wisdom;
    uint8_t  vitality;
    uint8_t  anti_magic;
    uint8_t  anti_fire;
} Theron_ChampionRecord;

const Theron_ChampionRecord *theron_v1_track02_us_champion(unsigned int index);
size_t theron_v1_track02_us_champion_count(void);

#endif /* THERON_V1_TRACK02_CHAMPION_ROSTER_H */
