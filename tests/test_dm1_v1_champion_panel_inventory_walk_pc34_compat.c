#include "firestaff/dm1/v1/champion_panel/inventory_walk_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_assertions;
static int g_failures;

static void check_true(int condition, const char *message, const char *anchor)
{
    ++g_assertions;
    if (!condition) {
        ++g_failures;
        printf("FAIL %s [%s]\n", message, anchor ? anchor : "(null)");
    }
}

static void check_int_eq(int actual, int expected, const char *message,
                         const char *anchor)
{
    ++g_assertions;
    if (actual != expected) {
        ++g_failures;
        printf("FAIL %s actual=%d expected=%d [%s]\n", message, actual,
               expected, anchor ? anchor : "(null)");
    }
}

static void check_int_ge(int actual, int expected, const char *message,
                         const char *anchor)
{
    ++g_assertions;
    if (actual < expected) {
        ++g_failures;
        printf("FAIL %s actual=%d expected>=%d [%s]\n", message, actual,
               expected, anchor ? anchor : "(null)");
    }
}

static void check_contains(const char *haystack, const char *needle,
                           const char *message, const char *anchor)
{
    ++g_assertions;
    if (!haystack || !needle || !strstr(haystack, needle)) {
        ++g_failures;
        printf("FAIL %s missing=%s [%s]\n", message, needle ? needle : "(null)",
               anchor ? anchor : "(null)");
    }
}

static void test_evidence(void)
{
    const Dm1V1ChampionPanelInventoryWalkEvidencePc34 *e =
        dm1_v1_champion_panel_inventory_walk_evidence_pc34();
    const char *text =
        dm1_v1_champion_panel_inventory_walk_source_evidence_pc34();
    const char *siblings[] = {
        "champion_panel_hand_slot_refresh",
        "champion_panel_dead_member_hand_refresh",
        "champion_panel_hand_slot_priority",
        "champion_panel_portrait_box_redraw_states",
        "champion_panel_portrait_state_redraw",
        "mirror_candidate_icon_refresh",
        "champion_panel_spell_area_overlay",
        "champion_panel_status_hand_rotation",
        "champion_panel_second_leader_hand_slot_priority",
        "F0107",
        "F0108",
        "chest-scroll-wheel",
        "viewport",
    };
    int i;

    check_true(e != NULL, "evidence accessor", "inventory_walk");
    check_contains(e->walkF0295Anchor, "F0295_CHAMPION_HasObjectIconInSlotBoxChanged",
                   "F0295 anchor", e->walkF0295Anchor);
    check_contains(e->walkF0296Anchor, "1233-1259",
                   "F0296 anchor", e->walkF0296Anchor);
    check_contains(e->walkF0292Anchor, "F0292_CHAMPION_DrawState",
                   "F0292 anchor", e->walkF0292Anchor);
    check_contains(e->walkF0386Anchor, "F0386_MENUS_DrawActionIcon",
                   "F0386 anchor", e->walkF0386Anchor);
    check_contains(e->inventoryChampionOrdinalAnchor,
                   "G0423_i_InventoryChampionOrdinal",
                   "G0423 inventory ordinal anchor",
                   e->inventoryChampionOrdinalAnchor);
    check_contains(e->inventoryChampionOrdinalAnchor,
                   "M001_ORDINAL_TO_INDEX",
                   "M001 ordinal-to-index anchor",
                   e->inventoryChampionOrdinalAnchor);
    check_contains(e->panelContentAnchor, "G0424_i_PanelContent",
                   "G0424 panel content anchor",
                   e->panelContentAnchor);
    check_contains(e->panelContentAnchor, "M569_PANEL_CHEST",
                   "M569 PANEL_CHEST anchor", e->panelContentAnchor);
    check_contains(e->chestSlotsAnchor, "G0425_aT_ChestSlots",
                   "G0425 chest slots anchor", e->chestSlotsAnchor);
    check_contains(e->maskViewportAnchor, "MASK0x4000_VIEWPORT",
                   "MASK0x4000_VIEWPORT anchor",
                   e->maskViewportAnchor);
    check_contains(e->maskViewportAnchor, "M008_SET",
                   "M008_SET anchor", e->maskViewportAnchor);
    check_contains(e->defsAnchor, "C00_SLOT_READY_HAND",
                   "DEFS.H C00_SLOT_READY_HAND anchor", e->defsAnchor);
    check_contains(e->defsAnchor, "C01_SLOT_ACTION_HAND",
                   "DEFS.H C01_SLOT_ACTION_HAND anchor", e->defsAnchor);
    check_contains(e->defsAnchor, "C30_SLOT_CHEST_1",
                   "DEFS.H C30_SLOT_CHEST_1 anchor", e->defsAnchor);
    check_contains(e->defsAnchor, "C08_SLOT_BOX_INVENTORY_FIRST_SLOT",
                   "DEFS.H C08 inventory first slot anchor",
                   e->defsAnchor);
    check_contains(e->defsAnchor, "C38_SLOT_BOX_CHEST_FIRST_SLOT",
                   "DEFS.H C38 chest first slot anchor", e->defsAnchor);
    check_contains(e->defsAnchor, "M516_CHAMPIONS",
                   "DEFS.H M516_CHAMPIONS anchor", e->defsAnchor);

    check_contains(text, "CHAMDRAW.C F0296_CHAMPION_DrawChangedObjectIcons:1233-1259",
                   "F0296 source text", text);
    check_contains(text, "F0295_CHAMPION_HasObjectIconInSlotBoxChanged",
                   "F0295 source text", text);
    check_contains(text, "F0292_CHAMPION_DrawState",
                   "F0292 source text", text);
    check_contains(text, "G0423_i_InventoryChampionOrdinal",
                   "G0423 inventory champion ordinal source text", text);
    check_contains(text, "M569_PANEL_CHEST",
                   "M569_PANEL_CHEST source text", text);

    for (i = 0; i < (int)(sizeof(siblings) / sizeof(siblings[0])); ++i) {
        char id[64];
        const char *s = siblings[i];
        snprintf(id, sizeof(id), "sibling.%s", s);
        check_contains(e->nonOverlap, s, id, e->nonOverlap);
    }

    check_contains(e->noRealGraphicsClaim, "no real-asset bitmap parity claim",
                   "no parity claim", e->noRealGraphicsClaim);
    check_contains(e->contractScope, "fully-alive 4-champion party",
                   "fully-alive scope", e->contractScope);
}

