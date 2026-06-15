#include "dm1_v1_mirror_candidate_inventory_toggle_pc34_compat.h"

#include <stdio.h>
#include <string.h>

/* ReDMCSB source-lock anchors tested here:
 * COMMAND.C F0380:2180-2184 C007..C011 inventory-toggle gate.
 * DEFS.H:244-248 C007..C011; DEFS.H:712-716 C04 close inventory.
 * DEFS.H:5694,5700 G0299/G0305; DEFS.H:5876 G0423.
 * PANEL.C F0355_INVENTORY_Toggle_CPSE:2244-2248 entrypoint signature.
 * COMMAND.C:2302-2311 sibling spell/action !G0299 gate.
 */

static int gTests;
static int gPasses;

#define CHECK_REDMCSB(cond, msg, anchor) do { \
    ++gTests; \
    if (cond) { \
        ++gPasses; \
    } else { \
        printf("FAIL: %s [%s]\n", msg, anchor); \
    } \
} while (0)

static Dm1V1MirrorCandidateInventoryToggleOutputPc34Compat probe(
    int command,
    unsigned int candidateOrdinal,
    unsigned int partyChampionCount,
    int inventoryOrdinal,
    int *returnValue)
{
    Dm1V1MirrorCandidateInventoryToggleInputPc34Compat input;
    Dm1V1MirrorCandidateInventoryToggleOutputPc34Compat output;

    input.command = command;
    input.candidate_champion_ordinal = candidateOrdinal;
    input.party_champion_count = partyChampionCount;
    input.current_inventory_champion_ordinal = inventoryOrdinal;
    *returnValue =
        dm1_v1_mirror_candidate_inventory_toggle_pc34_compat_probe(
            &input, &output);
    return output;
}

static void test_c007_in_range_no_candidate_dispatches(void)
{
    int returned;
    Dm1V1MirrorCandidateInventoryToggleOutputPc34Compat out =
        probe(7, 0u, 1u, 0, &returned);
    const char *anchor = out.evidence->commandGateAnchor;

    CHECK_REDMCSB(returned == 1, "C007 returns dispatched", anchor);
    CHECK_REDMCSB(out.command_in_inventory_toggle_range == 1,
                  "C007 is inside C007..C011", out.evidence->defsCommandAnchor);
    CHECK_REDMCSB(out.computed_champion_index == 0 &&
                      out.target_champion_index == 0,
                  "C007 computes championIndex zero", anchor);
    CHECK_REDMCSB(out.candidate_gate_passed == 1 &&
                      out.party_gate_passed == 1,
                  "C007 passes !G0299 and party-count gates", anchor);
    CHECK_REDMCSB(out.should_dispatch_toggle == 1 &&
                      out.would_call_f0355 == 1,
                  "C007 would call F0355", out.evidence->toggleEntrypointAnchor);
    CHECK_REDMCSB(out.route_taken ==
                      DM1_V1_MIRROR_CANDIDATE_INVENTORY_TOGGLE_DISPATCHED_PARTY_CHAMPION_PC34_COMPAT,
                  "C007 uses party champion route", anchor);
}

static void test_c011_close_no_candidate_dispatches(void)
{
    int returned;
    Dm1V1MirrorCandidateInventoryToggleOutputPc34Compat out =
        probe(11, 0u, 0u, 3, &returned);
    const char *anchor = out.evidence->commandGateAnchor;

    CHECK_REDMCSB(returned == 1, "C011 close returns dispatched", anchor);
    CHECK_REDMCSB(out.computed_champion_index == 4 &&
                      out.target_champion_index == 4,
                  "C011 computes C04 close champion index", anchor);
    CHECK_REDMCSB(out.is_close_inventory_command == 1,
                  "close command matches C04_CHAMPION_CLOSE_INVENTORY",
                  out.evidence->defsChampionAnchor);
    CHECK_REDMCSB(out.party_gate_passed == 1 &&
                      out.champion_index_inside_party == 0,
                  "close command bypasses party count", anchor);
    CHECK_REDMCSB(out.route_taken ==
                      DM1_V1_MIRROR_CANDIDATE_INVENTORY_TOGGLE_DISPATCHED_CLOSED_INVENTORY_PC34_COMPAT,
                  "C011 uses close-inventory dispatch route", anchor);
    CHECK_REDMCSB(out.inventory_ordinal_before == 3 &&
                      out.inventory_ordinal_after == 3,
                  "contract probe does not mutate G0423", out.evidence->defsInventoryOrdinalAnchor);
}

