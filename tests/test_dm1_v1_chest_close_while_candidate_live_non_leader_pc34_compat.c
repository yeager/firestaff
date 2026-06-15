#include "firestaff/dm1/v1/chest/dm1_v1_chest_close_while_candidate_live_non_leader_pc34_compat.h"

#include <stdio.h>
#include <string.h>

typedef struct {
    int assertions;
    int failures;
} Counters;

static void check_true(Counters* c, int condition, const char* label)
{
    ++c->assertions;
    if (!condition) {
        ++c->failures;
        printf("FAIL %s\n", label);
    }
}

static void check_int(Counters* c, int actual, int expected, const char* label)
{
    ++c->assertions;
    if (actual != expected) {
        ++c->failures;
        printf("FAIL %s actual=%d expected=%d\n", label, actual, expected);
    }
}

static void check_u32(Counters* c,
                      uint32_t actual,
                      uint32_t expected,
                      const char* label)
{
    ++c->assertions;
    if (actual != expected) {
        ++c->failures;
        printf("FAIL %s actual=0x%08X expected=0x%08X\n",
               label,
               (unsigned int)actual,
               (unsigned int)expected);
    }
}

static void check_contains(Counters* c,
                           const char* text,
                           const char* needle,
                           const char* label)
{
    ++c->assertions;
    if (!text || !needle || strstr(text, needle) == 0) {
        ++c->failures;
        printf("FAIL %s missing=%s\n", label, needle ? needle : "(null)");
    }
}

static void check_item(
    Counters* c,
    DM1_V1_ChestCloseWhileCandidateLiveNonLeaderItemPc34 item,
    int type,
    int weight,
    int charges,
    const char* label)
{
    check_int(c, item.type, type, label);
    check_int(c, item.weight, weight, label);
    check_int(c, item.charges, charges, label);
    check_int(c, item.allowedSlots, DM1_PC34_ALLOWED_CONTAINER, label);
}

static void check_empty_item(
    Counters* c,
    DM1_V1_ChestCloseWhileCandidateLiveNonLeaderItemPc34 item,
    const char* label)
{
    check_int(c, item.type, 0, label);
    check_int(c, item.weight, 0, label);
    check_int(c, item.charges, 0, label);
}

static void check_source(Counters* c)
{
    const char* evidence =
        dm1_v1_chest_close_while_candidate_live_non_leader_source_evidence_pc34();
    const DM1_V1_ChestCloseWhileCandidateLiveNonLeaderSpecPc34* spec =
        dm1_v1_chest_close_while_candidate_live_non_leader_spec_pc34();

    check_contains(c, evidence, "CHEST.C F0333:30-67", "F0333 evidence");
    check_contains(c, evidence, "CHEST.C F0334:113-132", "F0334 evidence");
    check_contains(c, evidence, "CHAMPION.C F0297:243-298", "F0297 evidence");
    check_contains(c, evidence, "F0298:270-298", "F0298 evidence");
    check_contains(c, evidence, "CHAMPION.C F0300:511-515", "F0300 evidence");
    check_contains(c, evidence, "CHAMPION.C F0301:606-614", "F0301 evidence");
    check_contains(c, evidence, "CHAMPION.C F0302:662-714", "F0302 evidence");
    check_contains(c, evidence, "REVIVE.C F0280:124-132", "F0280 evidence");
    check_contains(c, evidence, "REVIVE.C F0282:744-806", "F0282 evidence");
    check_contains(c, evidence, "COMMAND.C F0359:1985-1990", "F0359 evidence");
    check_contains(c, evidence, "C040/C537..C544/C030/G0425/G0426",
                   "DEFS evidence");
    check_contains(c, spec->contractMarker, "Contract-only", "contract marker");
    check_contains(c, spec->chestOpenAnchor, "F0333:30-67", "spec F0333");
    check_contains(c, spec->chestCloseAnchor, "F0334:113-132", "spec F0334");
    check_contains(c, spec->championHandAnchor, "F0297:243-298",
                   "spec F0297");
    check_contains(c, spec->championHandAnchor, "F0298:270-298",
                   "spec F0298");
    check_contains(c, spec->championSlotAnchor, "F0300:511-515",
                   "spec F0300");
    check_contains(c, spec->championSlotAnchor, "F0301:606-614",
                   "spec F0301");
    check_contains(c, spec->championSlotAnchor, "F0302:662-714",
                   "spec F0302");
    check_contains(c, spec->reviveOpenAnchor, "F0280:124-132", "spec F0280");
    check_contains(c, spec->revivePendingAnchor, "F0282:744-806",
                   "spec F0282");
    check_contains(c, spec->commandAnchor, "F0359:1985-1990", "spec F0359");
    check_contains(c, spec->defsAnchor, "C040", "spec C040");
    check_contains(c, spec->defsAnchor, "C537..C544", "spec C537");
    check_contains(c, spec->defsAnchor, "G0425", "spec G0425");
    check_contains(c, spec->defsAnchor, "G0426", "spec G0426");
    check_contains(c, spec->disjointnessNote, "pass710", "disjoint pass710");
    check_contains(c, spec->disjointnessNote, "pass711", "disjoint pass711");
    check_contains(c, spec->disjointnessNote, "pass728", "disjoint pass728");
    check_contains(c, spec->disjointnessNote, "pass731", "disjoint pass731");
    check_contains(c, spec->disjointnessNote, "pass732", "disjoint pass732");
    check_contains(c, spec->disjointnessNote, "pass735", "disjoint pass735");
    check_contains(c, spec->disjointnessNote, "pass736", "disjoint pass736");
    check_contains(c, spec->disjointnessNote,
                   "chest_pickup_during_resurrect_pending_non_leader",
                   "disjoint pickup pending non leader");
}

