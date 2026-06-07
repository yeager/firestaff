#include "../src/dm1/dm1_v1_inventory_pouch_quiver_backpack_swap_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_assertions;
static int g_passes;

static int expect_int(const char* label,
                      int got,
                      int want,
                      const char* redmcsbAnchor)
{
    ++g_assertions;
    if (!redmcsbAnchor || redmcsbAnchor[0] == '\0') {
        printf("FAIL %s missing ReDMCSB anchor\n", label);
        return 0;
    }
    if (got != want) {
        printf("FAIL %s got=%d want=%d anchor=%s\n",
               label, got, want, redmcsbAnchor);
        return 0;
    }
    ++g_passes;
    printf("PASS %s=%d anchor=%s\n", label, got, redmcsbAnchor);
    return 1;
}

static int expect_str(const char* label,
                      const char* got,
                      const char* want,
                      const char* redmcsbAnchor)
{
    ++g_assertions;
    if (!redmcsbAnchor || redmcsbAnchor[0] == '\0') {
        printf("FAIL %s missing ReDMCSB anchor\n", label);
        return 0;
    }
    if (!got || !want || strcmp(got, want) != 0) {
        printf("FAIL %s got=%s want=%s anchor=%s\n",
               label, got ? got : "(null)", want ? want : "(null)",
               redmcsbAnchor);
        return 0;
    }
    ++g_passes;
    printf("PASS %s=%s anchor=%s\n", label, got, redmcsbAnchor);
    return 1;
}

static int test_spec(void)
{
    const DM1_V1_InventoryPouchQuiverBackpackSwapSpecPc34* spec =
        dm1_v1_inventory_pouch_quiver_backpack_swap_spec_pc34();
    const char* f0333 = "ReDMCSB CHEST.C F0333 lines 53-67";
    const char* f0334 = "ReDMCSB CHEST.C F0334 lines 117-132";
    const char* allowed =
        "ReDMCSB INVENTORY.C (PC 3.4) AllowedSlots lookup; DEFS.H "
        "C545_AllowedSlots_Pouch/C546_AllowedSlots_Quiver/"
        "C547_AllowedSlots_Backpack";
    const char* dataMasks =
        "ReDMCSB DATA.C G0038_ai_Graphic562_SlotMasks lines 1050-1087";
    const char* f0133 = "ReDMCSB BLITMASK.C F0133 lines 30-33";
    const char* f0291 =
        "ReDMCSB CHAMDRAW.C F0291/F0296 lines 551-552,1249-1252";
    int ok = 1;

    ok &= expect_int("spec exists", spec != 0, 1, allowed);
    ok &= expect_int("contract only", spec->contractOnly, 1, f0333);
    ok &= expect_int("pouch pc34 slot", spec->pouchPc34Slot,
                     DM1_V1_IPQBS_POUCH_SLOT, dataMasks);
    ok &= expect_int("pouch storage slot", spec->pouchStorageSlot,
                     DM1_SLOT_POUCH1, dataMasks);
    ok &= expect_int("pouch mask", spec->pouchMask,
                     DM1_PC34_ALLOWED_POUCH, allowed);
    ok &= expect_int("quiver pc34 slot", spec->quiverPc34Slot,
                     DM1_V1_IPQBS_QUIVER_SLOT, dataMasks);
    ok &= expect_int("quiver storage slot", spec->quiverStorageSlot,
                     DM1_SLOT_QUIVER1, dataMasks);
    ok &= expect_int("quiver mask", spec->quiverMask,
                     DM1_PC34_ALLOWED_QUIVER_LINE1, allowed);
    ok &= expect_int("backpack pc34 slot", spec->backpackPc34Slot,
                     DM1_V1_IPQBS_BACKPACK_SLOT, dataMasks);
    ok &= expect_int("backpack storage slot", spec->backpackStorageSlot,
                     DM1_SLOT_BACKPACK1, dataMasks);
    ok &= expect_int("backpack mask", spec->backpackMask,
                     DM1_PC34_ALLOWED_ANY_SLOT, allowed);
    ok &= expect_int("zero allowed slots mask", spec->zeroAllowedSlotsMask,
                     0, allowed);
    ok &= expect_str("chest open anchor", spec->chestOpenAnchor,
                     "CHEST.C F0333:53-67 opens G0425 chest slots before inventory routing",
                     f0333);
    ok &= expect_str("chest close anchor", spec->chestCloseAnchor,
                     "CHEST.C F0334:117-132 rewrites non-empty G0425 slots on close",
                     f0334);
    ok &= expect_str("allowed slots anchor", spec->allowedSlotsAnchor,
                     "INVENTORY.C (PC 3.4) AllowedSlots lookup; DEFS.H "
                     "C545_AllowedSlots_Pouch/C546_AllowedSlots_Quiver/"
                     "C547_AllowedSlots_Backpack map to 0x0100/0x0040/0xFFFF",
                     allowed);
    ok &= expect_str("blit mask anchor", spec->blitMaskAnchor,
                     "BLITMASK.C F0133:30-33 masked bitmap blit dispatch",
                     f0133);
    ok &= expect_str("icon blit anchor", spec->iconBlitAnchor,
                     "CHAMDRAW.C F0291/F0296:551-552,1249-1252 redraws C30+ icons",
                     f0291);
    ok &= expect_str("scope", spec->scope,
                     "contract_only=1; synthetic DM1 V1 hand-to-pouch/quiver/backpack "
                     "mask-swap gate, no real-asset runtime claim.",
                     allowed);
    return ok;
}

