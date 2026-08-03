#include "theron_v1_track02_dungeon_loader.h"
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
    const char *home = getenv("HOME");
    if (!home) return NULL;
    static char path[512];
    snprintf(path, sizeof(path),
        "%s/.firestaff/data/theron/raw-us/"
        "Dungeon Master - Theron's Quest (USA) (Track 02).bin", home);
    FILE *fp = fopen(path, "rb");
    if (fp) { fclose(fp); return path; }
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

        printf("  %s: %d levels, %d things placed "
               "(%d doors, %d telep, %d act[%d fixed], %d cret, %d chmp, %d items) "
               "%d refs linked\n",
               names[d],
               result.levels_loaded,
               result.total_things_placed,
               result.doors_placed,
               result.teleporters_placed,
               result.actuators_placed,
               result.actuator_value_fixes,
               result.creatures_placed,
               result.champions_placed,
               result.items_placed,
               result.ground_refs_linked);

        assert(result.levels_loaded > 0);
        assert(result.total_things_placed > 0);
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
