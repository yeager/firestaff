/*
 * ui_scale_m11 — global UI scale state.
 *
 * Default 100% leaves V1 launches bit-identical. The font engine
 * (M11_Font_DrawString) already supports a per-call integer scale; this
 * module is the single source of truth for the value HUD/menu code
 * should pass.
 */

#include "ui_scale_m11.h"

static int g_ui_scale_percent = 100;

int M11_UIScale_NormalizePercent(int percent) {
    if (percent <= 100) return 100;
    if (percent <= 150) return 150;
    return 200;
}

void M11_UIScale_SetPercent(int percent) {
    g_ui_scale_percent = M11_UIScale_NormalizePercent(percent);
}

int M11_UIScale_GetPercent(void) {
    return g_ui_scale_percent;
}

int M11_UIScale_PercentToFontScale(int percent) {
    int p = M11_UIScale_NormalizePercent(percent);
    if (p == 100) return 1;
    if (p == 150) return 2;
    return 3;
}

int M11_UIScale_GetFontScale(void) {
    return M11_UIScale_PercentToFontScale(g_ui_scale_percent);
}

int M11_UIScale_Apply(int value) {
    return (value * g_ui_scale_percent + 50) / 100;
}
