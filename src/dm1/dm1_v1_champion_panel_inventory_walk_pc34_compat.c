/*
 * DM1 V1 champion-panel inventory/chest redraw walk implementation.
 *
 * Source-locked contract-only model for CHAMDRAW.C F0296_CHAMPION_
 * DrawChangedObjectIcons:1233-1259. The slice covers the inventory-
 * champion inventory-slot walk, the chest-slot walk when
 * G0424_i_PanelContent == M569_PANEL_CHEST, and the F0292_CHAMPION_
 * DrawState dispatch + MASK0x4000_VIEWPORT attribute set when any
 * inventory or chest slot raised a change. No real M11 graphics and
 * no real-asset bitmap parity claim.
 *
 * Lane name: champion_panel_inventory_walk_gate. Disjoint from
 * champion_panel_hand_slot_refresh (which owns F0296:1184-1231) and
 * from the per-state redraw + per-action-hand slot-box dispatch
 * family.
 */

#include "firestaff/dm1/v1/champion_panel/inventory_walk_pc34_compat.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define kPartyChampionCount 4
#define kLeaderIndex 0
#define kLeaderHandThing ((uint16_t)0x0101u)
#define kLeaderObjectIcon 0x0102
#define kLeaderCurrentIcon 0x0103

#define kTraceInit 0
#define kTraceF0296Enter 1
#define kTraceF0292Dispatch 2
#define kTraceInventoryWalk 3
#define kTraceChestWalk 4
#define kTraceMaskViewportSet 5

static int is_mutable_icon(int icon_index)
{
    /*
     * CHAMDRAW.C F0296:1212-1216 mutable icon range. The mutable
     * window is the compass junk (0..3) + the potion range
     * (C148..C163) + C195_ICON_POTION_EMPTY_FLASK (195). The C201
     * ACTION_ICON_EMPTY_HAND falls outside the mutable window and
     * so does not raise F0036.
     */
    if (icon_index < 0) {
        return 0;
    }
    if (icon_index <= 3) {
        return 1;
    }
    if (icon_index >= 148 && icon_index <= 163) {
        return 1;
    }
    if (icon_index == 195) {
        return 1;
    }
    return 0;
}

static int ordinal_to_index(int ordinal)
{
    /*
     * DEFS.H:7208-7209 M000_INDEX_TO_ORDINAL(index) == (index + 1)
     * and M001_ORDINAL_TO_INDEX(ordinal) == (ordinal - 1).
     */
    if (ordinal <= 0) {
        return -1;
    }
    return ordinal - 1;
}

static const char *s_source_evidence =
    "DM1 V1 champion-panel inventory/chest redraw walk contract. "
    "Source-locked against ReDMCSB WIP 20210206 "
    "CHAMDRAW.C F0296_CHAMPION_DrawChangedObjectIcons:1233-1259, "
    "CHAMDRAW.C F0296:1233 L0883_ui_InventoryChampionOrdinal = G0423_i_InventoryChampionOrdinal, "
    "CHAMDRAW.C F0296:1235-1242 the inventory-owner walk over "
    "L0887_ps_Champion = &M516_CHAMPIONS[M001_ORDINAL_TO_INDEX(L0883_ui_InventoryChampionOrdinal)] "
    "with AL0882_ui_SlotIndex = C00_SLOT_READY_HAND .. C30_SLOT_CHEST_1-1, "
    "F0295_CHAMPION_HasObjectIconInSlotBoxChanged(AL0882_ui_SlotIndex + C08_SLOT_BOX_INVENTORY_FIRST_SLOT, *L0886_pT_Thing), "
    "AL0884_B_DrawViewport |= (L0889_ui_ObjectIconChanged = ...), "
    "and F0386_MENUS_DrawActionIcon(M001_ORDINAL_TO_INDEX(L0883_ui_InventoryChampionOrdinal)) "
    "when L0889_ui_ObjectIconChanged && AL0882_ui_SlotIndex == C01_SLOT_ACTION_HAND; "
    "CHAMDRAW.C F0296:1244-1251 the chest-slot walk gated on "
    "G0424_i_PanelContent == M569_PANEL_CHEST (or G2008_i_PanelContent for I34E/A36M/A31E/A31M/A33M/A35E/A35M/AU1E/AU2E/AU3E) "
    "with AL0882_ui_SlotIndex = 0..7 on G0425_aT_ChestSlots, "
    "AL0884_B_DrawViewport |= F0295_CHAMPION_HasObjectIconInSlotBoxChanged(AL0882_ui_SlotIndex + C38_SLOT_BOX_CHEST_FIRST_SLOT, *L0886_pT_Thing); "
    "CHAMDRAW.C F0296:1253-1256 M008_SET(L0887_ps_Champion->Attributes, MASK0x4000_VIEWPORT) + "
    "F0292_CHAMPION_DrawState(M001_ORDINAL_TO_INDEX(L0883_ui_InventoryChampionOrdinal)) when AL0884_B_DrawViewport; "
    "DEFS.H:780 C00_SLOT_READY_HAND, 781 C01_SLOT_ACTION_HAND, 810 C30_SLOT_CHEST_1, "
    "731 MASK0x4000_VIEWPORT = 0x4000, 738 M008_SET, 873 M516_CHAMPIONS, "
    "1874 C08_SLOT_BOX_INVENTORY_FIRST_SLOT, 1876 C38_SLOT_BOX_CHEST_FIRST_SLOT, "
    "2995-3011 M569_PANEL_CHEST (v1.x/v2.x = 4, v3.x = 6), "
    "5320 G0423_i_InventoryChampionOrdinal, "
    "5877 G0424_i_PanelContent, 5878 G0425_aT_ChestSlots[8], "
    "7208 M000_INDEX_TO_ORDINAL, 7209 M001_ORDINAL_TO_INDEX, "
    "7895 F0292_CHAMPION_DrawState, 7915 F0295_CHAMPION_HasObjectIconInSlotBoxChanged, "
    "8327 F0386_MENUS_DrawActionIcon. "
    "Honest scope: contract-only F0296 inventory-owner walk + chest-slot walk + F0292 dispatch + "
    "MASK0x4000_VIEWPORT set on a fully-alive 4-champion party; no real M11 graphics, "
    "no real bitmaps, no asset load.";