static void check_probe(Counters* c,
                        DM1_V1_ChestCloseWhileCandidateLiveNonLeaderProbePc34*
                            p)
{
    int ok;
    int i;
    static const int expectedClosed[] = {0, 1, 3, 4, 6, 7};

    memset(p, 0, sizeof(*p));
    ok = dm1_v1_chest_close_while_candidate_live_non_leader_run_pc34(p);

    check_int(c, ok, 1, "run ok");
    check_int(c, p->modelFailures, 0, "model failures");
    check_true(c, p->modelAssertions >= 28, "model assertion floor");
    check_int(c, p->sourceLockedContractOnly, 1, "contract only");
    check_int(c, p->assetFree, 1, "asset free");
    check_int(c, p->stepCount, 5, "step count");
    check_int(c, p->stepTrace[0], 1, "step open");
    check_int(c, p->stepTrace[1], 2, "step c040 candidate");
    check_int(c, p->stepTrace[2], 3, "step reject click");
    check_int(c, p->stepTrace[3], 4, "step close");
    check_int(c, p->stepTrace[4], 5, "step verify");

    check_int(c, p->leader, DM1_PC34_CCLNL_LEADER, "leader");
    check_int(c, p->nonLeaderOwner, DM1_PC34_CCLNL_NON_LEADER_OWNER,
              "non leader owner");
    check_int(c, p->candidateOwner, DM1_PC34_CCLNL_CANDIDATE_OWNER,
              "candidate owner");
    check_true(c, p->candidateOwner != p->nonLeaderOwner,
               "candidate owner differs from chest owner");
    check_int(c, p->partyChampionCount, DM1_PC34_CCLNL_CHAMPION_COUNT,
              "party count");

    check_int(c, p->openResult, 1, "open result");
    check_int(c, p->openChestOwnerBeforeClose,
              DM1_PC34_CCLNL_NON_LEADER_OWNER, "open owner before");
    check_int(c, p->openChestThingBeforeClose,
              DM1_PC34_CCLNL_OPEN_CHEST_THING, "open thing before");
    check_int(c, p->openChestThingAfterClose, 0, "open thing after");
    check_int(c, p->closeCommand, DM1_PC34_CCLNL_CLOSE_BUTTON_COMMAND,
              "close command");
    check_int(c, p->closeButtonZone, DM1_PC34_CCLNL_CLOSE_BUTTON_ZONE,
              "close button zone");
    check_int(c, p->closeCount, 6, "close count");
    check_int(c, p->closeClearedOnlyOwnerG0426, 1, "clear only G0426 owner");
    check_int(c, p->ownerClosedOnly, 1, "owner closed only");

    check_int(c, p->panelBeforeClose, DM1_PC34_CCLNL_M568_RESURRECT_PANEL,
              "panel before");
    check_int(c, p->panelAfterClose, DM1_PC34_CCLNL_M568_RESURRECT_PANEL,
              "panel after");
    check_int(c, p->c038ChromeBeforeClose, DM1_PC34_CCLNL_C038_PANEL_CHROME,
              "C038 before");
    check_int(c, p->c038ChromeAfterClose, DM1_PC34_CCLNL_C038_PANEL_CHROME,
              "C038 after");
    check_int(c, p->c039ChromeBeforeClose, DM1_PC34_CCLNL_C039_PANEL_CHROME,
              "C039 before");
    check_int(c, p->c039ChromeAfterClose, DM1_PC34_CCLNL_C039_PANEL_CHROME,
              "C039 after");
    check_int(c, p->c040ChromeBeforeClose, DM1_PC34_CCLNL_C040_PANEL_CHROME,
              "C040 before");
    check_int(c, p->c040ChromeAfterClose, DM1_PC34_CCLNL_C040_PANEL_CHROME,
              "C040 after");
    check_int(c, p->c040PanelRoutePreserved, 1, "C040 route preserved");
    check_int(c, p->c038C039C040ChromePreserved, 1,
              "C038/C039/C040 chrome preserved");

    check_int(c, p->candidateOrdinalBeforeClose,
              DM1_PC34_CCLNL_CANDIDATE_OWNER + 1, "candidate ordinal before");
    check_int(c, p->candidateOrdinalAfterClose, p->candidateOrdinalBeforeClose,
              "candidate ordinal after");
    check_int(c, p->candidateOwnerBeforeClose,
              DM1_PC34_CCLNL_CANDIDATE_OWNER, "candidate owner before");
    check_int(c, p->candidateOwnerAfterClose, p->candidateOwnerBeforeClose,
              "candidate owner after");
    check_int(c, p->candidateSlotBeforeClose,
              DM1_PC34_CCLNL_CANDIDATE_OWNER, "candidate slot before");
    check_int(c, p->candidateSlotAfterClose, p->candidateSlotBeforeClose,
              "candidate slot after");
    check_int(c, p->candidateLiveBeforeClose, 1, "candidate live before");
    check_int(c, p->candidateLiveAfterClose, 1, "candidate live after");
    check_int(c, p->candidatePreservedAcrossClose, 1, "candidate preserved");

    check_int(c, p->rejectedPanelClickCommand,
              DM1_PC34_CCLNL_C039_REJECTED_PANEL_CLICK,
              "rejected click command");
    check_int(c, p->rejectedPanelClickDuringClose, 1, "rejected during close");
    check_int(c, p->rejectedPanelClickWouldHaveOpenedViaF0333, 1,
              "would have opened via F0333");
    check_int(c, p->f0333OpenCountBeforeRejectedClick, 1,
              "F0333 before rejected");
    check_int(c, p->f0333OpenCountAfterRejectedClick, 1,
              "F0333 after rejected");
    check_int(c, p->f0333OpenCountAfterClose, 1, "F0333 after close");

    check_int(c, p->c540Zone, DM1_PC34_CCLNL_C540_ZONE, "C540 zone");
    check_int(c, p->c540SlotBox, DM1_PC34_CCLNL_C540_SLOT_BOX,
              "C540 slot box");
    check_int(c, p->c540Pc34Slot, DM1_PC34_SLOT_CHEST_4, "C540 pc34 slot");
    check_item(c, p->c540ItemBeforeClose, DM1_PC34_CCLNL_FIRST_ITEM + 3,
               DM1_PC34_CCLNL_FIRST_WEIGHT + 3,
               DM1_PC34_CCLNL_FIRST_CHARGES + 3,
               "C540 item before");
    check_item(c, p->c540ItemAfterClose, DM1_PC34_CCLNL_FIRST_ITEM + 3,
               DM1_PC34_CCLNL_FIRST_WEIGHT + 3,
               DM1_PC34_CCLNL_FIRST_CHARGES + 3,
               "C540 item after");
    check_int(c, p->c540PanelRoutePreserved, 1, "C540 route preserved");

    check_item(c, p->visibleBefore[0], DM1_PC34_CCLNL_FIRST_ITEM,
               DM1_PC34_CCLNL_FIRST_WEIGHT,
               DM1_PC34_CCLNL_FIRST_CHARGES, "visible before 0");
    check_item(c, p->visibleBefore[1], DM1_PC34_CCLNL_FIRST_ITEM + 1,
               DM1_PC34_CCLNL_FIRST_WEIGHT + 1,
               DM1_PC34_CCLNL_FIRST_CHARGES + 1, "visible before 1");
    check_empty_item(c, p->visibleBefore[2], "visible before hole 2");
    check_item(c, p->visibleBefore[3], DM1_PC34_CCLNL_FIRST_ITEM + 3,
               DM1_PC34_CCLNL_FIRST_WEIGHT + 3,
               DM1_PC34_CCLNL_FIRST_CHARGES + 3, "visible before 3");
    check_item(c, p->visibleBefore[4], DM1_PC34_CCLNL_FIRST_ITEM + 4,
               DM1_PC34_CCLNL_FIRST_WEIGHT + 4,
               DM1_PC34_CCLNL_FIRST_CHARGES + 4, "visible before 4");
    check_empty_item(c, p->visibleBefore[5], "visible before hole 5");
    check_item(c, p->visibleBefore[6], DM1_PC34_CCLNL_FIRST_ITEM + 6,
               DM1_PC34_CCLNL_FIRST_WEIGHT + 6,
               DM1_PC34_CCLNL_FIRST_CHARGES + 6, "visible before 6");
    check_item(c, p->visibleBefore[7], DM1_PC34_CCLNL_FIRST_ITEM + 7,
               DM1_PC34_CCLNL_FIRST_WEIGHT + 7,
               DM1_PC34_CCLNL_FIRST_CHARGES + 7, "visible before 7");

    for (i = 0; i < 6; ++i) {
        check_item(c, p->closedChain[i],
                   DM1_PC34_CCLNL_FIRST_ITEM + expectedClosed[i],
                   DM1_PC34_CCLNL_FIRST_WEIGHT + expectedClosed[i],
                   DM1_PC34_CCLNL_FIRST_CHARGES + expectedClosed[i],
                   "closed visible rewrite");
    }
    for (i = 6; i < DM1_PC34_CCLNL_SLOT_COUNT; ++i) {
        check_empty_item(c, p->closedChain[i], "closed tail empty");
    }
    check_item(c, p->hiddenTail[0], DM1_PC34_CCLNL_FIRST_ITEM + 8,
               DM1_PC34_CCLNL_FIRST_WEIGHT + 8,
               DM1_PC34_CCLNL_FIRST_CHARGES + 8, "hidden tail 0");
    check_item(c, p->hiddenTail[1], DM1_PC34_CCLNL_FIRST_ITEM + 9,
               DM1_PC34_CCLNL_FIRST_WEIGHT + 9,
               DM1_PC34_CCLNL_FIRST_CHARGES + 9, "hidden tail 1");
    check_int(c, p->closedVisibleThingCount, 6, "closed visible count");
    check_int(c, p->hiddenTailTruncated, 1, "hidden tail truncated");
    check_int(c, p->visibleSlotChainRewritten, 1, "visible chain rewritten");

    for (i = 0; i < DM1_PC34_CCLNL_CHAMPION_COUNT; ++i) {
        check_int(c, p->leaderHandBefore[i], p->leaderHandAfter[i],
                  "leader hand preserved");
        check_u32(c, p->c030ChainHashAfter[i], p->c030ChainHashBefore[i],
                  "C030 chain hash preserved");
    }
    check_int(c, p->leaderHandC030ChainsPreserved, 1,
              "leader hand C030 chains preserved");

    check_int(c, p->f0333OpenCount, 1, "F0333 count");
    check_int(c, p->f0334CloseCount, 1, "F0334 count");
    check_int(c, p->f0297PutLeaderHandCount, 0, "F0297 count");
    check_int(c, p->f0298RemoveLeaderHandCount, 0, "F0298 count");
    check_int(c, p->f0300RemoveC030Count, 0, "F0300 count");
    check_int(c, p->f0301AddC030Count, 0, "F0301 count");
    check_int(c, p->f0302SlotBoxCount, 0, "F0302 count");
    check_int(c, p->f0280CandidatePublishCount, 1, "F0280 count");
    check_int(c, p->f0282CandidateConsumeCount, 0, "F0282 count");
    check_int(c, p->f0359C040DispatchCount, 0, "F0359 count");
    check_true(c, p->deterministicHash != 0, "hash nonzero");
}

int main(void)
{
    Counters c;
    DM1_V1_ChestCloseWhileCandidateLiveNonLeaderProbePc34 p;

    memset(&c, 0, sizeof(c));
    check_source(&c);
    check_probe(&c, &p);

    if (c.assertions < 80) {
        printf("FAIL dm1_v1_chest_close_while_candidate_live_non_leader_pc34_compat assertions=%d failures=%d hash=0x%08X\n",
               c.assertions,
               c.failures + 1,
               (unsigned int)p.deterministicHash);
        return 1;
    }
    if (c.failures != 0) {
        printf("FAIL dm1_v1_chest_close_while_candidate_live_non_leader_pc34_compat assertions=%d failures=%d hash=0x%08X\n",
               c.assertions,
               c.failures,
               (unsigned int)p.deterministicHash);
        return 1;
    }

    printf("PASS dm1_v1_chest_close_while_candidate_live_non_leader_pc34_compat assertions=%d failures=0 modelAssertions=%d hash=0x%08X\n",
           c.assertions,
           p.modelAssertions,
           (unsigned int)p.deterministicHash);
    return 0;
}
