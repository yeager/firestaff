/* DM1 V1 mirror candidate scroll-pickup while party rotation is in progress.
 *
 * Source-lock anchors:
 * - PANEL.C F0344:1895-1944 + F0345:1946-1999 route the panel click/cell path.
 * - CHAMPION.C F0297:243-268, F0298:270-298, F0302:662-713.
 * - COMMAND.C F0359:1985-1990, F0361:1709-1813, F0380:2045-2156.
 * - MOUSE.C F0077:97-126 + F0078:128-168.
 * - REVIVE.C F0280:124-132 + F0282:744-806.
 * - CHAMDRAW.C F0291:498-560, F0292:703-735, F0296:1185-1252.
 * - DEFS.H:277 C040; 810 C30; 3906-3913 C537..C544.
 */
#include "firestaff/dm1/v1/mirror/dm1_v1_mirror_candidate_scroll_pickup_with_party_rotate_in_progress_pc34_compat.h"

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

static void check_int_eq(int actual, int expected, const char *message,
                         const char *anchor)
{
    ++gAssertions;
    if (actual != expected) {
        ++gFailures;
        printf("FAIL: %s actual=%d expected=%d [%s]\n",
               message, actual, expected, anchor ? anchor : "(null)");
    }
}

static void check_uint_eq(unsigned int actual, unsigned int expected,
                          const char *message, const char *anchor)
{
    ++gAssertions;
    if (actual != expected) {
        ++gFailures;
        printf("FAIL: %s actual=0x%08X expected=0x%08X [%s]\n",
               message, actual, expected, anchor ? anchor : "(null)");
    }
}

static void check_contains(const char *haystack, const char *needle,
                           const char *message, const char *anchor)
{
    ++gAssertions;
    if (!haystack || !needle || !strstr(haystack, needle)) {
        ++gFailures;
        printf("FAIL: %s missing=%s [%s]\n",
               message, needle ? needle : "(null)",
               anchor ? anchor : "(null)");
    }
}

