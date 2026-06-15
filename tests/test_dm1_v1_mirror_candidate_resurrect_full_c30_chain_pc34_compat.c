/* DM1 V1 mirror-candidate resurrect full-C30-chain regression gate.
 *
 * Source-lock anchors:
 * - COMMAND.C F0359:1985-1990 gates M568/C040 dispatch on empty hand.
 * - REVIVE.C F0280:124-132 publishes candidates; F0282:744-806 consumes
 *   C160/C161/C162 candidate commands.
 * - CHEST.C F0333:30-67 and F0334:113-132 own G0425/G0426 C30 chains.
 * - CHAMPION.C F0284/F0297/F0298/F0300/F0301/F0302 own party, hand, and
 *   slot mutation.
 * - PANEL.C F0344/F0345/F0352 and F0346/F0347:1619-1657 own panel redraw.
 * - DEFS.H:810-817 C30..C37, 873/876 M516, 1878 M070, 2088 C10,
 *   2200 C040, 3001-3008 M568/M569, 5876-5881 G0425/G0426.
 */
#include "dm1_v1_mirror_candidate_resurrect_full_c30_chain_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int gAssertions;
static int gFailures;

static void check_true(int condition, const char *message, const char *anchor)
{
    ++gAssertions;
    if (!condition) {
        ++gFailures;
        printf("FAIL: %s [%s]\n", message, anchor ? anchor : "(null)");
    }
}

static void check_uint_eq(unsigned int actual,
                          unsigned int expected,
                          const char *message,
                          const char *anchor)
{
    ++gAssertions;
    if (actual != expected) {
        ++gFailures;
        printf("FAIL: %s actual=0x%X expected=0x%X [%s]\n",
               message,
               actual,
               expected,
               anchor ? anchor : "(null)");
    }
}

static void check_int_eq(int actual,
                         int expected,
                         const char *message,
                         const char *anchor)
{
    ++gAssertions;
    if (actual != expected) {
        ++gFailures;
        printf("FAIL: %s actual=%d expected=%d [%s]\n",
               message,
               actual,
               expected,
               anchor ? anchor : "(null)");
    }
}

static void check_contains(const char *haystack,
                           const char *needle,
                           const char *message,
                           const char *anchor)
{
    ++gAssertions;
    if (!haystack || !needle || !strstr(haystack, needle)) {
        ++gFailures;
        printf("FAIL: %s missing=%s [%s]\n",
               message,
               needle ? needle : "(null)",
               anchor ? anchor : "(null)");
    }
}

static void check_source_lock_metadata(void)
{
    const Dm1V1MirrorCandidateResurrectFullC30EvidencePc34Compat *e =
        dm1_v1_mirror_candidate_resurrect_full_c30_evidence_pc34_compat();
    const char *source =
        dm1_v1_mirror_candidate_resurrect_full_c30_source_evidence_pc34_compat();

    check_true(e != NULL, "evidence accessor returns metadata", "metadata");
    check_int_eq(e->contractOnly, 1, "contract-only marker",
                 "contract_only=1");
    check_contains(e->commandAnchor, "COMMAND.C F0359:1985-1990",
                   "F0359 dispatch range is cited", e->commandAnchor);
    check_contains(e->reviveOpenAnchor, "REVIVE.C F0280:124-132",
                   "F0280 publish guard is cited", e->reviveOpenAnchor);
    check_contains(e->reviveFinishAnchor, "REVIVE.C F0282:744-806",
                   "F0282 consume range is cited", e->reviveFinishAnchor);
    check_contains(e->chestAnchor, "CHEST.C F0333:30-67",
                   "F0333 materialization is cited", e->chestAnchor);
    check_contains(e->chestAnchor, "F0334:113-132",
                   "F0334 rewrite is cited", e->chestAnchor);
    check_contains(e->championPartyAnchor, "CHAMPION.C F0284:93-131",
                   "F0284 party mutation is cited", e->championPartyAnchor);
    check_contains(e->championHandAnchor, "F0297:243-268",
                   "F0297 leader-hand put is cited", e->championHandAnchor);
    check_contains(e->championHandAnchor, "F0298:270-298",
                   "F0298 leader-hand remove is cited", e->championHandAnchor);
    check_contains(e->championSlotAnchor, "F0300:511-584",
                   "F0300 slot remove is cited", e->championSlotAnchor);
    check_contains(e->championSlotAnchor, "F0301:606-614",
                   "F0301 slot add is cited", e->championSlotAnchor);
    check_contains(e->championSlotAnchor, "F0302:662-713",
                   "F0302 slot command is cited", e->championSlotAnchor);
    check_contains(e->panelAnchor, "F0344/F0345",
                   "F0344/F0345 panel click/release are cited",
                   e->panelAnchor);
    check_contains(e->panelAnchor, "F0352",
                   "F0352 eye dispatch is cited", e->panelAnchor);
    check_contains(e->panelAnchor, "F0346/F0347:1619-1657",
                   "F0346/F0347 C040 redraw is cited", e->panelAnchor);
    check_contains(e->defsAnchor, "DEFS.H:810-817 C30..C37",
                   "DEFS C30..C37 are cited", e->defsAnchor);
    check_contains(e->defsAnchor, "873/876 M516",
                   "DEFS M516 is cited", e->defsAnchor);
    check_contains(e->defsAnchor, "1878 M070",
                   "DEFS M070 is cited", e->defsAnchor);
    check_contains(e->defsAnchor, "2088 C10",
                   "DEFS line 2088 is cited", e->defsAnchor);
    check_contains(e->defsAnchor, "2200 C040",
                   "DEFS C040 is cited", e->defsAnchor);
    check_contains(e->defsAnchor, "5876-5881 G0425/G0426",
                   "DEFS G0425/G0426 are cited", e->defsAnchor);
    check_contains(e->scope, "pass742 full-C30 resurrect rejection",
                   "scope names this pass and gap", e->scope);
    check_contains(e->scope, "pass728",
                   "scope marks non-overlap with pass728", e->scope);
    check_contains(e->scope, "pass738",
                   "scope marks non-overlap with pass738", e->scope);
    check_contains(source, "REVIVE.C F0282:744-806",
                   "source evidence includes F0282", "source evidence");
    check_contains(source, "DEFS.H:810-817 C30..C37",
                   "source evidence includes DEFS C30 range",
                   "source evidence");
}

