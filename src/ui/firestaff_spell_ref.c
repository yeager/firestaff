
#include "firestaff_spell_ref.h"
#include <stddef.h>

/* Spell Reference — DM1 spell system.
 * Spells are cast by combining power symbols (1-6) with elemental
 * symbols in sequence. Source: common DM1 knowledge / manual. */

static const FS_SpellEntry g_spells[] = {
    /* Priest spells */
    {"Light",          "OH IR RA",     "Illuminates dark areas.",                2, "Priest", 7},
    {"Darkness",       "DES IR RA",    "Extinguishes nearby light sources.",     2, "Priest", 9},
    {"Heal",           "VI",           "Restores a small amount of health.",     1, "Priest", 5},
    {"Shield",         "YA IR",        "Creates a protective magical shield.",   2, "Priest", 10},
    {"Stamina",        "VI BRO",       "Restores stamina.",                      2, "Priest", 8},
    {"Cure Poison",    "VI BRO NETA",  "Removes poison from a champion.",       3, "Priest", 15},
    {"Party Shield",   "YA BRO NETA",  "Shields the entire party.",             4, "Priest", 25},

    /* Wizard spells */
    {"Fireball",       "FUL IR",       "Launches a ball of fire at enemies.",    2, "Wizard", 12},
    {"Lightning",      "OH KATH RA",   "Strikes enemies with lightning.",        3, "Wizard", 18},
    {"Poison Cloud",   "OH VEN",       "Creates a cloud of toxic gas.",          2, "Wizard", 10},
    {"Poison Bolt",    "DES VEN",      "Shoots a bolt of poison.",              3, "Wizard", 14},
    {"Open Door",      "ZO",           "Magically opens a locked door.",         1, "Wizard", 8},
    {"Torch",          "FUL",          "Creates a magical light source.",        1, "Wizard", 5},
    {"Invisibility",   "OH EW RA",     "Makes the party invisible briefly.",     4, "Wizard", 30},
    {"Dispell",        "DES EW",       "Removes magical effects.",              3, "Wizard", 20},

    {NULL, NULL, NULL, 0, NULL, 0}
};

int fs_spell_count(void) {
    int i = 0; while (g_spells[i].name) i++; return i;
}

const FS_SpellEntry *fs_spell_get(int index) {
    if (index < 0 || index >= fs_spell_count()) return NULL;
    return &g_spells[index];
}

