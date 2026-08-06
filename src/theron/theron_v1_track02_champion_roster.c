#include "theron_v1_track02_champion_roster.h"
#include "theron_v1_champions.h"

#if defined(FIRESTAFF_THERON_PRODUCTION)
/* The authenticated US BIN has no proven ASCII champion-name/title
 * consumer. Keep source-bound numeric records available to the forcefield
 * handoff, but never publish the DMWeb/fixture labels through production. */
#define TQR_NAME(value)  NULL
#define TQR_TITLE(value) NULL
#else
#define TQR_NAME(value)  value
#define TQR_TITLE(value) value
#endif

/* Source: authenticated Track 02 record extraction (US MD5
 * f23601102138f87c33025877767ebf76).
 * The old UD 0x09D1D6 comment was not a valid US roster locator: that
 * window is executable code in the supplied US BIN. Keep the cross-checked
 * numeric record table for explicit fixture/probe work and the bounded
 * forcefield record handoff, but do not treat its labels as a US text
 * consumer. Production compiles those labels out above.
 * Base stats verified against Track 02 binary extraction.
 * Luck, skill sub-levels, and starting equipment from DMWeb encyclopaedia
 * (dmweb.free.fr/?q=node/201), cross-validated: base stats match Track 02
 * extraction exactly for all 8 champions.
 * All 8 appear in all 7 dungeons except DOTAN absent from Dungeon 1.
 *
 * Starting equipment: track02 item indices (theron_v1_track02_item_names.h).
 * Equip slots use Theron_EquipSlot enum; -1 = inventory only. */

/* Track 02 item indices for readability */
#define TI_COMPASS         0
#define TI_ILLUMULET        1
#define TI_DAGGER           4
#define TI_DELTA            5
#define TI_AXE              8
#define TI_MORNINGSTAR      9
#define TI_CROSSBOW        10
#define TI_SLAYER          11
#define TI_THROWING_STAR   13
#define TI_THE_CONDUIT     15
#define TI_SCEPTRE_OF_LYF  16
#define TI_FINE_ROBE       17
#define TI_KIRTLE          18
#define TI_SILK_SHIRT      19
#define TI_LEATHER_JERKIN  20
#define TI_TUNIC           21
#define TI_MAIL_AKETON     22
#define TI_BARBARIAN_HIDE  24
#define TI_TABARD          25
#define TI_GUNNA           26
#define TI_LEATHER_PANTS   27
#define TI_BLUE_PANTS      28
#define TI_GHI_TROUSERS    29
#define TI_LEG_MAIL        30
#define TI_HELMET          32
#define TI_BASINET         33
#define TI_SMALL_SHIELD    34
#define TI_SANDALS         35
#define TI_SUEDE_BOOTS     36
#define TI_LEATHER_BOOTS   37
#define TI_HOSEN           38
#define TI_ROPE            41

/* Shorthand for equip slots */
#define ES_W  THERON_ESLOT_WEAPON
#define ES_A  THERON_ESLOT_ARMOR
#define ES_S  THERON_ESLOT_SHIELD
#define ES_H  THERON_ESLOT_HELM
#define ES_B  THERON_ESLOT_BOOTS
#define ES_AM THERON_ESLOT_AMULET
#define ES_I  (-1)  /* inventory, not equipped */

