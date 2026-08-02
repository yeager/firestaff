#include "theron_v1_track02_full_item_names.h"

/* Source: US Track 02 BIN (MD5 f23601102138f87c33025877767ebf76).
 * UD offset 0x099517, null-separated ASCII strings. */
static const char *const g_items[THERON_TRACK02_FULL_ITEM_COUNT] = {
    "COMPASS",         /*  0 — UD 0x099517 */
    "TORCH",           /*  1 — UD 0x09951F */
    "WATERSKIN",       /*  2 — UD 0x099525 */
    "WATER",           /*  3 — UD 0x09952F */
    "ILLUMULET",       /*  4 — UD 0x099535 */
    "FLAMITT",         /*  5 — UD 0x09953F */
    "OPEN SCROLL",     /*  6 — UD 0x099547 */
    "SCROLL",          /*  7 — UD 0x099553 */
    "DAGGER",          /*  8 — UD 0x09955A */
    "FALCHION",        /*  9 — UD 0x099561 */
    "SWORD",           /* 10 — UD 0x09956A */
    "DELTA",           /* 11 — UD 0x099570 */
    "AXE",             /* 12 — UD 0x099576 */
    "MORNINGSTAR",     /* 13 — UD 0x09957A */
    "CROSSBOW",        /* 14 — UD 0x099586 */
    "SLAYER",          /* 15 — UD 0x09958F */
    "ROCK",            /* 16 — UD 0x099596 */
    "THROWING STAR",   /* 17 — UD 0x09959B */
    "THE CONDUIT",     /* 18 — UD 0x0995A9 */
    "SCEPTRE OF LYF",  /* 19 — UD 0x0995B5 */
    "FINE ROBE",       /* 20 — UD 0x0995C4 */
    "KIRTLE",          /* 21 — UD 0x0995CE */
    "SILK SHIRT",      /* 22 — UD 0x0995D5 */
    "LEATHER JERKIN",  /* 23 — UD 0x0995E0 */
    "TUNIC",           /* 24 — UD 0x0995EF */
    "GHI",             /* 25 — UD 0x0995F5 */
    "MAIL AKETON",     /* 26 — UD 0x0995F9 */
    "PLATE OF LYTE",   /* 27 — UD 0x099605 */
    "BARBARIAN HIDE",  /* 28 — UD 0x099613 */
    "TABARD",          /* 29 — UD 0x099622 */
    "GUNNA",           /* 30 — UD 0x099629 */
    "LEATHER PANTS",   /* 31 — UD 0x09962F */
    "BLUE PANTS",      /* 32 — UD 0x09963D */
    "GHI TROUSERS",    /* 33 — UD 0x099648 */
    "LEG MAIL",        /* 34 — UD 0x099655 */
    "POLEYN OF LYTE",  /* 35 — UD 0x09965E */
    "HELMET",          /* 36 — UD 0x09966D */
    "BASINET",         /* 37 — UD 0x099674 */
    "HELM OF LYTE",    /* 38 — UD 0x09967C */
    "SMALL SHIELD",    /* 39 — UD 0x099689 */
    "WOODEN SHIELD",   /* 40 — UD 0x099696 */
    "SHIELD DEFIANT",  /* 41 — UD 0x0996A4 */
    "SANDALS",         /* 42 — UD 0x0996B3 */
    "SUEDE BOOTS",     /* 43 — UD 0x0996BB */
    "LEATHER BOOTS",   /* 44 — UD 0x0996C7 */
    "HOSEN",           /* 45 — UD 0x0996D5 */
    "GREAVE OF LYTE",  /* 46 — UD 0x0996DB */
    "GOLD COIN",       /* 47 — UD 0x0996EA */
    "BOULDER",         /* 48 — UD 0x0996F4 */
    "MAGICAL BOX",     /* 49 — UD 0x0996FC */
    "ROPE",            /* 50 — UD 0x099708 */
    "RABBIT'S FOOT",   /* 51 — UD 0x09970D */
    "TAZAHELM",        /* 52 — UD 0x09971B */
    "CHEST",           /* 53 — UD 0x099724 */
    "OPEN CHEST",      /* 54 — UD 0x09972A */
    "BONES",           /* 55 — UD 0x099735 */
    "VEN POTION",      /* 56 — UD 0x09973B */
    "SAR POTION",      /* 57 — UD 0x099746 */
    "ZO POTION",       /* 58 — UD 0x099751 */
    "ROS POTION",      /* 59 — UD 0x09975B */
    "KU POTION",       /* 60 — UD 0x099766 */
    "DANE POTION",     /* 61 — UD 0x099770 */
    "NETA POTION",     /* 62 — UD 0x09977C */
    "BRO POTION",      /* 63 — UD 0x099788 */
    "MA POTION",       /* 64 — UD 0x099793 */
    "YA POTION",       /* 65 — UD 0x09979D */
    "EE POTION",       /* 66 — UD 0x0997A7 */
    "VI POTION",       /* 67 — UD 0x0997B1 */
    "WATER FLASK",     /* 68 — UD 0x0997BB */
    "FUL BOMB",        /* 69 — UD 0x0997C7 */
    "APPLE",           /* 70 — UD 0x0997D0 */
    "CORN",            /* 71 — UD 0x0997D6 */
    "BREAD",           /* 72 — UD 0x0997DB */
    "CHEESE",          /* 73 — UD 0x0997E1 */
    "SCREAMER SLICE",  /* 74 — UD 0x0997E8 */
    "DRUMSTICK",       /* 75 — UD 0x0997F7 */
    "IRON KEY",        /* 76 — UD 0x099801 */
    "GOLD KEY",        /* 77 — UD 0x09980A */
    "RA KEY",          /* 78 — UD 0x099813 */
    "EMPTY FLASK",     /* 79 — UD 0x09981A */
};

const char *theron_v1_track02_us_full_item_name(unsigned int index) {
    if (index >= THERON_TRACK02_FULL_ITEM_COUNT) return NULL;
    return g_items[index];
}

size_t theron_v1_track02_us_full_item_count(void) {
    return THERON_TRACK02_FULL_ITEM_COUNT;
}
