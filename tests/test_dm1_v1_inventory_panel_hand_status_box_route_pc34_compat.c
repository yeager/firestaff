#include "dm1/dm1_v1_inventory_panel_hand_status_box_route_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_assertions;
static DM1_V1_InventoryPanelHandStatusBoxRouteProbePc34 g_probe;

static int expect_int(const char* label, int got, int want,
                      const char* redmcsbAnchor)
{
    ++g_assertions;
    if (!redmcsbAnchor || redmcsbAnchor[0] == '\0') {
        printf("FAIL %s missing ReDMCSB anchor\n", label);
        return 0;
    }
    if (got != want) {
        printf("FAIL %s got=%d want=%d anchor=%s\n", label, got, want,
               redmcsbAnchor);
        return 0;
    }
    printf("ok %s=%d anchor=%s\n", label, got, redmcsbAnchor);
    return 1;
}

static int expect_contains(const char* label, const char* got,
                           const char* want, const char* redmcsbAnchor)
{
    ++g_assertions;
    if (!redmcsbAnchor || redmcsbAnchor[0] == '\0') {
        printf("FAIL %s missing ReDMCSB anchor\n", label);
        return 0;
    }
    if (!got || !want || !strstr(got, want)) {
        printf("FAIL %s missing '%s' anchor=%s\n", label,
               want ? want : "(null)", redmcsbAnchor);
        return 0;
    }
    printf("ok %s contains=%s anchor=%s\n", label, want, redmcsbAnchor);
    return 1;
}

static int test_spec_and_evidence(void)
{
    const DM1_V1_InventoryPanelHandStatusBoxRouteSpecPc34* spec =
        dm1_v1_inventory_panel_hand_status_box_route_spec_pc34();
    const char* evidence =
        dm1_v1_inventory_panel_hand_status_box_route_source_evidence_pc34();
    const char* f0302 = "ReDMCSB CHAMPION.C F0302 lines 662-710";
    const char* chamdraw = "ReDMCSB CHAMDRAW.C F0292 lines 543-549";
    const char* defs = "ReDMCSB DEFS.H C08_SLOT_BOX_INVENTORY_FIRST_SLOT";
    int ok = 1;

    ok &= expect_int("spec present", spec != 0, 1, f0302);
    ok &= expect_int("contract only", spec->contractOnly, 1, f0302);
    ok &= expect_int("status slotbox first", spec->statusSlotBoxFirst,
                     DM1_V1_IPHSBR_STATUS_SLOT_BOX_FIRST, f0302);
    ok &= expect_int("status slotbox last", spec->statusSlotBoxLast,
                     DM1_V1_IPHSBR_STATUS_SLOT_BOX_LAST, f0302);
    ok &= expect_int("status slotbox count", spec->statusSlotBoxCount,
                     DM1_V1_IPHSBR_STATUS_SLOT_BOX_COUNT, f0302);
    ok &= expect_int("party limit", spec->partyLimit,
                     DM1_V1_IPHSBR_STATUS_SLOT_BOX_PARTY_LIMIT, f0302);
    ok &= expect_int("inventory first slotbox", spec->inventoryFirstSlotBox,
                     DM1_V1_IPHSBR_INVENTORY_FIRST_SLOT_BOX, defs);
    ok &= expect_int("thing end", spec->thingEnd,
                     DM1_V1_IPHSBR_THING_END, f0302);
    ok &= expect_int("thing none", spec->thingNone,
                     DM1_V1_IPHSBR_THING_NONE, f0302);
    ok &= expect_int("rejected sentinel", spec->rejected,
                     DM1_V1_IPHSBR_REJECTED, f0302);
    ok &= expect_int("food/water/poisoned object",
                     spec->foodWaterPoisonedObject,
                     DM1_V1_IPHSBR_FOOD_WATER_POISONED_OBJECT, f0302);
    ok &= expect_int("head object", spec->headObject,
                     DM1_V1_IPHSBR_HEAD_OBJECT, f0302);
    ok &= expect_int("chest object", spec->chestObject,
                     DM1_V1_IPHSBR_CHEST_OBJECT, f0302);
    ok &= expect_int("scroll object", spec->scrollObject,
                     DM1_V1_IPHSBR_SCROLL_OBJECT, f0302);
    ok &= expect_contains("spec f0302 anchor", spec->f0302Anchor,
                          "F0302_CHAMPION_ProcessCommands28To65_ClickOnSlotBox",
                          f0302);
    ok &= expect_contains("spec f0292 anchor", spec->f0292Anchor,
                          "F0292_CHAMPION_DrawState", f0302);
    ok &= expect_contains("spec chamdraw ready hand anchor",
                          spec->chamdrawReadyHandAnchor, "ready hand",
                          chamdraw);
    ok &= expect_contains("spec chamdraw action hand anchor",
                          spec->chamdrawActionHandAnchor, "action hand",
                          chamdraw);
    ok &= expect_contains("spec defs slotbox inventory first",
                          spec->defsSlotBoxInventoryFirstAnchor,
                          "C08_SLOT_BOX_INVENTORY_FIRST_SLOT", defs);
    ok &= expect_contains("spec defs hand slot index anchor",
                          spec->defsHandSlotIndexAnchor, "M070_HAND_SLOT_INDEX",
                          defs);
    ok &= expect_contains("contract scope", spec->scope, "contract_only=1",
                          f0302);
    ok &= expect_contains("contract avoids real assets", spec->scope,
                          "without real-asset", f0302);
    ok &= expect_contains("evidence f0302", evidence, "F0302:662-710", f0302);
    ok &= expect_contains("evidence chamdraw", evidence, "F0292:543-549",
                          chamdraw);
    ok &= expect_contains("evidence defs inventory first", evidence,
                          "C08_SLOT_BOX_INVENTORY_FIRST_SLOT=8", defs);
    ok &= expect_contains("evidence defs hand slot index", evidence,
                          "M070_HAND_SLOT_INDEX", defs);
    return ok;
}

