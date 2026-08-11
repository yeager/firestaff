#include "csb_v1_x68k_startup_handoff.h"

#include "csb_v1_x68k_dungeon_handoff.h"

#include <string.h>

void csb_v1_x68k_startup_handoff_cleanup(
    CSB_V1_X68kStartupHandoff *handoff)
{
    if (!handoff) return;
    if (handoff->graphics.initialized) {
        M11_AssetLoader_Shutdown(&handoff->graphics);
    }
    csb_v1_dungeon_free(&handoff->dungeon);
    memset(handoff, 0, sizeof(*handoff));
}

int csb_v1_x68k_startup_handoff_admit(
    CSB_V1_X68kStartupHandoff *out_handoff,
    const uint8_t *hdm, size_t hdm_size)
{
    CSB_V1_X68kHdmReceipt dungeon_media;

    if (!out_handoff || !hdm) {
        return CSB_V1_X68K_STARTUP_HANDOFF_ERR_ARGUMENT;
    }
    memset(out_handoff, 0, sizeof(*out_handoff));
    if (csb_v1_x68k_hdm_source_media_receipt(hdm, hdm_size,
                                             &out_handoff->media) !=
        CSB_V1_X68K_SOURCE_MEDIA_OK) {
        return CSB_V1_X68K_STARTUP_HANDOFF_ERR_MEDIA;
    }
    if (!M11_AssetLoader_InitCsbX68kFromHdm(&out_handoff->graphics, hdm,
                                             hdm_size) ||
        !out_handoff->graphics.csbX68k || out_handoff->graphics.csbAmiga) {
        csb_v1_x68k_startup_handoff_cleanup(out_handoff);
        return CSB_V1_X68K_STARTUP_HANDOFF_ERR_GRAPHICS;
    }
    memset(&dungeon_media, 0, sizeof(dungeon_media));
    if (csb_v1_x68k_hdm_load_dungeon(&out_handoff->dungeon, hdm, hdm_size,
                                     &dungeon_media) !=
        CSB_V1_X68K_DUNGEON_HANDOFF_OK) {
        csb_v1_x68k_startup_handoff_cleanup(out_handoff);
        return CSB_V1_X68K_STARTUP_HANDOFF_ERR_DUNGEON;
    }
    if (!csb_v1_dungeon_initial_party_pose_pc34(
            &out_handoff->dungeon, &out_handoff->initial_level,
            &out_handoff->initial_x, &out_handoff->initial_y,
            &out_handoff->initial_direction)) {
        csb_v1_x68k_startup_handoff_cleanup(out_handoff);
        return CSB_V1_X68K_STARTUP_HANDOFF_ERR_POSE;
    }
    out_handoff->x68000_identity_bound =
        out_handoff->media.x68000_identity_bound;
    out_handoff->host_program_execution_permitted = 0;
    out_handoff->admitted = 1;
    return CSB_V1_X68K_STARTUP_HANDOFF_OK;
}
