#ifndef FIRESTAFF_DM1_V1_ACTION_SPELL_FINAL_HUD_PAINT_LIFECYCLE_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_ACTION_SPELL_FINAL_HUD_PAINT_LIFECYCLE_PC34_COMPAT_H

#include "dm1_v1_action_spell_final_hud_paint_pc34_compat.h"

/*
 * Per-HUD paint state for the original F0407/F0412 clear-before-render path.
 * It stores only previous source-owned paint identity and physical clear box.
 */
typedef struct {
    int active;
    int presentationKind;
    DM1_V1_ActionSpellHudPaintRectPc34 previousClearRect;
    unsigned int frameTick;
    unsigned int sourceTick;
    unsigned int serial;
    unsigned int commandFingerprint;
    unsigned int orderingFingerprint;
    unsigned int lifecycleGeneration;
} DM1_V1_ActionSpellFinalHudPaintLifecycleStatePc34;

typedef struct {
    int accepted;
    int clearPreviousPaint;
    int clearCurrentPaint;
    int renderCurrentPaint;
    int alreadyCurrent;
    int suppressSyntheticFallback;
    DM1_V1_ActionSpellHudPaintRectPc34 previousClearRect;
    DM1_V1_ActionSpellHudPaintRectPc34 currentClearRect;
    DM1_V1_ActionSpellHudPaintRectPc34 renderRect;
    int sourceGraphicId;
    int sourceZoneId;
    int clearColor;
    unsigned int frameTick;
    unsigned int sourceTick;
    unsigned int serial;
    unsigned int commandFingerprint;
    unsigned int orderingFingerprint;
    unsigned int lifecycleGeneration;
} DM1_V1_ActionSpellFinalHudPaintLifecycleReceiptPc34;

/*
 * Fences the prior original clear before a later-tick original render. Exact
 * replay of the current paint is idempotent; stale or divergent same-tick
 * paint is rejected.
 */
int dm1_v1_action_spell_final_hud_paint_lifecycle_apply_pc34(
    DM1_V1_ActionSpellFinalHudPaintLifecycleStatePc34 *state,
    const DM1_V1_ActionSpellFinalHudPaintReceiptPc34 *paint,
    DM1_V1_ActionSpellFinalHudPaintLifecycleReceiptPc34 *outReceipt);

#endif
