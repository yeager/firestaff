
#include "dm2_v1_tech_magic.h"

#include <string.h>

/* DM2 object identity is a decoded DB pool/index plus GDAT metadata.  The
 * former ten-entry English table was authored by Firestaff: its IDs, names,
 * affinities and charges are not an imported original object definition.
 * Do not let it label HUD objects or authorize item mechanics. */

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
    if (!item) return -1;
    /* charges == 0 → empty; charges < 0 (e.g. -1) → unlimited (always usable) */
    if (item->charges == 0) return -1;
    switch (item->power_source) {
        case 0: return 0;  /* manual — no cost */
        case 1: return 1;  /* battery — 1 charge */
        case 2: return item->magic_level * 2; /* mana cost */
        case 3: return item->tech_level + item->magic_level; /* hybrid */
        default: return 0;
    }
}

int dm2_v1_tech_magic_lookup(int item_id, DM2_V1_TechMagicItem *out) {
    (void)item_id;
    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    return 0;
}

const char *dm2_v1_tech_magic_item_name(int item_id) {
    DM2_V1_TechMagicItem item;
    if (!dm2_v1_tech_magic_lookup(item_id, &item)) {
        return NULL;
    }
    return item.name;
}

int dm2_v1_tech_magic_consume_charge(DM2_V1_TechMagicItem *item) {
    if (!item) return 0;
    if (item->charges == 0) return 0;       /* already empty */
    if (item->charges < 0) return 1;        /* unlimited (-1) — no-op consume */
    item->charges--;
    return 1;
}

int dm2_v1_tech_magic_hybrid_power(const DM2_V1_TechMagicItem *item) {
    if (!item) return 0;
    if (item->affinity != DM2_ITEM_HYBRID) return 0;
    int p = item->tech_level * 25 + item->magic_level * 25;
    if (p > 100) p = 100;
    if (p < 0)   p = 0;
    return p;
}

const char *dm2_v1_tech_magic_source_evidence(void) {
    return
        "Source: SKProject SKWIN/SkWinCore.cpp object-ID and GDAT dispatch\n"
        "Admission: original DB/GDAT item definition ownership is not imported;\n"
        "           legacy fixture item IDs, names, affinities and charges are unavailable.\n";
}