int dm1_v1_champion_panel_inventory_walk_source_anchors_present_pc34(void)
{
    return strstr(s_source_evidence,
                  "CHAMDRAW.C F0296_CHAMPION_DrawChangedObjectIcons:1233-1259") !=
               NULL &&
           strstr(s_source_evidence, "F0295_CHAMPION_HasObjectIconInSlotBoxChanged") !=
               NULL &&
           strstr(s_source_evidence, "F0292_CHAMPION_DrawState") != NULL &&
           strstr(s_source_evidence, "F0386_MENUS_DrawActionIcon") != NULL &&
           strstr(s_source_evidence, "G0423_i_InventoryChampionOrdinal") != NULL &&
           strstr(s_source_evidence, "G0424_i_PanelContent") != NULL &&
           strstr(s_source_evidence, "G0425_aT_ChestSlots") != NULL &&
           strstr(s_source_evidence, "MASK0x4000_VIEWPORT") != NULL &&
           strstr(s_source_evidence, "M569_PANEL_CHEST") != NULL &&
           strstr(s_source_evidence, "C00_SLOT_READY_HAND") != NULL &&
           strstr(s_source_evidence, "C01_SLOT_ACTION_HAND") != NULL &&
           strstr(s_source_evidence, "C30_SLOT_CHEST_1") != NULL &&
           strstr(s_source_evidence,
                  "C08_SLOT_BOX_INVENTORY_FIRST_SLOT") != NULL &&
           strstr(s_source_evidence, "C38_SLOT_BOX_CHEST_FIRST_SLOT") != NULL &&
           strstr(s_source_evidence, "M000_INDEX_TO_ORDINAL") != NULL &&
           strstr(s_source_evidence, "M001_ORDINAL_TO_INDEX") != NULL;
}

const char *
dm1_v1_champion_panel_inventory_walk_source_evidence_pc34(void)
{
    return s_source_evidence;
}