static int test_status_row_slotbox_table(void)
{
    const char* f0302 = "ReDMCSB CHAMPION.C F0302 lines 662-710";
    const char* chamdraw = "ReDMCSB CHAMDRAW.C F0292 lines 543-549";
    const char* defs = "ReDMCSB DEFS.H M070_HAND_SLOT_INDEX";
    int slotBoxIndex;
    int ok = 1;

    for (slotBoxIndex = DM1_V1_IPHSBR_STATUS_SLOT_BOX_FIRST;
         slotBoxIndex <= DM1_V1_IPHSBR_STATUS_SLOT_BOX_LAST; ++slotBoxIndex) {
        const DM1_V1_InventoryPanelHandStatusBoxRouteCasePc34* row =
            &g_probe.slotBoxes[slotBoxIndex];
        const char* handAnchor = (slotBoxIndex & 1) ? chamdraw : chamdraw;
        char championLabel[64];
        char resolvedLabel[64];
        char slotLabel[64];

        snprintf(championLabel, sizeof(championLabel),
                 "slotbox %d champion index", slotBoxIndex);
        snprintf(resolvedLabel, sizeof(resolvedLabel),
                 "slotbox %d resolved return", slotBoxIndex);
        snprintf(slotLabel, sizeof(slotLabel),
                 "slotbox %d source slot", slotBoxIndex);

        ok &= expect_int(championLabel, row->expectedChampionIndex,
                         slotBoxIndex >> 1, f0302);
        ok &= expect_int(slotLabel, row->expectedPc34SourceSlot,
                         (slotBoxIndex & 1) ? DM1_PC34_SLOT_ACTION_HAND
                                            : DM1_PC34_SLOT_READY_HAND,
                         defs);
        ok &= expect_int(resolvedLabel, row->expectedResolved, 1, f0302);
        ok &= expect_int(row->slotBoxBelongsToStatusRow ? "slotbox < 8 owned by status row"
                                                         : "slotbox >= 8 owned by inventory",
                         row->slotBoxBelongsToStatusRow, 1, defs);
        ok &= expect_int(row->expectedResolved ? "slotbox resolves to champion" :
                                                  "slotbox above healthy count rejects",
                         row->resolvedReturn, row->expectedResolved, f0302);
        ok &= expect_int(row->expectedResolved ? "resolved champion index matches expectation" :
                                                  "rejected champion index sentinel",
                         row->resolvedChampionIndex, row->expectedResolved ?
                             row->expectedChampionIndex :
                             DM1_V1_IPHSBR_REJECTED,
                         f0302);
        ok &= expect_int(row->expectedResolved ? "resolved pc34 source slot matches expectation" :
                                                  "rejected pc34 source slot sentinel",
                         row->resolvedPc34SourceSlot, row->expectedResolved ?
                             row->expectedPc34SourceSlot :
                             DM1_V1_IPHSBR_REJECTED,
                         handAnchor);
        if (row->expectedResolved) {
            ok &= expect_int(row->expectedChampionIndex == 0
                                 ? "leader hand mouse has chest before click"
                                 : "leader hand mouse has chest before click",
                             row->leaderHandObjectBefore,
                             DM1_V1_IPHSBR_CHEST_OBJECT, f0302);
            ok &= expect_int(row->expectedChampionIndex == 0
                                 ? "leader hand mouse has chest before click"
                                 : "leader hand mouse has chest before click (redundant)",
                             row->mouseItemTypeBefore,
                             DM1_V1_IPHSBR_CHEST_OBJECT, f0302);
            ok &= expect_int("click result succeeds", row->clickResult, 1,
                             f0302);
            ok &= expect_int("slot receives chest object", row->slotItemTypeAfter,
                             DM1_V1_IPHSBR_CHEST_OBJECT, f0302);
            ok &= expect_int("mouse item now holds the swapped slot object",
                             row->mouseItemTypeAfter,
                             DM1_V1_IPHSBR_FOOD_WATER_POISONED_OBJECT, f0302);
            ok &= expect_int("leader hand after click holds the swapped slot object",
                             row->leaderHandObjectAfter,
                             DM1_V1_IPHSBR_FOOD_WATER_POISONED_OBJECT, f0302);
            ok &= expect_int("status row icon mutated", row->chestObjectIconUnchangedInStatusBox,
                             0, chamdraw);
        } else {
            ok &= expect_int("rejected slotbox click not attempted",
                             row->clickResult, 0, f0302);
            ok &= expect_int("rejected slotbox keeps leader hand intact",
                             row->leaderHandObjectAfter,
                             row->leaderHandObjectBefore, f0302);
            ok &= expect_int("rejected slotbox keeps mouse intact",
                             row->mouseItemTypeAfter,
                             row->mouseItemTypeBefore, f0302);
            ok &= expect_int("rejected slotbox keeps slot intact",
                             row->slotItemTypeAfter, row->slotItemTypeBefore,
                             f0302);
            ok &= expect_int("rejected slotbox leaves status row icon intact",
                             row->chestObjectIconUnchangedInStatusBox, 1,
                             chamdraw);
        }
    }

    ok &= expect_int("slotbox 0 reduces to champion 0 ready hand",
                     g_probe.slotbox0ReducesToChampion0ReadyHand, 1, chamdraw);
    ok &= expect_int("slotbox 3 reduces to champion 1 action hand",
                     g_probe.slotbox3ReducesToChampion1ActionHand, 1, chamdraw);
    ok &= expect_int("slotbox 4 reduces to champion 2 ready hand",
                     g_probe.slotbox4ReducesToChampion2ReadyHand, 1, chamdraw);
    ok &= expect_int("slotbox 7 reduces to champion 3 action hand",
                     g_probe.slotbox7ReducesToChampion3ActionHand, 1, chamdraw);
    ok &= expect_int("inventory first slotbox boundary",
                     g_probe.inventoryFirstSlotBox, 8, defs);
    ok &= expect_int("alive slotbox 0 click moves the chest mouse item",
                     g_probe.clickOnAliveChampionMovesObject, 1, f0302);
    return ok;
}

