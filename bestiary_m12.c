#include "bestiary_m12.h"
#include <stddef.h>

/* ── DM1 Creature Database ────────────────────────────────────────
 *
 * Data sourced from Dungeon Master 1 (Atari ST / PC).
 * HP ranges are approximate based on dungeon level scaling.
 * Art indices reference creature_art_m12.h thumbnails where available.
 *
 * Art index mapping (creature_art_data_m12.h):
 *   0 = GIANT SCORPION
 *   1 = RED DRAGON
 *   2 = SKELETON
 *   3 = STONE GOLEM
 *   4 = MUMMY
 */

static const M12_BestiaryEntry g_bestiaryEntries[] = {
    /* ── HUMANOID ─────────────────────────────────────────────── */
    {
        "SCREAMER",
        M12_BESTIARY_CAT_HUMANOID,
        15, 30,
        M12_BESTIARY_ATK_MELEE,
        "FIRE, NINJA SKILLS",
        "MUSHROOM-LIKE CREATURES THAT SHRIEK TO\n"
        "ALERT NEARBY MONSTERS. WEAK INDIVIDUALLY\n"
        "BUT DANGEROUS IN SWARMS. FOUND ON THE\n"
        "UPPER DUNGEON LEVELS.",
        -1, 1
    },
    {
        "GIGGLER",
        M12_BESTIARY_CAT_HUMANOID,
        20, 45,
        M12_BESTIARY_ATK_MELEE,
        "VORPAL BLADE, SPEED",
        "MISCHIEVOUS THIEVES THAT STEAL ITEMS FROM\n"
        "YOUR PARTY. FAST AND ANNOYING, THEY GIGGLE\n"
        "AS THEY DART IN AND OUT OF REACH. KILL THEM\n"
        "QUICKLY TO RECOVER STOLEN GOODS.",
        -1, 2
    },
    {
        "TROLIN",
        M12_BESTIARY_CAT_HUMANOID,
        60, 100,
        M12_BESTIARY_ATK_MELEE,
        "FIRE, MAGIC MISSILES",
        "BRUTISH TROLL-LIKE WARRIORS WITH THICK HIDE.\n"
        "THEY LUMBER THROUGH CORRIDORS SWINGING\n"
        "MASSIVE CLUBS. TOUGH BUT SLOW. OFTEN GUARD\n"
        "KEY PASSAGES IN THE MID DUNGEON.",
        -1, 4
    },
    /* ── UNDEAD ────────────────────────────────────────────────── */
    {
        "MUMMY",
        M12_BESTIARY_CAT_UNDEAD,
        50, 90,
        M12_BESTIARY_ATK_MELEE,
        "FIRE, VORPAL BLADE",
        "ANCIENT WRAPPED CORPSES ANIMATED BY DARK\n"
        "MAGIC. THEY SHAMBLE RELENTLESSLY AND HIT\n"
        "HARD. EXTREMELY VULNERABLE TO FIRE SPELLS.\n"
        "COMMON IN THE TOMB LEVELS.",
        4, 3
    },
    {
        "SKELETON",
        M12_BESTIARY_CAT_UNDEAD,
        25, 55,
        M12_BESTIARY_ATK_MELEE,
        "MACE, BLUNT WEAPONS",
        "REANIMATED BONES WIELDING RUSTY WEAPONS.\n"
        "FRAGILE BUT NUMEROUS. THEIR BONES SCATTER\n"
        "SATISFYINGLY WHEN STRUCK WITH BLUNT FORCE.\n"
        "GUARD THE EARLY CRYPT CHAMBERS.",
        2, 2
    },
    {
        "GHOST",
        M12_BESTIARY_CAT_UNDEAD,
        40, 70,
        M12_BESTIARY_ATK_MAGIC,
        "VORPAL BLADE, DES EW",
        "SPECTRAL ENTITIES THAT PHASE THROUGH WALLS.\n"
        "IMMUNE TO MOST PHYSICAL ATTACKS. ONLY\n"
        "MAGICAL WEAPONS AND SPELLS CAN HARM THEM.\n"
        "THEIR TOUCH DRAINS STAMINA.",
        -1, 5
    },
    /* ── BEAST ─────────────────────────────────────────────────── */
    {
        "GIANT SCORPION",
        M12_BESTIARY_CAT_BEAST,
        80, 140,
        M12_BESTIARY_ATK_POISON,
        "FIRE, SPEED ATTACKS",
        "ENORMOUS ARACHNIDS WITH DEADLY VENOMOUS\n"
        "STINGERS. THEIR POISON WEAKENS EVEN THE\n"
        "STRONGEST CHAMPIONS. APPROACH WITH ANTI-\n"
        "VENOM OR STRONG FIRE MAGIC.",
        0, 6
    },
    {
        "GIANT WASP",
        M12_BESTIARY_CAT_BEAST,
        30, 50,
        M12_BESTIARY_ATK_POISON,
        "FIRE, RANGED ATTACKS",
        "OVERSIZED INSECTS WITH PAINFUL STINGERS.\n"
        "THEY FLY ERRATICALLY, MAKING THEM HARD TO\n"
        "HIT IN MELEE. THEIR VENOM IS LESS POTENT\n"
        "THAN THE SCORPION BUT STILL DANGEROUS.",
        -1, 4
    },
    {
        "COUATL",
        M12_BESTIARY_CAT_BEAST,
        55, 85,
        M12_BESTIARY_ATK_MAGIC,
        "PHYSICAL ATTACKS",
        "FEATHERED SERPENTS OF ANCIENT POWER. THEY\n"
        "SPIT MAGICAL PROJECTILES AND SLITHER WITH\n"
        "SURPRISING SPEED. RESISTANT TO MAGIC BUT\n"
        "VULNERABLE TO STRONG MELEE STRIKES.",
        -1, 7
    },
    {
        "WORM",
        M12_BESTIARY_CAT_BEAST,
        70, 110,
        M12_BESTIARY_ATK_MELEE,
        "FIRE, SLASHING WEAPONS",
        "MASSIVE PURPLE WORMS THAT BURROW THROUGH\n"
        "THE DEEP DUNGEON. THEIR BULK FILLS ENTIRE\n"
        "CORRIDORS. TOUGH HIDE BUT SLOW TO TURN.\n"
        "CAN SWALLOW SMALL ITEMS WHOLE.",
        -1, 8
    },
    /* ── CONSTRUCT ─────────────────────────────────────────────── */
    {
        "STONE GOLEM",
        M12_BESTIARY_CAT_CONSTRUCT,
        120, 200,
        M12_BESTIARY_ATK_MELEE,
        "ZO KATH RA, MAGIC",
        "ANIMATED STONE GUARDIANS OF IMMENSE POWER.\n"
        "NEAR-IMPERVIOUS TO PHYSICAL DAMAGE. ONLY\n"
        "THE MOST POWERFUL SPELLS CAN CRACK THEIR\n"
        "GRANITE SHELLS. SLOW BUT DEVASTATING.",
        3, 10
    },
    {
        "ANIMATED ARMOUR",
        M12_BESTIARY_CAT_CONSTRUCT,
        90, 150,
        M12_BESTIARY_ATK_MELEE,
        "LIGHTNING, DES EW",
        "ENCHANTED SUITS OF PLATE MAIL THAT FIGHT\n"
        "WITHOUT A WEARER. THEY MIMIC THE COMBAT\n"
        "STYLE OF FALLEN KNIGHTS. RESISTANT TO\n"
        "EDGED WEAPONS BUT WEAK TO ELECTRICITY.",
        -1, 9
    },
    /* ── DEMON ─────────────────────────────────────────────────── */
    {
        "VEXIRK",
        M12_BESTIARY_CAT_DEMON,
        100, 160,
        M12_BESTIARY_ATK_FIRE,
        "VORPAL BLADE, ICE MAGIC",
        "DEMONIC BEINGS FROM THE LOWER PLANES.\n"
        "THEY HURL FIREBALLS AND TELEPORT SHORT\n"
        "DISTANCES. UNPREDICTABLE AND AGGRESSIVE.\n"
        "ONE OF LORD CHAOS'S FAVOURED SERVANTS.",
        -1, 11
    },
    {
        "WATER ELEMENTAL",
        M12_BESTIARY_CAT_DEMON,
        80, 130,
        M12_BESTIARY_ATK_MAGIC,
        "LIGHTNING, FUL",
        "SWIRLING MASSES OF ANIMATED WATER THAT\n"
        "CRASH AGAINST INTRUDERS. IMMUNE TO MOST\n"
        "PHYSICAL WEAPONS. FIRE AND LIGHTNING\n"
        "SPELLS DISRUPT THEIR FLUID FORM.",
        -1, 10
    },
    /* ── DRAGON ────────────────────────────────────────────────── */
    {
        "RED DRAGON",
        M12_BESTIARY_CAT_DRAGON,
        200, 350,
        M12_BESTIARY_ATK_FIRE,
        "ICE MAGIC, SPEED",
        "THE MOST FEARSOME CREATURE IN THE DUNGEON.\n"
        "BREATHES DEVASTATING FIRE AND POSSESSES\n"
        "INCREDIBLE STRENGTH. ONLY A FULLY EQUIPPED\n"
        "AND EXPERIENCED PARTY STANDS A CHANCE.",
        1, 12
    },
};

