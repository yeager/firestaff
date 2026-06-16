/*
 * dm1_v2_presentation_mode_pc34.c
 * DM1 V2 presentation-mode selection (V2.0 / V2.1 / V2.2).
 */
#include "dm1_v2_presentation_mode_pc34.h"
#include "dm1_v2_texture_upscale_pc34.h"
#include "dm1_v22_shapes.h"
#include "dm1_v2_settings_pc34.h"
#include "dm1_v2_asset_pipeline_pc34.h"
#include <string.h>

static DM1_V2_PresentationModeState g_pm_state;
static int g_pm_pack_override_valid = 0;
static int g_pm_pack_override_value = 0;

static int pm_modern_pack_detected(void) {
    if (g_pm_pack_override_valid) return g_pm_pack_override_value;
    return m11_v22_modern_assets_available();
}

static void pm_recompute(DM1_V2_PresentationModeKind resolved) {
    memset(&g_pm_state, 0, sizeof(g_pm_state));
    g_pm_state.kind = resolved;
    g_pm_state.v2Active = (resolved != DM1_V2_PM_V1_FAITHFUL);
    g_pm_state.v20FilterActive = (resolved == DM1_V2_PM_V20_FILTERED);
    g_pm_state.v21UpscaleActive = (resolved == DM1_V2_PM_V21_UPSCALED);
    g_pm_state.v22ModernActive = (resolved == DM1_V2_PM_V22_MODERN);
    g_pm_state.modernPackAvailable = pm_modern_pack_detected();
    switch (resolved) {
        case DM1_V2_PM_V1_FAITHFUL:  g_pm_state.upscaleScale = 1; break;
        case DM1_V2_PM_V20_FILTERED: g_pm_state.upscaleScale = 2; break;
        case DM1_V2_PM_V21_UPSCALED: g_pm_state.upscaleScale = 2; break;
        case DM1_V2_PM_V22_MODERN:   g_pm_state.upscaleScale = 2; break;
    }
    g_pm_state.setCount++;
}

void dm1_v2_presentation_mode_set(DM1_V2_PresentationModeKind kind) {
    DM1_V2_PresentationModeKind resolved = dm1_v2_presentation_mode_resolve(
        kind, pm_modern_pack_detected());
    pm_recompute(resolved);
    v2_upscale_set_scale(g_pm_state.upscaleScale);
    if (g_pm_state.v22ModernActive) m11_v22_shapes_init();
}

void dm1_v2_presentation_mode_set_m12(int m12PresentationMode) {
    DM1_V2_PresentationModeKind kind;
    switch (m12PresentationMode) {
        case 0:  kind = DM1_V2_PM_V1_FAITHFUL;  break;
        case 1:  kind = DM1_V2_PM_V20_FILTERED; break;
        case 2:  kind = DM1_V2_PM_V21_UPSCALED; break;
        case 3:  kind = DM1_V2_PM_V22_MODERN;   break;
        default: kind = DM1_V2_PM_V1_FAITHFUL;  break;
    }
    dm1_v2_presentation_mode_set(kind);
}

DM1_V2_PresentationModeKind dm1_v2_presentation_mode_get(void) { return g_pm_state.kind; }
const DM1_V2_PresentationModeState* dm1_v2_presentation_mode_state(void) { return &g_pm_state; }

void dm1_v2_presentation_mode_reset(void) {
    g_pm_pack_override_valid = 0;
    g_pm_pack_override_value = 0;
    pm_recompute(DM1_V2_PM_V1_FAITHFUL);
    v2_upscale_set_scale(1);
}

DM1_V2_PresentationModeKind dm1_v2_presentation_mode_resolve(
    DM1_V2_PresentationModeKind requested, int modernPackAvailable) {
    if (requested == DM1_V2_PM_V1_FAITHFUL) return DM1_V2_PM_V1_FAITHFUL;
    if (requested == DM1_V2_PM_V22_MODERN && !modernPackAvailable) return DM1_V2_PM_V21_UPSCALED;
    return requested;
}

int dm1_v2_presentation_mode_is_v1(void)  { return g_pm_state.kind == DM1_V2_PM_V1_FAITHFUL; }
int dm1_v2_presentation_mode_is_v20(void) { return g_pm_state.v20FilterActive; }
int dm1_v2_presentation_mode_is_v21(void) { return g_pm_state.v21UpscaleActive; }
int dm1_v2_presentation_mode_is_v22(void) { return g_pm_state.v22ModernActive; }

void dm1_v2_presentation_mode_set_modern_pack_available(int available) {
    g_pm_pack_override_valid = 1;
    g_pm_pack_override_value = (available != 0);
    g_pm_state.modernPackAvailable = (available != 0);
    if (g_pm_state.kind == DM1_V2_PM_V22_MODERN && !g_pm_state.modernPackAvailable) {
        pm_recompute(DM1_V2_PM_V21_UPSCALED);
    }
}

const char* dm1_v2_presentation_mode_name(DM1_V2_PresentationModeKind kind) {
    switch (kind) {
        case DM1_V2_PM_V1_FAITHFUL:  return "V1";
        case DM1_V2_PM_V20_FILTERED: return "V2.0";
        case DM1_V2_PM_V21_UPSCALED: return "V2.1";
        case DM1_V2_PM_V22_MODERN:   return "V2.2";
    }
    return "UNKNOWN";
}

const char* dm1_v2_presentation_mode_source_evidence(void) {
    return
        "DM1 V2 presentation-mode wiring:\n"
        "  ReDMCSB COMMAND.C F0359 LoadGameSettings - launcher menu selection\n"
        "  ReDMCSB COMMAND.C F0361 input dispatch - inherits mode\n"
        "  ReDMCSB CLIKMENU.C F0365/F0366 - V1 source-locked turn/move\n"
        "  CSBWin/Graphics.cpp 3186 - V2.0 palette+filter pair\n"
        "  CSBWin/Viewport.cpp 7290 - V2.1 EPX-style viewport blit\n"
        "  DM1 PC 3.4 GRAPHICS.DAT - V2.2 modern asset pack\n"
        "  M12_PRESENTATION_V1_ORIGINAL=0  -> V1_FAITHFUL\n"
        "  M12_PRESENTATION_V20_FILTERED=1 -> V20_FILTERED\n"
        "  M12_PRESENTATION_V21_UPSCALED=2 -> V21_UPSCALED\n"
        "  M12_PRESENTATION_V22_MODERN=3   -> V22_MODERN\n"
        "  Fallback chain: V22->V21->V20->V1\n";
}
