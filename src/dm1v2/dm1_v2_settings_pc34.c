#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dm1_v2_settings_pc34.h"

/* DM1 V2 release-safe settings adapter.
 *
 * Source-lock intent:
 * - ReDMCSB primary anchor DUNVIEW.C:F0128_DUNGEONVIEW_Draw_CPSF writes the
 *   original viewport bitmap dimensions through G2073_C224_ViewportPixelWidth
 *   and G2074_C136_ViewportHeight. V2 settings may scale/present that picture,
 *   but they must not redefine gameplay-space geometry.
 * - ReDMCSB DEFS.H:238-243 and CLIKMENU.C:142-290 keep V1 command ids,
 *   turn handling, and movement/collision timing outside presentation config.
 * - V2 settings live in the existing M12 config file so data-dir/package
 *   behavior stays unchanged.
 * - V1 behavior is not read from these fields; V2 consumers opt in through
 *   this adapter.
 * - Phase 1 presentation scaffold rule: these fields are deterministic V2-only
 *   render/input preferences carried beside gameplay state, never gameplay
 *   state themselves.
 * - Aspect mode never means stretch-to-fill. ORIGINAL_4_3 preserves the DM1
 *   picture shape; WIDESCREEN_16_9 is an explicit V2 presentation envelope for
 *   letterbox/pillarbox or designed side-area rendering. */

#define DM1_V2_SCALE_MIN 100
#define DM1_V2_SCALE_MAX 400
#define DM1_V2_SCALE_DEFAULT 100

static void dm1_v2_settings_apply_v21_presentation_defaults(DM1_V2_Settings* settings) {
    if (!settings) return;
    settings->viewport_scale = 2;
    settings->use_epx = 1;
    settings->use_bilinear = 0;
    settings->palette_enhanced = 0;
    settings->sound_enabled = 1;
    settings->music_enabled = 0;
    settings->fullscreen = 0;
    settings->vsync = 1;
}

void dm1_v2_settings_defaults(DM1_V2_Settings* settings) {
    if (!settings) return;
    settings->scalePercent = DM1_V2_SCALE_DEFAULT;
    settings->smoothingEnabled = 1;
    settings->dynamicLightingEnabled = 1;
    settings->accessibilityTouchEnabled = 0;
    settings->aspectMode = DM1_V2_ASPECT_ORIGINAL_4_3;
    dm1_v2_settings_apply_v21_presentation_defaults(settings);
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
    if (settings->viewport_scale < 1) {
        settings->viewport_scale = 1;
    }
    if (settings->viewport_scale > 16) {
        settings->viewport_scale = 16;
    }
    settings->use_epx = settings->use_epx ? 1 : 0;
    settings->use_bilinear = settings->use_bilinear ? 1 : 0;
    settings->palette_enhanced = settings->palette_enhanced ? 1 : 0;
    settings->sound_enabled = settings->sound_enabled ? 1 : 0;
    settings->music_enabled = settings->music_enabled ? 1 : 0;
    settings->fullscreen = settings->fullscreen ? 1 : 0;
    settings->vsync = settings->vsync ? 1 : 0;
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
    dm1_v2_settings_apply_v21_presentation_defaults(s);
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


/* ── Settings persistence — save/load to INI file ─────────────────── */

int v2_settings_save_to_file(const DM1_V2_Settings *s, const char *path) {
    DM1_V2_Settings copy;
    FILE *f;
    if (!s || !path) return -1;
    copy = *s;
    dm1_v2_settings_sanitize(&copy);
    f = fopen(path, "w");
    if (!f) return -1;
    fprintf(f, "[display]\n");
    fprintf(f, "scale_percent=%d\n", copy.scalePercent);
    fprintf(f, "smoothing=%d\n", copy.smoothingEnabled);
    fprintf(f, "dynamic_lighting=%d\n", copy.dynamicLightingEnabled);
    fprintf(f, "touch=%d\n", copy.accessibilityTouchEnabled);
    fprintf(f, "aspect_mode=%d\n", (int)copy.aspectMode);
    fprintf(f, "\n[video]\n");
    fprintf(f, "viewport_scale=%d\n", copy.viewport_scale);
    fprintf(f, "epx=%d\n", copy.use_epx);
    fprintf(f, "bilinear=%d\n", copy.use_bilinear);
    fprintf(f, "palette_enhanced=%d\n", copy.palette_enhanced);
    fprintf(f, "vsync=%d\n", copy.vsync);
    fprintf(f, "fullscreen=%d\n", copy.fullscreen);
    fprintf(f, "\n[audio]\n");
    fprintf(f, "sound=%d\n", copy.sound_enabled);
    fprintf(f, "music=%d\n", copy.music_enabled);
    fclose(f);
    return 0;
}

static int ini_read_int(const char *content, const char *key, int default_val) {
    char needle[64];
    const char *p;
    snprintf(needle, sizeof(needle), "%s=", key);
    p = strstr(content, needle);
    if (!p) return default_val;
    return atoi(p + strlen(needle));
}

int v2_settings_load_from_file(DM1_V2_Settings *s, const char *path) {
    FILE *f;
    char buf[2048];
    size_t n;
    if (!s || !path) return -1;
    dm1_v2_settings_defaults(s);
    f = fopen(path, "r");
    if (!f) return -1;
    n = fread(buf, 1, sizeof(buf) - 1, f);
    buf[n] = 0;
    fclose(f);
    s->scalePercent = ini_read_int(buf, "scale_percent", s->scalePercent);
    s->smoothingEnabled = ini_read_int(buf, "smoothing", s->smoothingEnabled);
    s->dynamicLightingEnabled = ini_read_int(buf, "dynamic_lighting", s->dynamicLightingEnabled);
    s->accessibilityTouchEnabled = ini_read_int(buf, "touch", s->accessibilityTouchEnabled);
    s->aspectMode = (DM1_V2_AspectMode)ini_read_int(buf, "aspect_mode", (int)s->aspectMode);
    s->viewport_scale = ini_read_int(buf, "viewport_scale", s->viewport_scale);
    s->use_epx = ini_read_int(buf, "epx", s->use_epx);
    s->use_bilinear = ini_read_int(buf, "bilinear", s->use_bilinear);
    s->palette_enhanced = ini_read_int(buf, "palette_enhanced", s->palette_enhanced);
    s->vsync = ini_read_int(buf, "vsync", s->vsync);
    s->fullscreen = ini_read_int(buf, "fullscreen", s->fullscreen);
    s->sound_enabled = ini_read_int(buf, "sound", s->sound_enabled);
    s->music_enabled = ini_read_int(buf, "music", s->music_enabled);
    dm1_v2_settings_sanitize(s);
    return 0;
}
