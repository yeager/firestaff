#include "theron_v1_track02_dungeon_loader.h"
#include "theron_v1_track02_dungeon_map.h"
#include "theron_v1_track02_thing_data.h"
#include "theron_v1_track02_actuator.h"
#include "theron_v1_track02_creature_names.h"
#include "theron_v1_track02_creature_spawn.h"
#include "theron_v1_mechanics.h"
#include "theron_v1_world.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SECTOR_SIZE 2352
#define UD_PER_SECTOR 2048
#define SYNC_OFFSET 16

static uint8_t *load_track02_ud(const char *path, size_t *out_size) {
    FILE *fp = fopen(path, "rb");
    if (!fp) return NULL;
    fseek(fp, 0, SEEK_END);
    long fsize = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    if (fsize <= 0) { fclose(fp); return NULL; }
    uint8_t *raw = malloc((size_t)fsize);
    if (!raw) { fclose(fp); return NULL; }
    fread(raw, 1, (size_t)fsize, fp);
    fclose(fp);
    size_t sectors = (size_t)fsize / SECTOR_SIZE;
    size_t ud_size = sectors * UD_PER_SECTOR;
    uint8_t *ud = calloc(1, ud_size);
    if (!ud) { free(raw); return NULL; }
    for (size_t s = 0; s < sectors; s++)
        memcpy(ud + s * UD_PER_SECTOR, raw + s * SECTOR_SIZE + SYNC_OFFSET, UD_PER_SECTOR);
    free(raw);
    *out_size = ud_size;
    return ud;
}

static uint8_t *load_raw_bytes(const char *path, size_t *out_size) {
    FILE *fp;
    long file_size;
    uint8_t *raw;

    if (!path || !out_size) return NULL;
    fp = fopen(path, "rb");
    if (!fp) return NULL;
    if (fseek(fp, 0, SEEK_END) != 0 ||
        (file_size = ftell(fp)) <= 0 ||
        fseek(fp, 0, SEEK_SET) != 0) {
        fclose(fp);
        return NULL;
    }
    raw = (uint8_t *)malloc((size_t)file_size);
    if (!raw || fread(raw, 1u, (size_t)file_size, fp) != (size_t)file_size) {
        free(raw);
        fclose(fp);
        return NULL;
    }
    fclose(fp);
    *out_size = (size_t)file_size;
    return raw;
}

static void test_authenticated_world_spawn_binding(
    const char *us_path, const char *jp_path) {
    size_t raw_size = 0u;
    uint8_t *raw = load_raw_bytes(us_path, &raw_size);
    Theron_Track02SpawnSource source;
    Theron_V1_World world;

    assert(raw != NULL);
    memset(&source, 0, sizeof(source));
    assert(theron_v1_track02_decode_spawn_source(
               raw, raw_size, THERON_V1_TRACK02_VARIANT_US_BIN,
               &source) == 1);
    theron_v1_world_init(&world);
    assert(theron_v1_world_bind_track02_spawn_source(
               &world, &source, THERON_V1_TRACK02_VARIANT_US_BIN) == 1);
    assert(world.track02_spawn_source.authenticated == 1);
    assert(world.track02_spawn_source.variant ==
           THERON_V1_TRACK02_VARIANT_US_BIN);
    for (unsigned int i = 0u; i < THERON_TRACK02_SPAWN_ZONE_COUNT; ++i) {
        const Theron_SpawnZoneDesc *zone = theron_v1_track02_spawn_zone(i);
        assert(zone != NULL);
        assert(world.track02_spawn_source.zones[i].map_width == zone->map_width);
        assert(world.track02_spawn_source.zones[i].map_height == zone->map_height);
        assert(theron_v1_world_track02_spawn_category(&world, i) ==
               world.track02_spawn_source.zones[i].category);
    }
    assert(theron_v1_world_track02_spawn_category(
               &world, THERON_TRACK02_SPAWN_ZONE_COUNT) == 0xffu);
    free(raw);

    if (jp_path) {
        raw = load_raw_bytes(jp_path, &raw_size);
        assert(raw != NULL);
        memset(&source, 0, sizeof(source));
        assert(theron_v1_track02_decode_spawn_source(
                   raw, raw_size, THERON_V1_TRACK02_VARIANT_JP_BIN,
                   &source) == 0);
        theron_v1_world_init(&world);
        assert(theron_v1_world_bind_track02_spawn_source(
                   &world, NULL,
                   THERON_V1_TRACK02_VARIANT_JP_BIN) == 0);
        assert(world.track02_spawn_source.authenticated == 0);
        assert(world.track02_spawn_source_variant ==
               THERON_V1_TRACK02_VARIANT_JP_BIN);
        assert(theron_v1_world_track02_spawn_category(&world, 0u) == 0xffu);
        free(raw);
    }
    printf("  authenticated Track 02 spawn source reaches world binding\n");
}

