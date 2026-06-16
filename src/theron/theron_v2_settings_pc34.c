/*
 * theron_v2_settings_pc34.c
 *
 * Theron V2 settings bridge. M12_Config <-> Theron V2 settings
 * struct <-> theron_v2_upscale + theron_v2_presentation_mode
 * runtime. Mirror of dm1_v2_settings_pc34.c + csb_v2_settings_pc34.c.
 */

#include "theron_v2_settings_pc34.h"
#include "config_m12.h"
#include "theron_v2_texture_upscale_pc34.h"
#include "theron_v2_filter_config_pc34.h"
#include "theron_v22_shapes.h"
#include "theron_v2_presentation_mode_pc34.h"

void theron_v2_settings_defaults(Theron_V2_Settings* settings) {
    if (!settings) return;
    settings->scalePercent = 200;
    settings->bilinearEnabled = 0;
    settings->crtScanlinesEnabled = 0;
    settings->crtScanlineStrength = 35;
    settings->paletteCorrectionEnabled = 0;
    settings->ditherCleanupEnabled = 0;
}

void theron_v2_settings_sanitize(Theron_V2_Settings* settings) {
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

void theron_v2_settings_from_m12_config(Theron_V2_Settings* settings,
                                       const M12_Config* config) {
    if (!settings) return;
    theron_v2_settings_defaults(settings);
    if (config) {
        settings->scalePercent = config->theronV2ScalePercent;
        settings->bilinearEnabled = config->theronV2BilinearEnabled;
        settings->crtScanlinesEnabled = config->theronV2CrtScanlinesEnabled;
        settings->crtScanlineStrength = config->theronV2CrtScanlineStrength;
        settings->paletteCorrectionEnabled = config->theronV2PaletteCorrectionEnabled;
        settings->ditherCleanupEnabled = config->theronV2DitherCleanupEnabled;
    }
    theron_v2_settings_sanitize(settings);
}

void theron_v2_settings_apply_to_m12_config(M12_Config* config,
                                             const Theron_V2_Settings* settings) {
    Theron_V2_Settings copy;
    if (!config) return;
    if (settings) {
        copy = *settings;
        theron_v2_settings_sanitize(&copy);
    } else {
        theron_v2_settings_defaults(&copy);
    }
    config->theronV2ScalePercent = copy.scalePercent;
    config->theronV2BilinearEnabled = copy.bilinearEnabled;
    config->theronV2CrtScanlinesEnabled = copy.crtScanlinesEnabled;
    config->theronV2CrtScanlineStrength = copy.crtScanlineStrength;
    config->theronV2PaletteCorrectionEnabled = copy.paletteCorrectionEnabled;
    config->theronV2DitherCleanupEnabled = copy.ditherCleanupEnabled;
}

void theron_v2_settings_apply_to_runtime(const Theron_V2_Settings* settings) {
    Theron_V2_Settings copy;
    if (settings) {
        copy = *settings;
        theron_v2_settings_sanitize(&copy);
    } else {
        theron_v2_settings_defaults(&copy);
    }
    {
        Theron_V2_TextureUpscaleConfig uc;
        theron_v2_upscale_init(NULL);
        uc.scale_factor = copy.scalePercent / 100;
        if (uc.scale_factor != 1 && uc.scale_factor != 2 && uc.scale_factor != 4) {
            uc.scale_factor = 2;
        }
        uc.use_bilinear = copy.bilinearEnabled ? true : false;
        uc.sharpen = 0;
        theron_v2_upscale_init(&uc);
    }
    {
        Theron_V2_FilterConfig fc;
        fc.crtScanlinesEnabled = copy.crtScanlinesEnabled;
        fc.crtScanlineStrength = copy.crtScanlineStrength;
        fc.paletteCorrectionEnabled = copy.paletteCorrectionEnabled;
        fc.ditherCleanupEnabled = copy.ditherCleanupEnabled;
        theron_v2_filter_config_apply(&fc);
    }
}

const char* theron_v2_settings_source_evidence(void) {
    return
        "Theron V2 settings bridge: M12_Config <-> Theron_V2_Settings <-> runtime.\n"
        "  Mirror of dm1_v2_settings_pc34 + csb_v2_settings_pc34.\n"
        "  Persists scale (1x/2x/4x), bilinear toggle, scanline + palette +\n"
        "  dither cleanup flags. Applies to theron_v2_upscale_init() +\n"
        "  the V2 presentation runtime.\n"
        "  Source: include/theron_v2_texture_upscale_pc34.h + include/theron_v22_shapes.h\n"
        "  + include/theron_v2_presentation_mode_pc34.h + include/config_m12.h\n"
        "  + THQUEST.ASM T400/T520/T600 + HuC6260/HuC6270 VDC/VCE.\n";
}
