/*
 * test_theron_v1_world_serialize_purchase_state.c
 *
 * Focused Theron's Quest V1 regression for persistent purchase-state data.
 *
 * The current Theron codebase stores party gold and champion inventories in
 * the world serialization stream, so this fixture-driven test guards that the
 * "shop / item purchase" state survives a serialize -> deserialize round-trip
 * and that the gold field remains at the front of the packed party block.
 *
 * Source-lock note: THQUEST.ASM T800 covers champion persistence / inventory
 * reset. The test intentionally stays inside the existing public API and does
 * not require Track 02 assets or a full launch.
 */

#include "theron_v1_world.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int g_failures = 0;

static void expect_true(int condition, const char *message) {
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        ++g_failures;
    }
}

static size_t world_gold_offset(void) {
    return sizeof(uint32_t) +
           sizeof(uint16_t) +
           sizeof(uint16_t) +
           sizeof(uint8_t) +
           sizeof(uint8_t) +
           sizeof(uint8_t) +
           sizeof(uint8_t) +
           sizeof(Theron_DungeonProgression);
}

static void seed_world(Theron_V1_World *world) {
    theron_v1_world_init(world);
    world->current_dungeon = THERON_DUNGEON_3_ABYSS_OF_FLAMES;
    world->current_level = 1;
    world->quest_items_in_dungeon = 1;
    world->dungeon_complete = 1;
    world->progression.current_dungeon = THERON_DUNGEON_3_ABYSS_OF_FLAMES;
    world->progression.current_level = 2;
    world->progression.quest_items_collected = THERON_QUEST_ITEM_1_SACRED_AMPLIFIER |
                                               THERON_QUEST_ITEM_2_SHADOW_KEY;
    world->progression.dungeon_states[0] = THERON_DUNGEON_STATE_COMPLETE;
    world->progression.dungeon_states[1] = THERON_DUNGEON_STATE_COMPLETE;
    world->progression.dungeon_states[2] = THERON_DUNGEON_STATE_AVAILABLE;
    world->party.gold = 0x0badf00dU;
    world->party.champions[0].inventory[0] = THERON_ITEM_KEY;
    world->party.champions[0].inventory[1] = THERON_ITEM_POTION;
    world->party.champions[1].inventory[0] = THERON_ITEM_SCROLL;
    world->party.champions[1].inventory[3] = THERON_ITEM_SHIELD;
    theron_v1_party_recalculate_loads(&world->party);
}

static void test_round_trip_keeps_purchase_state(void) {
    Theron_V1_World original;
    Theron_V1_World restored;
    size_t size;
    uint8_t *buffer;
    size_t gold_offset;
    uint32_t encoded_gold;

    printf("  %-55s ", "World serialize round-trip keeps gold and items");
    fflush(stdout);

    seed_world(&original);
    size = theron_v1_world_serialize_size(&original);
    expect_true(size > 0, "serialize size should be non-zero");
    buffer = (uint8_t *)malloc(size);
    expect_true(buffer != NULL, "serialize buffer allocated");
    if (!buffer) {
        printf("FAIL\n");
        ++g_failures;
        return;
    }

    expect_true(theron_v1_world_serialize(&original, buffer, size) == size,
                "serialize returned the expected size");

    gold_offset = world_gold_offset();
    expect_true(gold_offset + sizeof(uint32_t) <= size,
                "gold offset fits inside serialized world");
    if (gold_offset + sizeof(uint32_t) <= size) {
        encoded_gold = (uint32_t)buffer[gold_offset + 0]
                     | ((uint32_t)buffer[gold_offset + 1] << 8)
                     | ((uint32_t)buffer[gold_offset + 2] << 16)
                     | ((uint32_t)buffer[gold_offset + 3] << 24);
        expect_true(encoded_gold == original.party.gold,
                    "serialized party gold stays at the front of the party pack");
    }

    memset(&restored, 0, sizeof(restored));
    expect_true(theron_v1_world_deserialize(&restored, buffer, size) == 0,
                "deserialize succeeds");
    expect_true(restored.party.gold == original.party.gold,
                "party gold survives round-trip");
    expect_true(restored.party.champions[0].inventory[0] == THERON_ITEM_KEY,
                "leader inventory slot 0 survives round-trip");
    expect_true(restored.party.champions[0].inventory[1] == THERON_ITEM_POTION,
                "leader inventory slot 1 survives round-trip");
    expect_true(restored.party.champions[1].inventory[0] == THERON_ITEM_SCROLL,
                "companion inventory slot 0 survives round-trip");
    expect_true(restored.party.champions[1].inventory[3] == THERON_ITEM_SHIELD,
                "companion inventory slot 3 survives round-trip");
    expect_true(restored.progression.quest_items_collected ==
                original.progression.quest_items_collected,
                "quest item bitmask survives round-trip");
    expect_true(restored.dungeon_complete == original.dungeon_complete,
                "dungeon completion flag survives round-trip");

    free(buffer);

    if (g_failures == 0) {
        printf("PASS\n");
    } else {
        printf("FAIL\n");
    }
}

int main(void) {
    test_round_trip_keeps_purchase_state();
    return g_failures == 0 ? 0 : 1;
}
