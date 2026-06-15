#include "firestaff/dm1/v1/chest/dm1_v1_chest_pickup_during_resurrect_pending_non_leader_pc34_compat.h"

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

static void check_contains(Counters* c,
                           const char* text,
                           const char* needle,
                           const char* label)
{
    int ok = text && needle && strstr(text, needle) != 0;

    ++c->assertions;
    if (!ok) {
        ++c->failures;
        printf("FAIL %s missing=%s\n", label, needle ? needle : "(null)");
    }
}

static void check_item(Counters* c,
                       DM1_V1_ChestPickupDuringResurrectPendingNonLeaderItemPc34 item,
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

static void check_source(Counters* c)
{
    const char* evidence =
        dm1_v1_chest_pickup_during_resurrect_pending_non_leader_source_evidence_pc34();
    const DM1_V1_ChestPickupDuringResurrectPendingNonLeaderSpecPc34* spec =
        dm1_v1_chest_pickup_during_resurrect_pending_non_leader_spec_pc34();

    check_contains(c, evidence, "CHEST.C F0333:30-67", "F0333 evidence");
    check_contains(c, evidence, "CHEST.C F0334:113-132", "F0334 evidence");
    check_contains(c, evidence, "CHAMPION.C F0297:243-298", "F0297 evidence");
    check_contains(c, evidence, "F0298:270-298", "F0298 evidence");
    check_contains(c, evidence, "CHAMPION.C F0300:511-515", "F0300 evidence");
    check_contains(c, evidence, "CHAMPION.C F0301:606-614", "F0301 evidence");
    check_contains(c, evidence, "CHAMPION.C F0302:662-714", "F0302 evidence");
    check_contains(c, evidence, "CHAMPION.C F0284:93-131", "F0284 evidence");
    check_contains(c, evidence, "REVIVE.C F0280:124-132", "F0280 evidence");
    check_contains(c, evidence, "REVIVE.C F0282:744-806", "F0282 evidence");
    check_contains(c, evidence, "PANEL.C F0344:113-145", "F0344 evidence");
    check_contains(c, evidence, "F0345:155-200", "F0345 evidence");
    check_contains(c, evidence, "F0352:2111-2160", "F0352 evidence");
    check_contains(c, evidence, "COMMAND.C F0359:1985-1990", "F0359 evidence");
    check_contains(c, evidence, "COMMAND.C F0378:1973-1983", "F0378 evidence");
    check_contains(c, evidence, "DEFS.H:2088", "DEFS C10 evidence");
    check_contains(c, evidence, "3906-3913 C537..C544", "DEFS C537 evidence");
    check_contains(c, evidence, "C30/G0425/G0426/G0423/G0305/M070/M516/C040",
                   "DEFS globals evidence");
    check_contains(c, spec->disjointnessNote,
                   "chest_c545_non_leader_hand_to_mid_cast_leader",
                   "disjoint c545");
    check_contains(c, spec->disjointnessNote,
                   "chest_scroll_wheel_resurrect_confirmation",
                   "disjoint scroll wheel");
    check_contains(c, spec->disjointnessNote,
                   "mirror_candidate_resurrect",
                   "disjoint mirror resurrect");
    check_contains(c, spec->disjointnessNote,
                   "mirror_candidate_chest_open_during_pending",
                   "disjoint chest open pending");
    check_contains(c, spec->disjointnessNote,
                   "chest_close_while_party_rotate_pickup_pending",
                   "disjoint rotate close");
}

static void check_probe(Counters* c)
{
    DM1_V1_ChestPickupDuringResurrectPendingNonLeaderProbePc34 p;
    int ok;
    int i;

    memset(&p, 0, sizeof(p));
    ok = dm1_v1_chest_pickup_during_resurrect_pending_non_leader_run_pc34(&p);

    check_int(c, ok, 1, "run ok");
    check_int(c, p.modelFailures, 0, "model failures");
    check_true(c, p.modelAssertions >= 20, "model assertion floor");
    check_int(c, p.sourceLockedContractOnly, 1, "contract only");
    check_int(c, p.assetFree, 1, "asset free");
    check_int(c, p.stepCount, 6, "step count");
    check_int(c, p.stepTrace[0], 1, "step open");
    check_int(c, p.stepTrace[1], 2, "step c040");
    check_int(c, p.stepTrace[2], 3, "step queue");
    check_int(c, p.stepTrace[3], 4, "step close");
    check_int(c, p.stepTrace[4], 5, "step commit");
    check_int(c, p.stepTrace[5], 6, "step resolve");

    check_int(c, p.leaderBefore, DM1_PC34_CPRPNL_LEADER_BEFORE, "leader before");
    check_int(c, p.nonLeaderOwner, DM1_PC34_CPRPNL_NON_LEADER_OWNER, "owner");
    check_int(c, p.newLeaderAfterResurrect,
              DM1_PC34_CPRPNL_NEW_LEADER_AFTER_RESURRECT, "new leader");
    check_int(c, p.partyChampionCountBefore, DM1_PC34_CPRPNL_CHAMPION_COUNT,
              "party count before");
    check_int(c, p.partyChampionCountAfterCommit, DM1_PC34_CPRPNL_CHAMPION_COUNT,
              "party count after");
    check_int(c, p.partyDirectionBefore, 0, "direction before");
    check_int(c, p.partyDirectionAfterCommit, 1, "direction after");

    check_int(c, p.openResult, 1, "open result");
    check_int(c, p.openChestThingBeforePending, DM1_PC34_CPRPNL_CHEST_THING,
              "open thing");
    check_int(c, p.panelAfterOpen, DM1_PC34_PANEL_CHEST, "panel chest");
    check_int(c, p.c040PanelAfterPending, DM1_PC34_CPRPNL_M568_RESURRECT_PANEL,
              "panel c040");
    check_int(c, p.c040ChromeBeforeClose, DM1_PC34_CPRPNL_C040_GRAPHIC,
              "c040 before");
    check_int(c, p.c040ChromeAfterClose, DM1_PC34_CPRPNL_C040_GRAPHIC,
              "c040 after");
    check_int(c, p.c040ChromePreservedAcrossClose, 1, "c040 preserved");
    check_int(c, p.candidateOrdinalBeforeClose,
              DM1_PC34_CPRPNL_NEW_LEADER_AFTER_RESURRECT + 1,
              "candidate before");
    check_int(c, p.candidateOrdinalAfterClose,
              p.candidateOrdinalBeforeClose, "candidate after close");
    check_int(c, p.candidateOrdinalAfterCommit, 0, "candidate after commit");
    check_int(c, p.candidateSlotBeforeClose,
              DM1_PC34_CPRPNL_NEW_LEADER_AFTER_RESURRECT, "candidate slot before");
    check_int(c, p.candidateSlotAfterClose, p.candidateSlotBeforeClose,
              "candidate slot after");
    check_int(c, p.candidateSlotPreservedAcrossClose, 1,
              "candidate slot preserved");

    check_int(c, p.queuedCommand, DM1_PC34_CPRPNL_C30_SOURCE_SLOT,
              "queued command");
    check_int(c, p.queuedZone, DM1_PC34_CPRPNL_C537_ZONE, "queued zone");
    check_int(c, p.queuedSlotBox, DM1_PC34_CPRPNL_C537_SLOT_BOX,
              "queued slotbox");
    check_int(c, p.queuedPc34Slot, DM1_PC34_CPRPNL_C30_SOURCE_SLOT,
              "queued pc34");
    check_int(c, p.queuedOwner, DM1_PC34_CPRPNL_NON_LEADER_OWNER,
              "queued owner");
    check_int(c, p.queuedOpenChestThing, DM1_PC34_CPRPNL_CHEST_THING,
              "queued open thing");
    check_int(c, p.queuedBeforeClose, 1, "queued before close");
    check_int(c, p.queueReservedC537, 1, "reserved C537");
    check_int(c, p.queuePreservedAcrossClose, 1, "queue preserved");
    check_item(c, p.queuedItem, DM1_PC34_CPRPNL_FIRST_ITEM,
               DM1_PC34_CPRPNL_FIRST_WEIGHT,
               DM1_PC34_CPRPNL_FIRST_CHARGES, "queued item");

    check_int(c, p.closeCommand, DM1_PC34_CPRPNL_CLOSE_COMMAND_C045,
              "close command");
    check_int(c, p.closeButtonZone, DM1_PC34_CPRPNL_CLOSE_BUTTON_C503,
              "close button");
    check_int(c, p.pickupBlockedBeforeCommit, 1, "blocked before commit");
    check_int(c, p.closeCount, 5, "close count");
    check_int(c, p.openChestThingAfterClose, 0, "open thing after close");
    check_int(c, p.closeClearedG0426, 1, "G0426 clear");
    check_int(c, p.closeCompactedCleanly, 1, "compact clean");
    check_int(c, p.closedPickedCopies, 0, "closed picked copies");

    for (i = 0; i < 5; ++i) {
        check_item(c, p.closedChain[i],
                   DM1_PC34_CPRPNL_FIRST_ITEM + i + 1,
                   DM1_PC34_CPRPNL_FIRST_WEIGHT + i + 1,
                   DM1_PC34_CPRPNL_FIRST_CHARGES + i + 1,
                   "closed chain compacted");
    }
    for (i = 5; i < DM1_PC34_CPRPNL_SLOT_COUNT; ++i) {
        check_int(c, p.closedChain[i].type, 0, "closed chain tail type");
        check_int(c, p.closedChain[i].weight, 0, "closed chain tail weight");
        check_int(c, p.closedChain[i].charges, 0, "closed chain tail charges");
    }

    check_int(c, p.resurrectCommitResult, 1, "commit result");
    check_int(c, p.leaderAfterCommit,
              DM1_PC34_CPRPNL_NEW_LEADER_AFTER_RESURRECT, "leader after");
    check_int(c, p.f0282ClearedCandidate, 1, "F0282 clear");
    check_int(c, p.panelAfterCommit, DM1_PC34_PANEL_INVENTORY,
              "panel after commit");
    check_int(c, p.pickupResolveResult, 1, "pickup resolve");
    check_int(c, p.pickupResolvedAfterCommit, 1, "resolved after commit");
    check_int(c, p.pickupLandedInNewLeaderHand, 1, "landed new leader");
    check_int(c, p.pickupLandedInLeaderC30Chain, 1, "landed C30 chain");
    check_int(c, p.newLeaderHandType, p.queuedItem.type, "hand type");
    check_int(c, p.newLeaderHandWeight, p.queuedItem.weight, "hand weight");
    check_int(c, p.newLeaderHandCharges, p.queuedItem.charges, "hand charges");
    check_int(c, p.pickedCopiesIncludingHand, 1, "single picked copy");

    check_int(c, p.f0333OpenCount, 1, "F0333 count");
    check_int(c, p.f0334CloseCount, 1, "F0334 count");
    check_int(c, p.f0300ReserveCount, 1, "F0300 count");
    check_int(c, p.f0297PutCount, 1, "F0297 count");
    check_int(c, p.f0302DispatchCount, 1, "F0302 count");
    check_int(c, p.f0282CommitCount, 1, "F0282 count");
    check_int(c, p.f0359PanelDispatchCount, 1, "F0359 count");
    check_int(c, p.f0378ChestDispatchCount, 1, "F0378 count");
    check_true(c, p.deterministicHash != 0, "hash nonzero");

    printf("DM1_V1_CHEST_PICKUP_DURING_RESURRECT_PENDING_NON_LEADER_PC34_COMPAT_OK assertions=%d failures=%d deterministicHash=0x%08X\n",
           c->assertions + p.modelAssertions,
           c->failures + p.modelFailures,
           (unsigned int)p.deterministicHash);
}

int main(void)
{
    Counters c;

    memset(&c, 0, sizeof(c));
    check_source(&c);
    check_probe(&c);

    if (c.assertions < 80) {
        printf("FAIL assertion floor actual=%d expected>=80\n", c.assertions);
        return 1;
    }
    if (c.failures != 0) {
        printf("FAIL dm1_v1_chest_pickup_during_resurrect_pending_non_leader_pc34_compat assertions=%d failures=%d\n",
               c.assertions, c.failures);
        return 1;
    }
    return 0;
}
