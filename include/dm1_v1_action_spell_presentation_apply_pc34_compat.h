#ifndef FIRESTAFF_DM1_V1_ACTION_SPELL_PRESENTATION_APPLY_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_ACTION_SPELL_PRESENTATION_APPLY_PC34_COMPAT_H

#include "dm1_v1_action_spell_execution_receipt_pc34_compat.h"

/*
 * DM1-owned per-frame state following ReDMCSB's F0407/F0412 redraw handoff.
 * It retains admitted source commands only.  No pixels, labels, fallback
 * colors, or font synthesis exist at this boundary.
 */
typedef struct {
    unsigned int frameTick;
    int frameOpen;
    int hasPresentation;
    int presentationKind;
    int championIndex;
    int actionIndex;
    int spellKind;
    int spellPowerOrdinal;
    unsigned int sourceTick;
    unsigned int serial;
    unsigned int commandFingerprint;
    int commandCount;
    DM1_V1_ActionSpellRenderCommandPc34
        commands[DM1_V1_ACTION_SPELL_RENDER_COMMAND_MAX_PC34];
} DM1_V1_ActionSpellPresentationFrameStatePc34;

typedef struct {
    int accepted;
    int applied;
    int alreadyApplied;
    int presentationKind;
    int commandCount;
    unsigned int frameTick;
    unsigned int sourceTick;
    unsigned int serial;
    unsigned int commandFingerprint;
} DM1_V1_ActionSpellPresentationApplyReceiptPc34;

void dm1_v1_action_spell_presentation_frame_begin_pc34(
    DM1_V1_ActionSpellPresentationFrameStatePc34 *state,
    unsigned int frameTick);

/*
 * Rechecks the current source-owned surfaces at time of application and then
 * copies the exact admitted commands into the DM1 frame state.
 */
int dm1_v1_action_spell_presentation_apply_pc34(
    DM1_V1_ActionSpellPresentationFrameStatePc34 *state,
    const DM1_V1_ActionSpellExecutionReceiptPc34 *execution,
    const DM1_V1_ActionSpellRenderCommandReceiptPc34 *commands,
    const DM1_V1_ActionSpellHudMaterialSetPc34 *materials,
    DM1_V1_ActionSpellPresentationApplyReceiptPc34 *outReceipt);

#endif