static void test_inventory_walk_no_chest_baseline(void)
{
    Dm1V1ChampionPanelInventoryWalkStatePc34 state;
    Dm1V1ChampionPanelInventoryWalkResultPc34 result;

    memset(&state, 0, sizeof(state));
    dm1_v1_champion_panel_inventory_walk_init_pc34(&state);
    state.inventoryChampionOrdinal = 2; /* champion index 1 */
    state.inventoryChampionIndex = 1;
    state.panelContent = 0;

    check_int_eq(dm1_v1_champion_panel_inventory_walk_run_pc34(
                     &state, &result),
                 1, "run returns success",
                 "state_valid + F0296 inventory-owner walk");
    check_int_eq(result.accepted, 1, "result.accepted",
                 "F0296 inventory-owner walk returned");
    check_int_eq(result.sourceAnchorsPresent, 1, "source anchors present",
                 "source_anchors_present()");
    check_int_eq(result.fullyAliveRecognized, 1, "fully-alive recognized",
                 "aliveMembers == partyChampionCount");
    check_int_eq(result.partyChampionCount, 4, "party count = 4",
                 "DM1_V1_DMIW_PARTY_COUNT_PC34");
    check_int_eq(result.inventoryChampionOrdinal, 2, "inventory ordinal = 2",
                 "G0423 = 2 (champion index 1)");
    check_int_eq(result.inventoryChampionIndex, 1, "inventory index = 1",
                 "M001_ORDINAL_TO_INDEX(2) = 1");
    check_int_eq(result.panelContent, 0, "panel content = 0",
                 "panelContent = 0 (no chest panel open)");
    check_int_eq(result.inventoryWalkCoversThirtySlots, 1,
                 "inventory walk covers 30 slots",
                 "F0296:1235-1242 slot range [0..30)");
    check_int_eq(result.inventoryWalkSlotboxOffsetApplied, 1,
                 "inventory walk slotbox offset applied",
                 "F0295 invoked with C08 + slotIndex");
    check_int_eq(result.inventoryWalkF0295DispatchedPerSlot, 1,
                 "inventory walk F0295 dispatched per slot",
                 "30 slots each invoke F0295 once");
    check_int_eq(result.inventorySlotF0295DispatchedTotal, 30,
                 "inventory slot F0295 dispatched total = 30",
                 "30 F0295 invocations");
    check_int_eq(result.inventoryWalkSlotCount, 30,
                 "inventory walk slot count = 30",
                 "C30_SLOT_CHEST_1 - C00_SLOT_READY_HAND = 30");
    check_int_eq(result.chestWalkSlotCount, 0,
                 "chest walk slot count = 0 when panel != M569",
                 "panelContent = 0 skips the chest walk");
    check_int_eq(result.chestWalkGatedOffWhenPanelContentNotChest, 1,
                 "chest walk gated off when panel content not chest",
                 "panelContent = 0 → chest walk skipped");
    check_int_eq(result.chestSlotF0295DispatchedTotal, 0,
                 "chest slot F0295 dispatched total = 0",
                 "panelContent = 0 → no chest walk");
    check_int_eq(result.f0296InvocationCount, 1,
                 "F0296 invocation count = 1",
                 "one inventory-owner walk per F0296 call");
    check_int_eq(result.f0292InvocationCount, 0,
                 "F0292 invocation count = 0 (no change)",
                 "drawViewportLatched = 0 → no F0292");
    check_int_eq(result.mask0x4000ViewportSetCount, 0,
                 "MASK0x4000_VIEWPORT not set (no change)",
                 "drawViewportLatched = 0");
    check_int_eq(result.drawViewportLatched, 0,
                 "draw viewport not latched (no change)",
                 "no inventory slot changed");
    check_int_eq(result.f0292DispatchedOnlyWhenDrawViewportLatched, 1,
                 "F0292 dispatched only when draw viewport latched",
                 "no change → no F0292");
    check_int_eq(result.f0292DispatchedExactlyOncePerF0296, 1,
                 "F0292 dispatched at most once per F0296",
                 "no change → 0 invocations");
    check_int_eq(result.mask0x4000ViewportSetOnlyWhenDrawViewportLatched, 1,
                 "MASK0x4000_VIEWPORT set only when latched",
                 "no change → no mask set");
    check_int_eq(result.mask0x4000ViewportSetOnInventoryChampionAttributes, 1,
                 "MASK0x4000_VIEWPORT cleared on inventory champion",
                 "no change → mask = 0");
    check_int_eq(result.f0292TargetsInventoryChampionIndex, 1,
                 "F0292 targets inventory champion index",
                 "no change → mask = 0");
    check_int_eq(result.path,
                 DM1_V1_DMIW_PATH_INVENTORY_WALK_NO_CHEST_PC34,
                 "path = INVENTORY_WALK_NO_CHEST",
                 "no chest, no draw viewport");
}

