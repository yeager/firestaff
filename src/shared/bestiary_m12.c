#include "bestiary_m12.h"
#include <stddef.h>

/* ── DM1 Creature Database ────────────────────────────────────────
 *
 * This view is a source index, not a strategy guide.  The previous table
 * contained approximate HP ranges, invented weaknesses/lore and generated
 * thumbnail indices.  These rows now retain only the exact PC34 creature
 * identity and base-health value from ReDMCSB DUNGEON.C
 * G0243_as_Graphic559_CreatureInfo (C00-C26).  Attack details, weakness,
 * dungeon placement and pixels remain unavailable until their source
 * consumers are bound, so they are intentionally left empty.
 */

static const M12_BestiaryEntry g_bestiaryEntries[] = {
    /* category is a launcher-only grouping; it is not a game property. */
#define C(id, name, cat, hp) { name " (C" id ")", cat, hp, hp, M12_BESTIARY_ATK_SOURCE, NULL, "ReDMCSB DUNGEON.C G0243 PC34 source record C" id, -1, -1 }
    C("00", "MUMMY",             M12_BESTIARY_CAT_UNDEAD,    150),
    C("01", "SWAMP SLIME",       M12_BESTIARY_CAT_BEAST,     110),
    C("02", "GIGGLER",           M12_BESTIARY_CAT_HUMANOID,   10),
    C("03", "WIZARD EYE",        M12_BESTIARY_CAT_BEAST,      40),
    C("04", "PAIN RAT / HELLHOUND", M12_BESTIARY_CAT_BEAST,  101),
    C("05", "RUSTER",            M12_BESTIARY_CAT_BEAST,      60),
    C("06", "SCREAMER",          M12_BESTIARY_CAT_BEAST,     165),
    C("07", "ROCK / ROCKPILE",   M12_BESTIARY_CAT_CONSTRUCT,  50),
    C("08", "GHOST",             M12_BESTIARY_CAT_UNDEAD,     30),
    C("09", "STONE GOLEM",       M12_BESTIARY_CAT_CONSTRUCT, 120),
    C("10", "MUMMY",             M12_BESTIARY_CAT_UNDEAD,     33),
    C("11", "GHOST / RIVE",      M12_BESTIARY_CAT_UNDEAD,     80),
    C("12", "SKELETON",          M12_BESTIARY_CAT_UNDEAD,     20),
    C("13", "COUATL",            M12_BESTIARY_CAT_BEAST,      39),
    C("14", "VEXIRK",            M12_BESTIARY_CAT_DEMON,      44),
    C("15", "MAGENTA WORM / WORM", M12_BESTIARY_CAT_BEAST,   70),
    C("16", "TROLIN / ANTMAN",   M12_BESTIARY_CAT_HUMANOID,   20),
    C("17", "GIANT WASP / MUNCHER", M12_BESTIARY_CAT_BEAST,   8),
    C("18", "ANIMATED ARMOUR / DETH KNIGHT", M12_BESTIARY_CAT_CONSTRUCT, 60),
    C("19", "MATERIALIZER / ZYTAZ", M12_BESTIARY_CAT_DEMON,   33),
    C("20", "OITU",              M12_BESTIARY_CAT_DEMON,     144),
    C("21", "DEMON",             M12_BESTIARY_CAT_DEMON,      77),
    C("22", "DEMON",             M12_BESTIARY_CAT_DEMON,     100),
    C("23", "LORD CHAOS",        M12_BESTIARY_CAT_DEMON,     180),
    C("24", "RED DRAGON",        M12_BESTIARY_CAT_DRAGON,    255),
    C("25", "LORD ORDER",        M12_BESTIARY_CAT_DEMON,     180),
    C("26", "GREY LORD",         M12_BESTIARY_CAT_DEMON,     180),
#undef C
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