static const char *find_track02(void) {
    const char *explicit_path = getenv("FIRESTAFF_THERON_TRACK02_RAW");
    const char *home = getenv("HOME");
    static char path[512];
    const char *candidates[3] = { explicit_path, NULL, NULL };
    if (home && home[0]) {
        snprintf(path, sizeof(path), "%s/.firestaff/data/theron/TQUS02.bin",
                 home);
        candidates[1] = path;
        snprintf(path + 256, sizeof(path) - 256,
                 "%s/.firestaff/data/theron/raw-us/"
                 "Dungeon Master - Theron's Quest (USA) (Track 02).bin",
                 home);
        candidates[2] = path + 256;
    }
    for (unsigned int i = 0; i < 3u; ++i) {
        FILE *fp;
        if (!candidates[i] || !candidates[i][0]) continue;
        fp = fopen(candidates[i], "rb");
        if (fp) { fclose(fp); return candidates[i]; }
    }
    return NULL;
}

static const char *find_jp_track02(void) {
    const char *explicit_path = getenv("FIRESTAFF_THERON_TRACK02_JP_RAW");
    const char *home = getenv("HOME");
    static char path[512];
    if (explicit_path && explicit_path[0]) return explicit_path;
    if (!home || !home[0]) return NULL;
    snprintf(path, sizeof(path), "%s/.firestaff/data/theron/TQJP02.bin",
             home);
    FILE *fp = fopen(path, "rb");
    if (!fp) return NULL;
    fclose(fp);
    return path;
}

static void assert_source_category_census(
    const Theron_DungeonLoadResult *result) {
    unsigned int total = 0;
    for (unsigned int category = 0; category < THERON_ITEM_CATEGORY_COUNT;
         ++category) {
        total += result->source_category_counts[category];
        if ((category > THERON_CAT_MISC && category < THERON_CAT_MISSILE) ||
            category > THERON_CAT_CLOUD)
            assert(result->source_category_counts[category] == 0);
    }
    assert(total == result->source_object_count);
}

static void assert_source_type_census(
    const Theron_DungeonLoadResult *result) {
    uint32_t expected[THERON_ITEM_CATEGORY_COUNT][8] = {{0}};
    for (unsigned int i = 0; i < result->source_object_count; ++i) {
        const Theron_Track02SourceObjectOccurrence *occ =
            &result->source_objects[i];
        assert(occ->source_index < THERON_MAX_ITEMS_PER_CAT);
        unsigned int type_value = 0;
        int has_type = 1;
        switch (occ->category) {
        case THERON_CAT_MONSTER:
            type_value = occ->decoded.value.monster.type;
            break;
        case THERON_CAT_WEAPON:
            type_value = occ->decoded.value.weapon.type;
            break;
        case THERON_CAT_CLOTHING:
            type_value = occ->decoded.value.clothing.type;
            break;
        case THERON_CAT_SCROLL:
            type_value = occ->decoded.value.scroll.type;
            break;
        case THERON_CAT_POTION:
            type_value = occ->decoded.value.potion.type;
            break;
        case THERON_CAT_MISC:
            type_value = occ->decoded.value.misc.type;
            break;
        default:
            has_type = 0;
            break;
        }
        if (has_type)
            expected[occ->category][type_value >> 5] |=
                1u << (type_value & 31u);
    }
    assert(memcmp(expected, result->source_type_value_mask,
                  sizeof(expected)) == 0);
}

static unsigned int expected_live_monsters(const Theron_V1_World *world) {
    unsigned int count = 0;
    for (unsigned int i = 0; i < world->source_monster_count; ++i) {
        const Theron_V1_SourceMonsterRecord *record =
            &world->source_monsters[i];
        if (record->dungeon_id != world->current_dungeon ||
            record->level != world->current_level) continue;
        if (record->type >= THERON_TRACK02_CREATURE_TYPE_COUNT) continue;
        unsigned int members = (unsigned int)record->number + 1u;
        if (members > 4u) members = 4u;
        for (unsigned int slot = 0; slot < members; ++slot)
            if (record->health[slot] != 0u) ++count;
    }
    return count;
}

