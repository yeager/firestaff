/**
 * dm1_v1_spell_casting_pc34_compat.h — DM1 V1 Spell Casting System
 *
 * Source-locked to ReDMCSB (WIP20210206):
 *   SYMBOL.C      — F0399_MENUS_AddChampionSymbol, F0400_MENUS_DeleteChampionSymbol
 *   MENU.C        — F0409_MENUS_GetSpellFromSymbols, F0412_MENUS_GetChampionSpellCastResult,
 *                   G0485_aauc_Graphic560_SymbolBaseManaCost[4][6],
 *                   G0486_auc_Graphic560_SymbolManaCostMultiplier[6],
 *                   G0487_as_Graphic560_Spells[25]
 *   CHAMPION.C    — F0327_CHAMPION_IsProjectileSpellCast
 *   DEFS.H        — SPELL struct, spell kind/type/attribute macros, symbol step,
 *                   Champion.Symbols[5], Champion.SymbolStep
 *
 * Symbol encoding (DEFS.H:632, SYMBOL.C:36):
 *   Character = 96 + (SymbolStep * 6) + SymbolIndex
 *   Step 0: power symbols  (chars 96-101: Lo Um On Ee Pal Mon)
 *   Step 1: element symbols (chars 102-107: Ya Vi Oh Ful Des Zo)
 *   Step 2: class symbols   (chars 108-113: Ven Ew Kath Ir Bro Gor)
 *   Step 3: alignment syms  (chars 114-119: Ku Ros Dain Neta Ra Sar)
 */
#ifndef FIRESTAFF_DM1_V1_SPELL_CASTING_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_SPELL_CASTING_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Symbol step indices (DEFS.H:632 SymbolStep values 0..3) ──── */
#define DM1_SYMBOL_STEP_POWER    0
#define DM1_SYMBOL_STEP_ELEMENT  1
#define DM1_SYMBOL_STEP_CLASS    2
#define DM1_SYMBOL_STEP_ALIGN    3
#define DM1_SYMBOL_STEP_COUNT    4
#define DM1_SYMBOLS_PER_STEP     6
#define DM1_MAX_SPELL_SYMBOLS    4  /* Champion.Symbols[5] is null-terminated */

/* ── Symbol indices within each step (0..5) ────────────────────── */
/* Step 0 — Power (MENU.C:865, char 96 + index) */
#define DM1_POWER_LO   0  /* weakest  */
#define DM1_POWER_UM   1
#define DM1_POWER_ON   2
#define DM1_POWER_EE   3
#define DM1_POWER_PAL  4
#define DM1_POWER_MON  5  /* strongest */

/* Step 1 — Element (char 102 + index) */
#define DM1_ELEM_YA    0
#define DM1_ELEM_VI    1
#define DM1_ELEM_OH    2
#define DM1_ELEM_FUL   3
#define DM1_ELEM_DES   4
#define DM1_ELEM_ZO    5

/* Step 2 — Class (char 108 + index) */
#define DM1_CLASS_VEN  0
#define DM1_CLASS_EW   1
#define DM1_CLASS_KATH 2
#define DM1_CLASS_IR   3
#define DM1_CLASS_BRO  4
#define DM1_CLASS_GOR  5

/* Step 3 — Alignment (char 114 + index) */
#define DM1_ALIGN_KU   0
#define DM1_ALIGN_ROS  1
#define DM1_ALIGN_DAIN 2
#define DM1_ALIGN_NETA 3
#define DM1_ALIGN_RA   4
#define DM1_ALIGN_SAR  5

/* ── Spell kinds (DEFS.H:1760-1763) ────────────────────────────── */
#define DM1_SPELL_KIND_POTION     1
#define DM1_SPELL_KIND_PROJECTILE 2
#define DM1_SPELL_KIND_OTHER      3

/* ── Spell types — projectile (DEFS.H:1766) ────────────────────── */
#define DM1_SPELL_TYPE_PROJ_OPEN_DOOR 4

/* ── Spell types — other (DEFS.H:1767-1775) ────────────────────── */
#define DM1_SPELL_TYPE_OTHER_LIGHT        0
#define DM1_SPELL_TYPE_OTHER_DARKNESS      1
#define DM1_SPELL_TYPE_OTHER_THIEVES_EYE   2
#define DM1_SPELL_TYPE_OTHER_INVISIBILITY  3
#define DM1_SPELL_TYPE_OTHER_PARTY_SHIELD  4
#define DM1_SPELL_TYPE_OTHER_MAGIC_TORCH   5
#define DM1_SPELL_TYPE_OTHER_FOOTPRINTS    6
#define DM1_SPELL_TYPE_OTHER_ZOKATHRA      7
#define DM1_SPELL_TYPE_OTHER_FIRESHIELD    8

