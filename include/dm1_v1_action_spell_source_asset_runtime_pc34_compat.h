#ifndef FIRESTAFF_DM1_V1_ACTION_SPELL_SOURCE_ASSET_RUNTIME_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_ACTION_SPELL_SOURCE_ASSET_RUNTIME_PC34_COMPAT_H

#include "dm1_v1_action_spell_runtime_frame_admission_pc34_compat.h"
#include "dm1_v1_action_spell_command_frame_order_pc34_compat.h"

/*
 * ReDMCSB F0387/F0394 source asset proof carried through the live command and
 * presentation-frame routes. This receipt references no synthetic material.
 */
typedef struct {
    int accepted;
    int presentationKind;
    int originalGraphicId;
    int originalZoneId;
    int companionGraphicId;
    int sourceAssetCount;
    int sourceCommandCount;
    int suppressSyntheticFallback;
    unsigned int frameTick;
    unsigned int sourceTick;
    unsigned int serial;
    unsigned int commandFingerprint;
    unsigned int orderingFingerprint;
} DM1_V1_ActionSpellSourceAssetRuntimeReceiptPc34;

/*
 * Binds original C010/C011 action material or C009/C011/C013 spell material
 * to the admitted live command batch, presentation frame, ordering receipt,
 * and current runtime route. Missing or substitute surfaces fail closed.
 */
int dm1_v1_action_spell_source_asset_runtime_build_pc34(
    const DM1_V1_ActionSpellHudMaterialSetPc34 *materials,
    const DM1_V1_ActionSpellRenderCommandReceiptPc34 *commands,
    const DM1_V1_ActionSpellPresentationFrameStatePc34 *frameState,
    const DM1_V1_ActionSpellCommandFrameOrderReceiptPc34 *order,
    const DM1_V1_ActionSpellRuntimeFrameAdmissionReceiptPc34 *runtime,
    DM1_V1_ActionSpellSourceAssetRuntimeReceiptPc34 *outReceipt);

#endif