static void assert_real_item_roundtrip(Theron_V1_World *world) {
    int found = 0;
    int initial_objects;

    /* Exercise one object from the loaded level, not a hand-built fixture.
     * T900 ownership remains source-gated: the item must carry its real
     * category, source reference, item type and verified 6-byte property row
     * through TAKE and DROP. */
    for (int i = 0; i < world->object_count; ++i) {
        Theron_V1_Object *object = &world->objects[i];
        int inventory_slot = -1;
        if (object->level != world->current_level ||
            !object->source_ref || !object->source_property_valid ||
            (object->source_category != THERON_CAT_WEAPON &&
             object->source_category != THERON_CAT_CLOTHING &&
             object->source_category != THERON_CAT_SCROLL &&
             object->source_category != THERON_CAT_POTION)) {
            continue;
        }
        /* The source loader can retain more than one thing at a square;
         * click routing consumes the first object in source order. */
        if (theron_v1_object_at(world, world->current_level,
                                object->x, object->y) != object) {
            continue;
        }
        initial_objects = world->object_count;
        assert(theron_v1_click_route(world, object->x, object->y,
                                     THERON_CMD_TAKE) == 0);
        for (int slot = 0; slot < THERON_INVENTORY_SLOTS; ++slot) {
            if (world->party.champions[world->party.active_slot]
                    .inventory[slot] == object->source_item_type &&
                world->inventory_source[world->party.active_slot][slot]
                    .valid) {
                inventory_slot = slot;
                break;
            }
        }
        assert(inventory_slot >= 0);
        assert(world->inventory_source[world->party.active_slot]
                   [inventory_slot].source_ref == object->source_ref);
        assert(theron_v1_drop_inventory_source_item(
                   world, world->party.active_slot, inventory_slot,
                   object->x, object->y) > 0);
        assert(world->object_count == initial_objects + 1);
        assert(world->objects[world->object_count - 1].source_ref ==
               object->source_ref);
        found = 1;
        break;
    }
    assert(found);
}

static void assert_real_chests_are_not_itemrecords(
    const Theron_V1_World *world, unsigned int expected_chests) {
    unsigned int chest_count = 0;

    for (int i = 0; i < world->object_count; ++i) {
        const Theron_V1_Object *object = &world->objects[i];
        if (object->source_category != THERON_CAT_CHEST) continue;
        ++chest_count;
        assert(object->type == THERON_OBJTYPE_CHEST);
        assert(object->source_property_valid == 0);
        assert(object->source_item_category == 0);
        assert(object->source_item_type == 0);
    }
    /* This is a real-data regression guard, not a synthetic chest fixture. */
    assert(chest_count == expected_chests);
}

