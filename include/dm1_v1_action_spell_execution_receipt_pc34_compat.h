#ifndef FIRESTAFF_DM1_V1_ACTION_SPELL_EXECUTION_RECEIPT_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_ACTION_SPELL_EXECUTION_RECEIPT_PC34_COMPAT_H

#include "dm1_v1_action_spell_render_command_admission_pc34_compat.h"

/*
 * ReDMCSB MENU.C F0407/F0412 and their ACTIDRAW.C/CASTER.C redraw handoffs.
 * A receipt is an immutable DM1 runtime-to-presentation boundary, not a host
 * draw call.  It carries no text and cannot authorize fallback material.
 */
typedef struct {
    int accepted;
    int readyForPresentation;
    int sourceEffectKind;
    int presentationKind;
    int championIndex;
    int actionIndex;
    int spellKind;
    int spellPowerOrdinal;
    int commandCount;
    int sourceOwnedCommandCount;
    unsigned int sourceTick;
    unsigned int serial;
    unsigned int commandFingerprint;
} DM1_V1_ActionSpellExecutionReceiptPc34;

/*
 * Rechecks one live effect against its derived presentation and the already
 * admitted command batch.  Effects without an exact F0407/F0412 source route
 * are rejected before any backend can inspect the commands.
 */
int dm1_v1_action_spell_execution_receipt_build_pc34(
    const DM1_V1_LiveActionEffectPc34 *effect,
    const DM1_V1_ActionSpellHudPresentationReceiptPc34 *presentation,
    const DM1_V1_ActionSpellRenderCommandReceiptPc34 *commands,
    DM1_V1_ActionSpellExecutionReceiptPc34 *outReceipt);

#endif
