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
#include "theron_v1_track02_actuator.h"
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
           sizeof(uint32_t) + 22u;
}

static size_t world_party_control_offset(void) {
    return world_gold_offset() + sizeof(uint32_t) +
           THERON_MAX_CHAMPIONS * sizeof(Theron_V1_Champion);
}

static void put_le32(uint8_t *p, uint32_t value) {
    p[0] = (uint8_t)value;
    p[1] = (uint8_t)(value >> 8);
    p[2] = (uint8_t)(value >> 16);
    p[3] = (uint8_t)(value >> 24);
}

static void put_le16(uint8_t *p, uint16_t value) {
    p[0] = (uint8_t)value;
    p[1] = (uint8_t)(value >> 8);
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
    world->party.champion_count = 3;
    world->party.active_slot = 2;
    world->party.leader_x = 17;
    world->party.leader_y = 9;
    world->party.leader_dir = 3;
    world->party.levitating = 1;
    world->party.door_state_override = 0x1357;
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
    world->inventory_source[0][0].source_position = 2;
    world->inventory_source[0][0].source_origin_valid = 1;
    world->inventory_source[0][0].source_dungeon = 4;
    world->inventory_source[0][0].source_level = 3;
    world->inventory_source[0][0].source_x = 17;
    world->inventory_source[0][0].source_y = 9;
    world->inventory_source[0][0].text_ref = 0x0042;
    world->inventory_source[0][0].property_valid = 1;
    world->inventory_source[0][0].property[0] = 0x20;
    world->inventory_source[0][0].property[5] = 0x0a;
    world->inventory_source[0][0].source_raw_size = 6;
    world->inventory_source[0][0].source_raw[0] = 0xde;
    world->inventory_source[0][0].source_raw[5] = 0xad;
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
    world->objects[0].source_origin_valid = 1;
    world->objects[0].source_dungeon = 6;
    world->objects[0].source_level = 4;
    world->objects[0].source_x = 21;
    world->objects[0].source_y = 11;
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
    world->creature_count = 1;
    world->creatures[0].id = 77;
    world->creatures[0].type = THERON_CREATURE_AKUTUBA;
    world->creatures[0].level = 2;
    world->creatures[0].dungeon_id = THERON_DUNGEON_3_FORMIC;
    world->creatures[0].x = 11;
    world->creatures[0].y = 13;
    world->creatures[0].hp = 88;
    world->creatures[0].max_hp = 120;
    world->creatures[0].speed = 5;
    world->creatures[0].next_move_tick = 99;
    world->creatures[0].ai = THERON_AI_GUARD;
    world->creatures[0].primary_attack = THERON_ATTACK_SLASH;
    world->creatures[0].flags = THERON_CF_ACTIVE | THERON_CF_ALERTED;
    world->creatures[0].source_ref = 0x3456;
    world->creatures[0].source_index = 0x0012;
    world->creatures[0].source_chested = -2;
    world->creatures[0].source_position = 0x39;
    world->creatures[0].source_cell = 2;
    world->creatures[0].source_slot = 1;
    world->creatures[0].source_group_count = 3;
    world->creatures[0].source_direction_flags = 0x81;
    world->creatures[0].source_flags_word = 0x4567;
    world->creatures[0].source_unknown_word = 0x89ab;
    world->creatures[0].source_spawn_category = 3;
    world->creatures[0].source_raw_size = 16;
    for (int raw_index = 0; raw_index < 16; ++raw_index)
    world->creatures[0].source_raw[raw_index] =
            (uint8_t)(0x80u + (unsigned int)raw_index);
    world->source_monster_count = 1;
    world->source_monsters[0].dungeon_id = THERON_DUNGEON_3_FORMIC;
    world->source_monsters[0].level = 2;
    world->source_monsters[0].x = 19;
    world->source_monsters[0].y = 21;
    world->source_monsters[0].source_ref = 0x0c81;
    world->source_monsters[0].source_index = 0x0038;
    world->source_monsters[0].chested = -2;
    world->source_monsters[0].type = 0x0a;
    world->source_monsters[0].position = 1;
    world->source_monsters[0].number = 1;
    world->source_monsters[0].direction_flags = 4;
    world->source_monsters[0].flags_word = 0x0420;
    world->source_monsters[0].health[0] = 59;
    world->source_monsters[0].health[1] = 37;
    world->source_monsters[0].health[2] = 36;
    world->source_monsters[0].health[3] = 37;
    world->source_monsters[0].raw_size = 16;
    {
        static const uint8_t authentic_category4[16] = {
            0xfe, 0xff, 0xfe, 0xff, 0x0a, 0x01, 0x3b, 0x00,
            0x25, 0x00, 0x24, 0x00, 0x25, 0x00, 0x20, 0x04
        };
        memcpy(world->source_monsters[0].raw, authentic_category4,
               sizeof(authentic_category4));
    }
    world->source_generator_count = 1;
    world->source_generators[0].dungeon_id = THERON_DUNGEON_3_FORMIC;
    world->source_generators[0].level = 2;
    world->source_generators[0].x = 17;
    world->source_generators[0].y = 19;
    world->source_generators[0].source_ref = 0x4567;
    world->source_generators[0].source_index = 0x001a;
    world->source_generators[0].type = 6;
    world->source_generators[0].value = 0x0203;
    world->source_generators[0].once = 1;
    world->source_generators[0].effect = 0;
    world->source_generators[0].revert_effect = 1;
    world->source_generators[0].sound = 5;
    world->source_generators[0].delay = 6;
    world->source_generators[0].local_effect = 1;
    world->source_generators[0].graphism = 7;
    world->source_generators[0].target_x = 21;
    world->source_generators[0].target_y = 22;
    world->source_generators[0].target_facing = 3;
    world->source_generators[0].generator_fields_valid = 1;
    world->source_generators[0].generator_generation = 4;
    world->source_generators[0].generator_toughness = 0x12;
    world->source_generators[0].generator_pause = 0x34;
    world->generator_spawn_count[0] = 2;
    world->generator_next_tick[0] = 0x0102030405060708ULL;
    /* Real US Track 02 data contains a 14-generator map. Keep the last
     * slot populated so this cannot regress to the old five-slot fixture. */
    world->generator_spawn_count[13] = 9;
    world->generator_next_tick[13] = 0x1112131415161718ULL;
    world->generator_active_count = 14;
    world->source_object_count = 1;
    world->source_objects[0].dungeon_id = THERON_DUNGEON_3_FORMIC;
    world->source_objects[0].level = 2;
    world->source_objects[0].x = 7;
    world->source_objects[0].y = 9;
    world->source_objects[0].source_ref = 0x1234;
    world->source_objects[0].next_ref = 0x5678;
    world->source_objects[0].source_index = 0x0042;
    world->source_objects[0].category = THERON_CAT_WEAPON;
    world->source_objects[0].position = 0;
    world->source_objects[0].raw_size = 6;
    world->source_objects[0].raw[0] = 0xde;
    world->source_objects[0].raw[5] = 0xad;
    world->source_actuator_event_count = 1;
    world->source_actuator_events[0].due_tick = 114;
    world->source_actuator_events[0].source_ref = 0x0c5d;
    world->source_actuator_events[0].source_index = 93;
    world->source_actuator_events[0].dungeon_id = 3;
    world->source_actuator_events[0].level = 2;
    world->source_actuator_events[0].source_x = 5;
    world->source_actuator_events[0].source_y = 0;
    world->source_actuator_events[0].target_x = 6;
    world->source_actuator_events[0].target_y = 5;
    world->source_actuator_events[0].target_facing = 0;
    world->source_actuator_events[0].effect = TQ_ACT_EFFECT_SET;
    world->source_actuator_events[0].local_effect = 0;
    world->source_actuator_events[0].sound = 0;
    world->source_actuator_events[0].delay = 1;
    world->source_actuator_events[0].local_multiple = 0x0980;
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
    expect_true(restored.party.champion_count == 3 &&
                restored.party.active_slot == 2 &&
                restored.party.leader_x == 17 &&
                restored.party.leader_y == 9 &&
                restored.party.leader_dir == 3 &&
                restored.party.levitating == 1 &&
                restored.party.door_state_override == 0x1357,
                "selected roster and party controls survive round-trip");
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
                restored.inventory_source[0][0].source_position == 2 &&
                restored.inventory_source[0][0].source_origin_valid == 1 &&
                restored.inventory_source[0][0].source_dungeon == 4 &&
                restored.inventory_source[0][0].source_level == 3 &&
                restored.inventory_source[0][0].source_x == 17 &&
                restored.inventory_source[0][0].source_y == 9 &&
                restored.inventory_source[0][0].text_ref == 0x0042 &&
                restored.inventory_source[0][0].property_valid &&
                restored.inventory_source[0][0].property[0] == 0x20 &&
                restored.inventory_source[0][0].property[5] == 0x0a &&
                restored.inventory_source[0][0].source_raw_size == 6 &&
                restored.inventory_source[0][0].source_raw[0] == 0xde &&
                restored.inventory_source[0][0].source_raw[5] == 0xad,
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
                restored.objects[0].source_origin_valid == 1 &&
                restored.objects[0].source_dungeon == 6 &&
                restored.objects[0].source_level == 4 &&
                restored.objects[0].source_x == 21 &&
                restored.objects[0].source_y == 11 &&
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
    expect_true(restored.creature_count == 1 &&
                restored.creatures[0].id == 77 &&
                restored.creatures[0].type == THERON_CREATURE_AKUTUBA &&
                restored.creatures[0].x == 11 && restored.creatures[0].y == 13 &&
                restored.creatures[0].hp == 88 &&
                restored.creatures[0].max_hp == 120 &&
                restored.creatures[0].ai == THERON_AI_GUARD &&
                restored.creatures[0].primary_attack == THERON_ATTACK_SLASH &&
                restored.creatures[0].source_ref == 0x3456 &&
                restored.creatures[0].source_chested == -2 &&
                restored.creatures[0].source_cell == 2 &&
                restored.creatures[0].source_slot == 1 &&
                restored.creatures[0].source_flags_word == 0x4567 &&
                restored.creatures[0].source_unknown_word == 0x89ab &&
                restored.creatures[0].source_spawn_category == 3 &&
                restored.creatures[0].source_raw_size == 16 &&
                restored.creatures[0].source_raw[0] == 0x80 &&
                restored.creatures[0].source_raw[15] == 0x8f,
                "live creature record and source identity survive round-trip");
    expect_true(restored.source_monster_count == 1 &&
                restored.source_monsters[0].dungeon_id ==
                    THERON_DUNGEON_3_FORMIC &&
                restored.source_monsters[0].level == 2 &&
                restored.source_monsters[0].x == 19 &&
                restored.source_monsters[0].y == 21 &&
                restored.source_monsters[0].source_ref == 0x0c81 &&
                restored.source_monsters[0].source_index == 0x0038 &&
                restored.source_monsters[0].chested == -2 &&
                restored.source_monsters[0].type == 0x0a &&
                restored.source_monsters[0].position == 1 &&
                restored.source_monsters[0].number == 1 &&
                restored.source_monsters[0].direction_flags == 4 &&
                restored.source_monsters[0].flags_word == 0x0420 &&
                restored.source_monsters[0].health[0] == 59 &&
                restored.source_monsters[0].health[1] == 37 &&
                restored.source_monsters[0].raw_size == 16 &&
                restored.source_monsters[0].raw[0] == 0xfe &&
                restored.source_monsters[0].raw[15] == 0x04,
                "authenticated category-4 source ledger survives round-trip");
    expect_true(restored.source_generator_count == 1 &&
                restored.source_generators[0].dungeon_id == THERON_DUNGEON_3_FORMIC &&
                restored.source_generators[0].level == 2 &&
                restored.source_generators[0].x == 17 &&
                restored.source_generators[0].y == 19 &&
                restored.source_generators[0].source_ref == 0x4567 &&
                restored.source_generators[0].source_index == 0x001a &&
                restored.source_generators[0].type == 6 &&
                restored.source_generators[0].value == 0x0203 &&
                restored.source_generators[0].effect == 0 &&
                restored.source_generators[0].revert_effect == 1 &&
                restored.source_generators[0].local_effect == 1 &&
                restored.source_generators[0].target_facing == 3 &&
                restored.source_generators[0].generator_fields_valid == 1 &&
                restored.source_generators[0].generator_generation == 4 &&
                restored.source_generators[0].generator_toughness == 0x12 &&
                restored.source_generators[0].generator_pause == 0x34 &&
                restored.generator_spawn_count[0] == 2 &&
                restored.generator_next_tick[0] == 0x0102030405060708ULL &&
                restored.generator_spawn_count[13] == 9 &&
                restored.generator_next_tick[13] == 0x1112131415161718ULL &&
                restored.generator_active_count == 14,
                "generator source record and runtime state survive round-trip");
    expect_true(restored.source_object_count == 1 &&
                restored.source_objects[0].dungeon_id ==
                    THERON_DUNGEON_3_FORMIC &&
                restored.source_objects[0].level == 2 &&
                restored.source_objects[0].x == 7 &&
                restored.source_objects[0].y == 9 &&
                restored.source_objects[0].source_ref == 0x1234 &&
                restored.source_objects[0].next_ref == 0x5678 &&
                restored.source_objects[0].source_index == 0x0042 &&
                restored.source_objects[0].category == THERON_CAT_WEAPON &&
                restored.source_objects[0].raw_size == 6 &&
                restored.source_objects[0].raw[0] == 0xde &&
                restored.source_objects[0].raw[5] == 0xad,
                "source object occurrence survives round-trip");
    expect_true(restored.source_actuator_event_count == 1 &&
                restored.source_actuator_events[0].due_tick == 114 &&
                restored.source_actuator_events[0].source_ref == 0x0c5d &&
                restored.source_actuator_events[0].source_index == 93 &&
                restored.source_actuator_events[0].dungeon_id == 3 &&
                restored.source_actuator_events[0].level == 2 &&
                restored.source_actuator_events[0].source_x == 5 &&
                restored.source_actuator_events[0].source_y == 0 &&
                restored.source_actuator_events[0].target_x == 6 &&
                restored.source_actuator_events[0].target_y == 5 &&
                restored.source_actuator_events[0].effect ==
                    TQ_ACT_EFFECT_SET &&
                restored.source_actuator_events[0].delay == 1 &&
                restored.source_actuator_events[0].local_multiple == 0x0980,
                "authentic queued actuator event survives round-trip");

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

static void test_corrupt_party_controls_are_rejected(void) {
    Theron_V1_World original;
    Theron_V1_World restored;
    size_t size;
    uint8_t *buffer;
    size_t control_offset;

    printf("  %-55s ", "World deserialize rejects corrupt party controls");
    fflush(stdout);
    seed_world(&original);
    size = theron_v1_world_serialize_size(&original);
    buffer = (uint8_t *)malloc(size);
    expect_true(buffer != NULL, "corrupt-party buffer allocated");
    if (!buffer) {
        printf("FAIL\n");
        return;
    }
    expect_true(theron_v1_world_serialize(&original, buffer, size) == size,
                "corrupt-party fixture serialized");
    control_offset = world_party_control_offset();
    put_le32(buffer + control_offset, THERON_MAX_CHAMPIONS + 1u);
    memset(&restored, 0, sizeof(restored));
    expect_true(theron_v1_world_deserialize(&restored, buffer, size) == -4,
                "champion count above capacity is rejected");
    expect_true(theron_v1_world_serialize(&original, buffer, size) == size,
                "invalid empty-party fixture reserialized");
    put_le32(buffer + control_offset, 0u);
    put_le32(buffer + control_offset + sizeof(uint32_t), 2u);
    expect_true(theron_v1_world_deserialize(&restored, buffer, size) == -4,
                "empty party rejects an active companion slot");
    free(buffer);
    puts(g_failures == 0 ? "PASS" : "FAIL");
}

static void test_version_13_party_pack_is_accepted(void) {
    Theron_V1_World original;
    Theron_V1_World restored;
    size_t size_v14;
    size_t size_v13;
    size_t controls;
    size_t objects;
    uint8_t *buffer_v14;
    uint8_t *buffer_v13;

    printf("  %-55s ", "World deserialize accepts version 13 party pack");
    fflush(stdout);
    seed_world(&original);
    size_v14 = theron_v1_world_serialize_size(&original);
    /* Version 13 predates the 22-byte party controls, the version-15 queued
     * actuator tail and versions 16/17's two empty sparse-state counts. */
    size_v13 = size_v14 - 22u - sizeof(uint32_t) - 27u -
               sizeof(uint32_t) * 2u - sizeof(uint32_t) - 55u;
    buffer_v14 = (uint8_t *)malloc(size_v14);
    buffer_v13 = (uint8_t *)malloc(size_v13);
    expect_true(buffer_v14 != NULL && buffer_v13 != NULL,
                "version-13 compatibility buffers allocated");
    if (!buffer_v14 || !buffer_v13) {
        free(buffer_v14);
        free(buffer_v13);
        printf("FAIL\n");
        return;
    }
    expect_true(theron_v1_world_serialize(
                    &original, buffer_v14, size_v14) == size_v14,
                "version-14 source stream serialized");
    controls = world_party_control_offset();
    objects = world_object_count_offset();
    memcpy(buffer_v13, buffer_v14, controls);
    memcpy(buffer_v13 + controls, buffer_v14 + objects,
           size_v14 - objects - sizeof(uint32_t) - 27u -
               sizeof(uint32_t) * 2u - sizeof(uint32_t) - 55u);
    put_le16(buffer_v13 + sizeof(uint32_t), 13u);

    memset(&restored, 0, sizeof(restored));
    restored.source_monster_count = 1u;
    restored.source_monsters[0].dungeon_id = THERON_DUNGEON_3_FORMIC;
    restored.source_monsters[0].level = 7;
    restored.source_monsters[0].source_ref = 0x7abcu;
    restored.source_monsters[0].source_index = 0x0021u;
    expect_true(theron_v1_world_deserialize(
                    &restored, buffer_v13, size_v13) == 0,
                "version-13 stream deserializes");
    expect_true(restored.party.champion_count == THERON_MAX_CHAMPIONS,
                "version-13 stream retains historical four-member contract");
    expect_true(restored.party.gold == original.party.gold &&
                restored.party.champions[1].inventory[3] == THERON_ITEM_SHIELD &&
                restored.object_count == original.object_count &&
                restored.objects[0].source_ref == original.objects[0].source_ref,
                "version-13 payload remains aligned after legacy party pack");
    expect_true(restored.source_monster_count == 1u &&
                restored.source_monsters[0].dungeon_id ==
                    THERON_DUNGEON_3_FORMIC &&
                restored.source_monsters[0].level == 7 &&
                restored.source_monsters[0].source_ref == 0x7abcu &&
                restored.source_monsters[0].source_index == 0x0021u,
                "legacy save retains the Track-02-authenticated destination ledger");
    free(buffer_v14);
    free(buffer_v13);
    puts(g_failures == 0 ? "PASS" : "FAIL");
}

static void test_world_hash_covers_party_state(void) {
    Theron_V1_World original;
    Theron_V1_World changed;
    uint64_t base;

    printf("  %-55s ", "World hash covers roster, pose and party controls");
    fflush(stdout);
    seed_world(&original);
    base = theron_v1_world_hash(&original);
    expect_true(base != 0u, "seeded world hash is non-zero");

#define EXPECT_PARTY_HASH_CHANGE(statement, message) do { \
        changed = original;                                \
        statement;                                         \
        expect_true(theron_v1_world_hash(&changed) != base, message); \
    } while (0)
    EXPECT_PARTY_HASH_CHANGE(changed.party.champion_count = 2,
                             "champion count changes world hash");
    EXPECT_PARTY_HASH_CHANGE(changed.party.active_slot = 1,
                             "active champion changes world hash");
    EXPECT_PARTY_HASH_CHANGE(changed.party.leader_x += 1,
                             "leader position changes world hash");
    EXPECT_PARTY_HASH_CHANGE(changed.party.leader_dir = 2,
                             "leader direction changes world hash");
    EXPECT_PARTY_HASH_CHANGE(changed.party.gold += 1u,
                             "party gold changes world hash");
    EXPECT_PARTY_HASH_CHANGE(changed.party.champions[0].health += 1,
                             "source-bound champion state changes world hash");
    EXPECT_PARTY_HASH_CHANGE(changed.party.levitating = 0,
                             "levitation changes world hash");
    EXPECT_PARTY_HASH_CHANGE(changed.party.door_state_override ^= 1,
                             "door override changes world hash");
#undef EXPECT_PARTY_HASH_CHANGE
    puts(g_failures == 0 ? "PASS" : "FAIL");
}