static const Theron_ChampionRecord g_roster[THERON_TRACK02_CHAMPION_COUNT] = {
    /* 0: THERON — UD 0x09D1D6, protagonist (always in party) */
    { TQR_NAME("THERON"), NULL, 'M',
      175, 1500, 50, 80, 50, 40, 40, 45, 40, 45,
      .fighter_skills = {3, 0, 0, 0},
      .ninja_skills   = {0, 0, 0, 3},
      .priest_skills  = {0, 3, 0, 0},
      .wizard_skills  = {0, 0, 0, 3},
      .start_equip_slot  = { ES_A, ES_I, ES_B, -1,-1,-1,-1,-1,-1,-1,-1,-1 },
      .start_equip_item  = { TI_LEATHER_JERKIN, TI_GHI_TROUSERS, TI_LEATHER_BOOTS, -1,-1,-1,-1,-1,-1,-1,-1,-1 },
      .start_equip_count = 3,
    },

    /* 1: MARA — UD 0x09D1FE, Priest/Wizard specialist */
    { TQR_NAME("MARA"), TQR_TITLE("GUARDIAN OF WISDOM"), 'F',
      250, 1500, 200, 60, 30, 45, 70, 40, 40, 30,
      .fighter_skills = {0, 0, 0, 0},
      .ninja_skills   = {0, 0, 0, 0},
      .priest_skills  = {5, 8, 6, 6},
      .wizard_skills  = {3, 3, 7, 8},
      .start_equip_slot  = { ES_A, ES_I, ES_B, ES_W, -1,-1,-1,-1,-1,-1,-1,-1 },
      .start_equip_item  = { TI_KIRTLE, TI_GUNNA, TI_LEATHER_BOOTS, TI_SCEPTRE_OF_LYF, -1,-1,-1,-1,-1,-1,-1,-1 },
      .start_equip_count = 4,
    },

    /* 2: LINOS — UD 0x09D22E, all-round Ninja/Priest */
    { TQR_NAME("LINOS"), TQR_TITLE("THE RESOLUTE"), 'M',
      300, 3000, 100, 30, 55, 40, 45, 70, 60, 55,
      .fighter_skills = {4, 3, 4, 5},
      .ninja_skills   = {3, 6, 7, 9},
      .priest_skills  = {4, 5, 9, 4},
      .wizard_skills  = {5, 3, 4, 3},
      .start_equip_slot  = { ES_A, ES_I, ES_B, ES_W, ES_I, ES_I, ES_I, ES_I, ES_I, ES_I, -1,-1 },
      .start_equip_item  = { TI_LEATHER_JERKIN, TI_BLUE_PANTS, TI_LEATHER_BOOTS, TI_CROSSBOW, TI_SLAYER, TI_SLAYER, TI_SLAYER, TI_SLAYER, TI_SLAYER, TI_COMPASS, -1,-1 },
      .start_equip_count = 10,
    },

    /* 3: HEXA — UD 0x09D25A, perfectly balanced */
    { TQR_NAME("HEXA"), TQR_TITLE("LORD OF FEALTY"), 'M',
      350, 2500, 150, 50, 50, 50, 50, 50, 50, 50,
      .fighter_skills = {4, 4, 4, 4},
      .ninja_skills   = {4, 4, 4, 4},
      .priest_skills  = {4, 4, 4, 4},
      .wizard_skills  = {4, 4, 4, 4},
      .start_equip_slot  = { ES_H, ES_A, ES_I, ES_B, ES_W, ES_S, -1,-1,-1,-1,-1,-1 },
      .start_equip_item  = { TI_HELMET, TI_TUNIC, TI_LEATHER_PANTS, TI_LEATHER_BOOTS, TI_DELTA, TI_SMALL_SHIELD, -1,-1,-1,-1,-1,-1 },
      .start_equip_count = 6,
    },

    /* 4: HAKAR — UD 0x09D288, Master Fighter */
    { TQR_NAME("HAKAR"), TQR_TITLE("THE BRAVE"), 'M',
      400, 2000, 50, 45, 60, 35, 40, 45, 70, 60,
      .fighter_skills = {7, 5, 9, 5},
      .ninja_skills   = {0, 8, 6, 6},
      .priest_skills  = {0, 3, 0, 0},
      .wizard_skills  = {3, 0, 0, 0},
      .start_equip_slot  = { ES_A, ES_B, ES_W, -1,-1,-1,-1,-1,-1,-1,-1,-1 },
      .start_equip_item  = { TI_BARBARIAN_HIDE, TI_SANDALS, TI_AXE, -1,-1,-1,-1,-1,-1,-1,-1,-1 },
      .start_equip_count = 3,
    },

    /* 5: TIRAN — UD 0x09D2B2, pure Fighter */
    { TQR_NAME("TIRAN"), TQR_TITLE("KNIGHT OF STRENGTH"), 'M',
      450, 2750, 0, 40, 70, 30, 35, 55, 45, 45,
      .fighter_skills = {9, 8, 4, 7},
      .ninja_skills   = {0, 0, 0, 0},
      .priest_skills  = {0, 0, 0, 0},
      .wizard_skills  = {0, 0, 0, 0},
      .start_equip_slot  = { ES_H, ES_A, ES_I, ES_B, ES_W, -1,-1,-1,-1,-1,-1,-1 },
      .start_equip_item  = { TI_BASINET, TI_MAIL_AKETON, TI_LEG_MAIL, TI_HOSEN, TI_MORNINGSTAR, -1,-1,-1,-1,-1,-1,-1 },
      .start_equip_count = 5,
    },

    /* 6: DOTAN — UD 0x09D2E4, Ninja/Wizard (absent from Dungeon 1) */
    { TQR_NAME("DOTAN"), TQR_TITLE("MASTER OF THE WIND"), 'M',
      200, 1000, 180, 55, 35, 70, 60, 20, 55, 40,
      .fighter_skills = {0, 0, 0, 7},
      .ninja_skills   = {8, 7, 6, 6},
      .priest_skills  = {0, 0, 0, 0},
      .wizard_skills  = {7, 8, 3, 3},
      .start_equip_slot  = { ES_A, ES_I, ES_W, -1,-1,-1,-1,-1,-1,-1,-1,-1 },
      .start_equip_item  = { TI_FINE_ROBE, TI_FINE_ROBE, TI_THE_CONDUIT, -1,-1,-1,-1,-1,-1,-1,-1,-1 },
      .start_equip_count = 3,
    },

    /* 7: PENTAI — UD 0x09D314, survivalist */
    { TQR_NAME("PENTAI"), TQR_TITLE("THE SURVIVOR"), 'F',
      550, 2250, 120, 70, 40, 20, 30, 60, 30, 70,
      .fighter_skills = {0, 0, 0, 6},
      .ninja_skills   = {7, 5, 4, 3},
      .priest_skills  = {0, 6, 0, 7},
      .wizard_skills  = {0, 0, 0, 5},
      .start_equip_slot  = { ES_AM, ES_A, ES_I, ES_B, ES_I, ES_I, ES_I, ES_I, ES_W, ES_I, -1,-1 },
      .start_equip_item  = { TI_ILLUMULET, TI_SILK_SHIRT, TI_TABARD, TI_SUEDE_BOOTS, TI_THROWING_STAR, TI_THROWING_STAR, TI_THROWING_STAR, TI_THROWING_STAR, TI_DAGGER, TI_ROPE, -1,-1 },
      .start_equip_count = 10,
    },
};

const Theron_ChampionRecord *theron_v1_track02_us_champion(unsigned int index) {
    if (index >= THERON_TRACK02_CHAMPION_COUNT) return NULL;
    return &g_roster[index];
}

size_t theron_v1_track02_us_champion_count(void) {
    return THERON_TRACK02_CHAMPION_COUNT;
}
