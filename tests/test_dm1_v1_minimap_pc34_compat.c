#include "dm1_v1_minimap_pc34_compat.h"
#include "m11_qol_runtime.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_render_null_gameview(void)
{
    unsigned char fb[4] = {0};
    DM1_V1_Minimap_RenderPc34Compat(NULL, fb, 2, 2);
}

static void test_render_null_framebuffer(void)
{
    DM1_V1_Minimap_RenderPc34Compat(NULL, NULL, 320, 200);
}

static void test_authenticated_dm1_rejects_host_minimap(void)
{
    M11_GameViewState state;
    unsigned char fb[16];
    unsigned char expected[16];

    memset(&state, 0, sizeof(state));
    memset(fb, 0x5a, sizeof(fb));
    memcpy(expected, fb, sizeof(fb));
    state.active = 1;
    state.sourceKind = M11_GAME_SOURCE_BUILTIN_CATALOG;
    M11_QolRuntime_SetMinimapEnabled(1);

    DM1_V1_Minimap_RenderPc34Compat(&state, fb, 4, 4);
    assert(memcmp(fb, expected, sizeof(fb)) == 0);

    M11_QolRuntime_SetMinimapEnabled(0);
}

int main(void)
{
    test_render_null_gameview();
    test_render_null_framebuffer();
    test_authenticated_dm1_rejects_host_minimap();

    puts("ok: DM1 minimap (Q-DM1-03) 3 tests passed");
    return 0;
}
