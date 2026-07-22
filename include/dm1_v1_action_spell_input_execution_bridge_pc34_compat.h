#ifndef FIRESTAFF_DM1_V1_ACTION_SPELL_INPUT_EXECUTION_BRIDGE_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_ACTION_SPELL_INPUT_EXECUTION_BRIDGE_PC34_COMPAT_H

#include "dm1_v1_action_spell_input_command_admission_pc34_compat.h"
#include "dm1_v1_action_spell_execution_receipt_pc34_compat.h"

/*
 * ReDMCSB COMMAND.C -> F0407/F0412 bridge.  This receipt binds source input
 * admission to an exact runtime execution batch.  It does not invent command
 * text, material, or a host-side alternative route.
 */
typedef struct {
    int accepted;
    int inputKind;
    int presentationKind;
    int championIndex;
    int actionIndex;
    int inputZoneId;
    int runeValue;
    int commandCount;
    unsigned int sourceTick;
    unsigned int serial;
    unsigned int commandFingerprint;
    unsigned int bridgeFingerprint;
} DM1_V1_ActionSpellInputExecutionBridgeReceiptPc34;

/* Only admits an execution whose source facts are exactly owned by input. */
int dm1_v1_action_spell_input_execution_bridge_build_pc34(
    const DM1_V1_ActionSpellInputCommandAdmissionReceiptPc34 *input,
    const DM1_V1_LiveActionEffectPc34 *effect,
    const DM1_V1_ActionSpellHudPresentationReceiptPc34 *presentation,
    const DM1_V1_ActionSpellRenderCommandReceiptPc34 *commands,
    const DM1_V1_ActionSpellExecutionReceiptPc34 *execution,
    DM1_V1_ActionSpellInputExecutionBridgeReceiptPc34 *outReceipt);

#endif
