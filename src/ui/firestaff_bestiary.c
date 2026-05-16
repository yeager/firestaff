
#include "firestaff_bestiary.h"
#include <string.h>

/* Bestiary — creature stats from ReDMCSB DEFS.H creature info table.
 * Source: ReDMCSB DEFS.H G0243_CreatureInfo (24 creature types in DM1). */

static const FS_BestiaryEntry g_bestiary[] = {
    /* DM1 creatures (source: ReDMCSB DEFS.H creature properties) */
    {"Giant Scorpion",    "Venomous arachnid lurking in dark corridors.",
     0, 20, 15, 5, 3, 4, 8, 0, 0, 0, 0},
    {"Swamp Slime",       "Corrosive ooze that dissolves metal on contact.",
     0, 30, 10, 2, 0, 2, 0, 0, 0, 0, 1},
    {"Screamer",          "Fungoid creature. Shrieks to alert nearby monsters.",
     0, 15, 8, 1, 0, 1, 0, 0, 0, 0, 2},
    {"Rockpile",          "Animated pile of rocks. Surprisingly fast.",
     0, 60, 20, 15, 10, 3, 0, 0, 0, 0, 3},
    {"Ghost",             "Incorporeal undead. Immune to normal weapons.",
     0, 25, 18, 0, 0, 5, 0, 0, 0, 1, 4},
    {"Pain Rat",          "Swarm rodent. Weak alone, dangerous in groups.",
     0, 8, 6, 1, 0, 6, 0, 0, 0, 0, 5},
    {"Mummy",             "Shambling undead wrapped in decaying bandages.",
     0, 50, 22, 8, 4, 2, 0, 0, 0, 0, 6},
    {"Skeleton",          "Animated bones wielding rusted weapons.",
     0, 35, 18, 6, 5, 4, 0, 0, 0, 0, 7},
    {"Couatl",            "Flying serpent with venomous bite.",
     0, 40, 25, 4, 2, 6, 10, 0, 1, 0, 8},
    {"Giant Wasp",        "Aggressive insect with paralytic sting.",
     0, 18, 20, 2, 1, 7, 6, 0, 1, 0, 9},
    {"Muncher",           "Voracious beast that eats everything in its path.",
     0, 55, 30, 10, 6, 3, 0, 0, 0, 0, 10},
    {"Animated Armour",   "Enchanted suit of armor. Resistant to physical attacks.",
     0, 70, 25, 20, 15, 2, 0, 0, 0, 0, 11},
    {"Worm",              "Giant tunnel worm. Attacks from below.",
     0, 45, 22, 5, 3, 3, 4, 0, 0, 0, 12},
    {"Deth Knight",       "Powerful undead warrior. Casts spells.",
     0, 80, 35, 15, 12, 4, 0, 0, 0, 0, 13},
    {"Materializer",      "Appears and disappears at will. Difficult to hit.",
     0, 30, 20, 3, 0, 5, 0, 1, 0, 1, 14},
    {"Water Elemental",   "Living water. Resistant to fire, weak to lightning.",
     0, 50, 25, 8, 0, 4, 0, 0, 0, 0, 15},
    {"Oitu",              "Mysterious magical creature from deep dungeons.",
     0, 60, 30, 12, 8, 5, 0, 1, 0, 0, 16},
    {"Demon",             "Fiery demon from the abyss. Casts devastating spells.",
     0, 100, 45, 20, 15, 4, 0, 1, 0, 0, 17},
    {"Red Dragon",        "Ancient dragon. Breathes fire. The ultimate challenge.",
     0, 200, 60, 30, 20, 3, 0, 0, 1, 0, 18},
    {"Lord Chaos",        "The final enemy. Wields immense dark power.",
     0, 500, 80, 40, 25, 5, 0, 1, 0, 0, 19},

    /* CSB additional creatures */
    {"Stone Golem",       "Magical construct of living stone. Nearly indestructible.",
     1, 120, 40, 25, 20, 1, 0, 0, 0, 0, 20},
    {"Black Flame",       "Sentient dark fire. Burns all who approach.",
     1, 40, 35, 5, 0, 6, 0, 0, 1, 1, 21},

    /* DM2 creatures */
    {"Axeman",            "DM2 warrior enemy wielding a battle axe.",
     2, 65, 30, 12, 10, 4, 0, 0, 0, 0, 22},
    {"Tech Guardian",     "DM2 mechanical construct guarding tech areas.",
     2, 90, 35, 18, 15, 3, 0, 0, 0, 0, 23},

    {NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};

int fs_bestiary_count(void) {
    int i = 0;
    while (g_bestiary[i].name) i++;
    return i;
}

const FS_BestiaryEntry *fs_bestiary_get(int index) {
    if (index < 0 || index >= fs_bestiary_count()) return NULL;
    return &g_bestiary[index];
}

const FS_BestiaryEntry *fs_bestiary_find(const char *name) {
    for (int i = 0; g_bestiary[i].name; i++) {
        if (strcmp(g_bestiary[i].name, name) == 0) return &g_bestiary[i];
    }
    return NULL;
}

