#include "dm1_v1_spell_effect_render_pc34_compat.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_constants(void)
{
    assert(DM1_V1_SPELL_SYMBOL_STEPS_PC34 == 4);
    assert(DM1_V1_SYMBOLS_PER_STEP_PC34 == 6);
    assert(DM1_V1_MAX_SPELL_SYMBOLS_PC34 == 24);
    assert(DM1_V1_MAX_SPELL_LENGTH_PC34 == 4);
    assert(DM1_V1_SPELL_AREA_X_PC34 == 233);
    assert(DM1_V1_SPELL_AREA_Y_PC34 == 2);
    assert(DM1_V1_SPELL_AREA_W_PC34 == 85);
    assert(DM1_V1_SPELL_AREA_H_PC34 == 70);
    assert(DM1_V1_SPELL_SYMBOL_W_PC34 == 14);
    assert(DM1_V1_SPELL_SYMBOL_H_PC34 == 12);
}

static void test_init(void)
{
    DM1_V1_SpellRenderStatePc34 s;
    DM1_V1_SpellRender_InitPc34Compat(&s);
    assert(s.casterChampionIndex == -1);
    assert(s.currentSymbolStep == 0);
    assert(s.selectedCount == 0);
    assert(s.spellValid == 0);
    assert(s.castingInProgress == 0);
}

static void test_set_caster(void)
{
    DM1_V1_SpellRenderStatePc34 s;
    DM1_V1_SpellRender_InitPc34Compat(&s);
    DM1_V1_SpellRender_SetCasterPc34Compat(&s, 2);
    assert(s.casterChampionIndex == 2);
    assert(s.selectedCount == 0);
}

static void test_add_symbol(void)
{
    DM1_V1_SpellRenderStatePc34 s;
    DM1_V1_SpellRender_InitPc34Compat(&s);
    DM1_V1_SpellRender_SetCasterPc34Compat(&s, 0);
    int r1 = DM1_V1_SpellRender_AddSymbolPc34Compat(&s, 3);
    (void)r1;
    assert(r1 == 1);
    assert(s.selectedCount == 1);
    assert(s.selectedSymbols[0] == DM1_V1_SpellRender_SymbolCodePc34Compat(0, 3));
}

static void test_add_symbol_full(void)
{
    DM1_V1_SpellRenderStatePc34 s;
    DM1_V1_SpellRender_InitPc34Compat(&s);
    DM1_V1_SpellRender_SetCasterPc34Compat(&s, 0);
    for (int i = 0; i < DM1_V1_MAX_SPELL_LENGTH_PC34; i++)
        DM1_V1_SpellRender_AddSymbolPc34Compat(&s, i);
    int r = DM1_V1_SpellRender_AddSymbolPc34Compat(&s, 5);
    (void)r;
    assert(r == 0);
    assert(s.selectedCount == DM1_V1_MAX_SPELL_LENGTH_PC34);
}

static void test_remove_symbol(void)
{
    DM1_V1_SpellRenderStatePc34 s;
    DM1_V1_SpellRender_InitPc34Compat(&s);
    DM1_V1_SpellRender_SetCasterPc34Compat(&s, 0);
    DM1_V1_SpellRender_AddSymbolPc34Compat(&s, 1);
    int r = DM1_V1_SpellRender_RemoveSymbolPc34Compat(&s);
    (void)r;
    assert(r == 1);
    assert(s.selectedCount == 0);
}

static void test_remove_symbol_empty(void)
{
    DM1_V1_SpellRenderStatePc34 s;
    DM1_V1_SpellRender_InitPc34Compat(&s);
    int r = DM1_V1_SpellRender_RemoveSymbolPc34Compat(&s);
    (void)r;
    assert(r == 0);
}

static void test_clear(void)
{
    DM1_V1_SpellRenderStatePc34 s;
    DM1_V1_SpellRender_InitPc34Compat(&s);
    DM1_V1_SpellRender_SetCasterPc34Compat(&s, 1);
    DM1_V1_SpellRender_AddSymbolPc34Compat(&s, 0);
    DM1_V1_SpellRender_ClearPc34Compat(&s);
    assert(s.selectedCount == 0);
}

static void test_symbol_code(void)
{
    int c = DM1_V1_SpellRender_SymbolCodePc34Compat(0, 0);
    (void)c;
    assert(c == 96);
    int c2 = DM1_V1_SpellRender_SymbolCodePc34Compat(2, 3);
    (void)c2;
    assert(c2 == 96 + 6 * 2 + 3);
}

static void test_source_evidence(void)
{
    const char *ev = DM1_V1_SpellRender_SourceEvidencePc34Compat();
    (void)ev;
    assert(ev != NULL);
    assert(strlen(ev) > 0);
}

int main(void)
{
    test_constants();
    test_init();
    test_set_caster();
    test_add_symbol();
    test_add_symbol_full();
    test_remove_symbol();
    test_remove_symbol_empty();
    test_clear();
    test_symbol_code();
    test_source_evidence();

    puts("ok: DM1 spell effect render (Q-DM1-07) 10 tests passed");
    return 0;
}
