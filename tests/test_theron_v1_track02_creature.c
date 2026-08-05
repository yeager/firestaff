#include "theron_v1_track02_creature.h"
#include "theron_v1_track02_dungeon_map.h"
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
        snprintf(path, sizeof(path), "%s/.firestaff/data/theron/TQUS02.bin", home);
        candidates[1] = path;
        snprintf(path + 256, sizeof(path) - 256,
                 "%s/.firestaff/data/theron/raw-us/"
                 "Dungeon Master - Theron's Quest (USA) (Track 02).bin", home);
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

static const unsigned int expected_creature_totals[7] = { 5, 10, 6, 5, 3, 6, 4 };

static void test_creature_counts(const uint8_t *ud, size_t ud_size) {
    const char *names[] = {
        "AKUTUBA","DRATOR","FORMICIA","SARMON","SHADODAN","THIEVES","DEMON"
    };

    for (unsigned int d = 0; d < 7; d++) {
        Theron_DungeonData dd;
        assert(theron_v1_track02_dungeon_map_load(ud, ud_size, d, &dd));

        unsigned int total_creatures = 0;
        for (unsigned int m = 0; m < dd.map_count; m++)
            total_creatures += dd.maps[m].header.creature_count;

        assert(total_creatures == expected_creature_totals[d]);

        const Theron_DungeonCreatureTypes *ct = &theron_creature_types[d];
        assert(ct->count >= 2 && ct->count <= 3);

        printf("  %s: %u creatures across %u levels, types:",
            names[d], total_creatures, dd.map_count);
        for (unsigned int t = 0; t < ct->count; t++)
            printf(" %u", ct->types[t]);
        printf("\n");
    }
}

int main(void) {
    printf("test_theron_v1_track02_creature\n");

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
    test_creature_counts(ud, ud_size);
    free(ud);

    /* Generator table validation */
    {
        assert(theron_dungeon_generators[0].count == 1); /* AKUTUBA */
        assert(theron_dungeon_generators[0].gens[0].creature_type == 14); /* Mummy */
        assert(theron_dungeon_generators[1].count == 2); /* DRATOR: Skeleton ×2 */
        assert(theron_dungeon_generators[1].gens[0].creature_type == 11);
        assert(theron_dungeon_generators[2].count == 3); /* FORMICIA: Trolin ×3 */
        assert(theron_dungeon_generators[2].gens[0].creature_type == 17);
        assert(theron_dungeon_generators[3].count == 1); /* SARMON: Ghost */
        assert(theron_dungeon_generators[3].gens[0].creature_type == 7);
        assert(theron_dungeon_generators[4].count == 0); /* SHADODAN: none */
        assert(theron_dungeon_generators[5].count == 0); /* THIEVES: none */
        assert(theron_dungeon_generators[6].count == 5); /* DEMON: BlackFlame ×5 */
        assert(theron_dungeon_generators[6].gens[0].creature_type == 15);
        printf("  Generator table: OK\n");
    }

    printf("PASS\n");
    return 0;
}