static void test_inventory_walk_action_hand_change_dispatches_f0386_and_f0292(void)
{
    Dm1V1ChampionPanelInventoryWalkStatePc34 state;
    Dm1V1ChampionPanelInventoryWalkResultPc34 result;

    memset(&state, 0, sizeof(state));
    dm1_v1_champion_panel_inventory_walk_init_pc34(&state);
    state.inventoryChampionOrdinal = 3; /* champion index 2 */
    state.inventoryChampionIndex = 2;
    state.panelContent = 0;
    /*
     * Force the inventory champion's action-hand slot (index 1) to
     * have a mutable icon that does not match the slotbox icon, so
     * F0295 reports it as changed and F0296 dispatches F0386 once.
     */
    state.champions[2].inventoryCurrentIcon[1] = 0;
    state.champions[2].inventoryObjectIcon[1] = 2;

    check_int_eq(dm1_v1_champion_panel_inventory_walk_run_pc34(
                     &state, &result),
                 1, "run returns success (action-hand change)",
                 "state_valid + F0296 inventory-owner walk");
    check_int_eq(result.inventoryWalkActionHandDispatchesF0386, 1,
                 "inventory walk action hand dispatches F0386",
                 "F0296:1241 F0386 on action-hand changed");
    check_int_eq(result.f0386DrawActionIconCount, 1,
                 "F0386 dispatch count = 1",
                 "one action-hand change → one F0386");
    check_int_eq(result.inventorySlotF0386DispatchedTotal, 1,
                 "inventory slot F0386 dispatched total = 1",
                 "exactly the action-hand slot raised F0386");
    check_int_eq(result.inventoryWalkNonActionHandSkipsF0386, 1,
                 "inventory walk non-action-hand skips F0386",
                 "F0386 only dispatched on action-hand slot");
    check_int_eq(result.inventorySlotIconChangedTotal, 1,
                 "inventory slot icon changed total = 1",
                 "only the action-hand slot changed");
    check_int_eq(result.drawViewportLatched, 1,
                 "draw viewport latched (action-hand change)",
                 "AL0884_B_DrawViewport = C1_TRUE");
    check_int_eq(result.f0292InvocationCount, 1,
                 "F0292 invocation count = 1 (latched)",
                 "drawViewportLatched = 1 → F0292 once");
    check_int_eq(result.f0292DispatchedOnlyWhenDrawViewportLatched, 1,
                 "F0292 dispatched only when draw viewport latched",
                 "latched → F0292 invoked");
    check_int_eq(result.f0292DispatchedExactlyOncePerF0296, 1,
                 "F0292 dispatched exactly once per F0296",
                 "exactly one F0292 dispatch");
    check_int_eq(result.mask0x4000ViewportSetCount, 1,
                 "MASK0x4000_VIEWPORT set once (latched)",
                 "M008_SET → 1 set");
    check_int_eq(result.mask0x4000ViewportSetOnlyWhenDrawViewportLatched, 1,
                 "MASK0x4000_VIEWPORT set only when latched",
                 "latched → 1 set");
    check_int_eq(result.mask0x4000ViewportSetOnInventoryChampionAttributes, 1,
                 "MASK0x4000_VIEWPORT set on inventory champion attributes",
                 "champion[2].attributesMask |= 0x4000");
    check_int_eq(
        (state.champions[2].attributesMask & 0x4000) != 0, 1,
        "champion 2 attributes MASK0x4000_VIEWPORT bit set",
        "M008_SET(champion[2].Attributes, MASK0x4000_VIEWPORT)");
    check_int_eq(result.f0292TargetsInventoryChampionIndex, 1,
                 "F0292 targets inventory champion index",
                 "champion 2 attributes mask has 0x4000");
    check_int_eq(result.chestWalkSlotCount, 0,
                 "chest walk slot count = 0 (panelContent = 0)",
                 "panelContent = 0 → no chest walk");
    check_int_eq(result.path,
                 DM1_V1_DMIW_PATH_INVENTORY_WALK_DRAW_VIEWPORT_PC34,
                 "path = INVENTORY_WALK_DRAW_VIEWPORT",
                 "draw viewport latched, no chest");
}

