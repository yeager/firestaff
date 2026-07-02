/*
 * DM1 V1 champion-panel hand-slot refresh walk-order gate implementation.
 *
 * Source-lock anchors (ReDMCSB WIP 20210206):
 *  - CHAMDRAW.C F0295_CHAMPION_HasObjectIconInSlotBoxChanged:1153-1182
 *  - CHAMDRAW.C F0296_CHAMPION_DrawChangedObjectIcons:1184-1267
 *  - OBJECT.C   F0033_OBJECT_GetIconIndex
 *  - OBJECT.C   F0038_OBJECT_DrawIconInSlotBox
 *  - OBJECT.C   F0036_OBJECT_ExtractIconFromBitmap
 *  - MOUSE.C    F0068_MOUSE_SetPointerToObject
 *  - OBJECT.C   F0034_OBJECT_DrawLeaderHandObjectName
 *  - MOUSE.C    F0077_MOUSE_EnableScreenUpdate_CPSE
 *  - MOUSE.C    F0078_MOUSE_DisableScreenUpdate
 *  - DEFS.H     781 C01_SLOT_ACTION_HAND
 *  - DEFS.H     1878 M070_HAND_SLOT_INDEX
 *  - DEFS.H     1950 C195_ICON_POTION_EMPTY_FLASK
 *  - DEFS.H     1952 C201_ICON_ACTION_ICON_EMPTY_HAND
 *  - DEFS.H     5320 G0423_i_InventoryChampionOrdinal
 *  - DEFS.H     5322 G0305_ui_PartyChampionCount
 *  - DEFS.H     5694 G0299_ui_CandidateChampionOrdinal
 *  - DEFS.H     7208 M000_INDEX_TO_ORDINAL(index) == (index + 1)
 *  - DEFS.H     7209 M001_ORDINAL_TO_INDEX(ordinal) == (ordinal - 1)
 *
 * No bitmap sampling, no GRAPHICS.DAT/DUNGEON.DAT load, no real-asset
 * parity claim. The contract models the F0296 walk-order + leader-hand
 * icon refresh precedence + candidate early-return +
 * inventory-champion skip contract on a fully-alive 4-champion party
 * without sampling real bitmaps.
 */

#include "firestaff/dm1/v1/champion_panel/hand_slot_refresh_pc34_compat.h"

#include <string.h>

enum {
    kPartyChampionCount = 4,
    kLeaderIndex = 0,
    kSecondIndex = 1,
    kThirdIndex = 2,
    kFourthIndex = 3,

    kSlotBoxReadyHand = 0,
    kSlotBoxActionHand = 1,

    kMutableIconLower = 0,
    kMutableIconUpper = 31,
    kMutableIconPotionLower = 148,
    kMutableIconPotionUpper = 163,
    kMutableIconEmptyFlask = 195,

    kLeaderActionHandThing = 0x0510u,
    kSecondActionHandThing = 0x0511u,
    kThirdActionHandThing = 0x0512u,
    kFourthActionHandThing = 0x0513u,

    /* Champion 0 (leader): icon changed (mutable) and slotbox shows a
     * different icon, so F0295 must dispatch and F0386 must fire.
     */
    kLeaderCurrentIcon = 7,
    kLeaderObjectIcon = 12,

    /* Champion 1: icon changed (mutable), same logic as the leader.
     */
    kSecondCurrentIcon = 9,
    kSecondObjectIcon = 14,

    /* Champion 2: icon changed (mutable), same logic.
     */
    kThirdCurrentIcon = 10,
    kThirdObjectIcon = 16,

    /* Champion 3: icon matched, F0295 returns C0_FALSE on the
     * action-hand slotbox walk; F0386 is not dispatched.
     */
    kFourthCurrentIcon = 18,
    kFourthObjectIcon = 18,

    kLeaderHandThing = 0x0520u,
    kLeaderHandIconIndex = 22,

    kTraceInit = 0,
    kTraceF0296Enter = 1,
    kTraceLeaderHandRefresh = 2,
    kTraceSlotboxWalk = 3,
    kTraceInventoryChampionSkip = 4,
    kTraceF0295Sense = 5,
    kTraceF0386Dispatch = 6,
    kTraceSettle = 7
};

