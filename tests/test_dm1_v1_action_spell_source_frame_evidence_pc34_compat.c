#include "dm1_v1_action_spell_source_frame_evidence_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int failures;
#define CHECK(c) do { if (!(c)) { ++failures; fprintf(stderr, "FAIL: %s\n", #c); } } while (0)

static DM1_V1_ActionSpellSourceAssetRuntimeReceiptPc34
assets(unsigned int tick, int action)
{
    DM1_V1_ActionSpellSourceAssetRuntimeReceiptPc34 value;
    memset(&value, 0, sizeof(value));
    value.accepted = 1; value.suppressSyntheticFallback = 1;
    value.presentationKind = action ? DM1_V1_ACTION_HUD_PRESENTATION_ACTION_LOCK_PC34
                                    : DM1_V1_ACTION_HUD_PRESENTATION_SPELL_PROJECTILE_PC34;
    value.originalGraphicId = action ? 10 : 9; value.originalZoneId = action ? 11 : 13;
    value.companionGraphicId = action ? 0 : 11; value.sourceAssetCount = action ? 1 : 2;
    value.sourceCommandCount = action ? 3 : 4;
    value.frameTick = tick; value.sourceTick = tick - 800;
    value.serial = tick - 891; value.commandFingerprint = tick + 0x10u;
    value.orderingFingerprint = tick + 0x20u;
    return value;
}

int
main(void)
{
    DM1_V1_ActionSpellSourceFrameEvidenceStatePc34 state;
    DM1_V1_ActionSpellSourceFrameEvidenceReceiptPc34 evidence;
    DM1_V1_ActionSpellSourceAssetRuntimeReceiptPc34 spell = assets(900, 0);
    DM1_V1_ActionSpellSourceAssetRuntimeReceiptPc34 action = assets(901, 1);

    memset(&state, 0, sizeof(state));
    CHECK(dm1_v1_action_spell_source_frame_evidence_apply_pc34(&state, &spell, &evidence));
    CHECK(evidence.accepted && evidence.liveSourceFrameCurrent &&
          !evidence.clearStaleSourceFrame && evidence.originalGraphicId == 9 &&
          evidence.companionGraphicId == 11);
    CHECK(dm1_v1_action_spell_source_frame_evidence_apply_pc34(&state, &action, &evidence));
    CHECK(evidence.accepted && evidence.clearStaleSourceFrame && evidence.revokeStaleSourceFrame &&
          evidence.staleOriginalGraphicId == 9 && evidence.staleOriginalZoneId == 13 &&
          evidence.originalGraphicId == 10);
    CHECK(dm1_v1_action_spell_source_frame_evidence_apply_pc34(&state, &action, &evidence));
    CHECK(evidence.accepted && evidence.alreadyCurrent && !evidence.clearStaleSourceFrame);

    CHECK(!dm1_v1_action_spell_source_frame_evidence_apply_pc34(&state, &spell, &evidence));
    action.originalZoneId = 13;
    CHECK(!dm1_v1_action_spell_source_frame_evidence_apply_pc34(&state, &action, &evidence));
    action.originalZoneId = 11;
    action.suppressSyntheticFallback = 0;
    CHECK(!dm1_v1_action_spell_source_frame_evidence_apply_pc34(&state, &action, &evidence));

    printf("%s\n", failures ? "failed" : "ok: action/spell source frame evidence");
    return failures ? 1 : 0;
}
