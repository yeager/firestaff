#include "firestaff/dm1/v1/chest/open_mirror_rotation_three_way_pc34_compat.h"

#include <stdio.h>
#include <string.h>

#define EXPECTED_PASS779_HASH UINT32_C(0x939F0F83)

static int g_assertions;
static int g_failures;

#define PROBE_ASSERT(id, expr)                                                   \
    do {                                                                         \
        ++g_assertions;                                                          \
        if (!(expr)) {                                                           \
            ++g_failures;                                                        \
            printf("FAIL %s\n", (id));                                          \
        } else {                                                                 \
            printf("PASS %s\n", (id));                                          \
        }                                                                        \
    } while (0)

#define PROBE_ASSERT_EQ(id, got, want)                                           \
    do {                                                                         \
        int got_value__ = (int)(got);                                            \
        int want_value__ = (int)(want);                                          \
        ++g_assertions;                                                          \
        if (got_value__ != want_value__) {                                       \
            ++g_failures;                                                        \
            printf("FAIL %s got=%d want=%d\n",                                  \
                   (id), got_value__, want_value__);                             \
        } else {                                                                 \
            printf("PASS %s=%d\n", (id), got_value__);                          \
        }                                                                        \
    } while (0)

#define PROBE_ASSERT_U32(id, got, want)                                          \
    do {                                                                         \
        uint32_t got_value__ = (uint32_t)(got);                                  \
        uint32_t want_value__ = (uint32_t)(want);                                \
        ++g_assertions;                                                          \
        if (got_value__ != want_value__) {                                       \
            ++g_failures;                                                        \
            printf("FAIL %s got=0x%08X want=0x%08X\n",                          \
                   (id), (unsigned)got_value__, (unsigned)want_value__);         \
        } else {                                                                 \
            printf("PASS %s=0x%08X\n", (id), (unsigned)got_value__);            \
        }                                                                        \
    } while (0)

static void assert_contains(const char* id,
                            const char* haystack,
                            const char* needle)
{
    PROBE_ASSERT(id, haystack && needle && strstr(haystack, needle));
}

static int expected_before_type(int slotIndex)
{
    if (slotIndex == DM1_PC34_COMR3_TARGET_SLOT_INDEX) {
        return DM1_PC34_COMR3_TARGET_SLOT_ITEM;
    }
    return DM1_PC34_COMR3_FIRST_STABLE_ITEM + slotIndex;
}

static int expected_before_quantity(int slotIndex)
{
    if (slotIndex == DM1_PC34_COMR3_TARGET_SLOT_INDEX) {
        return DM1_PC34_COMR3_TARGET_SLOT_QUANTITY;
    }
    return DM1_PC34_COMR3_FIRST_QUANTITY + slotIndex;
}

static void assert_source_evidence(void)
{
    const char* evidence =
        dm1_v1_chest_open_mirror_rotation_three_way_source_evidence_pc34();
    const DM1_V1_ChestOpenMirrorRotationThreeWaySpecPc34* spec =
        dm1_v1_chest_open_mirror_rotation_three_way_spec_pc34();

    PROBE_ASSERT("spec.present", spec != NULL);
    assert_contains("evidence.F0333", evidence, "CHEST.C F0333:30-67");
    assert_contains("evidence.F0334", evidence, "CHEST.C F0334:113-132");
    assert_contains("evidence.F0297", evidence, "CHAMPION.C F0297:243-298");
    assert_contains("evidence.F0298", evidence, "CHAMPION.C F0298:270-298");
    assert_contains("evidence.F0300", evidence, "CHAMPION.C F0300:511-515");
    assert_contains("evidence.F0301", evidence, "CHAMPION.C F0301:606-614");
    assert_contains("evidence.F0302", evidence, "CHAMPION.C F0302:662-714");
    assert_contains("evidence.F0359", evidence, "COMMAND.C F0359:1452-1662");
    assert_contains("evidence.F0361", evidence, "COMMAND.C F0361:1709-1813");
    assert_contains("evidence.F0380", evidence, "COMMAND.C F0380:2045-2178");
    assert_contains("evidence.F0077", evidence, "IO.C F0077:1113-1122");
    assert_contains("evidence.F0078", evidence, "F0078:1102-1111");
    assert_contains("evidence.F0280", evidence, "REVIVE.C F0280:124-132");
    assert_contains("evidence.F0282", evidence, "REVIVE.C F0282:744-806");
    assert_contains("evidence.F0163", evidence, "DUNGEON.C F0163:1796-1837");
    assert_contains("evidence.DEFS", evidence, "C537..C544");
    assert_contains("evidence.MOUSE.deviation", evidence,
                    "MOUSE.C is not present");
    assert_contains("evidence.pass779", evidence, "pass779");
    assert_contains("spec.contract", spec->contractMarker, "pass779");
    assert_contains("spec.nonduplicate", spec->nonDuplicateMarker,
                    "not pass768");
}