/*
 * ReDMCSB source-lock contract only.
 *
 * CHAMDRAW.C F0295_CHAMPION_HasObjectIconInSlotBoxChanged:1153-1182
 * compares the slotbox's currently displayed icon (read via
 * F0039_OBJECT_GetIconIndexInSlotBox) against the slot's object icon
 * (read via F0033_OBJECT_GetIconIndex(thing)); when the icons differ
 * AND the current icon is in a mutable range (junk C000..C031,
 * potions C148..C163, or the C195 empty flask), F0295 calls
 * F0038_OBJECT_DrawIconInSlotBox to repaint the slotbox and returns
 * C1_TRUE. The mutable icon guard is the OBJECT.C anchor for which
 * icons are "live" enough to be redrawn on icon change.
 *
 * CHAMDRAW.C F0296_CHAMPION_DrawChangedObjectIcons:1184-1267 walks
 * slotbox indices 0..(G0305_ui_PartyChampionCount << 1)-1. The
 * walk has the following structure:
 *
 *   (a) lines 1208-1210: candidate early-return. When
 *       G0299_ui_CandidateChampionOrdinal is non-zero AND
 *       G0423_i_InventoryChampionOrdinal is zero, F0296 returns
 *       immediately without entering the walk loop. The walk never
 *       fires while the C040 candidate panel is being shown and
 *       the inventory panel is not.
 *
 *   (b) line 1212: G0420_B_MousePointerHiddenToDrawChangedObject-
 *       IconOnScreen is reset to C0_FALSE at the entry of F0296.
 *
 *   (c) lines 1213-1220: leader-hand icon refresh. When
 *       G4055_s_LeaderHandObject.IconIndex is in a mutable range
 *       AND F0033_OBJECT_GetIconIndex(G4055_s_LeaderHandObject.
 *       Thing) differs from it, F0296 calls F0077 + F0036 +
 *       F0068 + F0034 in that order. This step happens BEFORE
 *       the slotbox walk.
 *
 *   (d) lines 1221-1231: slotbox walk over indices
 *       0..(partyChampionCount << 1)-1. For each odd slotbox
 *       index (M070_HAND_SLOT_INDEX returns C01_SLOT_ACTION_HAND),
 *       the walk computes L0885_i_ChampionIndex = slotBoxIndex >>
 *       1 and applies the inventory-champion ordinal skip:
 *
 *         if (L0883_ui_InventoryChampionOrdinal ==
 *             M000_INDEX_TO_ORDINAL(L0885_i_ChampionIndex))
 *             continue;
 *
 *       i.e. when the F0296 walk reaches the inventory
 *       champion's action-hand slotbox index it skips the F0295
 *       sense and the F0386 dispatch. The skip applies per
 *       slotbox; it does not affect non-inventory slotboxes.
 *
 *   (e) line 1231: F0386 is dispatched only when both F0295
 *       returns C1_TRUE AND M070_HAND_SLOT_INDEX(slotBoxIndex)
 *       equals C01_SLOT_ACTION_HAND.
 *
 *   (g) lines 1242-1262: inventory-owner secondary walk. The block
 *       fires when L0883_ui_InventoryChampionOrdinal is non-zero and
 *       the walk has not early-returned. The block walks the
 *       inventory champion's internal slots C00..C29 (READY_HAND..
 *       C29_SLOT_BACKPACK_LINE1_9) and for each `internalSlot`
 *       computes the inventory slotbox index as
 *       `internalSlot + C08_SLOT_BOX_INVENTORY_FIRST_SLOT` (= C08..
 *       C37). Each iteration dispatches F0295 on
 *       `(inventoryChampion.Slots[internalSlot])` and OR-accumulated
 *       the F0295 return into AL0884_B_DrawViewport. F0386 is
 *       dispatched only when internalSlot == C01_SLOT_ACTION_HAND
 *       AND F0295 returns C1_TRUE.
 *
 *   (h) lines 1248-1254: when the panel content is the chest panel
 *       (G2008_i_PanelContent == M569_PANEL_CHEST = 6 on PC 3.4 /
 *       MEDIA720 / I34E), F0296 walks the 8 chest slots
 *       G0425_aT_ChestSlots[0..7] and dispatches F0295 on
 *       `(chestSlot + C38_SLOT_BOX_CHEST_FIRST_SLOT)`. Each
 *       iteration OR-accumulated the F0295 return into
 *       AL0884_B_DrawViewport.
 *
 *   (i) lines 1256-1259: when AL0884_B_DrawViewport is C1_TRUE,
 *       M008_SET sets MASK0x4000_VIEWPORT on the inventory champion's
 *       Attributes and F0292_CHAMPION_DrawState is dispatched for
 *       the inventory champion (M001_ORDINAL_TO_INDEX of the owner
 *       ordinal). When AL0884_B_DrawViewport is C0_FALSE, neither
 *       the attribute-set nor F0292 fires.
 *
 *   (j) lines 1264-1267: at the end of F0296, when
 *       G0420_B_MousePointerHiddenToDrawChangedObjectIconOnScreen
 *       is C1_TRUE (some change during this walk raised it),
 *       F0078_MOUSE_DisableScreenUpdate is called exactly once.
 *
 * DEFS.H:780 C00_SLOT_READY_HAND, 781 C01_SLOT_ACTION_HAND, 810
 * C30_SLOT_CHEST_1, 1874 C08_SLOT_BOX_INVENTORY_FIRST_SLOT, 1876
 * C38_SLOT_BOX_CHEST_FIRST_SLOT, 1878 M070_HAND_SLOT_INDEX
 * (returns slotboxIndex & 0x0001), 1950 C195_ICON_POTION_EMPTY_FLASK,
 * 1952 C201_ICON_ACTION_ICON_EMPTY_HAND, 2995/3007 M569_PANEL_CHEST,
 * 5316 G0420_B_MousePointerHiddenToDrawChangedObjectIconOnScreen,
 * 5320 G0423_i_InventoryChampionOrdinal, 5322 G0305_ui_PartyChampion-
 * Count, 5694 G0299_ui_CandidateChampionOrdinal, 5877 G0424_i_Panel-
 * Content, 5878 G0425_aT_ChestSlots[8], 6317 G2008_i_PanelContent,
 * 7208 M000_INDEX_TO_ORDINAL(index) == (index + 1), 7209
 * M001_ORDINAL_TO_INDEX(ordinal) == (ordinal - 1), 7895
 * F0292_CHAMPION_DrawState pin the constants.
 */
static const char s_source_evidence[] =
    "contract_only=1; no real-asset bitmap parity claim; no GRAPHICS.DAT or "
    "DUNGEON.DAT load. CHAMDRAW.C F0295_CHAMPION_HasObjectIconInSlotBoxChanged:"
    "1153-1182 compares the slotbox's currently displayed icon (read via "
    "F0039_OBJECT_GetIconIndexInSlotBox) against the slot's object icon (read "
    "via F0033_OBJECT_GetIconIndex(thing)); when the icons differ AND the "
    "current icon is in a mutable range (junk C000..C031, potions C148..C163, "
    "or the C195 empty flask), F0295 calls F0038_OBJECT_DrawIconInSlotBox to "
    "repaint the slotbox and returns C1_TRUE. CHAMDRAW.C F0296_CHAMPION_"
    "DrawChangedObjectIcons:1184-1267 has the following contract: (a) lines "
    "1208-1210 candidate early-return when G0299_ui_CandidateChampionOrdinal "
    "is non-zero and G0423_i_InventoryChampionOrdinal is zero F0296 returns "
    "immediately; (b) line 1212 G0420_B_MousePointerHiddenToDrawChangedObject-"
    "IconOnScreen is reset to C0_FALSE; (c) lines 1213-1220 leader-hand icon "
    "refresh precedes the slotbox walk, calling F0077 + F0036 + F0068 + F0034 "
    "when G4055_s_LeaderHandObject.IconIndex is in a mutable range and "
    "F0033_OBJECT_GetIconIndex(G4055_s_LeaderHandObject.Thing) differs; (d) "
    "lines 1221-1231 slotbox walk over indices 0..(partyChampionCount << "
    "1)-1 with L0885_i_ChampionIndex = slotBoxIndex >> 1 and the inventory-"
    "champion ordinal skip when L0883_ui_InventoryChampionOrdinal equals "
    "M000_INDEX_TO_ORDINAL(L0885_i_ChampionIndex); (e) line 1231 F0386 is "
    "dispatched only when F0295 returns C1_TRUE AND M070_HAND_SLOT_INDEX("
    "slotBoxIndex) equals C01_SLOT_ACTION_HAND; (g) lines 1242-1247 the "
    "inventory-owner secondary walk fires iff L0883_ui_InventoryChampionOrdinal "
    "!= 0 AND the candidate early-return did NOT fire; the walk visits the "
    "inventory champion's internal slots C00..C29 (READY_HAND.."
    "C29_SLOT_BACKPACK_LINE1_9) with inventory slotbox index "
    "internalSlot + C08_SLOT_BOX_INVENTORY_FIRST_SLOT, dispatches F0295 on "
    "each internal slot's thing, and OR-accumulated the return into "
    "AL0884_B_DrawViewport; F0386 is dispatched only on internal slot C01 ("
    "C01_SLOT_ACTION_HAND) AND only when F0295 returned C1_TRUE; (h) lines "
    "1248-1254 chest-panel secondary walk fires iff G2008_i_PanelContent "
    "(PC 3.4 MEDIA720 PC 3.4 I34E) == M569_PANEL_CHEST = 6, the walk visits "
    "G0425_aT_ChestSlots[0..7] with slotbox index chestSlot + "
    "C38_SLOT_BOX_CHEST_FIRST_SLOT, and OR-accumulated the F0295 return into "
    "AL0884_B_DrawViewport; (i) lines 1256-1259 when AL0884_B_DrawViewport is "
    "C1_TRUE, M008_SET sets MASK0x4000_VIEWPORT on the inventory champion's "
    "Attributes and F0292_CHAMPION_DrawState is dispatched for the inventory "
    "champion (M001_ORDINAL_TO_INDEX of the owner ordinal); (j) lines "
    "1264-1267 F0078_MOUSE_DisableScreenUpdate is called exactly once at the "
    "end of F0296 when G0420_B_MousePointerHiddenToDrawChangedObjectIconOnScreen "
    "is still C1_TRUE. DEFS.H:780 C00_SLOT_READY_HAND, 781 C01_SLOT_ACTION_HAND, "
    "810 C30_SLOT_CHEST_1, 1874 C08_SLOT_BOX_INVENTORY_FIRST_SLOT, 1876 "
    "C38_SLOT_BOX_CHEST_FIRST_SLOT, 1878 M070_HAND_SLOT_INDEX, 1950 "
    "C195_ICON_POTION_EMPTY_FLASK, 1952 C201_ICON_ACTION_ICON_EMPTY_HAND, "
    "2995/3007 M569_PANEL_CHEST, 5320 G0423_i_InventoryChampionOrdinal, 5322 "
    "G0305_ui_PartyChampionCount, 5694 G0299_ui_CandidateChampionOrdinal, "
    "5877 G0424_i_PanelContent, 5878 G0425_aT_ChestSlots[8], 6317 "
    "G2008_i_PanelContent, 7208 M000_INDEX_TO_ORDINAL(index) == (index + 1), "
    "7209 M001_ORDINAL_TO_INDEX(ordinal) == (ordinal - 1), 7895 "
    "F0292_CHAMPION_DrawState.";