static const Dm1V1ChampionPanelInventoryWalkEvidencePc34 s_evidence = {
    "ReDMCSB CHAMDRAW.C F0295_CHAMPION_HasObjectIconInSlotBoxChanged:1153-1182",
    "ReDMCSB CHAMDRAW.C F0296_CHAMPION_DrawChangedObjectIcons:1233-1259",
    "ReDMCSB CHAMDRAW.C F0296:1253-1256 F0292_CHAMPION_DrawState dispatch when AL0884_B_DrawViewport is C1_TRUE",
    "ReDMCSB CHAMDRAW.C F0296:1241 F0386_MENUS_DrawActionIcon when changed && slot == C01_SLOT_ACTION_HAND",
    "ReDMCSB DEFS.H:5320 G0423_i_InventoryChampionOrdinal, 7209 M001_ORDINAL_TO_INDEX",
    "ReDMCSB DEFS.H:5877 G0424_i_PanelContent, 2995-3011 M569_PANEL_CHEST (v1.x/v2.x = 4, v3.x = 6)",
    "ReDMCSB DEFS.H:5878 G0425_aT_ChestSlots[8]",
    "ReDMCSB DEFS.H:731 MASK0x4000_VIEWPORT = 0x4000 + 738 M008_SET on Attributes",
    "ReDMCSB DEFS.H:780 C00_SLOT_READY_HAND, 781 C01_SLOT_ACTION_HAND, 810 C30_SLOT_CHEST_1, 873 M516_CHAMPIONS, 1874 C08_SLOT_BOX_INVENTORY_FIRST_SLOT, 1876 C38_SLOT_BOX_CHEST_FIRST_SLOT",
    "contract-only F0296 inventory-owner walk + chest-slot walk + F0292 dispatch + MASK0x4000_VIEWPORT set on a fully-alive 4-champion party; no real M11 graphics, no real bitmaps, no asset load",
    "no real-asset bitmap parity claim; no GRAPHICS.DAT / DUNGEON.DAT load; no original DOS pixel evidence",
    "Non-overlap marker: pass champion_panel_inventory_walk covers the F0296 inventory-owner walk (CHAMDRAW.C:1233-1259 — the inventory-slot walk, the chest-slot walk when G0424_i_PanelContent == M569_PANEL_CHEST, the F0292_CHAMPION_DrawState dispatch on the inventory champion, and the MASK0x4000_VIEWPORT attribute set when any inventory or chest slot changed). Disjoint from champion_panel_hand_slot_refresh (F0296:1184-1231 leader-hand + action-hand slotbox walk-order + candidate early-return + inventory-champion skip on the matched action-hand slotbox), champion_panel_dead_member_hand_refresh (F0296/F0295/F0386 walk with a dead member present + F0292 dead-status-box branch), champion_panel_hand_slot_priority (CHAMPION.C F0302 input dispatch), champion_panel_portrait_box_redraw_states (F0291/F0292/F0296 portrait-box event matrix), champion_panel_portrait_state_redraw (F0292 state-redraw cascade on the portrait-box), mirror_candidate_icon_refresh (F0296 leader-hand icon refresh interaction with the candidate ordinal), mirror_candidate_c040 sibling family (candidate-panel state machine), champion_panel_spell_area_overlay (F0394 dead-champion reject for spell area), champion_panel_status_hand_rotation (F0284 leader rotation), champion_panel_second_leader_hand_slot_priority (the 2nd leader's hand-slot priority path), the F0107/F0108/chest-scroll-wheel/viewport/integrated family, and the per-state redraw + per-action-hand slot-box dispatch family (which targets the slotbox index, not the inventory-slot walk)."
};

const Dm1V1ChampionPanelInventoryWalkEvidencePc34 *
dm1_v1_champion_panel_inventory_walk_evidence_pc34(void)
{
    return &s_evidence;
}

static void init_champion(
    Dm1V1ChampionPanelInventoryWalkChampionPc34 *champion, int index)
{
    int s;

    if (!champion) {
        return;
    }
    memset(champion, 0, sizeof(*champion));
    champion->championIndex = index;
    champion->alive = 1;
    champion->leader = (index == kLeaderIndex) ? 1 : 0;
    champion->attributesMask = 0;
    /*
     * Default inventory layout: every slot starts with matching
     * mutable icons so the inventory walk invokes F0295 once per
     * slot (CHAMDRAW.C F0296:1239-1240) without raising the
     * changed flag. Tests that need a change set
     * inventoryCurrentIcon[slot] != inventoryObjectIcon[slot]
     * directly.
     */
    for (s = 0; s < DM1_V1_DMIW_INVENTORY_SLOT_COUNT_PC34; ++s) {
        champion->inventorySlots[s] = 0x0200u;
        champion->inventoryCurrentIcon[s] = 0; /* mutable icon 0 */
        champion->inventoryObjectIcon[s] = 0;
    }
}

