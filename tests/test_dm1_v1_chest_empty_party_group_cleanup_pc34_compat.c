#include "dm1/dm1_v1_chest_empty_party_group_cleanup_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_assertions;
static DM1_V1_ChestEmptyPartyGroupCleanupProbePc34 g_probe;

static int expect_int(const char* label, int got, int want,
                      const char* redmcsbAnchor)
{
    ++g_assertions;
    if (!redmcsbAnchor || redmcsbAnchor[0] == '\0') {
        printf("FAIL %s missing ReDMCSB anchor\n", label);
        return 0;
    }
    if (got != want) {
        printf("FAIL %s got=%d want=%d anchor=%s\n", label, got, want,
               redmcsbAnchor);
        return 0;
    }
    printf("ok %s=%d anchor=%s\n", label, got, redmcsbAnchor);
    return 1;
}

static int expect_contains(const char* label, const char* got,
                           const char* want, const char* redmcsbAnchor)
{
    ++g_assertions;
    if (!redmcsbAnchor || redmcsbAnchor[0] == '\0') {
        printf("FAIL %s missing ReDMCSB anchor\n", label);
        return 0;
    }
    if (!got || !want || !strstr(got, want)) {
        printf("FAIL %s missing '%s' anchor=%s\n", label,
               want ? want : "(null)", redmcsbAnchor);
        return 0;
    }
    printf("ok %s contains=%s anchor=%s\n", label, want, redmcsbAnchor);
    return 1;
}

static int test_spec_and_evidence(void)
{
    const DM1_V1_ChestEmptyPartyGroupCleanupSpecPc34* spec =
        dm1_v1_chest_empty_party_group_cleanup_spec_pc34();
    const char* evidence =
        dm1_v1_chest_empty_party_group_cleanup_source_evidence_pc34();
    const char* f0333 = "ReDMCSB CHEST.C F0333 lines 53-76";
    const char* f0334 = "ReDMCSB CHEST.C F0334 lines 113-132";
    const char* f0163 = "ReDMCSB DUNGEON.C F0163 lines 1796-1837";
    const char* f0164 = "ReDMCSB DUNGEON.C F0164 lines 1879-1918";
    const char* f0172 =
        "ReDMCSB DUNGEON.C F0172 lines 2517-2523,2670-2721";
    const char* f0302 = "ReDMCSB CHAMPION.C F0302 lines 688-710";
    int ok = 1;

    ok &= expect_int("spec present", spec != 0, 1, f0334);
    ok &= expect_int("contract only", spec->contractOnly, 1, f0334);
    ok &= expect_int("slot count", spec->slotCount,
                     DM1_PC34_CHEST_SLOT_COUNT, f0333);
    ok &= expect_int("thing end sentinel", spec->thingEndOfList,
                     DM1_PC34_CHEST_EMPTY_PARTY_GROUP_THING_END, f0163);
    ok &= expect_int("thing none sentinel", spec->thingNone,
                     DM1_PC34_CHEST_EMPTY_PARTY_GROUP_THING_NONE, f0334);
    ok &= expect_int("not on square sentinel", spec->notOnSquare,
                     DM1_PC34_CHEST_EMPTY_PARTY_GROUP_NOT_ON_SQUARE, f0163);
    ok &= expect_int("chest thing id", spec->chestThing,
                     DM1_PC34_CHEST_EMPTY_PARTY_GROUP_CHEST_THING, f0333);
    ok &= expect_int("sensor thing id", spec->sensorThing,
                     DM1_PC34_CHEST_EMPTY_PARTY_GROUP_SENSOR_THING, f0172);
    ok &= expect_int("floor thing id", spec->floorThing,
                     DM1_PC34_CHEST_EMPTY_PARTY_GROUP_FLOOR_THING, f0172);
    ok &= expect_contains("spec f0333 anchor", spec->f0333Anchor,
                          "F0333_INVENTORY_OpenAndDrawChest", f0333);
    ok &= expect_contains("spec f0334 anchor", spec->f0334Anchor,
                          "F0334_INVENTORY_CloseChest", f0334);
    ok &= expect_contains("spec f0163 anchor", spec->f0163Anchor,
                          "F0163_DUNGEON_LinkThingToList", f0163);
    ok &= expect_contains("spec f0164 anchor", spec->f0164Anchor,
                          "F0164_DUNGEON_UnlinkThingFromList", f0164);
    ok &= expect_contains("spec f0172 anchor", spec->f0172Anchor,
                          "F0172_DUNGEON_SetSquareAspect", f0172);
    ok &= expect_contains("spec f0302 anchor", spec->f0302Anchor,
                          "F0302_CHAMPION_ProcessCommands28To65", f0302);
    ok &= expect_contains("contract scope", spec->scope,
                          "contract_only=1", f0334);
    ok &= expect_contains("contract avoids real assets", spec->scope,
                          "without real-asset", f0334);
    ok &= expect_contains("evidence f0333", evidence, "F0333:53-76", f0333);
    ok &= expect_contains("evidence f0334", evidence, "F0334:113-132", f0334);
    ok &= expect_contains("evidence f0163", evidence, "F0163:1796-1837", f0163);
    ok &= expect_contains("evidence f0164", evidence, "F0164:1879-1918", f0164);
    ok &= expect_contains("evidence f0172", evidence, "F0172:2517-2523",
                          f0172);
    ok &= expect_contains("evidence f0302", evidence, "F0302:688-710", f0302);
    return ok;
}