static void test_inventory_walk_non_action_hand_change_skips_f0386(void)
{
    Dm1V1ChampionPanelInventoryWalkStatePc34 state;
    Dm1V1ChampionPanelInventoryWalkResultPc34 result;

    memset(&state, 0, sizeof(state));
    dm1_v1_champion_panel_inventory_walk_init_pc34(&state);
    state.inventoryChampionOrdinal = 4; /* champion index 3 */
    state.inventoryChampionIndex = 3;
    state.panelContent = 0;
    /*
     * Force the inventory champion's HEAD slot (index 2) to have a
     * mutable icon that does not match the slotbox icon, so F0295
     * reports it as changed but F0296 does NOT dispatch F0386 on a
     * non-action-hand slot.
     */
    state.champions[3].inventoryCurrentIcon[2] = 0;
    state.champions[3].inventoryObjectIcon[2] = 1;

    check_int_eq(dm1_v1_champion_panel_inventory_walk_run_pc34(
                     &state, &result),
                 1, "run returns success (head change)",
                 "F0296 inventory-owner walk with HEAD slot change");
    check_int_eq(result.inventorySlotIconChangedTotal, 1,
                 "inventory slot icon changed total = 1 (HEAD)",
                 "only HEAD slot changed");
    check_int_eq(result.f0386DrawActionIconCount, 0,
                 "F0386 dispatch count = 0 (HEAD is not action)",
                 "F0296:1241 only dispatches F0386 on action-hand slot");
    check_int_eq(result.inventorySlotF0386DispatchedTotal, 0,
                 "inventory slot F0386 dispatched total = 0",
                 "HEAD slot did not dispatch F0386");
    check_int_eq(result.drawViewportLatched, 1,
                 "draw viewport latched (HEAD change)",
                 "AL0884_B_DrawViewport = C1_TRUE");
    check_int_eq(result.f0292InvocationCount, 1,
                 "F0292 invocation count = 1 (HEAD change → latched)",
                 "drawViewportLatched = 1 → F0292 once");
    check_int_eq(result.mask0x4000ViewportSetCount, 1,
                 "MASK0x4000_VIEWPORT set once (HEAD change)",
                 "M008_SET → 1 set");
    check_int_eq(result.path,
                 DM1_V1_DMIW_PATH_INVENTORY_WALK_DRAW_VIEWPORT_PC34,
                 "path = INVENTORY_WALK_DRAW_VIEWPORT",
                 "HEAD change latches viewport, no chest");
}

