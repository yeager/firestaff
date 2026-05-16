#include "item_encyclopedia_m12.h"
#include <stddef.h>
#include <string.h>
#include <ctype.h>

/* ── DM1 Item Database ───────────────────────────────────────────
 *
 * Weights are in tenths-of-kg (matching the original game's internal
 * representation). 25 = 2.5 kg.
 *
 * Descriptions are lore/flavour text drawn from in-game knowledge
 * and the Dungeon Master Encyclopaedia.
 */
static const M12_ItemEntry g_itemDatabase[] = {
    /* ── Weapons ─────────────────────────────────────────────────── */
    {
        "FALCHION",
        M12_ITEM_CAT_WEAPON, 35,
        "A CURVED SINGLE-EDGED BLADE FAVOURED BY DUNGEON\n"
        "FIGHTERS. MODERATE DAMAGE WITH GOOD SWING SPEED."
    },
    {
        "DELTA",
        M12_ITEM_CAT_WEAPON, 30,
        "A THROWING STAR WITH THREE BLADES. LIGHT AND\n"
        "PRECISE, EFFECTIVE AT RANGE AGAINST SOFT TARGETS."
    },
    {
        "VORPAL BLADE",
        M12_ITEM_CAT_WEAPON, 38,
        "A LEGENDARY SWORD WITH AN IMPOSSIBLY SHARP EDGE.\n"
        "SAID TO SEVER HEADS WITH A SINGLE STROKE."
    },
    {
        "RAPIER",
        M12_ITEM_CAT_WEAPON, 22,
        "A SLENDER THRUSTING SWORD. FAST AND ELEGANT,\n"
        "BEST WIELDED BY NIMBLE CHAMPIONS."
    },
    {
        "MACE OF ORDER",
        M12_ITEM_CAT_WEAPON, 48,
        "A HEAVY MACE ENCHANTED WITH LAW MAGIC. DELIVERS\n"
        "CRUSHING BLOWS THAT STUN UNDEAD AND DEMONS."
    },
    {
        "STAFF OF CLAWS",
        M12_ITEM_CAT_WEAPON, 40,
        "A TWISTED WOODEN STAFF TIPPED WITH METAL CLAWS.\n"
        "CHANNELS MANA FOR BOTH MELEE AND SPELL COMBAT."
    },
    {
        "FURY",
        M12_ITEM_CAT_WEAPON, 42,
        "A BROAD AXE ETCHED WITH RUNES OF RAGE. THE\n"
        "WIELDER STRIKES HARDER AS WOUNDS ACCUMULATE."
    },
    {
        "THE FIRESTAFF",
        M12_ITEM_CAT_WEAPON, 28,
        "THE ARTEFACT OF POWER SOUGHT BY ALL WHO ENTER\n"
        "THE DUNGEON. CHANNELER OF IMMENSE MAGICAL FORCE."
    },
    {
        "SLING",
        M12_ITEM_CAT_WEAPON, 10,
        "A SIMPLE LEATHER SLING FOR HURLING STONES AND\n"
        "SMALL PROJECTILES. LIGHT AND EASY TO USE."
    },
    {
        "ARROW",
        M12_ITEM_CAT_WEAPON, 3,
        "A STANDARD WOODEN ARROW WITH IRON TIP. USED\n"
        "WITH BOWS FOR RANGED ATTACKS."
    },
    /* ── Armor ───────────────────────────────────────────────────── */
    {
        "MAIL AKETON",
        M12_ITEM_CAT_ARMOR, 52,
        "A PADDED JACKET REINFORCED WITH MAIL LINKS.\n"
        "GOOD PROTECTION WITHOUT EXCESSIVE WEIGHT."
    },
    {
        "LEG MAIL",
        M12_ITEM_CAT_ARMOR, 44,
        "CHAIN MAIL LEGGINGS COVERING THIGHS AND SHINS.\n"
        "STANDARD ISSUE FOR FRONT-LINE FIGHTERS."
    },
    {
        "HELM OF LYTE",
        M12_ITEM_CAT_ARMOR, 18,
        "AN ENCHANTED HELM THAT GLOWS WITH FAINT LIGHT.\n"
        "PROTECTS THE HEAD AND ILLUMINATES NEARBY AREA."
    },
    {
        "SHIELD OF LYTE",
        M12_ITEM_CAT_ARMOR, 36,
        "A STURDY SHIELD IMBUED WITH PROTECTIVE MAGIC.\n"
        "DEFLECTS BOTH PHYSICAL AND MAGICAL ATTACKS."
    },
    {
        "BOOTS OF SPEED",
        M12_ITEM_CAT_ARMOR, 14,
        "ENCHANTED LEATHER BOOTS THAT QUICKEN THE STEP.\n"
        "THE WEARER MOVES NOTICEABLY FASTER IN COMBAT."
    },
    /* ── Potions ──────────────────────────────────────────────────── */
    {
        "VEN POTION",
        M12_ITEM_CAT_POTION, 4,
        "A VIAL OF THICK GREEN LIQUID. RESTORES A\n"
        "MODERATE AMOUNT OF HEALTH WHEN CONSUMED."
    },
    {
        "MON POTION",
        M12_ITEM_CAT_POTION, 4,
        "A STAMINA RESTORATION BREW. THE BITTER TASTE\n"
        "IS QUICKLY FORGOTTEN AS ENERGY RETURNS."
    },
    {
        "ZO POTION",
        M12_ITEM_CAT_POTION, 4,
        "A SHIMMERING BLUE ELIXIR THAT REPLENISHES\n"
        "MAGICAL MANA RESERVES."
    },
    {
        "ROS POTION",
        M12_ITEM_CAT_POTION, 4,
        "A POTION OF DEXTERITY. TEMPORARILY ENHANCES\n"
        "AGILITY AND REFLEXES OF THE DRINKER."
    },
    {
        "KU POTION",
        M12_ITEM_CAT_POTION, 4,
        "A POTION OF STRENGTH. GRANTS TEMPORARILY\n"
        "INCREASED PHYSICAL POWER."
    },
    /* ── Scrolls ──────────────────────────────────────────────────── */
    {
        "SCROLL OF FIREBALL",
        M12_ITEM_CAT_SCROLL, 1,
        "A PARCHMENT INSCRIBED WITH THE FUL IR SPELL.\n"
        "INVOKES A DEVASTATING BALL OF FIRE WHEN READ."
    },
    {
        "SCROLL OF LIGHTNING",
        M12_ITEM_CAT_SCROLL, 1,
        "A PARCHMENT BEARING THE OH KATH RA INCANTATION.\n"
        "UNLEASHES A BOLT OF LIGHTNING AT THE TARGET."
    },
    {
        "SCROLL OF DARKNESS",
        M12_ITEM_CAT_SCROLL, 1,
        "A BLACK-EDGED SCROLL THAT PLUNGES THE AREA\n"
        "INTO MAGICAL DARKNESS WHEN INVOKED."
    },
    /* ── Miscellaneous ────────────────────────────────────────────── */
    {
        "TORCH",
        M12_ITEM_CAT_MISC, 14,
        "A WOODEN TORCH SOAKED IN OIL. PROVIDES RELIABLE\n"
        "LIGHT BUT BURNS DOWN OVER TIME."
    },
    {
        "COMPASS",
        M12_ITEM_CAT_MISC, 3,
        "A BRASS COMPASS THAT ALWAYS POINTS NORTH.\n"
        "INVALUABLE FOR NAVIGATION IN THE DEEP LEVELS."
    },
    {
        "ROPE",
        M12_ITEM_CAT_MISC, 20,
        "A STURDY HEMP ROPE. USEFUL FOR CLIMBING AND\n"
        "TRAVERSING PITS IN THE DUNGEON."
    },
    {
        "SKELETON KEY",
        M12_ITEM_CAT_MISC, 2,
        "A TARNISHED IRON KEY WITH MANY WARDS. OPENS\n"
        "SEVERAL COMMON LOCKS IN THE UPPER LEVELS."
    },
    {
        "GOLD COIN",
        M12_ITEM_CAT_MISC, 1,
        "A COIN STAMPED WITH THE GREY LORD'S SIGIL.\n"
        "SOMETIMES USEFUL AS A COUNTERWEIGHT."
    },
    {
        "ILLUMULET",
        M12_ITEM_CAT_MISC, 5,
        "A SMALL AMULET THAT EMITS A STEADY MAGICAL\n"
        "GLOW. PROVIDES PERMANENT HANDS-FREE LIGHT."
    },
    {
        "CHEST",
        M12_ITEM_CAT_MISC, 60,
        "A HEAVY WOODEN CHEST. CAN STORE MULTIPLE ITEMS\n"
        "AND BE CARRIED OR PLACED ON THE GROUND."
    },
};

