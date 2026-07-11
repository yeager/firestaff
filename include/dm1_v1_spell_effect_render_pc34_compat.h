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
#define DM1_V1_SPELL_SYMBOL_STEPS_PC34    4
#define DM1_V1_SYMBOLS_PER_STEP_PC34      6
#define DM1_V1_MAX_SPELL_SYMBOLS_PC34    24
#define DM1_V1_MAX_SPELL_LENGTH_PC34      4  /* max symbols in one spell */

/* Spell area layout constants */
#define DM1_V1_SPELL_AREA_X_PC34        233
#define DM1_V1_SPELL_AREA_Y_PC34          2
#define DM1_V1_SPELL_AREA_W_PC34         85
#define DM1_V1_SPELL_AREA_H_PC34         70
#define DM1_V1_SPELL_SYMBOL_W_PC34       14
#define DM1_V1_SPELL_SYMBOL_H_PC34       12

/* Spell casting state for rendering */
typedef struct {
    int casterChampionIndex;     /* -1 if no one is casting */
    int currentSymbolStep;       /* 0-3: which set of 6 symbols is shown */
    int selectedSymbols[DM1_V1_MAX_SPELL_LENGTH_PC34]; /* symbol indices, -1 = empty */
    int selectedCount;           /* how many symbols chosen so far */
    int spellValid;              /* current combination is a known spell */
    int castingInProgress;       /* animation/cooldown active */
    uint32_t castStartMs;
} DM1_V1_SpellRenderStatePc34;

/* Per-champion casting indicator */
typedef struct {
    int championIndex;
    int currentHealth;
    int canCast;                 /* health > 0 */
    int isActiveCaster;          /* this champion is the current caster */
} DM1_V1_SpellChampionInfoPc34;

/* Result from drawing spell area */
typedef struct {
    int areaRedrawn;
    int symbolsDrawn;            /* number of available symbols drawn */
    int championHighlightsDrawn; /* number of champion zones highlighted */
    int spellLineRedrawn;        /* the selected-spell line was updated */
} DM1_V1_SpellRenderResultPc34;

/*
 * Initialize spell render state.
 */
void DM1_V1_SpellRender_InitPc34Compat(DM1_V1_SpellRenderStatePc34 *s);

/*
 * Set the active caster and reset spell selection.
 */
void DM1_V1_SpellRender_SetCasterPc34Compat(DM1_V1_SpellRenderStatePc34 *s,
                                  int championIndex);

/*
 * Add a symbol to the current spell (player clicked a symbol button).
 * symbolIndex: 0-5 within the current step.
 * Returns 1 if added, 0 if spell is full.
 */
int DM1_V1_SpellRender_AddSymbolPc34Compat(DM1_V1_SpellRenderStatePc34 *s, int symbolIndex);

/*
 * Remove the last symbol (backspace).
 * Returns 1 if removed, 0 if nothing to remove.
 */
int DM1_V1_SpellRender_RemoveSymbolPc34Compat(DM1_V1_SpellRenderStatePc34 *s);

/*
 * Clear spell selection entirely.
 */
void DM1_V1_SpellRender_ClearPc34Compat(DM1_V1_SpellRenderStatePc34 *s);

/*
 * Draw spell area controls (F0393).
 * champInfo: array of 4 champion casting infos.
 * Returns rendering result.
 */
DM1_V1_SpellRenderResultPc34 DM1_V1_SpellRender_DrawControlsPc34Compat(
    const DM1_V1_SpellRenderStatePc34 *s,
    const DM1_V1_SpellChampionInfoPc34 champInfo[4],
    int partyChampionCount);

/*
 * Draw available symbols for the current step (F0397).
 * Returns number of symbols drawn (always 6 if step is valid).
 */
int DM1_V1_SpellRender_DrawSymbolsPc34Compat(const DM1_V1_SpellRenderStatePc34 *s);

/*
 * Get the global symbol index for a local index in the current step.
 * localIndex: 0-5, step: 0-3.
 * Returns: 96 + 6*step + localIndex (the character code used in original).
 */
int DM1_V1_SpellRender_SymbolCodePc34Compat(int step, int localIndex);

/*
 * Source evidence string.
 */
const char *DM1_V1_SpellRender_SourceEvidencePc34Compat(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_SPELL_EFFECT_RENDER_PC34_COMPAT_H */
