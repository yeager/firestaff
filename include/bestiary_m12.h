#ifndef FIRESTAFF_BESTIARY_M12_H
#define FIRESTAFF_BESTIARY_M12_H

#ifdef __cplusplus
extern "C" {
#endif

/* ── Bestiary / Monster Encyclopedia for the Firestaff launcher ───
 *
 * A browsable database of DM1 creatures with stats, descriptions,
 * and cross-references to creature_art_m12.h thumbnail data.
 * Accessible from the launcher menu as a dedicated view.
 */

/* ── Creature categories ─────────────────────────────────────────── */
typedef enum {
    M12_BESTIARY_CAT_ALL = 0,
    M12_BESTIARY_CAT_HUMANOID,
    M12_BESTIARY_CAT_UNDEAD,
    M12_BESTIARY_CAT_BEAST,
    M12_BESTIARY_CAT_CONSTRUCT,
    M12_BESTIARY_CAT_DEMON,
    M12_BESTIARY_CAT_DRAGON,
    M12_BESTIARY_CAT_COUNT
} M12_BestiaryCategory;

/* ── Attack type ─────────────────────────────────────────────────── */
typedef enum {
    M12_BESTIARY_ATK_MELEE = 0,
    M12_BESTIARY_ATK_RANGED,
    M12_BESTIARY_ATK_MAGIC,
    M12_BESTIARY_ATK_POISON,
    M12_BESTIARY_ATK_FIRE
} M12_BestiaryAttackType;

/* ── Single creature entry ───────────────────────────────────────── */
typedef struct {
    const char*            name;        /* Display name (uppercase)    */
    M12_BestiaryCategory   category;
    int                    hpMin;       /* HP range low end            */
    int                    hpMax;       /* HP range high end           */
    M12_BestiaryAttackType attackType;
    const char*            weakness;    /* Primary weakness (text)     */
    const char*            description; /* Lore / flavour text         */
    int                    artIndex;    /* Index into creature_art_m12
                                          thumbnails, or -1 if none   */
    int                    dungeonLevel;/* Typical first encounter     */
} M12_BestiaryEntry;

#define M12_BESTIARY_VISIBLE_LINES 10

/* ── Scroll / filter state ───────────────────────────────────────── */
typedef struct {
    int                    scrollOffset;    /* First visible entry     */
    int                    selectedIndex;   /* Cursor within filtered  */
    M12_BestiaryCategory   categoryFilter;  /* Current filter          */
    int                    filteredCount;   /* Cached count            */
} M12_BestiaryState;

/* Initialise the bestiary state (resets filter to ALL). */
void M12_Bestiary_Init(M12_BestiaryState* bs);

/* Total number of creatures in the database. */
int  M12_Bestiary_TotalCount(void);

/* Number of creatures matching the current category filter. */
int  M12_Bestiary_FilteredCount(const M12_BestiaryState* bs);

/* Get the nth entry in the filtered list (0-based).
 * Returns NULL if out of range. */
const M12_BestiaryEntry* M12_Bestiary_GetFiltered(
    const M12_BestiaryState* bs, int index);

/* Get the currently selected entry (under cursor). */
const M12_BestiaryEntry* M12_Bestiary_GetSelected(
    const M12_BestiaryState* bs);

/* Scroll by delta entries (positive = down). Clamps to bounds. */
void M12_Bestiary_Scroll(M12_BestiaryState* bs, int delta);

/* Cycle category filter forward (+1) or backward (-1).
 * Resets scroll and selection to 0. */
void M12_Bestiary_CycleCategory(M12_BestiaryState* bs, int direction);

/* Return display name for a category enum value. */
const char* M12_Bestiary_CategoryName(M12_BestiaryCategory cat);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_BESTIARY_M12_H */
