#include "theron_v1_track02_champion_roster.h"

/* Source: US Track 02 BIN (MD5 f23601102138f87c33025877767ebf76).
 * Champion records in DM1 5-bit packed text format at UD 0x09D1D6.
 * 8 champions, each with name/title/sex/stats in the standard DM1
 * dungeon data champion text format (name, title, sex, HP/STA/MANA,
 * 6 base attributes, skill levels, inventory).
 * All 8 appear in all 7 dungeons except DOTAN absent from Dungeon 1. */

static const Theron_ChampionRecord g_roster[THERON_TRACK02_CHAMPION_COUNT] = {
    /* 0: THERON — UD 0x09D1D6, protagonist (always in party) */
    { "THERON", NULL, 'M',
      175, 1500, 50, 50, 40, 40, 45, 40, 45 },

    /* 1: MARA — UD 0x09D1FE, wisdom/mana specialist */
    { "MARA", "GUARDIAN OF WISDOM", 'F',
      250, 1500, 200, 30, 45, 70, 40, 40, 30 },

    /* 2: LINOS — UD 0x09D22E, high vitality balanced */
    { "LINOS", "THE RESOLUTE", 'M',
      300, 3000, 100, 55, 40, 45, 70, 60, 55 },

    /* 3: HEXA — UD 0x09D25A, perfectly balanced */
    { "HEXA", "LORD OF FEALTY", 'M',
      350, 2500, 150, 50, 50, 50, 50, 50, 50 },

    /* 4: HAKAR — UD 0x09D288, anti-magic fighter */
    { "HAKAR", "THE BRAVE", 'M',
      400, 2000, 50, 60, 35, 40, 45, 70, 60 },

    /* 5: TIRAN — UD 0x09D2B2, pure strength, zero mana */
    { "TIRAN", "KNIGHT OF STRENGTH", 'M',
      450, 2750, 0, 70, 30, 35, 55, 45, 45 },

    /* 6: DOTAN — UD 0x09D2E4, dexterity/mana wizard (absent from Dungeon 1) */
    { "DOTAN", "MASTER OF THE WIND", 'M',
      200, 1000, 180, 35, 70, 60, 20, 55, 40 },

    /* 7: PENTAI — UD 0x09D314, high HP/anti-fire priest */
    { "PENTAI", "THE SURVIVOR", 'F',
      550, 2250, 120, 40, 20, 30, 60, 30, 70 },
};

const Theron_ChampionRecord *theron_v1_track02_us_champion(unsigned int index) {
    if (index >= THERON_TRACK02_CHAMPION_COUNT) return NULL;
    return &g_roster[index];
}

size_t theron_v1_track02_us_champion_count(void) {
    return THERON_TRACK02_CHAMPION_COUNT;
}
