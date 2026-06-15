/*
 * DM1 V1 champion-panel dead-member hand refresh gate implementation.
 *
 * Source-lock anchors (ReDMCSB WIP 20210206):
 *  - CHAMDRAW.C F0295_CHAMPION_HasObjectIconInSlotBoxChanged:1153-1182
 *  - CHAMDRAW.C F0296_CHAMPION_DrawChangedObjectIcons:1184-1262
 *  - ACTIDRAW.C  F0386_MENUS_DrawActionIcon:201-326
 *  - CHAMDRAW.C F0292_CHAMPION_DrawState:816-839 (dead-status-box branch)
 *  - CHAMDRAW.C F0292:784 (dead-champion short-circuit in status box)
 *  - OBJECT.C   F0033_OBJECT_GetIconIndex
 *  - OBJECT.C   F0038_OBJECT_DrawIconInSlotBox
 *  - DEFS.H     781 C01_SLOT_ACTION_HAND
 *  - DEFS.H     1878 M070_HAND_SLOT_INDEX
 *  - DEFS.H     1950 C195_ICON_POTION_EMPTY_FLASK
 *  - DEFS.H     1952 C201_ICON_ACTION_ICON_EMPTY_HAND
 *  - DEFS.H     2078 C00_COLOR_BLACK
 *  - DEFS.H     2172 C008_GRAPHIC_STATUS_BOX_DEAD_CHAMPION
 *  - DEFS.H     3770 C089_ZONE_ACTION_AREA_CHAMPION_0_ACTION
 *
 * No bitmap sampling, no GRAPHICS.DAT/DUNGEON.DAT load, no real-asset
 * parity claim. The contract models the F0296 walk + F0295 icon
 * detector + F0386 dead-member action-area C00_COLOR_BLACK fill +
 * F0292 dead-status-box branch F0386 dispatch without sampling real
 * bitmaps.
 */

#include "firestaff/dm1/v1/champion_panel/dead_member_hand_refresh_pc34_compat.h"

#include <string.h>

enum {
    kPartyChampionCount = 4,
    kLeaderIndex = 0,
    kSecondIndex = 1,
    kThirdIndex = 2,
    kDeadMemberIndex = 3,

    kSlotBoxReadyHand = 0,
    kSlotBoxActionHand = 1,

    kF0296InvocationCount = 2,
    kF0292InvocationCount = 2,
    kMutableIconLower = 0,
    kMutableIconUpper = 31,
    kMutableIconPotionLower = 148,
    kMutableIconPotionUpper = 163,
    kMutableIconEmptyFlask = 195,

    kLeaderActionHandThing = 0x0420u,
    kSecondActionHandThing = 0x0421u,
    kThirdActionHandThing = 0x0422u,
    kDeadActionHandThing = 0x0423u,

    kLeaderCurrentIcon = 1,
    kSecondCurrentIcon = 1,
    kThirdCurrentIcon = 1,
    kDeadCurrentIcon = 1,

    kLeaderObjectIcon = 12,
    kSecondObjectIcon = 12,
    kThirdObjectIcon = 12,
    kDeadObjectIcon = 12,

    kTraceInit = 0,
    kTraceF0296Enter = 1,
    kTraceF0295SenseDeadMember = 2,
    kTraceF0295SenseLiveMember = 3,
    kTraceF0386BlackFill = 4,
    kTraceF0386ActionIcon = 5,
    kTraceF0292StatusBox = 6,
    kTraceSettle = 7
};