static void test_c007_out_of_range_no_candidate_blocks(void)
{
    int returned;
    Dm1V1MirrorCandidateInventoryToggleOutputPc34Compat out =
        probe(7, 0u, 0u, 1, &returned);
    const char *anchor = out.evidence->commandGateAnchor;

    CHECK_REDMCSB(returned == 0, "C007 with empty party does not dispatch", anchor);
    CHECK_REDMCSB(out.candidate_gate_passed == 1,
                  "clear G0299 alone is insufficient", anchor);
    CHECK_REDMCSB(out.champion_index_inside_party == 0 &&
                      out.party_gate_passed == 0,
                  "championIndex zero is outside G0305 zero", out.evidence->defsGlobalsAnchor);
    CHECK_REDMCSB(out.should_dispatch_toggle == 0 &&
                      out.would_call_f0355 == 0,
                  "out-of-party C007 does not call F0355", out.evidence->toggleEntrypointAnchor);
    CHECK_REDMCSB(out.route_taken ==
                      DM1_V1_MIRROR_CANDIDATE_INVENTORY_TOGGLE_BLOCKED_OUT_OF_PARTY_PC34_COMPAT,
                  "out-of-party route is reported distinctly", anchor);
}

static void test_c007_in_range_g0299_blocks(void)
{
    int returned;
    Dm1V1MirrorCandidateInventoryToggleOutputPc34Compat out =
        probe(7, 2u, 1u, 1, &returned);
    const char *anchor = out.evidence->commandGateAnchor;

    CHECK_REDMCSB(returned == 0, "C007 with G0299 set does not dispatch", anchor);
    CHECK_REDMCSB(out.candidate_gate_passed == 0,
                  "candidate ordinal blocks inventory toggle", out.evidence->defsGlobalsAnchor);
    CHECK_REDMCSB(out.party_gate_passed == 1,
                  "blocked candidate case was otherwise in party", anchor);
    CHECK_REDMCSB(out.route_taken ==
                      DM1_V1_MIRROR_CANDIDATE_INVENTORY_TOGGLE_BLOCKED_BY_G0299_PC34_COMPAT,
                  "G0299 route is reported distinctly", anchor);
}

static void test_c011_close_g0299_blocks(void)
{
    int returned;
    Dm1V1MirrorCandidateInventoryToggleOutputPc34Compat out =
        probe(11, 4u, 0u, 2, &returned);
    const char *anchor = out.evidence->commandGateAnchor;

    CHECK_REDMCSB(returned == 0, "C011 close with G0299 set does not dispatch", anchor);
    CHECK_REDMCSB(out.is_close_inventory_command == 1 &&
                      out.party_gate_passed == 1,
                  "close still passes the C04 side of the party gate", anchor);
    CHECK_REDMCSB(out.candidate_gate_passed == 0 &&
                      out.should_dispatch_toggle == 0,
                  "G0299 blocks even the close-inventory command", anchor);
    CHECK_REDMCSB(out.route_taken ==
                      DM1_V1_MIRROR_CANDIDATE_INVENTORY_TOGGLE_BLOCKED_BY_G0299_PC34_COMPAT,
                  "close blocked by candidate route is reported", anchor);
}

static void test_party_count_boundaries(void)
{
    int returned;
    Dm1V1MirrorCandidateInventoryToggleOutputPc34Compat inBoundary =
        probe(10, 0u, 4u, 4, &returned);
    const char *anchor = inBoundary.evidence->commandGateAnchor;

    CHECK_REDMCSB(returned == 1, "C010 boundary dispatches at G0305 - 1", anchor);
    CHECK_REDMCSB(inBoundary.computed_champion_index == 3 &&
                      inBoundary.champion_index_inside_party == 1,
                  "championIndex three is inside party count four", anchor);
    CHECK_REDMCSB(inBoundary.route_taken ==
                      DM1_V1_MIRROR_CANDIDATE_INVENTORY_TOGGLE_DISPATCHED_PARTY_CHAMPION_PC34_COMPAT,
                  "G0305 - 1 boundary uses party champion route", anchor);

    Dm1V1MirrorCandidateInventoryToggleOutputPc34Compat outBoundary =
        probe(10, 0u, 3u, 4, &returned);
    CHECK_REDMCSB(returned == 0, "C010 boundary blocks at championIndex == G0305", anchor);
    CHECK_REDMCSB(outBoundary.computed_champion_index == 3 &&
                      outBoundary.champion_index_inside_party == 0,
                  "championIndex three is outside party count three", anchor);
    CHECK_REDMCSB(outBoundary.route_taken ==
                      DM1_V1_MIRROR_CANDIDATE_INVENTORY_TOGGLE_BLOCKED_OUT_OF_PARTY_PC34_COMPAT,
                  "G0305 equal boundary is blocked out-of-party", anchor);
}

