#ifndef FIRESTAFF_CSB_V2_FILTER_CONFIG_PC34_H
#define FIRESTAFF_CSB_V2_FILTER_CONFIG_PC34_H

/*
 * csb_v2_filter_config_pc34.h
 *
 * CSB V2 filter chain config: persistent storage for the per-frame
 * filter toggles (CRT scanlines, palette correction, dither cleanup).
 * These are set once at M11 launch (via the V2 settings wire-up in
 * M11_GameView_OpenSelectedMenuEntry) and read by the per-frame
 * render path that calls csb_v2_filter_crt_scanlines_rgba(),
 * csb_v2_filter_palette_interpolate_indexed(), etc.
 *
 * Pattern: same as csb_v2_upscale_global_config — module-level
 * static state with thread-unsafe single-writer assumption (the
 * M11 launch is the only writer; the per-frame render is the
 * single reader; no concurrent access).
 *
 * Source-lock references:
 *   - include/csb_v2_settings_pc34.h (bridge module)
 *   - include/csb_v2_filters.h (per-frame filter functions)
 *   - include/csb_v2_texture_upscale_pc34.h (parallel pattern)
 *   - include/dm1_v2_settings_pc34.h (mirror reference)
 *   - include/config_m12.h (csbV2* fields)
 *
 * Module: src/csb/csb_v2_filter_config_pc34.c
 * Test:   tests/test_csb_v2_filter_config_pc34.c
 * Probe:  probes/firestaff_csb_v2_filter_config_probe.c
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int crtScanlinesEnabled;       /* 0/1, default 0 */
    int crtScanlineStrength;       /* 0-100, default 35 */
    int paletteCorrectionEnabled;  /* 0/1, default 0 */
    int ditherCleanupEnabled;      /* 0/1, default 0 */
} CSB_V2_FilterConfig;

void csb_v2_filter_config_defaults(CSB_V2_FilterConfig* config);
void csb_v2_filter_config_sanitize(CSB_V2_FilterConfig* config);

/* Read the live global config (used by the per-frame render path). */
const CSB_V2_FilterConfig* csb_v2_filter_config_get(void);

/* Push a new config into the live global (used by the M11 launch
 * wire-up via the V2 settings bridge). The config is sanitized
 * (out-of-range values clamp) before being applied. */
void csb_v2_filter_config_apply(const CSB_V2_FilterConfig* config);

/* Convenience: reset to defaults. */
void csb_v2_filter_config_reset(void);

const char* csb_v2_filter_config_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CSB_V2_FILTER_CONFIG_PC34_H */
