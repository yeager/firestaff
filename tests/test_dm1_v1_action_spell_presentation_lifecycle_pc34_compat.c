#include "dm1_v1_action_spell_presentation_lifecycle_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int failures;
#define CHECK(c) do { if (!(c)) { ++failures; fprintf(stderr, "FAIL: %s\n", #c); } } while (0)

static void set_frame_order(
    DM1_V1_ActionSpellPresentationFrameStatePc34 *frame,
    DM1_V1_ActionSpellCommandFrameOrderReceiptPc34 *order,
    unsigned int frameTick,
    unsigned int serial)
{
    memset(frame, 0, sizeof(*frame));
    memset(order, 0, sizeof(*order));
    frame->frameOpen = 1;
    frame->hasPresentation = 1;
    frame->frameTick = frameTick;
    frame->sourceTick = frameTick - 1;
    frame->serial = serial;
    frame->presentationKind =
        DM1_V1_ACTION_HUD_PRESENTATION_SPELL_PROJECTILE_PC34;
    frame->commandCount = 4;
    frame->commandFingerprint = 0xabc00000u + serial;
    order->accepted = 1;
    order->readyForPresentation = 1;
    order->presentationKind = frame->presentationKind;
    order->commandCount = frame->commandCount;
    order->frameTick = frame->frameTick;
    order->sourceTick = frame->sourceTick;
    order->serial = frame->serial;
    order->commandFingerprint = frame->commandFingerprint;
}

int main(void)
{
    DM1_V1_ActionSpellPresentationLifecycleStatePc34 lifecycle;
    DM1_V1_ActionSpellPresentationFrameStatePc34 frame;
    DM1_V1_ActionSpellCommandFrameOrderReceiptPc34 order;
    DM1_V1_ActionSpellPresentationLifecycleReceiptPc34 receipt;

    memset(&lifecycle, 0, sizeof(lifecycle));
    dm1_v1_action_spell_presentation_lifecycle_begin_frame_pc34(&lifecycle, 100);
    set_frame_order(&frame, &order, 100, 1);
    CHECK(dm1_v1_action_spell_presentation_lifecycle_apply_pc34(
              &lifecycle, &frame, &order, &receipt));
    CHECK(receipt.repainted && !receipt.clearPrevious &&
          receipt.lifecycleGeneration == 1);
    CHECK(dm1_v1_action_spell_presentation_lifecycle_apply_pc34(
              &lifecycle, &frame, &order, &receipt));
    CHECK(receipt.alreadyCurrent && !receipt.repainted);

    dm1_v1_action_spell_presentation_lifecycle_begin_frame_pc34(&lifecycle, 101);
    set_frame_order(&frame, &order, 101, 2);
    CHECK(dm1_v1_action_spell_presentation_lifecycle_apply_pc34(
              &lifecycle, &frame, &order, &receipt));
    CHECK(receipt.clearPrevious && receipt.repainted &&
          receipt.lifecycleGeneration == 2);

    dm1_v1_action_spell_presentation_lifecycle_begin_frame_pc34(&lifecycle, 102);
    set_frame_order(&frame, &order, 101, 2);
    CHECK(!dm1_v1_action_spell_presentation_lifecycle_apply_pc34(
              &lifecycle, &frame, &order, &receipt));
    set_frame_order(&frame, &order, 102, 3);
    order.commandFingerprint++;
    CHECK(!dm1_v1_action_spell_presentation_lifecycle_apply_pc34(
              &lifecycle, &frame, &order, &receipt));

    printf("%s\n", failures ? "failed" : "ok: action/spell presentation lifecycle");
    return failures ? 1 : 0;
}
