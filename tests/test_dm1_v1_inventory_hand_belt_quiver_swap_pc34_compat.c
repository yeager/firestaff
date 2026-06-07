#include "dm1/dm1_v1_inventory_hand_belt_quiver_swap_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_assertions;
static int g_passes;

static int expect_int(const char* label, int got, int want,
                      const char* anchor)
{
    ++g_assertions;
    if (!anchor || anchor[0] == '\0') {
        printf("FAIL %s missing ReDMCSB anchor\n", label);
        return 0;
    }
    if (got != want) {
        printf("FAIL %s got=%d want=%d anchor=%s\n", label, got, want,
               anchor);
        return 0;
    }
    ++g_passes;
    return 1;
}

static int expect_contains(const char* label, const char* got,
                           const char* want, const char* anchor)
{
    ++g_assertions;
    if (!anchor || anchor[0] == '\0') {
        printf("FAIL %s missing ReDMCSB anchor\n", label);
        return 0;
    }
    if (!got || !want || !strstr(got, want)) {
        printf("FAIL %s missing '%s' anchor=%s\n", label,
               want ? want : "(null)", anchor);
        return 0;
    }
    ++g_passes;
    return 1;
}

static int test_spec_and_evidence(void)
{
    const DM1_V1_InventoryHandBeltQuiverSwapSpecPc34* spec =
        dm1_v1_inventory_hand_belt_quiver_swap_spec_pc34();
    const char* evidence =
        dm1_v1_inventory_hand_belt_quiver_swap_evidence_pc34();
    const char* f0302 = "ReDMCSB CHAMPION.C F0302 lines 684-710";
    const char* data = "ReDMCSB DATA.C G0038_ai_Graphic562_SlotMasks lines 1050-1087";
    int ok = 1;

    ok &= expect_int("spec exists", spec != 0, 1, f0302);
    ok &= expect_int("contract only", spec->contractOnly, 1, f0302);
    ok &= expect_int("pouch1 pc34 slot", spec->pouch1Pc34Slot,
                     DM1_PC34_SLOT_POUCH_1, data);
    ok &= expect_int("pouch2 pc34 slot", spec->pouch2Pc34Slot,
                     DM1_PC34_SLOT_POUCH_2, data);
    ok &= expect_int("quiver line1 pc34 slot", spec->quiverLine1Pc34Slot,
                     DM1_PC34_SLOT_QUIVER_LINE1_1, data);
    ok &= expect_int("quiver line2 first pc34 slot",
                     spec->quiverLine2FirstPc34Slot,
                     DM1_PC34_SLOT_QUIVER_LINE2_1, data);
    ok &= expect_int("quiver line1 second pc34 slot",
                     spec->quiverLine1SecondPc34Slot,
                     DM1_PC34_SLOT_QUIVER_LINE1_2, data);
    ok &= expect_int("quiver line2 second pc34 slot",
                     spec->quiverLine2SecondPc34Slot,
                     DM1_PC34_SLOT_QUIVER_LINE2_2, data);
    ok &= expect_int("backpack last pc34 slot", spec->backpackLastPc34Slot,
                     DM1_PC34_SLOT_BACKPACK_LINE1_9, data);
    ok &= expect_contains("spec f0302 anchor", spec->f0302Anchor,
                          "F0302", f0302);
    ok &= expect_contains("spec f0297/f0298 anchor", spec->f0297F0298Anchor,
                          "F0297/F0298",
                          "ReDMCSB CHAMPION.C F0297/F0298 lines 243-298");
    ok &= expect_contains("spec f0300/f0301 anchor", spec->f0300F0301Anchor,
                          "F0300/F0301",
                          "ReDMCSB CHAMPION.C F0300/F0301 lines 511-615");
    ok &= expect_contains("spec data anchor", spec->dataSlotMaskAnchor,
                          "1050-1087", data);
    ok &= expect_contains("scope contract marker", spec->scope,
                          "contract_only=1", f0302);
    ok &= expect_contains("scope no real asset claim", spec->scope,
                          "no real-asset", f0302);
    ok &= expect_contains("evidence slot masks", evidence,
                          "1050-1087", data);
    ok &= expect_contains("evidence swap route", evidence,
                          "684-710", f0302);
    return ok;
}

