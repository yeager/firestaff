#ifndef FIRESTAFF_DM1_V2_PRESENTATION_PROFILE_PC34_H
#define FIRESTAFF_DM1_V2_PRESENTATION_PROFILE_PC34_H

#include <stdint.h>

#include "dm1_v2_settings_pc34.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    DM1_V2_GAMEPLAY_ROUTE_V1_SOURCE = 0
} DM1_V2_GameplayRoute;

typedef enum {
    DM1_V2_PRESENTATION_MODE_V1_ORIGINAL = 0,
    DM1_V2_PRESENTATION_MODE_V2_SHELL = 1
} DM1_V2_PresentationMode;

typedef struct {
    int16_t mapX;
    int16_t mapY;
    int16_t facingDir;
    uint32_t gameTime;
    int16_t disabledMovementTicks;
    int16_t projectileDisabledMovementTicks;
} DM1_V2_GameplaySnapshot;

typedef struct {
    DM1_V2_GameplayRoute gameplayRoute;
    DM1_V2_PresentationMode presentationMode;
    int v2PresentationEnabled;
    DM1_V2_Settings settings;
    DM1_V2_GameplaySnapshot snapshot;
    uint32_t presentationFrameCount;
    int presentationDirty;
} DM1_V2_PresentationProfile;

void dm1_v2_gameplay_snapshot_init(DM1_V2_GameplaySnapshot* snapshot,
                                   int16_t mapX,
                                   int16_t mapY,
                                   int16_t facingDir,
                                   uint32_t gameTime);
void dm1_v2_presentation_profile_defaults(DM1_V2_PresentationProfile* profile);
void dm1_v2_presentation_profile_from_m12_config(DM1_V2_PresentationProfile* profile,
                                                 const M12_Config* config);
void dm1_v2_presentation_profile_set_enabled(DM1_V2_PresentationProfile* profile,
                                             int enabled);
void dm1_v2_presentation_profile_bind_snapshot(DM1_V2_PresentationProfile* profile,
                                               const DM1_V2_GameplaySnapshot* snapshot);
void dm1_v2_presentation_profile_tick(DM1_V2_PresentationProfile* profile);
int dm1_v2_presentation_profile_uses_v1_gameplay(const DM1_V2_PresentationProfile* profile);
const char* dm1_v2_presentation_profile_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V2_PRESENTATION_PROFILE_PC34_H */
