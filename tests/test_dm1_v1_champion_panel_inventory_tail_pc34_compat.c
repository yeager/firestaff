#include "firestaff/dm1/v1/champion_panel/inventory_tail_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_assertions;
static int g_failures;

static void check_int(int actual, int expected, const char *label,
                      const char *anchor)
{
    ++g_assertions;
    if (actual != expected) {
        ++g_failures;
        printf("FAIL %s actual=%d expected=%d [%s]\n", label, actual,
               expected, anchor ? anchor : "(null)");
    }
}

static void check_contains(const char *haystack, const char *needle,
                           const char *label, const char *anchor)
{
    ++g_assertions;
    if (!haystack || !needle || !strstr(haystack, needle)) {
        ++g_failures;
        printf("FAIL %s missing=%s [%s]\n", label, needle ? needle : "(null)",
               anchor ? anchor : "(null)");
    }
}

static void test_evidence(void)
{
    const Dm1V1ChampionPanelInventoryTailEvidencePc34 *e =
        dm1_v1_champion_panel_inventory_tail_evidence_pc34();
    const char *source =
        dm1_v1_champion_panel_inventory_tail_source_evidence_pc34();

    check_int(e != NULL, 1, "evidence accessor", "inventory_tail");
    check_contains(e->f0296InventoryTailAnchor, "1233-1247",
                   "F0296 inventory tail anchor", e->f0296InventoryTailAnchor);
    check_contains(e->f0295Anchor, "1153-1182", "F0295 anchor",
                   e->f0295Anchor);
    check_contains(e->defsAnchor, "MASK0x4000_VIEWPORT", "viewport mask",
                   e->defsAnchor);
    check_contains(e->defsAnchor, "C08_SLOT_BOX_INVENTORY_FIRST_SLOT",
                   "inventory slotbox base", e->defsAnchor);
    check_contains(e->defsAnchor, "C38_SLOT_BOX_CHEST_FIRST_SLOT",
                   "chest slotbox base", e->defsAnchor);
    check_contains(e->defsAnchor, "M569_PANEL_CHEST", "chest panel",
                   e->defsAnchor);
    check_contains(e->scope, "no pixel parity claim", "contract scope",
                   e->scope);
    check_contains(e->nonOverlap, "champion_panel_hand_slot_refresh",
                   "hand-slot refresh disjoint", e->nonOverlap);
    check_contains(e->nonOverlap, "inventory_panel_status_hand_open_chest_runtime",
                   "runtime hit-zone disjoint", e->nonOverlap);
    check_contains(source, "CHAMDRAW.C F0296_CHAMPION_DrawChangedObjectIcons:1233-1247",
                   "source F0296 text", source);
    check_contains(source, "CHAMDRAW.C F0295_CHAMPION_HasObjectIconInSlotBoxChanged:1153-1182",
                   "source F0295 text", source);
}

static void test_inventory_and_chest_tail(void)
{
    Dm1V1ChampionPanelInventoryTailStatePc34 state;
    Dm1V1ChampionPanelInventoryTailResultPc34 result;

    dm1_v1_champion_panel_inventory_tail_init_pc34(&state);
    check_int(dm1_v1_champion_panel_inventory_tail_run_pc34(&state, &result),
              1, "run accepted", "CHAMDRAW.C F0296:1233-1247");

    check_int(result.accepted, 1, "accepted", "contract-only state");
    check_int(result.sourceAnchorsPresent, 1, "source anchors present",
              "source evidence string");
    check_int(result.inventoryOwnerRequired, 1, "inventory owner required",
              "G0423_i_InventoryChampionOrdinal");
    check_int(result.inventoryOwnerIndexMatchesOrdinal, 1,
              "ordinal/index conversion", "M001_ORDINAL_TO_INDEX");
    check_int(result.inventorySlotScanCount, 30, "inventory scan count",
              "C00..C29 inventory slots");
    check_int(result.inventoryFirstSlotBox, 8, "first inventory slotbox",
              "C08_SLOT_BOX_INVENTORY_FIRST_SLOT");
    check_int(result.inventoryLastSlotBox, 37, "last inventory slotbox",
              "C08 + C30 - 1");
    check_int(result.chestSlotScanCount, 8, "chest scan count",
              "C38..C45 chest slotboxes");
    check_int(result.chestFirstSlotBox, 38, "first chest slotbox",
              "C38_SLOT_BOX_CHEST_FIRST_SLOT");
    check_int(result.chestLastSlotBox, 45, "last chest slotbox",
              "C38 + 8 - 1");
    check_int(result.inventoryChangedCount, 2, "inventory changed count",
              "F0295 inventory changes");
    check_int(result.chestChangedCount, 1, "chest changed count",
              "F0295 chest changes");
    check_int(result.f0038DrawIconInSlotBoxCount, 3, "F0038 count",
              "F0295 changed slots draw icons");
    check_int(result.f0386DrawActionIconCount, 1, "F0386 count",
              "only inventory action hand calls F0386");
    check_int(result.actionHandChangeDispatchesF0386, 1,
              "action hand dispatches F0386", "F0296:1238-1240");
    check_int(result.nonActionInventoryChangeSkipsF0386, 1,
              "non-action inventory change skips F0386", "C01 guard");
    check_int(result.chestChangeSkipsF0386, 1,
              "chest change skips F0386", "F0296:1242-1244");
    check_int(result.viewportMaskSet, DM1_V1_CPIT_MASK0X4000_VIEWPORT_PC34,
              "viewport mask set", "MASK0x4000_VIEWPORT");
    check_int(result.f0292DrawStateCount, 1, "F0292 count",
              "F0296:1245-1247");
    check_int(result.f0292CalledOnceForAnyTailChange, 1,
              "tail change calls F0292 once", "AL0884_B_DrawViewport");
}

