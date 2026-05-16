
#include "dm2_v1_tech_magic.h"

int dm2_v1_item_can_use(const DM2_V1_TechMagicItem *item,
    int champion_tech, int champion_magic)
{
    if (!item) return 0;
    switch (item->affinity) {
        case DM2_ITEM_TECH: return champion_tech >= item->tech_level;
        case DM2_ITEM_MAGIC: return champion_magic >= item->magic_level;
        case DM2_ITEM_HYBRID: return champion_tech >= item->tech_level &&
                                     champion_magic >= item->magic_level;
        default: return 0;
    }
}

int dm2_v1_item_power_cost(const DM2_V1_TechMagicItem *item) {
    if (!item || item->charges <= 0) return -1;
    switch (item->power_source) {
        case 0: return 0;  /* manual — no cost */
        case 1: return 1;  /* battery — 1 charge */
        case 2: return item->magic_level * 2; /* mana cost */
        case 3: return item->tech_level + item->magic_level; /* hybrid */
        default: return 0;
    }
}

const char *dm2_v1_tech_magic_source_evidence(void) {
    return "SKULL.ASM: tech/magic item routines\n"
           "DM2 feature: tech items (guns, bombs), hybrid items\n";
}

