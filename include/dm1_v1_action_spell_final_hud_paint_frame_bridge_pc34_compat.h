#ifndef FIRESTAFF_DM1_V1_ACTION_SPELL_FINAL_HUD_PAINT_FRAME_BRIDGE_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_ACTION_SPELL_FINAL_HUD_PAINT_FRAME_BRIDGE_PC34_COMPAT_H

#include "dm1_v1_action_spell_final_hud_paint_lifecycle_pc34_compat.h"

enum {
    DM1_V1_ACTION_SPELL_FINAL_HUD_FRAME_COMMAND_CLEAR_PC34 = 1,
    DM1_V1_ACTION_SPELL_FINAL_HUD_FRAME_COMMAND_RENDER_PC34 = 2,
    DM1_V1_ACTION_SPELL_FINAL_HUD_FRAME_COMMAND_MAX_PC34 = 3
};

/* One already-authorized original clear or blit operation for the HUD frame. */
typedef struct {
    int kind;
    DM1_V1_ActionSpellHudPaintRectPc34 rect;
    int clearColor;
    int sourceGraphicId;
    int sourceZoneId;
} DM1_V1_ActionSpellFinalHudFrameCommandPc34;

/*
 * The bridge publishes only lifecycle-approved original paint operations. It
 * contains no generated visual material and cannot publish a fallback.
 */
typedef struct {
    int accepted;
    int commandCount;
    int suppressSyntheticFallback;
    DM1_V1_ActionSpellFinalHudFrameCommandPc34
        commands[DM1_V1_ACTION_SPELL_FINAL_HUD_FRAME_COMMAND_MAX_PC34];
    unsigned int frameTick;
    unsigned int sourceTick;
    unsigned int serial;
    unsigned int commandFingerprint;
    unsigned int orderingFingerprint;
    unsigned int lifecycleGeneration;
} DM1_V1_ActionSpellFinalHudPaintFrameBridgeReceiptPc34;

/*
 * Converts a matching lifecycle transition to ordered frame commands: prior
 * clear, current clear, then current original render where those steps exist.
 */
int dm1_v1_action_spell_final_hud_paint_frame_bridge_build_pc34(
    const DM1_V1_ActionSpellFinalHudPaintReceiptPc34 *paint,
    const DM1_V1_ActionSpellFinalHudPaintLifecycleReceiptPc34 *lifecycle,
    DM1_V1_ActionSpellFinalHudPaintFrameBridgeReceiptPc34 *outReceipt);

#endif
