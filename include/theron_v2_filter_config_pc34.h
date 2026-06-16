#ifndef FIRESTAFF_THERON_V2_FILTER_CONFIG_PC34_H
#define FIRESTAFF_THERON_V2_FILTER_CONFIG_PC34_H

/*
 * theron_v2_filter_config_pc34.h
 *
 * Theron V2 filter chain config: persistent storage for the per-frame
 * filter toggles (CRT scanlines, palette correction, dither cleanup).
 * Parallel to include/csb_v2_filter_config_pc34.h but for the
 * PC Engine CD (HuC6260 VDC + HuC6270 VCE) Theron pipeline.
 *
 * Source-lock references:
 *   - include/theron_v2_settings_pc34.h (bridge module)
 *   - include/theron_v2_filters.h (per-frame filter functions, when
 *     they exist; some Theron filter work is shared with CSB
 *     csb_v2_filter_*)
 *   - include/theron_v2_texture_upscale_pc34.h (parallel pattern)
 *   - include/dm1_v2_settings_pc34.h (mirror reference)
 *   - include/config_m12.h (theronV2* fields)
 *   - THQUEST.ASM T400/T520/T600
 *   - HuC6260/HuC6270 VDC/VCE
 *
 * Module: src/theron/theron_v2_filter_config_pc34.c
 * Test:   tests/test_theron_v2_filter_config_pc34.c
 * Probe:  probes/firestaff_theron_v2_filter_config_probe.c
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
} Theron_V2_FilterConfig;

void theron_v2_filter_config_defaults(Theron_V2_FilterConfig* config);
void theron_v2_filter_config_sanitize(Theron_V2_FilterConfig* config);

const Theron_V2_FilterConfig* theron_v2_filter_config_get(void);

void theron_v2_filter_config_apply(const Theron_V2_FilterConfig* config);

void theron_v2_filter_config_reset(void);

const char* theron_v2_filter_config_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_THERON_V2_FILTER_CONFIG_PC34_H */
