#include "dm1_v1_inventory_slot_placement_pc34_compat.h"

#include <stdio.h>
#include <string.h>

/* DM1 V1 inventory slot placement rules regression test.
 *
 * Pins the six source-locked placement rules (hand / body / pouch /
 * quiver / backpack / container-reject) against the existing
 * m11_inventory_can_equip gate so a future change to either layer
 * breaks the test rather than silently regressing slot routing.
 *
 * ReDMCSB anchors:
 * - CHAMPION.C F0302 lines 684-710 leader-hand/slot swap contract
 * - CHAMPION.C F0302 lines 697-699 AllowedSlots & SlotMasks rejection
 * - DATA.C G0038_ai_Graphic562_SlotMasks lines 1049-1087
 * - DATA.C G0057_ai_Graphic562_SlotDropOrder lines 436-466
 * - CHAMPION.C F0300 line 1546 forced drop priority via G0057
 */

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

static int expect_true(const char* label, int got, const char* anchor)
{
    ++g_assertions;
    if (!anchor || anchor[0] == '\0') {
        printf("FAIL %s missing ReDMCSB anchor\n", label);
        return 0;
    }
    if (!got) {
        printf("FAIL %s expected truthy anchor=%s\n", label, anchor);
        return 0;
    }
    ++g_passes;
    return 1;
}

static int expect_case(const char* prefix,
                       const DM1_V1_InventorySlotPlacementCasePc34* row,
                       int expectedRule,
                       int expectedSlot,
                       const char* anchor)
{
    int ok = 1;
    char label[160];

    snprintf(label, sizeof(label), "%s placement rule", prefix);
    ok &= expect_int(label, row->expectedRule, expectedRule, anchor);
    snprintf(label, sizeof(label), "%s placement slot", prefix);
    ok &= expect_int(label, row->placementResult, expectedSlot, anchor);
    return ok;
}