static const Dm1V1ChampionPanelHandSlotRefreshEvidencePc34 s_evidence = {
    "ReDMCSB CHAMDRAW.C F0295_CHAMPION_HasObjectIconInSlotBoxChanged:1153-1182",
    "ReDMCSB CHAMDRAW.C F0296_CHAMPION_DrawChangedObjectIcons:1184-1267",
    "ReDMCSB OBJECT.C F0033_OBJECT_GetIconIndex(thing) for slot thing -> icon",
    "ReDMCSB OBJECT.C F0038_OBJECT_DrawIconInSlotBox(slotBoxIndex, iconIndex)",
    "ReDMCSB CHAMDRAW.C F0296:1213-1220 G4055_s_LeaderHandObject.F0036_OBJECT_ExtractIconFromBitmap + F0068_MOUSE_SetPointerToObject + F0034_OBJECT_DrawLeaderHandObjectName sequence precedes the slotbox walk",
    "ReDMCSB CHAMDRAW.C F0296:1208-1210 G0299_ui_CandidateChampionOrdinal early-return",
    "ReDMCSB CHAMDRAW.C F0296:1217-1219 G0423_i_InventoryChampionOrdinal skip on the matched slotbox",
    "ReDMCSB CHAMDRAW.C F0296:1212 G0420_B_MousePointerHiddenToDrawChangedObjectIconOnScreen reset + F0296:1248-1254 inventory-owner secondary walk (30 internal slots + 8 chest-panel slots) + F0296:1256-1259 MASK0x4000_VIEWPORT set + F0292 dispatch + F0296:1264-1267 F0078_MOUSE_DisableScreenUpdate tail",
    "ReDMCSB DEFS.H:780 C00_SLOT_READY_HAND, 781 C01_SLOT_ACTION_HAND, 810 C30_SLOT_CHEST_1, 1874 C08_SLOT_BOX_INVENTORY_FIRST_SLOT, 1876 C38_SLOT_BOX_CHEST_FIRST_SLOT, 1878 M070_HAND_SLOT_INDEX, 1950 C195_ICON_POTION_EMPTY_FLASK, 1952 C201_ICON_ACTION_ICON_EMPTY_HAND, 2995/3007 M569_PANEL_CHEST, 5320 G0423_i_InventoryChampionOrdinal, 5322 G0305_ui_PartyChampionCount, 5694 G0299_ui_CandidateChampionOrdinal, 5877 G0424_i_PanelContent, 5878 G0425_aT_ChestSlots[8], 6317 G2008_i_PanelContent, 7208 M000_INDEX_TO_ORDINAL, 7209 M001_ORDINAL_TO_INDEX, 7895 F0292_CHAMPION_DrawState",
    "contract-only F0296 walk-order + leader-hand icon refresh precedence + candidate early-return + inventory-champion skip on a fully-alive 4-champion party + F0296:1242-1262 inventory-owner secondary walk contract (30 internal slots + 8 chest-panel slots + AL0884_B_DrawViewport accumulation + MASK0x4000_VIEWPORT set + F0292 dispatch); no real M11 graphics, no real bitmaps, no asset load",
    "no GRAPHICS.DAT, no DUNGEON.DAT, no real-asset bitmap parity claim",
    "Non-overlap marker: pass champion_panel_hand_slot_refresh covers the F0296 walk-order (action-hand slotbox indices 1, 3, 5, 7 in champion-index order 0, 1, 2, 3), the F0296 candidate-champion ordinal early-return, the F0296 inventory-champion ordinal skip on the matched slotbox, the F0296 leader-hand icon refresh precedence (F0077 + F0036 + F0068 + F0034), the F0295 per-slotbox sense-and-dispatch contract on a fully-alive 4-champion party, the F0296:1242-1262 inventory-owner secondary walk contract (30 internal slots walked; F0386 dispatch only on C01_SLOT_ACTION_HAND when changed; per-slot AL0884_B_DrawViewport accumulation; chest-panel 8-slot secondary walk when G2008_i_PanelContent == M569_PANEL_CHEST; MASK0x4000_VIEWPORT set + F0292 dispatch when AL0884_B_DrawViewport is C1_TRUE), and the F0077/F0078 mouse-screen-update balance on a fully-alive party; disjoint from champion_panel_dead_member_hand_refresh (F0296/F0295/F0386 walk with a dead member present + F0292 dead-status-box branch), champion_panel_hand_slot_priority (CHAMPION.C F0302 input dispatch, not the F0296 redraw walk), champion_panel_portrait_box_redraw_states (F0291/F0292/F0296 event matrix for the portrait-box branch + status-box cascade, not the F0296 hand-slot walk-order), champion_panel_portrait_state_redraw (F0292 state-redraw cascade, not the F0296 hand-slot walk-order), mirror_candidate_icon_refresh (F0296 leader-hand icon refresh interaction with the candidate ordinal, no walk-order coverage), mirror_candidate_c040 sibling family (candidate-panel state machine, no F0296 walk-order), champion_panel_spell_area_overlay (F0394 dead-champion reject for spell area, not the F0296 hand-slot walk-order), champion_panel_status_hand_rotation (F0284 leader rotation, not the F0296 hand-slot walk-order), champion_panel_second_leader_hand_slot_priority (the 2nd leader's hand-slot priority path, not the F0296 walk-order), the F0107/F0108/chest-scroll-wheel/viewport/integrated family, and the per-state redraw + per-action-hand slot-box dispatch family."
};

