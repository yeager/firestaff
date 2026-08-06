#include "theron_v1_track02_dungeon_loader.h"
#include "theron_v1_track02_dungeon_map.h"
#include "theron_v1_track02_thing_data.h"
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
        if (category < THERON_CAT_MONSTER ||
            (category > THERON_CAT_MISC && category < THERON_CAT_MISSILE) ||
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

static void test_all_dungeons(const uint8_t *ud, size_t ud_size) {
    const char *names[] = {
        "AKUTUBA","DRATOR","FORMICIA","SARMON","SHADODAN","THIEVES","DEMON"
    };

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
               "%d unbound, %d raw-only) %d refs linked\n",
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
        assert(result.source_object_count ==
               (unsigned int)result.unbound_item_refs);
        assert(world->source_monster_count ==
               result.source_category_counts[THERON_CAT_MONSTER]);
        assert(world->creature_count == 0);
        assert_source_category_census(&result);
        assert_source_type_census(&result);
        for (unsigned int i = 0; i < result.source_object_count; ++i) {
            const Theron_Track02SourceObjectOccurrence *occ =
                &result.source_objects[i];
            assert(occ->category == THERON_CAT_MONSTER ||
                   (occ->category >= THERON_CAT_WEAPON &&
                    occ->category <= THERON_CAT_MISC) ||
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
        /* Champions and creatures are linked through actuators (champion
         * mirror type 127), not directly through ground refs. */
        assert(world->object_count == result.total_things_placed);

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
        assert(result.source_object_count ==
               (unsigned int)result.unbound_item_refs);
        assert_source_category_census(&result);
        assert_source_type_census(&result);
        free(world);
    }
    printf("  JP Track 02: all dungeon object records OK\n");
}

int main(void) {
    printf("test_theron_v1_track02_dungeon_loader\n");

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
    free(ud);

    const char *jp_path = find_jp_track02();
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