static int test_negative_cases(void)
{
    const char* f0302 = "ReDMCSB CHAMPION.C F0302 lines 662-710";
    const char* defs = "ReDMCSB DEFS.H M070_HAND_SLOT_INDEX";
    int ok = 1;

    ok &= expect_int("negative slotbox returns 0", g_probe.negativeSlotBoxReturn,
                     0, f0302);
    ok &= expect_int("negative slotbox champion sentinel",
                     g_probe.negativeSlotBoxOutChampionIndex,
                     DM1_V1_IPHSBR_REJECTED, f0302);
    ok &= expect_int("negative slotbox pc34 source slot sentinel",
                     g_probe.negativeSlotBoxOutPc34SourceSlot,
                     DM1_V1_IPHSBR_REJECTED, f0302);
    ok &= expect_int("overlarge slotbox returns 0",
                     g_probe.overlargeSlotBoxReturn, 0, f0302);
    ok &= expect_int("overlarge slotbox champion sentinel",
                     g_probe.overlargeSlotBoxOutChampionIndex,
                     DM1_V1_IPHSBR_REJECTED, f0302);
    ok &= expect_int("overlarge slotbox pc34 source slot sentinel",
                     g_probe.overlargeSlotBoxOutPc34SourceSlot,
                     DM1_V1_IPHSBR_REJECTED, defs);
    ok &= expect_int("negative party count returns 0",
                     g_probe.negativePartyCountReturn, 0, f0302);
    ok &= expect_int("negative party count champion sentinel",
                     g_probe.negativePartyCountOutChampionIndex,
                     DM1_V1_IPHSBR_REJECTED, f0302);
    ok &= expect_int("negative party count pc34 source slot sentinel",
                     g_probe.negativePartyCountOutPc34SourceSlot,
                     DM1_V1_IPHSBR_REJECTED, f0302);
    ok &= expect_int("overlarge party count returns 0",
                     g_probe.overlargePartyCountReturn, 0, f0302);
    ok &= expect_int("overlarge party count champion sentinel",
                     g_probe.overlargePartyCountOutChampionIndex,
                     DM1_V1_IPHSBR_REJECTED, f0302);
    ok &= expect_int("overlarge party count pc34 source slot sentinel",
                     g_probe.overlargePartyCountOutPc34SourceSlot,
                     DM1_V1_IPHSBR_REJECTED, f0302);
    ok &= expect_int("null health returns 0", g_probe.nullHealthReturn, 0,
                     f0302);
    ok &= expect_int("null health champion sentinel",
                     g_probe.nullHealthOutChampionIndex,
                     DM1_V1_IPHSBR_REJECTED, f0302);
    ok &= expect_int("null health pc34 source slot sentinel",
                     g_probe.nullHealthOutPc34SourceSlot,
                     DM1_V1_IPHSBR_REJECTED, f0302);
    ok &= expect_int("candidate champion rejects",
                     g_probe.candidateChampionRejected, 0, f0302);
    ok &= expect_int("candidate champion champion sentinel",
                     g_probe.candidateChampionOutChampionIndex,
                     DM1_V1_IPHSBR_REJECTED, f0302);
    ok &= expect_int("candidate champion pc34 source slot sentinel",
                     g_probe.candidateChampionOutPc34SourceSlot,
                     DM1_V1_IPHSBR_REJECTED, f0302);
    ok &= expect_int("slotbox 4 champion above party rejects",
                     g_probe.slotbox4ChampionAbovePartyReturn, 0, f0302);
    ok &= expect_int("slotbox 4 champion above party champion sentinel",
                     g_probe.slotbox4ChampionAbovePartyOutChampionIndex,
                     DM1_V1_IPHSBR_REJECTED, f0302);
    ok &= expect_int("slotbox 4 champion above party pc34 sentinel",
                     g_probe.slotbox4ChampionAbovePartyOutPc34SourceSlot,
                     DM1_V1_IPHSBR_REJECTED, f0302);
    ok &= expect_int("inventory champion 1 slotbox 0 rejects",
                     g_probe.inventoryChampion1Slotbox0Return, 0, f0302);
    ok &= expect_int("inventory champion 1 slotbox 0 champion sentinel",
                     g_probe.inventoryChampion1Slotbox0OutChampionIndex,
                     DM1_V1_IPHSBR_REJECTED, f0302);
    ok &= expect_int("inventory champion 1 slotbox 0 pc34 sentinel",
                     g_probe.inventoryChampion1Slotbox0OutPc34SourceSlot,
                     DM1_V1_IPHSBR_REJECTED, f0302);
    ok &= expect_int("dead champion 0 rejects", g_probe.deadChampion0Return, 0,
                     f0302);
    ok &= expect_int("dead champion 0 champion sentinel",
                     g_probe.deadChampion0OutChampionIndex,
                     DM1_V1_IPHSBR_REJECTED, f0302);
    ok &= expect_int("dead champion 0 pc34 sentinel",
                     g_probe.deadChampion0OutPc34SourceSlot,
                     DM1_V1_IPHSBR_REJECTED, f0302);
    return ok;
}

