#include "dm2_v1_startup_menu.h"

#include <assert.h>
#include <string.h>

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
#error "This test requires C11 or later."
#endif

/* skproject/SKWIN/SkWinCore.cpp::SHOW_MENU_SCREEN (55182-55235):
 * TITLE/0/dt07/4 is static while MessageLoop(true) waits for GAME_LOAD(). */
static void assert_static_menu(int source_tick)
{
    (void)source_tick;
    DM2_V1_StartupRuntimeHandoffReceipt receipt;
    (void)receipt;

    assert(dm2_v1_startup_runtime_handoff_receipt_from_tick(
        &receipt, 1, 1, source_tick));
    assert(receipt.startup_menu_active == 1);
    assert(strcmp(receipt.animation, "dm2-startup-menu") == 0);
    assert(receipt.animation_active == 0);
    assert(receipt.title_animation_tick == 0);
    assert(receipt.title_frame == 0);
    assert(receipt.title_frame_max == 0);
    assert(receipt.title_frame_duration_ticks == 0);
    assert(receipt.title_ready == 1);
    assert(receipt.runtime_menu_ready == 1);
    assert(receipt.runtime_action_ready == 0);
    assert(receipt.first_hud_frame_ready == 0);
}

int main(void)
{
    assert_static_menu(0);
    assert_static_menu(1);
    assert_static_menu(13);
    return 0;
}
