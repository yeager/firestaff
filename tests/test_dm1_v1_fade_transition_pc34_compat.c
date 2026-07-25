#include "dm1_v1_fade_transition_pc34_compat.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_constants(void)
{
    assert(DM1_V1_FADE_STEPS_PC34 == 16);
    assert(DM1_V1_FADE_FRAME_MS_PC34 == 33);
    assert(DM1_V1_SWOOSH_STEPS_PC34 == 32);
    assert(DM1_V1_OVERLAY_MAX_TEXT_PC34 == 256);
    assert(DM1_FADE_STEPS == 16);
    assert(DM1_FADE_FRAME_MS == 33);
    assert(DM1_SWOOSH_STEPS == 32);
}

static void test_fade_mode_enum(void)
{
    assert(DM1_V1_FADE_NONE_PC34 == 0);
    assert(DM1_V1_FADE_OUT_PC34 == 1);
    assert(DM1_V1_FADE_IN_PC34 == 2);
    assert(DM1_V1_FADE_TO_OVERLAY_PC34 == 3);
    assert(DM1_V1_FADE_SWOOSH_PC34 == 4);
}

static void test_init(void)
{
    DM1_V1_FadeStatePc34 s;
    DM1_V1_Fade_InitPc34Compat(&s);
    assert(s.mode == DM1_V1_FADE_NONE_PC34);
    assert(s.active == false);
    assert(s.overlay_active == false);
    assert(s.step == 0);
}

static void test_start_fade_out(void)
{
    DM1_V1_FadeStatePc34 s;
    DM1_V1_Fade_InitPc34Compat(&s);
    DM1_V1_Fade_StartOutPc34Compat(&s);
    assert(s.mode == DM1_V1_FADE_OUT_PC34);
    int a = DM1_V1_Fade_IsActivePc34Compat(&s);
    (void)a;
    assert(a);
}

static void test_start_fade_in(void)
{
    DM1_V1_FadeStatePc34 s;
    DM1_V1_Fade_InitPc34Compat(&s);
    DM1_V1_Fade_StartInPc34Compat(&s);
    assert(s.mode == DM1_V1_FADE_IN_PC34);
}

static void test_start_overlay(void)
{
    DM1_V1_FadeStatePc34 s;
    DM1_V1_Fade_InitPc34Compat(&s);
    DM1_V1_Fade_StartOverlayPc34Compat(&s, "Test text", 100, 80, 15);
    assert(s.mode == DM1_V1_FADE_TO_OVERLAY_PC34);
    assert(s.overlay_active == true);
    const char *txt = DM1_V1_Fade_GetOverlayTextPc34Compat(&s);
    (void)txt;
    assert(txt != NULL);
    assert(strcmp(txt, "Test text") == 0);
}

static void test_start_swoosh(void)
{
    DM1_V1_FadeStatePc34 s;
    DM1_V1_Fade_InitPc34Compat(&s);
    DM1_V1_Fade_StartSwooshPc34Compat(&s);
    assert(s.mode == DM1_V1_FADE_SWOOSH_PC34);
}

static void test_cancel(void)
{
    DM1_V1_FadeStatePc34 s;
    DM1_V1_Fade_InitPc34Compat(&s);
    DM1_V1_Fade_StartOutPc34Compat(&s);
    DM1_V1_Fade_CancelPc34Compat(&s);
    int a = DM1_V1_Fade_IsActivePc34Compat(&s);
    (void)a;
    assert(!a);
}

static void test_tick(void)
{
    DM1_V1_FadeStatePc34 s;
    DM1_V1_Fade_InitPc34Compat(&s);
    DM1_V1_Fade_StartOutPc34Compat(&s);
    int animating = DM1_V1_Fade_TickPc34Compat(&s);
    (void)animating;
    assert(s.step >= 1);
}

static void test_save_get_palette(void)
{
    DM1_V1_FadeStatePc34 s;
    DM1_V1_Fade_InitPc34Compat(&s);
    DM1_V1_FadeColorPc34 pal[16];
    memset(pal, 0, sizeof(pal));
    pal[0].rgb12 = 0x0F00;
    DM1_V1_Fade_SavePalettePc34Compat(&s, pal);
    assert(s.saved_palette[0].rgb12 == 0x0F00);
    DM1_V1_FadeColorPc34 out[16];
    DM1_V1_Fade_GetPalettePc34Compat(&s, out);
}

int main(void)
{
    test_constants();
    test_fade_mode_enum();
    test_init();
    test_start_fade_out();
    test_start_fade_in();
    test_start_overlay();
    test_start_swoosh();
    test_cancel();
    test_tick();
    test_save_get_palette();

    puts("ok: DM1 fade transition (Q-DM1-08) 10 tests passed");
    return 0;
}