static int test_empty_close_cleanup(void)
{
    const DM1_V1_ChestEmptyPartyGroupCleanupEmptyClosePc34* e =
        &g_probe.emptyClose;
    const char* f0333 = "ReDMCSB CHEST.C F0333 lines 53-76";
    const char* f0334 = "ReDMCSB CHEST.C F0334 lines 113-132";
    const char* f0164 = "ReDMCSB DUNGEON.C F0164 lines 1879-1918";
    const char* f0172 =
        "ReDMCSB DUNGEON.C F0172 lines 2517-2523,2670-2721";
    int ok = 1;

    ok &= expect_int("no-open close count", e->noOpenCloseCount, 0, f0334);
    ok &= expect_int("empty chest opens", e->openResult, 1, f0333);
    ok &= expect_int("open thing after empty open", e->openThingAfterOpen,
                     DM1_PC34_CHEST_EMPTY_PARTY_GROUP_CHEST_THING, f0333);
    ok &= expect_int("empty visible slot C537", e->emptyVisibleSlot0, 0,
                     f0333);
    ok &= expect_int("empty visible slot C544", e->emptyVisibleSlot7, 0,
                     f0333);
    ok &= expect_int("square present before cleanup",
                     e->emptyCloseSquarePresentBefore, 1, f0172);
    ok &= expect_int("square first thing before cleanup",
                     e->emptyCloseFirstThingBefore,
                     DM1_PC34_CHEST_EMPTY_PARTY_GROUP_CHEST_THING, f0172);
    ok &= expect_int("square first-count before cleanup",
                     e->emptyCloseSquareFirstThingCountBefore, 1, f0164);
    ok &= expect_int("column 1 before cleanup", e->emptyCloseColumn1Before,
                     1, f0164);
    ok &= expect_int("column 2 before cleanup", e->emptyCloseColumn2Before,
                     1, f0164);
    ok &= expect_int("empty close count", e->closeCount, 0, f0334);
    ok &= expect_int("open thing cleared by close", e->openThingAfterClose,
                     0, f0334);
    ok &= expect_int("closed slot hidden after close", e->getClosedSlotResult,
                     0, f0334);
    ok &= expect_int("empty close cleanup applied",
                     e->emptyCloseCleanupApplied, 1, f0164);
    ok &= expect_int("square present after cleanup",
                     e->emptyCloseSquarePresentAfter, 0, f0164);
    ok &= expect_int("square first after cleanup",
                     e->emptyCloseFirstThingAfter,
                     DM1_PC34_CHEST_EMPTY_PARTY_GROUP_THING_END, f0172);
    ok &= expect_int("square first-count after cleanup",
                     e->emptyCloseSquareFirstThingCountAfter, 0, f0164);
    ok &= expect_int("column 1 after cleanup", e->emptyCloseColumn1After,
                     0, f0164);
    ok &= expect_int("column 2 after cleanup", e->emptyCloseColumn2After,
                     0, f0164);
    ok &= expect_int("removed next after cleanup",
                     e->emptyCloseRemovedNextAfterUnlink,
                     DM1_PC34_CHEST_EMPTY_PARTY_GROUP_THING_END, f0164);
    ok &= expect_int("cell bits cleared on cleanup",
                     e->emptyCloseCellBitsCleared, 1, f0164);
    ok &= expect_int("second close still no-op", e->closeAgainCount, 0,
                     f0334);
    return ok;
}