static void test_evidence(void)
{
    const Dm1V1MirrorCandidateScrollPickupWithPartyRotateEvidencePc34Compat *e =
        DM1_V1_MirrorCandidateScrollPickupWithPartyRotate_EvidencePc34Compat();

    check_true(e != NULL, "evidence accessor returns metadata", "metadata");
    check_int_eq(e->contractOnly, 1, "contract-only marker",
                 "contract_only=1");
    check_contains(e->panelClickAnchor, "PANEL.C F0344:1895-1944",
                   "panel anchor cites F0344", e->panelClickAnchor);
    check_contains(e->panelClickAnchor, "F0345:1946-1999",
                   "panel anchor cites F0345", e->panelClickAnchor);
    check_contains(e->championPutAnchor, "CHAMPION.C F0297:243-268",
                   "champion put anchor cites F0297", e->championPutAnchor);
    check_contains(e->championRemoveAnchor, "CHAMPION.C F0298:270-298",
                   "champion remove anchor cites F0298",
                   e->championRemoveAnchor);
    check_contains(e->championSlotClickAnchor, "F0302:662-713",
                   "slot click anchor cites F0302",
                   e->championSlotClickAnchor);
    check_contains(e->championSlotClickAnchor, "C30/C537..C544",
                   "slot click anchor names C30/C537..C544",
                   e->championSlotClickAnchor);
    check_contains(e->commandClickAnchor, "COMMAND.C F0359:1985-1990",
                   "command click anchor cites F0359", e->commandClickAnchor);
    check_contains(e->commandKeyQueueAnchor, "COMMAND.C F0361:1709-1813",
                   "queue-write anchor cites F0361",
                   e->commandKeyQueueAnchor);
    check_contains(e->commandDispatchAnchor, "COMMAND.C F0380:2045-2156",
                   "dispatch anchor cites F0380", e->commandDispatchAnchor);
    check_contains(e->commandDispatchAnchor, "pending-click unlock",
                   "dispatch anchor names pending-click unlock",
                   e->commandDispatchAnchor);
    check_contains(e->mouseQueueAnchor, "MOUSE.C F0077:97-126",
                   "mouse anchor cites F0077", e->mouseQueueAnchor);
    check_contains(e->mouseQueueAnchor, "F0078:128-168",
                   "mouse anchor cites F0078", e->mouseQueueAnchor);
    check_contains(e->reviveOpenAnchor, "REVIVE.C F0280:124-132",
                   "revive open anchor cites F0280", e->reviveOpenAnchor);
    check_contains(e->reviveDecisionAnchor, "REVIVE.C F0282:744-806",
                   "revive decision anchor cites F0282",
                   e->reviveDecisionAnchor);
    check_contains(e->chamdrawSlotAnchor, "CHAMDRAW.C F0291:498-560",
                   "draw slot anchor cites F0291", e->chamdrawSlotAnchor);
    check_contains(e->chamdrawStateAnchor, "CHAMDRAW.C F0292:703-735",
                   "draw state anchor cites F0292", e->chamdrawStateAnchor);
    check_contains(e->chamdrawChangedAnchor, "CHAMDRAW.C F0296:1185-1252",
                   "changed-object anchor cites F0296",
                   e->chamdrawChangedAnchor);
    check_contains(e->defsAnchor, "DEFS.H:277 C040",
                   "defs anchor cites C040", e->defsAnchor);
    check_contains(e->defsAnchor, "810 C30",
                   "defs anchor cites C30", e->defsAnchor);
    check_contains(e->defsAnchor, "3906-3913 C537..C544",
                   "defs anchor cites chest zones", e->defsAnchor);
    check_contains(e->defsAnchor, "M516_CHAMPIONS",
                   "defs anchor cites champion array", e->defsAnchor);
    check_contains(e->nonOverlapScope, "party rotation",
                   "scope names party rotation", e->nonOverlapScope);
    check_contains(e->nonOverlapScope, "not candidate-internal rotation",
                   "scope excludes candidate-internal rotation",
                   e->nonOverlapScope);
    check_contains(e->nonOverlapScope, "pass760",
                   "scope records prior pass exclusion", e->nonOverlapScope);
}