/* ── Cast result codes (DEFS.H:2939-2941) ──────────────────────── */
#define DM1_SPELL_CAST_FAILURE             0
#define DM1_SPELL_CAST_SUCCESS             1
#define DM1_SPELL_CAST_FAILURE_NEEDS_FLASK 3

/* ── Failure message types (DEFS.H:2932-2935) ──────────────────── */
#define DM1_FAILURE_NEEDS_MORE_PRACTICE    0
#define DM1_FAILURE_MEANINGLESS_SPELL      1
#define DM1_FAILURE_NEEDS_FLASK_IN_HAND   10
#define DM1_FAILURE_NEEDS_MAGIC_MAP_IN_HAND 11

typedef struct {
    int failureType;
    int messageColor;
    int printsLineFeed;
    int printsChampionName;
    int appendsBaseSkillName;
    int clearsSymbolsOnCastClick;
    int redrawsAvailableSymbols;
    int redrawsChampionSymbols;
    const char* messageBeforeSkill;
    const char* messageAfterSkill;
} DM1_SpellFailureFeedback;

/* ── Skill indices (DEFS.H:757-776) ────────────────────────────── */
#define DM1_SKILL_FIGHTER    0
#define DM1_SKILL_NINJA      1
#define DM1_SKILL_PRIEST     2
#define DM1_SKILL_WIZARD     3
#define DM1_SKILL_IDENTIFY  12
#define DM1_SKILL_HEAL      13
#define DM1_SKILL_INFLUENCE 14
#define DM1_SKILL_DEFEND    15
#define DM1_SKILL_FIRE      16
#define DM1_SKILL_AIR       17
#define DM1_SKILL_EARTH     18
#define DM1_SKILL_WATER     19

/* ── SPELL struct (DEFS.H:1748-1752) ───────────────────────────── */
typedef struct {
    int32_t  symbols;                /* Packed symbol bytes, MSB = power byte */
    uint8_t  baseRequiredSkillLevel;
    uint8_t  skillIndex;
    uint16_t attributes;             /* bits 3-0: Kind, 9-4: Type, 15-10: Duration */
} DM1_Spell;

/* Spell attribute accessors (DEFS.H:1755-1757) */
#define DM1_SPELL_KIND(sp)          ((sp)->attributes & 0x000F)
#define DM1_SPELL_TYPE(sp)          (((sp)->attributes >> 4) & 0x003F)
#define DM1_SPELL_DISABLED_TICKS(sp) (((sp)->attributes >> 10) & 0x003F)

/* ── Spell table (25 DM1 spells) (MENU.C:50-76) ────────────────── */
#define DM1_SPELL_COUNT 25
extern const DM1_Spell dm1_spells[DM1_SPELL_COUNT];

/* ── Mana cost tables (MENU.C:44-49) ───────────────────────────── */
extern const uint8_t dm1_symbolBaseManaCost[DM1_SYMBOL_STEP_COUNT][DM1_SYMBOLS_PER_STEP];
extern const uint8_t dm1_symbolManaCostMultiplier[DM1_SYMBOLS_PER_STEP];

/* ── Per-champion spell state (mirrors Champion.SymbolStep + Symbols) ── */
typedef struct {
    uint8_t symbolStep;             /* 0..3 (DEFS.H:632) */
    char    symbols[5];             /* Null-terminated, up to 4 symbols (DEFS.H:633) */
} DM1_ChampionSpellInput;

/* ── Aggregate spell state ─────────────────────────────────────── */
typedef struct {
    DM1_ChampionSpellInput input[4]; /* Per-champion symbol input state */
    int magicCasterIndex;            /* G0514_i_MagicCasterChampionIndex, -1 = none */
} DM1_SpellCastingState;

/*
 * Champion stats interface — the spell system reads currentMana,
 * maximumMana, currentHealth, wisdom, and skill levels from external state.
 * This avoids duplicating the full champion structure here.
 */
typedef struct {
    int16_t currentMana;
    int16_t maximumMana;
    int16_t currentHealth;
    uint8_t wisdom;         /* Statistics[C3_STATISTIC_WISDOM][C1_CURRENT] */
    uint8_t skillLevels[20]; /* Indexed by DM1_SKILL_* constants */
} DM1_ChampionSpellStats;

/* ── Symbol encoding helpers ───────────────────────────────────── */

