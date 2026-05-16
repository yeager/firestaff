#include "dm1_v2_settings_pc34.h"

/* DM1 V2 release-safe settings adapter.
 *
 * Source-lock intent:
 * - ReDMCSB primary anchor DUNVIEW.C:F0128_DUNGEONVIEW_Draw_CPSF writes the
 *   original viewport bitmap dimensions through G2073_C224_ViewportPixelWidth
 *   and G2074_C136_ViewportHeight. V2 settings may scale/present that picture,
 *   but they must not redefine gameplay-space geometry.
 * - V2 settings live in the existing M12 config file so data-dir/package
 *   behavior stays unchanged.
 * - V1 behavior is not read from these fields; V2 consumers opt in through
 *   this adapter.
 * - Aspect mode never means stretch-to-fill. ORIGINAL_4_3 preserves the DM1
 *   picture shape; WIDESCREEN_16_9 is an explicit V2 presentation envelope for
 *   letterbox/pillarbox or designed side-area rendering. */

#define DM1_V2_SCALE_MIN 100
#define DM1_V2_SCALE_MAX 400
#define DM1_V2_SCALE_DEFAULT 100

void dm1_v2_settings_defaults(DM1_V2_Settings* settings) {
    if (!settings) return;
    settings->scalePercent = DM1_V2_SCALE_DEFAULT;
    settings->smoothingEnabled = 1;
    settings->dynamicLightingEnabled = 1;
    settings->accessibilityTouchEnabled = 0;
    settings->aspectMode = DM1_V2_ASPECT_ORIGINAL_4_3;
}

void dm1_v2_settings_sanitize(DM1_V2_Settings* settings) {
    if (!settings) return;
    if (settings->scalePercent < DM1_V2_SCALE_MIN) {
        settings->scalePercent = DM1_V2_SCALE_MIN;
    }
    if (settings->scalePercent > DM1_V2_SCALE_MAX) {
        settings->scalePercent = DM1_V2_SCALE_MAX;
    }
    settings->smoothingEnabled = settings->smoothingEnabled ? 1 : 0;
    settings->dynamicLightingEnabled = settings->dynamicLightingEnabled ? 1 : 0;
    settings->accessibilityTouchEnabled = settings->accessibilityTouchEnabled ? 1 : 0;
    if (settings->aspectMode != DM1_V2_ASPECT_WIDESCREEN_16_9) {
        settings->aspectMode = DM1_V2_ASPECT_ORIGINAL_4_3;
    }
}

void dm1_v2_settings_from_m12_config(DM1_V2_Settings* settings,
                                     const M12_Config* config) {
    if (!settings) return;
    dm1_v2_settings_defaults(settings);
    if (config) {
        settings->scalePercent = config->dm1V2ScalePercent;
        settings->smoothingEnabled = config->dm1V2SmoothingEnabled;
        settings->dynamicLightingEnabled = config->dm1V2DynamicLightingEnabled;
        settings->accessibilityTouchEnabled = config->dm1V2AccessibilityTouchEnabled;
        settings->aspectMode = config->dm1V2AspectMode == 1
            ? DM1_V2_ASPECT_WIDESCREEN_16_9
            : DM1_V2_ASPECT_ORIGINAL_4_3;
    }
    dm1_v2_settings_sanitize(settings);
}

void dm1_v2_settings_apply_to_m12_config(M12_Config* config,
                                         const DM1_V2_Settings* settings) {
    DM1_V2_Settings copy;
    if (!config) return;
    if (settings) {
        copy = *settings;
        dm1_v2_settings_sanitize(&copy);
    } else {
        dm1_v2_settings_defaults(&copy);
    }
    config->dm1V2ScalePercent = copy.scalePercent;
    config->dm1V2SmoothingEnabled = copy.smoothingEnabled;
    config->dm1V2DynamicLightingEnabled = copy.dynamicLightingEnabled;
    config->dm1V2AccessibilityTouchEnabled = copy.accessibilityTouchEnabled;
    config->dm1V2AspectMode = (copy.aspectMode == DM1_V2_ASPECT_WIDESCREEN_16_9) ? 1 : 0;
}

const char* dm1_v2_settings_aspect_id(DM1_V2_AspectMode mode) {
    return mode == DM1_V2_ASPECT_WIDESCREEN_16_9
        ? "16:9-widescreen"
        : "4:3-original";
}

/* ══════════════════════════════════════════════════════════════════════
 * V2.1 Settings — Upscale configuration
 * ══════════════════════════════════════════════════════════════════════ */

void v2_settings_apply_v21_defaults(DM1_V2_Settings *s) {
    if (!s) return;
    s->viewport_scale = 2;       /* EPX 2x by default */
    s->use_epx = 1;              /* EPX enabled */
    s->use_bilinear = 0;         /* no bilinear on indexed data */
    s->palette_enhanced = 0;     /* original VGA palette */
    s->sound_enabled = 1;
    s->music_enabled = 0;        /* PC-34 has no music */
    s->fullscreen = 0;
    s->vsync = 1;
}

const char *v21_settings_source_evidence(void) {
    return "V2.1 defaults: EPX 2x, original VGA palette, PC-34 audio\n";
}

