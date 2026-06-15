#include "firestaff/dm1/v1/mirror_candidate/resurrect_confirm_inventory_interrupt_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int gAssertions;
static int gFailures;

#define CHECK_TRUE(cond, msg, anchor) do { \
    ++gAssertions; \
    if (!(cond)) { \
        ++gFailures; \
        printf("FAIL: %s [%s]\n", (msg), (anchor)); \
    } \
} while (0)

static void check_source_lock_metadata(void)
{
    const Dm1V1MirrorRciiEvidencePc34Compat *e =
        dm1_v1_mirror_candidate_rcii_evidence_pc34_compat();
    const char *source =
        dm1_v1_mirror_candidate_rcii_source_evidence_pc34_compat();

    CHECK_TRUE(e != NULL && e->contractOnly == 1,
               "evidence metadata is available",
               "metadata");
    CHECK_TRUE(strstr(e->reviveOpenAnchor, "REVIVE.C F0280:124-132") != NULL,
               "REVIVE.C F0280 anchor is cited",
               e->reviveOpenAnchor);
    CHECK_TRUE(strstr(e->reviveFinishAnchor, "REVIVE.C F0282:744-806") != NULL,
               "REVIVE.C F0282 anchor is cited",
               e->reviveFinishAnchor);
    CHECK_TRUE(strstr(e->chestAnchor, "CHEST.C F0333:30-67") != NULL &&
                   strstr(e->chestAnchor, "F0334:113-132") != NULL,
               "CHEST.C F0333/F0334 anchors are cited",
               e->chestAnchor);
    CHECK_TRUE(strstr(e->championHandAnchor, "F0297:243-268") != NULL &&
                   strstr(e->championSlotAnchor, "F0300:511-584") != NULL &&
                   strstr(e->championSlotAnchor, "F0301:606-614") != NULL &&
                   strstr(e->championSlotAnchor, "F0302:662-713") != NULL,
               "CHAMPION.C F0297/F0300/F0301/F0302 anchors are cited",
               "CHAMPION.C hand/slot anchors");
    CHECK_TRUE(strstr(e->commandAnchor, "F0359:1985-1990") != NULL &&
                   strstr(e->commandAnchor, "F0380:2045-2156") != NULL,
               "COMMAND.C F0359/F0380 anchors are cited",
               e->commandAnchor);
    CHECK_TRUE(strstr(e->panelAnchor, "F0346/F0347:1619-1657") != NULL,
               "PANEL.C F0346/F0347 anchor is cited",
               e->panelAnchor);
    CHECK_TRUE(strstr(e->utamscrAnchor, "UTAMSCR.C F0077:147-151") != NULL &&
                   strstr(e->utamscrAnchor, "F0078:141-145") != NULL,
               "UTAMSCR.C F0077/F0078 anchors are cited",
               e->utamscrAnchor);
    CHECK_TRUE(strstr(e->objectAnchor, "OBJECT.C F0033:147-212") != NULL &&
                   strstr(e->objectAnchor, "F0038:395-423") != NULL,
               "OBJECT.C F0033/F0038 anchors are cited",
               e->objectAnchor);
    CHECK_TRUE(strstr(e->scope, "pass738") != NULL &&
                   strstr(e->scope, "pass736") != NULL &&
                   strstr(e->scope, "confirm-boundary") != NULL,
               "scope marks the route as distinct from pass736",
               e->scope);
    CHECK_TRUE(strstr(source, "REVIVE.C F0282:744-806") != NULL &&
                   strstr(source, "COMMAND.C F0359:1985-1990") != NULL &&
                   strstr(source, "OBJECT.C F0033:147-212") != NULL,
               "source evidence string is populated",
               "source evidence");
}