static void assert_default_state(void)
{
    DM1_V1_ChestOpenMirrorRotationThreeWayStatePc34 state =
        dm1_v1_chest_open_mirror_rotation_three_way_default_state_pc34();
    M11_Item item;
    uint32_t hash_a;
    uint32_t hash_b;

    PROBE_ASSERT_EQ("default.contract_only", state.contractOnly, 1);
    PROBE_ASSERT_EQ("default.asset_free", state.assetFree, 1);
    PROBE_ASSERT_EQ("default.no_pixel_claim", state.noDosPixelParityClaim, 1);
    PROBE_ASSERT_EQ("default.leader", state.currentLeaderIndex,
                    DM1_PC34_COMR3_LEADER_BEFORE);
    PROBE_ASSERT_EQ("default.open_nonleader",
                    DM1_PC34_COMR3_OPEN_NON_LEADER, 1);
    PROBE_ASSERT_EQ("default.inventory_ordinal",
                    state.g0423InventoryChampionOrdinal,
                    DM1_PC34_COMR3_INVENTORY_ORDINAL);
    PROBE_ASSERT_EQ("default.candidate_index",
                    state.c040CandidateChampionIndex,
                    DM1_PC34_COMR3_CANDIDATE_CHAMPION);
    PROBE_ASSERT_EQ("default.g0299", state.g0299CandidateOrdinal,
                    DM1_PC34_COMR3_CANDIDATE_ORDINAL);
    PROBE_ASSERT_EQ("default.c040_live", state.c040PanelLive, 1);
    PROBE_ASSERT_EQ("default.candidate_queue_depth",
                    state.candidateHandQueueDepth, 1);
    PROBE_ASSERT_EQ("default.candidate_queue_item",
                    state.candidateHandQueueItem.type,
                    DM1_PC34_COMR3_CANDIDATE_HAND_ITEM);
    PROBE_ASSERT_EQ("default.open_chest",
                    m11_inventory_get_open_chest_thing(
                        &state.inventory, DM1_PC34_COMR3_OPEN_NON_LEADER),
                    DM1_PC34_COMR3_CHEST_THING);
    PROBE_ASSERT_EQ("default.panel",
                    m11_inventory_get_panel_content_pc34(&state.inventory),
                    DM1_PC34_PANEL_CHEST);
    (void)m11_inventory_get_mouse_item(
        &state.inventory, DM1_PC34_COMR3_OPEN_NON_LEADER, &item);
    PROBE_ASSERT_EQ("default.nonleader_hand", item.itemType,
                    DM1_PC34_COMR3_NON_LEADER_HAND_ITEM);
    PROBE_ASSERT_EQ("default.nonleader_hand_quantity",
                    state.handQuantities[DM1_PC34_COMR3_OPEN_NON_LEADER],
                    DM1_PC34_COMR3_HAND_QUANTITY);
    (void)m11_inventory_get_item_in_chest_slot(
        &state.inventory, DM1_PC34_COMR3_OPEN_NON_LEADER,
        DM1_PC34_COMR3_TARGET_SLOT_INDEX, &item);
    PROBE_ASSERT_EQ("default.c540_item", item.itemType,
                    DM1_PC34_COMR3_TARGET_SLOT_ITEM);
    PROBE_ASSERT_EQ("default.c540_quantity",
                    state.chestQuantities[DM1_PC34_COMR3_OPEN_NON_LEADER]
                                         [DM1_PC34_COMR3_TARGET_SLOT_INDEX],
                    DM1_PC34_COMR3_TARGET_SLOT_QUANTITY);
    hash_a =
        dm1_v1_chest_open_mirror_rotation_three_way_hash_state_pc34(&state);
    hash_b =
        dm1_v1_chest_open_mirror_rotation_three_way_hash_state_pc34(&state);
    PROBE_ASSERT("default.hash_nonzero", hash_a != 0u);
    PROBE_ASSERT_U32("default.hash_stable", hash_a, hash_b);
}