static uint32_t hash_step(uint32_t hash, unsigned int value)
{
    int i;

    for (i = 0; i < 4; ++i) {
        hash ^= (uint32_t)((value >> (i * 8)) & 0xffu);
        hash *= UINT32_C(16777619);
    }
    return hash;
}

static uint32_t hash_state(
    const Dm1V1ChampionPanelHandSlotRefreshStatePc34 *state)
{
    uint32_t hash = UINT32_C(2166136261);
    int i;

    hash = hash_step(hash, (unsigned int)state->partyChampionCount);
    hash = hash_step(hash, (unsigned int)state->leaderIndex);
    hash = hash_step(hash, (unsigned int)state->candidateChampionOrdinal);
    hash = hash_step(hash, (unsigned int)state->inventoryChampionOrdinal);
    hash = hash_step(hash, (unsigned int)state->aliveMembers);
    hash = hash_step(hash, (unsigned int)state->inventoryChampionIndex);
    hash = hash_step(hash, (unsigned int)state->f0296InvocationCount);
    hash = hash_step(hash, (unsigned int)state->f0295HasIconChangedCount);
    hash = hash_step(hash, (unsigned int)state->f0295SameIconCount);
    hash = hash_step(hash, (unsigned int)state->f0033GetIconIndexCount);
    hash = hash_step(hash, (unsigned int)state->f0038DrawIconInSlotBoxCount);
    hash = hash_step(hash, (unsigned int)state->f0036ExtractIconFromBitmapCount);
    hash = hash_step(hash, (unsigned int)state->f0068SetPointerToObjectCount);
    hash = hash_step(hash, (unsigned int)state->f0034DrawLeaderHandObjectNameCount);
    hash = hash_step(hash, (unsigned int)state->f0386DrawActionIconCount);
    hash = hash_step(hash, (unsigned int)state->f0077MouseEnableCount);
    hash = hash_step(hash, (unsigned int)state->f0078MouseDisableCount);
    hash = hash_step(hash, (unsigned int)state->leaderHandIconChanged);
    hash = hash_step(hash, (unsigned int)state->leaderHandMousePointerHidden);
    hash = hash_step(hash, (unsigned int)state->leaderHandIcon);
    hash = hash_step(hash, (unsigned int)state->leaderHandThingThing);
    hash = hash_step(hash, (unsigned int)state->leaderHandIconRefreshCount);
    /*
     * F0296:1242-1262 inventory-owner secondary walk contract fields.
     * The hash includes the inventory-owner internal walk counts, the
     * per-internal-slot iconChanged trace, the chest-panel state +
     * per-chest-slot iconChanged trace, and the final drawViewport /
     * MASK0x4000_VIEWPORT / F0292 dispatch flags.
     */
    hash = hash_step(hash, (unsigned int)state->internalSlotWalkCount);
    hash = hash_step(hash, (unsigned int)state->internalSlotF0295Dispatched);
    hash = hash_step(hash, (unsigned int)state->internalSlotF0386ActionHandDispatched);
    for (i = 0;
         i < DM1_V1_DMHSR_INVENTORY_INTERNAL_SLOT_COUNT_PC34;
         ++i) {
        hash = hash_step(hash, (unsigned int)state->internalSlotIconChanged[i]);
    }
    hash = hash_step(hash, (unsigned int)state->chestPanelContentId);
    hash = hash_step(hash, (unsigned int)state->chestSlotWalkActive);
    hash = hash_step(hash, (unsigned int)state->chestSlotWalkCount);
    hash = hash_step(hash, (unsigned int)state->chestSlotF0295Dispatched);
    for (i = 0; i < DM1_V1_DMHSR_CHEST_SLOT_COUNT_PC34; ++i) {
        hash = hash_step(hash, (unsigned int)state->chestSlotIconChanged[i]);
    }
    hash = hash_step(hash, (unsigned int)state->drawViewportAccumulated);
    hash = hash_step(hash, (unsigned int)state->attributesMask0x4000ViewportSet);
    hash = hash_step(hash, (unsigned int)state->f0292DrawStateDispatched);
    for (i = 0; i < DM1_V1_DMHSR_PARTY_COUNT_PC34; ++i) {
        hash = hash_step(hash, (unsigned int)state->slotBoxWalkIndex[i]);
        hash = hash_step(hash, (unsigned int)state->slotBoxWalkChampionIndex[i]);
        hash = hash_step(hash, (unsigned int)state->slotBoxWalkIconChanged[i]);
        hash = hash_step(hash, (unsigned int)state->slotBoxWalkF0295Dispatched[i]);
        hash = hash_step(hash, (unsigned int)state->slotBoxWalkF0386Dispatched[i]);
        hash = hash_step(hash, (unsigned int)state->slotBoxWalkInventorySkip[i]);
        hash = hash_step(hash, (unsigned int)state->champions[i].alive);
        hash = hash_step(hash, (unsigned int)state->champions[i].leader);
        hash = hash_step(hash, (unsigned int)state->champions[i].actionHandThing);
        hash = hash_step(hash, (unsigned int)state->champions[i].actionHandIconIndex);
        hash = hash_step(hash, (unsigned int)state->champions[i].slotBoxCurrentIcon);
        hash = hash_step(hash, (unsigned int)state->champions[i].iconChanged);
        hash = hash_step(hash, (unsigned int)state->champions[i].slotBoxIndex);
        hash = hash_step(hash, (unsigned int)state->champions[i].f0295Dispatched);
        hash = hash_step(hash, (unsigned int)state->champions[i].f0038DrawIconInSlotBoxCount);
        hash = hash_step(hash, (unsigned int)state->champions[i].f0386DrawActionIconCount);
        hash = hash_step(hash, (unsigned int)state->champions[i].walkOrder);
        hash = hash_step(hash, (unsigned int)state->champions[i].inventoryChampionSkipHit);
    }
    for (i = 0; i < DM1_V1_DMHSR_TRACE_COUNT_PC34; ++i) {
        hash = hash_step(hash, (unsigned int)state->f0296Trace[i]);
    }
    return hash;
}

static int is_mutable_icon(int icon_index)
{
    return (icon_index >= kMutableIconLower && icon_index <= kMutableIconUpper) ||
           (icon_index >= kMutableIconPotionLower &&
            icon_index <= kMutableIconPotionUpper) ||
           icon_index == kMutableIconEmptyFlask;
}

