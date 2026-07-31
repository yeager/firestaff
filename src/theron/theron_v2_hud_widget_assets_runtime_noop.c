/*
 * Production seam for Theron V2 HUD asset classification.
 *
 * The manifest parser remains available to focused fixture tests, but no
 * Track 02 HUD widget bank has been decoded yet.  Production must therefore
 * report an unavailable gate rather than expose procedural HUD pixels.
 */

#include "theron_v2_hud_widget_assets_pc34.h"

Theron_V2_HudWidgetGate theron_v2_hud_widget_assets_gate(void)
{
    return THERON_V2_HUD_WIDGET_GATE_NO_MANIFEST;
}

const char *theron_v2_hud_widget_assets_source_evidence(void)
{
    return "Track 02 HUD widget records not decoded; production HUD route blocked";
}
