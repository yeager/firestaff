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
#include "theron_v1_track02_thing_data.h"

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

static size_t world_object_count_offset(void) {
    return world_gold_offset() +
           THERON_MAX_CHAMPIONS * sizeof(Theron_V1_Champion) +
           sizeof(uint32_t);
}

static void put_le32(uint8_t *p, uint32_t value) {
    p[0] = (uint8_t)value;
    p[1] = (uint8_t)(value >> 8);
    p[2] = (uint8_t)(value >> 16);
    p[3] = (uint8_t)(value >> 24);
}

static void seed_world(Theron_V1_World *world) {
    theron_v1_world_init(world);
    world->current_dungeon = THERON_DUNGEON_3_FORMIC;
    world->current_level = 1;
    world->quest_items_in_dungeon = 1;
    world->dungeon_complete = 1;
    world->progression.current_dungeon = THERON_DUNGEON_3_FORMIC;
    world->progression.current_level = 2;
    world->progression.quest_items_collected = THERON_QUEST_ITEM_1_SHIELD_DEFIANT |
                                               THERON_QUEST_ITEM_2_TAZA_BOOTS;
    world->progression.dungeon_states[0] = THERON_DUNGEON_STATE_COMPLETE;
    world->progression.dungeon_states[1] = THERON_DUNGEON_STATE_COMPLETE;
    world->progression.dungeon_states[2] = THERON_DUNGEON_STATE_AVAILABLE;
    world->party.gold = 0x0badf00dU;
    world->party.champions[0].inventory[0] = THERON_ITEM_KEY;
    world->party.champions[0].inventory[1] = THERON_ITEM_POTION;
    world->party.champions[1].inventory[0] = THERON_ITEM_SCROLL;
    world->party.champions[1].inventory[3] = THERON_ITEM_SHIELD;
    world->inventory_source[0][0].valid = 1;
    world->inventory_source[0][0].category = THERON_CAT_WEAPON;
    world->inventory_source[0][0].item_type = 9;
    world->inventory_source[0][0].poisoned = 1;
    world->inventory_source[0][0].charges = 7;
    world->inventory_source[0][0].source_ref = 0x1234;
    world->inventory_source[0][0].source_next_ref = 0x5678;
    world->inventory_source[0][0].text_ref = 0x0042;
    world->inventory_source[0][0].property_valid = 1;
    world->inventory_source[0][0].property[0] = 0x20;
    world->inventory_source[0][0].property[5] = 0x0a;
    world->object_count = 1;
    world->objects[0].id = 0x10203040;
    world->objects[0].type = THERON_OBJTYPE_WEAPON;
    world->objects[0].state = 3;
    world->objects[0].x = 7;
    world->objects[0].y = 9;
    world->objects[0].level = 2;
    world->objects[0].dungeon_id = THERON_DUNGEON_3_FORMIC;
    world->objects[0].quantity = 4;
    world->objects[0].item_index = 12;
    world->objects[0].linked_id = 0x55667788;
    world->objects[0].flags = 0xa5a5a5a5U;
    world->objects[0].source_ref = 0x2345;
    world->objects[0].source_category = THERON_CAT_POTION;
    world->objects[0].source_raw_size = 6;
    world->objects[0].source_raw[0] = 0xde;
    world->objects[0].source_raw[5] = 0xad;
    world->objects[0].source_text_ref = 0x0088;
    world->objects[0].source_property_valid = 1;
    world->objects[0].source_property[2] = 0x44;
    world->timer_count = 1;
    world->timers[0].id = 19;
    world->timers[0].kind = THERON_TIMER_REPEAT;
    world->timers[0].level = 2;
    world->timers[0].remaining_ticks = 37;
    world->timers[0].interval_ticks = 120;
    world->timers[0].flags = 0x01020304U;
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
    expect_true(restored.inventory_source[0][0].valid &&
                restored.inventory_source[0][0].category == THERON_CAT_WEAPON &&
                restored.inventory_source[0][0].item_type == 9 &&
                restored.inventory_source[0][0].poisoned == 1 &&
                restored.inventory_source[0][0].charges == 7 &&
                restored.inventory_source[0][0].source_ref == 0x1234 &&
                restored.inventory_source[0][0].source_next_ref == 0x5678 &&
                restored.inventory_source[0][0].text_ref == 0x0042 &&
                restored.inventory_source[0][0].property_valid &&
                restored.inventory_source[0][0].property[0] == 0x20 &&
                restored.inventory_source[0][0].property[5] == 0x0a,
                "source item provenance survives round-trip");
    expect_true(restored.progression.quest_items_collected ==
                original.progression.quest_items_collected,
                "quest item bitmask survives round-trip");
    expect_true(restored.dungeon_complete == original.dungeon_complete,
                "dungeon completion flag survives round-trip");
    expect_true(restored.object_count == 1 &&
                restored.objects[0].id == 0x10203040 &&
                restored.objects[0].type == THERON_OBJTYPE_WEAPON &&
                restored.objects[0].x == 7 && restored.objects[0].y == 9 &&
                restored.objects[0].flags == 0xa5a5a5a5U &&
                restored.objects[0].source_ref == 0x2345 &&
                restored.objects[0].source_raw[0] == 0xde &&
                restored.objects[0].source_raw[5] == 0xad &&
                restored.objects[0].source_property[2] == 0x44,
                "portable object record survives round-trip");
    expect_true(restored.timer_count == 1 && restored.timers[0].id == 19 &&
                restored.timers[0].kind == THERON_TIMER_REPEAT &&
                restored.timers[0].remaining_ticks == 37 &&
                restored.timers[0].interval_ticks == 120 &&
                restored.timers[0].flags == 0x01020304U &&
                restored.timers[0].userdata == NULL,
                "portable timer record survives round-trip");

    free(buffer);

    if (g_failures == 0) {
        printf("PASS\n");
    } else {
        printf("FAIL\n");
    }
}

static void test_corrupt_counts_are_rejected(void) {
    Theron_V1_World original;
    Theron_V1_World restored;
    size_t size;
    uint8_t *buffer;
    size_t count_offset;

    printf("  %-55s ", "World deserialize rejects corrupt object/timer counts");
    fflush(stdout);
    seed_world(&original);
    size = theron_v1_world_serialize_size(&original);
    buffer = (uint8_t *)malloc(size);
    expect_true(buffer != NULL, "corrupt-count buffer allocated");
    if (!buffer) {
        printf("FAIL\n");
        return;
    }
    expect_true(theron_v1_world_serialize(&original, buffer, size) == size,
                "corrupt-count fixture serialized");
    count_offset = world_object_count_offset();
    put_le32(buffer + count_offset, THERON_MAX_OBJECTS + 1u);
    memset(&restored, 0, sizeof(restored));
    expect_true(theron_v1_world_deserialize(&restored, buffer, size) == -1,
                "object count above capacity is rejected");

    expect_true(theron_v1_world_serialize(&original, buffer, size) == size,
                "timer-count fixture reserialized");
    put_le32(buffer + count_offset, 0u);
    put_le32(buffer + count_offset + sizeof(uint32_t), THERON_MAX_TIMERS + 1u);
    expect_true(theron_v1_world_deserialize(&restored, buffer, size) == -1,
                "timer count above capacity is rejected");
    free(buffer);
    puts(g_failures == 0 ? "PASS" : "FAIL");
}

int main(void) {
    test_round_trip_keeps_purchase_state();
    test_corrupt_counts_are_rejected();
    return g_failures == 0 ? 0 : 1;
}
