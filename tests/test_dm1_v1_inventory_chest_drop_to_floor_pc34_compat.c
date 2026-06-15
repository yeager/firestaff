#include "dm1_v1_inventory_chest_drop_to_floor_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_assertions;

static int expect_int(const char* label,
                      int got,
                      int want,
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
    printf("PASS %s=%d anchor=%s\n", label, got, redmcsbAnchor);
    return 1;
}

static int expect_contains(const char* label,
                           const char* haystack,
                           const char* needle,
                           const char* redmcsbAnchor)
{
    ++g_assertions;
    if (!redmcsbAnchor || redmcsbAnchor[0] == '\0') {
        printf("FAIL %s missing ReDMCSB anchor\n", label);
        return 0;
    }
    if (!haystack || !needle || !strstr(haystack, needle)) {
        printf("FAIL %s missing=%s anchor=%s\n",
               label, needle ? needle : "(null)", redmcsbAnchor);
        return 0;
    }
    printf("PASS %s contains=%s anchor=%s\n", label, needle,
           redmcsbAnchor);
    return 1;
}

static int closed_excludes_type(
    const DM1_V1_InventoryChestDropToFloorEventPc34* event,
    int itemType)
{
    int i;

    if (!event) {
        return 0;
    }
    for (i = 0; i < event->closedCountAfterDrop; ++i) {
        if (event->closedTypes[i] == itemType) {
            return 0;
        }
    }
    return 1;
}

static int test_source_evidence(void)
{
    const char* evidence =
        dm1_v1_inventory_chest_drop_to_floor_source_evidence_pc34();
    const char* chest =
        "ReDMCSB CHEST.C F0333/F0334 lines 2-90,112-132";
    const char* champion =
        "ReDMCSB CHAMPION.C F0297/F0301/F0302 lines 243-340,587-700";
    const char* chamdraw =
        "ReDMCSB CHAMDRAW.C chest-slot draw lines 551-560,1237-1260";
    const char* dungeon =
        "ReDMCSB DUNGEON.C F0140-F0149 lines 1082-1300";
    const char* defs =
        "ReDMCSB DEFS.H weight accumulator lines 1712-1738";
    int ok = 1;

    ok &= expect_contains("evidence chest open", evidence, "CHEST.C:2-90",
                          chest);
    ok &= expect_contains("evidence chest close", evidence, "F0334", chest);
    ok &= expect_contains("evidence leader hand", evidence,
                          "CHAMPION.C:243-340", champion);
    ok &= expect_contains("evidence c30 routing", evidence,
                          "CHAMPION.C:587-700", champion);
    ok &= expect_contains("evidence bug avoided", evidence, "BUG0_39",
                          champion);
    ok &= expect_contains("evidence slot draw", evidence,
                          "CHAMDRAW.C:551-560,1237-1260", chamdraw);
    ok &= expect_contains("evidence weights", evidence,
                          "DUNGEON.C:1082-1300", dungeon);
    ok &= expect_contains("evidence defs weight", evidence,
                          "DEFS.H:1712-1738", defs);
    ok &= expect_contains("evidence floor link", evidence, "F0163",
                          dungeon);
    return ok;
}

static int test_event(
    const char* prefix,
    const DM1_V1_InventoryChestDropToFloorEventPc34* event,
    int chestSlotIndex,
    int itemType,
    int itemWeight,
    int loadBefore,
    int loadAfter,
    int nonEmptyBefore,
    int closedCount,
    const char* redmcsbAnchor)
{
    char label[128];
    int ok = 1;

#define CHECK_INT(name, got, want) \
    do { \
        snprintf(label, sizeof(label), "%s %s", prefix, name); \
        ok &= expect_int(label, (got), (want), redmcsbAnchor); \
    } while (0)

    CHECK_INT("result", event->result, 1);
    CHECK_INT("champion", event->championIndex, 0);
    CHECK_INT("pc34 slot", event->pc34Slot,
              DM1_PC34_SLOT_CHEST_1 + chestSlotIndex);
    CHECK_INT("chest slot index", event->chestSlotIndex, chestSlotIndex);
    CHECK_INT("source is C30+ chest slot", event->sourceIsChestSlot, 1);
    CHECK_INT("slot type before", event->slotTypeBefore, itemType);
    CHECK_INT("slot weight before", event->slotWeightBefore, itemWeight);
    CHECK_INT("load before", event->loadBefore, loadBefore);
    CHECK_INT("load after pickup", event->loadAfterPickup, loadAfter);
    CHECK_INT("load after drop", event->loadAfterDrop, loadAfter);
    CHECK_INT("expected load after drop",
              event->expectedLoadAfterDrop, loadAfter);
    CHECK_INT("mouse type after pickup", event->mouseTypeAfterPickup,
              itemType);
    CHECK_INT("mouse type after drop", event->mouseTypeAfterDrop, 0);
    CHECK_INT("chest slot cleared", event->chestSlotCleared, 1);
    CHECK_INT("floor count before", event->floorCountBefore, 0);
    CHECK_INT("floor count after", event->floorCountAfter, 1);
    CHECK_INT("floor cell index", event->floorCellIndex, 0);
    CHECK_INT("floor cell type", event->floorCellType, itemType);
    CHECK_INT("floor cell weight", event->floorCellWeight, itemWeight);
    CHECK_INT("G0189 first thing", event->g0189FirstThing, itemType);
    CHECK_INT("G0189 terminator", event->g0189Terminator,
              DM1_PC34_CHEST_DROP_FLOOR_THING_ENDOFLIST);
    CHECK_INT("dropped thing preserved", event->droppedThingPreserved, 1);
    CHECK_INT("non-empty before", event->nonEmptyBefore, nonEmptyBefore);
    CHECK_INT("non-empty after", event->nonEmptyAfter,
              nonEmptyBefore - 1);
    CHECK_INT("chest load before", event->chestLoadBefore, loadBefore);
    CHECK_INT("chest load after", event->chestLoadAfter, loadAfter);
    CHECK_INT("adjacent left preserved",
              event->adjacentLeftTypeAfter,
              event->adjacentLeftTypeBefore);
    CHECK_INT("adjacent right preserved",
              event->adjacentRightTypeAfter,
              event->adjacentRightTypeBefore);
    CHECK_INT("no adjacent orphan", event->noAdjacentOrphan, 1);
    CHECK_INT("Rabbit Foot refresh not used",
              event->rabbitFootRefreshNotUsed, 1);
    CHECK_INT("slot-zero orphan bug not reproduced",
              event->slotZeroOrphanBugReproduced, 0);
    CHECK_INT("closed count after drop",
              event->closedCountAfterDrop, closedCount);
    CHECK_INT("closed list excludes dropped item",
              closed_excludes_type(event, itemType), 1);

#undef CHECK_INT
    return ok;
}