/*
 * ReDMCSB source-lock contract only.
 *
 * CHAMDRAW.C F0295_CHAMPION_HasObjectIconInSlotBoxChanged:1153-1182
 * compares the slotbox's currently displayed icon (read via
 * F0039_OBJECT_GetIconIndexInSlotBox) against the slot's object icon
 * (read via F0033_OBJECT_GetIconIndex(thing)); when the icons differ
 * AND the current icon is in a mutable range (junk C000..C031, potions
 * C148..C163, or the C195 empty flask), F0295 calls
 * F0038_OBJECT_DrawIconInSlotBox to repaint the slotbox and returns
 * C1_TRUE. The mutable icon guard is a CHAMPION.C + OBJECT.C anchor
 * for which icons are "live" enough to be redrawn on icon change.
 *
 * CHAMDRAW.C F0296_CHAMPION_DrawChangedObjectIcons:1184-1262 walks
 * slotbox indices 0..(G0305_ui_PartyChampionCount << 1)-1 with
 * L0885_i_ChampionIndex = slotBoxIndex >> 1. For each slotbox at
 * index 2*idx+1 (the action hand of champion idx), the loop calls
 * F0295 against
 *   M516_CHAMPIONS[L0885_i_ChampionIndex].Slots[C01_SLOT_ACTION_HAND]
 * and, if F0295 returns C1_TRUE, dispatches
 *   F0386_MENUS_DrawActionIcon(L0885_i_ChampionIndex) at line 1231.
 * The F0296 walk does NOT skip a dead champion — it walks the full
 * 2 * G0305_ui_PartyChampionCount range. The dead-member hand
 * refresh is the F0295 + F0386 cycle for the dead champion's
 * action hand slotbox.
 *
 * ACTIDRAW.C F0386_MENUS_DrawActionIcon:201-326 begins with the
 * G0509_B_ActionAreaContainsIcons short-circuit (line 226); the
 * dead-member guard is at lines 234-242 where
 *   if (!L1183_ps_Champion->CurrentHealth) {
 *     F0733_FillZoneByIndex(P0760_ui_ChampionIndex +
 *                            C089_ZONE_ACTION_AREA_CHAMPION_0_ACTION,
 *                          C00_COLOR_BLACK);
 *     return;
 *   }
 * The C00_COLOR_BLACK fill writes the entire 22x19 (or
 * G2075_ObjectIconWidth x G2076_ObjectIconHeight) action-area
 * rectangle for the dead champion, then returns. The action icon
 * blit at lines 257-326 (the live-member path) is unreachable
 * while CurrentHealth == 0.
 *
 * CHAMDRAW.C F0292_CHAMPION_DrawState:816-839 is the
 * dead-status-box branch: it draws
 * C008_GRAPHIC_STATUS_BOX_DEAD_CHAMPION into the
 * C151+championIndex status-box zone, prints the champion name in
 * C13_COLOR_LIGHTEST_GRAY on a C01_COLOR_DARK_GRAY background, and
 * calls F0386_MENUS_DrawActionIcon(P0615_ui_ChampionIndex) at line
 * 835 before the T0292042 return. The F0292 call site for the
 * dead champion goes through the F0386 dead-member guard.
 *
 * The CHAMDRAW.C F0292:784 fork fires when L0865_ps_Champion->
 * CurrentHealth == 0 and the MASK0x1000_STATUS_BOX is set; the
 * dead branch takes the F0386 dispatch and never reaches F0354.
 *
 * DEFS.H:781 C01_SLOT_ACTION_HAND is the action-hand slot ordinal,
 * DEFS.H:1878 M070_HAND_SLOT_INDEX(slotboxindex) returns
 * (slotboxindex & 0x0001) so that M070_HAND_SLOT_INDEX(2*idx+1) ==
 * C01_SLOT_ACTION_HAND. DEFS.H:1950 C195_ICON_POTION_EMPTY_FLASK
 * and DEFS.H:1952 C201_ICON_ACTION_ICON_EMPTY_HAND are the
 * mutable-icon anchors. DEFS.H:2078 C00_COLOR_BLACK is the fill
 * color used by the F0386 dead-member guard. DEFS.H:2172
 * C008_GRAPHIC_STATUS_BOX_DEAD_CHAMPION is the dead-status-box
 * graphic drawn by the F0292 dead branch. DEFS.H:3770
 * C089_ZONE_ACTION_AREA_CHAMPION_0_ACTION is the action-area zone
 * base; the action-area zone for champion idx is
 * C089 + idx (0..3).
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
    "DrawChangedObjectIcons:1184-1262 walks slotbox indices 0..(G0305_ui_"
    "PartyChampionCount << 1)-1 with L0885_i_ChampionIndex = slotBoxIndex >> "
    "1; for each slotbox at index 2*idx+1 (the action hand of champion idx), "
    "the loop calls F0295 against M516_CHAMPIONS[L0885_i_ChampionIndex]."
    "Slots[C01_SLOT_ACTION_HAND] and, if F0295 returns C1_TRUE, dispatches "
    "F0386_MENUS_DrawActionIcon(L0885_i_ChampionIndex) at line 1231. The "
    "F0296 walk does NOT skip a dead champion; it walks the full 2 * G0305_"
    "ui_PartyChampionCount range. ACTIDRAW.C F0386_MENUS_DrawActionIcon:201-"
    "326 begins with the G0509_B_ActionAreaContainsIcons short-circuit at "
    "line 226; the dead-member guard is at lines 234-242 where "
    "if (!L1183_ps_Champion->CurrentHealth) { F0733_FillZoneByIndex(P0760_"
    "ui_ChampionIndex + C089_ZONE_ACTION_AREA_CHAMPION_0_ACTION, C00_COLOR_"
    "BLACK); return; }. The C00_COLOR_BLACK fill writes the entire action-"
    "area rectangle for the dead champion, then returns. The action icon "
    "blit at lines 257-326 (the live-member path) is unreachable while "
    "CurrentHealth == 0. CHAMDRAW.C F0292_CHAMPION_DrawState:816-839 is the "
    "dead-status-box branch: it draws C008_GRAPHIC_STATUS_BOX_DEAD_CHAMPION "
    "into the C151+championIndex status-box zone, prints the champion name, "
    "and calls F0386_MENUS_DrawActionIcon(P0615_ui_ChampionIndex) at line "
    "835 before the T0292042 return. CHAMDRAW.C F0292:784 forks the dead "
    "branch when CurrentHealth == 0 and MASK0x1000_STATUS_BOX is set; the "
    "dead branch takes the F0386 dispatch and never reaches F0354. DEFS.H:"
    "781 C01_SLOT_ACTION_HAND, 1878 M070_HAND_SLOT_INDEX, 1950 C195_ICON_"
    "POTION_EMPTY_FLASK, 1952 C201_ICON_ACTION_ICON_EMPTY_HAND, 2078 C00_"
    "COLOR_BLACK, 2172 C008_GRAPHIC_STATUS_BOX_DEAD_CHAMPION, 3770 C089_ZONE_"
    "ACTION_AREA_CHAMPION_0_ACTION pin the constants.";

static const Dm1V1ChampionPanelDeadMemberHandRefreshEvidencePc34 s_evidence = {
    "ReDMCSB CHAMDRAW.C F0295_CHAMPION_HasObjectIconInSlotBoxChanged:1153-1182",
    "ReDMCSB CHAMDRAW.C F0296_CHAMPION_DrawChangedObjectIcons:1184-1262",
    "ReDMCSB ACTIDRAW.C F0386_MENUS_DrawActionIcon:201-326",
    "ReDMCSB CHAMDRAW.C F0292_CHAMPION_DrawState:816-839 dead-status-box branch",
    "ReDMCSB OBJECT.C F0033_OBJECT_GetIconIndex(thing) for slot thing -> icon",
    "ReDMCSB OBJECT.C F0038_OBJECT_DrawIconInSlotBox(slotBoxIndex, iconIndex)",
    "ReDMCSB DEFS.H:781 C01_SLOT_ACTION_HAND, 1878 M070_HAND_SLOT_INDEX, 1950 C195_ICON_POTION_EMPTY_FLASK, 1952 C201_ICON_ACTION_ICON_EMPTY_HAND, 2078 C00_COLOR_BLACK, 2172 C008_GRAPHIC_STATUS_BOX_DEAD_CHAMPION, 3770 C089_ZONE_ACTION_AREA_CHAMPION_0_ACTION",
    "ReDMCSB C089+championIndex action-area zone for champion 0..3",
    "contract-only walk of F0296/F0295/F0386 dead-member action-hand refresh and F0292 dead-status-box F0386 dispatch; no real M11 graphics, no real bitmaps, no asset load",
    "no GRAPHICS.DAT, no DUNGEON.DAT, no real-asset bitmap parity claim",
    "Non-overlap marker: pass dead_member_hand_refresh covers F0296 + F0295 + F0386 dead-member action-hand refresh and F0292 dead-status-box F0386 dispatch; disjoint from champion_panel_portrait_box_blit_gate (F0292+F0354 dispatch + F0254 secondary + F0293 dispatch order), champion_panel_portrait_box_redraw_states (F0296 changed-icon scan + chest scan, no dead-champion handling), champion_panel_portrait_state_redraw (F0292 dead-status-box branch state redraw order, not F0296 dead-member hand refresh), champion_panel_hand_slot_priority (CHAMPION.C F0302 dispatch, not F0296 dead-member hand refresh), mirror_candidate_icon_refresh (F0296 leader hand candidate-icon scan, not F0296 dead-member hand refresh), champion_panel_spell_area_overlay (F0394 dead-champion reject for spell area, not F0296 dead-member hand refresh), mirror_candidate_c045_accept_dead_owner_guard (C045 accept while owner dead, not F0296 dead-member hand refresh), inventory_champion_switch_hand_carry (dead target leaves leader hand unchanged, not F0296 dead-member hand refresh), the F0107/F0108/chest-scroll-wheel/viewport/integrated family, and the per-state redraw + per-action-hand slot-box dispatch family."
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
    const Dm1V1ChampionPanelDeadMemberHandRefreshStatePc34 *state)
{
    uint32_t hash = UINT32_C(2166136261);
    int i;

    hash = hash_step(hash, (unsigned int)state->partyChampionCount);
    hash = hash_step(hash, (unsigned int)state->leaderIndex);
    hash = hash_step(hash, (unsigned int)state->deadMemberIndex);
    hash = hash_step(hash, (unsigned int)state->aliveMembers);
    hash = hash_step(hash, (unsigned int)state->f0296InvocationCount);
    hash = hash_step(hash, (unsigned int)state->f0292InvocationCount);
    hash = hash_step(hash, (unsigned int)state->f0295HasIconChangedCount);
    hash = hash_step(hash, (unsigned int)state->f0295SameIconCount);
    hash = hash_step(hash, (unsigned int)state->f0033GetIconIndexCount);
    hash = hash_step(hash, (unsigned int)state->f0038DrawIconInSlotBoxCount);
    hash = hash_step(hash, (unsigned int)state->f0386DrawActionIconCount);
    hash = hash_step(hash, (unsigned int)state->f0077MouseEnableCount);
    hash = hash_step(hash, (unsigned int)state->f0078MouseDisableCount);
    for (i = 0; i < DM1_V1_DMHR_PARTY_COUNT_PC34; ++i) {
        hash = hash_step(hash, (unsigned int)state->leaderHandThingThing[i]);
        hash = hash_step(hash, (unsigned int)state->leaderHandObjectIcon[i]);
        hash = hash_step(hash,
                         (unsigned int)state->leaderHandSlotBoxCurrentIcon[i]);
        hash = hash_step(hash, (unsigned int)state->leaderHandIconChanged[i]);
        hash = hash_step(
            hash, (unsigned int)state->leaderHandBlackFillCount[i]);
        hash = hash_step(
            hash, (unsigned int)state->leaderHandActionIconBlitCount[i]);
        hash = hash_step(hash, (unsigned int)state->champions[i].alive);
        hash = hash_step(hash, (unsigned int)state->champions[i].leader);
        hash = hash_step(hash, (unsigned int)state->champions[i].actionHandThing);
        hash = hash_step(hash, (unsigned int)state->champions[i].objectIconIndex);
        hash = hash_step(
            hash, (unsigned int)state->champions[i].slotBoxCurrentIcon);
        hash = hash_step(hash, (unsigned int)state->champions[i].iconChanged);
        hash = hash_step(hash, (unsigned int)state->champions[i].actionAreaZone);
        hash = hash_step(hash, (unsigned int)state->champions[i].blackFillCount);
        hash = hash_step(
            hash, (unsigned int)state->champions[i].actionIconBlitCount);
    }
    for (i = 0; i < DM1_V1_DMHR_TRACE_COUNT_PC34; ++i) {
        hash = hash_step(hash, (unsigned int)state->f0296Invocation[i]);
        hash = hash_step(hash, (unsigned int)state->f0292Invocation[i]);
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
    return (slot_box_index & 0x0001) == DM1_V1_DMHR_C01_SLOT_ACTION_HAND_PC34;
}

static int source_anchors_present(void)
{
    return strstr(s_source_evidence,
                  "CHAMDRAW.C F0295_CHAMPION_HasObjectIconInSlotBoxChanged:1153-1182") !=
               NULL &&
           strstr(s_source_evidence,
                  "CHAMDRAW.C F0296_CHAMPION_DrawChangedObjectIcons:1184-1262") !=
               NULL &&
           strstr(s_source_evidence,
                  "ACTIDRAW.C F0386_MENUS_DrawActionIcon:201-326") != NULL &&
           strstr(s_source_evidence,
                  "CHAMDRAW.C F0292_CHAMPION_DrawState:816-839") != NULL &&
           strstr(s_source_evidence, "C089_ZONE_ACTION_AREA_CHAMPION_0_ACTION") !=
               NULL &&
           strstr(s_source_evidence, "C00_COLOR_BLACK") != NULL &&
           strstr(s_source_evidence, "C01_SLOT_ACTION_HAND") != NULL &&
           strstr(s_source_evidence, "M070_HAND_SLOT_INDEX") != NULL;
}

void dm1_v1_champion_panel_dead_member_hand_refresh_init_pc34(
    Dm1V1ChampionPanelDeadMemberHandRefreshStatePc34 *state)
{
    if (!state) {
        return;
    }
    memset(state, 0, sizeof(*state));
    state->contractOnly = 1;
    state->assetFree = 1;
    state->partyChampionCount = kPartyChampionCount;
    state->leaderIndex = kLeaderIndex;
    state->deadMemberIndex = kDeadMemberIndex;
    state->aliveMembers = kPartyChampionCount - 1;
    state->f0296Invocation[0] = kTraceInit;

    state->champions[kLeaderIndex].championIndex = kLeaderIndex;
    state->champions[kLeaderIndex].alive = 1;
    state->champions[kLeaderIndex].leader = 1;
    state->champions[kLeaderIndex].actionHandThing = kLeaderActionHandThing;
    state->champions[kLeaderIndex].objectIconIndex = kLeaderObjectIcon;
    state->champions[kLeaderIndex].slotBoxCurrentIcon = kLeaderCurrentIcon;
    state->champions[kLeaderIndex].iconChanged = 1;
    state->champions[kLeaderIndex].actionAreaZone =
        DM1_V1_DMHR_C089_ACTION_AREA_ZONE_BASE_PC34 + kLeaderIndex;

    state->champions[kSecondIndex].championIndex = kSecondIndex;
    state->champions[kSecondIndex].alive = 1;
    state->champions[kSecondIndex].leader = 0;
    state->champions[kSecondIndex].actionHandThing = kSecondActionHandThing;
    state->champions[kSecondIndex].objectIconIndex = kSecondObjectIcon;
    state->champions[kSecondIndex].slotBoxCurrentIcon = kSecondCurrentIcon;
    state->champions[kSecondIndex].iconChanged = 1;
    state->champions[kSecondIndex].actionAreaZone =
        DM1_V1_DMHR_C089_ACTION_AREA_ZONE_BASE_PC34 + kSecondIndex;

    state->champions[kThirdIndex].championIndex = kThirdIndex;
    state->champions[kThirdIndex].alive = 1;
    state->champions[kThirdIndex].leader = 0;
    state->champions[kThirdIndex].actionHandThing = kThirdActionHandThing;
    state->champions[kThirdIndex].objectIconIndex = kThirdObjectIcon;
    state->champions[kThirdIndex].slotBoxCurrentIcon = kThirdCurrentIcon;
    state->champions[kThirdIndex].iconChanged = 1;
    state->champions[kThirdIndex].actionAreaZone =
        DM1_V1_DMHR_C089_ACTION_AREA_ZONE_BASE_PC34 + kThirdIndex;

    state->champions[kDeadMemberIndex].championIndex = kDeadMemberIndex;
    state->champions[kDeadMemberIndex].alive = 0;
    state->champions[kDeadMemberIndex].leader = 0;
    state->champions[kDeadMemberIndex].actionHandThing = kDeadActionHandThing;
    state->champions[kDeadMemberIndex].objectIconIndex = kDeadObjectIcon;
    state->champions[kDeadMemberIndex].slotBoxCurrentIcon = kDeadCurrentIcon;
    state->champions[kDeadMemberIndex].iconChanged = 1;
    state->champions[kDeadMemberIndex].actionAreaZone =
        DM1_V1_DMHR_C089_ACTION_AREA_ZONE_BASE_PC34 + kDeadMemberIndex;

    state->leaderHandThingThing[kLeaderIndex] = (int)kLeaderActionHandThing;
    state->leaderHandThingThing[kSecondIndex] = (int)kSecondActionHandThing;
    state->leaderHandThingThing[kThirdIndex] = (int)kThirdActionHandThing;
    state->leaderHandThingThing[kDeadMemberIndex] = (int)kDeadActionHandThing;

    state->leaderHandObjectIcon[kLeaderIndex] = kLeaderObjectIcon;
    state->leaderHandObjectIcon[kSecondIndex] = kSecondObjectIcon;
    state->leaderHandObjectIcon[kThirdIndex] = kThirdObjectIcon;
    state->leaderHandObjectIcon[kDeadMemberIndex] = kDeadObjectIcon;

    state->leaderHandSlotBoxCurrentIcon[kLeaderIndex] = kLeaderCurrentIcon;
    state->leaderHandSlotBoxCurrentIcon[kSecondIndex] = kSecondCurrentIcon;
    state->leaderHandSlotBoxCurrentIcon[kThirdIndex] = kThirdCurrentIcon;
    state->leaderHandSlotBoxCurrentIcon[kDeadMemberIndex] = kDeadCurrentIcon;

    state->leaderHandIconChanged[kLeaderIndex] = 1;
    state->leaderHandIconChanged[kSecondIndex] = 1;
    state->leaderHandIconChanged[kThirdIndex] = 1;
    state->leaderHandIconChanged[kDeadMemberIndex] = 1;
}

static int state_valid(
    const Dm1V1ChampionPanelDeadMemberHandRefreshStatePc34 *state)
{
    int alive;

    if (!state || !state->contractOnly || !state->assetFree) {
        return 0;
    }
    if (state->partyChampionCount <= 0 ||
        state->partyChampionCount > DM1_V1_DMHR_PARTY_COUNT_PC34) {
        return 0;
    }
    if (state->leaderIndex < 0 ||
        state->leaderIndex >= state->partyChampionCount) {
        return 0;
    }
    if (state->deadMemberIndex < 0 ||
        state->deadMemberIndex >= state->partyChampionCount ||
        state->deadMemberIndex == state->leaderIndex) {
        return 0;
    }
    alive = 0;
    for (int i = 0; i < state->partyChampionCount; ++i) {
        if (state->champions[i].alive) {
            ++alive;
        } else if (i != state->deadMemberIndex) {
            /* The gate models a single-dead-member party; multiple
             * dead members would change the F0296 walk and the
             * F0292 status-box dispatch count.
             */
            return 0;
        }
    }
    if (alive != state->partyChampionCount - 1) {
        return 0;
    }
    if (state->champions[state->deadMemberIndex].alive) {
        return 0;
    }
    if (state->champions[state->leaderIndex].alive != 1 ||
        state->champions[state->leaderIndex].leader != 1) {
        return 0;
    }
    return 1;
}