static void check_full_c30_rejects_cleanly(void)
{
    Dm1V1MirrorCandidateResurrectFullC30StatePc34Compat state;
    Dm1V1MirrorCandidateResurrectFullC30ResultPc34Compat result;
    int ok;

    dm1_v1_mirror_candidate_resurrect_full_c30_init_pc34_compat(&state);
    check_int_eq(state.contractOnly, 1, "fixture is contract-only",
                 "contract_only=1");
    check_uint_eq(state.candidateOrdinal, 3u,
                  "fixture starts with candidate ordinal 3",
                  "REVIVE.C F0280:124-132");
    check_int_eq(state.partyChampionCount, 3,
                 "party has one free champion row",
                 "REVIVE.C F0280:127-132");
    check_int_eq(state.leaderEmptyHanded, 1,
                 "leader hand is empty for F0359 admission",
                 "COMMAND.C F0359:1985-1990");
    check_int_eq(state.c040PanelOpen, 1, "C040 panel is open",
                 "PANEL.C F0346/F0347:1619-1657");
    check_int_eq(state.panelContent, DM1_V1_MCRFC30_M568_PANEL_PC34_COMPAT,
                 "M568 owns the panel content", "DEFS.H:3001-3008");
    check_uint_eq(state.c30Chain[0], 0x7300u, "C30 slot 1 is occupied",
                  "CHEST.C F0333:64-65");
    check_uint_eq(state.c30Chain[7], 0x7307u, "C37 slot 8 is occupied",
                  "DEFS.H:810-817");

    ok = dm1_v1_mirror_candidate_resurrect_full_c30_attempt_pc34_compat(
        &state, &result);

    check_int_eq(ok, 0, "full C30 chain rejects the resurrect click",
                 "CHAMPION.C F0302:688-698");
    check_int_eq(result.accepted, 0, "result is not accepted",
                 "REVIVE.C F0282:785-806");
    check_int_eq(result.rejectedNoEmptyC30, 1,
                 "result records no-empty-C30 rejection",
                 "DEFS.H:810-817 C30..C37");
    check_int_eq(result.cleanFailure, 1,
                 "rejection is clean and non-destructive",
                 "CHEST.C F0334:113-132");
    check_int_eq(result.c30Unchanged, 1, "C30 chain remains unchanged",
                 "CHEST.C F0333/F0334");
    check_int_eq(result.championSlotsUnchanged, 1,
                 "candidate champion slots remain unchanged",
                 "REVIVE.C F0282:789-793");
    check_uint_eq(result.candidateAfter, 3u,
                  "G0299 candidate remains pending",
                  "REVIVE.C F0280/F0282");
    check_int_eq(result.panelOpenAfter, 1, "C040 remains open after reject",
                 "PANEL.C F0346/F0347:1619-1657");
    check_uint_eq(result.leaderHandAfter,
                  DM1_V1_MCRFC30_THING_NONE_PC34_COMPAT,
                  "leader hand remains empty",
                  "CHAMPION.C F0297/F0298");
    check_int_eq(result.f0282After, result.f0282Before,
                 "F0282 consume count does not advance",
                 "REVIVE.C F0282:744-806");
    check_int_eq(result.f0334After, result.f0334Before,
                 "F0334 close/relink count does not advance",
                 "CHEST.C F0334:113-132");
    check_int_eq(state.f0359PanelDispatchCount, 1,
                 "F0359 saw exactly one C160 panel dispatch",
                 "COMMAND.C F0359:1985-1990");
    check_int_eq(state.f0300RemoveSlotCount, 0,
                 "no slot removal runs on rejection",
                 "CHAMPION.C F0300:511-584");
    check_int_eq(state.f0301AddSlotCount, 0,
                 "no slot add runs on rejection",
                 "CHAMPION.C F0301:606-614");
    check_int_eq(state.f0334CloseChestCount, 0,
                 "open chest is not relinked by rejection",
                 "CHEST.C F0334:113-132");
    check_int_eq(state.noCrashGuard, 1, "no-crash guard remains armed",
                 "runtime guard");
}