static int test_spec_and_evidence(void)
{
    const DM1_V1_InventorySlotPlacementSpecPc34* spec =
        dm1_v1_inventory_slot_placement_spec_pc34();
    const char* evidence =
        dm1_v1_inventory_slot_placement_evidence_pc34();
    const char* dataSlotMask =
        "ReDMCSB DATA.C G0038_ai_Graphic562_SlotMasks lines 1049-1087";
    const char* dataDropOrder =
        "ReDMCSB DATA.C G0057_ai_Graphic562_SlotDropOrder lines 436-466";
    const char* f0302 =
        "ReDMCSB CHAMPION.C F0302 lines 684-710";
    const char* f0300 =
        "ReDMCSB CHAMPION.C F0300 line 1546";
    int ok = 1;

    ok &= expect_true("spec exists", spec != 0, dataSlotMask);
    ok &= expect_int("contract only", spec->contractOnly, 1, dataSlotMask);
    ok &= expect_int("ready hand pc34 slot",
                     spec->readyHandPc34Slot, DM1_PC34_SLOT_READY_HAND,
                     dataSlotMask);
    ok &= expect_int("action hand pc34 slot",
                     spec->actionHandPc34Slot, DM1_PC34_SLOT_ACTION_HAND,
                     dataSlotMask);
    ok &= expect_int("head pc34 slot",
                     spec->headPc34Slot, DM1_PC34_SLOT_HEAD, dataSlotMask);
    ok &= expect_int("torso pc34 slot",
                     spec->torsoPc34Slot, DM1_PC34_SLOT_TORSO, dataSlotMask);
    ok &= expect_int("legs pc34 slot",
                     spec->legsPc34Slot, DM1_PC34_SLOT_LEGS, dataSlotMask);
    ok &= expect_int("feet pc34 slot",
                     spec->feetPc34Slot, DM1_PC34_SLOT_FEET, dataSlotMask);
    ok &= expect_int("neck pc34 slot",
                     spec->neckPc34Slot, DM1_PC34_SLOT_NECK, dataSlotMask);
    ok &= expect_int("pouch1 pc34 slot",
                     spec->pouch1Pc34Slot, DM1_PC34_SLOT_POUCH_1,
                     dataSlotMask);
    ok &= expect_int("pouch2 pc34 slot",
                     spec->pouch2Pc34Slot, DM1_PC34_SLOT_POUCH_2,
                     dataSlotMask);
    ok &= expect_int("quiver line1 pc34 slot",
                     spec->quiverLine1Pc34Slot,
                     DM1_PC34_SLOT_QUIVER_LINE1_1, dataSlotMask);
    ok &= expect_int("quiver line2 first pc34 slot",
                     spec->quiverLine2FirstPc34Slot,
                     DM1_PC34_SLOT_QUIVER_LINE2_1, dataSlotMask);
    ok &= expect_int("quiver line1 second pc34 slot",
                     spec->quiverLine1SecondPc34Slot,
                     DM1_PC34_SLOT_QUIVER_LINE1_2, dataSlotMask);
    ok &= expect_int("quiver line2 second pc34 slot",
                     spec->quiverLine2SecondPc34Slot,
                     DM1_PC34_SLOT_QUIVER_LINE2_2, dataSlotMask);
    ok &= expect_int("backpack first pc34 slot",
                     spec->backpackFirstPc34Slot,
                     DM1_PC34_SLOT_BACKPACK_LINE1_1, dataDropOrder);
    ok &= expect_int("backpack last pc34 slot",
                     spec->backpackLastPc34Slot,
                     DM1_PC34_SLOT_BACKPACK_LINE1_9, dataDropOrder);
    ok &= expect_true("data slot mask anchor present",
                      spec->dataSlotMaskAnchor &&
                      strstr(spec->dataSlotMaskAnchor, "G0038") != 0,
                      dataSlotMask);
    ok &= expect_true("data drop order anchor present",
                      spec->dataDropOrderAnchor &&
                      strstr(spec->dataDropOrderAnchor, "G0057") != 0,
                      dataDropOrder);
    ok &= expect_true("F0302 anchor present",
                      spec->championF0302Anchor &&
                      strstr(spec->championF0302Anchor, "F0302") != 0,
                      f0302);
    ok &= expect_true("F0300 anchor present",
                      spec->championF0300Anchor &&
                      strstr(spec->championF0300Anchor, "F0300") != 0,
                      f0300);

    ok &= expect_true("evidence references G0038",
                      evidence && strstr(evidence, "G0038") != 0,
                      dataSlotMask);
    ok &= expect_true("evidence references G0057",
                      evidence && strstr(evidence, "G0057") != 0,
                      dataDropOrder);
    ok &= expect_true("evidence references F0302 line 1546 forced drop",
                      evidence && strstr(evidence, "1546") != 0,
                      f0300);
    ok &= expect_true("evidence references F0302 684-710",
                      evidence && strstr(evidence, "684-710") != 0,
                      f0302);
    return ok;
}

