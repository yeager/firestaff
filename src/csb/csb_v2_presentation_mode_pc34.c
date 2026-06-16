/*
 * csb_v2_presentation_mode_pc34.c
 * CSB V2 presentation-mode selection (V2.0 / V2.1 / V2.2).
 * Parallel to dm1_v2_presentation_mode_pc34.c.
 */
#include "csb_v2_presentation_mode_pc34.h"
#include "csb_v2_texture_upscale_pc34.h"
#include "csb_v22_shapes.h"
#include <string.h>

extern int m11_v22_modern_assets_available(void);

static CSB_V2_PresentationModeState g_csb_pm_state;
static int g_csb_pm_pack_override_valid = 0;
static int g_csb_pm_pack_override_value = 0;

static int csb_pm_modern_pack_detected(void) {
    if (g_csb_pm_pack_override_valid) return g_csb_pm_pack_override_value;
    return m11_v22_modern_assets_available();
}

static void csb_pm_recompute(CSB_V2_PresentationModeKind resolved) {
    /* Capture setCount before zeroing so the new state carries the
     * incremented value, not a fresh 0+1=1. */
    uint32_t nextCount = g_csb_pm_state.setCount + 1;
    memset(&g_csb_pm_state, 0, sizeof(g_csb_pm_state));
    g_csb_pm_state.kind = resolved;
    g_csb_pm_state.v2Active = (resolved != CSB_V2_PM_V1_FAITHFUL);
    g_csb_pm_state.v20FilterActive = (resolved == CSB_V2_PM_V20_FILTERED);
    g_csb_pm_state.v21UpscaleActive = (resolved == CSB_V2_PM_V21_UPSCALED);
    g_csb_pm_state.v22ModernActive = (resolved == CSB_V2_PM_V22_MODERN);
    g_csb_pm_state.modernPackAvailable = csb_pm_modern_pack_detected();
    switch (resolved) {
        case CSB_V2_PM_V1_FAITHFUL:  g_csb_pm_state.upscaleScale = 1; break;
        case CSB_V2_PM_V20_FILTERED: g_csb_pm_state.upscaleScale = 2; break;
        case CSB_V2_PM_V21_UPSCALED: g_csb_pm_state.upscaleScale = 2; break;
        case CSB_V2_PM_V22_MODERN:   g_csb_pm_state.upscaleScale = 2; break;
    }
    g_csb_pm_state.setCount = nextCount;
}

void csb_v2_presentation_mode_set(CSB_V2_PresentationModeKind kind) {
    CSB_V2_PresentationModeKind resolved = csb_v2_presentation_mode_resolve(
        kind, csb_pm_modern_pack_detected());
    csb_pm_recompute(resolved);
    /* Side-effects: push the active scale into the CSB V2.1 upscale
     * pipeline and initialise the CSB V2.2 shape book on V22 entry.
     * Mirror of dm1_v2_presentation_mode_set() in DM1 V2. */
    csb_v2_upscale_set_scale(g_csb_pm_state.upscaleScale);
    if (g_csb_pm_state.v22ModernActive) {
        csb_v22_shapes_init();
    }
}

void csb_v2_presentation_mode_set_m12(int m12PresentationMode) {
    CSB_V2_PresentationModeKind kind;
    switch (m12PresentationMode) {
        case 0:  kind = CSB_V2_PM_V1_FAITHFUL;  break;
        case 1:  kind = CSB_V2_PM_V20_FILTERED; break;
        case 2:  kind = CSB_V2_PM_V21_UPSCALED; break;
        case 3:  kind = CSB_V2_PM_V22_MODERN;   break;
        default: kind = CSB_V2_PM_V1_FAITHFUL;  break;
    }
    csb_v2_presentation_mode_set(kind);
}

CSB_V2_PresentationModeKind csb_v2_presentation_mode_get(void) { return g_csb_pm_state.kind; }
const CSB_V2_PresentationModeState* csb_v2_presentation_mode_state(void) { return &g_csb_pm_state; }

void csb_v2_presentation_mode_reset(void) {
    g_csb_pm_pack_override_valid = 0;
    g_csb_pm_pack_override_value = 0;
    csb_pm_recompute(CSB_V2_PM_V1_FAITHFUL);
    csb_v2_upscale_set_scale(1);
}

CSB_V2_PresentationModeKind csb_v2_presentation_mode_resolve(
    CSB_V2_PresentationModeKind requested, int modernPackAvailable) {
    if (requested == CSB_V2_PM_V1_FAITHFUL) return CSB_V2_PM_V1_FAITHFUL;
    if (requested == CSB_V2_PM_V22_MODERN && !modernPackAvailable) return CSB_V2_PM_V21_UPSCALED;
    return requested;
}

int csb_v2_presentation_mode_is_v1(void)  { return g_csb_pm_state.kind == CSB_V2_PM_V1_FAITHFUL; }
int csb_v2_presentation_mode_is_v20(void) { return g_csb_pm_state.v20FilterActive; }
int csb_v2_presentation_mode_is_v21(void) { return g_csb_pm_state.v21UpscaleActive; }
int csb_v2_presentation_mode_is_v22(void) { return g_csb_pm_state.v22ModernActive; }

void csb_v2_presentation_mode_set_modern_pack_available(int available) {
    g_csb_pm_pack_override_valid = 1;
    g_csb_pm_pack_override_value = (available != 0);
    g_csb_pm_state.modernPackAvailable = (available != 0);
    if (g_csb_pm_state.kind == CSB_V2_PM_V22_MODERN && !g_csb_pm_state.modernPackAvailable) {
        csb_pm_recompute(CSB_V2_PM_V21_UPSCALED);
    }
}

const char* csb_v2_presentation_mode_name(CSB_V2_PresentationModeKind kind) {
    switch (kind) {
        case CSB_V2_PM_V1_FAITHFUL:  return "V1";
        case CSB_V2_PM_V20_FILTERED: return "V2.0";
        case CSB_V2_PM_V21_UPSCALED: return "V2.1";
        case CSB_V2_PM_V22_MODERN:   return "V2.2";
    }
    return "UNKNOWN";
}

const char* csb_v2_presentation_mode_source_evidence(void) {
    return
        "CSB V2 presentation-mode wiring:\n"
        "  ReDMCSB COMMAND.C F0359 LoadGameSettings - launcher menu\n"
        "  ReDMCSB COMMAND.C F0361 input dispatch\n"
        "  ReDMCSB CLIKMENU.C F0365/F0366 - V1 source-locked turn/move\n"
        "  ReDMCSB DUNGEON.C:35-44 - CSB direction step tables\n"
        "  ReDMCSB GAMELOOP.C:150-155 - V1 tick cadence\n"
        "  ReDMCSB ENTRANCE.C - CSB prison door + intro\n"
        "  CSBWin/Viewport.cpp:7290 - CSB 9-square viewport\n"
        "  CSBWin/Chaos.cpp:60-69 - DSA call dispatch\n"
        "  CSBWin/Graphics.cpp:3186 - V2.0 filter pair\n"
        "  M12_PRESENTATION_V1_ORIGINAL=0  -> V1_FAITHFUL\n"
        "  M12_PRESENTATION_V20_FILTERED=1 -> V20_FILTERED\n"
        "  M12_PRESENTATION_V21_UPSCALED=2 -> V21_UPSCALED\n"
        "  M12_PRESENTATION_V22_MODERN=3   -> V22_MODERN\n"
        "  Fallback chain: V22->V21->V20->V1\n";
}
