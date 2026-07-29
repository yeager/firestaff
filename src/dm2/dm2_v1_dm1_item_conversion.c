#include "dm2_v1_dm1_item_conversion.h"
#include <string.h>

#define W DM2_V1_DB_CATEGORY_WEAPON
#define C DM2_V1_DB_CATEGORY_CLOTHING
#define S DM2_V1_DB_CATEGORY_SCROLL
#define P DM2_V1_DB_CATEGORY_POTION
#define T DM2_V1_DB_CATEGORY_CONTAINER
#define M DM2_V1_DB_CATEGORY_MISC_ITEM

const DM2_V1_DM1ItemConvEntry dm2_v1_dm1_item_conv_table[DM2_V1_DM1_ITEM_CONV_TABLE_SIZE] = {
    /* 0-3: COMPASS */
    {M, 5}, {M, 5}, {M, 5}, {M, 5},
    /* 4-7: TORCH */
    {W, 2}, {W, 2}, {W, 2}, {W, 2},
    /* 8-13: 2-STATE MISC */
    {M, 1}, {M, 1}, {M, 2}, {M, 2}, {M, 3}, {M, 3},
    /* 14-29: 2-STATE WEAPON */
    {W, 3}, {W, 3},   /* FLAMITT */
    {W, 0}, {W, 0},   /* EYE OF TIME */
    {W, 1}, {W, 1},   /* STORMRING */
    {W, 4}, {W, 4}, {W, 4},  /* STAFF OF CLAWS */
    {W, 5}, {W, 5},   /* BOLT BLADE */
    {W, 6}, {W, 6},   /* FURY */
    {W,45},            /* THE FIRESTAFF */
    {W, 7}, {W, 7},   /* THE FIRESTAFF */
    /* 30-31: SCROLLS */
    {S, 0}, {S, 0},
    /* 32-66: WEAPONS (DAGGER+) */
    {W, 8}, {W, 9}, {W,10}, {W,11}, {W,12}, {W,13}, {W,14}, {W,15},
    {W,16}, {W,17}, {W,18}, {W,19}, {W,20}, {W,21}, {W,22}, {W,23},
    {W,24}, {W,25}, {W,26}, {W,27}, {W,28}, {W,29}, {W,30}, {W,31},
    {W,32}, {W,33}, {W,34}, {W,35}, {W,36}, {W,37}, {W,38}, {W,39},
    {W,40}, {W,41}, {W,42},
    /* 67-119: CLOTHINGS */
    {C, 5}, {C, 7}, {C, 9}, {C,10}, {C,13}, {C,16}, {C,20}, {C,21},
    {C,32}, {C,34}, {C,39}, {C,44}, {C,49}, {C, 0}, {C, 1}, {C, 2},
    {C, 6}, {C, 8}, {C,11}, {C,12}, {C,14}, {C,17}, {C,19}, {C,22},
    {C,33}, {C,35}, {C,40}, {C,45}, {C,50}, {C,25}, {C,26}, {C,27},
    {C,36}, {C,38}, {C,43}, {C,48}, {C,23}, {C,24}, {C,28}, {C,29},
    {C,31}, {C,30}, {C,42}, {C,47}, {C,52}, {C, 3}, {C,18}, {C, 4},
    {C,37}, {C,41}, {C,46}, {C,51}, {C,15},
    /* 120-134: MISCS */
    {M,37}, {M,38}, {M,39}, {M,40}, {M,41}, {M, 6}, {M, 7}, {M, 8},
    {M,25}, {M,26}, {M,27}, {M,28}, {M,42}, {M,43}, {M,44},
    /* 135: HORN OF FEAR (weapon, not misc) */
    {W,43},
    /* 136-139: more miscs */
    {M,45}, {M,46}, {M,47}, {M,48},
    /* 140-147: SPECIAL ITEMS */
    {C,53},  /* DEXHELM */
    {C,54},  /* FLAMEBAIN */
    {C,55},  /* POWERTOWERS */
    {W,44},  /* SPEEDBOW */
    {T, 0},  /* CHEST */
    {T, 0},  /* CHEST */
    {M, 4},  /* ASHES */
    {M,52},  /* BONES */
    /* 148-167: POTIONS */
    {P, 0}, {P, 1}, {P, 2}, {P, 3}, {P, 4}, {P, 5}, {P, 6}, {P, 7},
    {P, 8}, {P, 9}, {P,10}, {P,11}, {P,12}, {P,13}, {P,14}, {P,15},
    {P,16}, {P,17}, {P,18}, {P,19},
    /* 168-175: FOOD */
    {M,29}, {M,30}, {M,31}, {M,32}, {M,33}, {M,34}, {M,35}, {M,36},
    /* 176-191: KEYS */
    {M, 9}, {M,10}, {M,11}, {M,12}, {M,13}, {M,14}, {M,15}, {M,16},
    {M,17}, {M,18}, {M,19}, {M,20}, {M,21}, {M,22}, {M,23}, {M,24},
    /* 192-193: gap (zero-init) */
    {0, 0}, {0, 0},
    /* 194-198: LAST ITEMS */
    {C,56},  /* BOOTS OF SPEED */
    {P,20},  /* EMPTY FLASK */
    {C,57},  /* BOOTS OF SPEED */
    {M,51},  /* ZOKATHRA SPELL */
    {M,52},  /* BONES */
};

#undef W
#undef C
#undef S
#undef P
#undef T
#undef M

int dm2_v1_dm1_item_conv_lookup(int dm1_index, DM2_V1_DM1ItemConvReceipt *out)
{
    if (!out) return 0;
    memset(out, 0, sizeof(*out));

    if (dm1_index < 0 || dm1_index >= DM2_V1_DM1_ITEM_CONV_TABLE_SIZE)
        return 0;

    out->valid = 1;
    out->item_db = dm2_v1_dm1_item_conv_table[dm1_index].item_db;
    out->item_id = dm2_v1_dm1_item_conv_table[dm1_index].item_id;
    return 1;
}

const char *dm2_v1_dm1_item_conversion_source_evidence(void)
{
    return "skproject SKWIN/SkWinCore2.cpp EXTENDED_LOAD_DM1_ITEM_CONVERSION_LIST:71; "
           "199-entry DM1-to-DM2 item category+ID conversion table.";
}