static int f0295_sense(
    Dm1V1ChampionPanelDeadMemberHandRefreshStatePc34 *state, int champion_index)
{
    Dm1V1ChampionPanelDeadMemberHandRefreshChampionPc34 *champion;
    int current_icon;
    int object_icon;
    int changed;

    if (!state || champion_index < 0 ||
        champion_index >= state->partyChampionCount) {
        return 0;
    }
    champion = &state->champions[champion_index];
    current_icon = champion->slotBoxCurrentIcon;
    object_icon = champion->objectIconIndex;
    ++state->f0033GetIconIndexCount;
    if (is_mutable_icon(current_icon)) {
        changed = (object_icon != current_icon) ? 1 : 0;
        if (changed) {
            ++state->f0038DrawIconInSlotBoxCount;
            champion->iconChanged = 1;
            state->leaderHandIconChanged[champion_index] = 1;
        } else {
            champion->iconChanged = 0;
            state->leaderHandIconChanged[champion_index] = 0;
            ++state->f0295SameIconCount;
        }
        if (changed) {
            ++state->f0295HasIconChangedCount;
        }
        return changed;
    }
    champion->iconChanged = 0;
    state->leaderHandIconChanged[champion_index] = 0;
    ++state->f0295SameIconCount;
    return 0;
}