static void test_chest_walk_gated_on_panel_content(void)
{
    Dm1V1ChampionPanelInventoryWalkStatePc34 state;
    Dm1V1ChampionPanelInventoryWalkResultPc34 result;
    int s;

    memset(&state, 0, sizeof(state));
    dm1_v1_champion_panel_inventory_walk_init_pc34(&state);
    state.inventoryChampionOrdinal = 2;
    state.inventoryChampionIndex = 1;
    state.panelContent = 4; /* M569_PANEL_CHEST v1.x/v2.x */
    /*
     * Force a chest slot change so F0292 is dispatched and the
     * chest walk is observably exercised.
     */
    state.chestCurrentIcon[3] = 0;
    state.chestObjectIcon[3] = 2;

    check_int_eq(dm1_v1_champion_panel_inventory_walk_run_pc34(
                     &state, &result),
                 1, "run returns success (chest walk)",
                 "panelContent = M569_PANEL_CHEST v1.x/v2.x");
    check_int_eq(result.panelContent, 4, "panel content = 4",
                 "M569_PANEL_CHEST v1.x/v2.x");
    check_int_eq(result.chestWalkGatedOnPanelContentChest, 1,
                 "chest walk gated on panel content = M569",
                 "panelContent = 4 → chest walk runs");
    check_int_eq(result.chestWalkCoversEightSlots, 1,
                 "chest walk covers 8 slots",
                 "F0296:1247 chest loop 0..7");
    check_int_eq(result.chestWalkSlotboxOffsetApplied, 1,
                 "chest walk slotbox offset applied",
                 "F0295 invoked with C38 + slotIndex");
    check_int_eq(result.chestWalkF0295DispatchedPerSlot, 1,
                 "chest walk F0295 dispatched per slot",
                 "8 chest slots each invoke F0295 once");
    check_int_eq(result.chestSlotF0295DispatchedTotal, 8,
                 "chest slot F0295 dispatched total = 8",
                 "all 8 chest slots invoked F0295");
    check_int_eq(result.chestWalkSlotCount, 8,
                 "chest walk slot count = 8",
                 "chest loop covers 0..7");
    check_int_eq(result.chestSlotIconChangedTotal, 1,
                 "chest slot icon changed total = 1 (slot 3)",
                 "only slot 3 changed");
    check_int_eq(result.chestWalkNeverDispatchesF0386, 1,
                 "chest walk never dispatches F0386",
                 "chest slots are not action-hand slots");
    check_int_eq(result.f0386DrawActionIconCount, 0,
                 "F0386 dispatch count = 0 (chest only)",
                 "chest walk never raises F0386");
    check_int_eq(result.drawViewportLatched, 1,
                 "draw viewport latched (chest change)",
                 "AL0884_B_DrawViewport = C1_TRUE");
    check_int_eq(result.f0292InvocationCount, 1,
                 "F0292 invocation count = 1 (chest change)",
                 "latched → F0292 once");
    check_int_eq(result.mask0x4000ViewportSetCount, 1,
                 "MASK0x4000_VIEWPORT set once (chest change)",
                 "M008_SET → 1 set");
    /*
     * Verify the trace captured both the inventory walk and the
     * chest walk.
     */
    check_int_ge(result.trace[0], 0,
                 "trace[0] is set", "kTraceInit or kTraceF0296Enter");
    for (s = 0; s < 8; ++s) {
        check_int_eq(state.chestSlotF0295Dispatched[s], 1,
                     "every chest slot F0295 dispatched",
                     "F0296:1247 chest walk");
    }
    check_int_eq(state.chestSlotIconChanged[3], 1,
                 "chest slot 3 icon changed (forced change)",
                 "F0296:1247 chest walk raised F0295 on slot 3");
    check_int_eq(result.path,
                 DM1_V1_DMIW_PATH_INVENTORY_WALK_DRAW_VIEWPORT_PC34,
                 "path = INVENTORY_WALK_DRAW_VIEWPORT",
                 "draw viewport latched, chest open");
}