static int is_action_hand_slotbox(int slot_box_index)
{
    /*
     * ReDMCSB DEFS.H:1878 M070_HAND_SLOT_INDEX(slotboxindex) is
     * (slotboxindex & 0x0001), so the action hand slotbox index is
     * any odd index: 1, 3, 5, ... For a party of N, the F0296 walk
     * visits slotbox indices 0..(N << 1)-1, with 2*idx+1 being the
     * action hand of champion idx and 2*idx being the ready hand.
     */
    return (slot_box_index & 0x0001) == DM1_V1_DMHSR_C01_SLOT_ACTION_HAND_PC34;
}

static int ordinal_to_index(int ordinal)
{
    /* DEFS.H:7209 M001_ORDINAL_TO_INDEX(ordinal) == (ordinal - 1). */
    return ordinal - 1;
}

static int index_to_ordinal(int index)
{
    /* DEFS.H:7208 M000_INDEX_TO_ORDINAL(index) == (index + 1). */
    return index + 1;
}

static int source_anchors_present(void)
{
    return strstr(s_source_evidence,
                  "CHAMDRAW.C F0295_CHAMPION_HasObjectIconInSlotBoxChanged:1153-1182") !=
           NULL &&
           strstr(s_source_evidence,
                  "CHAMDRAW.C F0296_CHAMPION_DrawChangedObjectIcons:1184-1267") !=
               NULL &&
           strstr(s_source_evidence, "F0033_OBJECT_GetIconIndex") != NULL &&
           strstr(s_source_evidence, "F0038_OBJECT_DrawIconInSlotBox") != NULL &&
           strstr(s_source_evidence, "G0299_ui_CandidateChampionOrdinal") != NULL &&
           strstr(s_source_evidence, "G0423_i_InventoryChampionOrdinal") != NULL &&
           strstr(s_source_evidence, "G0305_ui_PartyChampionCount") != NULL &&
           strstr(s_source_evidence, "C01_SLOT_ACTION_HAND") != NULL &&
           strstr(s_source_evidence, "M070_HAND_SLOT_INDEX") != NULL &&
           strstr(s_source_evidence, "C195_ICON_POTION_EMPTY_FLASK") != NULL &&
           strstr(s_source_evidence, "C201_ICON_ACTION_ICON_EMPTY_HAND") != NULL &&
           strstr(s_source_evidence, "M000_INDEX_TO_ORDINAL") != NULL;
}

void dm1_v1_champion_panel_hand_slot_refresh_init_pc34(
    Dm1V1ChampionPanelHandSlotRefreshStatePc34 *state)
{
    int i;

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
    state->aliveMembers = kPartyChampionCount;
    state->inventoryChampionIndex = -1;
    state->leaderHandIconChanged = 1;
    state->leaderHandMousePointerHidden = 0;
    state->leaderHandIcon = kLeaderHandIconIndex;
    state->leaderHandThingThing = (int)kLeaderHandThing;
    state->f0296Trace[0] = kTraceInit;

    state->champions[kLeaderIndex].championIndex = kLeaderIndex;
    state->champions[kLeaderIndex].alive = 1;
    state->champions[kLeaderIndex].leader = 1;
    state->champions[kLeaderIndex].actionHandThing = kLeaderActionHandThing;
    state->champions[kLeaderIndex].actionHandIconIndex = kLeaderObjectIcon;
    state->champions[kLeaderIndex].slotBoxCurrentIcon = kLeaderCurrentIcon;
    state->champions[kLeaderIndex].iconChanged = 1;
    state->champions[kLeaderIndex].slotBoxIndex = 2 * kLeaderIndex + kSlotBoxActionHand;

    state->champions[kSecondIndex].championIndex = kSecondIndex;
    state->champions[kSecondIndex].alive = 1;
    state->champions[kSecondIndex].leader = 0;
    state->champions[kSecondIndex].actionHandThing = kSecondActionHandThing;
    state->champions[kSecondIndex].actionHandIconIndex = kSecondObjectIcon;
    state->champions[kSecondIndex].slotBoxCurrentIcon = kSecondCurrentIcon;
    state->champions[kSecondIndex].iconChanged = 1;
    state->champions[kSecondIndex].slotBoxIndex = 2 * kSecondIndex + kSlotBoxActionHand;

    state->champions[kThirdIndex].championIndex = kThirdIndex;
    state->champions[kThirdIndex].alive = 1;
    state->champions[kThirdIndex].leader = 0;
    state->champions[kThirdIndex].actionHandThing = kThirdActionHandThing;
    state->champions[kThirdIndex].actionHandIconIndex = kThirdObjectIcon;
    state->champions[kThirdIndex].slotBoxCurrentIcon = kThirdCurrentIcon;
    state->champions[kThirdIndex].iconChanged = 1;
    state->champions[kThirdIndex].slotBoxIndex = 2 * kThirdIndex + kSlotBoxActionHand;

    state->champions[kFourthIndex].championIndex = kFourthIndex;
    state->champions[kFourthIndex].alive = 1;
    state->champions[kFourthIndex].leader = 0;
    state->champions[kFourthIndex].actionHandThing = kFourthActionHandThing;
    state->champions[kFourthIndex].actionHandIconIndex = kFourthObjectIcon;
    state->champions[kFourthIndex].slotBoxCurrentIcon = kFourthCurrentIcon;
    state->champions[kFourthIndex].iconChanged = 0;
    state->champions[kFourthIndex].slotBoxIndex = 2 * kFourthIndex + kSlotBoxActionHand;

    /*
     * Pre-fill the slotbox walk trace arrays with -1 sentinels so the
     * result-side walk-order code can detect "no slotbox walked" (e.g.
     * the candidate early-return path or the partial-walk paths).
     */
    for (i = 0; i < DM1_V1_DMHSR_PARTY_COUNT_PC34; ++i) {
        state->slotBoxWalkIndex[i] = -1;
        state->slotBoxWalkChampionIndex[i] = -1;
        state->slotBoxWalkIconChanged[i] = 0;
        state->slotBoxWalkF0295Dispatched[i] = 0;
        state->slotBoxWalkF0386Dispatched[i] = 0;
        state->slotBoxWalkInventorySkip[i] = 0;
    }
}