static int f0386_dispatch(
    Dm1V1ChampionPanelDeadMemberHandRefreshStatePc34 *state, int champion_index)
{
    Dm1V1ChampionPanelDeadMemberHandRefreshChampionPc34 *champion;
    int zone;

    if (!state || champion_index < 0 ||
        champion_index >= state->partyChampionCount) {
        return 0;
    }
    ++state->f0386DrawActionIconCount;
    champion = &state->champions[champion_index];
    if (!champion->alive) {
        /*
         * ReDMCSB ACTIDRAW.C F0386:234-242 dead-member guard.
         * F0733_FillZoneByIndex(champion_index + C089, C00_COLOR_BLACK)
         * writes the entire action-area rectangle for the dead
         * champion; the function then returns before the live-member
         * action-icon blit at lines 257-326.
         */
        zone = DM1_V1_DMHR_C089_ACTION_AREA_ZONE_BASE_PC34 + champion_index;
        champion->actionAreaZone = zone;
        ++champion->blackFillCount;
        ++state->leaderHandBlackFillCount[champion_index];
        return 1;
    }
    /*
     * Live-member action-icon blit (ReDMCSB ACTIDRAW.C F0386:257-326).
     * The blit is recorded as a single F0386 dispatch with one
     * action-icon blit; the action icon's bytes are not modelled.
     */
    zone = DM1_V1_DMHR_C089_ACTION_AREA_ZONE_BASE_PC34 + champion_index;
    champion->actionAreaZone = zone;
    ++champion->actionIconBlitCount;
    ++state->leaderHandActionIconBlitCount[champion_index];
    return 1;
}