static int test_case(const char* name,
                     const DM1_V1_InventoryPouchQuiverBackpackSwapCasePc34* row,
                     int wantPc34Slot,
                     int wantStorageSlot,
                     int wantMask,
                     int wantItem,
                     int wantAllowedSlots,
                     int wantWeight)
{
    const char* allowed =
        "ReDMCSB INVENTORY.C (PC 3.4) AllowedSlots lookup; DEFS.H "
        "C545/C546/C547 pouch/quiver/backpack masks";
    const char* f0302 =
        "ReDMCSB CHAMPION.C F0302 lines 697-710 AllowedSlots & SlotMasks";
    const char* f0297 =
        "ReDMCSB CHAMPION.C F0297/F0298/F0300/F0301 lines 243-298,511-515,606-610";
    char label[128];
    int ok = 1;

    snprintf(label, sizeof(label), "%s pc34 slot", name);
    ok &= expect_int(label, row->pc34Slot, wantPc34Slot, allowed);
    snprintf(label, sizeof(label), "%s storage slot", name);
    ok &= expect_int(label, row->storageSlot, wantStorageSlot, allowed);
    snprintf(label, sizeof(label), "%s slot mask", name);
    ok &= expect_int(label, row->slotMask, wantMask, allowed);
    snprintf(label, sizeof(label), "%s source item", name);
    ok &= expect_int(label, row->sourceItemType, wantItem, f0302);
    snprintf(label, sizeof(label), "%s source allowed slots", name);
    ok &= expect_int(label, row->sourceAllowedSlots, wantAllowedSlots,
                     allowed);
    snprintf(label, sizeof(label), "%s source weight", name);
    ok &= expect_int(label, row->sourceWeight, wantWeight, f0297);
    snprintf(label, sizeof(label), "%s empty destination before", name);
    ok &= expect_int(label, row->slotBefore, 0, f0302);
    snprintf(label, sizeof(label), "%s leader hand before", name);
    ok &= expect_int(label, row->handBefore, wantItem, f0302);
    snprintf(label, sizeof(label), "%s leader hand allowed before", name);
    ok &= expect_int(label, row->handAllowedBefore, wantAllowedSlots,
                     allowed);
    snprintf(label, sizeof(label), "%s load before", name);
    ok &= expect_int(label, row->loadBefore, 0, f0297);
    snprintf(label, sizeof(label), "%s mask overlap", name);
    ok &= expect_int(label, row->maskOverlap, wantAllowedSlots & wantMask,
                     allowed);
    snprintf(label, sizeof(label), "%s can equip before click", name);
    ok &= expect_int(label, row->canEquipBeforeClick, 1, f0302);
    snprintf(label, sizeof(label), "%s accepted click", name);
    ok &= expect_int(label, row->acceptedClick, 1, f0302);
    snprintf(label, sizeof(label), "%s leader hand empty after accepted", name);
    ok &= expect_int(label, row->handAfterAccepted, 0, f0297);
    snprintf(label, sizeof(label), "%s leader hand mask cleared", name);
    ok &= expect_int(label, row->handAllowedAfterAccepted, 0, f0297);
    snprintf(label, sizeof(label), "%s slot after accepted", name);
    ok &= expect_int(label, row->slotAfterAccepted, wantItem, f0297);
    snprintf(label, sizeof(label), "%s slot mask after accepted", name);
    ok &= expect_int(label, row->slotAllowedAfterAccepted, wantAllowedSlots,
                     allowed);
    snprintf(label, sizeof(label), "%s slot weight after accepted", name);
    ok &= expect_int(label, row->slotWeightAfterAccepted, wantWeight, f0297);
    snprintf(label, sizeof(label), "%s load after accepted", name);
    ok &= expect_int(label, row->loadAfterAccepted, wantWeight, f0297);
    snprintf(label, sizeof(label), "%s accepted hand-empty summary", name);
    ok &= expect_int(label, row->handEmptyAfterAccepted, 1, f0297);
    snprintf(label, sizeof(label), "%s accepted slot source summary", name);
    ok &= expect_int(label, row->slotReceivedSource, 1, f0297);
    snprintf(label, sizeof(label), "%s incompatible zero allowed", name);
    ok &= expect_int(label, row->incompatibleAllowedSlots, 0, allowed);
    snprintf(label, sizeof(label), "%s incompatible mask overlap", name);
    ok &= expect_int(label, row->incompatibleMaskOverlap, 0, f0302);
    snprintf(label, sizeof(label), "%s incompatible can equip", name);
    ok &= expect_int(label, row->incompatibleCanEquip, 0, f0302);
    snprintf(label, sizeof(label), "%s incompatible click rejected", name);
    ok &= expect_int(label, row->incompatibleClick, 0, f0302);
    snprintf(label, sizeof(label), "%s incompatible hand preserved", name);
    ok &= expect_int(label, row->incompatibleHandAfter,
                     DM1_V1_IPQBS_INCOMPATIBLE_ITEM, f0302);
    snprintf(label, sizeof(label), "%s incompatible slot unchanged", name);
    ok &= expect_int(label, row->incompatibleSlotAfter, 0, f0302);
    snprintf(label, sizeof(label), "%s incompatible rejected summary", name);
    ok &= expect_int(label, row->incompatibleRejected, 1, f0302);
    return ok;
}