static void assert_probe(
    const DM1_V1_ChestOpenMirrorRotationThreeWayProbePc34* p)
{
    int i;

    PROBE_ASSERT_EQ("probe.runtime_regression", p->runtimeRegression, 1);
    PROBE_ASSERT_EQ("probe.step_count", p->stepCount, 5);
    PROBE_ASSERT_EQ("probe.step.default", p->stepTrace[0],
                    DM1_PC34_COMR3_STEP_DEFAULT_STATE);
    PROBE_ASSERT_EQ("probe.step.queue", p->stepTrace[1],
                    DM1_PC34_COMR3_STEP_QUEUE_WHEEL_AND_ROTATION);
    PROBE_ASSERT_EQ("probe.step.wheel", p->stepTrace[2],
                    DM1_PC34_COMR3_STEP_DRAIN_WHEEL_C540);
    PROBE_ASSERT_EQ("probe.step.rotation", p->stepTrace[3],
                    DM1_PC34_COMR3_STEP_DRAIN_ROTATION);
    PROBE_ASSERT_EQ("probe.step.still_live", p->stepTrace[4],
                    DM1_PC34_COMR3_STEP_ASSERT_C040_STILL_LIVE);

    PROBE_ASSERT_EQ("probe.leader_before", p->leaderBefore,
                    DM1_PC34_COMR3_LEADER_BEFORE);
    PROBE_ASSERT_EQ("probe.nonleader_open", p->nonLeaderOpenChampion,
                    DM1_PC34_COMR3_OPEN_NON_LEADER);
    PROBE_ASSERT_EQ("probe.candidate_index", p->candidateChampionIndex,
                    DM1_PC34_COMR3_CANDIDATE_CHAMPION);
    PROBE_ASSERT_EQ("probe.candidate_ordinal_before",
                    p->candidateOrdinalBefore,
                    DM1_PC34_COMR3_CANDIDATE_ORDINAL);
    PROBE_ASSERT_EQ("probe.candidate_chain0", p->candidateChainBefore[0],
                    DM1_PC34_COMR3_CANDIDATE_ORDINAL);
    PROBE_ASSERT_EQ("probe.candidate_chain1", p->candidateChainBefore[1], 4);
    PROBE_ASSERT_EQ("probe.candidate_queue_before",
                    p->candidateHandQueueDepthBefore, 1);
    PROBE_ASSERT_EQ("probe.candidate_item_before",
                    p->candidateHandQueueItemBefore,
                    DM1_PC34_COMR3_CANDIDATE_HAND_ITEM);
    PROBE_ASSERT_EQ("probe.open_chest_before", p->openChestThingBefore,
                    DM1_PC34_COMR3_CHEST_THING);
    PROBE_ASSERT_EQ("probe.g0426_open_before", p->g0426OpenBefore, 1);
    PROBE_ASSERT_EQ("probe.panel_before", p->panelContentBefore,
                    DM1_PC34_PANEL_CHEST);
    PROBE_ASSERT_EQ("probe.hand_before", p->handTypeBefore,
                    DM1_PC34_COMR3_NON_LEADER_HAND_ITEM);
    PROBE_ASSERT_EQ("probe.hand_quantity_before", p->handQuantityBefore,
                    DM1_PC34_COMR3_HAND_QUANTITY);
    PROBE_ASSERT_EQ("probe.target_before", p->targetSlotTypeBefore,
                    DM1_PC34_COMR3_TARGET_SLOT_ITEM);
    PROBE_ASSERT_EQ("probe.target_quantity_before",
                    p->targetSlotQuantityBefore,
                    DM1_PC34_COMR3_TARGET_SLOT_QUANTITY);
    for (i = 0; i < DM1_PC34_COMR3_SLOT_COUNT; ++i) {
        char label[64];
        snprintf(label, sizeof(label), "probe.visible_type_before.%d", i);
        PROBE_ASSERT_EQ(label, p->visibleTypesBefore[i],
                        expected_before_type(i));
        snprintf(label, sizeof(label), "probe.visible_qty_before.%d", i);
        PROBE_ASSERT_EQ(label, p->visibleQuantitiesBefore[i],
                        expected_before_quantity(i));
    }

    PROBE_ASSERT_EQ("probe.wheel_queued", p->wheelQueued, 1);
    PROBE_ASSERT_EQ("probe.rotation_queued", p->rotationQueued, 1);
    PROBE_ASSERT_EQ("probe.queue_depth", p->commandQueueDepthAfterQueue, 2);
    PROBE_ASSERT_EQ("probe.queued_wheel", p->queuedWheelCommand,
                    DM1_PC34_COMR3_TARGET_COMMAND);
    PROBE_ASSERT_EQ("probe.queued_rotation", p->queuedRotationCommand,
                    DM1_PC34_COMR3_ROTATION_COMMAND);
    PROBE_ASSERT_EQ("probe.wheel_before_rotation",
                    p->queuedWheelBeforeRotation, 1);

    PROBE_ASSERT_EQ("probe.wheel_result", p->wheelDrainResult, 1);
    PROBE_ASSERT_EQ("probe.wheel_before_rotation_drain",
                    p->wheelDrainedBeforeRotation, 1);
    PROBE_ASSERT_EQ("probe.f0077_f0078_balanced",
                    p->f0077F0078BalancedAfterWheel, 1);
    PROBE_ASSERT_EQ("probe.f0302_after_wheel",
                    p->f0302DispatchCountAfterWheel, 1);
    PROBE_ASSERT_EQ("probe.f0300_after_wheel",
                    p->f0300ClearCountAfterWheel, 1);
    PROBE_ASSERT_EQ("probe.f0301_after_wheel",
                    p->f0301WriteCountAfterWheel, 1);
    PROBE_ASSERT_EQ("probe.hand_after_wheel", p->handTypeAfterWheel,
                    DM1_PC34_COMR3_TARGET_SLOT_ITEM);
    PROBE_ASSERT_EQ("probe.hand_quantity_after_wheel",
                    p->handQuantityAfterWheel,
                    DM1_PC34_COMR3_TARGET_SLOT_QUANTITY);
    PROBE_ASSERT_EQ("probe.c540_after_wheel", p->c540TypeAfterWheel,
                    DM1_PC34_COMR3_NON_LEADER_HAND_ITEM);
    PROBE_ASSERT_EQ("probe.c540_quantity_after_wheel",
                    p->c540QuantityAfterWheel,
                    DM1_PC34_COMR3_HAND_QUANTITY);
    PROBE_ASSERT_EQ("probe.queue_depth_after_wheel",
                    p->commandQueueDepthAfterWheel, 1);
    PROBE_ASSERT_EQ("probe.rotation_still_queued",
                    p->rotationStillQueuedAfterWheel, 1);
    PROBE_ASSERT_EQ("probe.candidate_after_wheel",
                    p->candidateOrdinalAfterWheel,
                    DM1_PC34_COMR3_CANDIDATE_ORDINAL);
    PROBE_ASSERT_EQ("probe.candidate_queue_after_wheel",
                    p->candidateHandQueueDepthAfterWheel, 1);
    PROBE_ASSERT_EQ("probe.candidate_item_after_wheel",
                    p->candidateHandQueueItemAfterWheel,
                    DM1_PC34_COMR3_CANDIDATE_HAND_ITEM);
    PROBE_ASSERT_EQ("probe.chain_stable_after_wheel",
                    p->candidateChainStableAfterWheel, 1);
    PROBE_ASSERT_EQ("probe.g0299_stable_after_wheel",
                    p->g0299StableAfterWheel, 1);
    PROBE_ASSERT_EQ("probe.g0426_stable_after_wheel",
                    p->g0426StableAfterWheel, 1);
    PROBE_ASSERT_EQ("probe.panel_still_chest_after_wheel",
                    p->panelStillChestAfterWheel, 1);

    PROBE_ASSERT_EQ("probe.rotation_result", p->rotationDrainResult, 1);
    PROBE_ASSERT_EQ("probe.leader_after_rotation", p->leaderAfterRotation,
                    DM1_PC34_COMR3_LEADER_AFTER_ROTATION);
    PROBE_ASSERT_EQ("probe.queue_depth_after_rotation",
                    p->commandQueueDepthAfterRotation, 0);
    PROBE_ASSERT_EQ("probe.candidate_after_rotation",
                    p->candidateOrdinalAfterRotation,
                    DM1_PC34_COMR3_CANDIDATE_ORDINAL);
    PROBE_ASSERT_EQ("probe.candidate_queue_after_rotation",
                    p->candidateHandQueueDepthAfterRotation, 1);
    PROBE_ASSERT_EQ("probe.candidate_item_after_rotation",
                    p->candidateHandQueueItemAfterRotation,
                    DM1_PC34_COMR3_CANDIDATE_HAND_ITEM);
    PROBE_ASSERT_EQ("probe.chain_stable_after_rotation",
                    p->candidateChainStableAfterRotation, 1);
    PROBE_ASSERT_EQ("probe.g0426_stable_after_rotation",
                    p->g0426StableAfterRotation, 1);
    PROBE_ASSERT_EQ("probe.panel_still_chest_after_rotation",
                    p->panelStillChestAfterRotation, 1);
    PROBE_ASSERT_EQ("probe.hand_after_rotation", p->handTypeAfterRotation,
                    DM1_PC34_COMR3_TARGET_SLOT_ITEM);
    PROBE_ASSERT_EQ("probe.c540_after_rotation", p->c540TypeAfterRotation,
                    DM1_PC34_COMR3_NON_LEADER_HAND_ITEM);
    PROBE_ASSERT_EQ("probe.chain_coherent_after_rotation",
                    p->c537ToC544ChainCoherentAfterRotation, 1);
    PROBE_ASSERT_EQ("probe.f0282_never_drained",
                    p->f0282NeverDrainedCandidate, 1);
    PROBE_ASSERT_EQ("probe.chest_never_closed", p->chestNeverClosed, 1);
    PROBE_ASSERT_EQ("probe.no_asset_read", p->noAssetRead, 1);
    PROBE_ASSERT_EQ("probe.no_pixel_claim", p->noPixelParityClaim, 1);
    PROBE_ASSERT_EQ("probe.nonduplicate_three_way",
                    p->nonDuplicateThreeWay, 1);
    PROBE_ASSERT_U32("probe.hash", p->deterministicHash,
                     EXPECTED_PASS779_HASH);
}

int main(void)
{
    DM1_V1_ChestOpenMirrorRotationThreeWayProbePc34 probe;
    int ok;

    assert_source_evidence();
    assert_default_state();
    ok = dm1_v1_chest_open_mirror_rotation_three_way_run_pc34(&probe);
    PROBE_ASSERT_EQ("run.ok", ok, 1);
    assert_probe(&probe);

    if (g_failures) {
        printf("FAIL assertions=%d failures=%d hash=0x%08X\n",
               g_assertions, g_failures,
               (unsigned)probe.deterministicHash);
        return 1;
    }
    printf("PASS test_dm1_v1_chest_open_mirror_rotation_three_way_pc34_compat "
           "assertions=%d failures=0 hash=0x%08X\n",
           g_assertions, (unsigned)probe.deterministicHash);
    return 0;
}
