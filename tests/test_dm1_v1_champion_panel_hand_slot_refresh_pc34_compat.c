#include "firestaff/dm1/v1/champion_panel/hand_slot_refresh_pc34_compat.h"

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
    const Dm1V1ChampionPanelHandSlotRefreshEvidencePc34 *e =
        dm1_v1_champion_panel_hand_slot_refresh_evidence_pc34();
    const char *text =
        dm1_v1_champion_panel_hand_slot_refresh_source_evidence_pc34();
    const char *siblings[] = {
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

    check_true(e != NULL, "evidence accessor", "hand_slot_refresh");
    check_contains(e->walkF0295Anchor, "1153-1182",
                   "F0295 anchor", e->walkF0295Anchor);
    check_contains(e->walkF0296Anchor, "1184-1262",
                   "F0296 anchor", e->walkF0296Anchor);
    check_contains(e->walkF0033Anchor, "F0033_OBJECT_GetIconIndex",
                   "F0033 anchor", e->walkF0033Anchor);
    check_contains(e->walkF0038Anchor, "F0038_OBJECT_DrawIconInSlotBox",
                   "F0038 anchor", e->walkF0038Anchor);
    check_contains(e->leaderHandAnchor, "F0036_OBJECT_ExtractIconFromBitmap",
                   "F0036 anchor", e->leaderHandAnchor);
    check_contains(e->leaderHandAnchor, "F0068_MOUSE_SetPointerToObject",
                   "F0068 anchor", e->leaderHandAnchor);
    check_contains(e->leaderHandAnchor, "F0034_OBJECT_DrawLeaderHandObjectName",
                   "F0034 anchor", e->leaderHandAnchor);
    check_contains(e->candidateOrdinalAnchor,
                   "G0299_ui_CandidateChampionOrdinal",
                   "G0299 candidate ordinal anchor",
                   e->candidateOrdinalAnchor);
    check_contains(e->inventoryChampionOrdinalAnchor,
                   "G0423_i_InventoryChampionOrdinal",
                   "G0423 inventory ordinal anchor",
                   e->inventoryChampionOrdinalAnchor);
    check_contains(e->mouseBracketAnchor, "F0078_MOUSE_DisableScreenUpdate",
                   "F0078 mouse disable anchor", e->mouseBracketAnchor);
    check_contains(e->defsAnchor, "C01_SLOT_ACTION_HAND",
                   "DEFS.H C01_SLOT_ACTION_HAND anchor", e->defsAnchor);
    check_contains(e->defsAnchor, "M070_HAND_SLOT_INDEX",
                   "DEFS.H M070_HAND_SLOT_INDEX anchor", e->defsAnchor);
    check_contains(e->defsAnchor, "C195_ICON_POTION_EMPTY_FLASK",
                   "DEFS.H C195 empty flask anchor", e->defsAnchor);
    check_contains(e->defsAnchor, "C201_ICON_ACTION_ICON_EMPTY_HAND",
                   "DEFS.H C201 empty hand anchor", e->defsAnchor);
    check_contains(e->defsAnchor, "G0305_ui_PartyChampionCount",
                   "DEFS.H G0305 party count anchor", e->defsAnchor);
    check_contains(e->defsAnchor, "M000_INDEX_TO_ORDINAL",
                   "DEFS.H M000_INDEX_TO_ORDINAL anchor", e->defsAnchor);

    check_contains(text, "CHAMDRAW.C F0295_CHAMPION_HasObjectIconInSlotBoxChanged:1153-1182",
                   "F0295 source text", text);
    check_contains(text, "CHAMDRAW.C F0296_CHAMPION_DrawChangedObjectIcons:1184-1262",
                   "F0296 source text", text);
    check_contains(text, "G0299_ui_CandidateChampionOrdinal",
                   "G0299 candidate ordinal source text", text);
    check_contains(text, "G0423_i_InventoryChampionOrdinal",
                   "G0423 inventory champion ordinal source text", text);
    check_contains(text, "M000_INDEX_TO_ORDINAL",
                   "M000_INDEX_TO_ORDINAL source text", text);

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

static void test_walk_order_fully_alive(void)
{
    Dm1V1ChampionPanelHandSlotRefreshStatePc34 state;
    Dm1V1ChampionPanelHandSlotRefreshResultPc34 result;

    memset(&state, 0, sizeof(state));
    dm1_v1_champion_panel_hand_slot_refresh_init_pc34(&state);

    check_int_eq(state.partyChampionCount, 4,
                 "party count = 4", "DM1_V1_DMHSR_PARTY_COUNT_PC34");
    check_int_eq(state.aliveMembers, 4,
                 "alive members = 4", "fully-alive party");
    check_int_eq(state.champions[0].alive, 1,
                 "champion 0 alive = 1",
                 "fully-alive party (no dead member)");
    check_int_eq(state.champions[3].alive, 1,
                 "champion 3 alive = 1",
                 "fully-alive party (no dead member)");

    check_int_eq(dm1_v1_champion_panel_hand_slot_refresh_run_pc34(
                     &state, &result),
                 1, "run returns success",
                 "state_valid + F0296 walk");

    check_int_eq(result.accepted, 1, "result.accepted",
                 "F0296 walk returned");
    check_int_eq(result.sourceAnchorsPresent, 1, "source anchors present",
                 "source_anchors_present()");
    check_int_eq(result.fullyAliveRecognized, 1,
                 "fully-alive recognized",
                 "aliveMembers == partyChampionCount");
    check_int_eq(result.path, DM1_V1_DMHSR_PATH_FULLY_ALIVE_F0296_WALK_PC34,
                 "path = FULLY_ALIVE_F0296_WALK",
                 "fully-alive baseline");

    /*
     * Walk-order contract: action-hand slotbox indices are
     * 1, 3, 5, 7 (the odd indices) in strict ascending order, and
     * the computed champion index matches slotBoxIndex >> 1.
     */
    check_int_eq(result.walkOrderChampionIndexAscending, 1,
                 "walk order champion index ascending",
                 "F0296:1221-1231 strict ascending walk");
    check_int_eq(result.walkOrderActionHandIndicesOdd, 1,
                 "walk order action-hand indices odd",
                 "M070_HAND_SLOT_INDEX = 1 on action-hand slotbox");
    check_int_eq(result.walkOrderChampionIndexPerSlotbox, 1,
                 "walk order champion index per slotbox",
                 "L0885_i_ChampionIndex = slotBoxIndex >> 1");
    check_int_eq(state.slotBoxWalkIndex[0], 1,
                 "first walked slotbox index = 1",
                 "2 * 0 + 1 = action hand of champion 0");
    check_int_eq(state.slotBoxWalkIndex[1], 3,
                 "second walked slotbox index = 3",
                 "2 * 1 + 1 = action hand of champion 1");
    check_int_eq(state.slotBoxWalkIndex[2], 5,
                 "third walked slotbox index = 5",
                 "2 * 2 + 1 = action hand of champion 2");
    check_int_eq(state.slotBoxWalkIndex[3], 7,
                 "fourth walked slotbox index = 7",
                 "2 * 3 + 1 = action hand of champion 3");
    check_int_eq(state.slotBoxWalkChampionIndex[0], 0,
                 "first walked champion index = 0",
                 "slotBoxIndex >> 1 = 0");
    check_int_eq(state.slotBoxWalkChampionIndex[1], 1,
                 "second walked champion index = 1",
                 "slotBoxIndex >> 1 = 1");
    check_int_eq(state.slotBoxWalkChampionIndex[2], 2,
                 "third walked champion index = 2",
                 "slotBoxIndex >> 1 = 2");
    check_int_eq(state.slotBoxWalkChampionIndex[3], 3,
                 "fourth walked champion index = 3",
                 "slotBoxIndex >> 1 = 3");

    check_int_eq(result.f0296WalksExactlyN2Slotboxes, 1,
                 "F0296 walks exactly N*2 slotboxes",
                 "F0296:1221 (partyChampionCount << 1) loop");
    check_int_eq(result.leaderHandSlotBoxesWalked, 4,
                 "leader-hand slotboxes walked = 4",
                 "4 action-hand slotboxes on a 4-champion party");
    check_int_eq(result.f0296InvocationCount, 1,
                 "F0296 invocation count = 1",
                 "one F0296 walk during the gate");
}

static void test_leader_hand_refresh_precedes_walk(void)
{
    Dm1V1ChampionPanelHandSlotRefreshStatePc34 state;
    Dm1V1ChampionPanelHandSlotRefreshResultPc34 result;

    memset(&state, 0, sizeof(state));
    dm1_v1_champion_panel_hand_slot_refresh_init_pc34(&state);

    check_int_eq(dm1_v1_champion_panel_hand_slot_refresh_run_pc34(
                     &state, &result),
                 1, "run returns success (leader-hand precedence)",
                 "F0296:1213-1220 leader-hand refresh precedes walk");

    /*
     * The leader-hand icon is mutable (kLeaderHandIconIndex = 22 is
     * in the C000..C031 junk range) and the synthetic F0033 result
     * always differs, so the F0077 + F0036 + F0068 + F0034 sequence
     * must fire exactly once per F0296 invocation, before the
     * slotbox walk.
     */
    check_int_eq(state.leaderHandIconRefreshCount, 1,
                 "leader-hand icon refresh count = 1",
                 "F0296:1213-1220 leader-hand refresh fires once");
    check_int_eq(result.leaderHandIconRefreshOncePerF0296, 1,
                 "leader-hand refresh once per F0296",
                 "F0296:1213-1220 leader-hand refresh = 1 / F0296 invocation");
    check_int_eq(state.f0077MouseEnableCount, 1,
                 "F0077 mouse enable count = 1",
                 "F0296:1218 leader-hand refresh F0077");
    check_int_eq(state.f0036ExtractIconFromBitmapCount, 1,
                 "F0036 extract icon count = 1",
                 "F0296:1219 leader-hand refresh F0036");
    check_int_eq(state.f0068SetPointerToObjectCount, 1,
                 "F0068 set pointer count = 1",
                 "F0296:1219 leader-hand refresh F0068");
    check_int_eq(state.f0034DrawLeaderHandObjectNameCount, 1,
                 "F0034 draw leader-hand name count = 1",
                 "F0296:1220 leader-hand refresh F0034");
    check_int_eq(result.leaderHandF0036F0068F0034Sequence, 1,
                 "F0036/F0068/F0034 sequence preserved",
                 "F0296:1219-1220 leader-hand refresh helper order");
    check_int_eq(result.leaderHandPrecedesWalk, 1,
                 "leader-hand refresh precedes slotbox walk",
                 "F0296:1213-1220 precedes F0296:1221-1231 walk");
    check_int_eq(state.f0078MouseDisableCount, 1,
                 "F0078 mouse disable count = 1",
                 "F0296:1248-1251 F0078 tail when icon changed");

    /*
     * Trace ordering: trace[1] must be the leader-hand refresh
     * marker (between the F0296 enter marker at trace[0] and the
     * first slotbox walk marker at trace[2]).
     */
    check_int_eq(state.f0296Trace[0], 1,
                 "trace[0] = kTraceF0296Enter (1)",
                 "F0296 entry marker");
    check_int_eq(state.f0296Trace[1], 2,
                 "trace[1] = kTraceLeaderHandRefresh (2)",
                 "leader-hand refresh marker follows F0296 entry");
    check_int_eq(state.f0296Trace[2], 6,
                 "trace[2] = kTraceF0386Dispatch (6) for champion 0",
                 "slotbox walk + F0386 dispatch for champion 0 (mutable + changed)");
    check_int_eq(state.f0296Trace[3], 6,
                 "trace[3] = kTraceF0386Dispatch (6) for champion 1",
                 "slotbox walk + F0386 dispatch for champion 1 (mutable + changed)");
    check_int_eq(state.f0296Trace[4], 6,
                 "trace[4] = kTraceF0386Dispatch (6) for champion 2",
                 "slotbox walk + F0386 dispatch for champion 2 (mutable + changed)");
    check_int_eq(state.f0296Trace[5], 3,
                 "trace[5] = kTraceSlotboxWalk (3) for champion 3",
                 "slotbox walk only (mutable + same, F0295 = C0_FALSE, F0386 not dispatched)");
}

static void test_f0295_sense_and_f0386_dispatch(void)
{
    Dm1V1ChampionPanelHandSlotRefreshStatePc34 state;
    Dm1V1ChampionPanelHandSlotRefreshResultPc34 result;

    memset(&state, 0, sizeof(state));
    dm1_v1_champion_panel_hand_slot_refresh_init_pc34(&state);

    check_int_eq(dm1_v1_champion_panel_hand_slot_refresh_run_pc34(
                     &state, &result),
                 1, "run returns success (F0295/F0386)",
                 "F0296:1231 F0386 dispatch on F0295 = C1_TRUE");

    /*
     * Champions 0, 1, 2 have mutable + changed icons (F0295 fires,
     * F0386 dispatches). Champion 3 has matching icons (F0295
     * returns C0_FALSE, F0386 not dispatched).
     */
    check_int_eq(result.f0295HasIconChangedCount, 3,
                 "F0295 has-icon-changed count = 3",
                 "champions 0/1/2 with mutable + changed icons");
    check_int_eq(result.f0295SameIconCount, 1,
                 "F0295 same-icon count = 1",
                 "champion 3 with mutable + matching icons");
    check_int_eq(state.f0033GetIconIndexCount, 5,
                 "F0033 get icon index count = 5",
                 "1 leader-hand refresh + 4 slotbox walk senses");
    check_int_eq(result.f0038DrawIconInSlotBoxCount, 3,
                 "F0038 draw-icon-in-slotbox count = 3",
                 "F0295:1175-1182 F0038 on each changed slotbox");
    check_int_eq(state.champions[0].f0038DrawIconInSlotBoxCount, 1,
                 "champion 0 F0038 count = 1", "F0295 sense for champion 0");
    check_int_eq(state.champions[3].f0038DrawIconInSlotBoxCount, 0,
                 "champion 3 F0038 count = 0", "F0295 sense for champion 3");
    check_int_eq(result.f0386DispatchedForChangedActionHand, 3,
                 "F0386 dispatched for changed action hand = 3",
                 "F0296:1231 F0386 dispatch on changed action hand");
    check_int_eq(state.champions[0].f0386DrawActionIconCount, 1,
                 "champion 0 F0386 dispatch count = 1",
                 "F0296:1231 F0386 for champion 0");
    check_int_eq(state.champions[3].f0386DrawActionIconCount, 0,
                 "champion 3 F0386 dispatch count = 0",
                 "F0296:1231 F0386 skipped for champion 3");
    check_int_eq(result.f0295SenseContractOnMutableIcon, 1,
                 "F0295 sense contract on mutable icon",
                 "F0295:1153-1182 mutable-icon guard");
    check_int_eq(result.f0295NoChangeSkipsF0386, 1,
                 "F0295 no-change skips F0386",
                 "F0295:1175-1182 C0_FALSE return skips F0386");
    check_int_eq(result.mouseScreenUpdateBalancedPerF0296, 1,
                 "F0077 == F0078 (balanced mouse screen update)",
                 "F0296:1218 + F0296:1248-1251 balanced pair");
    check_int_eq(result.mouseScreenUpdateNeverRaisedWithoutChange, 1,
                 "mouse screen update never raised without change",
                 "F0296:1218 only when leader-hand icon changed");
}

static void test_inventory_champion_ordinal_skip(void)
{
    Dm1V1ChampionPanelHandSlotRefreshStatePc34 state;
    Dm1V1ChampionPanelHandSlotRefreshResultPc34 result;

    memset(&state, 0, sizeof(state));
    dm1_v1_champion_panel_hand_slot_refresh_init_pc34(&state);

    /*
     * Open the inventory panel on champion 2 (ordinal = 2 + 1 = 3
     * per DEFS.H:7208 M000_INDEX_TO_ORDINAL). The F0296 walk must
     * skip the action-hand slotbox for champion 2 (slotBoxIndex 5)
     * via the G0423 skip at line 1217-1219.
     */
    state.inventoryChampionOrdinal = 3;
    state.inventoryChampionIndex = 2;

    check_int_eq(dm1_v1_champion_panel_hand_slot_refresh_run_pc34(
                     &state, &result),
                 1, "run returns success (inventory-champion skip)",
                 "F0296:1217-1219 G0423 skip");

    check_int_eq(result.path, DM1_V1_DMHSR_PATH_INVENTORY_CHAMPION_SKIP_PC34,
                 "path = INVENTORY_CHAMPION_SKIP",
                 "G0423 != 0 with slotbox walk");
    check_int_eq(result.inventoryChampionSkipAppliedPerSlotbox, 1,
                 "inventory-champion skip applied per slotbox",
                 "F0296:1217-1219 inventory champion ordinal skip");
    check_int_eq(result.slotBoxWalkInventorySkip, 1,
                 "slotbox walk inventory skip count = 1",
                 "one slotbox skipped by G0423");
    check_int_eq(state.slotBoxWalkInventorySkip[2], 1,
                 "third walked slotbox has inventory skip = 1",
                 "champion 2's slotbox is the one skipped");
    check_int_eq(state.slotBoxWalkF0386Dispatched[2], 0,
                 "third walked slotbox F0386 = 0",
                 "inventory skip overrides F0386 dispatch");
    check_int_eq(state.champions[2].inventoryChampionSkipHit, 1,
                 "champion 2 inventory-champion skip hit = 1",
                 "F0296:1217-1219 skip applies to champion 2 only");
    check_int_eq(state.champions[0].inventoryChampionSkipHit, 0,
                 "champion 0 inventory-champion skip hit = 0",
                 "F0296:1217-1219 skip does not apply to champion 0");
    check_int_eq(state.champions[3].inventoryChampionSkipHit, 0,
                 "champion 3 inventory-champion skip hit = 0",
                 "F0296:1217-1219 skip does not apply to champion 3");
    check_int_eq(state.f0038DrawIconInSlotBoxCount, 2,
                 "F0038 draw count = 2 (skipping champion 2)",
                 "F0296:1217-1219 skip suppresses F0295 for champion 2");
    check_int_eq(result.f0386DispatchedForChangedActionHand, 2,
                 "F0386 dispatched count = 2",
                 "F0296:1231 F0386 for champions 0/1 only");
    check_int_eq(state.f0077MouseEnableCount, 1,
                 "F0077 mouse enable count = 1 (unchanged)",
                 "F0296:1218 leader-hand refresh still fires");
    check_int_eq(state.f0078MouseDisableCount, 1,
                 "F0078 mouse disable count = 1 (unchanged)",
                 "F0296:1248-1251 tail still fires");
    check_int_eq(result.mouseScreenUpdateBalancedPerF0296, 1,
                 "mouse screen update still balanced",
                 "F0296:1218 + F0296:1248-1251 balanced pair");
}

static void test_candidate_early_return(void)
{
    Dm1V1ChampionPanelHandSlotRefreshStatePc34 state;
    Dm1V1ChampionPanelHandSlotRefreshResultPc34 result;

    memset(&state, 0, sizeof(state));
    dm1_v1_champion_panel_hand_slot_refresh_init_pc34(&state);

    /*
     * Open the C040 candidate panel on champion 1 (ordinal = 1 + 1
     * = 2) without an inventory panel. F0296 must early-return at
     * line 1208-1210 without entering the slotbox walk.
     */
    state.candidateChampionOrdinal = 2;

    check_int_eq(dm1_v1_champion_panel_hand_slot_refresh_run_pc34(
                     &state, &result),
                 1, "run returns success (candidate early-return)",
                 "F0296:1208-1210 G0299 early-return");

    check_int_eq(result.path, DM1_V1_DMHSR_PATH_CANDIDATE_EARLY_RETURN_PC34,
                 "path = CANDIDATE_EARLY_RETURN",
                 "F0296:1208-1210 candidate early-return");
    check_int_eq(result.candidateEarlyReturnBeforeWalk, 1,
                 "candidate early-return before walk",
                 "F0296:1208-1210 G0299 early-return");
    check_int_eq(result.f0295HasIconChangedCount, 0,
                 "F0295 has-icon-changed count = 0",
                 "F0296:1208-1210 early-return suppresses walk");
    check_int_eq(result.f0386DispatchedForChangedActionHand, 0,
                 "F0386 dispatched count = 0",
                 "F0296:1208-1210 early-return suppresses dispatch");
    check_int_eq(result.leaderHandSlotBoxesWalked, 0,
                 "leader-hand slotboxes walked = 0",
                 "F0296:1208-1210 early-return suppresses walk");
    check_int_eq(state.f0077MouseEnableCount, 0,
                 "F0077 mouse enable count = 0",
                 "F0296:1218 leader-hand refresh not entered");
    check_int_eq(state.f0078MouseDisableCount, 0,
                 "F0078 mouse disable count = 0",
                 "F0296:1248-1251 tail not entered");
}

static void test_rejects(void)
{
    Dm1V1ChampionPanelHandSlotRefreshStatePc34 base;
    Dm1V1ChampionPanelHandSlotRefreshResultPc34 result;
    Dm1V1ChampionPanelHandSlotRefreshStatePc34 probe;
    int rejected;

    memset(&base, 0, sizeof(base));
    dm1_v1_champion_panel_hand_slot_refresh_init_pc34(&base);

    check_int_eq(dm1_v1_champion_panel_hand_slot_refresh_run_pc34(
                     &base, &result),
                 1, "baseline run returns success", "baseline gate");

    /* Reject 1: a dead member present. */
    probe = base;
    probe.champions[3].alive = 0;
    probe.aliveMembers = 3;
    rejected = dm1_v1_champion_panel_hand_slot_refresh_run_pc34(
                   &probe, &result) == 0;
    check_int_eq(rejected, 1, "reject: dead member present",
                 "champion[3].alive = 0");

    /* Reject 2: partyChampionCount = 0. */
    probe = base;
    probe.partyChampionCount = 0;
    probe.aliveMembers = 0;
    rejected = dm1_v1_champion_panel_hand_slot_refresh_run_pc34(
                   &probe, &result) == 0;
    check_int_eq(rejected, 1, "reject: party size zero",
                 "G0305_ui_PartyChampionCount = 0");

    /* Reject 3: leaderIndex = -1. */
    probe = base;
    probe.leaderIndex = -1;
    rejected = dm1_v1_champion_panel_hand_slot_refresh_run_pc34(
                   &probe, &result) == 0;
    check_int_eq(rejected, 1, "reject: negative leader index",
                 "leaderIndex = -1");

    /* Reject 4: NULL state. */
    rejected = dm1_v1_champion_panel_hand_slot_refresh_run_pc34(
                   NULL, &result) == 0;
    check_int_eq(rejected, 1, "reject: NULL state", "NULL state guard");

    /* Reject 5: NULL result. */
    memset(&probe, 0, sizeof(probe));
    dm1_v1_champion_panel_hand_slot_refresh_init_pc34(&probe);
    rejected = dm1_v1_champion_panel_hand_slot_refresh_run_pc34(
                   &probe, NULL) == 0;
    check_int_eq(rejected, 1, "reject: NULL result", "NULL result guard");

    /* Reject 6: candidate + inventory both non-zero. */
    probe = base;
    probe.candidateChampionOrdinal = 2;
    probe.inventoryChampionOrdinal = 3;
    rejected = dm1_v1_champion_panel_hand_slot_refresh_run_pc34(
                   &probe, &result) == 0;
    check_int_eq(rejected, 1, "reject: candidate + inventory both non-zero",
                 "G0299 + G0423 both non-zero");

    /* Reject 7: inventory champion ordinal out of range. */
    probe = base;
    probe.inventoryChampionOrdinal = 99;
    probe.inventoryChampionIndex = 98;
    rejected = dm1_v1_champion_panel_hand_slot_refresh_run_pc34(
                   &probe, &result) == 0;
    check_int_eq(rejected, 1, "reject: inventory ordinal out of range",
                 "G0423 = 99 (out of [1..partyChampionCount])");
}

static void test_guards_match_expectations(void)
{
    Dm1V1ChampionPanelHandSlotRefreshStatePc34 state;
    Dm1V1ChampionPanelHandSlotRefreshResultPc34 result;

    memset(&state, 0, sizeof(state));
    dm1_v1_champion_panel_hand_slot_refresh_init_pc34(&state);

    check_int_eq(dm1_v1_champion_panel_hand_slot_refresh_run_pc34(
                     &state, &result),
                 1, "run returns success (guard test)",
                 "baseline gate");

    check_int_eq(result.rejectsPartySizeZero, 0,
                 "guard: party size zero not flagged on baseline",
                 "baseline partyChampionCount = 4");
    check_int_eq(result.rejectsNegativeLeaderIndex, 0,
                 "guard: negative leader index not flagged on baseline",
                 "baseline leaderIndex = 0");
}

static void test_baseline_deterministic_hash(void)
{
    Dm1V1ChampionPanelHandSlotRefreshStatePc34 state;
    Dm1V1ChampionPanelHandSlotRefreshResultPc34 result1;
    Dm1V1ChampionPanelHandSlotRefreshResultPc34 result2;

    memset(&state, 0, sizeof(state));
    dm1_v1_champion_panel_hand_slot_refresh_init_pc34(&state);
    check_int_eq(dm1_v1_champion_panel_hand_slot_refresh_run_pc34(
                     &state, &result1),
                 1, "first run returns success", "deterministic hash");

    memset(&state, 0, sizeof(state));
    dm1_v1_champion_panel_hand_slot_refresh_init_pc34(&state);
    check_int_eq(dm1_v1_champion_panel_hand_slot_refresh_run_pc34(
                     &state, &result2),
                 1, "second run returns success", "deterministic hash");

    check_int_eq(result1.hash == result2.hash, 1,
                 "hash is deterministic across runs",
                 "FNV-1a hash of the same state");
    check_int_ge(result1.f0296InvocationCount, 1,
                 "F0296 invocation count >= 1",
                 "baseline always walks F0296 once");
}

int main(void)
{
    test_evidence();
    test_walk_order_fully_alive();
    test_leader_hand_refresh_precedes_walk();
    test_f0295_sense_and_f0386_dispatch();
    test_inventory_champion_ordinal_skip();
    test_candidate_early_return();
    test_rejects();
    test_guards_match_expectations();
    test_baseline_deterministic_hash();

    Dm1V1ChampionPanelHandSlotRefreshStatePc34 state;
    Dm1V1ChampionPanelHandSlotRefreshResultPc34 result;
    memset(&state, 0, sizeof(state));
    dm1_v1_champion_panel_hand_slot_refresh_init_pc34(&state);
    dm1_v1_champion_panel_hand_slot_refresh_run_pc34(&state, &result);
    printf("hand_slot_refresh hash=0x%08X assertions=%d failures=%d\n",
           (unsigned)result.hash, g_assertions, g_failures);
    return g_failures == 0 ? 0 : 1;
}