static void test_initial_and_manual_sequence(void)
{
    Dm1V1MirrorCandidateScrollPickupWithPartyRotateStatePc34Compat state;
    Dm1V1MirrorCandidateScrollPickupWithPartyRotateSnapshotPc34Compat before;
    Dm1V1MirrorCandidateScrollPickupWithPartyRotateSnapshotPc34Compat after;
    unsigned int hashBefore;
    unsigned int hashAfter;

    DM1_V1_MirrorCandidateScrollPickupWithPartyRotate_InitPc34Compat(&state);
    check_int_eq(state.contractOnly, 1, "initial contract-only flag",
                 "contract_only=1");
    check_int_eq(state.partyChampionCount, 3, "party starts with candidate room",
                 "REVIVE.C F0280:124-132");
    check_int_eq(state.leaderIndex, 0, "leader index starts at zero",
                 "CHAMPION.C F0297:243-268");
    check_int_eq(state.partyDirection, 0, "party starts north",
                 "COMMAND.C F0380:2045-2156");
    check_int_eq(state.candidateChampionIndex, 3, "candidate row index",
                 "REVIVE.C F0280:124-132");
    check_int_eq(state.candidateOrdinal, 4, "candidate ordinal is live target",
                 "DEFS.H:5694 G0299");
    check_int_eq(state.candidateIndex, 2, "candidate index seed",
                 "REVIVE.C F0280:124-132");
    check_int_eq(state.candidateChainLength, 3, "candidate chain length seed",
                 "REVIVE.C F0280:124-132");
    check_int_eq(state.c040PanelOpen, 0, "C040 starts closed",
                 "DEFS.H:277 C040");
    check_uint_eq(state.leaderHandThing, DM1_V1_MCSPPR_THING_NONE_PC34,
                  "leader hand starts empty", "CHAMPION.C F0298:270-298");
    check_int_eq(state.leaderHandEmpty, 1, "leader empty flag starts true",
                 "REVIVE.C F0280:124-132");
    check_int_eq(state.scrollChestSlotIndex, 2, "scroll starts in chest slot 3",
                 "DEFS.H:810 C30");
    check_int_eq(state.scrollPc34Slot, DM1_V1_MCSPPR_C30_CHEST_SLOT_1_PC34 + 2,
                 "scroll pc34 slot uses C30 offset", "DEFS.H:810 C30");
    check_int_eq(state.scrollZone, DM1_V1_MCSPPR_C537_ZONE_CHEST_SLOT_1_PC34 + 2,
                 "scroll zone uses C537 offset", "DEFS.H:3906-3913");
    check_uint_eq(state.chestSlots[2], DM1_V1_MCSPPR_SCROLL_THING_PC34,
                  "scroll thing is seeded in chest slot",
                  "CHAMPION.C F0302:662-713");

    check_true(DM1_V1_MirrorCandidateScrollPickupWithPartyRotate_OpenCandidatePc34Compat(&state),
               "candidate panel opens", "REVIVE.C F0280:124-132");
    check_int_eq(state.c040PanelOpen, 1, "C040 panel is live",
                 "DEFS.H:277 C040");
    check_int_eq(state.c040Graphic, DM1_V1_MCSPPR_C040_GRAPHIC_PC34,
                 "C040 graphic is set", "DEFS.H:277 C040");
    check_int_eq(state.f0280OpenCount, 1, "F0280 open counted",
                 "REVIVE.C F0280:124-132");
    check_int_eq(state.f0291SlotDrawCount, 1, "F0291 slot draw counted",
                 "CHAMDRAW.C F0291:498-560");
    check_int_eq(state.f0292StateDrawCount, 1, "F0292 state draw counted",
                 "CHAMDRAW.C F0292:703-735");
    check_int_eq(state.f0296ChangedObjectIconCount, 1,
                 "F0296 changed-icon draw counted",
                 "CHAMDRAW.C F0296:1185-1252");

    check_true(DM1_V1_MirrorCandidateScrollPickupWithPartyRotate_QueueTurnPc34Compat(&state, DM1_V1_MCSPPR_C002_TURN_RIGHT_PC34),
               "C002 turn is queued by F0361", "COMMAND.C F0361:1709-1813");
    check_int_eq(state.f0361QueueWriteCount, 1, "F0361 write counted",
                 "COMMAND.C F0361:1709-1813");
    check_int_eq(state.queuedTurnCommand, DM1_V1_MCSPPR_C002_TURN_RIGHT_PC34,
                 "queued turn command is right turn",
                 "COMMAND.C F0380:2045-2156");
    check_true(DM1_V1_MirrorCandidateScrollPickupWithPartyRotate_BeginF0380PartyRotationPc34Compat(&state),
               "F0380 party rotation begins", "COMMAND.C F0380:2045-2156");
    check_int_eq(state.commandQueueLocked, 1, "queue remains locked mid-dispatch",
                 "COMMAND.C F0380:2075-2127");
    check_int_eq(state.f0380DispatchInProgress, 1,
                 "F0380 dispatch is marked in progress",
                 "COMMAND.C F0380:2045-2156");
    check_int_eq(state.partyRotationInProgress, 1,
                 "party rotation is in progress",
                 "COMMAND.C F0380:2150-2156");
    check_int_eq(state.candidateInternalRotationCount, 0,
                 "candidate-internal rotation is not used",
                 "PANEL.C F0344/F0345");

    DM1_V1_MirrorCandidateScrollPickupWithPartyRotate_SnapshotPc34Compat(&state, &before);
    hashBefore =
        DM1_V1_MirrorCandidateScrollPickupWithPartyRotate_HashPc34Compat(&state, NULL);
    check_true(!DM1_V1_MirrorCandidateScrollPickupWithPartyRotate_ClickScrollPickupPc34Compat(&state),
               "scroll pickup is ignored during party rotation",
               "COMMAND.C F0380:2045-2156");
    DM1_V1_MirrorCandidateScrollPickupWithPartyRotate_SnapshotPc34Compat(&state, &after);
    hashAfter =
        DM1_V1_MirrorCandidateScrollPickupWithPartyRotate_HashPc34Compat(&state, NULL);
    check_int_eq(state.ignoredPickupDuringPartyRotationCount, 1,
                 "ignored pickup counted once", "PANEL.C F0344/F0345");
    check_int_eq(state.f0359PanelClickCount, 1, "F0359 panel click counted",
                 "COMMAND.C F0359:1985-1990");
    check_int_eq(state.f0344PanelCellRouteCount, 1,
                 "F0344 panel route counted", "PANEL.C F0344:1895-1944");
    check_int_eq(state.f0345PanelHighlightRouteCount, 1,
                 "F0345 panel route counted", "PANEL.C F0345:1946-1999");
    check_int_eq(state.f0302SlotClickCount, 0,
                 "F0302 slot pickup does not run mid-rotation",
                 "CHAMPION.C F0302:662-713");
    check_int_eq(state.f0297PutCount, 0,
                 "leader hand put does not run mid-rotation",
                 "CHAMPION.C F0297:243-268");
    check_uint_eq(state.leaderHandThing, DM1_V1_MCSPPR_THING_NONE_PC34,
                  "leader hand stays empty during ignored click",
                  "CHAMPION.C F0297/F0298");
    check_uint_eq(state.chestSlots[2], DM1_V1_MCSPPR_SCROLL_THING_PC34,
                  "scroll slot stays populated during ignored click",
                  "CHAMPION.C F0302:662-713");
    check_int_eq(after.candidateOrdinal, before.candidateOrdinal,
                 "candidate ordinal preserved during ignore",
                 "REVIVE.C F0280/F0282");
    check_int_eq(after.candidateIndex, before.candidateIndex,
                 "candidate index preserved during ignore",
                 "REVIVE.C F0280/F0282");
    check_int_eq(after.candidateChainLength, before.candidateChainLength,
                 "candidate chain length preserved during ignore",
                 "REVIVE.C F0280/F0282");
    check_uint_eq(after.candidateChainHash, before.candidateChainHash,
                  "candidate chain hash preserved during ignore",
                  "REVIVE.C F0280/F0282");
    check_int_eq(memcmp(after.candidateChain, before.candidateChain,
                        sizeof(after.candidateChain)),
                 0, "candidate chain bytes preserved during ignore",
                 "REVIVE.C F0280/F0282");
    check_int_eq(after.c040PanelOpen, before.c040PanelOpen,
                 "C040 panel-open flag preserved during ignore",
                 "DEFS.H:277 C040");
    check_int_eq(after.c040Graphic, before.c040Graphic,
                 "C040 graphic preserved during ignore", "DEFS.H:277 C040");
    check_int_eq(after.c040RedrawSerial, before.c040RedrawSerial,
                 "C040 redraw serial preserved during ignore",
                 "CHAMDRAW.C F0291/F0292/F0296");
    check_uint_eq(after.c040RedrawHash, before.c040RedrawHash,
                  "C040 redraw hash preserved during ignore",
                  "CHAMDRAW.C F0291/F0292/F0296");
    check_int_eq(after.f0282DecisionCount, before.f0282DecisionCount,
                 "F0282 decision path did not fire during ignore",
                 "REVIVE.C F0282:744-806");
    check_int_eq(after.candidateInternalRotationCount,
                 before.candidateInternalRotationCount,
                 "candidate-internal rotation count remains stable",
                 "PANEL.C F0344/F0345");
    check_true(hashAfter != hashBefore,
               "attempt counter changes the audit hash even though candidate state is stable",
               "PANEL.C F0344/F0345");

    check_true(DM1_V1_MirrorCandidateScrollPickupWithPartyRotate_CompletePartyRotationPc34Compat(&state),
               "party rotation completes", "COMMAND.C F0380:2150-2156");
    check_int_eq(state.partyDirection, 1, "party turns east after C002",
                 "COMMAND.C F0380:2150-2156");
    check_int_eq(state.commandQueueLocked, 0, "queue unlocks after dispatch",
                 "COMMAND.C F0380:2126-2127");
    check_int_eq(state.f0380DispatchInProgress, 0,
                 "F0380 dispatch exits before pickup is honored",
                 "COMMAND.C F0380:2045-2156");
    check_int_eq(state.partyRotationInProgress, 0,
                 "party rotation flag clears before pickup is honored",
                 "COMMAND.C F0380:2150-2156");
    check_int_eq(state.rotationCompletedBeforePickup, 1,
                 "rotation completion marker is set before pickup",
                 "COMMAND.C F0380:2150-2156");

    check_true(DM1_V1_MirrorCandidateScrollPickupWithPartyRotate_ClickScrollPickupPc34Compat(&state),
               "scroll pickup is honored after rotation completion",
               "CHAMPION.C F0302:662-713");
    check_int_eq(state.honoredPickupAfterRotationCount, 1,
                 "honored pickup counted once", "CHAMPION.C F0302:662-713");
    check_int_eq(state.f0359PanelClickCount, 2,
                 "second panel click is the honored one",
                 "COMMAND.C F0359:1985-1990");
    check_int_eq(state.f0302SlotClickCount, 1,
                 "F0302 runs once after rotation", "CHAMPION.C F0302:662-713");
    check_int_eq(state.f0297PutCount, 1,
                 "F0297 runs once after rotation", "CHAMPION.C F0297:243-268");
    check_uint_eq(state.leaderHandThing, DM1_V1_MCSPPR_SCROLL_THING_PC34,
                  "leader hand receives scroll after rotation",
                  "CHAMPION.C F0297:243-268");
    check_int_eq(state.leaderHandEmpty, 0,
                 "leader hand is no longer empty after pickup",
                 "CHAMPION.C F0297:243-268");
    check_uint_eq(state.chestSlots[2], DM1_V1_MCSPPR_THING_NONE_PC34,
                  "scroll chest slot clears after honored pickup",
                  "CHAMPION.C F0302:662-713");
    check_int_eq(state.f0282DecisionCount, 0,
                 "F0282 never clears the candidate in this pickup race",
                 "REVIVE.C F0282:744-806");
    check_int_eq(state.c040PanelOpen, 1,
                 "C040 candidate panel remains live after pickup",
                 "DEFS.H:277 C040");
}