static void test_all_dungeons(const uint8_t *ud, size_t ud_size) {
    const char *names[] = {
        "AKUTUBA","DRATOR","FORMICIA","SARMON","SHADODAN","THIEVES","DEMON"
    };
    /* Only generators reachable from real ground-reference chains are
     * placed in the world ledger; the remaining category-3 records are
     * source table entries without a map occurrence. */
    const unsigned int expected_source_generators[] = {3, 7, 9, 2, 1, 14, 10};

    for (int d = 0; d < 7; d++) {
        Theron_V1_World *world = calloc(1, sizeof(Theron_V1_World));
        assert(world);
        theron_v1_world_init(world);
        world->current_dungeon = d + 1;

        Theron_DungeonLoadResult result;
        int rc = theron_v1_track02_load_full_dungeon(
            world, d + 1, ud, ud_size, &result);
        assert(rc == 0);

        Theron_DungeonData source_maps;
        assert(theron_v1_track02_dungeon_map_load(
                   ud, ud_size, (unsigned int)d, &source_maps));
        assert(source_maps.map_count == (uint8_t)result.levels_loaded);
        assert(world->source_thing_directory_verified[d] == 1);
        assert(world->source_column_thing_count_total[d] ==
               source_maps.column_thing_count_total);
        assert(memcmp(world->source_thing_descriptor_sizes[d],
                      source_maps.thing_descriptor_sizes,
                      sizeof(source_maps.thing_descriptor_sizes)) == 0);
        for (unsigned int m = 0; m < source_maps.map_count; ++m) {
            const Theron_MapHeader *src = &source_maps.maps[m].header;
            const Theron_V1_Level *dst = &world->levels[d][m];
            assert(dst->source_header_verified == 1);
            assert(dst->source_header_level_index == src->map_id);
            assert(dst->source_map_x_offset == src->x_offset);
            assert(dst->source_map_y_offset == src->y_offset);
            assert(dst->source_header_unk1 == src->unk1);
            assert(dst->source_header_unk2 == src->unk2);
            assert(dst->source_xp_modifier == src->xp_modifier);
            assert(dst->source_door_type1 == src->door_type1);
            assert(dst->source_door_type2 == src->door_type2);
            assert(dst->source_creature_gfx_bank ==
                   source_maps.creature_gfx_bank[m]);
            assert(dst->source_cumulative_column_items ==
                   source_maps.cumulative_column_items[m]);
            assert(dst->creature_budget == src->creature_count);
        }

        printf("  %s: %d levels, %d things placed "
               "(%d doors, %d telep, %d act[%d fixed], %d source records, "
               "%d unbound, %d raw-only, %d materialized, %d properties) "
               "%d refs linked\n",
               names[d],
               result.levels_loaded,
               result.total_things_placed,
               result.doors_placed,
               result.teleporters_placed,
               result.actuators_placed,
               result.actuator_value_fixes,
               result.source_records_decoded,
               result.unbound_item_refs,
               result.raw_only_item_refs,
               result.source_objects_materialized,
               result.source_item_properties_bound,
               result.ground_refs_linked);

        printf("    source categories: monster=%u weapon=%u clothing=%u "
               "scroll=%u potion=%u chest=%u misc=%u missile=%u cloud=%u\n",
               result.source_category_counts[THERON_CAT_MONSTER],
               result.source_category_counts[THERON_CAT_WEAPON],
               result.source_category_counts[THERON_CAT_CLOTHING],
               result.source_category_counts[THERON_CAT_SCROLL],
               result.source_category_counts[THERON_CAT_POTION],
               result.source_category_counts[THERON_CAT_CHEST],
               result.source_category_counts[THERON_CAT_MISC],
               result.source_category_counts[THERON_CAT_MISSILE],
               result.source_category_counts[THERON_CAT_CLOUD]);

        assert(result.levels_loaded > 0);
        assert(result.total_things_placed > 0);
        assert(result.source_records_decoded > 0);
        assert(result.unbound_item_refs == result.source_records_decoded +
               result.raw_only_item_refs);
        assert(result.raw_only_item_refs == 0);
        assert(result.source_objects_materialized > 0);
        assert(result.source_item_properties_bound > 0);
        assert(result.source_property_table_verified == 1);
        assert(result.source_property_table_offset == 0x099825u);
        assert(result.source_text_data_count <= THERON_TRACK02_SOURCE_TEXT_MAX);
        if (d == 0)
            assert(result.source_text_data_count == 0x013C);
        assert(result.source_object_count ==
               (unsigned int)result.source_occurrences_decoded);
        assert(result.source_occurrences_decoded ==
               (int)result.source_object_count);
        assert(world->source_monster_count ==
               result.source_category_counts[THERON_CAT_MONSTER]);
        assert(world->source_generator_count == expected_source_generators[d]);
        /* Every decoded Track 02 ground-reference occurrence is retained in
         * the world provenance ledger; gameplay consumers remain separately
         * gated until their source semantics are authenticated. */
        assert(world->source_object_count ==
               (unsigned int)result.source_object_count);
        /* Static category-4 group records are now admitted to the live pool
         * for the current level.  This is source materialization, not the
         * still-gated random generator path. */
        assert((unsigned int)world->creature_count ==
               expected_live_monsters(world));
        for (int ci = 0; ci < world->creature_count; ++ci) {
            const Theron_V1_Creature *creature = &world->creatures[ci];
            const Theron_V1_SourceMonsterRecord *source = NULL;
            for (unsigned int si = 0; si < world->source_monster_count; ++si) {
                const Theron_V1_SourceMonsterRecord *candidate =
                    &world->source_monsters[si];
                if (candidate->source_ref == creature->source_ref &&
                    candidate->source_index == creature->source_index) {
                    source = candidate;
                    break;
                }
            }
            assert(creature->flags & THERON_CF_ACTIVE);
            assert(creature->source_ref != 0u);
            assert(source != NULL);
            assert(creature->type == (uint8_t)(source->type + 1u));
            assert(creature->source_chested == source->chested);
            assert(creature->type >= THERON_CREATURE_AKUTUBA &&
                   creature->type <= THERON_CREATURE_DEMON);
            assert(creature->source_cell ==
                   (uint8_t)((creature->source_position >>
                              (creature->source_slot * 2u)) & 0x03u));
            assert(creature->source_slot < 4u);
            assert(creature->source_group_count ==
                   (uint8_t)(source->number + 1u));
            assert(creature->hp == (int)source->health[creature->source_slot]);
            assert(creature->max_hp == creature->hp);
            assert(creature->attack == 0);
            assert(creature->defense == 0);
            assert(creature->speed == 0);
            assert(creature->hp == creature->max_hp);
            assert(creature->primary_attack == THERON_ATTACK_NONE);
            assert(creature->ai == THERON_AI_UNAVAILABLE);
            assert(creature->secondary_attack == THERON_ATTACK_NONE);
            /* This direct user-data loader has no authenticated spawn-source
             * receipt.  Static creature type must not be promoted into a
             * regular-spawn category; the authenticated binding test above
             * covers the real source-consumer path. */
            assert(creature->source_spawn_category == 0xffu);
        }
        theron_v1_world_init_generators(world);
        world->world_tick = 60;
        theron_v1_world_tick_generators(world);
        assert((unsigned int)world->creature_count ==
               expected_live_monsters(world));
        for (int i = 0; i < world->generator_active_count; ++i)
            assert(world->generator_spawn_count[i] == 0);
        assert_source_category_census(&result);
        assert_source_type_census(&result);
        for (unsigned int i = 0; i < result.source_object_count; ++i) {
            const Theron_Track02SourceObjectOccurrence *occ =
                &result.source_objects[i];
            assert(occ->category <= THERON_CAT_MISC ||
                   occ->category == THERON_CAT_MISSILE ||
                   occ->category == THERON_CAT_CLOUD);
            assert(occ->raw_size == theron_item_bytes[occ->category]);
            assert(occ->next_ref ==
                   ((uint16_t)occ->raw[0] | ((uint16_t)occ->raw[1] << 8)));
            assert(occ->decoded_valid);
            assert(occ->decoded.category == occ->category);
            assert(occ->decoded.next_ref == occ->next_ref);
        }
        for (unsigned int i = 0; i < world->source_monster_count; ++i) {
            const Theron_V1_SourceMonsterRecord *monster =
                &world->source_monsters[i];
            const Theron_Track02SourceObjectOccurrence *source = NULL;
            assert(monster->dungeon_id == d + 1);
            assert(monster->level >= 0 &&
                   monster->level < THERON_MAX_LEVELS_PER_DUNGEON);
            assert(monster->x < THERON_MAX_MAP_SIZE);
            assert(monster->y < THERON_MAX_MAP_SIZE);
            for (unsigned int j = 0; j < result.source_object_count; ++j) {
                const Theron_Track02SourceObjectOccurrence *candidate =
                    &result.source_objects[j];
                if (candidate->category == THERON_CAT_MONSTER &&
                    candidate->source_ref == monster->source_ref &&
                    candidate->source_index == monster->source_index) {
                    source = candidate;
                    break;
                }
            }
            assert(source != NULL);
            assert(monster->level == (int)source->map);
            assert(monster->x == (int)source->x);
            assert(monster->y == (int)source->y);
            assert(monster->type == source->decoded.value.monster.type);
            assert(monster->position == source->decoded.value.monster.position);
            assert(monster->number == source->decoded.value.monster.number);
            assert(monster->direction_flags ==
                   source->decoded.value.monster.direction_flags);
            assert(memcmp(monster->health, source->decoded.value.monster.health,
                          sizeof(monster->health)) == 0);
        }
        for (unsigned int i = 0; i < world->source_generator_count; ++i) {
            const Theron_V1_SourceGeneratorRecord *generator =
                &world->source_generators[i];
            assert(generator->dungeon_id == d + 1);
            assert(generator->level >= 0 &&
                   generator->level < THERON_MAX_LEVELS_PER_DUNGEON);
            assert(generator->x < THERON_MAX_MAP_SIZE);
            assert(generator->y < THERON_MAX_MAP_SIZE);
            assert(generator->type == TQ_ACT_FLOOR_MONSTER_GEN);
        }
        for (unsigned int i = 0; i < world->source_object_count; ++i) {
            const Theron_V1_SourceObjectRecord *object =
                &world->source_objects[i];
            const Theron_Track02SourceObjectOccurrence *source = NULL;
            for (unsigned int j = 0; j < result.source_object_count; ++j) {
                const Theron_Track02SourceObjectOccurrence *candidate =
                    &result.source_objects[j];
                if (candidate->source_ref == object->source_ref &&
                    candidate->source_index == object->source_index &&
                    candidate->category == object->category) {
                    source = candidate;
                    break;
                }
            }
            assert(source != NULL);
            assert(object->dungeon_id == d + 1);
            assert(object->level == (int)source->map);
            assert(object->x == (int)source->x);
            assert(object->y == (int)source->y);
            assert(object->source_ref == source->source_ref);
            assert(object->next_ref == source->next_ref);
            assert(object->source_index == source->source_index);
            assert(object->category == source->category);
            assert(object->position == source->position);
            assert(object->raw_size == source->raw_size);
            assert(memcmp(object->raw, source->raw, object->raw_size) == 0);
        }
        /* Champions and creatures are linked through actuators (champion
         * mirror type 127), not directly through ground refs. */
        assert(world->object_count == result.total_things_placed);
        if (d == 0)
            assert_real_item_roundtrip(world);
            assert_real_chests_are_not_itemrecords(
                world, result.source_category_counts[THERON_CAT_CHEST]);

        free(world);
    }
}