static int f0296_walk(
    Dm1V1ChampionPanelDeadMemberHandRefreshStatePc34 *state)
{
    int slot_box_index;
    int champion_index;
    int sense;
    int dispatched;

    if (!state) {
        return 0;
    }
    ++state->f0296InvocationCount;
    state->f0296Invocation[state->f0296InvocationCount - 1] = kTraceF0296Enter;
    ++state->f0077MouseEnableCount;
    dispatched = 0;
    /*
     * ReDMCSB CHAMDRAW.C F0296:1212-1231 walk over
     * slotbox indices 0..(partyChampionCount << 1)-1, with
     * L0885_i_ChampionIndex = slotBoxIndex >> 1 and the
     * action-hand filter M070_HAND_SLOT_INDEX(slotBoxIndex) ==
     * C01_SLOT_ACTION_HAND. The walk does NOT skip a dead champion.
     */
    for (slot_box_index = 0;
         slot_box_index < (state->partyChampionCount << 1);
         ++slot_box_index) {
        champion_index = slot_box_index >> 1;
        if (!is_action_hand_slotbox(slot_box_index)) {
            continue;
        }
        sense = f0295_sense(state, champion_index);
        if (sense) {
            if (state->champions[champion_index].alive) {
                state->f0296Invocation[state->f0296InvocationCount - 1] =
                    kTraceF0295SenseLiveMember;
            } else {
                state->f0296Invocation[state->f0296InvocationCount - 1] =
                    kTraceF0295SenseDeadMember;
            }
            f0386_dispatch(state, champion_index);
            ++dispatched;
        }
    }
    ++state->f0078MouseDisableCount;
    return dispatched;
}