static int test_chest_close(
    const DM1_V1_InventoryPouchQuiverBackpackChestClosePc34* row)
{
    const char* f0333 = "ReDMCSB CHEST.C F0333 lines 53-67";
    const char* f0334 = "ReDMCSB CHEST.C F0334 lines 117-132";
    const char* f0302 =
        "ReDMCSB CHAMPION.C F0302 lines 697-710 hand-to-belt swap";
    const char* f0291 =
        "ReDMCSB CHAMDRAW.C F0291/F0296 lines 551-552,1249-1252";
    const char* f0133 = "ReDMCSB BLITMASK.C F0133 lines 30-33";
    int ok = 1;
    int i;

    ok &= expect_int("chest open result", row->openResult, 1, f0333);
    ok &= expect_int("chest open thing before close",
                     row->openThingBeforeClose, DM1_V1_IPQBS_CHEST_THING,
                     f0333);
    ok &= expect_int("belt swap while chest open", row->beltSwapResult, 1,
                     f0302);
    ok &= expect_int("hand empty after belt swap", row->handAfterBeltSwap, 0,
                     f0302);
    ok &= expect_int("belt slot after swap", row->beltSlotAfterSwap,
                     DM1_V1_IPQBS_POUCH_ITEM, f0302);
    ok &= expect_int("chest close count", row->closeCount,
                     DM1_V1_IPQBS_CHEST_ITEM_COUNT, f0334);
    ok &= expect_int("open thing after close", row->openThingAfterClose, 0,
                     f0334);
    ok &= expect_int("hand after chest close stays empty",
                     row->handAfterClose, 0, f0334);
    ok &= expect_int("belt slot after chest close",
                     row->beltSlotAfterClose, DM1_V1_IPQBS_POUCH_ITEM,
                     f0334);
    for (i = 0; i < DM1_V1_IPQBS_CHEST_ITEM_COUNT; ++i) {
        char label[96];

        snprintf(label, sizeof(label), "closed chest thing %d", i);
        ok &= expect_int(label, row->closedTypes[i],
                         DM1_V1_IPQBS_CHEST_FIRST_ITEM + i, f0334);
    }
    ok &= expect_int("chest kept original items",
                     row->chestKeptOriginalItems, 1, f0334);
    ok &= expect_int("belt item did not enter chest close chain",
                     row->chestDidNotReceiveBeltItem, 1, f0334);
    ok &= expect_int("hand empty after close summary",
                     row->handEmptyAfterClose, 1, f0334);
    ok &= expect_int("C30+ icon blit reruns after swap",
                     row->c30IconBlitRerunAfterSwap, 1, f0291);
    ok &= expect_int("mask blit dispatch acknowledged",
                     row->maskBlitDispatchAcknowledged, 1, f0133);
    return ok;
}

