#include "theron_v1_track19_item_names.h"

#include <string.h>

/* Source: US Track 19 ISO (MD5 51b40a17b92a30339957ba564aa0015c).
 * User-data offset 0x0E9271, null-separated ASCII strings.
 * This is the runtime item name table loaded during dungeon gameplay.
 * Compared to Track 02's table, it adds: WATERSKIN, WATER, STORMRING,
 * MACE, TORSO PLATE, LEG PLATE, ARMET, FOOT PLATE, SOUL CAGE,
 * SARMON'S BONES, IRON KEY, SKELETON KEY, EMERALD KEY, LOCK PICKS. */
static const char *const g_us_item_names[THERON_TRACK19_US_ITEM_NAME_COUNT] = {
    "COMPASS",             /*  0 — T19 0x0E9271 */
    "WATERSKIN",           /*  1 — T19 0x0E9279 */
    "WATER",               /*  2 — T19 0x0E9283 */
    "ILLUMULET",           /*  3 — T19 0x0E9289 */
    "STORMRING",           /*  4 — T19 0x0E9293 */
    "OPEN SCROLL",         /*  5 — T19 0x0E929D */
    "SCROLL",              /*  6 — T19 0x0E92A9 */
    "DAGGER",              /*  7 — T19 0x0E92B0 */
    "DELTA",               /*  8 — T19 0x0E92B7 */
    "VORPAL BLADE",        /*  9 — T19 0x0E92BD */
    "AXE",                 /* 10 — T19 0x0E92CA */
    "MACE",                /* 11 — T19 0x0E92CE */
    "MORNINGSTAR",         /* 12 — T19 0x0E92D3 */
    "CROSSBOW",            /* 13 — T19 0x0E92DF */
    "SLAYER",              /* 14 — T19 0x0E92E8 */
    "ROCK",                /* 15 — T19 0x0E92EF */
    "THROWING STAR",       /* 16 — T19 0x0E92F4 */
    "THE CONDUIT",         /* 17 — T19 0x0E9302 */
    "SCEPTRE OF LYF",      /* 18 — T19 0x0E930E */
    "FINE ROBE",           /* 19 — T19 0x0E931D */
    "KIRTLE",              /* 20 — T19 0x0E9327 */
    "SILK SHIRT",          /* 21 — T19 0x0E932E */
    "LEATHER JERKIN",      /* 22 — T19 0x0E9339 */
    "TUNIC",               /* 23 — T19 0x0E9348 */
    "MAIL AKETON",         /* 24 — T19 0x0E934E */
    "TORSO PLATE",         /* 25 — T19 0x0E935A */
    "BARBARIAN HIDE",      /* 26 — T19 0x0E9366 */
    "TABARD",              /* 27 — T19 0x0E9375 */
    "GUNNA",               /* 28 — T19 0x0E937C */
    "LEATHER PANTS",       /* 29 — T19 0x0E9382 */
    "BLUE PANTS",          /* 30 — T19 0x0E9390 */
    "GHI TROUSERS",        /* 31 — T19 0x0E939B */
    "LEG MAIL",            /* 32 — T19 0x0E93A8 */
    "LEG PLATE",           /* 33 — T19 0x0E93B1 */
    "HELMET",              /* 34 — T19 0x0E93BB */
    "BASINET",             /* 35 — T19 0x0E93C2 */
    "ARMET",               /* 36 — T19 0x0E93CA */
    "SMALL SHIELD",        /* 37 — T19 0x0E93D0 */
    "SANDALS",             /* 38 — T19 0x0E93DD */
    "SUEDE BOOTS",         /* 39 — T19 0x0E93E5 */
    "LEATHER BOOTS",       /* 40 — T19 0x0E93F1 */
    "HOSEN",               /* 41 — T19 0x0E93FF */
    "FOOT PLATE",          /* 42 — T19 0x0E9405 */
    "SOUL CAGE",           /* 43 — T19 0x0E9410 */
    "MAGICAL BOX",         /* 44 — T19 0x0E941A */
    "ROPE",                /* 45 — T19 0x0E9426 */
    "SARMON'S BONES",      /* 46 — T19 0x0E942B */
    "BONES",               /* 47 — T19 0x0E943A */
    "VEN POTION",          /* 48 — T19 0x0E9440 */
    "SAR POTION",          /* 49 — T19 0x0E944B */
    "ZO POTION",           /* 50 — T19 0x0E9456 */
    "ROS POTION",          /* 51 — T19 0x0E9460 */
    "KU POTION",           /* 52 — T19 0x0E946B */
    "DANE POTION",         /* 53 — T19 0x0E9475 */
    "NETA POTION",         /* 54 — T19 0x0E9481 */
    "BRO POTION",          /* 55 — T19 0x0E948D */
    "MA POTION",           /* 56 — T19 0x0E9498 */
    "YA POTION",           /* 57 — T19 0x0E94A2 */
    "EE POTION",           /* 58 — T19 0x0E94AC */
    "VI POTION",           /* 59 — T19 0x0E94B6 */
    "WATER FLASK",         /* 60 — T19 0x0E94C0 */
    "FUL BOMB",            /* 61 — T19 0x0E94CC */
    "APPLE",               /* 62 — T19 0x0E94D5 */
    "DRUMSTICK",           /* 63 — T19 0x0E94DB */
    "IRON KEY",            /* 64 — T19 0x0E94E5 */
    "SKELETON KEY",        /* 65 — T19 0x0E94EE */
    "EMERALD KEY",         /* 66 — T19 0x0E94FB */
    "LOCK PICKS",          /* 67 — T19 0x0E9507 */
    "EMPTY FLASK",         /* 68 — T19 0x0E9512 */
};

const char *theron_v1_track19_us_item_name(unsigned int index) {
    if (index >= THERON_TRACK19_US_ITEM_NAME_COUNT) return NULL;
    return g_us_item_names[index];
}

size_t theron_v1_track19_us_item_name_count(void) {
    return THERON_TRACK19_US_ITEM_NAME_COUNT;
}

int theron_v1_track19_us_item_name_from_iso(
        const uint8_t *iso, size_t iso_size, unsigned int index,
        char *out, size_t out_capacity) {
    size_t cursor = THERON_TRACK19_US_ITEM_NAME_OFFSET;
    unsigned int i;

    if (!iso || index >= THERON_TRACK19_US_ITEM_NAME_COUNT || !out ||
        out_capacity == 0u || cursor >= iso_size) {
        return 0;
    }
    out[0] = '\0';

    /* Track 19's table is source-owned data, not a hint for a generated
     * label. Validate every entry so a partial or shifted span cannot
     * publish one plausible-looking name. */
    for (i = 0u; i < THERON_TRACK19_US_ITEM_NAME_COUNT; ++i) {
        const char *expected = g_us_item_names[i];
        size_t remaining = iso_size - cursor;
        const uint8_t *terminator = memchr(iso + cursor, 0, remaining);
        size_t length;

        if (!terminator) return 0;
        length = (size_t)(terminator - (iso + cursor));
        if (length != strlen(expected) ||
            memcmp(iso + cursor, expected, length) != 0) {
            return 0;
        }
        if (i == index) {
            if (length + 1u > out_capacity) return 0;
            memcpy(out, iso + cursor, length);
            out[length] = '\0';
        }
        cursor += length + 1u;
    }
    return out[0] != '\0';
}
