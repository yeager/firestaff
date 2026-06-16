/*
 * theron_v2_presentation_mode_pc34.c
 *
 * Theron V2 presentation-mode selection (V2.0 / V2.1 / V2.2).
 * Mirror of dm1_v2_presentation_mode_pc34.c + csb_v2_presentation_mode_pc34.c.
 */
#include "theron_v2_presentation_mode_pc34.h"
#include <string.h>

extern int m11_v22_modern_assets_available(void);

static Theron_V2_PresentationModeState g_theron_pm_state;
static int g_theron_pm_pack_override_valid = 0;
static int g_theron_pm_pack_override_value = 0;

static int theron_pm_modern_pack_detected(void) {
    if (g_theron_pm_pack_override_valid) return g_theron_pm_pack_override_value;
    return m11_v22_modern_assets_available();
}

static void theron_pm_recompute(Theron_V2_PresentationModeKind resolved) {
    uint32_t nextCount = g_theron_pm_state.setCount + 1;
    memset(&g_theron_pm_state, 0, sizeof(g_theron_pm_state));
    g_theron_pm_state.kind = resolved;
    g_theron_pm_state.v2Active = (resolved != THERON_V2_PM_V1_FAITHFUL);
    g_theron_pm_state.v20FilterActive = (resolved == THERON_V2_PM_V20_FILTERED);
    g_theron_pm_state.v21UpscaleActive = (resolved == THERON_V2_PM_V21_UPSCALED);
    g_theron_pm_state.v22ModernActive = (resolved == THERON_V2_PM_V22_MODERN);
    g_theron_pm_state.modernPackAvailable = theron_pm_modern_pack_detected();
    switch (resolved) {
        case THERON_V2_PM_V1_FAITHFUL:  g_theron_pm_state.upscaleScale = 1; break;
        case THERON_V2_PM_V20_FILTERED: g_theron_pm_state.upscaleScale = 2; break;
        case THERON_V2_PM_V21_UPSCALED: g_theron_pm_state.upscaleScale = 2; break;
        case THERON_V2_PM_V22_MODERN:   g_theron_pm_state.upscaleScale = 2; break;
    }
    g_theron_pm_state.setCount = nextCount;
}

void theron_v2_presentation_mode_set(Theron_V2_PresentationModeKind kind) {
    Theron_V2_PresentationModeKind resolved = theron_v2_presentation_mode_resolve(
        kind, theron_pm_modern_pack_detected());
    theron_pm_recompute(resolved);
}

void theron_v2_presentation_mode_set_m12(int m12PresentationMode) {
    Theron_V2_PresentationModeKind kind;
    switch (m12PresentationMode) {
        case 0:  kind = THERON_V2_PM_V1_FAITHFUL;  break;
        case 1:  kind = THERON_V2_PM_V20_FILTERED; break;
        case 2:  kind = THERON_V2_PM_V21_UPSCALED; break;
        case 3:  kind = THERON_V2_PM_V22_MODERN;   break;
        default: kind = THERON_V2_PM_V1_FAITHFUL;  break;
    }
    theron_v2_presentation_mode_set(kind);
}

Theron_V2_PresentationModeKind theron_v2_presentation_mode_get(void) { return g_theron_pm_state.kind; }
const Theron_V2_PresentationModeState* theron_v2_presentation_mode_state(void) { return &g_theron_pm_state; }

void theron_v2_presentation_mode_reset(void) {
    g_theron_pm_pack_override_valid = 0;
    g_theron_pm_pack_override_value = 0;
    theron_pm_recompute(THERON_V2_PM_V1_FAITHFUL);
}

Theron_V2_PresentationModeKind theron_v2_presentation_mode_resolve(
    Theron_V2_PresentationModeKind requested, int modernPackAvailable) {
    if (requested == THERON_V2_PM_V1_FAITHFUL) return THERON_V2_PM_V1_FAITHFUL;
    if (requested == THERON_V2_PM_V22_MODERN && !modernPackAvailable) return THERON_V2_PM_V21_UPSCALED;
    return requested;
}

int theron_v2_presentation_mode_is_v1(void)  { return g_theron_pm_state.kind == THERON_V2_PM_V1_FAITHFUL; }
int theron_v2_presentation_mode_is_v20(void) { return g_theron_pm_state.v20FilterActive; }
int theron_v2_presentation_mode_is_v21(void) { return g_theron_pm_state.v21UpscaleActive; }
int theron_v2_presentation_mode_is_v22(void) { return g_theron_pm_state.v22ModernActive; }

void theron_v2_presentation_mode_set_modern_pack_available(int available) {
    g_theron_pm_pack_override_valid = 1;
    g_theron_pm_pack_override_value = (available != 0);
    g_theron_pm_state.modernPackAvailable = (available != 0);
    if (g_theron_pm_state.kind == THERON_V2_PM_V22_MODERN && !g_theron_pm_state.modernPackAvailable) {
        theron_pm_recompute(THERON_V2_PM_V21_UPSCALED);
    }
}

const char* theron_v2_presentation_mode_name(Theron_V2_PresentationModeKind kind) {
    switch (kind) {
        case THERON_V2_PM_V1_FAITHFUL:  return "V1";
        case THERON_V2_PM_V20_FILTERED: return "V2.0";
        case THERON_V2_PM_V21_UPSCALED: return "V2.1";
        case THERON_V2_PM_V22_MODERN:   return "V2.2";
    }
    return "UNKNOWN";
}

const char* theron_v2_presentation_mode_source_evidence(void) {
    return
        "Theron V2 presentation-mode wiring:\n"
        "  ReDMCSB COMMAND.C F0359 LoadGameSettings - launcher menu\n"
        "  ReDMCSB CLIKMENU.C F0365/F0366 - V1 source-locked turn/move\n"
        "  ReDMCSB MOVESENS.C:475-538 - move/turn sense table\n"
        "  THQUEST.ASM T400 - tile bank loading\n"
        "  THQUEST.ASM T520 - tile selection\n"
        "  THQUEST.ASM T560/T600 - UI overlay zones\n"
        "  THQUEST.ASM T700/T800/T900 - gameplay\n"
        "  HuC6260/HuC6270 datasheet - VDC/VCE rendering\n"
        "  tqr_v1_phase2_data_formats_H2339.md §7 - tile data format\n"
        "  M12_PRESENTATION_V1_ORIGINAL=0  -> V1_FAITHFUL\n"
        "  M12_PRESENTATION_V20_FILTERED=1 -> V20_FILTERED\n"
        "  M12_PRESENTATION_V21_UPSCALED=2 -> V21_UPSCALED\n"
        "  M12_PRESENTATION_V22_MODERN=3   -> V22_MODERN\n"
        "  Fallback chain: V22->V21->V20->V1\n";
}
