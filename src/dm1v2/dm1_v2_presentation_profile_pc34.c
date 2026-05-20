#include "dm1_v2_presentation_profile_pc34.h"

#include <string.h>

/* DM1 V2 presentation profile boundary.
 *
 * Source-lock anchors:
 * - ReDMCSB GAMELOOP.C:90 calls F0128_DUNGEONVIEW_Draw_CPSF with the V1
 *   gameplay globals G0308_i_PartyDirection, G0306_i_PartyMapX and
 *   G0307_i_PartyMapY.
 * - ReDMCSB GAMELOOP.C:164-219 owns the input wait loop and command queue
 *   dispatch through F0380_COMMAND_ProcessQueue_CPSC; presentation state must
 *   not short-circuit that loop.
 * - ReDMCSB DUNVIEW.C:2999-3000 fixes the original viewport bitmap dimensions
 *   on G0296_puc_Bitmap_Viewport before presentation.
 * - ReDMCSB DUNVIEW.C:8318-8338 starts F0128_DUNGEONVIEW_Draw_CPSF from a
 *   direction/map snapshot, not from renderer preferences.
 * - ReDMCSB DEFS.H:238-243 and COMMAND.C:2045-2155 keep command ids and
 *   gameplay command dispatch outside presentation configuration.
 *
 * Phase 1 keeps the gameplay route pinned to V1. Enabling V2 changes only the
 * presentation mode and V2 settings carried beside a copied gameplay snapshot;
 * this module never mutates source gameplay state or command routing. */

void dm1_v2_gameplay_snapshot_init(DM1_V2_GameplaySnapshot* snapshot,
                                   int16_t mapX,
                                   int16_t mapY,
                                   int16_t facingDir,
                                   uint32_t gameTime) {
    if (!snapshot) return;
    memset(snapshot, 0, sizeof(*snapshot));
    snapshot->mapX = mapX;
    snapshot->mapY = mapY;
    snapshot->facingDir = (int16_t)(facingDir & 3);
    snapshot->gameTime = gameTime;
}

void dm1_v2_presentation_profile_defaults(DM1_V2_PresentationProfile* profile) {
    if (!profile) return;
    memset(profile, 0, sizeof(*profile));
    profile->gameplayRoute = DM1_V2_GAMEPLAY_ROUTE_V1_SOURCE;
    profile->presentationMode = DM1_V2_PRESENTATION_MODE_V1_ORIGINAL;
    profile->v2PresentationEnabled = 0;
    dm1_v2_settings_defaults(&profile->settings);
    dm1_v2_gameplay_snapshot_init(&profile->snapshot, 0, 0, 0, 0U);
    profile->presentationFrameCount = 0U;
    profile->presentationDirty = 0;
}

void dm1_v2_presentation_profile_from_m12_config(DM1_V2_PresentationProfile* profile,
                                                 const M12_Config* config) {
    int enableV2;
    if (!profile) return;
    dm1_v2_presentation_profile_defaults(profile);
    dm1_v2_settings_from_m12_config(&profile->settings, config);
    enableV2 = config && config->graphicsIndex == 1;
    dm1_v2_presentation_profile_set_enabled(profile, enableV2);
}

void dm1_v2_presentation_profile_set_enabled(DM1_V2_PresentationProfile* profile,
                                             int enabled) {
    if (!profile) return;
    profile->gameplayRoute = DM1_V2_GAMEPLAY_ROUTE_V1_SOURCE;
    profile->v2PresentationEnabled = enabled ? 1 : 0;
    profile->presentationMode = enabled
        ? DM1_V2_PRESENTATION_MODE_V2_SHELL
        : DM1_V2_PRESENTATION_MODE_V1_ORIGINAL;
    profile->presentationDirty = 1;
}

void dm1_v2_presentation_profile_bind_snapshot(DM1_V2_PresentationProfile* profile,
                                               const DM1_V2_GameplaySnapshot* snapshot) {
    if (!profile || !snapshot) return;
    profile->snapshot = *snapshot;
    profile->snapshot.facingDir = (int16_t)(profile->snapshot.facingDir & 3);
    profile->presentationDirty = 1;
}

void dm1_v2_presentation_profile_tick(DM1_V2_PresentationProfile* profile) {
    if (!profile) return;
    if (profile->v2PresentationEnabled) {
        profile->presentationFrameCount++;
    }
    profile->presentationDirty = 0;
}

int dm1_v2_presentation_profile_uses_v1_gameplay(const DM1_V2_PresentationProfile* profile) {
    return profile && profile->gameplayRoute == DM1_V2_GAMEPLAY_ROUTE_V1_SOURCE;
}

const char* dm1_v2_presentation_profile_source_evidence(void) {
    return
        "GAMELOOP.C:90 draws from V1 map/direction globals; "
        "GAMELOOP.C:164-219 owns command wait/dispatch; "
        "DUNVIEW.C:2999-3000 fixes viewport bitmap dimensions; "
        "DUNVIEW.C:8318-8338 draws from a direction/map snapshot; "
        "DEFS.H:238-243 and COMMAND.C:2045-2155 keep commands in V1 gameplay.";
}