static void test_all_jp_dungeons(const uint8_t *ud, size_t ud_size) {
    for (int d = 0; d < 7; d++) {
        Theron_V1_World *world = calloc(1, sizeof(Theron_V1_World));
        assert(world);
        theron_v1_world_init(world);
        world->current_dungeon = d + 1;

        Theron_DungeonLoadResult result;
        assert(theron_v1_track02_load_full_dungeon_for_variant(
                   world, d + 1, ud, ud_size,
                   THERON_TRACK02_VARIANT_JP_BIN, &result) == 0);
        assert(result.levels_loaded > 0);
        assert(result.source_records_decoded > 0);
        assert(result.raw_only_item_refs == 0);
        assert(result.source_item_properties_bound > 0);
        assert(result.source_property_table_verified == 1);
        assert(result.source_property_table_offset == 0x0990a2u ||
               result.source_property_table_offset == 0x119d4du ||
               result.source_property_table_offset == 0x15955du ||
               result.source_property_table_offset == 0x1d91d9u ||
               result.source_property_table_offset == 0x219b13u);
        assert(result.source_text_data_count == 0);
        assert(result.source_object_count ==
               (unsigned int)result.source_occurrences_decoded);
        assert_source_category_census(&result);
        assert_source_type_census(&result);
        /* JP category-4 records must reach the same source-bound live pool as
         * US records.  This is static group materialization only; the
         * separate random-generator consumer remains fail-closed. */
        assert((unsigned int)world->creature_count ==
               expected_live_monsters(world));
        for (int ci = 0; ci < world->creature_count; ++ci) {
            const Theron_V1_Creature *creature = &world->creatures[ci];
            assert(creature->flags & THERON_CF_ACTIVE);
            assert(creature->source_ref != 0u);
            assert(creature->type >= THERON_CREATURE_AKUTUBA &&
                   creature->type <= THERON_CREATURE_DEMON);
            assert(creature->hp == creature->max_hp);
            assert(creature->primary_attack == THERON_ATTACK_NONE);
            assert(creature->ai == THERON_AI_UNAVAILABLE);
            assert(creature->secondary_attack == THERON_ATTACK_NONE);
        }
        if (d == 0)
            assert_real_item_roundtrip(world);
            assert_real_chests_are_not_itemrecords(
                world, result.source_category_counts[THERON_CAT_CHEST]);
        free(world);
    }
    printf("  JP Track 02: all dungeon object records OK\n");
}