static int test_dead_champion_no_click_path(void)
{
    const char* f0302 = "ReDMCSB CHAMPION.C F0302 lines 662-710";
    int ok = 1;

    /* The dead-champion early return in F0302:681 is enforced by the
     * status hand slot box routing, not by m11_inventory_click_pc34_source_slot.
     * The contract-only probe therefore demonstrates that the dispatch code
     * in F0302:677-684 short-circuits before the F0302:695-708 swap path, so
     * the click on a dead champion is never reached and no click is needed.
     * This stays inside the F0302:677-684 contract and does NOT claim
     * real-asset parity. */
    ok &= expect_int("dead champion click is suppressed by routing",
                     g_probe.clickOnDeadChampionLeavesMouseIntact, 1, f0302);
    ok &= expect_int("dead champion slot is left at sentinel 0 (no click ran)",
                     g_probe.deadChampionSlotItemTypeAfter, 0, f0302);
    ok &= expect_int("dead champion mouse is left at sentinel 0 (no click ran)",
                     g_probe.deadChampionMouseItemTypeAfter, 0, f0302);
    return ok;
}

int main(void)
{
    const char* f0302 = "ReDMCSB CHAMPION.C F0302 lines 662-710";
    int ok = 1;

    printf("probe=dm1_v1_inventory_panel_hand_status_box_route_pc34_compat\n");
    printf("sourceEvidence=%s\n",
           dm1_v1_inventory_panel_hand_status_box_route_source_evidence_pc34());

    ok &= expect_int("probe run",
                     dm1_v1_inventory_panel_hand_status_box_route_pc34(&g_probe),
                     1, f0302);
    ok &= expect_int("probe contract only", g_probe.contractOnly, 1, f0302);
    ok &= test_spec_and_evidence();
    ok &= test_status_row_slotbox_table();
    ok &= test_negative_cases();
    ok &= test_dead_champion_no_click_path();

    g_probe.totalAssertions = g_assertions;
    printf("assertions=%d\n", g_assertions);
    printf("dm1V1InventoryPanelHandStatusBoxRouteOk=%d\n", ok ? 1 : 0);
    return ok ? 0 : 1;
}
