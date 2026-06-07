#include "dm1_v1_inventory_chest_pickup_encumbrance_pc34_compat.h"

#include <stdio.h>
#include <stdint.h>

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

static int expect_u32(const char* label,
                      uint32_t got,
                      uint32_t want,
                      const char* redmcsbAnchor)
{
    ++g_assertions;
    if (!redmcsbAnchor || redmcsbAnchor[0] == '\0') {
        printf("FAIL %s missing ReDMCSB anchor\n", label);
        return 0;
    }
    if (got != want) {
        printf("FAIL %s got=%u want=%u anchor=%s\n", label, got, want,
               redmcsbAnchor);
        return 0;
    }
    printf("PASS %s=%u anchor=%s\n", label, got, redmcsbAnchor);
    return 1;
}

static int test_event(const char* prefix,
                      const DM1_V1_InventoryChestPickupEncumbranceEventPc34* event,
                      int champion,
                      int chestSlot,
                      int storageSlot,
                      uint32_t weight,
                      uint32_t before,
                      uint32_t after,
                      int wasEncumbered,
                      int isEncumbered,
                      int saturated,
                      const char* redmcsbAnchor)
{
    int ok = 1;
    char label[96];

#define CHECK_INT(name, got, want) \
    do { \
        snprintf(label, sizeof(label), "%s %s", prefix, name); \
        ok &= expect_int(label, (got), (want), redmcsbAnchor); \
    } while (0)
#define CHECK_U32(name, got, want) \
    do { \
        snprintf(label, sizeof(label), "%s %s", prefix, name); \
        ok &= expect_u32(label, (got), (want), redmcsbAnchor); \
    } while (0)

    CHECK_INT("result", event->result, 1);
    CHECK_INT("champion", event->championIndex, champion);
    CHECK_INT("chest slot", event->chestSlotIndex, chestSlot);
    CHECK_INT("storage slot", event->storageSlotIndex, storageSlot);
    CHECK_U32("item weight", event->itemWeight, weight);
    CHECK_U32("load before", event->loadBefore, before);
    CHECK_U32("load after", event->loadAfter, after);
    CHECK_INT("encumbered before", event->encumberedBefore, wasEncumbered);
    CHECK_INT("encumbered after", event->encumberedAfter, isEncumbered);
    CHECK_INT("stored in champion slot", event->storedInChampionSlot, 1);
    CHECK_INT("cleared chest slot", event->clearedChestSlot, 1);
    CHECK_INT("saturated flag", event->saturated, saturated);

#undef CHECK_INT
#undef CHECK_U32
    return ok;
}

static int test_probe(const DM1_V1_InventoryChestPickupEncumbranceProbePc34* probe)
{
    const char* f0302F0301 =
        "ReDMCSB CHAMPION.C F0302 lines 688-710; F0297 lines 263-266; F0301 lines 609-615";
    const char* f0310 =
        "ReDMCSB CHAMPION.C F0309 lines 1167-1177; F0310 lines 1198-1205";
    const char* f0140Defs =
        "ReDMCSB DUNGEON.C F0140 lines 1102-1119; DEFS.H lines 1712-1738";
    int ok = 1;
    int i;

    ok &= expect_int("source locked contract marker",
                     probe->sourceLockedContractOnly, 1, f0302F0301);

    ok &= test_event("first pickup", &probe->firstPickup, 0, 0,
                     DM1_SLOT_BACKPACK1, 23, 12, 35, 0, 0, 0, f0302F0301);
    ok &= test_event("threshold pickup", &probe->thresholdPickup, 1, 1,
                     DM1_SLOT_BACKPACK2, 13, 38, 51, 0, 1, 0, f0310);
    ok &= test_event("saturating pickup", &probe->saturatingPickup, 2, 2,
                     DM1_SLOT_BACKPACK3, 9, UINT32_MAX - 3U, UINT32_MAX,
                     1, 1, 1, f0310);
    ok &= test_event("double pickup", &probe->doublePickup, 3, 3,
                     DM1_SLOT_BACKPACK4, 42, 42, 84, 0, 0, 0, f0302F0301);

    for (i = 0; i < DM1_PC34_CHEST_PICKUP_ENCUMBRANCE_CHAMPION_COUNT; ++i) {
        const uint32_t before = (uint32_t)(i * 10);
        const uint32_t weight = (uint32_t)(11 + (i * 7));
        const uint32_t after = before + weight;
        const uint32_t maxLoad = (uint32_t)(40 + (i * 25));
        char label[96];

        snprintf(label, sizeof(label), "champion %d isolated pickup", i);
        ok &= test_event(label, &probe->championPickups[i], i, i,
                         DM1_SLOT_BACKPACK1 + i, weight, before, after,
                         0, after >= maxLoad ? 1 : 0, 0, f0302F0301);
        snprintf(label, sizeof(label), "champion %d final load", i);
        ok &= expect_u32(label, probe->championLoads[i], after, f0302F0301);
        snprintf(label, sizeof(label), "champion %d max load", i);
        ok &= expect_u32(label, probe->championMaxLoads[i], maxLoad, f0310);
        snprintf(label, sizeof(label), "champion %d encumbrance independent", i);
        ok &= expect_int(label, probe->championEncumbered[i],
                         after >= maxLoad ? 1 : 0, f0310);
        snprintf(label, sizeof(label), "champion %d storage independent", i);
        ok &= expect_int(label, probe->independentChampionStorage[i],
                         1000 + i, f0302F0301);
    }

    ok &= expect_u32("weight byte zero",
                     probe->weightByteCases[0], 0, f0140Defs);
    ok &= expect_u32("weight byte one",
                     probe->weightByteCases[1], 1, f0140Defs);
    ok &= expect_u32("weight byte high nonmax",
                     probe->weightByteCases[2], 254, f0140Defs);
    ok &= expect_u32("weight byte max",
                     probe->weightByteCases[3], 255, f0140Defs);
    ok &= expect_u32("four byte weights accumulate past one byte",
                     probe->weightByteCaseSum, 510, f0140Defs);
    ok &= expect_u32("four byte accumulator saturates not wraps",
                     probe->weightByteCaseSaturatedSum, UINT32_MAX,
                     f0140Defs);

    ok &= expect_int("minimum assertion budget",
                     g_assertions >= 50 ? 1 : 0, 1, f0302F0301);

    return ok;
}

int main(void)
{
    const char* f0333 =
        "ReDMCSB CHEST.C F0333 lines 53-76";
    DM1_V1_InventoryChestPickupEncumbranceProbePc34 probe;
    int ok = 1;

    printf("probe=dm1_v1_inventory_chest_pickup_encumbrance_pc34_compat\n");
    printf("sourceEvidence=%s\n",
           dm1_v1_inventory_chest_pickup_encumbrance_source_evidence_pc34());

    ok &= expect_int("probe setup",
                     m11_inventory_pc34_probe_chest_pickup_encumbrance(&probe),
                     1, f0333);
    if (ok) {
        ok &= test_probe(&probe);
    }

    printf("assertionCount=%d\n", g_assertions);
    printf("inventoryChestPickupEncumbranceInvariantOk=%d\n", ok ? 1 : 0);
    return ok ? 0 : 1;
}
