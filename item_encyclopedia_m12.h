#ifndef FIRESTAFF_ITEM_ENCYCLOPEDIA_M12_H
#define FIRESTAFF_ITEM_ENCYCLOPEDIA_M12_H

#ifdef __cplusplus
extern "C" {
#endif

/* ── Item Encyclopedia for the Firestaff launcher ─────────────────
 *
 * A browsable, searchable database of DM1 items with categories,
 * weights, and lore descriptions. Accessible from the launcher menu
 * as a dedicated view under Museum of Lore.
 */

/* ── Item categories ─────────────────────────────────────────────── */
typedef enum {
    M12_ITEM_CAT_ALL = 0,
    M12_ITEM_CAT_WEAPON,
    M12_ITEM_CAT_ARMOR,
    M12_ITEM_CAT_POTION,
    M12_ITEM_CAT_SCROLL,
    M12_ITEM_CAT_MISC,
    M12_ITEM_CAT_COUNT
} M12_ItemCategory;

/* ── Single item entry ───────────────────────────────────────────── */
typedef struct {
    const char*       name;        /* Display name (uppercase)       */
    M12_ItemCategory  category;
    int               weight;      /* In tenths-of-kg (25 = 2.5 kg) */
    const char*       description; /* Lore / flavour text            */
} M12_ItemEntry;

#define M12_ITEM_VISIBLE_LINES 10
#define M12_ITEM_SEARCH_MAX    32

/* ── Scroll / filter / search state ──────────────────────────────── */
typedef struct {
    int               scrollOffset;    /* First visible entry        */
    int               selectedIndex;   /* Cursor within filtered     */
    M12_ItemCategory  categoryFilter;  /* Current filter             */
    int               filteredCount;   /* Cached count               */
    char              searchQuery[M12_ITEM_SEARCH_MAX];
    int               searchLen;       /* Current query length       */
} M12_ItemEncyclopediaState;

/* Initialise the encyclopedia state (resets filter to ALL). */
void M12_ItemEncyclopedia_Init(M12_ItemEncyclopediaState* es);

/* Total number of items in the database. */
int  M12_ItemEncyclopedia_TotalCount(void);

/* Number of items matching the current category filter and search. */
int  M12_ItemEncyclopedia_FilteredCount(const M12_ItemEncyclopediaState* es);

/* Get the nth entry in the filtered list (0-based).
 * Returns NULL if out of range. */
const M12_ItemEntry* M12_ItemEncyclopedia_GetFiltered(
    const M12_ItemEncyclopediaState* es, int index);

/* Get the currently selected entry (under cursor). */
const M12_ItemEntry* M12_ItemEncyclopedia_GetSelected(
    const M12_ItemEncyclopediaState* es);

/* Scroll by delta entries (positive = down). Clamps to bounds. */
void M12_ItemEncyclopedia_Scroll(M12_ItemEncyclopediaState* es, int delta);

/* Cycle category filter forward (+1) or backward (-1).
 * Resets scroll and selection to 0. */
void M12_ItemEncyclopedia_CycleCategory(M12_ItemEncyclopediaState* es,
                                        int direction);

/* Return display name for a category enum value. */
const char* M12_ItemEncyclopedia_CategoryName(M12_ItemCategory cat);

/* Search by name substring (case-insensitive).
 * Pass NULL or "" to clear the search filter.
 * Resets scroll and selection to 0. */
void M12_ItemEncyclopedia_SetSearch(M12_ItemEncyclopediaState* es,
                                    const char* query);

/* Append a character to the current search query. */
void M12_ItemEncyclopedia_SearchAppend(M12_ItemEncyclopediaState* es,
                                       char ch);

/* Remove the last character from the search query. */
void M12_ItemEncyclopedia_SearchBackspace(M12_ItemEncyclopediaState* es);

/* Clear the search query. */
void M12_ItemEncyclopedia_SearchClear(M12_ItemEncyclopediaState* es);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_ITEM_ENCYCLOPEDIA_M12_H */