static int test_square_splice_cases(void)
{
    const DM1_V1_ChestEmptyPartyGroupCleanupSquareCasesPc34* s =
        &g_probe.squareCases;
    const char* f0164 = "ReDMCSB DUNGEON.C F0164 lines 1879-1918";
    const char* f0172 =
        "ReDMCSB DUNGEON.C F0172 lines 2517-2523,2670-2721";
    int ok = 1;

    ok &= expect_int("head case applies", s->headCleanupApplied, 1, f0164);
    ok &= expect_int("head present before", s->headPresentBefore, 1, f0172);
    ok &= expect_int("head present after", s->headPresentAfter, 1, f0164);
    ok &= expect_int("head first before", s->headFirstBefore,
                     DM1_PC34_CHEST_EMPTY_PARTY_GROUP_CHEST_THING, f0172);
    ok &= expect_int("head first after", s->headFirstAfter,
                     DM1_PC34_CHEST_EMPTY_PARTY_GROUP_SENSOR_THING, f0164);
    ok &= expect_int("head count before", s->headCountBefore, 1, f0164);
    ok &= expect_int("head count after", s->headCountAfter, 1, f0164);
    ok &= expect_int("head chest next before", s->headChestNextBefore,
                     DM1_PC34_CHEST_EMPTY_PARTY_GROUP_SENSOR_THING, f0164);
    ok &= expect_int("head chest next after", s->headChestNextAfter,
                     DM1_PC34_CHEST_EMPTY_PARTY_GROUP_THING_END, f0164);
    ok &= expect_int("head sensor next after", s->headSensorNextAfter,
                     DM1_PC34_CHEST_EMPTY_PARTY_GROUP_THING_END, f0164);

    ok &= expect_int("tail case applies", s->tailCleanupApplied, 1, f0164);
    ok &= expect_int("tail present before", s->tailPresentBefore, 1, f0172);
    ok &= expect_int("tail present after", s->tailPresentAfter, 1, f0164);
    ok &= expect_int("tail first before", s->tailFirstBefore,
                     DM1_PC34_CHEST_EMPTY_PARTY_GROUP_SENSOR_THING, f0172);
    ok &= expect_int("tail first after", s->tailFirstAfter,
                     DM1_PC34_CHEST_EMPTY_PARTY_GROUP_SENSOR_THING, f0164);
    ok &= expect_int("tail count before", s->tailCountBefore, 1, f0164);
    ok &= expect_int("tail count after", s->tailCountAfter, 1, f0164);
    ok &= expect_int("tail sensor next before", s->tailSensorNextBefore,
                     DM1_PC34_CHEST_EMPTY_PARTY_GROUP_CHEST_THING, f0164);
    ok &= expect_int("tail sensor next after", s->tailSensorNextAfter,
                     DM1_PC34_CHEST_EMPTY_PARTY_GROUP_THING_END, f0164);
    ok &= expect_int("tail chest next after", s->tailChestNextAfter,
                     DM1_PC34_CHEST_EMPTY_PARTY_GROUP_THING_END, f0164);
    ok &= expect_int("missing thing cleanup no-op", s->missingCleanupResult,
                     0, f0164);
    ok &= expect_int("end-of-list cleanup no-op", s->endOfListCleanupResult,
                     0, f0164);
    return ok;
}

