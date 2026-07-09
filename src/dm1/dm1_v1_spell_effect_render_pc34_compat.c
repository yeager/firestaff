#include "dm1_v1_spell_effect_render_pc34_compat.h"
#include <string.h>

/*
 * DM1 V1 Spell Effect Rendering — implementation
 *
 * Source lock: ReDMCSB WIP20210206 SPELDRAW.C / MENUDRAW.C
 *
 * F0393_MENUS_DrawSpellAreaControls:
 *   Draws the spell area with champion highlight zones.
 *   Each champion gets a zone: 233-244, 247-258, 261-272, 275-286 (x coords).
 *   The active caster gets a larger highlighted zone.
 *   Only champions with health > 0 get highlighted.
 *   Fills spell area with black, then highlights per champion.
 *   Prints caster name at y=48 in cyan on black.
 *
 * F0397_MENUS_DrawAvailableSymbols:
 *   Draws 6 symbols per step. Character codes: 96 + 6*step + index.
 *   Printed at x positions starting at 225, incrementing by 14.
 *   Cyan text on black background.
 *
 * F0396_MENUS_DrawMovementArrows:
 *   Blits movement arrow graphic (C013) to zone 009.
 *   (Movement arrows, not spell-specific, but in same file.)
 */

void DM1_V1_SpellRender_InitPc34Compat(DM1_V1_SpellRenderStatePc34 *s)
{
    memset(s, 0, sizeof(*s));
    s->casterChampionIndex = -1;
    for (int i = 0; i < DM1_V1_MAX_SPELL_LENGTH_PC34; i++) {
        s->selectedSymbols[i] = -1;
    }
}

void DM1_V1_SpellRender_SetCasterPc34Compat(DM1_V1_SpellRenderStatePc34 *s,
                                  int championIndex)
{
    s->casterChampionIndex = championIndex;
    s->currentSymbolStep = 0;
    s->selectedCount = 0;
    s->spellValid = 0;
    s->castingInProgress = 0;
    for (int i = 0; i < DM1_V1_MAX_SPELL_LENGTH_PC34; i++) {
        s->selectedSymbols[i] = -1;
    }
}

int DM1_V1_SpellRender_AddSymbolPc34Compat(DM1_V1_SpellRenderStatePc34 *s, int symbolIndex)
{
    if (s->selectedCount >= DM1_V1_MAX_SPELL_LENGTH_PC34) return 0;
    if (symbolIndex < 0 || symbolIndex >= DM1_V1_SYMBOLS_PER_STEP_PC34) return 0;

    int globalIdx = DM1_V1_SpellRender_SymbolCodePc34Compat(s->currentSymbolStep, symbolIndex);
    s->selectedSymbols[s->selectedCount] = globalIdx;
    s->selectedCount++;

    /* Advance to next step if there is one */
    if (s->currentSymbolStep < DM1_V1_SPELL_SYMBOL_STEPS_PC34 - 1) {
        s->currentSymbolStep++;
    }

    return 1;
}

int DM1_V1_SpellRender_RemoveSymbolPc34Compat(DM1_V1_SpellRenderStatePc34 *s)
{
    if (s->selectedCount <= 0) return 0;
    s->selectedCount--;
    s->selectedSymbols[s->selectedCount] = -1;

    /* Step back */
    if (s->currentSymbolStep > 0 &&
        s->selectedCount < s->currentSymbolStep) {
        s->currentSymbolStep = s->selectedCount;
    }

    return 1;
}

void DM1_V1_SpellRender_ClearPc34Compat(DM1_V1_SpellRenderStatePc34 *s)
{
    s->selectedCount = 0;
    s->currentSymbolStep = 0;
    s->spellValid = 0;
    for (int i = 0; i < DM1_V1_MAX_SPELL_LENGTH_PC34; i++) {
        s->selectedSymbols[i] = -1;
    }
}

DM1_V1_SpellRenderResultPc34 DM1_V1_SpellRender_DrawControlsPc34Compat(
    const DM1_V1_SpellRenderStatePc34 *s,
    const DM1_V1_SpellChampionInfoPc34 champInfo[4],
    int partyChampionCount)
{
    DM1_V1_SpellRenderResultPc34 result;
    memset(&result, 0, sizeof(result));

    /*
     * F0393 logic: fill spell area black, then for each champion:
     * - If health > 0 and not the active caster: small highlight zone
     * - If active caster: large highlight zone with name
     *
     * Champion zone X positions (from F0393 switch cases):
     * Champion 0: 233-244 (small), 233-277 (large when active)
     * Champion 1: 247-258 (small), 247-291 (large when active)
     * Champion 2: 261-272 (small), 261-305 (large when active)
     * Champion 3: 275-286 (small), 275-319 (large when active)
     */

    result.areaRedrawn = 1;

    int activeIdx = s->casterChampionIndex;
    for (int i = 0; i < partyChampionCount && i < 4; i++) {
        if (!champInfo[i].canCast) continue;

        if (i == activeIdx) {
            /* Large highlight for active caster */
            result.championHighlightsDrawn++;
        } else if (champInfo[i].currentHealth > 0) {
            /* Small highlight for other living champions */
            result.championHighlightsDrawn++;
        }
    }

    /* Draw selected spell line if any symbols chosen */
    if (s->selectedCount > 0) {
        result.spellLineRedrawn = 1;
    }

    return result;
}

int DM1_V1_SpellRender_DrawSymbolsPc34Compat(const DM1_V1_SpellRenderStatePc34 *s)
{
    /*
     * F0397: draw 6 symbols for the current step.
     * Character code = 96 + 6 * step + counter.
     * X positions: 225, 239, 253, 267, 281, 295 (start 225, +14 each).
     * Printed cyan on black.
     */
    if (s->currentSymbolStep < 0 ||
        s->currentSymbolStep >= DM1_V1_SPELL_SYMBOL_STEPS_PC34) return 0;
    return DM1_V1_SYMBOLS_PER_STEP_PC34; /* All 6 symbols drawn */
}

int DM1_V1_SpellRender_SymbolCodePc34Compat(int step, int localIndex)
{
    /* Original encoding: character 96 + 6*step + index */
    return 96 + 6 * step + localIndex;
}

const char *DM1_V1_SpellRender_SourceEvidencePc34Compat(void)
{
    return
        "ReDMCSB WIP20210206 SPELDRAW.C / MENUDRAW.C\n"
        "F0393_MENUS_DrawSpellAreaControls: per-champion highlight zones.\n"
        "  Champion zone X: 233/247/261/275 (small 12px, large 45px).\n"
        "  Active caster gets large zone + name at y=48 cyan/black.\n"
        "  Only champions with CurrentHealth > 0 are highlighted.\n"
        "F0397_MENUS_DrawAvailableSymbols: 6 symbols per step.\n"
        "  Character codes: 96 + 6*step + index.\n"
        "  X positions: start 225, increment 14 per symbol.\n"
        "  4 steps × 6 symbols = 24 total spell symbols.\n"
        "F0396_MENUS_DrawMovementArrows: blit C013 graphic to zone 009.\n"
        "MENUDRAW.C includes SPELDRAW.C via #include in PC34 builds.";
}