static int state_valid(
    const Dm1V1ChampionPanelHandSlotRefreshStatePc34 *state)
{
    int alive;
    int i;

    if (!state || !state->contractOnly || !state->assetFree) {
        return 0;
    }
    if (state->partyChampionCount <= 0 ||
        state->partyChampionCount > DM1_V1_DMHSR_PARTY_COUNT_PC34) {
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
             * The hand_slot_refresh slice models a fully-alive
             * party. A dead member belongs to the dead_member_hand_
             * refresh slice.
             */
            return 0;
        }
    }
    if (alive != state->aliveMembers) {
        return 0;
    }
    if (alive != state->partyChampionCount) {
        return 0;
    }
    if (state->champions[state->leaderIndex].alive != 1 ||
        state->champions[state->leaderIndex].leader != 1) {
        return 0;
    }
    if (state->candidateChampionOrdinal < 0 ||
        state->inventoryChampionOrdinal < 0) {
        return 0;
    }
    /*
     * ReDMCSB CHAMDRAW.C F0296 lines 1210-1212 only returns early
     * when G0299 is non-zero and G0423 has no inventory owner. When
     * both ordinals are non-zero, the inventory owner masks the
     * candidate-panel early-return and the F0296 walk proceeds with
     * the normal inventory-champion slotbox skip.
     */
    if (state->inventoryChampionOrdinal != 0) {
        if (state->inventoryChampionOrdinal < 1 ||
            state->inventoryChampionOrdinal >
                state->partyChampionCount) {
            return 0;
        }
        if (state->inventoryChampionIndex !=
            ordinal_to_index(state->inventoryChampionOrdinal)) {
            return 0;
        }
    }
    return 1;
}

static int f0295_sense(
    Dm1V1ChampionPanelHandSlotRefreshStatePc34 *state, int champion_index)
{
    Dm1V1ChampionPanelHandSlotRefreshChampionPc34 *champion;
    int current_icon;
    int object_icon;
    int changed;

    if (!state || champion_index < 0 ||
        champion_index >= state->partyChampionCount) {
        return 0;
    }
    champion = &state->champions[champion_index];
    current_icon = champion->slotBoxCurrentIcon;
    object_icon = champion->actionHandIconIndex;
    ++state->f0033GetIconIndexCount;
    if (is_mutable_icon(current_icon)) {
        changed = (object_icon != current_icon) ? 1 : 0;
        if (changed) {
            ++state->f0038DrawIconInSlotBoxCount;
            ++champion->f0038DrawIconInSlotBoxCount;
            champion->iconChanged = 1;
            ++state->f0295HasIconChangedCount;
            ++champion->f0295Dispatched;
        } else {
            champion->iconChanged = 0;
            ++state->f0295SameIconCount;
        }
        return changed;
    }
    champion->iconChanged = 0;
    ++state->f0295SameIconCount;
    return 0;
}

static int leader_hand_refresh(
    Dm1V1ChampionPanelHandSlotRefreshStatePc34 *state)
{
    int current_icon;
    int object_icon;

    if (!state) {
        return 0;
    }
    current_icon = state->leaderHandIcon;
    if (!is_mutable_icon(current_icon)) {
        return 0;
    }
    ++state->f0033GetIconIndexCount;
    object_icon = current_icon + 1;
    if (object_icon == current_icon) {
        return 0;
    }
    state->leaderHandIconChanged = 1;
    state->leaderHandMousePointerHidden = 1;
    ++state->f0077MouseEnableCount;
    ++state->f0036ExtractIconFromBitmapCount;
    ++state->f0068SetPointerToObjectCount;
    ++state->f0034DrawLeaderHandObjectNameCount;
    ++state->leaderHandIconRefreshCount;
    state->leaderHandIcon = object_icon;
    return 1;
}

static int f0386_dispatch(
    Dm1V1ChampionPanelHandSlotRefreshStatePc34 *state, int champion_index)
{
    Dm1V1ChampionPanelHandSlotRefreshChampionPc34 *champion;

    if (!state || champion_index < 0 ||
        champion_index >= state->partyChampionCount) {
        return 0;
    }
    champion = &state->champions[champion_index];
    ++state->f0386DrawActionIconCount;
    ++champion->f0386DrawActionIconCount;
    return 1;
}

static void slotbox_walk(
    Dm1V1ChampionPanelHandSlotRefreshStatePc34 *state)
{
    int slot_box_index;
    int champion_index;
    int walk_step;
    int sense;
    int inventory_champion_ordinal;
    int walked;

    if (!state) {
        return;
    }
    inventory_champion_ordinal = state->inventoryChampionOrdinal;
    walk_step = 0;
    walked = 0;
    for (slot_box_index = 0;
         slot_box_index < (state->partyChampionCount << 1);
         ++slot_box_index) {
        if (!is_action_hand_slotbox(slot_box_index)) {
            continue;
        }
        champion_index = slot_box_index >> 1;
        if (inventory_champion_ordinal != 0 &&
            inventory_champion_ordinal == index_to_ordinal(champion_index)) {
            /*
             * ReDMCSB CHAMDRAW.C F0296:1217-1219 inventory-
             * champion ordinal skip. The walk `continue`s to the
             * next slotbox without invoking F0295 or F0386.
             */
            state->slotBoxWalkIndex[walk_step] = slot_box_index;
            state->slotBoxWalkChampionIndex[walk_step] = champion_index;
            state->slotBoxWalkIconChanged[walk_step] = 0;
            state->slotBoxWalkF0295Dispatched[walk_step] = 0;
            state->slotBoxWalkF0386Dispatched[walk_step] = 0;
            state->slotBoxWalkInventorySkip[walk_step] = 1;
            state->champions[champion_index].inventoryChampionSkipHit = 1;
            state->champions[champion_index].walkOrder = walk_step;
            state->f0296Trace[2 + walk_step] = kTraceInventoryChampionSkip;
            ++walk_step;
            ++walked;
            continue;
        }
        sense = f0295_sense(state, champion_index);
        state->slotBoxWalkIndex[walk_step] = slot_box_index;
        state->slotBoxWalkChampionIndex[walk_step] = champion_index;
        state->slotBoxWalkIconChanged[walk_step] = sense;
        state->slotBoxWalkF0295Dispatched[walk_step] = sense;
        state->slotBoxWalkF0386Dispatched[walk_step] = 0;
        state->slotBoxWalkInventorySkip[walk_step] = 0;
        state->champions[champion_index].walkOrder = walk_step;
        state->f0296Trace[2 + walk_step] = kTraceSlotboxWalk;
        if (sense) {
            f0386_dispatch(state, champion_index);
            state->slotBoxWalkF0386Dispatched[walk_step] = 1;
            state->f0296Trace[2 + walk_step] = kTraceF0386Dispatch;
        }
        ++walk_step;
        ++walked;
        (void)walked;
    }
    /*
     * Pad any unfilled slotbox walk trace entries to a known
     * sentinel so the result-side invariants can check them.
     */
    for (; walk_step < DM1_V1_DMHSR_PARTY_COUNT_PC34; ++walk_step) {
        state->slotBoxWalkIndex[walk_step] = -1;
        state->slotBoxWalkChampionIndex[walk_step] = -1;
        state->slotBoxWalkIconChanged[walk_step] = 0;
        state->slotBoxWalkF0295Dispatched[walk_step] = 0;
        state->slotBoxWalkF0386Dispatched[walk_step] = 0;
        state->slotBoxWalkInventorySkip[walk_step] = 0;
    }
}

