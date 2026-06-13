#include "firestaff/dm1/v1/champion_panel/dead_member_hand_refresh_pc34_compat.h"

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
    const Dm1V1ChampionPanelDeadMemberHandRefreshEvidencePc34 *e =
        dm1_v1_champion_panel_dead_member_hand_refresh_evidence_pc34();
    const char *text =
        dm1_v1_champion_panel_dead_member_hand_refresh_source_evidence_pc34();
    const char *siblings[] = {
        "champion_panel_portrait_box_blit_gate",
        "champion_panel_portrait_box_redraw_states",
        "champion_panel_portrait_state_redraw",
        "champion_panel_hand_slot_priority",
        "mirror_candidate_icon_refresh",
        "champion_panel_spell_area_overlay",
        "mirror_candidate_c045_accept_dead_owner_guard",
        "inventory_champion_switch_hand_carry",
        "F0107",
        "F0108",
        "chest-scroll-wheel",
        "viewport",
    };
    int i;

    check_true(e != NULL, "evidence accessor", "dead_member_hand_refresh");
    check_contains(e->redrawF0295Anchor, "1153-1182",
                   "F0295 anchor", e->redrawF0295Anchor);
    check_contains(e->redrawF0296Anchor, "1184-1262",
                   "F0296 anchor", e->redrawF0296Anchor);
    check_contains(e->redrawF0386Anchor, "201-326",
                   "F0386 anchor", e->redrawF0386Anchor);
    check_contains(e->redrawF0292Anchor, "816-839",
                   "F0292 dead-status-box anchor", e->redrawF0292Anchor);
    check_contains(e->objectF0033Anchor, "F0033_OBJECT_GetIconIndex",
                   "F0033 anchor", e->objectF0033Anchor);
    check_contains(e->objectF0038Anchor, "F0038_OBJECT_DrawIconInSlotBox",
                   "F0038 anchor", e->objectF0038Anchor);
    check_contains(e->defsAnchor, "C01_SLOT_ACTION_HAND",
                   "DEFS.H C01_SLOT_ACTION_HAND anchor", e->defsAnchor);
    check_contains(e->defsAnchor, "M070_HAND_SLOT_INDEX",
                   "DEFS.H M070_HAND_SLOT_INDEX anchor", e->defsAnchor);
    check_contains(e->defsAnchor, "C089_ZONE_ACTION_AREA_CHAMPION_0_ACTION",
                   "DEFS.H C089 zone anchor", e->defsAnchor);
    check_contains(e->defsAnchor, "C00_COLOR_BLACK",
                   "DEFS.H C00_COLOR_BLACK anchor", e->defsAnchor);
    check_contains(e->zoneAnchor, "C089+championIndex",
                   "zone anchor", e->zoneAnchor);
    check_contains(text, "F0295_CHAMPION_HasObjectIconInSlotBoxChanged:1153-1182",
                   "F0295 source text", text);
    check_contains(text, "F0296_CHAMPION_DrawChangedObjectIcons:1184-1262",
                   "F0296 source text", text);
    check_contains(text, "F0386_MENUS_DrawActionIcon:201-326",
                   "F0386 source text", text);
    check_contains(text, "CHAMDRAW.C F0292_CHAMPION_DrawState:816-839",
                   "F0292 source text", text);
    check_contains(text, "C089_ZONE_ACTION_AREA_CHAMPION_0_ACTION",
                   "C089 source text", text);

    for (i = 0; i < (int)(sizeof(siblings) / sizeof(siblings[0])); ++i) {
        char id[64];
        const char *s = siblings[i];
        snprintf(id, sizeof(id), "sibling.%s", s);
        check_contains(e->nonOverlap, s, id, e->nonOverlap);
    }
}

