#ifndef FIRESTAFF_DM1_V1_SPELL_EFFECT_RENDER_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_SPELL_EFFECT_RENDER_PC34_COMPAT_H

/*
 * DM1 V1 Spell Effect Rendering & Spell Area Controls
 * Source-locked to ReDMCSB SPELDRAW.C / MENUDRAW.C
 *
 * Draws the spell casting UI: symbol buttons, selected spell display,
 * champion casting indicators, and in-viewport spell visual effects.
 *
 * Source lock (ReDMCSB WIP20210206, Toolchains/Common/Source):
 *   SPELDRAW.C: F0393 (draw spell area controls — champion highlights,
 *               health-based zone enabling)
 *   MENUDRAW.C: F0397 (draw available symbols — 6 per step),
 *               F0396 (load spell area lines bitmap),
 *               included via #include "SPELDRAW.C" in PC34 builds
 *   CASTER.C: spell definition tables (referenced, not implemented here)
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Spell symbol steps — 4 steps of 6 symbols each = 24 total */
#define M11_SPELL_SYMBOL_STEPS    4
#define M11_SYMBOLS_PER_STEP      6
#define M11_MAX_SPELL_SYMBOLS    24
#define M11_MAX_SPELL_LENGTH      4  /* max symbols in one spell */

/* Spell area layout constants */
#define M11_SPELL_AREA_X        233
#define M11_SPELL_AREA_Y          2
#define M11_SPELL_AREA_W         85
#define M11_SPELL_AREA_H         70
#define M11_SPELL_SYMBOL_W       14
#define M11_SPELL_SYMBOL_H       12

/* Spell casting state for rendering */
typedef struct {
    int casterChampionIndex;     /* -1 if no one is casting */
    int currentSymbolStep;       /* 0-3: which set of 6 symbols is shown */
    int selectedSymbols[M11_MAX_SPELL_LENGTH]; /* symbol indices, -1 = empty */
    int selectedCount;           /* how many symbols chosen so far */
    int spellValid;              /* current combination is a known spell */
    int castingInProgress;       /* animation/cooldown active */
    uint32_t castStartMs;
} M11_SpellRenderState;

/* Per-champion casting indicator */
typedef struct {
    int championIndex;
    int currentHealth;
    int canCast;                 /* health > 0 */
    int isActiveCaster;          /* this champion is the current caster */
} M11_SpellChampionInfo;

/* Result from drawing spell area */
typedef struct {
    int areaRedrawn;
    int symbolsDrawn;            /* number of available symbols drawn */
    int championHighlightsDrawn; /* number of champion zones highlighted */
    int spellLineRedrawn;        /* the selected-spell line was updated */
} M11_SpellRenderResult;

/*
 * Initialize spell render state.
 */
void m11_spell_render_init(M11_SpellRenderState *s);

/*
 * Set the active caster and reset spell selection.
 */
void m11_spell_render_set_caster(M11_SpellRenderState *s,
                                  int championIndex);

/*
 * Add a symbol to the current spell (player clicked a symbol button).
 * symbolIndex: 0-5 within the current step.
 * Returns 1 if added, 0 if spell is full.
 */
int m11_spell_render_add_symbol(M11_SpellRenderState *s, int symbolIndex);

/*
 * Remove the last symbol (backspace).
 * Returns 1 if removed, 0 if nothing to remove.
 */
int m11_spell_render_remove_symbol(M11_SpellRenderState *s);

/*
 * Clear spell selection entirely.
 */
void m11_spell_render_clear(M11_SpellRenderState *s);

/*
 * Draw spell area controls (F0393).
 * champInfo: array of 4 champion casting infos.
 * Returns rendering result.
 */
M11_SpellRenderResult m11_spell_render_draw_controls(
    const M11_SpellRenderState *s,
    const M11_SpellChampionInfo champInfo[4],
    int partyChampionCount);

/*
 * Draw available symbols for the current step (F0397).
 * Returns number of symbols drawn (always 6 if step is valid).
 */
int m11_spell_render_draw_symbols(const M11_SpellRenderState *s);

/*
 * Get the global symbol index for a local index in the current step.
 * localIndex: 0-5, step: 0-3.
 * Returns: 96 + 6*step + localIndex (the character code used in original).
 */
int m11_spell_render_symbol_code(int step, int localIndex);

/*
 * Source evidence string.
 */
const char *m11_spell_render_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_SPELL_EFFECT_RENDER_PC34_COMPAT_H */