static void test_real_bank_reload_clears_stale_levels(
    const uint8_t *ud, size_t ud_size) {
    Theron_V1_World *world = calloc(1, sizeof(Theron_V1_World));
    Theron_DungeonData long_bank;
    Theron_DungeonData short_bank;

    assert(world);
    theron_v1_world_init(world);
    assert(theron_v1_track02_dungeon_map_load_for_variant(
               ud, ud_size, THERON_TRACK02_VARIANT_US_BIN, 1u,
               &long_bank));
    assert(long_bank.map_count == 8u);
    assert(theron_v1_world_load_track02_dungeon(world, 2, &long_bank) == 8);
    assert(world->level_loaded[1][7] == 1);

    assert(theron_v1_track02_dungeon_map_load_for_variant(
               ud, ud_size, THERON_TRACK02_VARIANT_US_BIN, 4u,
               &short_bank));
    assert(short_bank.map_count == 3u);
    assert(theron_v1_world_load_track02_dungeon(world, 2, &short_bank) == 3);
    for (unsigned int level = 0u; level < 3u; ++level)
        assert(world->level_loaded[1][level] == 1);
    for (unsigned int level = 3u; level < THERON_MAX_LEVELS_PER_DUNGEON;
         ++level)
        assert(world->level_loaded[1][level] == 0);

    free(world);
    printf("  US Track 02: real bank reload clears stale level records\n");
}

static unsigned int count_objects_in_dungeon(
    const Theron_V1_World *world, int dungeon_id) {
    unsigned int count = 0;
    for (int i = 0; i < world->object_count; ++i)
        if (world->objects[i].dungeon_id == dungeon_id) ++count;
    return count;
}

static void assert_object_ids_unique(const Theron_V1_World *world) {
    for (int i = 0; i < world->object_count; ++i) {
        assert(world->objects[i].id > 0);
        for (int j = i + 1; j < world->object_count; ++j)
            assert(world->objects[i].id != world->objects[j].id);
    }
}