static void test_f0296_f0295_f0386_dead_member_walk(void)
{
    Dm1V1ChampionPanelDeadMemberHandRefreshStatePc34 state;
    Dm1V1ChampionPanelDeadMemberHandRefreshResultPc34 result;

    memset(&state, 0, sizeof(state));
    dm1_v1_champion_panel_dead_member_hand_refresh_init_pc34(&state);

    check_int_eq(state.partyChampionCount, 4,
                 "party count = 4", "DM1_V1_DMHR_PARTY_COUNT_PC34");
    check_int_eq(state.deadMemberIndex, 3,
                 "dead member index = 3", "kDeadMemberIndex");
    check_int_eq(state.aliveMembers, 3,
                 "alive members = 3", "4 - 1 dead member");
    check_int_eq(state.champions[3].alive, 0,
                 "champion 3 alive = 0 (dead)",
                 "F0386:234-242 dead-member guard");
    check_int_eq(state.champions[0].alive, 1,
                 "champion 0 alive = 1 (leader)",
                 "F0292:784 dead branch skips leader");
    check_int_eq(state.champions[0].leader, 1,
                 "champion 0 leader = 1",
                 "F0292:843-895 leader/non-leader name color");

    check_int_eq(dm1_v1_champion_panel_dead_member_hand_refresh_run_pc34(
                     &state, &result),
                 1, "run returns success",
                 "state_valid + F0296 walk + F0292 dispatch");

    check_int_eq(result.accepted, 1, "result.accepted",
                 "F0296 + F0292 walk returned");
    check_int_eq(result.sourceAnchorsPresent, 1, "source anchors present",
                 "source_anchors_present()");
    check_int_eq(result.deadMemberRecognized, 1, "dead member recognized",
                 "champion 3 alive = 0");
    check_int_eq(result.liveMembersUnchanged, 1, "live members unchanged",
                 "champion 0,1,2 alive = 1");

    check_int_eq(result.deadMemberBlackFillOnly, 1,
                 "dead member black-fill only",
                 "F0386:234-242 dead-member guard fills C00_COLOR_BLACK");
    check_int_eq(result.deadMemberActionIconNeverBlits, 1,
                 "dead member action icon never blits",
                 "F0386:257-326 unreachable while CurrentHealth == 0");
    check_int_eq(result.deadMemberZoneFilledAtCorrectIndex, 1,
                 "dead member zone filled at correct index",
                 "C089 + deadMemberIndex");

    check_int_eq(result.f0296WalksEverySlotBox, 1,
                 "F0296 walks every slotbox",
                 "F0296:1212-1231 (partyChampionCount << 1)");
    check_int_eq(result.f0295SensesAllChampions, 1,
                 "F0295 senses all champions",
                 "F0295:1153-1182 dispatched for each action-hand slot");
    check_int_eq(result.f0386DispatchesForEachActionHand, 1,
                 "F0386 dispatches for each action hand",
                 "F0296:1231 + F0292:835");
    check_int_eq(result.f0292DeadStatusBoxBranchReachesF0386, 1,
                 "F0292 dead-status-box branch reaches F0386",
                 "F0292:816-839 F0386 dispatch at line 835");

    check_int_eq(result.mouseScreenUpdateBracketedPerF0296, 1,
                 "mouse screen-update bracketed per F0296",
                 "F0077/F0078 around F0296 walk");
    check_int_eq(result.noF0296DispatchForAliveF0295False, 1,
                 "no F0296 dispatch when F0295 reports no change",
                 "F0295 = C0_FALSE skips F0386");

    check_int_eq(result.f0296InvocationCount, 1,
                 "F0296 invocation count = 1",
                 "one F0296 walk during the gate");
    check_int_eq(result.f0292InvocationCount, 1,
                 "F0292 invocation count = 1",
                 "one F0292 walk during the gate");
    check_int_eq(result.f0386DrawActionIconCount >= 4, 1,
                 "F0386 draw action icon count >= 4",
                 "3 live + 1 dead (F0296) + 1 dead (F0292)");
    check_int_eq(result.deadMemberBlackFillCount >= 2, 1,
                 "dead member black-fill count >= 2",
                 "F0296 + F0292 both fill the dead member zone");
    check_int_eq(result.deadMemberActionIconBlitCount, 0,
                 "dead member action icon blit count = 0",
                 "F0386 dead-member guard returns before the blit");
    check_int_eq(result.liveMemberActionIconBlitCount >= 3, 1,
                 "live member action icon blit count >= 3",
                 "F0296 dispatches F0386 for each live member");
    check_int_eq(result.f0077MouseEnableCount, result.f0078MouseDisableCount,
                 "F0077 == F0078 (balanced mouse screen update)",
                 "F0296:1212-1251 balanced pair");
}

