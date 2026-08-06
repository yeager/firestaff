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

        assert(result.levels_loaded > 0);
        assert(result.total_things_placed > 0);
        assert(result.source_records_decoded > 0);
        assert(result.unbound_item_refs == result.source_records_decoded +
               result.raw_only_item_refs);
        assert(result.raw_only_item_refs == 0);
        assert(result.source_object_count ==
               (unsigned int)result.unbound_item_refs);
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
        /* Champions and creatures are linked through actuators (champion
         * mirror type 127), not directly through ground refs. */
        assert(world->object_count == result.total_things_placed);

        free(world);
    }
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
    printf("PASS\n");
    return 0;
}