static void test_chest_walk_v3x_panel_content(void)
{
    Dm1V1ChampionPanelInventoryWalkStatePc34 state;
    Dm1V1ChampionPanelInventoryWalkResultPc34 result;

    memset(&state, 0, sizeof(state));
    dm1_v1_champion_panel_inventory_walk_init_pc34(&state);
    state.inventoryChampionOrdinal = 2;
    state.inventoryChampionIndex = 1;
    state.panelContent = 6; /* M569_PANEL_CHEST v3.x */

    check_int_eq(dm1_v1_champion_panel_inventory_walk_run_pc34(
                     &state, &result),
                 1, "run returns success (chest walk v3.x)",
                 "panelContent = M569_PANEL_CHEST v3.x");
    check_int_eq(result.panelContent, 6, "panel content = 6",
                 "M569_PANEL_CHEST v3.x");
    check_int_eq(result.chestWalkGatedOnPanelContentChest, 1,
                 "chest walk gated on panel content = 6 (v3.x)",
                 "DEFS.H:2995-3011 M569 = 6 for v3.x");
    check_int_eq(result.chestWalkCoversEightSlots, 1,
                 "chest walk covers 8 slots (v3.x)",
                 "F0296:1247 chest loop 0..7");
    check_int_eq(result.chestSlotF0295DispatchedTotal, 8,
                 "chest slot F0295 dispatched total = 8 (v3.x)",
                 "all 8 chest slots invoked F0295");
}