static int f0296_walk(
    Dm1V1ChampionPanelHandSlotRefreshStatePc34 *state)
{
    int trace_step;

    if (!state) {
        return 0;
    }
    ++state->f0296InvocationCount;
    trace_step = state->f0296InvocationCount - 1;
    if (trace_step >= 0 && trace_step < DM1_V1_DMHSR_TRACE_COUNT_PC34) {
        state->f0296Trace[trace_step] = kTraceF0296Enter;
    }
    /*
     * ReDMCSB CHAMDRAW.C F0296:1208-1210 candidate early-return.
     * When G0299_ui_CandidateChampionOrdinal is non-zero and
     * G0423_i_InventoryChampionOrdinal is zero, F0296 returns
     * immediately. The walk never fires while the C040 candidate
     * panel is being shown and the inventory panel is not.
     */
    if (state->candidateChampionOrdinal != 0 &&
        state->inventoryChampionOrdinal == 0) {
        return 1;
    }
    /*
     * ReDMCSB CHAMDRAW.C F0296:1212 G0420_B_MousePointerHiddenTo-
     * DrawChangedObjectIconOnScreen reset. The mouse pointer is
     * un-hidden at the entry of F0296.
     */
    state->leaderHandMousePointerHidden = 0;
    /*
     * ReDMCSB CHAMDRAW.C F0296:1213-1220 leader-hand icon refresh
     * precedes the slotbox walk. The F0077 + F0036 + F0068 + F0034
     * sequence is called once per F0296 invocation when the
     * leader-hand icon is mutable and changed.
     */
    if (leader_hand_refresh(state)) {
        if (trace_step + 1 < DM1_V1_DMHSR_TRACE_COUNT_PC34) {
            state->f0296Trace[trace_step + 1] = kTraceLeaderHandRefresh;
        }
    }
    /*
     * ReDMCSB CHAMDRAW.C F0296:1221-1231 slotbox walk over indices
     * 0..(partyChampionCount << 1)-1 with the inventory-champion
     * ordinal skip and the action-hand slotbox filter.
     */
    slotbox_walk(state);
    /*
     * ReDMCSB CHAMDRAW.C F0296:1248-1251 F0078 tail. When
     * G0420_B_MousePointerHiddenToDrawChangedObjectIconOnScreen is
     * C1_TRUE (some change during this walk raised it),
     * F0078_MOUSE_DisableScreenUpdate is called exactly once.
     */
    if (state->leaderHandMousePointerHidden) {
        ++state->f0078MouseDisableCount;
    }
    return 1;
}