static void check_free_slot_allows_same_resurrect_path(void)
{
    Dm1V1MirrorCandidateResurrectFullC30StatePc34Compat state;
    Dm1V1MirrorCandidateResurrectFullC30ResultPc34Compat result;
    int ok;

    dm1_v1_mirror_candidate_resurrect_full_c30_init_pc34_compat(&state);
    dm1_v1_mirror_candidate_resurrect_full_c30_free_last_slot_pc34_compat(
        &state);
    ok = dm1_v1_mirror_candidate_resurrect_full_c30_attempt_pc34_compat(
        &state, &result);

    check_int_eq(ok, 1, "same path succeeds after freeing C37",
                 "DEFS.H:817 C37_SLOT_CHEST_8");
    check_int_eq(result.accepted, 1, "result is accepted with one free slot",
                 "REVIVE.C F0282:785-806");
    check_int_eq(result.cleanSuccess, 1, "success clears candidate cleanly",
                 "REVIVE.C F0282:785-806");
    check_uint_eq(result.candidateAfter, 0u, "candidate ordinal is cleared",
                  "REVIVE.C F0282:785");
    check_int_eq(result.panelOpenAfter, 0, "C040 closes after success",
                 "PANEL.C F0346/F0347:1619-1657");
    check_uint_eq(result.c30Slot7After, 0x7A42u,
                  "freed C37 receives deterministic resurrect marker",
                  "CHAMPION.C F0301:606-614");
    check_int_eq(state.f0282FinishCount, 1, "F0282 count advances once",
                 "REVIVE.C F0282:744-806");
    check_int_eq(state.f0301AddSlotCount, 1, "F0301 add count advances once",
                 "CHAMPION.C F0301:606-614");
    check_int_eq(state.f0334CloseChestCount, 0,
                 "success path still avoids chest close/relink",
                 "CHEST.C F0334:113-132");
}

int main(void)
{
    int ok;
    int selfAssertions;
    int selfFailures;
    unsigned int hash;

    check_source_lock_metadata();
    check_full_c30_rejects_cleanly();
    check_free_slot_allows_same_resurrect_path();

    ok = dm1_v1_mirror_candidate_resurrect_full_c30_run_self_test_pc34_compat();
    selfAssertions =
        dm1_v1_mirror_candidate_resurrect_full_c30_assertions_pc34_compat();
    selfFailures =
        dm1_v1_mirror_candidate_resurrect_full_c30_failures_pc34_compat();
    hash = dm1_v1_mirror_candidate_resurrect_full_c30_hash_pc34_compat();

    check_int_eq(ok, 1, "library self-test passes", "self-test");
    check_int_eq(selfFailures, 0, "library self-test has no failures",
                 "self-test");
    check_true(selfAssertions >= 35,
               "library self-test covers reject and accepted paths",
               "self-test");
    check_true(hash != 0u, "deterministic FNV hash is non-zero",
               "deterministic hash");

    if (gFailures != 0 || selfFailures != 0) {
        printf("FAIL dm1_v1_mirror_candidate_resurrect_full_c30_chain_pc34_compat assertions=%d failures=%d self_assertions=%d self_failures=%d hash=0x%08X\n",
               gAssertions,
               gFailures,
               selfAssertions,
               selfFailures,
               hash);
        return 1;
    }

    printf("PASS dm1_v1_mirror_candidate_resurrect_full_c30_chain_pc34_compat assertions=%d failures=0 self_assertions=%d self_failures=0 hash=0x%08X\n",
           gAssertions,
           selfAssertions,
           hash);
    return 0;
}
