#include "theron_v1_track02_champion_roster.h"

/* Source: US Track 02 BIN (MD5 f23601102138f87c33025877767ebf76).
 * Champion records at UD 0x09D1D6 in PCE-specific packed format.
 * Base stats verified against Track 02 binary extraction.
 * Skill sub-levels from DMWeb encyclopaedia (dmweb.free.fr/?q=node/201),
 * cross-validated: base stats match Track 02 extraction exactly for all 8.
 * All 8 appear in all 7 dungeons except DOTAN absent from Dungeon 1. */

static const Theron_ChampionRecord g_roster[THERON_TRACK02_CHAMPION_COUNT] = {
    /* 0: THERON — UD 0x09D1D6, protagonist (always in party) */
    { "THERON", NULL, 'M',
      175, 1500, 50, 50, 40, 40, 45, 40, 45,
      .fighter_skills = {3, 0, 0, 0},   /* Apprentice Fighter: Swing 3 */
      .ninja_skills   = {0, 0, 0, 3},   /* Apprentice Ninja: Shoot 3 */
      .priest_skills  = {0, 3, 0, 0},   /* Apprentice Priest: Heal 3 */
      .wizard_skills  = {0, 0, 0, 3},   /* Apprentice Wizard: Water 3 */
    },

    /* 1: MARA — UD 0x09D1FE, Priest/Wizard specialist */
    { "MARA", "GUARDIAN OF WISDOM", 'F',
      250, 1500, 200, 30, 45, 70, 40, 40, 30,
      .fighter_skills = {0, 0, 0, 0},
      .ninja_skills   = {0, 0, 0, 0},
      .priest_skills  = {5, 8, 6, 6},   /* Expert Priest */
      .wizard_skills  = {3, 3, 7, 8},   /* Expert Wizard */
    },

    /* 2: LINOS — UD 0x09D22E, all-round Ninja/Priest */
    { "LINOS", "THE RESOLUTE", 'M',
      300, 3000, 100, 55, 40, 45, 70, 60, 55,
      .fighter_skills = {4, 3, 4, 5},   /* Artisan Fighter */
      .ninja_skills   = {3, 6, 7, 9},   /* Master Ninja */
      .priest_skills  = {4, 5, 9, 4},   /* Master Priest */
      .wizard_skills  = {5, 3, 4, 3},   /* Artisan Wizard (highest sub=5) */
    },

    /* 3: HEXA — UD 0x09D25A, perfectly balanced */
    { "HEXA", "LORD OF FEALTY", 'M',
      350, 2500, 150, 50, 50, 50, 50, 50, 50,
      .fighter_skills = {4, 4, 4, 4},   /* Artisan Fighter */
      .ninja_skills   = {4, 4, 4, 4},   /* Artisan Ninja */
      .priest_skills  = {4, 4, 4, 4},   /* Artisan Priest */
      .wizard_skills  = {4, 4, 4, 4},   /* Artisan Wizard */
    },

    /* 4: HAKAR — UD 0x09D288, Master Fighter */
    { "HAKAR", "THE BRAVE", 'M',
      400, 2000, 50, 60, 35, 40, 45, 70, 60,
      .fighter_skills = {7, 5, 9, 5},   /* Master Fighter: Club 9 */
      .ninja_skills   = {0, 8, 6, 6},   /* Expert Ninja: Fight 8 */
      .priest_skills  = {0, 3, 0, 0},   /* Apprentice Priest */
      .wizard_skills  = {3, 0, 0, 0},   /* Apprentice Wizard: Fire 3 */
    },

    /* 5: TIRAN — UD 0x09D2B2, pure Fighter */
    { "TIRAN", "KNIGHT OF STRENGTH", 'M',
      450, 2750, 0, 70, 30, 35, 55, 45, 45,
      .fighter_skills = {9, 8, 4, 7},   /* Master Fighter: Swing 9, Thrust 8 */
      .ninja_skills   = {0, 0, 0, 0},
      .priest_skills  = {0, 0, 0, 0},
      .wizard_skills  = {0, 0, 0, 0},
    },

    /* 6: DOTAN — UD 0x09D2E4, Ninja/Wizard (absent from Dungeon 1) */
    { "DOTAN", "MASTER OF THE WIND", 'M',
      200, 1000, 180, 35, 70, 60, 20, 55, 40,
      .fighter_skills = {0, 0, 0, 7},   /* Adept Fighter: Parry 7 */
      .ninja_skills   = {8, 7, 6, 6},   /* Master Ninja: Steal 8 */
      .priest_skills  = {0, 0, 0, 0},
      .wizard_skills  = {7, 8, 3, 3},   /* Expert Wizard: Air 8 */
    },

    /* 7: PENTAI — UD 0x09D314, survivalist */
    { "PENTAI", "THE SURVIVOR", 'F',
      550, 2250, 120, 40, 20, 30, 60, 30, 70,
      .fighter_skills = {0, 0, 0, 6},   /* Artisan Fighter: Parry 6 */
      .ninja_skills   = {7, 5, 4, 3},   /* Adept Ninja: Steal 7 */
      .priest_skills  = {0, 6, 0, 7},   /* Adept Priest: Defend 7 */
      .wizard_skills  = {0, 0, 0, 5},   /* Artisan Wizard: Water 5 (DMWeb: Craftsman tier) */
    },
};

const Theron_ChampionRecord *theron_v1_track02_us_champion(unsigned int index) {
    if (index >= THERON_TRACK02_CHAMPION_COUNT) return NULL;
    return &g_roster[index];
}

size_t theron_v1_track02_us_champion_count(void) {
    return THERON_TRACK02_CHAMPION_COUNT;
}