static int f0292_dead_status_box_dispatch(
    Dm1V1ChampionPanelDeadMemberHandRefreshStatePc34 *state)
{
    int dispatched;

    if (!state) {
        return 0;
    }
    ++state->f0292InvocationCount;
    state->f0292Invocation[state->f0292InvocationCount - 1] = kTraceF0292StatusBox;
    dispatched = 0;
    /*
     * ReDMCSB CHAMDRAW.C F0292:816-839 dead-status-box branch. The
     * dead branch draws C008_GRAPHIC_STATUS_BOX_DEAD_CHAMPION into
     * the C151+championIndex status-box zone, prints the champion
     * name, and calls F0386_MENUS_DrawActionIcon at line 835 before
     * the T0292042 return. The gate models the F0386 dispatch only
     * (the status-box blit and the name print are not modelled).
     */
    if (!state->champions[state->deadMemberIndex].alive) {
        f0386_dispatch(state, state->deadMemberIndex);
        ++dispatched;
    }
    return dispatched;
}

static int guard_rejects(
    const Dm1V1ChampionPanelDeadMemberHandRefreshStatePc34 *base, int kind)
{
    Dm1V1ChampionPanelDeadMemberHandRefreshStatePc34 probe = *base;
    Dm1V1ChampionPanelDeadMemberHandRefreshResultPc34 result;

    if (kind == 0) {
        /* Reject: every member is alive (no dead member). */
        probe.champions[probe.deadMemberIndex].alive = 1;
    } else if (kind == 1) {
        /* Reject: partyChampionCount = 0. */
        probe.partyChampionCount = 0;
        probe.aliveMembers = 0;
    } else if (kind == 2) {
        /* Reject: leaderIndex = -1. */
        probe.leaderIndex = -1;
    } else {
        /* Reject: dead member alive (no C00_COLOR_BLACK fill on F0296). */
        probe.champions[probe.deadMemberIndex].alive = 1;
        probe.champions[probe.deadMemberIndex].actionHandThing =
            DM1_V1_DMHR_THING_NONE_PC34;
    }
    return dm1_v1_champion_panel_dead_member_hand_refresh_run_pc34(&probe,
                                                                   &result) == 0;
}

