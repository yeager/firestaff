#include "m11_qol_runtime.h"
#include "config_m12.h"

#include <stddef.h>

/* Live QoL state. Defaults match original DM1 behaviour. */
static int g_speedMultiplier   = 100;
static int g_minimapEnabled    = 0;
static int g_minimapSize       = 128;
static int g_minimapCorner     = 0;
static int g_autoMapEnabled    = 1;
static int g_combatLogEnabled  = 0;
static int g_combatLogMaxLines = 200;

static int clamp_speed(int multiplier) {
    if (multiplier <= 50)  return 50;
    if (multiplier <= 100) return 100;
    if (multiplier <= 150) return 150;
    return 200;
}

void M11_QolRuntime_InitFromConfig(const M12_Config* config) {
    if (!config) {
        return;
    }
    g_speedMultiplier   = clamp_speed(config->gameSpeedMultiplier > 0
                                     ? config->gameSpeedMultiplier : 100);
    g_minimapEnabled    = config->minimapEnabled ? 1 : 0;
    g_minimapSize       = (config->minimapSize >= 64 && config->minimapSize <= 256)
                              ? config->minimapSize : 128;
    g_minimapCorner     = (config->minimapCorner >= 0 && config->minimapCorner <= 3)
                              ? config->minimapCorner : 0;
    g_autoMapEnabled    = config->autoMapEnabled ? 1 : 0;
    g_combatLogEnabled  = config->combatLogEnabled ? 1 : 0;
    g_combatLogMaxLines = (config->combatLogMaxLines >= 50 &&
                            config->combatLogMaxLines <= 500)
                              ? config->combatLogMaxLines : 200;
}

int  M11_QolRuntime_GetSpeedMultiplier(void) { return g_speedMultiplier; }
void M11_QolRuntime_SetSpeedMultiplier(int multiplier) {
    g_speedMultiplier = clamp_speed(multiplier);
}
int  M11_QolRuntime_CycleSpeedMultiplier(void) {
    switch (g_speedMultiplier) {
        case 50:  g_speedMultiplier = 100; break;
        case 100: g_speedMultiplier = 150; break;
        case 150: g_speedMultiplier = 200; break;
        default:  g_speedMultiplier = 50;  break;
    }
    return g_speedMultiplier;
}

int  M11_QolRuntime_GetMinimapEnabled(void) { return g_minimapEnabled; }
void M11_QolRuntime_SetMinimapEnabled(int enabled) {
    g_minimapEnabled = enabled ? 1 : 0;
}
int  M11_QolRuntime_ToggleMinimap(void) {
    g_minimapEnabled = !g_minimapEnabled;
    return g_minimapEnabled;
}
int  M11_QolRuntime_GetMinimapSize(void)   { return g_minimapSize; }
int  M11_QolRuntime_GetMinimapCorner(void) { return g_minimapCorner; }

int  M11_QolRuntime_GetAutoMapEnabled(void) { return g_autoMapEnabled; }

int  M11_QolRuntime_GetCombatLogEnabled(void) { return g_combatLogEnabled; }
void M11_QolRuntime_SetCombatLogEnabled(int enabled) {
    g_combatLogEnabled = enabled ? 1 : 0;
}
int  M11_QolRuntime_ToggleCombatLog(void) {
    g_combatLogEnabled = !g_combatLogEnabled;
    return g_combatLogEnabled;
}
int  M11_QolRuntime_GetCombatLogMaxLines(void) { return g_combatLogMaxLines; }