static int test_accepted_case(
    const char* name,
    const DM1_V1_InventoryHandBeltQuiverSwapCasePc34* row,
    int wantPc34Slot,
    int wantStorageSlot,
    int wantMask,
    const char* anchor)
{
    int ok = 1;
    char label[96];

    snprintf(label, sizeof(label), "%s pc34 slot", name);
    ok &= expect_int(label, row->pc34Slot, wantPc34Slot, anchor);
    snprintf(label, sizeof(label), "%s storage slot", name);
    ok &= expect_int(label, row->expectedStorageSlot, wantStorageSlot, anchor);
    snprintf(label, sizeof(label), "%s slot mask", name);
    ok &= expect_int(label, row->expectedSlotMask, wantMask, anchor);
    snprintf(label, sizeof(label), "%s load before", name);
    ok &= expect_int(label, row->loadBefore, 3,
                     "ReDMCSB CHAMPION.C F0300/F0301 lines 582-615");
    snprintf(label, sizeof(label), "%s accepted click", name);
    ok &= expect_int(label, row->acceptedClick, 1,
                     "ReDMCSB CHAMPION.C F0302 lines 697-710");
    snprintf(label, sizeof(label), "%s compatible item stored", name);
    ok &= expect_int(label, row->slotItemAfterAccepted,
                     row->compatibleItemType,
                     "ReDMCSB CHAMPION.C F0301 lines 606-615");
    snprintf(label, sizeof(label), "%s old slot item moves to mouse", name);
    ok &= expect_int(label, row->mouseItemAfterAccepted,
                     DM1_V1_IHBQS_EXISTING_SLOT_ITEM,
                     "ReDMCSB CHAMPION.C F0300/F0297 lines 511-518,243-267");
    snprintf(label, sizeof(label), "%s load after", name);
    ok &= expect_int(label, row->loadAfterAccepted, 5,
                     "ReDMCSB CHAMPION.C F0300/F0301 lines 582-615");
    return ok;
}

static int test_rejected_case(
    const char* name,
    const DM1_V1_InventoryHandBeltQuiverSwapCasePc34* row,
    const char* anchor)
{
    int ok = 1;
    char label[96];

    snprintf(label, sizeof(label), "%s incompatible rejected", name);
    ok &= expect_int(label, row->rejectedClick, 0, anchor);
    snprintf(label, sizeof(label), "%s rejected keeps slot", name);
    ok &= expect_int(label, row->slotItemAfterRejected,
                     row->slotItemBefore, anchor);
    snprintf(label, sizeof(label), "%s rejected keeps mouse", name);
    ok &= expect_int(label, row->mouseItemAfterRejected,
                     DM1_V1_IHBQS_HEAD_ONLY_ITEM, anchor);
    snprintf(label, sizeof(label), "%s rejected keeps load", name);
    ok &= expect_int(label, row->loadAfterRejected, 3, anchor);
    return ok;
}