/** Encode a symbol step + index into the stored character (SYMBOL.C:36). */
static inline char dm1_encodeSymbol(int step, int index) {
    return (char)(96 + (step * 6) + index);
}

/** Decode a symbol character back to its step. */
static inline int dm1_symbolCharStep(char sym) {
    return ((int)(unsigned char)sym - 96) / 6;
}

/** Decode a symbol character back to its index within its step. */
static inline int dm1_symbolCharIndex(char sym) {
    return ((int)(unsigned char)sym - 96) % 6;
}

/* ── Core spell system API ─────────────────────────────────────── */

/** Initialize all spell casting state. */
void dm1_spell_init(DM1_SpellCastingState* s);

/**
 * Add a symbol to a champion's spell input (source: SYMBOL.C F0399).
 *
 * @param s         Spell casting state
 * @param champIdx  Champion index (0-3)
 * @param stats     Champion's current mana stats (modified: currentMana deducted)
 * @param symbolIdx Symbol index within current step (0-5)
 * @return 1 on success (symbol added, mana deducted), 0 on insufficient mana
 */
int dm1_spell_addSymbol(DM1_SpellCastingState* s, int champIdx,
                        DM1_ChampionSpellStats* stats, int symbolIdx);

/**
 * Delete (recant) the last symbol (source: SYMBOL.C F0400).
 * Does NOT refund mana (matches original behavior).
 */
void dm1_spell_deleteSymbol(DM1_SpellCastingState* s, int champIdx);

/**
 * Look up a spell from the champion's current symbol sequence (source: MENU.C F0409).
 * @return Pointer to matching spell in dm1_spells[], or NULL if no match.
 */
const DM1_Spell* dm1_spell_lookup(const DM1_SpellCastingState* s, int champIdx);

/**
 * Compute the mana cost for adding a symbol at the current step (source: SYMBOL.C F0399).
 */
int dm1_spell_symbolManaCost(const DM1_SpellCastingState* s, int champIdx, int symbolIdx);

/**
 * Attempt to cast the current spell (source: MENU.C F0412).
 *
 * @param s         Spell casting state
 * @param champIdx  Champion index
 * @param stats     Champion stats (mana modified for projectile spells)
 * @param rng16     16-bit random value (caller provides; original uses F0027)
 * @param[out] outSpell If non-NULL, set to the matched spell on success
 * @param[out] outPowerOrdinal If non-NULL, set to power symbol ordinal (1-6)
 * @param[out] outFailure If non-NULL, set to failure type on failure
 * @return DM1_SPELL_CAST_SUCCESS, DM1_SPELL_CAST_FAILURE, or DM1_SPELL_CAST_FAILURE_NEEDS_FLASK
 */
int dm1_spell_cast(DM1_SpellCastingState* s, int champIdx,
                   DM1_ChampionSpellStats* stats, uint16_t rng16,
                   const DM1_Spell** outSpell, int* outPowerOrdinal,
                   int* outFailure);

/**
 * Return source-locked failure message/redraw metadata (SPELFAIL.C F0410
 * plus MENU.C F0408 cast-click cleanup policy), or NULL for unknown failures.
 */
const DM1_SpellFailureFeedback* dm1_spell_failureFeedback(int failureType);

/** MENU.C F0408 cleanup predicate: all cast results except needs-flask clear. */
int dm1_spell_castClearsSymbolsForResult(int castResult);

/** Apply F0408 symbol clear/redraw state effect for the cast-click result. */
void dm1_spell_applyCastClickSymbolFeedback(DM1_SpellCastingState* s,
                                           int champIdx,
                                           int castResult);

/**
 * Compute projectile kinetic energy (source: MENU.C F0412 case C2).
 */
int dm1_spell_projectileKineticEnergy(int powerOrdinal, int skillLevel, int spellType);

/**
 * Compute projectile step energy (source: CHAMPION.C F0327).
 */
int dm1_spell_projectileStepEnergy(int16_t maximumMana);

/**
 * Compute the experience gained from a spell cast (source: MENU.C F0412).
 */
uint16_t dm1_spell_experience(int powerOrdinal, int baseRequiredSkill, int rng8);

/** Get the name string for a symbol character. */
const char* dm1_spell_symbolName(char sym);

/** Get a human-readable name for a spell (from its index in dm1_spells). */
const char* dm1_spell_name(int spellIndex);

/** Source anchors for the spell failure feedback contract. */
const char *dm1_spell_failure_feedback_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_SPELL_CASTING_PC34_COMPAT_H */