static int test_rule_classifier(void)
{
    int ok = 1;
    const char* anchor =
        "ReDMCSB DATA.C G0038_ai_Graphic562_SlotMasks lines 1049-1087";

    ok &= expect_int("empty mask is no fit",
                     dm1_v1_inventory_slot_placement_rule_for_pc34(0),
                     DM1_V1_ISP_RULE_NO_FIT, anchor);
    ok &= expect_int("container-only mask rejects auto-place",
                     dm1_v1_inventory_slot_placement_rule_for_pc34(
                         DM1_PC34_ALLOWED_CONTAINER),
                     DM1_V1_ISP_RULE_CONTAINER_REJECT, anchor);
    ok &= expect_int("hands-only mask is hand rule",
                     dm1_v1_inventory_slot_placement_rule_for_pc34(
                         DM1_PC34_ALLOWED_HANDS),
                     DM1_V1_ISP_RULE_HAND, anchor);
    ok &= expect_int("pouch-only mask is pouch rule",
                     dm1_v1_inventory_slot_placement_rule_for_pc34(
                         DM1_PC34_ALLOWED_POUCH),
                     DM1_V1_ISP_RULE_POUCH, anchor);
    ok &= expect_int("quiver line1-only mask is quiver rule",
                     dm1_v1_inventory_slot_placement_rule_for_pc34(
                         DM1_PC34_ALLOWED_QUIVER_LINE1),
                     DM1_V1_ISP_RULE_QUIVER, anchor);
    ok &= expect_int("quiver line2-only mask is quiver rule",
                     dm1_v1_inventory_slot_placement_rule_for_pc34(
                         DM1_PC34_ALLOWED_QUIVER_LINE2),
                     DM1_V1_ISP_RULE_QUIVER, anchor);
    ok &= expect_int("body torso-only mask is body rule",
                     dm1_v1_inventory_slot_placement_rule_for_pc34(
                         DM1_PC34_ALLOWED_TORSO),
                     DM1_V1_ISP_RULE_BODY, anchor);
    ok &= expect_int("any-slot mask is backpack rule",
                     dm1_v1_inventory_slot_placement_rule_for_pc34(
                         DM1_PC34_ALLOWED_ANY_SLOT),
                     DM1_V1_ISP_RULE_BACKPACK, anchor);
    ok &= expect_int("torso + pouch prefers body rule",
                     dm1_v1_inventory_slot_placement_rule_for_pc34(
                         DM1_PC34_ALLOWED_TORSO | DM1_PC34_ALLOWED_POUCH),
                     DM1_V1_ISP_RULE_BODY, anchor);
    ok &= expect_int("hands + pouch prefers hand rule",
                     dm1_v1_inventory_slot_placement_rule_for_pc34(
                         DM1_PC34_ALLOWED_HANDS | DM1_PC34_ALLOWED_POUCH),
                     DM1_V1_ISP_RULE_HAND, anchor);
    ok &= expect_int("container + any-slot still routes to any-slot class",
                     dm1_v1_inventory_slot_placement_rule_for_pc34(
                         DM1_PC34_ALLOWED_CONTAINER |
                         DM1_PC34_ALLOWED_ANY_SLOT),
                     DM1_V1_ISP_RULE_BACKPACK, anchor);
    ok &= expect_int("head body is body rule",
                     dm1_v1_inventory_slot_placement_rule_for_pc34(
                         DM1_PC34_ALLOWED_HEAD),
                     DM1_V1_ISP_RULE_BODY, anchor);
    ok &= expect_int("feet body is body rule",
                     dm1_v1_inventory_slot_placement_rule_for_pc34(
                         DM1_PC34_ALLOWED_FEET),
                     DM1_V1_ISP_RULE_BODY, anchor);
    ok &= expect_int("neck body is body rule",
                     dm1_v1_inventory_slot_placement_rule_for_pc34(
                         DM1_PC34_ALLOWED_NECK),
                     DM1_V1_ISP_RULE_BODY, anchor);
    ok &= expect_int("legs body is body rule",
                     dm1_v1_inventory_slot_placement_rule_for_pc34(
                         DM1_PC34_ALLOWED_LEGS),
                     DM1_V1_ISP_RULE_BODY, anchor);
    return ok;
}

