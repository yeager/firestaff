/*
 * theron_v2_filter_config_pc34.c
 *
 * Theron V2 filter chain config — see include/theron_v2_filter_config_pc34.h
 * for the design contract and source-lock list.
 */

#include "theron_v2_filter_config_pc34.h"

static Theron_V2_FilterConfig g_theron_v2_filter_config;

void theron_v2_filter_config_defaults(Theron_V2_FilterConfig* config) {
    if (!config) return;
    config->crtScanlinesEnabled = 0;
    config->crtScanlineStrength = 35;
    config->paletteCorrectionEnabled = 0;
    config->ditherCleanupEnabled = 0;
}

void theron_v2_filter_config_sanitize(Theron_V2_FilterConfig* config) {
    if (!config) return;
    if (config->crtScanlinesEnabled < 0) config->crtScanlinesEnabled = 0;
    if (config->crtScanlinesEnabled > 1) config->crtScanlinesEnabled = 1;
    if (config->crtScanlineStrength < 0) config->crtScanlineStrength = 0;
    if (config->crtScanlineStrength > 100) config->crtScanlineStrength = 100;
    if (config->paletteCorrectionEnabled < 0) config->paletteCorrectionEnabled = 0;
    if (config->paletteCorrectionEnabled > 1) config->paletteCorrectionEnabled = 1;
    if (config->ditherCleanupEnabled < 0) config->ditherCleanupEnabled = 0;
    if (config->ditherCleanupEnabled > 1) config->ditherCleanupEnabled = 1;
}

const Theron_V2_FilterConfig* theron_v2_filter_config_get(void) {
    return &g_theron_v2_filter_config;
}

void theron_v2_filter_config_apply(const Theron_V2_FilterConfig* config) {
    Theron_V2_FilterConfig copy;
    if (config) {
        copy = *config;
        theron_v2_filter_config_sanitize(&copy);
    } else {
        theron_v2_filter_config_defaults(&copy);
    }
    g_theron_v2_filter_config = copy;
}

void theron_v2_filter_config_reset(void) {
    theron_v2_filter_config_defaults(&g_theron_v2_filter_config);
}

const char* theron_v2_filter_config_source_evidence(void) {
    return
        "Theron V2 filter config: persistent storage for the per-frame filter\n"
        "  toggles (CRT scanlines, palette correction, dither cleanup). Set\n"
        "  once at M11 launch via the V2 settings bridge; read by the per-frame\n"
        "  render path. Pattern: same as theron_v2_upscale_global_config.\n"
        "  Source: include/theron_v2_settings_pc34.h + include/theron_v2_filters.h\n"
        "  + include/theron_v2_texture_upscale_pc34.h + include/dm1_v2_settings_pc34.h\n"
        "  + include/config_m12.h + THQUEST.ASM T400/T520/T600 + HuC6260/HuC6270.\n";
}
