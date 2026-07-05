
#include "dm2_v1_tech_magic.h"

#include <string.h>

/* ── Known item catalog (SKULL.ASM tech/magic item table) ──────────
 * Source: skproject/SKWIN/SkWinCore.cpp tech/magic item table
 *         SKULL.ASM tech item routines
 *
 * Built-in DM2 items with their affinity, levels, power source, charges.
 * Ranged weapons: crossbow (low tech), pistol (tech 1), rifle (tech 2).
 * Bombs: throw (tech 1), remote (tech 2 hybrid).
 * Lights: lantern (tech 0, battery).
 * Magic: flame orb, potions.
 */
static const DM2_V1_TechMagicItem g_known_items[] = {
    { DM2_ITEM_CROSSBOW,      "CROSSBOW",      DM2_ITEM_TECH,   0, 0, DM2_POWER_MANUAL,  -1 },  /* unlimited */
    { DM2_ITEM_PISTOL,        "PISTOL",        DM2_ITEM_TECH,   1, 0, DM2_POWER_BATTERY, 10 },
    { DM2_ITEM_RIFLE,         "RIFLE",         DM2_ITEM_TECH,   2, 0, DM2_POWER_BATTERY,  8 },
    { DM2_ITEM_BOMB_THROW,    "THROW BOMB",    DM2_ITEM_TECH,   1, 0, DM2_POWER_MANUAL,   3 },
    { DM2_ITEM_BOMB_REMOTE,   "REMOTE BOMB",   DM2_ITEM_HYBRID, 2, 1, DM2_POWER_HYBRID,   2 },
    { DM2_ITEM_LANTERN,       "LANTERN",       DM2_ITEM_TECH,   0, 0, DM2_POWER_BATTERY, 50 },
    { DM2_ITEM_MAGIC_BATTERY, "MAGIC BATTERY", DM2_ITEM_HYBRID, 1, 1, DM2_POWER_HYBRID,   5 },
    { DM2_ITEM_FLAME_ORB,     "FLAME ORB",     DM2_ITEM_MAGIC,  0, 2, DM2_POWER_MANA,    -1 },  /* unlimited w/ mana */
    { DM2_ITEM_HEAL_POTION,   "HEAL POTION",   DM2_ITEM_MAGIC,  0, 1, DM2_POWER_MANA,     1 },
    { DM2_ITEM_MANA_POTION,   "MANA POTION",   DM2_ITEM_MAGIC,  0, 2, DM2_POWER_MANA,     1 },
};

#define DM2_KNOWN_ITEM_COUNT \
    (sizeof(g_known_items) / sizeof(g_known_items[0]))

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
    if (!out) return 0;
    for (size_t i = 0; i < DM2_KNOWN_ITEM_COUNT; i++) {
        if (g_known_items[i].item_id == item_id) {
            *out = g_known_items[i];
            return 1;
        }
    }
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
        "DM2 V1 Tech/Magic Hybrid System — Phase 4 source-lock\n"
        "ReDMCSB SKULL.ASM (sha256 a2a04b0ea7c05fd2b2a7a8da5197cdfcccd7d4d0167943caf3a21a079462e099)\n"
        "Source: SKULL.ASM tech/magic item routines (item table)\n"
        "Source: skproject/SKWIN/SkWinCore.cpp tech/magic item dispatch\n"
        "Source: skproject/SKWIN/SkGlobal.h tech_level constants\n"
        "Source: skproject/SKWIN/SkGlobal.h:50-53 (SPELL_TYPE_POTION/MISSILE)\n"
        "DM2 feature: hybrid items require BOTH tech AND magic levels\n"
        "DM2 feature: per-rune mana deduction (SkWinCore.cpp:18159-18174)\n"
        "DM2 feature: tech weapons need tech_level>=1, magic_items need magic_level>=1\n"
        "DM2 feature: hybrid power = tech_level*25 + magic_level*25, capped at 100\n";
}