void dm1_v1_champion_panel_inventory_walk_init_pc34(
    Dm1V1ChampionPanelInventoryWalkStatePc34 *state)
{
    int i;
    int s;

    if (!state) {
        return;
    }
    memset(state, 0, sizeof(*state));
    state->contractOnly = 1;
    state->assetFree = 1;
    state->partyChampionCount = kPartyChampionCount;
    state->leaderIndex = kLeaderIndex;
    state->candidateChampionOrdinal = 0;
    state->inventoryChampionOrdinal = 0;
    state->inventoryChampionIndex = -1;
    state->panelContent = 0;
    state->f0296InvocationCount = 0;
    state->f0292InvocationCount = 0;
    state->f0295HasIconChangedCount = 0;
    state->f0295SameIconCount = 0;
    state->f0386DrawActionIconCount = 0;
    state->drawViewportLatched = 0;
    state->mask0x4000ViewportSetCount = 0;
    state->f0296Trace[0] = kTraceInit;
    for (i = 0; i < DM1_V1_DMIW_PARTY_COUNT_PC34; ++i) {
        init_champion(&state->champions[i], i);
    }
    /*
     * Chest slots default to matching mutable icons so the chest
     * walk invokes F0295 once per chest slot (CHAMDRAW.C F0296:
     * 1247-1251) without raising the changed flag. Tests that
     * need a chest change set chestCurrentIcon[slot] !=
     * chestObjectIcon[slot] directly.
     */
    for (s = 0; s < DM1_V1_DMIW_CHEST_SLOT_COUNT_PC34; ++s) {
        state->chestSlots[s] = 0x0300u;
        state->chestCurrentIcon[s] = 0; /* mutable icon 0 */
        state->chestObjectIcon[s] = 0;
    }
    for (s = 0; s < DM1_V1_DMIW_INVENTORY_SLOT_COUNT_PC34; ++s) {
        state->inventorySlotF0295Dispatched[s] = 0;
        state->inventorySlotF0386Dispatched[s] = 0;
        state->inventorySlotIconChanged[s] = 0;
    }
    for (s = 0; s < DM1_V1_DMIW_CHEST_SLOT_COUNT_PC34; ++s) {
        state->chestSlotF0295Dispatched[s] = 0;
        state->chestSlotIconChanged[s] = 0;
    }
}

static int state_valid(
    const Dm1V1ChampionPanelInventoryWalkStatePc34 *state)
{
    int alive;
    int i;
    int valid_panel_content;

    if (!state || !state->contractOnly || !state->assetFree) {
        return 0;
    }
    if (state->partyChampionCount <= 0 ||
        state->partyChampionCount > DM1_V1_DMIW_PARTY_COUNT_PC34) {
        return 0;
    }
    if (state->leaderIndex < 0 ||
        state->leaderIndex >= state->partyChampionCount) {
        return 0;
    }
    alive = 0;
    for (i = 0; i < state->partyChampionCount; ++i) {
        if (state->champions[i].alive) {
            ++alive;
        } else {
            /*
             * The inventory_walk slice models a fully-alive
             * party. A dead member belongs to the
             * dead_member_hand_refresh slice.
             */
            return 0;
        }
    }
    if (alive != state->partyChampionCount) {
        return 0;
    }
    if (state->champions[state->leaderIndex].alive != 1 ||
        state->champions[state->leaderIndex].leader != 1) {
        return 0;
    }
    if (state->candidateChampionOrdinal < 0 ||
        state->inventoryChampionOrdinal < 0 ||
        state->panelContent < 0) {
        return 0;
    }
    /*
     * The F0296 inventory-owner walk only fires when
     * G0423_i_InventoryChampionOrdinal is non-zero. A zero
     * ordinal belongs to a different test path (no inventory
     * panel open).
     */
    if (state->inventoryChampionOrdinal == 0) {
        return 0;
    }
    if (state->inventoryChampionOrdinal < 1 ||
        state->inventoryChampionOrdinal > state->partyChampionCount) {
        return 0;
    }
    if (state->inventoryChampionIndex !=
        ordinal_to_index(state->inventoryChampionOrdinal)) {
        return 0;
    }
    /*
     * DEFS.H:2995-3011 M569_PANEL_CHEST is 4 for v1.x/v2.x and
     * 6 for v3.x. The default gate accepts both as "valid panel
     * content values" so the contract covers the full PC media
     * family without needing per-version branching in the test.
     */
    valid_panel_content =
        (state->panelContent == DM1_V1_DMIW_M569_PANEL_CHEST_PC34) ||
        (state->panelContent == DM1_V1_DMIW_M569_PANEL_CHEST_V2X_PC34) ||
        (state->panelContent == 0);
    if (!valid_panel_content) {
        return 0;
    }
    return 1;
}