static void test_commands_outside_range_are_not_this_route(void)
{
    int returned;
    Dm1V1MirrorCandidateInventoryToggleOutputPc34Compat below =
        probe(6, 0u, 4u, 0, &returned);
    const char *anchor = below.evidence->commandGateAnchor;

    CHECK_REDMCSB(returned == 0, "C006 is not the inventory-toggle route", anchor);
    CHECK_REDMCSB(below.command_in_inventory_toggle_range == 0,
                  "C006 is below C007", below.evidence->defsCommandAnchor);
    CHECK_REDMCSB(below.target_champion_index ==
                      DM1_V1_MIRROR_CANDIDATE_INVENTORY_TOGGLE_NONE_PC34_COMPAT,
                  "out-of-range command has no target champion", anchor);
    CHECK_REDMCSB(below.route_taken ==
                      DM1_V1_MIRROR_CANDIDATE_INVENTORY_TOGGLE_NOT_IN_INVENTORY_TOGGLE_RANGE_PC34_COMPAT,
                  "below-range command is not marked blocked", anchor);

    Dm1V1MirrorCandidateInventoryToggleOutputPc34Compat above =
        probe(12, 3u, 4u, 0, &returned);
    CHECK_REDMCSB(returned == 0, "C012 is not the inventory-toggle route", anchor);
    CHECK_REDMCSB(above.command_in_inventory_toggle_range == 0,
                  "C012 is above C011", above.evidence->defsCommandAnchor);
    CHECK_REDMCSB(above.route_taken ==
                      DM1_V1_MIRROR_CANDIDATE_INVENTORY_TOGGLE_NOT_IN_INVENTORY_TOGGLE_RANGE_PC34_COMPAT,
                  "above-range command is not marked blocked by G0299", anchor);
}

static void test_source_lock_metadata(void)
{
    const Dm1V1MirrorCandidateInventoryToggleEvidencePc34Compat *e =
        dm1_v1_mirror_candidate_inventory_toggle_pc34_compat_evidence();

    CHECK_REDMCSB(e != NULL,
                  "evidence struct is available",
                  "COMMAND.C F0380:2180-2184");
    CHECK_REDMCSB(strstr(e->commandGateAnchor, "2180-2184") != NULL,
                  "COMMAND.C inventory-toggle anchor is present",
                  e->commandGateAnchor);
    CHECK_REDMCSB(strstr(e->defsCommandAnchor, "244-248") != NULL,
                  "DEFS.H command constants anchor is present",
                  e->defsCommandAnchor);
    CHECK_REDMCSB(strstr(e->defsChampionAnchor, "C04") != NULL,
                  "C04 close-inventory anchor is present",
                  e->defsChampionAnchor);
    CHECK_REDMCSB(strstr(e->defsGlobalsAnchor, "5694") != NULL &&
                      strstr(e->defsGlobalsAnchor, "5700") != NULL,
                  "G0299/G0305 global anchors are present",
                  e->defsGlobalsAnchor);
    CHECK_REDMCSB(strstr(e->defsInventoryOrdinalAnchor, "5876") != NULL,
                  "G0423 inventory ordinal anchor is present",
                  e->defsInventoryOrdinalAnchor);
    CHECK_REDMCSB(strstr(e->toggleEntrypointAnchor, "2244-2248") != NULL,
                  "F0355 entrypoint anchor is present",
                  e->toggleEntrypointAnchor);
    CHECK_REDMCSB(strstr(e->spellActionGateAnchor, "2302-2311") != NULL,
                  "sibling spell/action !G0299 gate anchor is present",
                  e->spellActionGateAnchor);
    CHECK_REDMCSB(strstr(e->contractScope, "contract-only") != NULL &&
                      strstr(e->contractScope, "no real inventory") != NULL,
                  "contract scope rejects runtime inventory claims",
                  e->contractScope);
    CHECK_REDMCSB(strstr(e->disjointFunctions, "click_cancel") != NULL &&
                      strstr(e->disjointFunctions, "close_button") != NULL &&
                      strstr(e->disjointFunctions, "icon_refresh") != NULL,
                  "evidence lists disjoint mirror-candidate slices",
                  e->disjointFunctions);
    CHECK_REDMCSB(strstr(e->disjointFunctions, "champion_mirror_pc34_compat") != NULL,
                  "evidence lists champion mirror disjointness",
                  e->disjointFunctions);
}

int main(void)
{
    test_c007_in_range_no_candidate_dispatches();
    test_c011_close_no_candidate_dispatches();
    test_c007_out_of_range_no_candidate_blocks();
    test_c007_in_range_g0299_blocks();
    test_c011_close_g0299_blocks();
    test_party_count_boundaries();
    test_commands_outside_range_are_not_this_route();
    test_source_lock_metadata();

    printf("PASS dm1_v1_mirror_candidate_inventory_toggle_pc34_compat %d/%d assertions\n",
           gPasses, gTests);
    return gPasses == gTests ? 0 : 1;
}