#define G_ITEM_DATABASE_COUNT \
    ((int)(sizeof(g_itemDatabase) / sizeof(g_itemDatabase[0])))

/* ── Case-insensitive substring search ───────────────────────────── */
static int m12_item_strcasestr(const char* haystack, const char* needle) {
    int hLen, nLen, i, j;
    if (!needle || !needle[0]) return 1;
    if (!haystack) return 0;
    hLen = (int)strlen(haystack);
    nLen = (int)strlen(needle);
    if (nLen > hLen) return 0;
    for (i = 0; i <= hLen - nLen; i++) {
        for (j = 0; j < nLen; j++) {
            if (toupper((unsigned char)haystack[i + j]) !=
                toupper((unsigned char)needle[j])) {
                break;
            }
        }
        if (j == nLen) return 1;
    }
    return 0;
}

/* ── Check if an item matches current filter + search ────────────── */
static int m12_item_matches(const M12_ItemEntry* item,
                            M12_ItemCategory catFilter,
                            const char* searchQuery) {
    if (catFilter != M12_ITEM_CAT_ALL && item->category != catFilter) {
        return 0;
    }
    if (searchQuery && searchQuery[0]) {
        if (!m12_item_strcasestr(item->name, searchQuery)) {
            return 0;
        }
    }
    return 1;
}

