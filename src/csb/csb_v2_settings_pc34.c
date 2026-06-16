/*
 * csb_v2_settings_pc34.c
 *
 * CSB V2 settings bridge. M12_Config <-> CSB V2 settings struct
 * <-> csb_v2_upscale + csb_v2_presentation_mode runtime. Mirror of
 * dm1_v2_settings_pc34.c.
 */

#include "csb_v2_settings_pc34.h"
#include "config_m12.h"
#include "csb_v2_texture_upscale_pc34.h"
#include "csb_v22_shapes.h"
#include "csb_v2_presentation_mode_pc34.h"

void csb_v2_settings_defaults(CSB_V2_Settings* settings) {
    if (!settings) return;
    settings->scalePercent = 200;
    settings->bilinearEnabled = 0;
    settings->crtScanlinesEnabled = 0;
    settings->crtScanlineStrength = 35;
    settings->paletteCorrectionEnabled = 0;
    settings->ditherCleanupEnabled = 0;
}

void csb_v2_settings_sanitize(CSB_V2_Settings* settings) {
    if (!settings) return;
    if (settings->scalePercent < 100) settings->scalePercent = 100;
    if (settings->scalePercent > 400) settings->scalePercent = 400;
    if (settings->bilinearEnabled < 0) settings->bilinearEnabled = 0;
    if (settings->bilinearEnabled > 1) settings->bilinearEnabled = 1;
    if (settings->crtScanlinesEnabled < 0) settings->crtScanlinesEnabled = 0;
    if (settings->crtScanlinesEnabled > 1) settings->crtScanlinesEnabled = 1;
    if (settings->crtScanlineStrength < 0) settings->crtScanlineStrength = 0;
    if (settings->crtScanlineStrength > 100) settings->crtScanlineStrength = 100;
    if (settings->paletteCorrectionEnabled < 0) settings->paletteCorrectionEnabled = 0;
    if (settings->paletteCorrectionEnabled > 1) settings->paletteCorrectionEnabled = 1;
    if (settings->ditherCleanupEnabled < 0) settings->ditherCleanupEnabled = 0;
    if (settings->ditherCleanupEnabled > 1) settings->ditherCleanupEnabled = 1;
}

void csb_v2_settings_from_m12_config(CSB_V2_Settings* settings,
                                     const M12_Config* config) {
    if (!settings) return;
    csb_v2_settings_defaults(settings);
    if (config) {
        settings->scalePercent = config->csbV2ScalePercent;
        settings->bilinearEnabled = config->csbV2BilinearEnabled;
        settings->crtScanlinesEnabled = config->csbV2CrtScanlinesEnabled;
        settings->crtScanlineStrength = config->csbV2CrtScanlineStrength;
        settings->paletteCorrectionEnabled = config->csbV2PaletteCorrectionEnabled;
        settings->ditherCleanupEnabled = config->csbV2DitherCleanupEnabled;
    }
    csb_v2_settings_sanitize(settings);
}

void csb_v2_settings_apply_to_m12_config(M12_Config* config,
                                          const CSB_V2_Settings* settings) {
    CSB_V2_Settings copy;
    if (!config) return;
    if (settings) {
        copy = *settings;
        csb_v2_settings_sanitize(&copy);
    } else {
        csb_v2_settings_defaults(&copy);
    }
    config->csbV2ScalePercent = copy.scalePercent;
    config->csbV2BilinearEnabled = copy.bilinearEnabled;
    config->csbV2CrtScanlinesEnabled = copy.crtScanlinesEnabled;
    config->csbV2CrtScanlineStrength = copy.crtScanlineStrength;
    config->csbV2PaletteCorrectionEnabled = copy.paletteCorrectionEnabled;
    config->csbV2DitherCleanupEnabled = copy.ditherCleanupEnabled;
}

void csb_v2_settings_apply_to_runtime(const CSB_V2_Settings* settings) {
    CSB_V2_Settings copy;
    if (settings) {
        copy = *settings;
        csb_v2_settings_sanitize(&copy);
    } else {
        csb_v2_settings_defaults(&copy);
    }
    /* Push the scale into the V2.1 EPX pipeline. The presentation
     * mode gate (V20/V21/V22) decides whether EPX runs at all;
     * here we just set the global scale config so csb_v2_upscale_*
     * uses the user's preferred scale. */
    {
        CSB_V2_TextureUpscaleConfig uc;
        csb_v2_upscale_init(NULL);
        uc.scale_factor = copy.scalePercent / 100;
        if (uc.scale_factor != 1 && uc.scale_factor != 2 && uc.scale_factor != 4) {
            uc.scale_factor = 2;
        }
        uc.use_bilinear = copy.bilinearEnabled ? true : false;
        uc.sharpen = 0;
        csb_v2_upscale_init(&uc);
    }
}

const char* csb_v2_settings_source_evidence(void) {
    return
        "CSB V2 settings bridge: M12_Config <-> CSB_V2_Settings <-> runtime.\n"
        "  Mirror of dm1_v2_settings_pc34 for CSB.\n"
        "  Persists scale (1x/2x/4x), bilinear toggle, scanline + palette +\n"
        "  dither cleanup flags. Applies to csb_v2_upscale_init() + the V2\n"
        "  presentation runtime.\n"
        "  Source: include/csb_v2_texture_upscale_pc34.h + include/csb_v22_shapes.h\n"
        "  + include/csb_v2_presentation_mode_pc34.h + include/config_m12.h.\n";
}