static void test_real_source_ledgers_survive_other_dungeon_reload(
    const uint8_t *ud, size_t ud_size) {
    Theron_V1_World *world = calloc(1, sizeof(Theron_V1_World));
    Theron_DungeonLoadResult first;
    Theron_DungeonLoadResult second;
    Theron_DungeonLoadResult reloaded;
    unsigned int first_monsters;
    unsigned int first_generators;
    unsigned int first_source_objects;
    unsigned int first_placed_objects;
    unsigned int second_monsters;
    unsigned int second_generators;
    unsigned int second_source_objects;

    assert(world);
    theron_v1_world_init(world);
    world->current_dungeon = 1;
    assert(theron_v1_track02_load_full_dungeon_for_variant(
               world, 1, ud, ud_size, THERON_TRACK02_VARIANT_US_BIN,
               &first) == 0);
    first_monsters = world->source_monster_count;
    first_generators = world->source_generator_count;
    first_source_objects = world->source_object_count;
    first_placed_objects = count_objects_in_dungeon(world, 1);
    assert(first_monsters > 0);
    assert(first_generators > 0);
    assert(first_source_objects > 0);
    assert(first_placed_objects == (unsigned int)first.total_things_placed);

    /* Loading dungeon 2 must replace only dungeon 2's records.  Dungeon 1
     * remains source-authenticated so a later Continue/return route does not
     * silently lose its monster, generator, or object consumers. */
    world->current_dungeon = 2;
    world->current_level = 0;
    assert(theron_v1_track02_load_full_dungeon_for_variant(
               world, 2, ud, ud_size, THERON_TRACK02_VARIANT_US_BIN,
               &second) == 0);
    assert(world->source_monster_count > first_monsters);
    assert(world->source_generator_count > first_generators);
    assert(world->source_object_count > first_source_objects);
    assert(count_objects_in_dungeon(world, 1) == first_placed_objects);
    assert(count_objects_in_dungeon(world, 2) ==
           (unsigned int)second.total_things_placed);
    second_monsters = world->source_monster_count - first_monsters;
    second_generators = world->source_generator_count - first_generators;
    second_source_objects = world->source_object_count - first_source_objects;
    assert(second_monsters == second.source_category_counts[THERON_CAT_MONSTER]);
    assert(second_generators > 0);
    assert(second_source_objects == (unsigned int)second.source_object_count);
    for (unsigned int i = 0; i < world->source_monster_count; ++i)
        assert(world->source_monsters[i].dungeon_id == 1 ||
               world->source_monsters[i].dungeon_id == 2);
    for (unsigned int i = 0; i < world->source_generator_count; ++i)
        assert(world->source_generators[i].dungeon_id == 1 ||
               world->source_generators[i].dungeon_id == 2);
    for (unsigned int i = 0; i < world->source_object_count; ++i)
        assert(world->source_objects[i].dungeon_id == 1 ||
               world->source_objects[i].dungeon_id == 2);
    assert_object_ids_unique(world);

    /* A second load of the same dungeon must not duplicate its records or
     * objects, while the untouched dungeon-1 ledger remains unchanged. */
    assert(theron_v1_track02_load_full_dungeon_for_variant(
               world, 2, ud, ud_size, THERON_TRACK02_VARIANT_US_BIN,
               &reloaded) == 0);
    assert(world->source_monster_count == first_monsters + second_monsters);
    assert(world->source_generator_count == first_generators + second_generators);
    assert(world->source_object_count == first_source_objects +
           second_source_objects);
    assert(count_objects_in_dungeon(world, 1) == first_placed_objects);
    assert(count_objects_in_dungeon(world, 2) ==
           (unsigned int)reloaded.total_things_placed);
    assert_object_ids_unique(world);

    free(world);
    printf("  US Track 02: dungeon-scoped source ledgers survive reload\n");
}

static void test_real_campaign_source_capacity(
    const uint8_t *ud, size_t ud_size) {
    static const unsigned int expected_generators[THERON_DUNGEON_COUNT] =
        {3u, 7u, 9u, 2u, 1u, 14u, 10u};
    Theron_V1_World *world = calloc(1, sizeof(Theron_V1_World));
    unsigned int expected_monsters = 0;
    unsigned int expected_generators_total = 0;
    unsigned int expected_source_objects = 0;
    unsigned int expected_placed_objects = 0;

    assert(world);
    theron_v1_world_init(world);
    for (int dungeon = 1; dungeon <= THERON_DUNGEON_COUNT; ++dungeon) {
        Theron_DungeonLoadResult result;
        unsigned int previous_monsters = world->source_monster_count;
        unsigned int previous_generators = world->source_generator_count;
        unsigned int previous_source_objects = world->source_object_count;

        world->current_dungeon = dungeon;
        world->current_level = 0;
        assert(theron_v1_track02_load_full_dungeon_for_variant(
                   world, dungeon, ud, ud_size,
                   THERON_TRACK02_VARIANT_US_BIN, &result) == 0);
        assert(world->source_monster_count > previous_monsters);
        assert(world->source_generator_count > previous_generators);
        assert(world->source_object_count > previous_source_objects);
        assert(world->source_monster_count - previous_monsters ==
               result.source_category_counts[THERON_CAT_MONSTER]);
        assert(world->source_generator_count - previous_generators ==
               expected_generators[dungeon - 1]);
        assert(world->source_object_count - previous_source_objects ==
               (unsigned int)result.source_object_count);
        assert(count_objects_in_dungeon(world, dungeon) ==
               (unsigned int)result.total_things_placed);

        expected_monsters += result.source_category_counts[THERON_CAT_MONSTER];
        expected_generators_total += expected_generators[dungeon - 1];
        expected_source_objects += (unsigned int)result.source_object_count;
        expected_placed_objects += (unsigned int)result.total_things_placed;
    }

    assert(world->source_monster_count == expected_monsters);
    assert(world->source_generator_count == expected_generators_total);
    assert(world->source_object_count == expected_source_objects);
    assert(world->object_count == (int)expected_placed_objects);
    assert(expected_monsters == 165u);
    assert(expected_generators_total == 46u);
    /* Category-4 monster records do not carry a linked-list next_ref: their
     * first word is signed `chested`.  Stop each monster chain there; the
     * authentic US campaign then has three fewer false follow-on records. */
    assert(expected_source_objects == 2266u);
    assert(expected_placed_objects == 2186u);
    assert_object_ids_unique(world);

    free(world);
    printf("  US Track 02: all seven dungeon ledgers fit and remain distinct\n");
}

