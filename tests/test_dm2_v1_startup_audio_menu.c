#include "dm2_v1_asset_loader.h"
#include "dm2_v1_boot.h"
#include "dm2_v1_startup_menu.h"
#include "dm2_v1_startup_presentation.h"

#include <stdio.h>
#include <string.h>

int main(void)
{
    DM2_V1_StartupRuntimeHandoffReceipt handoff;
    DM2_V1_StartupMenu menu;
    DM2_V1_StartupDrawCommand commands[2];
    int count;

    if (!dm2_v1_startup_runtime_handoff_receipt_from_tick(
            &handoff, 1, 1, 47) ||
        handoff.animation_active != 1 ||
        handoff.title_animation_tick != 0 ||
        handoff.title_frame != 0 ||
        handoff.title_frame_max != 0 ||
        handoff.title_frame_duration_ticks != 0 ||
        handoff.title_ready != 1 ||
        handoff.music_cue != 0 ||
        handoff.music_loop != 1 ||
        handoff.music_cue_played != 0 ||
        handoff.music_cue_source_silence != 0 ||
        handoff.show_menu_screen_after_music != 1) {
        fprintf(stderr, "DM2 startup music/menu handoff mismatch\n");
        return 1;
    }

    dm2_v1_startup_menu_init(&menu, "");
    if (!dm2_v1_startup_menu_refresh(&menu, 1, 1u << 2)) {
        fprintf(stderr, "DM2 startup menu setup failed\n");
        return 1;
    }
    count = dm2_v1_startup_presentation_build(
        &menu, commands, (int)(sizeof(commands) / sizeof(commands[0])));
    if (count != 2 || commands[0].kind != DM2_V1_STARTUP_DRAW_GDAT_IMAGE ||
        commands[0].gdat_category != DM2_GDAT_CATEGORY_TITLE ||
        commands[0].gdat_index != 0 ||
        commands[0].gdat_field != 4 ||
        commands[0].rect.x != 0 || commands[0].rect.y != 0 ||
        commands[0].rect.w != 320 || commands[0].rect.h != 200 ||
        commands[0].frame_owner != DM2_V1_FRAME_OWNER_STARTUP_TITLE ||
        commands[1].kind != DM2_V1_STARTUP_DRAW_GDAT_IMAGE ||
        commands[1].gdat_category != DM2_GDAT_CATEGORY_TITLE ||
        commands[1].gdat_index != 0 ||
        commands[1].gdat_field != 4 ||
        commands[1].rect.x != 0 || commands[1].rect.y != 0 ||
        commands[1].rect.w != 320 || commands[1].rect.h != 200 ||
        commands[1].frame_owner != DM2_V1_FRAME_OWNER_STARTUP_MENU) {
        fprintf(stderr, "DM2 startup package has invalid TITLE GDAT owners\n");
        return 1;
    }

    puts("PASS DM2 startup keeps a static TITLE/0 dt07/4 menu");
    return 0;
}