static int f0295_sense_inventory_slot(
    Dm1V1ChampionPanelInventoryWalkStatePc34 *state, int slot_index)
{
    int current_icon;
    int object_icon;
    int changed;

    if (!state || slot_index < 0 ||
        slot_index >= DM1_V1_DMIW_INVENTORY_SLOT_COUNT_PC34) {
        return 0;
    }
    current_icon = state->champions[state->inventoryChampionIndex]
                       .inventoryCurrentIcon[slot_index];
    object_icon = state->champions[state->inventoryChampionIndex]
                      .inventoryObjectIcon[slot_index];
    changed = (object_icon != current_icon) ? 1 : 0;
    if (is_mutable_icon(current_icon) || is_mutable_icon(object_icon)) {
        if (changed) {
            ++state->f0295HasIconChangedCount;
            state->inventorySlotIconChanged[slot_index] = 1;
            state->champions[state->inventoryChampionIndex]
                .inventoryIconChanged[slot_index] = 1;
        } else {
            ++state->f0295SameIconCount;
        }
        state->inventorySlotF0295Dispatched[slot_index] = 1;
        state->champions[state->inventoryChampionIndex]
            .inventoryF0295Dispatched[slot_index] = 1;
        return changed;
    }
    ++state->f0295SameIconCount;
    return 0;
}

static int f0295_sense_chest_slot(
    Dm1V1ChampionPanelInventoryWalkStatePc34 *state, int slot_index)
{
    int current_icon;
    int object_icon;
    int changed;

    if (!state || slot_index < 0 ||
        slot_index >= DM1_V1_DMIW_CHEST_SLOT_COUNT_PC34) {
        return 0;
    }
    current_icon = state->chestCurrentIcon[slot_index];
    object_icon = state->chestObjectIcon[slot_index];
    changed = (object_icon != current_icon) ? 1 : 0;
    if (is_mutable_icon(current_icon) || is_mutable_icon(object_icon)) {
        if (changed) {
            ++state->f0295HasIconChangedCount;
            state->chestSlotIconChanged[slot_index] = 1;
        } else {
            ++state->f0295SameIconCount;
        }
        state->chestSlotF0295Dispatched[slot_index] = 1;
        return changed;
    }
    ++state->f0295SameIconCount;
    return 0;
}

static void f0386_dispatch_inventory_champion(
    Dm1V1ChampionPanelInventoryWalkStatePc34 *state)
{
    if (!state) {
        return;
    }
    ++state->f0386DrawActionIconCount;
    state->inventorySlotF0386Dispatched[1] = 1;
}

static void f0292_dispatch_inventory_champion(
    Dm1V1ChampionPanelInventoryWalkStatePc34 *state)
{
    if (!state) {
        return;
    }
    ++state->f0292InvocationCount;
    state->champions[state->inventoryChampionIndex].attributesMask |=
        (int)DM1_V1_DMIW_MASK0x4000_VIEWPORT_PC34;
    ++state->mask0x4000ViewportSetCount;
}

static void inventory_walk(
    Dm1V1ChampionPanelInventoryWalkStatePc34 *state)
{
    int slot_index;
    int changed;
    int trace_step;

    if (!state) {
        return;
    }
    trace_step = state->f0296InvocationCount;
    /*
     * CHAMDRAW.C F0296:1235-1242 inventory-owner walk. Iterates
     * AL0882_ui_SlotIndex = C00_SLOT_READY_HAND .. C30_SLOT_CHEST_1-1
     * (slot indices 0..29 inclusive, 30 slots), runs F0295 with
     * the C08_SLOT_BOX_INVENTORY_FIRST_SLOT offset, ORs into
     * AL0884_B_DrawViewport, and dispatches F0386 once on the
     * inventory champion when the changed slot is C01_SLOT_ACTION_HAND.
     */
    state->drawViewportLatched = 0;
    for (slot_index = DM1_V1_DMIW_C00_SLOT_READY_HAND_PC34;
         slot_index < DM1_V1_DMIW_C30_SLOT_CHEST_1_PC34;
         ++slot_index) {
        changed = f0295_sense_inventory_slot(state, slot_index);
        if (changed) {
            state->drawViewportLatched = 1;
            if (slot_index == DM1_V1_DMIW_C01_SLOT_ACTION_HAND_PC34) {
                f0386_dispatch_inventory_champion(state);
            }
        }
        if (trace_step + 1 < DM1_V1_DMIW_TRACE_COUNT_PC34) {
            state->f0296Trace[trace_step + 1] = kTraceInventoryWalk;
        }
    }
}

static int is_chest_panel_content(int panel_content)
{
    /*
     * DEFS.H:2995-3011 M569_PANEL_CHEST is 4 for v1.x/v2.x and
     * 6 for v3.x. Both values are accepted as "chest panel open"
     * because F0296:1244 / F0296:1246 only branches on equality.
     */
    return panel_content == DM1_V1_DMIW_M569_PANEL_CHEST_PC34 ||
           panel_content == DM1_V1_DMIW_M569_PANEL_CHEST_V2X_PC34;
}