/* ── Recount filtered items ──────────────────────────────────────── */
static int m12_item_recount(M12_ItemCategory catFilter,
                            const char* searchQuery) {
    int count = 0;
    int i;
    for (i = 0; i < G_ITEM_DATABASE_COUNT; i++) {
        if (m12_item_matches(&g_itemDatabase[i], catFilter, searchQuery)) {
            count++;
        }
    }
    return count;
}

/* ── Get nth matching item ───────────────────────────────────────── */
static const M12_ItemEntry* m12_item_get_nth(M12_ItemCategory catFilter,
                                              const char* searchQuery,
                                              int n) {
    int count = 0;
    int i;
    for (i = 0; i < G_ITEM_DATABASE_COUNT; i++) {
        if (m12_item_matches(&g_itemDatabase[i], catFilter, searchQuery)) {
            if (count == n) return &g_itemDatabase[i];
            count++;
        }
    }
    return NULL;
}

/* ── Public API ──────────────────────────────────────────────────── */

void M12_ItemEncyclopedia_Init(M12_ItemEncyclopediaState* es) {
    if (!es) return;
    es->scrollOffset = 0;
    es->selectedIndex = 0;
    es->categoryFilter = M12_ITEM_CAT_ALL;
    es->filteredCount = G_ITEM_DATABASE_COUNT;
    memset(es->searchQuery, 0, M12_ITEM_SEARCH_MAX);
    es->searchLen = 0;
}

int M12_ItemEncyclopedia_TotalCount(void) {
    return G_ITEM_DATABASE_COUNT;
}

int M12_ItemEncyclopedia_FilteredCount(const M12_ItemEncyclopediaState* es) {
    if (!es) return 0;
    return es->filteredCount;
}

const M12_ItemEntry* M12_ItemEncyclopedia_GetFiltered(
        const M12_ItemEncyclopediaState* es, int index) {
    if (!es || index < 0 || index >= es->filteredCount) return NULL;
    return m12_item_get_nth(es->categoryFilter, es->searchQuery, index);
}

const M12_ItemEntry* M12_ItemEncyclopedia_GetSelected(
        const M12_ItemEncyclopediaState* es) {
    if (!es) return NULL;
    return M12_ItemEncyclopedia_GetFiltered(es, es->selectedIndex);
}

