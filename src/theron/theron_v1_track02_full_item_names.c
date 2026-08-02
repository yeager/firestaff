#include "theron_v1_track02_full_item_names.h"

/* Source: US Track 02 BIN (MD5 f23601102138f87c33025877767ebf76).
 * UD offset 0x09951F, null-separated ASCII strings.
 * Per-bank runtime item name table (replicated per dungeon overlay). */
static const char *const g_items[THERON_TRACK02_FULL_ITEM_COUNT] = {
    "TORCH",           /*  0 — UD 0x09951F */
    "WATERSKIN",       /*  1 — UD 0x099525 */
    "WATER",           /*  2 — UD 0x09952F */
    "ILLUMULET",       /*  3 — UD 0x099535 */
    "FLAMITT",         /*  4 — UD 0x09953F */
    "OPEN SCROLL",     /*  5 — UD 0x099547 */
    "SCROLL",          /*  6 — UD 0x099553 */
    "DAGGER",          /*  7 — UD 0x09955A */
    "FALCHION",        /*  8 — UD 0x099561 */
    "SWORD",           /*  9 — UD 0x09956A */
    "DELTA",           /* 10 — UD 0x099570 */
    "AXE",             /* 11 — UD 0x099576 */
    "MORNINGSTAR",     /* 12 — UD 0x09957A */
    "CROSSBOW",        /* 13 — UD 0x099586 */
    "SLAYER",          /* 14 — UD 0x09958F */
    "ROCK",            /* 15 — UD 0x099596 */
    "THROWING STAR",   /* 16 — UD 0x09959B */
    "THE CONDUIT",     /* 17 — UD 0x0995A9 */
    "SCEPTRE OF LYF",  /* 18 — UD 0x0995B5 */
    "FINE ROBE",       /* 19 — UD 0x0995C4 */
    "KIRTLE",          /* 20 — UD 0x0995CE */
    "SILK SHIRT",      /* 21 — UD 0x0995D5 */
    "LEATHER JERKIN",  /* 22 — UD 0x0995E0 */
    "TUNIC",           /* 23 — UD 0x0995EF */
    "GHI",             /* 24 — UD 0x0995F5 */
    "MAIL AKETON",     /* 25 — UD 0x0995F9 */
    "PLATE OF LYTE",   /* 26 — UD 0x099605 */
    "BARBARIAN HIDE",  /* 27 — UD 0x099613 */
    "TABARD",          /* 28 — UD 0x099622 */
    "GUNNA",           /* 29 — UD 0x099629 */
    "LEATHER PANTS",   /* 30 — UD 0x09962F */
    "BLUE PANTS",      /* 31 — UD 0x09963D */
    "GHI TROUSERS",    /* 32 — UD 0x099648 */
    "LEG MAIL",        /* 33 — UD 0x099655 */
    "POLEYN OF LYTE",  /* 34 — UD 0x09965E */
    "HELMET",          /* 35 — UD 0x09966D */
    "BASINET",         /* 36 — UD 0x099674 */
    "HELM OF LYTE",    /* 37 — UD 0x09967C */
    "SMALL SHIELD",    /* 38 — UD 0x099689 */
    "WOODEN SHIELD",   /* 39 — UD 0x099696 */
    "SHIELD DEFIANT",  /* 40 — UD 0x0996A4 */
    "SANDALS",         /* 41 — UD 0x0996B3 */
    "SUEDE BOOTS",     /* 42 — UD 0x0996BB */
    "LEATHER BOOTS",   /* 43 — UD 0x0996C7 */
    "HOSEN",           /* 44 — UD 0x0996D5 */
    "GREAVE OF LYTE",  /* 45 — UD 0x0996DB */
    "GOLD COIN",       /* 46 — UD 0x0996EA */
    "BOULDER",         /* 47 — UD 0x0996F4 */
    "MAGICAL BOX",     /* 48 — UD 0x0996FC */
    "ROPE",            /* 49 — UD 0x099708 */
    "RABBIT'S FOOT",   /* 50 — UD 0x09970D */
    "TAZAHELM",        /* 51 — UD 0x09971B */
    "CHEST",           /* 52 — UD 0x099724 */
    "OPEN CHEST",      /* 53 — UD 0x09972A */
    "BONES",           /* 54 — UD 0x099735 */
    "VEN POTION",      /* 55 — UD 0x09973B */
    "SAR POTION",      /* 56 — UD 0x099746 */
    "ZO POTION",       /* 57 — UD 0x099751 */
    "ROS POTION",      /* 58 — UD 0x09975B */
    "KU POTION",       /* 59 — UD 0x099766 */
    "DANE POTION",     /* 60 — UD 0x099770 */
    "NETA POTION",     /* 61 — UD 0x09977C */
    "BRO POTION",      /* 62 — UD 0x099788 */
    "MA POTION",       /* 63 — UD 0x099793 */
    "YA POTION",       /* 64 — UD 0x09979D */
    "EE POTION",       /* 65 — UD 0x0997A7 */
    "VI POTION",       /* 66 — UD 0x0997B1 */
    "WATER FLASK",     /* 67 — UD 0x0997BB */
    "FUL BOMB",        /* 68 — UD 0x0997C7 */
    "APPLE",           /* 69 — UD 0x0997D0 */
    "CORN",            /* 70 — UD 0x0997D6 */
    "BREAD",           /* 71 — UD 0x0997DB */
    "CHEESE",          /* 72 — UD 0x0997E1 */
    "SCREAMER SLICE",  /* 73 — UD 0x0997E8 */
    "DRUMSTICK",       /* 74 — UD 0x0997F7 */
    "IRON KEY",        /* 75 — UD 0x099801 */
    "GOLD KEY",        /* 76 — UD 0x09980A */
    "RA KEY",           /* 77 — UD 0x099813 */
    "EMPTY FLASK",     /* 78 — UD 0x09981A */
};

const char *theron_v1_track02_us_full_item_name(unsigned int index) {
    if (index >= THERON_TRACK02_FULL_ITEM_COUNT) return NULL;
    return g_items[index];
}

size_t theron_v1_track02_us_full_item_count(void) {
    return THERON_TRACK02_FULL_ITEM_COUNT;
}
