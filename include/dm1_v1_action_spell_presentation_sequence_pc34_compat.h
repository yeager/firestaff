#ifndef FIRESTAFF_DM1_V1_ACTION_SPELL_PRESENTATION_SEQUENCE_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_ACTION_SPELL_PRESENTATION_SEQUENCE_PC34_COMPAT_H

#include "dm1_v1_live_action_effects_pc34_compat.h"

/*
 * ReDMCSB source-lock: ACTIDRAW.C F0387/F0386 and CASTER.C F0394/F0396/
 * F0397/F0398.  This module owns the source-material sequence only.  A host
 * renderer may consume a completed receipt, but cannot add labels, substitute
 * a font, or invent a missing background/rune row.
 */
enum {
    DM1_V1_ACTION_SPELL_SEQUENCE_MAX_STEPS_PC34 = 12,
    DM1_V1_ACTION_SPELL_SEQUENCE_STEP_BLIT_PC34 = 1,
    DM1_V1_ACTION_SPELL_SEQUENCE_STEP_FONT_ZONE_PC34 = 2
};

typedef struct {
    int kind;
    int graphicId;
    int zoneId;
    int zoneCount;
    int sourceX;
    int sourceY;
    int sourceW;
    int sourceH;
} DM1_V1_ActionSpellPresentationSequenceStepPc34;

typedef struct {
    int accepted;
    int drawable;
    int presentationKind;
    int stepCount;
    int sourceSurfaceCount;
    DM1_V1_ActionSpellPresentationSequenceStepPc34
        steps[DM1_V1_ACTION_SPELL_SEQUENCE_MAX_STEPS_PC34];
} DM1_V1_ActionSpellPresentationSequenceReceiptPc34;

/*
 * Builds the complete row/zone ordering after the live-effect material
 * admission has already confirmed the exact GRAPHICS.DAT surfaces.
 */
int dm1_v1_action_spell_presentation_sequence_build_pc34(
    const DM1_V1_ActionSpellHudPresentationReceiptPc34 *presentation,
    const DM1_V1_ActionSpellHudMaterialReceiptPc34 *materials,
    int actionMenuRowCount,
    DM1_V1_ActionSpellPresentationSequenceReceiptPc34 *outReceipt);

#endif