static void test_generator_binding_rejects_non_source_records(void) {
    Theron_V1_World world;
    memset(&world, 0, sizeof(world));
    world.level_loaded[0][0] = 1;
    world.levels[0][0].source_header_verified = 1;

    assert(theron_v1_world_bind_track02_generator(
               &world, 1, 0, 1, 2, 3, 4, TQ_ACT_FLOOR_MONSTER, 5,
               0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) == -1);
    assert(world.source_generator_count == 0);
    assert(theron_v1_world_bind_track02_generator(
               &world, 1, 0, 1, 2, THERON_MAX_MAP_SIZE, 4,
               TQ_ACT_FLOOR_MONSTER_GEN, 5, 0, 0, 0, 0, 0, 0, 0, 0,
               0, 0, 0, 0, 0) == -1);
    assert(world.source_generator_count == 0);
    assert(theron_v1_world_bind_track02_generator(
               &world, 1, 0, 1, 2, 3, 4, TQ_ACT_FLOOR_MONSTER_GEN, 5,
               0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) == 0);
    assert(world.source_generator_count == 1);
}

static void test_object_binding_rejects_unverified_locations(void) {
    Theron_V1_World world;
    const uint8_t raw[2] = {0xFF, 0xFF};
    memset(&world, 0, sizeof(world));

    assert(theron_v1_world_bind_track02_source_object(
               &world, 1, 0, 1, 2, 3, THERON_CAT_WEAPON, 0, 3, 4,
               raw, sizeof(raw)) == -1);
    assert(world.source_object_count == 0);
    world.level_loaded[0][0] = 1;
    world.levels[0][0].source_header_verified = 1;
    assert(theron_v1_world_bind_track02_source_object(
               &world, 1, 0, 1, 2, 3, THERON_CAT_WEAPON, 0,
               THERON_MAX_MAP_SIZE, 4, raw, sizeof(raw)) == -1);
    assert(world.source_object_count == 0);
    assert(theron_v1_world_bind_track02_source_object(
               &world, 1, 0, 1, 2, 3, THERON_CAT_WEAPON, 0, 3, 4,
               raw, sizeof(raw)) == 0);
    assert(world.source_object_count == 1);
}

static void test_authentic_coordinate_teleporter_without_endpoint(
    const uint8_t *ud, size_t ud_size) {
    Theron_V1_World world;
    Theron_DungeonLoadResult result;

    theron_v1_world_init(&world);
    world.current_dungeon = 1;
    world.current_level = 0;
    assert(theron_v1_track02_load_full_dungeon(
               &world, 1, ud, ud_size, &result) == 0);

    assert(theron_v1_object_at_in_dungeon(
               &world, 1, 0, 0, 0)->type == THERON_OBJTYPE_TELEPORTER);

    /* Authentic US AKUTUBA M0 teleporter record 0 is at (0,0) and points to
     * (2,3) on M0.  (2,3) is a real floor square without a second object
     * record, so requiring an endpoint object would reject source data. */
    assert(theron_v1_teleporter_resolve(&world, 0, 0) == 0);
    assert(world.transition_pending == 1);
    assert(world.transition_target_level == 0);
    assert(world.transition_spawn_x == 2);
    assert(world.transition_spawn_y == 3);
    assert(theron_v1_transition_execute(&world) == 0);
    assert(world.current_level == 0);
    assert(world.party.leader_x == 2);
    assert(world.party.leader_y == 3);
    printf("  authentic Track 02 coordinate teleporter lands on floor OK\n");
}

int main(void) {
    printf("test_theron_v1_track02_dungeon_loader\n");

    test_generator_binding_rejects_non_source_records();
    test_object_binding_rejects_unverified_locations();

    const char *path = find_track02();
    if (!path) {
        printf("  SKIP: Track 02 BIN not found\n");
        return 0;
    }
    size_t ud_size = 0;
    uint8_t *ud = load_track02_ud(path, &ud_size);
    if (!ud) {
        printf("  SKIP: could not load Track 02\n");
        return 0;
    }
    test_all_dungeons(ud, ud_size);
    test_authentic_coordinate_teleporter_without_endpoint(ud, ud_size);
    test_real_bank_reload_clears_stale_levels(ud, ud_size);
    test_real_source_ledgers_survive_other_dungeon_reload(ud, ud_size);
    test_real_campaign_source_capacity(ud, ud_size);

    const char *jp_path = find_jp_track02();
    test_authenticated_world_spawn_binding(path, jp_path);
    free(ud);
    if (jp_path) {
        size_t jp_ud_size = 0;
        uint8_t *jp_ud = load_track02_ud(jp_path, &jp_ud_size);
        if (jp_ud) {
            test_all_jp_dungeons(jp_ud, jp_ud_size);
            free(jp_ud);
        }
    } else {
        printf("  SKIP: Japanese Track 02 BIN not found\n");
    }
    printf("PASS\n");
    return 0;
}
