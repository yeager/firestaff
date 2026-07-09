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
};

#endif
