#ifndef FIRESTAFF_THERON_V2_SETTINGS_PC34_H
#define FIRESTAFF_THERON_V2_SETTINGS_PC34_H

/*
 * theron_v2_settings_pc34.h
 *
 * Theron V2.0/V2.1/V2.2 settings bridge. Companion to
 * dm1_v2_settings_pc34.h and csb_v2_settings_pc34.h but for
 * Theron's Quest (PC Engine CD, HuC6280 + HuC6260 VDC + HuC6270 VCE).
 *
 * Same pattern as csb_v2_settings_pc34.h: M12_Config <-> Theron V2
 * settings struct <-> theron_v2_upscale_init() +
 * theron_v2_presentation_mode_set().
 *
 * Source-lock references:
 *   - include/dm1_v2_settings_pc34.h, include/csb_v2_settings_pc34.h
 *   - include/theron_v2_texture_upscale_pc34.h
 *   - include/theron_v22_shapes.h
 *   - include/theron_v2_presentation_mode_pc34.h
 *   - include/config_m12.h
 *   - THQUEST.ASM T400/T520/T600
 *   - HuC6260/HuC6270 VDC/VCE datasheet
 *
 * Module: src/theron/theron_v2_settings_pc34.c
 * Test:   tests/test_theron_v2_settings_pc34.c
 * Probe:  probes/firestaff_theron_v2_settings_probe.c
 */

#include <stdint.h>
#include "config_m12.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int scalePercent;          /* 100-400, default 200 (2x EPX) */
    int bilinearEnabled;       /* 0 = EPX, 1 = bilinear; default 0 */
    int crtScanlinesEnabled;    /* 0/1, default 0 */
    int crtScanlineStrength;    /* 0-100, default 35 */
    int paletteCorrectionEnabled; /* 0/1, default 0 */
    int ditherCleanupEnabled;   /* 0/1, default 0 */
} Theron_V2_Settings;

void theron_v2_settings_defaults(Theron_V2_Settings* settings);
void theron_v2_settings_sanitize(Theron_V2_Settings* settings);

void theron_v2_settings_from_m12_config(Theron_V2_Settings* settings,
                                       const M12_Config* config);

void theron_v2_settings_apply_to_m12_config(M12_Config* config,
                                             const Theron_V2_Settings* settings);

void theron_v2_settings_apply_to_runtime(const Theron_V2_Settings* settings);

const char* theron_v2_settings_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_THERON_V2_SETTINGS_PC34_H */