static void test_f0295_no_change_keeps_f0296_silent(void)
{
    Dm1V1ChampionPanelDeadMemberHandRefreshStatePc34 state;
    Dm1V1ChampionPanelDeadMemberHandRefreshResultPc34 result;

    memset(&state, 0, sizeof(state));
    dm1_v1_champion_panel_dead_member_hand_refresh_init_pc34(&state);

    /*
     * Force every champion's slotbox current icon to match its
     * object icon, so F0295 must return C0_FALSE for every
     * action-hand slotbox. F0296 must still walk the full
     * 2 * partyChampionCount range (the walk does not skip), but
     * F0386 must not be dispatched.
     */
    for (int i = 0; i < state.partyChampionCount; ++i) {
        state.champions[i].slotBoxCurrentIcon = state.champions[i].objectIconIndex;
        state.leaderHandSlotBoxCurrentIcon[i] = state.champions[i].objectIconIndex;
    }

    check_int_eq(dm1_v1_champion_panel_dead_member_hand_refresh_run_pc34(
                     &state, &result),
                 1, "run returns success (no-change F0295)",
                 "F0295:1175-1182 no-change return C0_FALSE");

    check_int_eq(result.f0295HasIconChangedCount, 0,
                 "F0295 has-icon-changed count = 0",
                 "F0295:1175-1182 mutable icon guard + no change");
    check_int_eq(result.f0295SameIconCount >= 4, 1,
                 "F0295 same-icon count >= 4",
                 "F0295:1175-1182 same-icon return for each champion");
    check_int_eq(result.f0038DrawIconInSlotBoxCount, 0,
                 "F0038 draw-icon-in-slotbox count = 0",
                 "F0295:1175-1182 F0038 not called when icons match");
    check_int_eq(result.f0386DrawActionIconCount >= 1, 1,
                 "F0386 dispatch count >= 1 (F0292 still fires)",
                 "F0292:835 F0386 dispatch for the dead member");
    check_int_eq(result.deadMemberBlackFillCount >= 1, 1,
                 "dead member black-fill count >= 1",
                 "F0292 dead-status-box branch fills the dead zone");
    check_int_eq(result.deadMemberActionIconBlitCount, 0,
                 "dead member action icon blit count = 0 (no-change path)",
                 "F0386 dead-member guard never blits the icon");
    check_int_eq(result.liveMemberActionIconBlitCount, 0,
                 "live member action icon blit count = 0 (no-change path)",
                 "F0296:1231 F0386 not dispatched when F0295 = C0_FALSE");
    check_int_eq(result.mouseScreenUpdateBracketedPerF0296, 1,
                 "F0077/F0078 still balanced on no-change F0296 walk",
                 "F0296:1212-1251 balanced pair runs unconditionally");
}

