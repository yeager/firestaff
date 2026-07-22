#ifndef FIRESTAFF_DM1_V1_ACTION_SPELL_RESULT_FEEDBACK_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_ACTION_SPELL_RESULT_FEEDBACK_PC34_COMPAT_H

#include "dm1_v1_action_spell_input_execution_bridge_pc34_compat.h"

/* ReDMCSB F0407/F0412 result states.  These are visual-state identities, not
 * host strings, glyphs, colors, or fallback effects. */
enum {
    DM1_V1_ACTION_SPELL_RESULT_ACTION_SUCCESS_PC34 = 1,
    DM1_V1_ACTION_SPELL_RESULT_ACTION_FAILURE_PC34 = 2,
    DM1_V1_ACTION_SPELL_RESULT_SPELL_SUCCESS_PC34 = 3,
    DM1_V1_ACTION_SPELL_RESULT_SPELL_FAILURE_PC34 = 4
};

typedef struct {
    int sourceOwned;
    int resultKind;
    int inputKind;
    unsigned int sourceTick;
    unsigned int serial;
    unsigned int commandFingerprint;
} DM1_V1_ActionSpellSourceResultPc34;

typedef struct {
    int accepted;
    int resultKind;
    int presentationKind;
    int championIndex;
    int inputZoneId;
    int requiresCommandRepaint;
    int suppressSyntheticFallback;
    unsigned int sourceTick;
    unsigned int serial;
    unsigned int commandFingerprint;
} DM1_V1_ActionSpellResultFeedbackReceiptPc34;

/*
 * Publishes one source result visual state only after rechecking the exact
 * command fingerprint that reached execution through the input bridge.
 */
int dm1_v1_action_spell_result_feedback_build_pc34(
    const DM1_V1_ActionSpellSourceResultPc34 *result,
    const DM1_V1_ActionSpellInputExecutionBridgeReceiptPc34 *bridge,
    const DM1_V1_ActionSpellExecutionReceiptPc34 *execution,
    const DM1_V1_ActionSpellRenderCommandReceiptPc34 *commands,
    DM1_V1_ActionSpellResultFeedbackReceiptPc34 *outReceipt);

#endif