static void check_manual_resurrect_confirm_interrupt(void)
{
    Dm1V1MirrorRciiStatePc34Compat state;
    Dm1V1MirrorRciiResultPc34Compat begin;
    Dm1V1MirrorRciiResultPc34Compat interrupt;
    Dm1V1MirrorRciiResultPc34Compat finish;

    dm1_v1_mirror_candidate_rcii_init_pc34_compat(&state);

    CHECK_TRUE(state.candidateChampionOrdinal == 2 &&
                   state.activePanelCandidateOrdinal == 2,
               "fixture starts with candidate ordinal 2",
               "REVIVE.C F0280:124-132");
    CHECK_TRUE(state.leaderHandThing == 0x7131 &&
                   state.leaderHandIcon == 0x31 &&
                   state.sourceC30Thing == 0x7038,
               "fixture starts with stable hand and C30 object metadata",
               "CHAMPION.C F0297:243-268; OBJECT.C F0033:147-212");
    CHECK_TRUE(state.panelRedrawOwner ==
                   DM1_V1_MIRROR_RCII_C040_PANEL_PC34_COMPAT,
               "C040 owns the initial panel redraw",
               "PANEL.C F0346/F0347:1619-1657");

    CHECK_TRUE(dm1_v1_mirror_candidate_rcii_begin_confirm_pc34_compat(
                   &state,
                   DM1_V1_MIRROR_RCII_FINISH_RESURRECT_PC34_COMPAT,
                   &begin) == 1,
               "resurrect confirm enters F0359/F0282 boundary",
               "COMMAND.C F0359:1985-1990");
    CHECK_TRUE(begin.pendingFinishAfter ==
                   DM1_V1_MIRROR_RCII_C160_RESURRECT_PC34_COMPAT &&
                   begin.candidateIdentityPreserved == 1,
               "pending C160 confirm keeps candidate identity",
               "REVIVE.C F0282:744-806");

    CHECK_TRUE(dm1_v1_mirror_candidate_rcii_inventory_interrupt_pc34_compat(
                   &state, &interrupt) == 1,
               "inventory/chest slot click interrupts pending confirm",
               "CHAMPION.C F0302:662-713");
    CHECK_TRUE(interrupt.queued == 1 &&
                   interrupt.queuedCommandAfter ==
                       DM1_V1_MIRROR_RCII_C038_SLOT_BOX_PC34_COMPAT,
               "interrupt is queued as C038 slot-box work",
               "COMMAND.C F0380:2045-2156");
    CHECK_TRUE(interrupt.candidateIdentityPreserved == 1 &&
                   state.f0282FinishCount == 0,
               "interrupt does not clear candidate before F0282",
               "REVIVE.C F0282:744-806");
    CHECK_TRUE(interrupt.handMetadataPreserved == 1 &&
                   interrupt.queuedMetadataPreserved == 1,
               "interrupt preserves leader-hand and queued object metadata",
               "CHAMPION.C F0297/F0300/F0301/F0302; OBJECT.C F0033:147-212");
    CHECK_TRUE(interrupt.panelRedrawOwnershipPreserved == 1 &&
                   state.panelRedrawSawThing == 0x7038,
               "C040 redraw owner observes the queued chest thing",
               "PANEL.C F0346/F0347:1619-1657; CHEST.C F0333:30-67");

    CHECK_TRUE(dm1_v1_mirror_candidate_rcii_finish_confirm_pc34_compat(
                   &state, &finish) == 1,
               "F0282 completes resurrect confirm after interrupt",
               "REVIVE.C F0282:744-806");
    CHECK_TRUE(finish.candidateAfter == DM1_V1_MIRROR_RCII_NONE_PC34_COMPAT &&
                   state.f0282FinishCount == 1,
               "candidate is cleared only by F0282",
               "REVIVE.C F0282:785");
    CHECK_TRUE(finish.dispatched == 1 &&
                   state.interruptDispatchedAfterFinishCount == 1,
               "queued inventory work dispatches after candidate finish",
               "COMMAND.C F0380:2045-2156");
    CHECK_TRUE(finish.handMetadataPreserved == 1 &&
                   state.leaderHandThing == 0x7131 &&
                   state.leaderHandIcon == 0x31,
               "leader-hand object metadata survives post-finish dispatch",
               "CHAMPION.C F0297:243-268; OBJECT.C F0033:147-212");
    CHECK_TRUE(state.f0077EnableCount == 1 &&
                   state.f0078DisableCount == 1 &&
                   state.f0038SlotDraws == 2,
               "redraw/pointer bracket and slot draw counts are deterministic",
               "UTAMSCR.C F0077/F0078; OBJECT.C F0038:395-423");
}

int main(void)
{
    int ok;
    int selfAssertions;
    int selfFailures;
    int hash;

    check_source_lock_metadata();
    check_manual_resurrect_confirm_interrupt();

    ok = dm1_v1_mirror_candidate_rcii_run_self_test_pc34_compat();
    selfAssertions = dm1_v1_mirror_candidate_rcii_assertions_pc34_compat();
    selfFailures = dm1_v1_mirror_candidate_rcii_failures_pc34_compat();
    hash = dm1_v1_mirror_candidate_rcii_hash_pc34_compat();

    CHECK_TRUE(ok == 1 && selfFailures == 0,
               "library self-test passes",
               "self-test");
    CHECK_TRUE(selfAssertions >= 30,
               "library self-test covers resurrect and reincarnate confirm",
               "REVIVE.C F0282:744-806");
    CHECK_TRUE(hash != 0,
               "deterministic hash is non-zero",
               "deterministic hash");

    if (gFailures != 0 || selfFailures != 0) {
        printf("FAIL dm1_v1_mirror_candidate_resurrect_confirm_inventory_interrupt_pc34_compat assertions=%d failures=%d self_assertions=%d self_failures=%d hash=0x%08X\n",
               gAssertions,
               gFailures,
               selfAssertions,
               selfFailures,
               (unsigned int)hash);
        return 1;
    }

    printf("PASS dm1_v1_mirror_candidate_resurrect_confirm_inventory_interrupt_pc34_compat assertions=%d failures=0 self_assertions=%d self_failures=0 hash=0x%08X\n",
           gAssertions,
           selfAssertions,
           (unsigned int)hash);
    return 0;
}