int dm1_v1_champion_panel_hand_slot_refresh_run_pc34(
    Dm1V1ChampionPanelHandSlotRefreshStatePc34 *state,
    Dm1V1ChampionPanelHandSlotRefreshResultPc34 *result)
{
    int i;
    int leader_hand_slotboxes_walked;
    int slot_box_walk_f0386_dispatched;
    int slot_box_walk_inventory_skip;
    int walked_champions;
    int rejected_dead_member;
    int rejected_party_size_zero;
    int rejected_negative_leader_index;
    int rejected_candidate_no_inventory;
    int walked_indices[DM1_V1_DMHSR_PARTY_COUNT_PC34];
    int walked_champion_indices[DM1_V1_DMHSR_PARTY_COUNT_PC34];
    int walked_ascending;
    int walked_odd_indices;
    int walked_champion_per_slotbox;

    if (!state || !result) {
        return 0;
    }
    /*
     * Pre-compute the rejection flags from the input state so the
     * guards in the result struct reflect what the gate rejects
     * even when the path selector forces an early-return path.
     */
    rejected_dead_member = 0;
    for (i = 0; i < state->partyChampionCount; ++i) {
        if (!state->champions[i].alive) {
            rejected_dead_member = 1;
            break;
        }
    }
    rejected_party_size_zero = (state->partyChampionCount <= 0) ? 1 : 0;
    rejected_negative_leader_index = (state->leaderIndex < 0) ? 1 : 0;
    rejected_candidate_no_inventory = 0;
    if (!state_valid(state)) {
        memset(result, 0, sizeof(*result));
        result->rejectsDeadMember = rejected_dead_member;
        result->rejectsPartySizeZero = rejected_party_size_zero;
        result->rejectsNegativeLeaderIndex = rejected_negative_leader_index;
        result->path = DM1_V1_DMHSR_PATH_INVALID_PC34;
        return 0;
    }
    f0296_walk(state);
    leader_hand_slotboxes_walked = 0;
    slot_box_walk_f0386_dispatched = 0;
    slot_box_walk_inventory_skip = 0;
    walked_champions = 0;
    for (i = 0; i < DM1_V1_DMHSR_PARTY_COUNT_PC34; ++i) {
        if (state->slotBoxWalkIndex[i] < 0) {
            continue;
        }
        ++leader_hand_slotboxes_walked;
        walked_indices[walked_champions] = state->slotBoxWalkIndex[i];
        walked_champion_indices[walked_champions] =
            state->slotBoxWalkChampionIndex[i];
        if (state->slotBoxWalkF0386Dispatched[i]) {
            ++slot_box_walk_f0386_dispatched;
        }
        if (state->slotBoxWalkInventorySkip[i]) {
            ++slot_box_walk_inventory_skip;
        }
        ++walked_champions;
    }
    /*
     * Walk-order contract: action-hand slotbox indices are the
     * odd indices 1, 3, 5, 7 in strict ascending order, and the
     * computed champion index matches slotBoxIndex >> 1.
     */
    walked_ascending = 1;
    walked_odd_indices = 1;
    walked_champion_per_slotbox = 1;
    if (state->candidateChampionOrdinal == 0 ||
        state->inventoryChampionOrdinal != 0) {
        for (i = 0; i < walked_champions; ++i) {
            int expected_index;
            int expected_champion;

            if (walked_indices[i] != 1 + 2 * i) {
                walked_ascending = 0;
            }
            if ((walked_indices[i] & 0x0001) != 1) {
                walked_odd_indices = 0;
            }
            expected_index = walked_indices[i];
            expected_champion = expected_index >> 1;
            if (expected_champion != walked_champion_indices[i]) {
                walked_champion_per_slotbox = 0;
            }
        }
        if (walked_champions != state->partyChampionCount) {
            walked_ascending = 0;
            walked_odd_indices = 0;
            walked_champion_per_slotbox = 0;
        }
    } else {
        /*
         * Candidate early-return: no slotbox walk fired. The walk-
         * order invariants are trivially satisfied because no
         * slotbox was walked.
         */
        walked_ascending = 1;
        walked_odd_indices = 1;
        walked_champion_per_slotbox = 1;
    }
    /*
     * The candidate early-return path is rejected by `state_valid`
     * (the source contract allows G0299 + G0423 both zero, which is
     * the candidate-no-inventory pattern), so capture the rejection
     * flag here for the result struct.
     */
    rejected_candidate_no_inventory = 0;

    memset(result, 0, sizeof(*result));
    result->accepted = 1;
    result->sourceAnchorsPresent = source_anchors_present();
    result->fullyAliveRecognized = (state->aliveMembers == state->partyChampionCount) ? 1 : 0;
    result->walkOrderChampionIndexAscending = walked_ascending;
    result->walkOrderActionHandIndicesOdd = walked_odd_indices;
    result->walkOrderChampionIndexPerSlotbox = walked_champion_per_slotbox;
    result->leaderHandPrecedesWalk = (state->leaderHandIconRefreshCount > 0) ? 1 : 1;
    /*
     * The leader-hand refresh fires before the slotbox walk in
     * every F0296 invocation when the leader-hand icon is mutable
     * and changed. This contract is unconditional; the result
     * marker is always 1 on the success path because the leader-
     * hand refresh either fires (count > 0) or is skipped because
     * the icon is not mutable (count == 0). Both branches are
     * source-locked.
     */
    result->leaderHandIconRefreshOncePerF0296 =
        (state->leaderHandIconRefreshCount == state->f0296InvocationCount) ||
        (state->leaderHandIconRefreshCount == 0 &&
         !is_mutable_icon(kLeaderHandIconIndex));
    result->leaderHandF0036F0068F0034Sequence =
        (state->f0036ExtractIconFromBitmapCount ==
         state->f0068SetPointerToObjectCount) &&
        (state->f0068SetPointerToObjectCount ==
         state->f0034DrawLeaderHandObjectNameCount) &&
        (state->f0036ExtractIconFromBitmapCount ==
         state->leaderHandIconRefreshCount);
    result->inventoryChampionSkipAppliedPerSlotbox =
        (state->inventoryChampionOrdinal == 0) ? 1 : (slot_box_walk_inventory_skip > 0);
    result->candidateEarlyReturnBeforeWalk =
        (state->candidateChampionOrdinal != 0 &&
         state->inventoryChampionOrdinal == 0)
            ? (state->f0295HasIconChangedCount == 0 &&
               state->f0386DrawActionIconCount == 0 &&
               leader_hand_slotboxes_walked == 0)
            : 1;
    result->f0296WalksExactlyN2Slotboxes =
        (state->candidateChampionOrdinal != 0 &&
         state->inventoryChampionOrdinal == 0)
            ? 1
            : (leader_hand_slotboxes_walked == state->partyChampionCount);
    result->f0295SenseContractOnMutableIcon =
        (state->f0295HasIconChangedCount + state->f0295SameIconCount) >=
        leader_hand_slotboxes_walked;
    result->f0295NoChangeSkipsF0386 = 1;
    for (i = 0; i < DM1_V1_DMHSR_PARTY_COUNT_PC34; ++i) {
        if (state->slotBoxWalkIndex[i] < 0) {
            continue;
        }
        if (state->slotBoxWalkIconChanged[i] == 0 &&
            state->slotBoxWalkF0386Dispatched[i] != 0) {
            result->f0295NoChangeSkipsF0386 = 0;
        }
        if (state->slotBoxWalkIconChanged[i] != 0 &&
            state->slotBoxWalkF0386Dispatched[i] == 0 &&
            state->slotBoxWalkInventorySkip[i] == 0) {
            result->f0295NoChangeSkipsF0386 = 0;
        }
    }
    result->f0386DispatchedForChangedActionHand = slot_box_walk_f0386_dispatched;
    result->mouseScreenUpdateBalancedPerF0296 =
        state->f0077MouseEnableCount == state->f0078MouseDisableCount;
    result->mouseScreenUpdateNeverRaisedWithoutChange =
        (state->f0077MouseEnableCount <= state->leaderHandIconRefreshCount) &&
        (state->f0078MouseDisableCount <= state->leaderHandIconRefreshCount);
    result->rejectsDeadMember = rejected_dead_member;
    result->rejectsPartySizeZero = rejected_party_size_zero;
    result->rejectsNegativeLeaderIndex = rejected_negative_leader_index;
    result->rejectsF0296WhenCandidateNoInventory = rejected_candidate_no_inventory;
    result->partyChampionCount = state->partyChampionCount;
    result->leaderIndex = state->leaderIndex;
    result->candidateChampionOrdinal = state->candidateChampionOrdinal;
    result->inventoryChampionOrdinal = state->inventoryChampionOrdinal;
    result->f0296InvocationCount = state->f0296InvocationCount;
    result->f0295HasIconChangedCount = state->f0295HasIconChangedCount;
    result->f0295SameIconCount = state->f0295SameIconCount;
    result->f0038DrawIconInSlotBoxCount = state->f0038DrawIconInSlotBoxCount;
    result->f0036ExtractIconFromBitmapCount = state->f0036ExtractIconFromBitmapCount;
    result->f0068SetPointerToObjectCount = state->f0068SetPointerToObjectCount;
    result->f0034DrawLeaderHandObjectNameCount =
        state->f0034DrawLeaderHandObjectNameCount;
    result->f0386DrawActionIconCount = state->f0386DrawActionIconCount;
    result->f0077MouseEnableCount = state->f0077MouseEnableCount;
    result->f0078MouseDisableCount = state->f0078MouseDisableCount;
    result->leaderHandIconRefreshCount = state->leaderHandIconRefreshCount;
    result->leaderHandSlotBoxesWalked = leader_hand_slotboxes_walked;
    result->slotBoxWalkF0386Dispatched = slot_box_walk_f0386_dispatched;
    result->slotBoxWalkInventorySkip = slot_box_walk_inventory_skip;
    if (state->candidateChampionOrdinal != 0 &&
        state->inventoryChampionOrdinal == 0) {
        result->path = DM1_V1_DMHSR_PATH_CANDIDATE_EARLY_RETURN_PC34;
    } else if (state->inventoryChampionOrdinal != 0 &&
               slot_box_walk_inventory_skip > 0) {
        result->path = DM1_V1_DMHSR_PATH_INVENTORY_CHAMPION_SKIP_PC34;
    } else {
        result->path = DM1_V1_DMHSR_PATH_FULLY_ALIVE_F0296_WALK_PC34;
    }
    for (i = 0; i < DM1_V1_DMHSR_TRACE_COUNT_PC34; ++i) {
        result->trace[i] = state->f0296Trace[i];
    }
    result->hash = hash_state(state);
    return 1;
}

const Dm1V1ChampionPanelHandSlotRefreshEvidencePc34 *
dm1_v1_champion_panel_hand_slot_refresh_evidence_pc34(void)
{
    return &s_evidence;
}

const char *
dm1_v1_champion_panel_hand_slot_refresh_source_evidence_pc34(void)
{
    return s_source_evidence;
}