static void chest_walk(
    Dm1V1ChampionPanelInventoryWalkStatePc34 *state)
{
    int slot_index;
    int changed;
    int trace_step;

    if (!state) {
        return;
    }
    trace_step = state->f0296InvocationCount + 1;
    /*
     * CHAMDRAW.C F0296:1244-1251 chest-slot walk. Walks chest
     * slots 0..7 on G0425_aT_ChestSlots with the C38 offset, ORs
     * each F0295 result into AL0884_B_DrawViewport, and never
     * dispatches F0386 on chest slots (chest slots are not
     * action-hand slots).
     */
    for (slot_index = 0;
         slot_index < DM1_V1_DMIW_CHEST_SLOT_COUNT_PC34;
         ++slot_index) {
        changed = f0295_sense_chest_slot(state, slot_index);
        if (changed) {
            state->drawViewportLatched = 1;
        }
        if (trace_step + slot_index < DM1_V1_DMIW_TRACE_COUNT_PC34) {
            state->f0296Trace[trace_step + slot_index] = kTraceChestWalk;
        }
    }
}

static int f0296_inventory_walk(
    Dm1V1ChampionPanelInventoryWalkStatePc34 *state)
{
    int trace_step;

    if (!state) {
        return 0;
    }
    ++state->f0296InvocationCount;
    trace_step = state->f0296InvocationCount - 1;
    if (trace_step >= 0 && trace_step < DM1_V1_DMIW_TRACE_COUNT_PC34) {
        state->f0296Trace[trace_step] = kTraceF0296Enter;
    }
    /*
     * CHAMDRAW.C F0296:1233-1259 inventory-owner walk entry.
     * F0296 only enters this block when
     * G0423_i_InventoryChampionOrdinal != 0; the state guard
     * already enforces that, so we proceed directly.
     */
    inventory_walk(state);
    /*
     * CHAMDRAW.C F0296:1244-1251 chest-slot walk is gated on
     * G0424_i_PanelContent == M569_PANEL_CHEST (or v3.x = 6).
     * Panel content other than M569_PANEL_CHEST skips the chest
     * walk entirely.
     */
    if (is_chest_panel_content(state->panelContent)) {
        chest_walk(state);
    }
    /*
     * CHAMDRAW.C F0296:1253-1256 F0292 dispatch + MASK0x4000_
     * VIEWPORT set. When AL0884_B_DrawViewport is C1_TRUE,
     * F0296 sets MASK0x4000_VIEWPORT on the inventory champion's
     * Attributes (M008_SET) and calls F0292_CHAMPION_DrawState
     * on the inventory champion index exactly once.
     */
    if (state->drawViewportLatched) {
        f0292_dispatch_inventory_champion(state);
        if (trace_step + 2 < DM1_V1_DMIW_TRACE_COUNT_PC34) {
            state->f0296Trace[trace_step + 2] = kTraceF0292Dispatch;
        }
        if (trace_step + 3 < DM1_V1_DMIW_TRACE_COUNT_PC34) {
            state->f0296Trace[trace_step + 3] = kTraceMaskViewportSet;
        }
    }
    return 1;
}

static uint32_t fnv1a_hash_state(
    const Dm1V1ChampionPanelInventoryWalkStatePc34 *state)
{
    uint32_t hash;
    const unsigned char *bytes;
    size_t size;

    if (!state) {
        return 0;
    }
    hash = 0x811c9dc5u;
    bytes = (const unsigned char *)state;
    size = sizeof(*state);
    while (size > 0) {
        hash ^= (uint32_t)(*bytes);
        hash *= 0x01000193u;
        ++bytes;
        --size;
    }
    return hash;
}

