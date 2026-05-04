#ifndef FIRESTAFF_SPELL_REFERENCE_M12_H
#define FIRESTAFF_SPELL_REFERENCE_M12_H

#ifdef __cplusplus
extern "C" {
#endif

/* ── Spell Reference / Encyclopedia for the Firestaff launcher ────
 *
 * A browsable, searchable database of all 25 DM1 spells with rune
 * sequences, mana costs, skill requirements, effect descriptions,
 * and spell-kind categorisation. Accessible from the launcher menu
 * as a dedicated view under Museum of Lore.
 *
 * Data sourced from the DM1 spell table (MENU.C:50..77) and the
 * M10 Phase 14 magic module (memory_magic_pc34_compat).
 */

/* ── Spell schools (categories for filtering) ────────────────────── */
typedef enum {
    M12_SPELL_SCHOOL_ALL = 0,
    M12_SPELL_SCHOOL_FIRE,      /* Ful-rooted spells             */
    M12_SPELL_SCHOOL_AIR,       /* Oh-rooted spells              */
    M12_SPELL_SCHOOL_EARTH,     /* Des-rooted spells             */
    M12_SPELL_SCHOOL_WATER,     /* Zo-rooted spells              */
    M12_SPELL_SCHOOL_BODY,      /* Ya-rooted spells (potions)    */
    M12_SPELL_SCHOOL_MIND,      /* Vi-rooted spells              */
    M12_SPELL_SCHOOL_COUNT
} M12_SpellSchool;

/* ── Spell kind (matches original engine kinds) ──────────────────── */
typedef enum {
    M12_SPELL_KIND_POTION = 1,
    M12_SPELL_KIND_PROJECTILE = 2,
    M12_SPELL_KIND_OTHER = 3,
    M12_SPELL_KIND_MAGIC_MAP = 4
} M12_SpellKind;

/* ── Single spell entry ──────────────────────────────────────────── */
typedef struct {
    const char*      name;            /* Display name (uppercase)    */
    M12_SpellSchool  school;
    M12_SpellKind    kind;
    const char*      runeSequence;    /* Rune symbols, e.g. "FUL IR" */
    int              runeCount;       /* Number of runes (1-4)       */
    int              baseMana;        /* Base mana cost (LO power)   */
    int              baseSkillLevel;  /* Minimum skill level required */
    const char*      skillName;       /* Governing skill name        */
    const char*      effectSummary;   /* Short one-line effect       */
    const char*      description;     /* Full lore / strategy text   */
    int              iconIndex;       /* UI icon reference, -1=none  */
} M12_SpellReferenceEntry;

#define M12_SPELL_VISIBLE_LINES 10
#define M12_SPELL_SEARCH_MAX    32

/* ── Scroll / filter / search state ──────────────────────────────── */
typedef struct {
    int               scrollOffset;    /* First visible entry        */
    int               selectedIndex;   /* Cursor within filtered     */
    M12_SpellSchool   schoolFilter;    /* Current school filter      */
    int               filteredCount;   /* Cached count               */
    char              searchQuery[M12_SPELL_SEARCH_MAX];
    int               searchLen;       /* Current query length       */
} M12_SpellReferenceState;

/* Initialise the spell reference state (resets filter to ALL). */
void M12_SpellReference_Init(M12_SpellReferenceState* sr);

/* Total number of spells in the database. */
int  M12_SpellReference_TotalCount(void);

/* Number of spells matching the current school filter and search. */
int  M12_SpellReference_FilteredCount(const M12_SpellReferenceState* sr);

/* Get the nth entry in the filtered list (0-based).
 * Returns NULL if out of range. */
const M12_SpellReferenceEntry* M12_SpellReference_GetFiltered(
    const M12_SpellReferenceState* sr, int index);

/* Get the currently selected entry (under cursor). */
const M12_SpellReferenceEntry* M12_SpellReference_GetSelected(
    const M12_SpellReferenceState* sr);

/* Scroll by delta entries (positive = down). Clamps to bounds. */
void M12_SpellReference_Scroll(M12_SpellReferenceState* sr, int delta);

/* Cycle school filter forward (+1) or backward (-1).
 * Resets scroll and selection to 0. */
void M12_SpellReference_CycleSchool(M12_SpellReferenceState* sr,
                                     int direction);

/* Return display name for a school enum value. */
const char* M12_SpellReference_SchoolName(M12_SpellSchool school);

/* Return display name for a spell kind enum value. */
const char* M12_SpellReference_KindName(M12_SpellKind kind);

/* Search by name or rune substring (case-insensitive).
 * Pass NULL or "" to clear the search filter.
 * Resets scroll and selection to 0. */
void M12_SpellReference_SetSearch(M12_SpellReferenceState* sr,
                                   const char* query);

/* Append a character to the current search query. */
void M12_SpellReference_SearchAppend(M12_SpellReferenceState* sr,
                                      char ch);

/* Remove the last character from the search query. */
void M12_SpellReference_SearchBackspace(M12_SpellReferenceState* sr);

/* Clear the search query. */
void M12_SpellReference_SearchClear(M12_SpellReferenceState* sr);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_SPELL_REFERENCE_M12_H */
