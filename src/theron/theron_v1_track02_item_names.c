#include "theron_v1_track02_item_names.h"

/* Source: US Track 02 BIN, user-data offset 0x21A08E.
 * Hash: f23601102138f87c33025877767ebf76 (MD5).
 * Evidence: parity-evidence/pass215_theron_track02_binary_analysis.md */
static const char *const g_us_item_names[THERON_TRACK02_US_ITEM_NAME_COUNT] = {
    "COMPASS",
    "ILLUMULET",
    "OPEN SCROLL",
    "SCROLL",
    "DAGGER",
    "DELTA",
    "VORPAL BLADE",
    "THE RETALIATOR",
    "AXE",
    "MORNINGSTAR",
    "CROSSBOW",
    "SLAYER",
    "ROCK",
    "THROWING STAR",
    "STAFF OF MANAR",
    "THE CONDUIT",
    "SCEPTRE OF LYF",
    "FINE ROBE",
    "KIRTLE",
    "SILK SHIRT",
    "LEATHER JERKIN",
    "TUNIC",
    "MAIL AKETON",
    "MITHRAL AKETON",
    "BARBARIAN HIDE",
    "TABARD",
    "GUNNA",
    "LEATHER PANTS",
    "BLUE PANTS",
    "GHI TROUSERS",
    "LEG MAIL",
    "MITHRAL MAIL",
    "HELMET",
    "BASINET",
    "SMALL SHIELD",
    "SANDALS",
    "SUEDE BOOTS",
    "LEATHER BOOTS",
    "HOSEN",
    "EKKHARD CROSS",
    "MAGICAL BOX",
    "ROPE",
    "RABBIT'S FOOT",
    "TAZAHELM",
    "CHEST",
    "OPEN CHEST",
    "BONES",
    "VEN POTION",
    "SAR POTION",
    "ZO POTION",
    "ROS POTION",
    "KU POTION",
    "DANE POTION",
    "NETA POTION",
    "BRO POTION",
    "MA POTION",
    "YA POTION",
    "EE POTION",
    "VI POTION",
    "WATER FLASK",
    "FUL BOMB",
    "APPLE",
    "CORN",
    "DRUMSTICK",
    "TOPAZ KEY",
    "EMPTY FLASK",
};

const char *theron_v1_track02_us_item_name(unsigned int index) {
    if (index >= THERON_TRACK02_US_ITEM_NAME_COUNT) return NULL;
    return g_us_item_names[index];
}

size_t theron_v1_track02_us_item_name_count(void) {
    return THERON_TRACK02_US_ITEM_NAME_COUNT;
}