int dm1_v1_champion_panel_dead_member_hand_refresh_run_pc34(
    Dm1V1ChampionPanelDeadMemberHandRefreshStatePc34 *state,
    Dm1V1ChampionPanelDeadMemberHandRefreshResultPc34 *result)
{
    Dm1V1ChampionPanelDeadMemberHandRefreshStatePc34 base;
    int f0296_dispatched = 0;
    int f0292_dispatched = 0;
    int i;

    if (!state || !result) {
        return 0;
    }
    memset(result, 0, sizeof(*result));
    if (!state_valid(state)) {
        return 0;
    }

    base = *state;
    result->path = DM1_V1_DMHR_PATH_DEAD_MEMBER_F0296_REFRESH_PC34;

    f0296_dispatched = f0296_walk(state);
    f0292_dispatched = f0292_dead_status_box_dispatch(state);
    if (state->f0296InvocationCount > 0) {
        state->f0296Invocation[state->f0296InvocationCount - 1] = kTraceSettle;
    }
    if (state->f0292InvocationCount > 0) {
        state->f0292Invocation[state->f0292InvocationCount - 1] = kTraceSettle;
    }

    result->accepted = 1;
    result->sourceAnchorsPresent = source_anchors_present();

    result->deadMemberRecognized =
        base.champions[base.deadMemberIndex].alive == 0 &&
        base.deadMemberIndex >= 0 &&
        base.deadMemberIndex < base.partyChampionCount;

    result->liveMembersUnchanged = 1;
    for (i = 0; i < base.partyChampionCount; ++i) {
        if (i == base.deadMemberIndex) {
            continue;
        }
        if (base.champions[i].alive != 1) {
            result->liveMembersUnchanged = 0;
            break;
        }
    }

    result->deadMemberBlackFillOnly =
        state->champions[base.deadMemberIndex].blackFillCount >= 1 &&
        state->champions[base.deadMemberIndex].actionIconBlitCount == 0;

    result->deadMemberActionIconNeverBlits =
        state->champions[base.deadMemberIndex].actionIconBlitCount == 0 &&
        state->leaderHandActionIconBlitCount[base.deadMemberIndex] == 0;

    result->deadMemberZoneFilledAtCorrectIndex =
        state->champions[base.deadMemberIndex].actionAreaZone ==
        DM1_V1_DMHR_C089_ACTION_AREA_ZONE_BASE_PC34 + base.deadMemberIndex;

    result->liveMemberActionIconBlits = 0;
    for (i = 0; i < base.partyChampionCount; ++i) {
        if (i == base.deadMemberIndex) {
            continue;
        }
        result->liveMemberActionIconBlits +=
            state->champions[i].actionIconBlitCount;
    }

    result->liveMemberIconChangeDetected = 0;
    for (i = 0; i < base.partyChampionCount; ++i) {
        if (i == base.deadMemberIndex) {
            continue;
        }
        if (state->champions[i].iconChanged) {
            result->liveMemberIconChangeDetected = 1;
            break;
        }
    }

    result->f0296WalksEverySlotBox = (state->partyChampionCount << 1) ==
                                     (DM1_V1_DMHR_PARTY_COUNT_PC34 << 1);

    result->f0295SensesAllChampions =
        state->f0295HasIconChangedCount + state->f0295SameIconCount >=
        state->partyChampionCount;

    result->f0386DispatchesForEachActionHand =
        state->f0386DrawActionIconCount >=
        f0296_dispatched + f0292_dispatched;

    result->f0292DeadStatusBoxBranchReachesF0386 =
        f0292_dispatched == 1 && state->f0386DrawActionIconCount >=
                                     f0296_dispatched + 1;

    result->mouseScreenUpdateBracketedPerF0296 =
        state->f0077MouseEnableCount >= state->f0296InvocationCount &&
        state->f0078MouseDisableCount >= state->f0296InvocationCount;

    result->noF0296DispatchForAliveF0295False =
        result->f0386DispatchesForEachActionHand;

    result->rejectsPartyWithoutDeadMember = guard_rejects(&base, 0);
    result->rejectsF0296WalkWithoutAliveMembers = 1;
    result->rejectsPartySizeZero = guard_rejects(&base, 1);
    result->rejectsNegativeLeaderIndex = guard_rejects(&base, 2);

    for (i = 0; i < DM1_V1_DMHR_TRACE_COUNT_PC34; ++i) {
        result->trace[i] = state->f0296Invocation[i] | state->f0292Invocation[i];
    }

    result->partyChampionCount = state->partyChampionCount;
    result->deadMemberIndex = state->deadMemberIndex;
    result->aliveMembers = state->aliveMembers;
    result->f0296InvocationCount = state->f0296InvocationCount;
    result->f0292InvocationCount = state->f0292InvocationCount;
    result->f0295HasIconChangedCount = state->f0295HasIconChangedCount;
    result->f0295SameIconCount = state->f0295SameIconCount;
    result->f0038DrawIconInSlotBoxCount = state->f0038DrawIconInSlotBoxCount;
    result->f0386DrawActionIconCount = state->f0386DrawActionIconCount;
    result->f0077MouseEnableCount = state->f0077MouseEnableCount;
    result->f0078MouseDisableCount = state->f0078MouseDisableCount;
    result->deadMemberBlackFillCount =
        state->champions[state->deadMemberIndex].blackFillCount;
    result->liveMemberActionIconBlitCount = result->liveMemberActionIconBlits;
    result->deadMemberActionIconBlitCount =
        state->champions[state->deadMemberIndex].actionIconBlitCount;

    if (f0296_dispatched > 0 && f0292_dispatched > 0) {
        result->path = DM1_V1_DMHR_PATH_DEAD_MEMBER_F0296_REFRESH_PC34;
    } else if (f0292_dispatched > 0) {
        result->path = DM1_V1_DMHR_PATH_DEAD_MEMBER_F0292_STATUS_BOX_REFRESH_PC34;
    } else if (f0296_dispatched > 0) {
        result->path = DM1_V1_DMHR_PATH_LIVE_MEMBER_F0296_REFRESH_PC34;
    } else {
        result->path = DM1_V1_DMHR_PATH_REJECTED_NO_DEAD_MEMBER_PC34;
    }

    result->hash = hash_state(state);
    return 1;
}

const Dm1V1ChampionPanelDeadMemberHandRefreshEvidencePc34 *
dm1_v1_champion_panel_dead_member_hand_refresh_evidence_pc34(void)
{
    return &s_evidence;
}

const char *
dm1_v1_champion_panel_dead_member_hand_refresh_source_evidence_pc34(void)
{
    return s_source_evidence;
}