static int test_pick_helper(void)
{
    int ok = 1;
    const char* f0302 =
        "ReDMCSB CHAMPION.C F0302 lines 684-710";

    /* HAND rule: Ready Hand wins, regardless of other flags. */
    ok &= expect_int("hand rule picks ready hand",
                     dm1_v1_inventory_slot_placement_pick_pc34(
                         DM1_PC34_ALLOWED_HANDS,
                         0, 0, 0, 0, 0, 0, 0),
                     DM1_PC34_SLOT_READY_HAND, f0302);

    /* BODY rule: each body part has one slot. */
    ok &= expect_int("head body picks head slot",
                     dm1_v1_inventory_slot_placement_pick_pc34(
                         DM1_PC34_ALLOWED_HEAD,
                         0, 0, 0, 0, 0, 0, 0),
                     DM1_PC34_SLOT_HEAD, f0302);
    ok &= expect_int("torso body picks torso slot",
                     dm1_v1_inventory_slot_placement_pick_pc34(
                         DM1_PC34_ALLOWED_TORSO,
                         0, 0, 0, 0, 0, 0, 0),
                     DM1_PC34_SLOT_TORSO, f0302);
    ok &= expect_int("legs body picks legs slot",
                     dm1_v1_inventory_slot_placement_pick_pc34(
                         DM1_PC34_ALLOWED_LEGS,
                         0, 0, 0, 0, 0, 0, 0),
                     DM1_PC34_SLOT_LEGS, f0302);
    ok &= expect_int("feet body picks feet slot",
                     dm1_v1_inventory_slot_placement_pick_pc34(
                         DM1_PC34_ALLOWED_FEET,
                         0, 0, 0, 0, 0, 0, 0),
                     DM1_PC34_SLOT_FEET, f0302);
    ok &= expect_int("neck body picks neck slot",
                     dm1_v1_inventory_slot_placement_pick_pc34(
                         DM1_PC34_ALLOWED_NECK,
                         0, 0, 0, 0, 0, 0, 0),
                     DM1_PC34_SLOT_NECK, f0302);

    /* POUCH rule: Pouch 1 -> Pouch 2 -> reject. */
    ok &= expect_int("pouch rule picks pouch1",
                     dm1_v1_inventory_slot_placement_pick_pc34(
                         DM1_PC34_ALLOWED_POUCH,
                         0, 0, 0, 0, 0, 0, 0),
                     DM1_PC34_SLOT_POUCH_1, f0302);
    ok &= expect_int("pouch rule picks pouch2 when pouch1 taken",
                     dm1_v1_inventory_slot_placement_pick_pc34(
                         DM1_PC34_ALLOWED_POUCH,
                         1, 0, 0, 0, 0, 0, 0),
                     DM1_PC34_SLOT_POUCH_2, f0302);
    ok &= expect_int("pouch rule rejects when both occupied",
                     dm1_v1_inventory_slot_placement_pick_pc34(
                         DM1_PC34_ALLOWED_POUCH,
                         1, 1, 0, 0, 0, 0, 0),
                     -1, f0302);

    /* QUIVER rule. */
    ok &= expect_int("quiver line1-only picks line1 slot 1",
                     dm1_v1_inventory_slot_placement_pick_pc34(
                         DM1_PC34_ALLOWED_QUIVER_LINE1,
                         0, 0, 0, 0, 0, 0, 0),
                     DM1_PC34_SLOT_QUIVER_LINE1_1, f0302);
    ok &= expect_int("quiver line2-only picks line2 slot 1",
                     dm1_v1_inventory_slot_placement_pick_pc34(
                         DM1_PC34_ALLOWED_QUIVER_LINE2,
                         0, 0, 0, 0, 0, 0, 0),
                     DM1_PC34_SLOT_QUIVER_LINE2_1, f0302);
    ok &= expect_int("quiver line2-only falls to line1 slot 2",
                     dm1_v1_inventory_slot_placement_pick_pc34(
                         DM1_PC34_ALLOWED_QUIVER_LINE2,
                         0, 0, 0, 1, 0, 0, 0),
                     DM1_PC34_SLOT_QUIVER_LINE1_2, f0302);
    ok &= expect_int("quiver line2-only falls to line2 slot 2",
                     dm1_v1_inventory_slot_placement_pick_pc34(
                         DM1_PC34_ALLOWED_QUIVER_LINE2,
                         0, 0, 0, 1, 1, 0, 0),
                     DM1_PC34_SLOT_QUIVER_LINE2_2, f0302);
    ok &= expect_int("quiver line2-only rejects when all 4 slots taken",
                     dm1_v1_inventory_slot_placement_pick_pc34(
                         DM1_PC34_ALLOWED_QUIVER_LINE2,
                         0, 0, 0, 1, 1, 1, 0),
                     -1, f0302);

    /* BACKPACK rule: G0057 reverse priority (C29 Line1_9 first,
     * then C28 Line1_8, ..., C13 Line1_1, C14..C21 Line2_2..9).
     * The helper tracks the leading-occupied count for the first
     * four Line1 slots in G0057-reverse order. */
    ok &= expect_int("backpack rule picks C29 line1 slot 9 first",
                     dm1_v1_inventory_slot_placement_pick_pc34(
                         DM1_PC34_ALLOWED_ANY_SLOT,
                         0, 0, 0, 0, 0, 0, 0),
                     DM1_PC34_SLOT_BACKPACK_LINE1_9, f0302);
    ok &= expect_int("backpack rule picks C28 line1 slot 8 next",
                     dm1_v1_inventory_slot_placement_pick_pc34(
                         DM1_PC34_ALLOWED_ANY_SLOT,
                         0, 0, 0, 0, 0, 0, 1),
                     DM1_PC34_SLOT_BACKPACK_LINE1_8, f0302);
    ok &= expect_int("backpack rule picks C27 line1 slot 7 next",
                     dm1_v1_inventory_slot_placement_pick_pc34(
                         DM1_PC34_ALLOWED_ANY_SLOT,
                         0, 0, 0, 0, 0, 0, 2),
                     DM1_PC34_SLOT_BACKPACK_LINE1_7, f0302);
    ok &= expect_int("backpack rule picks C26 line1 slot 6 next",
                     dm1_v1_inventory_slot_placement_pick_pc34(
                         DM1_PC34_ALLOWED_ANY_SLOT,
                         0, 0, 0, 0, 0, 0, 3),
                     DM1_PC34_SLOT_BACKPACK_LINE1_6, f0302);
    ok &= expect_int("backpack rule rejects when 4 leading slots taken",
                     dm1_v1_inventory_slot_placement_pick_pc34(
                         DM1_PC34_ALLOWED_ANY_SLOT,
                         0, 0, 0, 0, 0, 0, 4),
                     -1, f0302);

    /* CONTAINER reject: never auto-place. */
    ok &= expect_int("container-only mask rejects auto-place",
                     dm1_v1_inventory_slot_placement_pick_pc34(
                         DM1_PC34_ALLOWED_CONTAINER,
                         0, 0, 0, 0, 0, 0, 0),
                     -1, f0302);

    /* Multi-mask priority: body+pouch prefers body (torso); the
     * helper routes via the BODY branch because BODY is the most
     * specific mask in the rule classifier. */
    ok &= expect_int("torso + pouch prefers torso",
                     dm1_v1_inventory_slot_placement_pick_pc34(
                         DM1_PC34_ALLOWED_TORSO | DM1_PC34_ALLOWED_POUCH,
                         0, 0, 0, 0, 0, 0, 0),
                     DM1_PC34_SLOT_TORSO, f0302);
    ok &= expect_int("hands + pouch prefers ready hand",
                     dm1_v1_inventory_slot_placement_pick_pc34(
                         DM1_PC34_ALLOWED_HANDS | DM1_PC34_ALLOWED_POUCH,
                         0, 0, 0, 0, 0, 0, 0),
                     DM1_PC34_SLOT_READY_HAND, f0302);
    return ok;
}

