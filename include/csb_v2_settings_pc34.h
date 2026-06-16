#ifndef FIRESTAFF_CSB_V2_SETTINGS_PC34_H
#define FIRESTAFF_CSB_V2_SETTINGS_PC34_H

/*
 * csb_v2_settings_pc34.h
 *
 * CSB V2.0/V2.1/V2.2 settings bridge. Companion to
 * dm1_v2_settings_pc34.h but for CSB. The CSB V2.1 upscale +
 * V2.2 shape modules consume a small per-game V2 settings struct
 * (scale factor, bilinear toggle, filter-chain toggles).
 *
 * Pattern: M12_Config (which holds the persisted csbV2* fields)
 * <-> this CSB V2 settings struct <-> csb_v2_upscale_init() +
 * csb_v2_presentation_mode_set(). The DM1 equivalent is
 * dm1_v2_settings_from_m12_config / dm1_v2_settings_apply_to_m12_config.
 *
 * Source-lock references:
 *   - include/dm1_v2_settings_pc34.h (mirror reference)
 *   - include/csb_v2_texture_upscale_pc34.h
 *   - include/csb_v22_shapes.h
 *   - include/csb_v2_presentation_mode_pc34.h
 *   - include/config_m12.h
 *
 * Module: src/csb/csb_v2_settings_pc34.c
 * Test:   tests/test_csb_v2_settings_pc34.c
 * Probe:  probes/firestaff_csb_v2_settings_probe.c
 */

#include <stdint.h>
#include "config_m12.h"

#ifdef __cplusplus
extern "C" {
#endif

/* CSB V2 settings — read from M12_Config, applied to the
 * CSB V2.1 upscale + V2.2 module set. */
typedef struct {
    int scalePercent;          /* 100-400, default 200 (2x EPX) */
    int bilinearEnabled;       /* 0 = EPX, 1 = bilinear; default 0 */
    int crtScanlinesEnabled;    /* 0/1, default 0 */
    int crtScanlineStrength;    /* 0-100, default 35 */
    int paletteCorrectionEnabled; /* 0/1, default 0 */
    int ditherCleanupEnabled;   /* 0/1, default 0 */
} CSB_V2_Settings;

void csb_v2_settings_defaults(CSB_V2_Settings* settings);
void csb_v2_settings_sanitize(CSB_V2_Settings* settings);

/* M12_Config -> CSB V2 settings. config can be NULL; defaults
 * are loaded in that case. */
void csb_v2_settings_from_m12_config(CSB_V2_Settings* settings,
                                     const M12_Config* config);

/* CSB V2 settings -> M12_Config. settings can be NULL; defaults
 * are written in that case. */
void csb_v2_settings_apply_to_m12_config(M12_Config* config,
                                          const CSB_V2_Settings* settings);

/* Apply the settings to the live CSB V2 modules (csb_v2_upscale
 * global config + presentation mode). settings can be NULL; the
 * module defaults are used in that case. */
void csb_v2_settings_apply_to_runtime(const CSB_V2_Settings* settings);

const char* csb_v2_settings_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CSB_V2_SETTINGS_PC34_H */
