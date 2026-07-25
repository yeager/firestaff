#include "dm1_v1_screen_framebuffer_pc34_compat.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static DM1_V1_ScreenStatePc34 s_screen;

static void test_constants(void)
{
    assert(DM1_V1_SCREEN_W_PC34 == 320);
    assert(DM1_V1_SCREEN_H_PC34 == 200);
    assert(DM1_V1_PALETTE_SIZE_PC34 == 16);
}

static void test_init(void)
{
    DM1_V1_Screen_InitPc34Compat(&s_screen);
    assert(s_screen.dirty == 0);
    assert(s_screen.presentCount == 0);
}

static void test_get_back_buffer(void)
{
    DM1_V1_Screen_InitPc34Compat(&s_screen);
    uint8_t *bb = DM1_V1_Screen_GetBackBufferPc34Compat(&s_screen);
    (void)bb;
    assert(bb != NULL);
    assert(bb == (uint8_t *)s_screen.backBuffer);
}

static void test_get_front_buffer(void)
{
    DM1_V1_Screen_InitPc34Compat(&s_screen);
    const uint8_t *fb = DM1_V1_Screen_GetFrontBufferPc34Compat(&s_screen);
    (void)fb;
    assert(fb != NULL);
    assert(fb == (const uint8_t *)s_screen.frontBuffer);
}

static void test_clear_back(void)
{
    DM1_V1_Screen_InitPc34Compat(&s_screen);
    DM1_V1_Screen_ClearBackPc34Compat(&s_screen, 7);
    uint8_t *bb = DM1_V1_Screen_GetBackBufferPc34Compat(&s_screen);
    assert(bb[0] == 7);
    assert(bb[320 * 200 - 1] == 7);
}

static void test_mark_dirty(void)
{
    DM1_V1_Screen_InitPc34Compat(&s_screen);
    assert(DM1_V1_Screen_IsDirtyPc34Compat(&s_screen) == 0);
    DM1_V1_Screen_MarkDirtyPc34Compat(&s_screen);
    int d = DM1_V1_Screen_IsDirtyPc34Compat(&s_screen);
    (void)d;
    assert(d != 0);
}

static void test_set_palette(void)
{
    DM1_V1_Screen_InitPc34Compat(&s_screen);
    DM1_V1_Screen_SetPalettePc34Compat(&s_screen, 5, 63, 32, 0);
    DM1_V1_PaletteEntryPc34 p = DM1_V1_Screen_GetPalettePc34Compat(&s_screen, 5);
    (void)p;
    assert(p.r == 63);
    assert(p.g == 32);
    assert(p.b == 0);
}

static void test_present(void)
{
    DM1_V1_Screen_InitPc34Compat(&s_screen);
    DM1_V1_Screen_ClearBackPc34Compat(&s_screen, 3);
    DM1_V1_Screen_MarkDirtyPc34Compat(&s_screen);
    DM1_V1_Screen_PresentPc34Compat(&s_screen, 1000);
    assert(DM1_V1_Screen_IsDirtyPc34Compat(&s_screen) == 0);
    assert(s_screen.presentCount == 1);
    const uint8_t *fb = DM1_V1_Screen_GetFrontBufferPc34Compat(&s_screen);
    assert(fb[0] == 3);
}

static void test_source_evidence(void)
{
    const char *ev = DM1_V1_Screen_SourceEvidencePc34Compat();
    (void)ev;
    assert(ev != NULL);
    assert(strlen(ev) > 0);
}

int main(void)
{
    test_constants();
    test_init();
    test_get_back_buffer();
    test_get_front_buffer();
    test_clear_back();
    test_mark_dirty();
    test_set_palette();
    test_present();
    test_source_evidence();

    puts("ok: DM1 screen framebuffer (Q-DM1-03) 9 tests passed");
    return 0;
}
