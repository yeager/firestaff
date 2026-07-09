#ifndef DM2_V1_BOOT_STARTUP_VIEW_MODEL_H
#define DM2_V1_BOOT_STARTUP_VIEW_MODEL_H

#include "dm2_v1_boot.h"
#include "dm2_v1_startup_presentation.h"

struct DM2_V1_BootStartupViewModel {
    DM2_V1_StartupDrawCommand commands[DM2_V1_BOOT_STARTUP_VIEW_MODEL_COMMAND_CAP];
    int command_count;
    DM2_V1_StartupViewReceipt view_receipt;
    char phase[DM2_V1_BOOT_STARTUP_VIEW_MODEL_TEXT_CAP];
    int startup_active;
    char animation[DM2_V1_BOOT_STARTUP_VIEW_MODEL_ANIMATION_CAP];
    int animation_active;
    int title_frame;
    int title_frame_max;
    int title_ready;
    int initialize_v2_runtime;
    int initialize_hud_runtime;
    int initialize_touch_runtime;
    int hud_runtime_ready;
    int runtime_menu_ready;
    int runtime_action_ready;
    int first_hud_frame_ready;
    int title_backdrop_ready;
    int resume_menu_ready;
    int save_slot_menu_ready;
    int new_game_menu_ready;
    int full_start_graphics_ready;
    int title_gdat_asset_ready;
    int title_gdat_asset_w;
    int title_gdat_asset_h;
    int title_gdat_asset_stride;
    int full_start_real_asset_ready;
    DM2_V1_BootStartupFullStartReceipt full_start_receipt;
    DM2_V1_BootStartupHostViewReceipt host_view_receipt;
};

#endif
