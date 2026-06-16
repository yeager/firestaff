/*
 * csb_v2_filter_config_pc34.c
 *
 * CSB V2 filter chain config — see include/csb_v2_filter_config_pc34.h
 * for the design contract and source-lock list.
 */

#include "csb_v2_filter_config_pc34.h"
#include "csb_v2_filters.h"

static CSB_V2_FilterConfig g_csb_v2_filter_config;

void csb_v2_filter_config_defaults(CSB_V2_FilterConfig* config) {
    if (!config) return;
    config->crtScanlinesEnabled = 0;
    config->crtScanlineStrength = 35;
    config->paletteCorrectionEnabled = 0;
    config->ditherCleanupEnabled = 0;
}

void csb_v2_filter_config_sanitize(CSB_V2_FilterConfig* config) {
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

const CSB_V2_FilterConfig* csb_v2_filter_config_get(void) {
    return &g_csb_v2_filter_config;
}

void csb_v2_filter_config_apply(const CSB_V2_FilterConfig* config) {
    CSB_V2_FilterConfig copy;
    if (config) {
        copy = *config;
        csb_v2_filter_config_sanitize(&copy);
    } else {
        csb_v2_filter_config_defaults(&copy);
    }
    g_csb_v2_filter_config = copy;
}

void csb_v2_filter_config_reset(void) {
    csb_v2_filter_config_defaults(&g_csb_v2_filter_config);
}

int csb_v2_filter_chain_apply_indexed(unsigned char* fb, int w, int h) {
    const CSB_V2_FilterConfig* fc;
    int applied = 0;
    if (!fb || w <= 0 || h <= 0) return 0;
    fc = csb_v2_filter_config_get();
    if (!fc) return 0;
    /* dither_cleanup: 3x3 mode filter on the indexed framebuffer.
     * Requires w > 2 and h > 2 to be meaningful. */
    if (fc->ditherCleanupEnabled) {
        if (w > 2 && h > 2) {
            (void)csb_v2_filter_dither_cleanup_indexed(fb, w, h);
            applied++;
        }
    }
    /* palette_interpolate: smooth per-pixel brightness gradient.
     * Strength defaults to crtScanlineStrength if a non-zero
     * value is available; otherwise 50% (mid-strength). */
    if (fc->paletteCorrectionEnabled) {
        if (w > 2 && h > 2) {
            int strength = fc->crtScanlineStrength;
            if (strength <= 0) strength = 50;
            (void)csb_v2_filter_palette_interpolate_indexed(fb, w, h, strength);
            applied++;
        }
    }
    return applied;
}

int csb_v2_filter_chain_apply_rgba(unsigned char* rgba, int w, int h) {
    const CSB_V2_FilterConfig* fc;
    int applied = 0;
    if (!rgba || w <= 2 || h <= 2) return 0;
    fc = csb_v2_filter_config_get();
    if (!fc) return 0;
    /* crt_scanlines: even-row dimming on the RGBA surface. */
    if (fc->crtScanlinesEnabled) {
        (void)csb_v2_filter_crt_scanlines_rgba(rgba, w, h, fc->crtScanlineStrength);
        applied++;
    }
    return applied;
}

const char* csb_v2_filter_config_source_evidence(void) {
    return
        "CSB V2 filter config: persistent storage for the per-frame filter\n"
        "  toggles (CRT scanlines, palette correction, dither cleanup). Set\n"
        "  once at M11 launch via the V2 settings bridge; read by the per-frame\n"
        "  render path. Pattern: same as csb_v2_upscale_global_config.\n"
        "  Source: include/csb_v2_settings_pc34.h + include/csb_v2_filters.h\n"
        "  + include/csb_v2_texture_upscale_pc34.h + include/dm1_v2_settings_pc34.h\n"
        "  + include/config_m12.h.\n";
}