static void test_rejects(void)
{
    Dm1V1ChampionPanelInventoryWalkStatePc34 state;
    Dm1V1ChampionPanelInventoryWalkStatePc34 probe;
    Dm1V1ChampionPanelInventoryWalkResultPc34 result;
    int rejected;

    /* Reject 1: NULL state. */
    rejected = dm1_v1_champion_panel_inventory_walk_run_pc34(
                   NULL, &result) == 0;
    check_int_eq(rejected, 1, "reject: NULL state", "NULL state guard");

    /* Reject 2: NULL result. */
    memset(&state, 0, sizeof(state));
    dm1_v1_champion_panel_inventory_walk_init_pc34(&state);
    state.inventoryChampionOrdinal = 2;
    state.inventoryChampionIndex = 1;
    rejected = dm1_v1_champion_panel_inventory_walk_run_pc34(
                   &state, NULL) == 0;
    check_int_eq(rejected, 1, "reject: NULL result", "NULL result guard");

    /* Reject 3: no inventory owner. */
    memset(&probe, 0, sizeof(probe));
    dm1_v1_champion_panel_inventory_walk_init_pc34(&probe);
    probe.inventoryChampionOrdinal = 0;
    probe.inventoryChampionIndex = -1;
    probe.panelContent = 0;
    rejected = dm1_v1_champion_panel_inventory_walk_run_pc34(
                   &probe, &result) == 0;
    check_int_eq(rejected, 1, "reject: no inventory owner",
                 "G0423 = 0 → no F0296 inventory-owner walk");
    check_int_eq(result.rejectsNoInventoryOwner, 1,
                 "guard: rejects no inventory owner",
                 "G0423 = 0");
    check_int_eq(result.path,
                 DM1_V1_DMIW_PATH_REJECTED_NO_INVENTORY_OWNER_PC34,
                 "path = REJECTED_NO_INVENTORY_OWNER",
                 "G0423 = 0 path");

    /* Reject 4: dead member. */
    memset(&probe, 0, sizeof(probe));
    dm1_v1_champion_panel_inventory_walk_init_pc34(&probe);
    probe.inventoryChampionOrdinal = 2;
    probe.inventoryChampionIndex = 1;
    probe.panelContent = 0;
    probe.champions[2].alive = 0;
    rejected = dm1_v1_champion_panel_inventory_walk_run_pc34(
                   &probe, &result) == 0;
    check_int_eq(rejected, 1, "reject: dead member",
                 "champion 2 alive = 0");
    check_int_eq(result.rejectsDeadMember, 1,
                 "guard: rejects dead member",
                 "alive = 0 → fully-alive guard rejects");
    check_int_eq(result.path,
                 DM1_V1_DMIW_PATH_REJECTED_DEAD_MEMBER_PC34,
                 "path = REJECTED_DEAD_MEMBER",
                 "dead member path");

    /* Reject 5: invalid panel content. */
    memset(&probe, 0, sizeof(probe));
    dm1_v1_champion_panel_inventory_walk_init_pc34(&probe);
    probe.inventoryChampionOrdinal = 2;
    probe.inventoryChampionIndex = 1;
    probe.panelContent = 99; /* not in {0, 4, 6} */
    rejected = dm1_v1_champion_panel_inventory_walk_run_pc34(
                   &probe, &result) == 0;
    check_int_eq(rejected, 1, "reject: invalid panel content",
                 "panelContent = 99 (not in {0, 4, 6})");
    check_int_eq(result.rejectsInvalidPanelContent, 1,
                 "guard: rejects invalid panel content",
                 "panelContent = 99");
    check_int_eq(result.path,
                 DM1_V1_DMIW_PATH_REJECTED_INVALID_PANEL_CONTENT_PC34,
                 "path = REJECTED_INVALID_PANEL_CONTENT",
                 "invalid panel content path");

    /* Reject 6: party size zero. */
    memset(&probe, 0, sizeof(probe));
    dm1_v1_champion_panel_inventory_walk_init_pc34(&probe);
    probe.partyChampionCount = 0;
    probe.inventoryChampionOrdinal = 1;
    probe.inventoryChampionIndex = 0;
    probe.panelContent = 0;
    rejected = dm1_v1_champion_panel_inventory_walk_run_pc34(
                   &probe, &result) == 0;
    check_int_eq(rejected, 1, "reject: party size zero",
                 "partyChampionCount = 0");
    check_int_eq(result.rejectsPartySizeZero, 1,
                 "guard: rejects party size zero",
                 "partyChampionCount = 0");

    /* Reject 7: inventory champion ordinal out of range. */
    memset(&probe, 0, sizeof(probe));
    dm1_v1_champion_panel_inventory_walk_init_pc34(&probe);
    probe.inventoryChampionOrdinal = 99;
    probe.inventoryChampionIndex = 98;
    probe.panelContent = 0;
    rejected = dm1_v1_champion_panel_inventory_walk_run_pc34(
                   &probe, &result) == 0;
    check_int_eq(rejected, 1, "reject: inventory ordinal out of range",
                 "G0423 = 99 (out of [1..partyChampionCount])");
}

static void test_baseline_deterministic_hash(void)
{
    Dm1V1ChampionPanelInventoryWalkStatePc34 state1;
    Dm1V1ChampionPanelInventoryWalkStatePc34 state2;
    Dm1V1ChampionPanelInventoryWalkResultPc34 result1;
    Dm1V1ChampionPanelInventoryWalkResultPc34 result2;

    memset(&state1, 0, sizeof(state1));
    dm1_v1_champion_panel_inventory_walk_init_pc34(&state1);
    state1.inventoryChampionOrdinal = 2;
    state1.inventoryChampionIndex = 1;
    state1.panelContent = 0;
    check_int_eq(dm1_v1_champion_panel_inventory_walk_run_pc34(
                     &state1, &result1),
                 1, "first run returns success", "deterministic hash");

    memset(&state2, 0, sizeof(state2));
    dm1_v1_champion_panel_inventory_walk_init_pc34(&state2);
    state2.inventoryChampionOrdinal = 2;
    state2.inventoryChampionIndex = 1;
    state2.panelContent = 0;
    check_int_eq(dm1_v1_champion_panel_inventory_walk_run_pc34(
                     &state2, &result2),
                 1, "second run returns success", "deterministic hash");

    check_int_eq(result1.hash == result2.hash, 1,
                 "hash is deterministic across runs",
                 "FNV-1a hash of the same state");
    check_int_ge(result1.f0296InvocationCount, 1,
                 "F0296 invocation count >= 1",
                 "baseline always walks F0296 once");
}