static void test_late_deserialize_failure_is_atomic(void) {
    Theron_V1_World original;
    Theron_V1_World restored;
    Theron_V1_World *saved;
    size_t size;
    uint8_t *buffer;

    printf("  %-55s ", "Late world deserialize failure is atomic");
    fflush(stdout);
    seed_world(&original);
    seed_world(&restored);
    restored.world_tick = 0x1122334455667788ULL;
    restored.party.gold ^= 0x55aa55aaU;
    size = theron_v1_world_serialize_size(&original);
    buffer = (uint8_t *)malloc(size);
    saved = (Theron_V1_World *)malloc(sizeof(*saved));
    expect_true(buffer != NULL && saved != NULL,
                "atomic-deserialize buffers allocated");
    if (!buffer || !saved) {
        free(buffer);
        free(saved);
        printf("FAIL\n");
        return;
    }
    expect_true(theron_v1_world_serialize(
                    &original, buffer, size) == size,
                "atomic-deserialize source world serialized");
    *saved = restored;
    expect_true(theron_v1_world_deserialize(
                    &restored, buffer, size - 1u) == -1,
                "one-byte-truncated late world tail is rejected");
    expect_true(memcmp(&restored, saved, sizeof(restored)) == 0,
                "late deserialize failure leaves live world byte-identical");
    expect_true(theron_v1_world_serialize(
                    &original, buffer, size) == size,
                "atomic source-ledger fixture reserialized");
    /* Version 18 ends with count + 55-byte category-4 rows.  Byte 22 in the
     * row is the decoded type; changing it without changing raw byte 4 must
     * fail the authenticated decoded/raw join. */
    buffer[size - 55u + 22u] ^= 1u;
    expect_true(theron_v1_world_deserialize(
                    &restored, buffer, size) == -1,
                "decoded/raw mismatch in saved category-4 ledger is rejected");
    expect_true(memcmp(&restored, saved, sizeof(restored)) == 0,
                "source-ledger rejection leaves live world byte-identical");
    free(buffer);
    free(saved);
    puts(g_failures == 0 ? "PASS" : "FAIL");
}

int main(void) {
    test_round_trip_keeps_purchase_state();
    test_corrupt_counts_are_rejected();
    test_corrupt_party_controls_are_rejected();
    test_version_13_party_pack_is_accepted();
    test_world_hash_covers_party_state();
    test_late_deserialize_failure_is_atomic();
    return g_failures == 0 ? 0 : 1;
}