static void test_f0292_dead_status_box_branch(void)
{
    Dm1V1ChampionPanelDeadMemberHandRefreshStatePc34 state;
    Dm1V1ChampionPanelDeadMemberHandRefreshResultPc34 result;

    memset(&state, 0, sizeof(state));
    dm1_v1_champion_panel_dead_member_hand_refresh_init_pc34(&state);

    /*
     * Force every champion's icon to match, so the F0296 walk has
     * no F0295-changed action hand to dispatch. The F0292
     * dead-status-box branch at line 835 must still call F0386 for
     * the dead member.
     */
    for (int i = 0; i < state.partyChampionCount; ++i) {
        state.champions[i].slotBoxCurrentIcon = state.champions[i].objectIconIndex;
        state.leaderHandSlotBoxCurrentIcon[i] = state.champions[i].objectIconIndex;
    }

    check_int_eq(dm1_v1_champion_panel_dead_member_hand_refresh_run_pc34(
                     &state, &result),
                 1, "run returns success (F0292 branch only)",
                 "F0292:816-839 dead-status-box branch");

    check_int_eq(result.f0292DeadStatusBoxBranchReachesF0386, 1,
                 "F0292 dead-status-box branch reaches F0386",
                 "F0292:835 F0386 dispatch for the dead member");
    check_int_eq(result.f0292InvocationCount, 1,
                 "F0292 invocation count = 1",
                 "one F0292 walk during the gate");
    check_int_eq(result.deadMemberBlackFillCount >= 1, 1,
                 "dead member black-fill count >= 1 (F0292 path)",
                 "F0292:835 -> F0386:234-242 C00_COLOR_BLACK fill");
    check_int_eq(result.deadMemberActionIconBlitCount, 0,
                 "dead member action icon blit count = 0 (F0292 path)",
                 "F0386:257-326 unreachable while CurrentHealth == 0");
}

static void test_dead_member_zone_id(void)
{
    Dm1V1ChampionPanelDeadMemberHandRefreshStatePc34 state;
    Dm1V1ChampionPanelDeadMemberHandRefreshResultPc34 result;

    memset(&state, 0, sizeof(state));
    dm1_v1_champion_panel_dead_member_hand_refresh_init_pc34(&state);

    check_int_eq(dm1_v1_champion_panel_dead_member_hand_refresh_run_pc34(
                     &state, &result),
                 1, "run returns success (zone check)",
                 "F0386:234-242 C089+championIndex zone");

    check_int_eq(state.champions[3].actionAreaZone, 92,
                 "dead member action area zone = 92",
                 "C089_ZONE_ACTION_AREA_CHAMPION_0_ACTION + 3 = 92");
    check_int_eq(state.champions[0].actionAreaZone, 89,
                 "leader action area zone = 89",
                 "C089_ZONE_ACTION_AREA_CHAMPION_0_ACTION + 0 = 89");
    check_int_eq(state.champions[1].actionAreaZone, 90,
                 "champion 1 action area zone = 90",
                 "C089 + 1 = 90");
    check_int_eq(state.champions[2].actionAreaZone, 91,
                 "champion 2 action area zone = 91",
                 "C089 + 2 = 91");
    check_int_eq(result.deadMemberZoneFilledAtCorrectIndex, 1,
                 "dead member zone filled at correct index",
                 "F0386:234-242 zone = C089 + 3");
}