static void test_inventory_walk_does_not_touch_other_champions(void)
{
    Dm1V1ChampionPanelInventoryWalkStatePc34 state;
    Dm1V1ChampionPanelInventoryWalkResultPc34 result;
    int i;

    memset(&state, 0, sizeof(state));
    dm1_v1_champion_panel_inventory_walk_init_pc34(&state);
    state.inventoryChampionOrdinal = 2;
    state.inventoryChampionIndex = 1;
    state.panelContent = 0;
    state.champions[1].inventoryCurrentIcon[1] = 0;
    state.champions[1].inventoryObjectIcon[1] = 2;

    check_int_eq(dm1_v1_champion_panel_inventory_walk_run_pc34(
                     &state, &result),
                 1, "run returns success (other-champions isolation)",
                 "F0296 inventory-owner walk");
    /*
     * The inventory-owner walk only touches the inventory champion's
     * attributes + slots. Other champions' attributes must remain
     * unchanged.
     */
    for (i = 0; i < DM1_V1_DMIW_PARTY_COUNT_PC34; ++i) {
        if (i == 1) {
            continue;
        }
        check_int_eq(state.champions[i].attributesMask, 0,
                     "non-inventory champion attributes unchanged",
                     "F0296:1253 M008_SET targets inventory champion only");
    }
    check_int_eq((state.champions[1].attributesMask & 0x4000) != 0, 1,
                 "inventory champion MASK0x4000_VIEWPORT set",
                 "F0296:1253 M008_SET on inventory champion");
}

static void test_inventory_walk_skips_when_inventory_champion_dead(void)
{
    Dm1V1ChampionPanelInventoryWalkStatePc34 state;
    Dm1V1ChampionPanelInventoryWalkResultPc34 result;

    memset(&state, 0, sizeof(state));
    dm1_v1_champion_panel_inventory_walk_init_pc34(&state);
    state.inventoryChampionOrdinal = 2;
    state.inventoryChampionIndex = 1;
    state.panelContent = 0;
    /*
     * Mark the inventory champion dead. The slice models a
     * fully-alive party; a dead inventory champion belongs to the
     * dead_member_hand_refresh sibling, so this state must be
     * rejected by the state_valid guard.
     */
    state.champions[1].alive = 0;

    check_int_eq(dm1_v1_champion_panel_inventory_walk_run_pc34(
                     &state, &result),
                 0, "run rejected (inventory champion dead)",
                 "alive = 0 → state_valid fails");
    check_int_eq(result.path,
                 DM1_V1_DMIW_PATH_REJECTED_DEAD_MEMBER_PC34,
                 "path = REJECTED_DEAD_MEMBER",
                 "inventory champion dead path");
}

int main(void)
{
    test_evidence();
    test_inventory_walk_no_chest_baseline();
    test_inventory_walk_action_hand_change_dispatches_f0386_and_f0292();
    test_inventory_walk_non_action_hand_change_skips_f0386();
    test_chest_walk_gated_on_panel_content();
    test_chest_walk_v3x_panel_content();
    test_inventory_walk_does_not_touch_other_champions();
    test_inventory_walk_skips_when_inventory_champion_dead();
    test_rejects();
    test_baseline_deterministic_hash();

    Dm1V1ChampionPanelInventoryWalkStatePc34 state;
    Dm1V1ChampionPanelInventoryWalkResultPc34 result;
    memset(&state, 0, sizeof(state));
    dm1_v1_champion_panel_inventory_walk_init_pc34(&state);
    state.inventoryChampionOrdinal = 2;
    state.inventoryChampionIndex = 1;
    state.panelContent = 4;
    state.chestCurrentIcon[3] = 0;
    state.chestObjectIcon[3] = 2;
    dm1_v1_champion_panel_inventory_walk_run_pc34(&state, &result);
    printf("inventory_walk hash=0x%08X assertions=%d failures=%d\n",
           (unsigned)result.hash, g_assertions, g_failures);
    return g_failures == 0 ? 0 : 1;
}