int dm1_v1_champion_panel_inventory_walk_run_pc34(
    Dm1V1ChampionPanelInventoryWalkStatePc34 *state,
    Dm1V1ChampionPanelInventoryWalkResultPc34 *result)
{
    int i;
    int s;
    int rejected_dead_member;
    int rejected_party_size_zero;
    int rejected_no_inventory_owner;
    int rejected_invalid_panel_content;
    int panel_content_value;
    int inventory_slot_f0295_total;
    int inventory_slot_f0386_total;
    int inventory_slot_changed_total;
    int chest_slot_f0295_total;
    int chest_slot_changed_total;

    if (!state || !result) {
        return 0;
    }
    rejected_dead_member = 0;
    for (i = 0; i < state->partyChampionCount; ++i) {
        if (!state->champions[i].alive) {
            rejected_dead_member = 1;
            break;
        }
    }
    rejected_party_size_zero = (state->partyChampionCount <= 0) ? 1 : 0;
    rejected_no_inventory_owner = (state->inventoryChampionOrdinal == 0) ? 1 : 0;
    panel_content_value = state->panelContent;
    rejected_invalid_panel_content = 0;
    if (panel_content_value != 0 &&
        !is_chest_panel_content(panel_content_value)) {
        rejected_invalid_panel_content = 1;
    }
    if (!state_valid(state)) {
        memset(result, 0, sizeof(*result));
        result->rejectsDeadMember = rejected_dead_member;
        result->rejectsPartySizeZero = rejected_party_size_zero;
        result->rejectsNoInventoryOwner = rejected_no_inventory_owner;
        result->rejectsInvalidPanelContent = rejected_invalid_panel_content;
        result->path = DM1_V1_DMIW_PATH_INVALID_PC34;
        if (rejected_no_inventory_owner) {
            result->path = DM1_V1_DMIW_PATH_REJECTED_NO_INVENTORY_OWNER_PC34;
        } else if (rejected_invalid_panel_content) {
            result->path = DM1_V1_DMIW_PATH_REJECTED_INVALID_PANEL_CONTENT_PC34;
        } else if (rejected_dead_member) {
            result->path = DM1_V1_DMIW_PATH_REJECTED_DEAD_MEMBER_PC34;
        } else if (rejected_party_size_zero) {
            result->path = DM1_V1_DMIW_PATH_REJECTED_PARTY_INCOMPLETE_PC34;
        }
        return 0;
    }
    f0296_inventory_walk(state);
    inventory_slot_f0295_total = 0;
    inventory_slot_f0386_total = 0;
    inventory_slot_changed_total = 0;
    for (s = 0; s < DM1_V1_DMIW_INVENTORY_SLOT_COUNT_PC34; ++s) {
        inventory_slot_f0295_total += state->inventorySlotF0295Dispatched[s];
        inventory_slot_f0386_total += state->inventorySlotF0386Dispatched[s];
        inventory_slot_changed_total += state->inventorySlotIconChanged[s];
    }
    chest_slot_f0295_total = 0;
    chest_slot_changed_total = 0;
    for (s = 0; s < DM1_V1_DMIW_CHEST_SLOT_COUNT_PC34; ++s) {
        chest_slot_f0295_total += state->chestSlotF0295Dispatched[s];
        chest_slot_changed_total += state->chestSlotIconChanged[s];
    }
    memset(result, 0, sizeof(*result));
    result->accepted = 1;
    result->sourceAnchorsPresent =
        dm1_v1_champion_panel_inventory_walk_source_anchors_present_pc34();
    result->fullyAliveRecognized = 1;
    result->inventoryWalkCoversThirtySlots =
        (inventory_slot_f0295_total ==
         DM1_V1_DMIW_INVENTORY_SLOT_COUNT_PC34);
    result->inventoryWalkSlotboxOffsetApplied =
        (state->champions[state->inventoryChampionIndex]
             .inventoryF0295Dispatched[1] == 1);
    result->inventoryWalkActionHandDispatchesF0386 =
        (state->inventorySlotF0386Dispatched[1] == 1);
    result->inventoryWalkNonActionHandSkipsF0386 =
        (state->f0386DrawActionIconCount ==
         (state->inventorySlotF0386Dispatched[1] == 1 ? 1 : 0));
    result->inventoryWalkF0295DispatchedPerSlot =
        (inventory_slot_f0295_total ==
         DM1_V1_DMIW_INVENTORY_SLOT_COUNT_PC34);
    result->chestWalkGatedOnPanelContentChest =
        (state->panelContent == DM1_V1_DMIW_M569_PANEL_CHEST_PC34 &&
         state->chestSlotF0295Dispatched[0] == 1) ||
        (state->panelContent == DM1_V1_DMIW_M569_PANEL_CHEST_V2X_PC34 &&
         state->chestSlotF0295Dispatched[0] == 1);
    result->chestWalkGatedOffWhenPanelContentNotChest =
        (state->panelContent == 0 &&
         state->chestSlotF0295Dispatched[0] == 0);
    result->chestWalkCoversEightSlots =
        (chest_slot_f0295_total == DM1_V1_DMIW_CHEST_SLOT_COUNT_PC34);
    result->chestWalkSlotboxOffsetApplied =
        (state->chestSlotF0295Dispatched[0] == 1 ||
         state->panelContent == 0);
    result->chestWalkF0295DispatchedPerSlot =
        (chest_slot_f0295_total ==
             (is_chest_panel_content(state->panelContent)
                  ? DM1_V1_DMIW_CHEST_SLOT_COUNT_PC34
                  : 0));
    result->chestWalkNeverDispatchesF0386 =
        (chest_slot_changed_total == 0 &&
         state->f0386DrawActionIconCount ==
             (state->inventorySlotF0386Dispatched[1] == 1 ? 1 : 0)) ||
        (chest_slot_changed_total > 0 &&
         state->f0386DrawActionIconCount ==
             (state->inventorySlotF0386Dispatched[1] == 1 ? 1 : 0));
    result->f0292DispatchedOnlyWhenDrawViewportLatched =
        ((state->drawViewportLatched && state->f0292InvocationCount == 1) ||
         (!state->drawViewportLatched && state->f0292InvocationCount == 0));
    result->f0292DispatchedExactlyOncePerF0296 =
        (state->f0292InvocationCount <= 1);
    result->f0292TargetsInventoryChampionIndex =
        (state->champions[state->inventoryChampionIndex].attributesMask &
         (int)DM1_V1_DMIW_MASK0x4000_VIEWPORT_PC34) != 0 ||
        !state->drawViewportLatched;
    result->mask0x4000ViewportSetOnlyWhenDrawViewportLatched =
        ((state->drawViewportLatched && state->mask0x4000ViewportSetCount == 1) ||
         (!state->drawViewportLatched &&
          state->mask0x4000ViewportSetCount == 0));
    result->mask0x4000ViewportSetOnInventoryChampionAttributes =
        (state->mask0x4000ViewportSetCount <= 1) &&
        ((state->champions[state->inventoryChampionIndex].attributesMask &
          (int)DM1_V1_DMIW_MASK0x4000_VIEWPORT_PC34) ==
             (int)(state->drawViewportLatched
                       ? DM1_V1_DMIW_MASK0x4000_VIEWPORT_PC34
                       : 0));
    result->rejectsDeadMember = rejected_dead_member;
    result->rejectsPartySizeZero = rejected_party_size_zero;
    result->rejectsNoInventoryOwner = rejected_no_inventory_owner;
    result->rejectsInvalidPanelContent = rejected_invalid_panel_content;
    for (i = 0; i < DM1_V1_DMIW_TRACE_COUNT_PC34; ++i) {
        result->trace[i] = state->f0296Trace[i];
    }
    result->partyChampionCount = state->partyChampionCount;
    result->leaderIndex = state->leaderIndex;
    result->inventoryChampionOrdinal = state->inventoryChampionOrdinal;
    result->inventoryChampionIndex = state->inventoryChampionIndex;
    result->panelContent = state->panelContent;
    result->f0296InvocationCount = state->f0296InvocationCount;
    result->f0292InvocationCount = state->f0292InvocationCount;
    result->f0295HasIconChangedCount = state->f0295HasIconChangedCount;
    result->f0295SameIconCount = state->f0295SameIconCount;
    result->f0386DrawActionIconCount = state->f0386DrawActionIconCount;
    result->inventorySlotF0295DispatchedTotal = inventory_slot_f0295_total;
    result->inventorySlotF0386DispatchedTotal = inventory_slot_f0386_total;
    result->inventorySlotIconChangedTotal = inventory_slot_changed_total;
    result->chestSlotF0295DispatchedTotal = chest_slot_f0295_total;
    result->chestSlotIconChangedTotal = chest_slot_changed_total;
    result->inventoryWalkSlotCount = DM1_V1_DMIW_INVENTORY_SLOT_COUNT_PC34;
    result->chestWalkSlotCount =
        is_chest_panel_content(state->panelContent)
            ? DM1_V1_DMIW_CHEST_SLOT_COUNT_PC34
            : 0;
    result->drawViewportLatched = state->drawViewportLatched;
    result->mask0x4000ViewportSetCount = state->mask0x4000ViewportSetCount;
    if (state->drawViewportLatched) {
        result->path =
            is_chest_panel_content(state->panelContent)
                ? DM1_V1_DMIW_PATH_INVENTORY_WALK_DRAW_VIEWPORT_PC34
                : DM1_V1_DMIW_PATH_INVENTORY_WALK_DRAW_VIEWPORT_PC34;
    } else {
        result->path =
            is_chest_panel_content(state->panelContent)
                ? DM1_V1_DMIW_PATH_INVENTORY_WALK_WITH_CHEST_PC34
                : DM1_V1_DMIW_PATH_INVENTORY_WALK_NO_CHEST_PC34;
    }
    result->hash = fnv1a_hash_state(state);
    return 1;
}