#define G_BESTIARY_COUNT \
    ((int)(sizeof(g_bestiaryEntries) / sizeof(g_bestiaryEntries[0])))

/* ── Category display names ──────────────────────────────────────── */
static const char* const g_categoryNames[M12_BESTIARY_CAT_COUNT] = {
    "ALL",
    "HUMANOID",
    "UNDEAD",
    "BEAST",
    "CONSTRUCT",
    "DEMON",
    "DRAGON"
};

/* ── Internal: rebuild filtered count ────────────────────────────── */
static int bestiary_count_filtered(M12_BestiaryCategory cat) {
    int i, count = 0;
    if (cat == M12_BESTIARY_CAT_ALL) {
        return G_BESTIARY_COUNT;
    }
    for (i = 0; i < G_BESTIARY_COUNT; ++i) {
        if (g_bestiaryEntries[i].category == cat) {
            ++count;
        }
    }
    return count;
}

/* ── Internal: map filtered index to database index ──────────────── */
static int bestiary_filtered_to_db(M12_BestiaryCategory cat, int filtIdx) {
    int i, seen = 0;
    if (cat == M12_BESTIARY_CAT_ALL) {
        return (filtIdx >= 0 && filtIdx < G_BESTIARY_COUNT) ? filtIdx : -1;
    }
    for (i = 0; i < G_BESTIARY_COUNT; ++i) {
        if (g_bestiaryEntries[i].category == cat) {
            if (seen == filtIdx) {
                return i;
            }
            ++seen;
        }
    }
    return -1;
}