static void test_rejects(void)
{
    Dm1V1ChampionPanelDeadMemberHandRefreshStatePc34 base;
    Dm1V1ChampionPanelDeadMemberHandRefreshResultPc34 result;
    Dm1V1ChampionPanelDeadMemberHandRefreshStatePc34 probe;
    int rejected;

    memset(&base, 0, sizeof(base));
    dm1_v1_champion_panel_dead_member_hand_refresh_init_pc34(&base);

    check_int_eq(dm1_v1_champion_panel_dead_member_hand_refresh_run_pc34(
                     &base, &result),
                 1, "baseline run returns success", "baseline gate");

    /* Reject 1: every member alive (no dead member). */
    probe = base;
    probe.champions[probe.deadMemberIndex].alive = 1;
    rejected = dm1_v1_champion_panel_dead_member_hand_refresh_run_pc34(
                   &probe, &result) == 0;
    check_int_eq(rejected, 1, "reject: no dead member",
                 "champion[deadMemberIndex].alive = 1");

    /* Reject 2: partyChampionCount = 0. */
    probe = base;
    probe.partyChampionCount = 0;
    probe.aliveMembers = 0;
    rejected = dm1_v1_champion_panel_dead_member_hand_refresh_run_pc34(
                   &probe, &result) == 0;
    check_int_eq(rejected, 1, "reject: party size zero",
                 "G0305_ui_PartyChampionCount = 0");

    /* Reject 3: leaderIndex = -1. */
    probe = base;
    probe.leaderIndex = -1;
    rejected = dm1_v1_champion_panel_dead_member_hand_refresh_run_pc34(
                   &probe, &result) == 0;
    check_int_eq(rejected, 1, "reject: negative leader index",
                 "G0411_i_LeaderIndex = -1");

    /* Reject 4: dead member alive (no C00_COLOR_BLACK fill on F0296). */
    probe = base;
    probe.champions[probe.deadMemberIndex].alive = 1;
    probe.champions[probe.deadMemberIndex].actionHandThing =
        DM1_V1_DMHR_THING_NONE_PC34;
    rejected = dm1_v1_champion_panel_dead_member_hand_refresh_run_pc34(
                   &probe, &result) == 0;
    check_int_eq(rejected, 1, "reject: dead member alive",
                 "alive member has no C00_COLOR_BLACK fill");

    /* Reject 5: NULL state. */
    rejected = dm1_v1_champion_panel_dead_member_hand_refresh_run_pc34(
                   NULL, &result) == 0;
    check_int_eq(rejected, 1, "reject: NULL state", "NULL state guard");

    /* Reject 6: NULL result. */
    memset(&probe, 0, sizeof(probe));
    dm1_v1_champion_panel_dead_member_hand_refresh_init_pc34(&probe);
    rejected = dm1_v1_champion_panel_dead_member_hand_refresh_run_pc34(
                   &probe, NULL) == 0;
    check_int_eq(rejected, 1, "reject: NULL result", "NULL result guard");
}

static void test_guards_match_expectations(void)
{
    Dm1V1ChampionPanelDeadMemberHandRefreshStatePc34 state;
    Dm1V1ChampionPanelDeadMemberHandRefreshResultPc34 result;

    memset(&state, 0, sizeof(state));
    dm1_v1_champion_panel_dead_member_hand_refresh_init_pc34(&state);

    check_int_eq(dm1_v1_champion_panel_dead_member_hand_refresh_run_pc34(
                     &state, &result),
                 1, "run returns success (guard test)",
                 "baseline gate");

    check_int_eq(result.rejectsPartyWithoutDeadMember, 1,
                 "guard rejects party without dead member",
                 "alive=1 for every champion");
    check_int_eq(result.rejectsPartySizeZero, 1,
                 "guard rejects party size zero", "partyChampionCount = 0");
    check_int_eq(result.rejectsNegativeLeaderIndex, 1,
                 "guard rejects negative leader index",
                 "leaderIndex = -1");
}

int main(void)
{
    test_evidence();
    test_f0296_f0295_f0386_dead_member_walk();
    test_f0295_no_change_keeps_f0296_silent();
    test_f0292_dead_status_box_branch();
    test_dead_member_zone_id();
    test_rejects();
    test_guards_match_expectations();

    Dm1V1ChampionPanelDeadMemberHandRefreshStatePc34 state;
    Dm1V1ChampionPanelDeadMemberHandRefreshResultPc34 result;
    memset(&state, 0, sizeof(state));
    dm1_v1_champion_panel_dead_member_hand_refresh_init_pc34(&state);
    dm1_v1_champion_panel_dead_member_hand_refresh_run_pc34(&state, &result);
    printf("dead_member_hand_refresh hash=0x%08X assertions=%d failures=%d\n",
           (unsigned)result.hash, g_assertions, g_failures);
    return g_failures == 0 ? 0 : 1;
}