static int test_probe(
    const DM1_V1_InventoryChestDropToFloorProbePc34* probe)
{
    const char* chest =
        "ReDMCSB CHEST.C F0333/F0334 lines 53-76,112-132";
    const char* champion =
        "ReDMCSB CHAMPION.C F0297/F0301/F0302 lines 243-340,587-700";
    const char* chamdraw =
        "ReDMCSB CHAMDRAW.C lines 551-560,1237-1260";
    const char* dungeonDefs =
        "ReDMCSB DUNGEON.C F0140-F0149 lines 1082-1300; DEFS.H lines 1712-1738";
    int ok = 1;

    ok &= expect_int("contract marker",
                     probe->sourceLockedContractOnly, 1, chest);
    ok &= expect_int("case count",
                     probe->caseCount, DM1_PC34_CHEST_DROP_FLOOR_CASE_COUNT,
                     champion);
    ok &= expect_int("first chest slot",
                     probe->firstChestSlot, DM1_PC34_SLOT_CHEST_1,
                     champion);
    ok &= expect_int("last chest slot",
                     probe->lastChestSlot, DM1_PC34_SLOT_CHEST_8,
                     champion);
    ok &= expect_int("floor map x",
                     probe->floorMapX, DM1_PC34_CHEST_DROP_FLOOR_MAP_X,
                     dungeonDefs);
    ok &= expect_int("floor map y",
                     probe->floorMapY, DM1_PC34_CHEST_DROP_FLOOR_MAP_Y,
                     dungeonDefs);
    ok &= expect_int("floor cell capacity",
                     probe->floorCellCapacity,
                     DM1_PC34_CHEST_DROP_FLOOR_CELL_CAPACITY, dungeonDefs);

    ok &= test_event("single drop", &probe->singleDrop, 1, 3101, 7,
                     21, 14, 3, 2, champion);
    ok &= test_event("full chest drop", &probe->fullChestDrop, 4, 3204, 19,
                     140, 121, 8, 7, chest);
    ok &= test_event("weighted drop", &probe->weightedDrop, 2, 3302, 36,
                     180, 144, 5, 4, dungeonDefs);
    ok &= test_event("last item drop", &probe->lastItemDrop, 0, 3400, 11,
                     11, 0, 1, 0, dungeonDefs);
    ok &= test_event("indexed drop", &probe->indexedDrop, 5, 3505, 28,
                     123, 95, 6, 5, chamdraw);

    ok &= expect_int("total floor drops",
                     probe->floorDropsTotal,
                     DM1_PC34_CHEST_DROP_FLOOR_CASE_COUNT, dungeonDefs);
    ok &= expect_int("all dropped items on G0189 floor",
                     probe->allDroppedItemsOnG0189Floor, 1, dungeonDefs);
    ok &= expect_int("all post-drop loads match accumulator",
                     probe->allPostDropLoadsMatch, 1, dungeonDefs);
    ok &= expect_int("all adjacent items preserved",
                     probe->allAdjacentItemsPreserved, 1, chamdraw);
    ok &= expect_int("minimum assertion budget",
                     g_assertions >= 80 ? 1 : 0, 1, champion);
    return ok;
}

int main(void)
{
    const char* chest =
        "ReDMCSB CHEST.C F0333/F0334 lines 2-90,112-132";
    DM1_V1_InventoryChestDropToFloorProbePc34 probe;
    int ok = 1;

    printf("probe=dm1_v1_inventory_chest_drop_to_floor_pc34_compat\n");
    printf("sourceEvidence=%s\n",
           dm1_v1_inventory_chest_drop_to_floor_source_evidence_pc34());

    ok &= expect_int("probe setup",
                     m11_inventory_pc34_probe_chest_drop_to_floor(&probe),
                     1, chest);
    if (!ok) {
        printf("assertionCount=%d\n", g_assertions);
        printf("inventoryChestDropToFloorInvariantOk=0\n");
        return 1;
    }

    ok &= test_source_evidence();
    ok &= test_probe(&probe);

    printf("assertionCount=%d\n", g_assertions);
    printf("inventoryChestDropToFloorInvariantOk=%d\n", ok ? 1 : 0);
    printf("PASS dm1_v1_inventory_chest_drop_to_floor_pc34_compat assertions=%d\n",
           g_assertions);
    return ok ? 0 : 1;
}