static int test_probe(void)
{
    int ok = 1;
    const char* f0302 = "ReDMCSB CHAMPION.C F0302 lines 684-710";
    const char* dataSlotMask =
        "ReDMCSB DATA.C G0038_ai_Graphic562_SlotMasks lines 1049-1087";
    const char* dataDropOrder =
        "ReDMCSB DATA.C G0057_ai_Graphic562_SlotDropOrder lines 436-466";
    DM1_V1_InventorySlotPlacementProbePc34 probe;

    if (!dm1_v1_inventory_slot_placement_probe_pc34(&probe)) {
        printf("FAIL placement probe returned 0\n");
        return 0;
    }
    ++g_assertions;
    ++g_passes;

    ok &= expect_int("probe contractOnly",
                     probe.contractOnly, 1, f0302);

    /* HAND rule cases. */
    ok &= expect_case("hand empty ready", &probe.handEmptyReady,
                      DM1_V1_ISP_RULE_HAND, DM1_PC34_SLOT_READY_HAND,
                      dataSlotMask);
    ok &= expect_case("hand empty action", &probe.handEmptyAction,
                      DM1_V1_ISP_RULE_HAND, DM1_PC34_SLOT_READY_HAND,
                      dataSlotMask);
    ok &= expect_case("hand both occupied", &probe.handBothOccupiedRejects,
                      DM1_V1_ISP_RULE_HAND, DM1_PC34_SLOT_READY_HAND,
                      dataSlotMask);

    /* BODY rule cases. */
    ok &= expect_case("head", &probe.head,
                      DM1_V1_ISP_RULE_BODY, DM1_PC34_SLOT_HEAD,
                      dataSlotMask);
    ok &= expect_case("torso", &probe.torso,
                      DM1_V1_ISP_RULE_BODY, DM1_PC34_SLOT_TORSO,
                      dataSlotMask);
    ok &= expect_case("legs", &probe.legs,
                      DM1_V1_ISP_RULE_BODY, DM1_PC34_SLOT_LEGS,
                      dataSlotMask);
    ok &= expect_case("feet", &probe.feet,
                      DM1_V1_ISP_RULE_BODY, DM1_PC34_SLOT_FEET,
                      dataSlotMask);
    ok &= expect_case("neck", &probe.neck,
                      DM1_V1_ISP_RULE_BODY, DM1_PC34_SLOT_NECK,
                      dataSlotMask);

    /* POUCH rule cases. */
    ok &= expect_case("pouch1 empty", &probe.pouch1Empty,
                      DM1_V1_ISP_RULE_POUCH, DM1_PC34_SLOT_POUCH_1,
                      dataSlotMask);
    ok &= expect_case("pouch1 occupied falls to pouch2",
                      &probe.pouch1OccupiedFallsToPouch2,
                      DM1_V1_ISP_RULE_POUCH, DM1_PC34_SLOT_POUCH_2,
                      dataSlotMask);
    ok &= expect_case("pouch both occupied rejects",
                      &probe.pouchBothOccupiedRejects,
                      DM1_V1_ISP_RULE_POUCH, -1, dataSlotMask);

    /* QUIVER rule cases. */
    ok &= expect_case("quiver line1 empty", &probe.quiverLine1Empty,
                      DM1_V1_ISP_RULE_QUIVER, DM1_PC34_SLOT_QUIVER_LINE1_1,
                      dataSlotMask);
    ok &= expect_case("quiver line2 empty", &probe.quiverLine2Empty,
                      DM1_V1_ISP_RULE_QUIVER, DM1_PC34_SLOT_QUIVER_LINE2_1,
                      dataSlotMask);
    ok &= expect_case("quiver line2 first occupied falls line1 second",
                      &probe.quiverLine2FirstOccupiedFallsLine1Second,
                      DM1_V1_ISP_RULE_QUIVER, DM1_PC34_SLOT_QUIVER_LINE1_2,
                      dataSlotMask);
    ok &= expect_case("quiver line2 all 4 taken rejects",
                      &probe.quiverLine2BothSecondSlotsTakenRejects,
                      DM1_V1_ISP_RULE_QUIVER, -1, dataSlotMask);

    /* BACKPACK rule cases. */
    ok &= expect_case("backpack first free", &probe.backpackFirstFree,
                      DM1_V1_ISP_RULE_BACKPACK,
                      DM1_PC34_SLOT_BACKPACK_LINE1_9, dataDropOrder);
    ok &= expect_case("backpack line1 slot 9 occupied falls line1 slot 8",
                      &probe.backpackLine1OccupiedFallsLine2,
                      DM1_V1_ISP_RULE_BACKPACK,
                      DM1_PC34_SLOT_BACKPACK_LINE1_8, dataDropOrder);

    /* CONTAINER reject. */
    ok &= expect_case("container-only rejected",
                      &probe.containerOnlyRejected,
                      DM1_V1_ISP_RULE_CONTAINER_REJECT, -1, f0302);

    /* Multi-mask priority cases. */
    ok &= expect_case("body + pouch prefers body",
                      &probe.bodyAndPouchPrefersBody,
                      DM1_V1_ISP_RULE_BODY, DM1_PC34_SLOT_TORSO,
                      dataSlotMask);
    ok &= expect_case("hands + pouch prefers hand",
                      &probe.handsAndPouchPrefersHand,
                      DM1_V1_ISP_RULE_HAND, DM1_PC34_SLOT_READY_HAND,
                      dataSlotMask);

    /* Aggregate invariants. */
    ok &= expect_int("hand rule honored",
                     probe.handRuleHonored, 1, f0302);
    ok &= expect_int("body rule honored",
                     probe.bodyRuleHonored, 1, f0302);
    ok &= expect_int("pouch rule honored",
                     probe.pouchRuleHonored, 1, f0302);
    ok &= expect_int("quiver rule honored",
                     probe.quiverRuleHonored, 1, f0302);
    ok &= expect_int("backpack rule honored",
                     probe.backpackRuleHonored, 1, dataDropOrder);
    ok &= expect_int("container reject honored",
                     probe.containerRejectHonored, 1, f0302);
    ok &= expect_int("priority order honored",
                     probe.priorityOrderHonored, 1, dataSlotMask);
    ok &= expect_int("matches can_equip honored",
                     probe.matchesCanEquipHonored, 1, f0302);
    ok &= expect_int("fits open chest honored",
                     probe.fitsOpenChestHonored, 1, f0302);
    return ok;
}

int main(void)
{
    int ok = 1;

    ok &= test_spec_and_evidence();
    ok &= test_rule_classifier();
    ok &= test_pick_helper();
    ok &= test_probe();
    if (!ok) {
        printf("FAIL dm1_v1_inventory_slot_placement_pc34_compat "
               "%d/%d assertions\n", g_passes, g_assertions);
        return 1;
    }
    printf("PASS dm1_v1_inventory_slot_placement_pc34_compat "
           "%d/%d assertions\n", g_passes, g_assertions);
    return 0;
}