static int test_probe(void)
{
    DM1_V1_InventoryPouchQuiverBackpackSwapProbePc34 probe;
    const char* f0333 = "ReDMCSB CHEST.C F0333 lines 53-67";
    const char* allowed =
        "ReDMCSB INVENTORY.C (PC 3.4) AllowedSlots lookup; DEFS.H "
        "C545/C546/C547 pouch/quiver/backpack masks";
    int ok = 1;

    ok &= expect_int("probe builds",
                     dm1_v1_inventory_pouch_quiver_backpack_swap_probe_pc34(
                         &probe),
                     1, f0333);
    ok &= expect_int("probe contract only", probe.contractOnly, 1, f0333);
    ok &= expect_int("assertion budget", probe.assertionBudget, 80,
                     allowed);
    ok &= test_case("pouch", &probe.pouch, DM1_V1_IPQBS_POUCH_SLOT,
                    DM1_SLOT_POUCH1, DM1_PC34_ALLOWED_POUCH,
                    DM1_V1_IPQBS_POUCH_ITEM, DM1_PC34_ALLOWED_POUCH, 7);
    ok &= test_case("quiver", &probe.quiver, DM1_V1_IPQBS_QUIVER_SLOT,
                    DM1_SLOT_QUIVER1, DM1_PC34_ALLOWED_QUIVER_LINE1,
                    DM1_V1_IPQBS_QUIVER_ITEM,
                    DM1_PC34_ALLOWED_QUIVER_LINE1, 9);
    ok &= test_case("backpack", &probe.backpack,
                    DM1_V1_IPQBS_BACKPACK_SLOT, DM1_SLOT_BACKPACK1,
                    DM1_PC34_ALLOWED_ANY_SLOT, DM1_V1_IPQBS_BACKPACK_ITEM,
                    DM1_PC34_ALLOWED_ANY_SLOT, 11);
    ok &= expect_int("all destination masks allow source",
                     probe.allDestinationMasksAllowSource, 1, allowed);
    ok &= expect_int("all zero-mask incompatible routes rejected",
                     probe.allIncompatibleZeroMaskRoutesRejected, 1,
                     allowed);
    ok &= expect_int("all accepted hands empty",
                     probe.allAcceptedHandsEmpty, 1, allowed);
    ok &= test_chest_close(&probe.chestClose);
    return ok;
}

int main(void)
{
    const char* f0333 = "ReDMCSB CHEST.C F0333 lines 53-67";
    int ok = 1;

    printf("probe=dm1_v1_inventory_pouch_quiver_backpack_swap_pc34_compat\n");
    printf("sourceEvidence=%s\n",
           dm1_v1_inventory_pouch_quiver_backpack_swap_evidence_pc34());
    ok &= test_spec();
    ok &= test_probe();
    ok &= expect_int("minimum assertion count",
                     g_assertions >= 50 ? 1 : 0, 1, f0333);

    printf("assertionCount=%d\n", g_assertions);
    printf("PASS dm1_v1_inventory_pouch_quiver_backpack_swap_pc34_compat "
           "%d/%d assertions\n", g_passes, g_assertions);
    return ok ? 0 : 1;
}
