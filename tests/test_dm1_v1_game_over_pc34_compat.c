#include "dm1_v1_game_over_pc34_compat.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_death_effect_init(void)
{
    DM1_V1_DeathEffectStatePc34 state;
    memset(&state, 0xFF, sizeof(state));
    DM1_V1_DeathEffect_InitPc34Compat(&state);
    assert(state.phase == DM1_DEATH_IDLE);
    assert(state.currentFrame == 0);
}

static void test_death_effect_start(void)
{
    DM1_V1_DeathEffectStatePc34 state;
    DM1_V1_DeathEffect_InitPc34Compat(&state);
    DM1_V1_DeathEffect_StartPc34Compat(&state, 0, 1);
    assert(state.phase != DM1_DEATH_IDLE);
    assert(state.gameWon == 0);
    assert(state.restartAllowed == 1);
}

static void test_death_effect_tick(void)
{
    DM1_V1_DeathEffectStatePc34 state;
    DM1_V1_DeathEffect_InitPc34Compat(&state);
    DM1_V1_DeathEffect_StartPc34Compat(&state, 0, 1);
    DM1_V1_DeathEffectPhasePc34 phase = DM1_V1_DeathEffect_TickPc34Compat(&state, 1000);
    (void)phase;
    assert(phase >= DM1_DEATH_IDLE && phase <= DM1_DEATH_COMPLETE);
}

static void test_death_effect_fade_level(void)
{
    DM1_V1_DeathEffectStatePc34 state;
    DM1_V1_DeathEffect_InitPc34Compat(&state);
    int fade = DM1_V1_DeathEffect_FadeLevelPc34Compat(&state);
    (void)fade;
    assert(fade >= 0 && fade <= 15);
}

static void test_death_effect_is_complete_idle(void)
{
    DM1_V1_DeathEffectStatePc34 state;
    DM1_V1_DeathEffect_InitPc34Compat(&state);
    int complete = DM1_V1_DeathEffect_IsCompletePc34Compat(&state);
    (void)complete;
    assert(complete == 0);
}

static void test_death_effect_restart_not_requested(void)
{
    DM1_V1_DeathEffectStatePc34 state;
    DM1_V1_DeathEffect_InitPc34Compat(&state);
    int req = DM1_V1_DeathEffect_RestartRequestedPc34Compat(&state);
    (void)req;
    assert(req == 0);
}

static void test_title_screen_init(void)
{
    DM1_V1_TitleScreenStatePc34 state;
    memset(&state, 0xFF, sizeof(state));
    DM1_V1_TitleScreen_InitPc34Compat(&state, 200000);
    assert(state.phase == DM1_TITLE_IDLE);
    assert(state.hasEnoughMemory == 1);
}

static void test_title_screen_low_memory(void)
{
    DM1_V1_TitleScreenStatePc34 state;
    DM1_V1_TitleScreen_InitPc34Compat(&state, 1000);
    assert(state.hasEnoughMemory == 0);
}

static void test_title_screen_start(void)
{
    DM1_V1_TitleScreenStatePc34 state;
    DM1_V1_TitleScreen_InitPc34Compat(&state, 200000);
    DM1_V1_TitleScreen_StartPc34Compat(&state);
    assert(state.phase != DM1_TITLE_IDLE);
}

static void test_title_screen_tick(void)
{
    DM1_V1_TitleScreenStatePc34 state;
    DM1_V1_TitleScreen_InitPc34Compat(&state, 200000);
    DM1_V1_TitleScreen_StartPc34Compat(&state);
    DM1_V1_TitlePhasePc34 phase = DM1_V1_TitleScreen_TickPc34Compat(&state, 1000);
    (void)phase;
    assert(phase >= DM1_TITLE_IDLE && phase <= DM1_TITLE_COMPLETE);
}

static void test_title_screen_get_zoom(void)
{
    DM1_V1_TitleScreenStatePc34 state;
    DM1_V1_TitleScreen_InitPc34Compat(&state, 200000);
    int w = 0, h = 0, x = 0, y = 0;
    DM1_V1_TitleScreen_GetZoomPc34Compat(&state, &w, &h, &x, &y);
    assert(w >= 0 && h >= 0);
}

static void test_title_screen_get_palette(void)
{
    DM1_V1_TitleScreenStatePc34 state;
    DM1_V1_TitleScreen_InitPc34Compat(&state, 200000);
    const uint16_t *pal = DM1_V1_TitleScreen_GetPalettePc34Compat(&state);
    (void)pal;
    assert(pal != NULL);
}

static void test_title_screen_is_complete(void)
{
    DM1_V1_TitleScreenStatePc34 state;
    DM1_V1_TitleScreen_InitPc34Compat(&state, 200000);
    int complete = DM1_V1_TitleScreen_IsCompletePc34Compat(&state);
    (void)complete;
    assert(complete == 0);
}

static void test_title_constants(void)
{
    assert(DM1_TITLE_ZOOM_STEPS == 18);
    assert(DM1_TITLE_INITIAL_WIDTH == 48);
    assert(DM1_TITLE_FINAL_WIDTH == 320);
    assert(DM1_DEATH_ANIM_FRAMES == 30);
}

static void test_source_evidence(void)
{
    const char *ev = DM1_V1_GameOver_SourceEvidencePc34Compat();
    (void)ev;
    assert(ev != NULL);
    assert(strlen(ev) > 0);
}

int main(void)
{
    test_death_effect_init();
    test_death_effect_start();
    test_death_effect_tick();
    test_death_effect_fade_level();
    test_death_effect_is_complete_idle();
    test_death_effect_restart_not_requested();
    test_title_screen_init();
    test_title_screen_low_memory();
    test_title_screen_start();
    test_title_screen_tick();
    test_title_screen_get_zoom();
    test_title_screen_get_palette();
    test_title_screen_is_complete();
    test_title_constants();
    test_source_evidence();

    puts("ok: DM1 game over/title (Q-DM1-08) 15 tests passed");
    return 0;
}
