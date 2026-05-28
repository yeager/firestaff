#ifndef FIRESTAFF_DM1_V2_SETTINGS_PC34_H
#define FIRESTAFF_DM1_V2_SETTINGS_PC34_H

#include "config_m12.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    DM1_V2_ASPECT_ORIGINAL_4_3 = 0,
    DM1_V2_ASPECT_WIDESCREEN_16_9 = 1
} DM1_V2_AspectMode;

typedef struct {
    int scalePercent;
    int smoothingEnabled;
    int dynamicLightingEnabled;
    int accessibilityTouchEnabled;
    DM1_V2_AspectMode aspectMode;
    int smoothTurnPanEnabled;
    int viewport_scale;
    int use_epx;
    int use_bilinear;
    int palette_enhanced;
    int sound_enabled;
    int music_enabled;
    int fullscreen;
    int vsync;
} DM1_V2_Settings;

void dm1_v2_settings_defaults(DM1_V2_Settings* settings);
void dm1_v2_settings_sanitize(DM1_V2_Settings* settings);
void dm1_v2_settings_from_m12_config(DM1_V2_Settings* settings,
                                     const M12_Config* config);
void dm1_v2_settings_apply_to_m12_config(M12_Config* config,
                                         const DM1_V2_Settings* settings);
const char* dm1_v2_settings_aspect_id(DM1_V2_AspectMode mode);

void v2_settings_apply_v21_defaults(DM1_V2_Settings* settings);
int v2_settings_save_to_file(const DM1_V2_Settings* settings, const char* path);
int v2_settings_load_from_file(DM1_V2_Settings* settings, const char* path);
const char* v21_settings_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V2_SETTINGS_PC34_H */
