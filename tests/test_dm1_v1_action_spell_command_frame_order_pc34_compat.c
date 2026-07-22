#include "dm1_v1_action_spell_command_frame_order_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int failures;
#define CHECK(c) do { if (!(c)) { ++failures; fprintf(stderr, "FAIL: %s\n", #c); } } while (0)

int main(void)
{
    static const unsigned char background[87 * 25] = { 1 };
    static const unsigned char lines[14 * 39] = { 1 };
    static const unsigned char font[1] = { 1 };
    DM1_V1_ActionSpellHudSurfacePc34 surfaces[] = {
        { 9, 87, 25, 87 * 25, background, 1 },
        { 11, 14, 39, 14 * 39, lines, 1 },
        { 695, 1, 1, 1, font, 1 }
    };
    DM1_V1_ActionSpellHudMaterialSetPc34 materials = { surfaces, 3, 0 };
    DM1_V1_ActionSpellPresentationFrameStatePc34 state;
    DM1_V1_ActionSpellPresentationApplyReceiptPc34 apply;
    DM1_V1_ActionSpellCommandFrameOrderReceiptPc34 receipt;

    memset(&state, 0, sizeof(state));
    memset(&apply, 0, sizeof(apply));
    state.frameOpen = 1;
    state.hasPresentation = 1;
    state.frameTick = 75;
    state.sourceTick = 74;
    state.serial = 4;
    state.presentationKind = DM1_V1_ACTION_HUD_PRESENTATION_SPELL_PROJECTILE_PC34;
    state.commandCount = 4;
    state.commandFingerprint = 0x1a2b3c4du;
    state.commands[0] = (DM1_V1_ActionSpellRenderCommandPc34) { 1, 9, 13, 1, 0, 0, 87, 25, 0 };
    state.commands[1] = (DM1_V1_ActionSpellRenderCommandPc34) { 1, 11, 245, 6, 0, 13, 14, 12, 1 };
    state.commands[2] = (DM1_V1_ActionSpellRenderCommandPc34) { 1, 11, 261, 4, 0, 26, 14, 12, 1 };
    state.commands[3] = (DM1_V1_ActionSpellRenderCommandPc34) { 2, 695, 221, 1, 0, 0, 0, 0, 2 };
    apply.accepted = 1;
    apply.applied = 1;
    apply.presentationKind = state.presentationKind;
    apply.commandCount = state.commandCount;
    apply.frameTick = state.frameTick;
    apply.sourceTick = state.sourceTick;
    apply.serial = state.serial;
    apply.commandFingerprint = state.commandFingerprint;

    CHECK(dm1_v1_action_spell_command_frame_order_build_pc34(
              &state, &apply, &materials, &receipt));
    CHECK(receipt.accepted && receipt.readyForPresentation &&
          receipt.commandCount == 4 && receipt.orderingFingerprint != 0);
    CHECK(receipt.orderedSurfaceIndices[0] == 0 &&
          receipt.orderedSurfaceIndices[1] == 1 &&
          receipt.orderedSurfaceIndices[3] == 2);
    surfaces[1].sourceOwned = 0;
    CHECK(!dm1_v1_action_spell_command_frame_order_build_pc34(
              &state, &apply, &materials, &receipt));
    surfaces[1].sourceOwned = 1;
    state.commands[1].sourceY = 26;
    CHECK(!dm1_v1_action_spell_command_frame_order_build_pc34(
              &state, &apply, &materials, &receipt));
    state.commands[1].sourceY = 13;
    apply.commandFingerprint = 0;
    CHECK(!dm1_v1_action_spell_command_frame_order_build_pc34(
              &state, &apply, &materials, &receipt));

    printf("%s\n", failures ? "failed" : "ok: action/spell command frame order");
    return failures ? 1 : 0;
}