void M12_ItemEncyclopedia_Scroll(M12_ItemEncyclopediaState* es, int delta) {
    int maxOffset;
    if (!es) return;
    es->selectedIndex += delta;
    if (es->selectedIndex < 0) {
        es->selectedIndex = 0;
    }
    if (es->selectedIndex >= es->filteredCount) {
        es->selectedIndex = es->filteredCount > 0
                          ? es->filteredCount - 1
                          : 0;
    }
    /* Adjust scroll window to keep selection visible */
    if (es->selectedIndex < es->scrollOffset) {
        es->scrollOffset = es->selectedIndex;
    }
    if (es->selectedIndex >= es->scrollOffset + M12_ITEM_VISIBLE_LINES) {
        es->scrollOffset = es->selectedIndex - M12_ITEM_VISIBLE_LINES + 1;
    }
    maxOffset = es->filteredCount - M12_ITEM_VISIBLE_LINES;
    if (maxOffset < 0) maxOffset = 0;
    if (es->scrollOffset > maxOffset) {
        es->scrollOffset = maxOffset;
    }
    if (es->scrollOffset < 0) {
        es->scrollOffset = 0;
    }
}

void M12_ItemEncyclopedia_CycleCategory(M12_ItemEncyclopediaState* es,
                                        int direction) {
    int cat;
    if (!es) return;
    cat = (int)es->categoryFilter + direction;
    if (cat < 0) cat = M12_ITEM_CAT_COUNT - 1;
    if (cat >= M12_ITEM_CAT_COUNT) cat = 0;
    es->categoryFilter = (M12_ItemCategory)cat;
    es->scrollOffset = 0;
    es->selectedIndex = 0;
    es->filteredCount = m12_item_recount(es->categoryFilter,
                                         es->searchQuery);
}

const char* M12_ItemEncyclopedia_CategoryName(M12_ItemCategory cat) {
    switch (cat) {
        case M12_ITEM_CAT_ALL:    return "ALL";
        case M12_ITEM_CAT_WEAPON: return "WEAPONS";
        case M12_ITEM_CAT_ARMOR:  return "ARMOR";
        case M12_ITEM_CAT_POTION: return "POTIONS";
        case M12_ITEM_CAT_SCROLL: return "SCROLLS";
        case M12_ITEM_CAT_MISC:   return "MISCELLANEOUS";
        default:                  return "UNKNOWN";
    }
}

void M12_ItemEncyclopedia_SetSearch(M12_ItemEncyclopediaState* es,
                                    const char* query) {
    if (!es) return;
    if (!query || !query[0]) {
        memset(es->searchQuery, 0, M12_ITEM_SEARCH_MAX);
        es->searchLen = 0;
    } else {
        int len = (int)strlen(query);
        if (len >= M12_ITEM_SEARCH_MAX) len = M12_ITEM_SEARCH_MAX - 1;
        memcpy(es->searchQuery, query, (size_t)len);
        es->searchQuery[len] = '\0';
        es->searchLen = len;
    }
    es->scrollOffset = 0;
    es->selectedIndex = 0;
    es->filteredCount = m12_item_recount(es->categoryFilter,
                                         es->searchQuery);
}

void M12_ItemEncyclopedia_SearchAppend(M12_ItemEncyclopediaState* es,
                                       char ch) {
    if (!es) return;
    if (es->searchLen >= M12_ITEM_SEARCH_MAX - 1) return;
    es->searchQuery[es->searchLen] = ch;
    es->searchLen++;
    es->searchQuery[es->searchLen] = '\0';
    es->scrollOffset = 0;
    es->selectedIndex = 0;
    es->filteredCount = m12_item_recount(es->categoryFilter,
                                         es->searchQuery);
}

void M12_ItemEncyclopedia_SearchBackspace(M12_ItemEncyclopediaState* es) {
    if (!es || es->searchLen <= 0) return;
    es->searchLen--;
    es->searchQuery[es->searchLen] = '\0';
    es->scrollOffset = 0;
    es->selectedIndex = 0;
    es->filteredCount = m12_item_recount(es->categoryFilter,
                                         es->searchQuery);
}

void M12_ItemEncyclopedia_SearchClear(M12_ItemEncyclopediaState* es) {
    if (!es) return;
    memset(es->searchQuery, 0, M12_ITEM_SEARCH_MAX);
    es->searchLen = 0;
    es->scrollOffset = 0;
    es->selectedIndex = 0;
    es->filteredCount = m12_item_recount(es->categoryFilter,
                                         es->searchQuery);
}
