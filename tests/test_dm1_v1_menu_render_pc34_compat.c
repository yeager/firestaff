#include "dm1_v1_menu_render_pc34_compat.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_zone_constants(void)
{
    assert(DM1_V1_ZONE_MOVEMENT_ARROWS_PC34 == 9);
    assert(DM1_V1_ZONE_SPELL_AREA_PC34 == 10);
    assert(DM1_V1_ZONE_ACTION_AREA_PC34 == 11);
}

static void test_action_hand_enum(void)
{
    assert(DM1_ACTION_HAND_NONE == 0);
    assert(DM1_ACTION_HAND_OPEN == 1);
    assert(DM1_ACTION_HAND_HOLDING == 2);
    assert(DM1_ACTION_HAND_COMBAT == 3);
    assert(DM1_ACTION_HAND_COUNT == 4);
}

static void test_action_type_enum(void)
{
    assert(DM1_ACTION_NONE == 0);
    assert(DM1_ACTION_ATTACK == 1);
    assert(DM1_ACTION_CAST == 2);
    assert(DM1_ACTION_USE == 3);
    assert(DM1_ACTION_THROW == 4);
    assert(DM1_ACTION_COUNT == 5);
}

static void test_init(void)
{
    DM1_V1_MenuRenderStatePc34 s;
    memset(&s, 0xFF, sizeof(s));
    DM1_V1_MenuRender_InitPc34Compat(&s);
    assert(s.movementArrowsEnabled == 1);
    assert(s.spellAreaEnabled == 1);
    assert(s.actionAreaEnabled == 1);
    assert(s.leaderHand == DM1_ACTION_HAND_NONE);
    assert(s.leaderHandObjectIcon == -1);
    assert(s.commandHighlightActive == 0);
}

static void test_enable_movement(void)
{
    DM1_V1_MenuRenderStatePc34 s;
    DM1_V1_MenuRender_InitPc34Compat(&s);
    assert(s.movementArrowsEnabled == 1);
    DM1_V1_MenuRender_EnableMovementPc34Compat(&s, 0);
    assert(s.movementArrowsEnabled == 0);
    DM1_V1_MenuRender_EnableMovementPc34Compat(&s, 1);
    assert(s.movementArrowsEnabled == 1);
}

static void test_enable_spells(void)
{
    DM1_V1_MenuRenderStatePc34 s;
    DM1_V1_MenuRender_InitPc34Compat(&s);
    DM1_V1_MenuRender_EnableSpellsPc34Compat(&s, 1);
    assert(s.spellAreaEnabled == 1);
}

static void test_enable_actions(void)
{
    DM1_V1_MenuRenderStatePc34 s;
    DM1_V1_MenuRender_InitPc34Compat(&s);
    DM1_V1_MenuRender_EnableActionsPc34Compat(&s, 1);
    assert(s.actionAreaEnabled == 1);
}

static void test_set_highlight(void)
{
    DM1_V1_MenuRenderStatePc34 s;
    DM1_V1_MenuRender_InitPc34Compat(&s);
    DM1_V1_MenuRender_SetHighlightPc34Compat(&s, 10, 20, 30, 40);
    assert(s.commandHighlightActive == 1);
    assert(s.commandHighlightX == 10);
    assert(s.commandHighlightY == 20);
    assert(s.commandHighlightW == 30);
    assert(s.commandHighlightH == 40);
}

static void test_clear_highlight(void)
{
    DM1_V1_MenuRenderStatePc34 s;
    DM1_V1_MenuRender_InitPc34Compat(&s);
    DM1_V1_MenuRender_SetHighlightPc34Compat(&s, 10, 20, 30, 40);
    DM1_V1_MenuRender_ClearHighlightPc34Compat(&s);
    assert(s.commandHighlightActive == 0);
}

static void test_set_leader_hand(void)
{
    DM1_V1_MenuRenderStatePc34 s;
    DM1_V1_MenuRender_InitPc34Compat(&s);
    DM1_V1_MenuRender_SetLeaderHandPc34Compat(&s, DM1_ACTION_HAND_COMBAT, 42);
    assert(s.leaderHand == DM1_ACTION_HAND_COMBAT);
    assert(s.leaderHandObjectIcon == 42);
}

static void test_draw_enabled_all_off(void)
{
    DM1_V1_MenuRenderStatePc34 s;
    DM1_V1_MenuRender_InitPc34Compat(&s);
    DM1_V1_MenuRender_EnableMovementPc34Compat(&s, 0);
    DM1_V1_MenuRender_EnableSpellsPc34Compat(&s, 0);
    DM1_V1_MenuRender_EnableActionsPc34Compat(&s, 0);
    DM1_V1_MenuRenderResultPc34 r = DM1_V1_MenuRender_DrawEnabledPc34Compat(&s);
    (void)r;
    assert(r.movementArrowsDrawn == 0);
    assert(r.spellAreaDrawn == 0);
    assert(r.actionAreaDrawn == 0);
}

static void test_draw_enabled_movement_only(void)
{
    DM1_V1_MenuRenderStatePc34 s;
    DM1_V1_MenuRender_InitPc34Compat(&s);
    DM1_V1_MenuRender_EnableSpellsPc34Compat(&s, 0);
    DM1_V1_MenuRender_EnableActionsPc34Compat(&s, 0);
    DM1_V1_MenuRenderResultPc34 r = DM1_V1_MenuRender_DrawEnabledPc34Compat(&s);
    (void)r;
    assert(r.movementArrowsDrawn == 1);
    assert(r.spellAreaDrawn == 0);
    assert(r.actionAreaDrawn == 0);
    assert(r.enabledMenusDrawn == 1);
}

static void test_source_evidence(void)
{
    const char *ev = DM1_V1_MenuRender_SourceEvidencePc34Compat();
    (void)ev;
    assert(ev != NULL);
    assert(strlen(ev) > 0);
}

int main(void)
{
    test_zone_constants();
    test_action_hand_enum();
    test_action_type_enum();
    test_init();
    test_enable_movement();
    test_enable_spells();
    test_enable_actions();
    test_set_highlight();
    test_clear_highlight();
    test_set_leader_hand();
    test_draw_enabled_all_off();
    test_draw_enabled_movement_only();
    test_source_evidence();

    puts("ok: DM1 menu render (Q-DM1-07) 13 tests passed");
    return 0;
}
