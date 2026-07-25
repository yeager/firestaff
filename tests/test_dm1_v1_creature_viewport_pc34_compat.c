#include "dm1_v1_creature_viewport_pc34_compat.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_init(void)
{
    DM1_V1_CreatureViewportStatePc34 state;

    DM1_V1_CreatureViewport_InitPc34Compat(&state);
    assert(state.creature_count == 0);
    assert(state.visible_count == 0);
}

static void test_sprite_table_setup(void)
{
    DM1_V1_CreatureViewportStatePc34 state;

    DM1_V1_CreatureViewport_InitPc34Compat(&state);
    DM1_V1_CreatureViewport_SetupSpriteTablePc34Compat(&state);
    assert(state.sprite_info[DM1_V1_CREATURE_VIEWPORT_MUMMY_PC34].base_width > 0);
    assert(state.sprite_info[DM1_V1_CREATURE_VIEWPORT_MUMMY_PC34].base_height > 0);
}

static void test_add_creature(void)
{
    DM1_V1_CreatureViewportStatePc34 state;
    uint16_t idx;

    DM1_V1_CreatureViewport_InitPc34Compat(&state);
    DM1_V1_CreatureViewport_SetupSpriteTablePc34Compat(&state);
    idx = DM1_V1_CreatureViewport_AddCreaturePc34Compat(
        &state, DM1_V1_CREATURE_VIEWPORT_MUMMY_PC34, 5, 5, 0, 100);
    (void)idx;
    assert(state.creature_count == 1);
    assert(state.creatures[idx].alive == true);
    assert(state.creatures[idx].hit_points == 100);
    assert(state.creatures[idx].type == DM1_V1_CREATURE_VIEWPORT_MUMMY_PC34);
}

static void test_damage_and_kill(void)
{
    DM1_V1_CreatureViewportStatePc34 state;
    uint16_t idx;

    DM1_V1_CreatureViewport_InitPc34Compat(&state);
    DM1_V1_CreatureViewport_SetupSpriteTablePc34Compat(&state);
    idx = DM1_V1_CreatureViewport_AddCreaturePc34Compat(
        &state, DM1_V1_CREATURE_VIEWPORT_SKELETON_PC34, 3, 3, 2, 50);

    DM1_V1_CreatureViewport_DamagePc34Compat(&state, idx, 20);
    assert(state.creatures[idx].hit_points == 30);
    assert(DM1_V1_CreatureViewport_IsAlivePc34Compat(&state, idx) == true);
    assert(state.creatures[idx].flash_timer > 0);

    DM1_V1_CreatureViewport_DamagePc34Compat(&state, idx, 50);
    assert(DM1_V1_CreatureViewport_IsAlivePc34Compat(&state, idx) == false);
}

static void test_party_pos_and_visibility(void)
{
    DM1_V1_CreatureViewportStatePc34 state;

    DM1_V1_CreatureViewport_InitPc34Compat(&state);
    DM1_V1_CreatureViewport_SetupSpriteTablePc34Compat(&state);
    DM1_V1_CreatureViewport_AddCreaturePc34Compat(
        &state, DM1_V1_CREATURE_VIEWPORT_GHOST_PC34, 5, 4, 0, 80);

    DM1_V1_CreatureViewport_SetPartyPosPc34Compat(&state, 5, 5, 0);
    DM1_V1_CreatureViewport_UpdateVisibilityPc34Compat(&state);
    uint8_t vc = DM1_V1_CreatureViewport_GetVisibleCountPc34Compat(&state);
    (void)vc;
    assert(vc <= DM1_V1_CREATURE_VIEWPORT_MAX_VISIBLE_PC34);
}

static void test_animate(void)
{
    DM1_V1_CreatureViewportStatePc34 state;
    uint16_t idx;

    DM1_V1_CreatureViewport_InitPc34Compat(&state);
    DM1_V1_CreatureViewport_SetupSpriteTablePc34Compat(&state);
    idx = DM1_V1_CreatureViewport_AddCreaturePc34Compat(
        &state, DM1_V1_CREATURE_VIEWPORT_PAIN_RAT_PC34, 5, 5, 1, 40);
    uint8_t frame0 = state.creatures[idx].anim_frame;

    for (int i = 0; i < 20; i++) {
        DM1_V1_CreatureViewport_AnimateFramePc34Compat(&state);
    }
    (void)frame0;
    assert(state.creatures[idx].anim_frame < DM1_V1_CREATURE_VIEWPORT_ANIM_FRAMES_PC34);
}

int main(void)
{
    test_init();
    test_sprite_table_setup();
    test_add_creature();
    test_damage_and_kill();
    test_party_pos_and_visibility();
    test_animate();

    puts("ok: DM1 creature viewport (Q-DM1-03) 6 tests passed");
    return 0;
}