/* ── Public API ──────────────────────────────────────────────────── */

void M12_Bestiary_Init(M12_BestiaryState* bs) {
    if (!bs) return;
    bs->scrollOffset   = 0;
    bs->selectedIndex  = 0;
    bs->categoryFilter = M12_BESTIARY_CAT_ALL;
    bs->filteredCount  = G_BESTIARY_COUNT;
}

int M12_Bestiary_TotalCount(void) {
    return G_BESTIARY_COUNT;
}

int M12_Bestiary_FilteredCount(const M12_BestiaryState* bs) {
    if (!bs) return 0;
    return bs->filteredCount;
}

const M12_BestiaryEntry* M12_Bestiary_GetFiltered(
    const M12_BestiaryState* bs, int index)
{
    int dbIdx;
    if (!bs || index < 0 || index >= bs->filteredCount) {
        return NULL;
    }
    dbIdx = bestiary_filtered_to_db(bs->categoryFilter, index);
    if (dbIdx < 0) return NULL;
    return &g_bestiaryEntries[dbIdx];
}

const M12_BestiaryEntry* M12_Bestiary_GetSelected(
    const M12_BestiaryState* bs)
{
    if (!bs) return NULL;
    return M12_Bestiary_GetFiltered(bs, bs->selectedIndex);
}

void M12_Bestiary_Scroll(M12_BestiaryState* bs, int delta) {
    int maxSel;
    if (!bs || bs->filteredCount == 0) return;

    bs->selectedIndex += delta;
    if (bs->selectedIndex < 0) {
        bs->selectedIndex = 0;
    }
    maxSel = bs->filteredCount - 1;
    if (bs->selectedIndex > maxSel) {
        bs->selectedIndex = maxSel;
    }

    /* Adjust scroll window */
    if (bs->selectedIndex < bs->scrollOffset) {
        bs->scrollOffset = bs->selectedIndex;
    }
    if (bs->selectedIndex >= bs->scrollOffset + M12_BESTIARY_VISIBLE_LINES) {
        bs->scrollOffset = bs->selectedIndex - M12_BESTIARY_VISIBLE_LINES + 1;
    }
}

void M12_Bestiary_CycleCategory(M12_BestiaryState* bs, int direction) {
    int cat;
    if (!bs) return;

    cat = (int)bs->categoryFilter + direction;
    if (cat < 0) {
        cat = M12_BESTIARY_CAT_COUNT - 1;
    }
    if (cat >= M12_BESTIARY_CAT_COUNT) {
        cat = 0;
    }
    bs->categoryFilter = (M12_BestiaryCategory)cat;
    bs->filteredCount  = bestiary_count_filtered(bs->categoryFilter);
    bs->scrollOffset   = 0;
    bs->selectedIndex  = 0;
}

const char* M12_Bestiary_CategoryName(M12_BestiaryCategory cat) {
    if (cat < 0 || cat >= M12_BESTIARY_CAT_COUNT) {
        return "UNKNOWN";
    }
    return g_categoryNames[cat];
}