static int test_probe(void)
{
    DM1_V1_InventoryHandBeltQuiverSwapProbePc34 probe;
    const char* f0302 = "ReDMCSB CHAMPION.C F0302 lines 684-710";
    const char* data = "ReDMCSB DATA.C G0038_ai_Graphic562_SlotMasks lines 1050-1087";
    int ok = 1;

    ok &= expect_int("probe builds",
                     dm1_v1_inventory_hand_belt_quiver_swap_probe_pc34(
                         &probe),
                     1, f0302);
    ok &= expect_int("probe contract only", probe.contractOnly, 1, f0302);

    ok &= test_accepted_case("pouch1", &probe.pouch1,
                             DM1_PC34_SLOT_POUCH_1, DM1_SLOT_POUCH1,
                             DM1_PC34_ALLOWED_POUCH, data);
    ok &= test_accepted_case("pouch2", &probe.pouch2,
                             DM1_PC34_SLOT_POUCH_2, DM1_SLOT_POUCH2,
                             DM1_PC34_ALLOWED_POUCH, data);
    ok &= test_accepted_case("quiver line1", &probe.quiverLine1,
                             DM1_PC34_SLOT_QUIVER_LINE1_1, DM1_SLOT_QUIVER1,
                             DM1_PC34_ALLOWED_QUIVER_LINE1, data);
    ok &= test_accepted_case("quiver line2 first", &probe.quiverLine2First,
                             DM1_PC34_SLOT_QUIVER_LINE2_1, DM1_SLOT_QUIVER2,
                             DM1_PC34_ALLOWED_QUIVER_LINE2, data);
    ok &= test_accepted_case("quiver line1 second", &probe.quiverLine1Second,
                             DM1_PC34_SLOT_QUIVER_LINE1_2, DM1_SLOT_QUIVER3,
                             DM1_PC34_ALLOWED_QUIVER_LINE2, data);
    ok &= test_accepted_case("quiver line2 second", &probe.quiverLine2Second,
                             DM1_PC34_SLOT_QUIVER_LINE2_2, DM1_SLOT_QUIVER4,
                             DM1_PC34_ALLOWED_QUIVER_LINE2, data);
    ok &= test_accepted_case("backpack line1 nine", &probe.backpackLast,
                             DM1_PC34_SLOT_BACKPACK_LINE1_9,
                             DM1_SLOT_BACKPACK17,
                             DM1_PC34_ALLOWED_ANY_SLOT, data);

    ok &= test_rejected_case("pouch1", &probe.pouch1,
                             "ReDMCSB CHAMPION.C F0302 lines 697-699");
    ok &= test_rejected_case("pouch2", &probe.pouch2,
                             "ReDMCSB CHAMPION.C F0302 lines 697-699");
    ok &= test_rejected_case("quiver line1", &probe.quiverLine1,
                             "ReDMCSB CHAMPION.C F0302 lines 697-699");
    ok &= test_rejected_case("quiver line2 first", &probe.quiverLine2First,
                             "ReDMCSB CHAMPION.C F0302 lines 697-699");
    ok &= test_rejected_case("quiver line2 second", &probe.quiverLine2Second,
                             "ReDMCSB CHAMPION.C F0302 lines 697-699");

    ok &= expect_int("quiver line1 second rejects line1-only object",
                     probe.quiverLine1Second.rejectedClick, 0,
                     "ReDMCSB DATA.C line 1058 uses MASK0x0080 for Quiver Line1 2");
    ok &= expect_int("quiver line1 second rejection keeps slot",
                     probe.quiverLine1Second.slotItemAfterRejected,
                     probe.quiverLine1Second.slotItemBefore,
                     "ReDMCSB CHAMPION.C F0302 lines 697-699");
    ok &= expect_int("backpack accepts head-only because mask is any",
                     probe.backpackAcceptsHeadOnly, 1,
                     "ReDMCSB DATA.C lines 1063-1079");
    ok &= expect_int("backpack rejected fixture actually swaps",
                     probe.backpackLast.rejectedClick, 1,
                     "ReDMCSB DATA.C lines 1063-1079");
    ok &= expect_int("backpack stores head-only item",
                     probe.backpackLast.slotItemAfterRejected,
                     DM1_V1_IHBQS_HEAD_ONLY_ITEM,
                     "ReDMCSB CHAMPION.C F0302 lines 697-710");
    ok &= expect_int("pouch rejects head-only summary",
                     probe.pouchRejectsHeadOnly, 1,
                     "ReDMCSB DATA.C lines 1056,1061");
    ok &= expect_int("quiver rejects head-only summary",
                     probe.quiverRejectsHeadOnly, 1,
                     "ReDMCSB DATA.C lines 1057-1062");
    ok &= expect_int("line1 rejects line2-only summary",
                     probe.quiverLine1RejectsLine2Only, 1,
                     "ReDMCSB DATA.C line 1062");
    ok &= expect_int("line1 second uses line2 mask summary",
                     probe.quiverLine1SecondUsesLine2Mask, 1,
                     "ReDMCSB DATA.C line 1058");
    return ok;
}

int main(void)
{
    int ok = 1;

    ok &= test_spec_and_evidence();
    ok &= test_probe();
    if (!ok) {
        printf("FAIL dm1_v1_inventory_hand_belt_quiver_swap_pc34_compat "
               "%d/%d assertions\n", g_passes, g_assertions);
        return 1;
    }
    printf("PASS dm1_v1_inventory_hand_belt_quiver_swap_pc34_compat "
           "%d/%d assertions\n", g_passes, g_assertions);
    return 0;
}
