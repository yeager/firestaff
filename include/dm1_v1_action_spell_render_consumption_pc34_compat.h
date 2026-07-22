#ifndef FIRESTAFF_DM1_V1_ACTION_SPELL_RENDER_CONSUMPTION_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_ACTION_SPELL_RENDER_CONSUMPTION_PC34_COMPAT_H

#include "dm1_v1_action_spell_final_hud_paint_frame_bridge_pc34_compat.h"

/*
 * Source-owned proof that a final action/spell frame is safe for the host
 * renderer to consume. It is an admission gate, never an M11 implementation.
 */
typedef struct {
    int accepted;
    int renderReadyForHost;
    int clearCount;
    int sourceGraphicId;
    int sourceZoneId;
    int suppressSyntheticFallback;
    DM1_V1_ActionSpellHudPaintRectPc34 renderRect;
    unsigned int frameTick;
    unsigned int sourceTick;
    unsigned int serial;
    unsigned int commandFingerprint;
    unsigned int orderingFingerprint;
    unsigned int lifecycleGeneration;
} DM1_V1_ActionSpellRenderConsumptionReceiptPc34;

/*
 * Validates the full original clear-before-render command ordering and exact
 * PC34 graphic/zone geometry before a renderer may consume the frame.
 */
int dm1_v1_action_spell_render_consumption_build_pc34(
    const DM1_V1_ActionSpellFinalHudPaintFrameBridgeReceiptPc34 *bridge,
    DM1_V1_ActionSpellRenderConsumptionReceiptPc34 *outReceipt);

#endif
