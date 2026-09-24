#include "asset_status_m12.h"
#include "theron_v1_startup_runtime_entry.h"
#include "theron_v1_track02.h"
#include "theron_v1_world.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *find_track02(const char *environment_name,
                                const char *filename) {
    const char *configured = getenv(environment_name);
    const char *home;
    static char standard_path[1024];
    if (configured && configured[0]) {
        FILE *f = fopen(configured, "rb");
        if (f) { fclose(f); return configured; }
        return NULL;
    }
    home = getenv("HOME");
    if (home && home[0] &&
        snprintf(standard_path, sizeof(standard_path),
                 "%s/.firestaff/data/theron/%s", home, filename) > 0) {
        FILE *f = fopen(standard_path, "rb");
        if (f) { fclose(f); return standard_path; }
    }
    return NULL;
}

static unsigned char *read_file(const char *path, size_t *out_size) {
    FILE *f;
    long end;
    unsigned char *bytes;
    if (out_size) *out_size = 0u;
    f = fopen(path, "rb");
    if (!f || fseek(f, 0, SEEK_END) != 0 || (end = ftell(f)) <= 0 ||
        fseek(f, 0, SEEK_SET) != 0) {
        if (f) fclose(f);
        return NULL;
    }
    bytes = (unsigned char *)malloc((size_t)end);
    if (!bytes || fread(bytes, 1u, (size_t)end, f) != (size_t)end) {
        free(bytes); fclose(f); return NULL;
    }
    fclose(f);
    if (out_size) *out_size = (size_t)end;
    return bytes;
}

static int verify_region(const char *region, const char *path,
                         const char *expected_md5) {
    static const unsigned int expected_map_counts[THERON_DUNGEON_COUNT] = {
        4u, 8u, 5u, 6u, 3u, 4u, 4u
    };
    unsigned char *track02;
    size_t track02_size;
    Theron_V1_World world;
    char receipt[256];
    char md5[33];
    Theron_DungeonID dungeon_id;
    unsigned int total_source_objects = 0u;
    unsigned int total_source_maps = 0u;

    if (!path) {
        printf("SKIP: authentic Theron %s Track 02 is not staged\n", region);
        return 77;
    }
    track02 = read_file(path, &track02_size);
    if (!track02) return 1;
    if (!m12_file_md5_hex(path, md5) ||
        strcmp(md5, expected_md5) != 0) {
        fprintf(stderr, "FAIL: %s Track 02 identity is not authentic: %s\n",
                region, md5);
        free(track02);
        return 1;
    }
    for (dungeon_id = THERON_DUNGEON_1_AKUTUBA;
         dungeon_id <= THERON_DUNGEON_7_DEMON;
         dungeon_id = (Theron_DungeonID)(dungeon_id + 1)) {
        const int slot = (int)dungeon_id - 1;
        unsigned int loaded_maps = 0u;
        theron_v1_world_init(&world);
        memset(receipt, 0, sizeof(receipt));
        if (!theron_v1_startup_runtime_load_source_dungeon(
                &world, track02, track02_size, md5, dungeon_id,
                receipt, sizeof(receipt)) ||
            world.current_dungeon != (int)dungeon_id || world.current_level != 0 ||
            !world.level_loaded[slot][0] ||
            world.levels[slot][0].width == 0 ||
            world.levels[slot][0].height == 0 ||
            world.source_object_count == 0u ||
            !strstr(receipt, "visual capture remains gated")) {
            fprintf(stderr,
                    "FAIL: authentic %s Track 02 source-dungeon handoff: %s (dungeon=%d level=%d loaded=%d size=%dx%d objects=%u)\n",
                    region, receipt, world.current_dungeon, world.current_level,
                    world.level_loaded[slot][0], world.levels[slot][0].width,
                    world.levels[slot][0].height, world.source_object_count);
            free(track02);
            return 1;
        }
        if (!world.source_thing_directory_verified[slot]) {
            fprintf(stderr,
                    "FAIL: %s dungeon %d thing directory is not source-verified\n",
                    region, (int)dungeon_id);
            free(track02);
            return 1;
        }
        for (unsigned int level = 0u;
             level < THERON_MAX_LEVELS_PER_DUNGEON; ++level) {
            const Theron_V1_Level *source_level = &world.levels[slot][level];
            if (!world.level_loaded[slot][level]) continue;
            if (!source_level->source_header_verified ||
                !source_level->source_item_property_table_verified ||
                source_level->width <= 0 || source_level->height <= 0 ||
                source_level->width > THERON_MAX_MAP_SIZE ||
                source_level->height > THERON_MAX_MAP_SIZE) {
                fprintf(stderr,
                        "FAIL: %s dungeon %d map %u lacks its authenticated source envelope\n",
                        region, (int)dungeon_id, level);
                free(track02);
                return 1;
            }
            ++loaded_maps;
        }
        if (loaded_maps != expected_map_counts[slot]) {
            fprintf(stderr,
                    "FAIL: %s dungeon %d loaded %u source maps, expected %u\n",
                    region, (int)dungeon_id, loaded_maps,
                    expected_map_counts[slot]);
            free(track02);
            return 1;
        }
        total_source_maps += loaded_maps;
        total_source_objects += world.source_object_count;
    }
    printf("PASS: authentic %s Track 02 binds all seven source dungeons (%u maps, %u source objects)\n",
           region, total_source_maps, total_source_objects);
    free(track02);
    return 0;
}

int main(void) {
    int jp_result = verify_region(
        "JP", find_track02("FIRESTAFF_THERON_JP_TRACK02", "TQJP02.bin"),
        THERON_TRACK02_MD5_JP_BIN);
    int us_result = verify_region(
        "US", find_track02("FIRESTAFF_THERON_US_TRACK02_BIN", "TQUS02.bin"),
        THERON_TRACK02_MD5_US_BIN);
    if (jp_result != 0 && jp_result != 77) return jp_result;
    if (us_result != 0 && us_result != 77) return us_result;
    return jp_result == 77 && us_result == 77 ? 77 : 0;
}
