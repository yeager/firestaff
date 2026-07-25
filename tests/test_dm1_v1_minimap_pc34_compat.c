#include "dm1_v1_minimap_pc34_compat.h"

#include <assert.h>
#include <stdio.h>

static void test_render_null_gameview(void)
{
    unsigned char fb[4] = {0};
    DM1_V1_Minimap_RenderPc34Compat(NULL, fb, 2, 2);
}

static void test_render_null_framebuffer(void)
{
    DM1_V1_Minimap_RenderPc34Compat(NULL, NULL, 320, 200);
}

int main(void)
{
    test_render_null_gameview();
    test_render_null_framebuffer();

    puts("ok: DM1 minimap (Q-DM1-03) 2 tests passed");
    return 0;
}