static void test_run_result_and_hash(void)
{
    Dm1V1MirrorCandidateScrollPickupWithPartyRotateStatePc34Compat stateA;
    Dm1V1MirrorCandidateScrollPickupWithPartyRotateStatePc34Compat stateB;
    Dm1V1MirrorCandidateScrollPickupWithPartyRotateResultPc34Compat resultA;
    Dm1V1MirrorCandidateScrollPickupWithPartyRotateResultPc34Compat resultB;

    check_true(DM1_V1_MirrorCandidateScrollPickupWithPartyRotate_RunPc34Compat(&stateA, &resultA),
               "run helper returns success", "pass764");
    check_true(DM1_V1_MirrorCandidateScrollPickupWithPartyRotate_RunPc34Compat(&stateB, &resultB),
               "second run helper returns success", "pass764");
    check_int_eq(resultA.ok, 1, "result ok flag", "pass764");
    check_int_eq(resultA.initialized, 1, "result initialized",
                 "contract_only=1");
    check_int_eq(resultA.openedCandidate, 1, "result opened candidate",
                 "REVIVE.C F0280:124-132");
    check_int_eq(resultA.queuedPartyTurn, 1, "result queued turn",
                 "COMMAND.C F0361:1709-1813");
    check_int_eq(resultA.beganF0380PartyRotation, 1,
                 "result began F0380 party rotation",
                 "COMMAND.C F0380:2045-2156");
    check_int_eq(resultA.ignoredPickupDuringPartyRotation, 1,
                 "result ignored pickup during rotation",
                 "PANEL.C F0344/F0345");
    check_int_eq(resultA.candidateIndexPreservedDuringIgnore, 1,
                 "result candidate index preserved", "REVIVE.C F0280/F0282");
    check_int_eq(resultA.candidateChainPreservedDuringIgnore, 1,
                 "result candidate chain preserved", "REVIVE.C F0280/F0282");
    check_int_eq(resultA.c040RedrawPreservedDuringIgnore, 1,
                 "result C040 redraw preserved",
                 "CHAMDRAW.C F0291/F0292/F0296");
    check_int_eq(resultA.candidateNotInternallyRotated, 1,
                 "result excludes candidate-internal rotation",
                 "PANEL.C F0344/F0345");
    check_int_eq(resultA.noLeaderHandPickupDuringRotation, 1,
                 "result no leader-hand pickup during rotation",
                 "CHAMPION.C F0297:243-268");
    check_int_eq(resultA.noSlotMutationDuringRotation, 1,
                 "result no slot mutation during rotation",
                 "CHAMPION.C F0302:662-713");
    check_int_eq(resultA.rotationCompleted, 1,
                 "result rotation completed", "COMMAND.C F0380:2150-2156");
    check_int_eq(resultA.rotationCompletedBeforePickup, 1,
                 "result rotation completed before pickup",
                 "COMMAND.C F0380:2150-2156");
    check_int_eq(resultA.pickupHonoredAfterRotation, 1,
                 "result pickup honored after rotation",
                 "CHAMPION.C F0302:662-713");
    check_int_eq(resultA.leaderHandReceivedScroll, 1,
                 "result leader hand received scroll",
                 "CHAMPION.C F0297:243-268");
    check_int_eq(resultA.scrollSlotClearedAfterPickup, 1,
                 "result scroll slot cleared", "CHAMPION.C F0302:662-713");
    check_int_eq(resultA.candidateStillLiveAfterPickup, 1,
                 "result candidate still live after pickup",
                 "REVIVE.C F0280/F0282");
    check_int_eq(resultA.f0302OnlyAfterRotation, 1,
                 "result F0302 only after rotation",
                 "CHAMPION.C F0302:662-713");
    check_int_eq(resultA.f0297OnlyAfterRotation, 1,
                 "result F0297 only after rotation",
                 "CHAMPION.C F0297:243-268");
    check_int_eq(resultA.f0282NeverFired, 1,
                 "result F0282 never fires", "REVIVE.C F0282:744-806");
    check_int_eq(resultA.contractOnly, 1, "result contract-only flag",
                 "contract_only=1");
    check_int_eq(resultA.noAssetsOrPixelParity, 1,
                 "result makes no asset or pixel-parity claim", "pass764");
    check_uint_eq(resultA.finalHash, resultB.finalHash,
                  "final hash is deterministic across runs", "pass764");
    check_uint_eq(resultA.ignoredSnapshotHash, resultB.ignoredSnapshotHash,
                  "ignored snapshot hash is deterministic across runs",
                  "pass764");
    check_uint_eq(resultA.finalHash, 0xE64C6945u,
                  "final hash matches locked pass764 value", "pass764");
    check_uint_eq(resultA.ignoredSnapshotHash, 0xF8A7B3B3u,
                  "ignored snapshot hash matches locked pass764 value",
                  "pass764");
    check_true(resultA.finalHash != 0u, "final hash is non-zero", "pass764");
    check_true(resultA.ignoredSnapshotHash != 0u,
               "ignored snapshot hash is non-zero", "pass764");
    printf("pass764 deterministic hash: 0x%08X ignored=0x%08X assertions=%d\n",
           resultA.finalHash, resultA.ignoredSnapshotHash, gAssertions);
}

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    test_evidence();
    test_initial_and_manual_sequence();
    test_run_result_and_hash();

    if (gFailures) {
        printf("FAIL: %d/%d assertions failed\n", gFailures, gAssertions);
        return 1;
    }
    printf("PASS: %d assertions\n", gAssertions);
    return 0;
}