static int test_open_close_cycle_edges(void)
{
    const DM1_V1_ChestEmptyPartyGroupCleanupOpenClosePc34* c =
        &g_probe.openClose;
    const char* f0333 = "ReDMCSB CHEST.C F0333 lines 53-76";
    const char* f0334 = "ReDMCSB CHEST.C F0334 lines 113-132";
    const char* f0163 = "ReDMCSB DUNGEON.C F0163 lines 1796-1837";
    const char* f0164 = "ReDMCSB DUNGEON.C F0164 lines 1879-1918";
    const char* f0172 =
        "ReDMCSB DUNGEON.C F0172 lines 2517-2523,2670-2721";
    int ok = 1;

    ok &= expect_int("nonempty open succeeds", c->openResult, 1, f0333);
    ok &= expect_int("nonempty close count", c->closeCount, 2, f0334);
    ok &= expect_int("nonempty cleanup skipped", c->cleanupAttempted, 0,
                     f0164);
    ok &= expect_int("nonempty square still present",
                     c->squarePresentAfterClose, 1, f0172);
    ok &= expect_int("nonempty first thing still chest",
                     c->firstThingAfterClose,
                     DM1_PC34_CHEST_EMPTY_PARTY_GROUP_CHEST_THING, f0172);
    ok &= expect_int("closed type 0", c->closedTypes[0],
                     DM1_PC34_CHEST_EMPTY_PARTY_GROUP_ITEM0, f0334);
    ok &= expect_int("closed weight 0", c->closedWeights[0], 4, f0334);
    ok &= expect_int("closed type 1", c->closedTypes[1],
                     DM1_PC34_CHEST_EMPTY_PARTY_GROUP_ITEM1, f0334);
    ok &= expect_int("closed weight 1", c->closedWeights[1], 9, f0334);
    ok &= expect_int("closed type 2 empty", c->closedTypes[2], 0, f0334);
    ok &= expect_int("close clears open thing", c->closeClearsOpenThing, 1,
                     f0334);
    ok &= expect_int("relink before empty reopen", c->reopenEmptyResult, 1,
                     f0163);
    ok &= expect_int("reopen empty close count", c->reopenEmptyCloseCount, 0,
                     f0334);
    ok &= expect_int("reopen empty cleanup applied",
                     c->reopenEmptyCleanupApplied, 1, f0164);
    ok &= expect_int("reopen empty square absent",
                     c->reopenEmptySquarePresentAfter, 0, f0164);
    ok &= expect_int("reopen empty first thing end",
                     c->reopenEmptyFirstThingAfter,
                     DM1_PC34_CHEST_EMPTY_PARTY_GROUP_THING_END, f0172);
    return ok;
}

int main(void)
{
    const char* f0334 = "ReDMCSB CHEST.C F0334 lines 113-132";
    int ok = 1;

    printf("probe=dm1_v1_chest_empty_party_group_cleanup_pc34_compat\n");
    printf("sourceEvidence=%s\n",
           dm1_v1_chest_empty_party_group_cleanup_source_evidence_pc34());

    ok &= expect_int("probe run",
                     dm1_v1_chest_empty_party_group_cleanup_pc34(&g_probe),
                     1, f0334);
    ok &= expect_int("probe contract only", g_probe.contractOnly, 1, f0334);
    ok &= test_spec_and_evidence();
    ok &= test_empty_close_cleanup();
    ok &= test_square_splice_cases();
    ok &= test_open_close_cycle_edges();

    printf("assertions=%d\n", g_assertions);
    printf("dm1V1ChestEmptyPartyGroupCleanupOk=%d\n", ok ? 1 : 0);
    return ok ? 0 : 1;
}
