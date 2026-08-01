#include "theron_v1_track02_dm1_item_names.h"

/* Source: US Track 02 BIN, user-data offset 0x1D9737.
 * DM1-compatible item ID namespace — includes full plate armor set
 * (TORSO PLATE, LEG PLATE, ARMET, FOOT PLATE), TORCH, EYE OF TIME,
 * FURY, GOLD COIN, RUBY KEY. Does NOT include TQ-unique items
 * (see theron_v1_track02_item_names.h for those).
 * Hash: f23601102138f87c33025877767ebf76 (MD5). */
static const char *const g_dm1_item_names[THERON_TRACK02_DM1_ITEM_NAME_COUNT] = {
    "COMPASS",         /*  0 */
    "TORCH",           /*  1 */
    "ILLUMULET",       /*  2 */
    "EYE OF TIME",     /*  3 */
    "FURY",            /*  4 */
    "OPEN SCROLL",     /*  5 */
    "SCROLL",          /*  6 */
    "DAGGER",          /*  7 */
    "DELTA",           /*  8 */
    "AXE",             /*  9 */
    "MORNINGSTAR",     /* 10 */
    "CROSSBOW",        /* 11 */
    "SLAYER",          /* 12 */
    "THROWING STAR",   /* 13 */
    "THE CONDUIT",     /* 14 */
    "SCEPTRE OF LYF",  /* 15 */
    "FINE ROBE",       /* 16 */
    "KIRTLE",          /* 17 */
    "SILK SHIRT",      /* 18 */
    "LEATHER JERKIN",  /* 19 */
    "TUNIC",           /* 20 */
    "MAIL AKETON",     /* 21 */
    "TORSO PLATE",     /* 22 */
    "BARBARIAN HIDE",  /* 23 */
    "TABARD",          /* 24 */
    "GUNNA",           /* 25 */
    "LEATHER PANTS",   /* 26 */
    "BLUE PANTS",      /* 27 */
    "GHI TROUSERS",    /* 28 */
    "LEG MAIL",        /* 29 */
    "LEG PLATE",       /* 30 */
    "HELMET",          /* 31 */
    "BASINET",         /* 32 */
    "ARMET",           /* 33 */
    "SMALL SHIELD",    /* 34 */
    "SANDALS",         /* 35 */
    "SUEDE BOOTS",     /* 36 */
    "LEATHER BOOTS",   /* 37 */
    "HOSEN",           /* 38 */
    "FOOT PLATE",      /* 39 */
    "GOLD COIN",       /* 40 */
    "MAGICAL BOX",     /* 41 */
    "ROPE",            /* 42 */
    "TAZAHELM",        /* 43 */
    "BONES",           /* 44 */
    "VEN POTION",      /* 45 */
    "SAR POTION",      /* 46 */
    "ZO POTION",       /* 47 */
    "ROS POTION",      /* 48 */
    "KU POTION",       /* 49 */
    "DANE POTION",     /* 50 */
    "NETA POTION",     /* 51 */
    "BRO POTION",      /* 52 */
    "MA POTION",       /* 53 */
    "YA POTION",       /* 54 */
    "EE POTION",       /* 55 */
    "VI POTION",       /* 56 */
    "WATER FLASK",     /* 57 */
    "FUL BOMB",        /* 58 */
    "APPLE",           /* 59 */
    "TOPAZ KEY",       /* 60 */
    "RUBY KEY",        /* 61 */
    "EMPTY FLASK",     /* 62 */
};

const char *theron_v1_track02_dm1_item_name(unsigned int index) {
    if (index >= THERON_TRACK02_DM1_ITEM_NAME_COUNT) return NULL;
    return g_dm1_item_names[index];
}

size_t theron_v1_track02_dm1_item_name_count(void) {
    return THERON_TRACK02_DM1_ITEM_NAME_COUNT;
}