static void test_inventory_only_tail(void)
{
    Dm1V1ChampionPanelInventoryTailStatePc34 state;
    Dm1V1ChampionPanelInventoryTailResultPc34 result;

    dm1_v1_champion_panel_inventory_tail_init_pc34(&state);
    state.panelContentIsChest = 0;
    check_int(dm1_v1_champion_panel_inventory_tail_run_pc34(&state, &result),
              1, "run accepted without chest", "M569_PANEL_CHEST guard");
    check_int(result.inventorySlotScanCount, 30, "inventory scan count",
              "inventory tail still scans");
    check_int(result.chestSlotScanCount, 0, "chest scan count",
              "not M569_PANEL_CHEST");
    check_int(result.chestFirstSlotBox, -1, "no first chest slotbox",
              "chest branch skipped");
    check_int(result.chestLastSlotBox, -1, "no last chest slotbox",
              "chest branch skipped");
    check_int(result.inventoryChangedCount, 2, "inventory changes",
              "inventory tail unchanged");
    check_int(result.chestChangedCount, 0, "no chest changes",
              "chest branch skipped");
    check_int(result.f0038DrawIconInSlotBoxCount, 2, "F0038 count",
              "inventory changed slots only");
    check_int(result.f0386DrawActionIconCount, 1, "F0386 count",
              "inventory action hand only");
    check_int(result.viewportMaskSet, DM1_V1_CPIT_MASK0X4000_VIEWPORT_PC34,
              "viewport mask still set", "inventory change triggers F0292");
}

static void test_no_change_skips_viewport_cascade(void)
{
    Dm1V1ChampionPanelInventoryTailStatePc34 state;
    Dm1V1ChampionPanelInventoryTailResultPc34 result;
    int i;

    dm1_v1_champion_panel_inventory_tail_init_pc34(&state);
    for (i = 0; i < DM1_V1_CPIT_INVENTORY_SLOT_COUNT_PC34; ++i) {
        state.inventoryObjectIcon[i] = state.inventoryCurrentIcon[i];
    }
    for (i = 0; i < DM1_V1_CPIT_CHEST_SLOT_COUNT_PC34; ++i) {
        state.chestObjectIcon[i] = state.chestCurrentIcon[i];
    }

    check_int(dm1_v1_champion_panel_inventory_tail_run_pc34(&state, &result),
              1, "run accepted no-change", "F0296 inventory tail");
    check_int(result.inventorySlotScanCount, 30, "inventory scan count",
              "F0295 still scans inventory");
    check_int(result.chestSlotScanCount, 8, "chest scan count",
              "F0295 still scans chest");
    check_int(result.inventoryChangedCount, 0, "inventory changed count",
              "no changed inventory icons");
    check_int(result.chestChangedCount, 0, "chest changed count",
              "no changed chest icons");
    check_int(result.f0038DrawIconInSlotBoxCount, 0, "F0038 count",
              "no F0295 changed slots");
    check_int(result.f0386DrawActionIconCount, 0, "F0386 count",
              "no action-hand change");
    check_int(result.viewportMaskSet, 0, "viewport mask not set",
              "AL0884_B_DrawViewport remains false");
    check_int(result.f0292DrawStateCount, 0, "F0292 not called",
              "F0296:1245 branch skipped");
    check_int(result.noChangeSkipsViewportCascade, 1,
              "no-change skips viewport cascade", "F0296:1245-1247");
}

static void test_rejects(void)
{
    Dm1V1ChampionPanelInventoryTailStatePc34 state;
    Dm1V1ChampionPanelInventoryTailResultPc34 result;

    dm1_v1_champion_panel_inventory_tail_init_pc34(&state);
    check_int(dm1_v1_champion_panel_inventory_tail_run_pc34(NULL, &result), 0,
              "reject null state", "state guard");
    check_int(dm1_v1_champion_panel_inventory_tail_run_pc34(&state, NULL), 0,
              "reject null result", "result guard");

    state.inventoryChampionOrdinal = 0;
    state.inventoryChampionIndex = -1;
    check_int(dm1_v1_champion_panel_inventory_tail_run_pc34(&state, &result),
              0, "reject no inventory owner",
              "tail requires G0423_i_InventoryChampionOrdinal");

    dm1_v1_champion_panel_inventory_tail_init_pc34(&state);
    state.inventoryChampionIndex = 0;
    check_int(dm1_v1_champion_panel_inventory_tail_run_pc34(&state, &result),
              0, "reject ordinal/index mismatch",
              "M001_ORDINAL_TO_INDEX");
}

static void test_hash_deterministic(void)
{
    Dm1V1ChampionPanelInventoryTailStatePc34 state;
    Dm1V1ChampionPanelInventoryTailResultPc34 first;
    Dm1V1ChampionPanelInventoryTailResultPc34 second;

    dm1_v1_champion_panel_inventory_tail_init_pc34(&state);
    check_int(dm1_v1_champion_panel_inventory_tail_run_pc34(&state, &first),
              1, "first run accepted", "determinism");
    dm1_v1_champion_panel_inventory_tail_init_pc34(&state);
    check_int(dm1_v1_champion_panel_inventory_tail_run_pc34(&state, &second),
              1, "second run accepted", "determinism");
    check_int(first.hash == second.hash, 1, "hash deterministic",
              "FNV-1a contract hash");
}

int main(void)
{
    test_evidence();
    test_inventory_and_chest_tail();
    test_inventory_only_tail();
    test_no_change_skips_viewport_cascade();
    test_rejects();
    test_hash_deterministic();

    printf("PASS dm1_v1_champion_panel_inventory_tail_pc34_compat "
           "assertions=%d failures=%d\n",
           g_assertions, g_failures);
    return g_failures == 0 ? 0 : 1;
}
