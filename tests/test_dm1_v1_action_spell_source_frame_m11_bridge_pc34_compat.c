#include "dm1_v1_action_spell_source_frame_m11_bridge_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int failures;
#define CHECK(c) do { if (!(c)) { ++failures; fprintf(stderr, "FAIL: %s\n", #c); } } while (0)

static void
set_evidence(DM1_V1_ActionSpellSourceFrameEvidenceReceiptPc34 *evidence, int action)
{
    memset(evidence, 0, sizeof(*evidence));
    evidence->accepted = 1; evidence->liveSourceFrameCurrent = 1;
    evidence->suppressSyntheticFallback = 1; evidence->frameTick = 901;
    evidence->sourceTick = 101; evidence->serial = 10;
    evidence->commandFingerprint = 0x41u; evidence->orderingFingerprint = 0x42u;
    evidence->presentationKind = action ? DM1_V1_ACTION_HUD_PRESENTATION_ACTION_LOCK_PC34
                                        : DM1_V1_ACTION_HUD_PRESENTATION_SPELL_PROJECTILE_PC34;
    evidence->originalGraphicId = action ? 10 : 9;
    evidence->originalZoneId = action ? 11 : 13;
    evidence->companionGraphicId = action ? 0 : 11;
    evidence->sourceAssetCount = action ? 1 : 2;
    evidence->sourceCommandCount = action ? 3 : 4;
}

int
main(void)
{
    DM1_V1_ActionSpellSourceFrameEvidenceReceiptPc34 evidence;
    DM1_V1_ActionSpellSourceFrameM11BridgeReceiptPc34 bridge;

    set_evidence(&evidence, 1);
    evidence.clearStaleSourceFrame = evidence.revokeStaleSourceFrame = 1;
    evidence.staleOriginalGraphicId = 9; evidence.staleOriginalZoneId = 13;
    evidence.staleCompanionGraphicId = 11;
    CHECK(dm1_v1_action_spell_source_frame_m11_bridge_build_pc34(&evidence, &bridge));
    CHECK(bridge.accepted && bridge.m11SourceFrameReady && bridge.originalGraphicId == 10 &&
          bridge.clearStaleSourceFrame && bridge.revokeStaleSourceFrame &&
          bridge.staleOriginalGraphicId == 9 && bridge.staleCompanionGraphicId == 11);

    set_evidence(&evidence, 0);
    CHECK(dm1_v1_action_spell_source_frame_m11_bridge_build_pc34(&evidence, &bridge));
    CHECK(bridge.originalGraphicId == 9 && bridge.originalZoneId == 13 &&
          bridge.companionGraphicId == 11 && bridge.sourceAssetCount == 2);

    evidence.clearStaleSourceFrame = 1;
    CHECK(!dm1_v1_action_spell_source_frame_m11_bridge_build_pc34(&evidence, &bridge));
    evidence.clearStaleSourceFrame = 0;
    evidence.originalZoneId = 11;
    CHECK(!dm1_v1_action_spell_source_frame_m11_bridge_build_pc34(&evidence, &bridge));
    evidence.originalZoneId = 13;
    evidence.liveSourceFrameCurrent = 0;
    CHECK(!dm1_v1_action_spell_source_frame_m11_bridge_build_pc34(&evidence, &bridge));

    printf("%s\n", failures ? "failed" : "ok: action/spell source frame M11 bridge");
    return failures ? 1 : 0;
}
