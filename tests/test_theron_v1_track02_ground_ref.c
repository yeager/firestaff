#include "theron_v1_track02_ground_ref.h"
#include "theron_v1_track02_dungeon_map.h"
#include "theron_v1_track02_thing_data.h"
#include "theron_v1_track02_actuator.h"
#include "theron_v1_track02_item_id_map.h"
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

static void test_ref_encode_decode(void) {
    uint16_t ref = theron_ref_make(3, 1, 42);
    assert(theron_ref_id(ref) == 42);
    assert(theron_ref_category(ref) == 3);
    assert(theron_ref_position(ref) == 1);
    assert(!theron_ref_is_end(ref));
    assert(theron_ref_is_end(THERON_REF_NONE));
    assert(theron_ref_is_end(THERON_REF_UNUSED));
    printf("  Ref encode/decode OK\n");
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

static const uint8_t *get_item_record(const Theron_ThingData *td,
                                       unsigned int cat, unsigned int id) {
    if (cat >= 16 || id >= td->object_counts[cat]) return NULL;
    size_t item_size = theron_item_bytes[cat];
    if (item_size == 0) return NULL;
    return &td->items[cat][id * item_size];
}

static uint16_t get_item_next_ref(const Theron_ThingData *td,
                                   unsigned int cat, unsigned int id) {
    const uint8_t *rec = get_item_record(td, cat, id);
    if (!rec) return THERON_REF_NONE;
    return (uint16_t)rec[0] | ((uint16_t)rec[1] << 8);
}

static void test_all_dungeons(const uint8_t *ud, size_t ud_size) {
    const char *names[] = {
        "AKUTUBA","DRATOR","FORMICIA","SARMON","SHADODAN","THIEVES","DEMON"
    };

    for (unsigned int d = 0; d < 7; d++) {
        Theron_DungeonData dd;
        assert(theron_v1_track02_dungeon_map_load(ud, ud_size, d, &dd));

        unsigned int total_tiles = 0;
        uint8_t flat_tiles[8192];
        unsigned int fp2 = 0;
        for (unsigned int m = 0; m < dd.map_count; m++) {
            unsigned int w = dd.maps[m].header.x_dim + 1u;
            unsigned int h = dd.maps[m].header.y_dim + 1u;
            total_tiles += w * h;
            for (unsigned int x = 0; x < w; x++)
                for (unsigned int y = 0; y < h; y++)
                    flat_tiles[fp2++] = dd.maps[m].tiles[x][y];
        }

        unsigned int gref_count =
            theron_v1_track02_compute_ground_ref_count(flat_tiles, total_tiles);
        Theron_ThingData *td = calloc(1, sizeof(Theron_ThingData));
        assert(td);
        assert(theron_v1_track02_thing_data_load(
            ud, ud_size, d, dd.object_counts, gref_count, td));

        /* Walk all ground refs and follow item chains */
        unsigned int total_chain_items = 0;
        unsigned int max_chain_depth = 0;
        unsigned int cat_visit_counts[16] = {0};

        for (unsigned int g = 0; g < gref_count; g++) {
            uint16_t ref = td->ground_refs[g];
            unsigned int depth = 0;
            while (!theron_ref_is_end(ref) && depth < 100) {
                unsigned int cat = theron_ref_category(ref);
                unsigned int id = theron_ref_id(ref);
                assert(cat < 16);
                assert(id < td->object_counts[cat]);
                cat_visit_counts[cat]++;
                total_chain_items++;
                depth++;
                ref = get_item_next_ref(td, cat, id);
            }
            if (depth > max_chain_depth)
                max_chain_depth = depth;
        }

        /* Verify actuator value fix translation */
        unsigned int fixed = 0;
        for (unsigned int i = 0; i < dd.object_counts[3]; i++) {
            Theron_Actuator act;
            assert(theron_v1_track02_actuator_decode(
                &td->items[3][i * 8], &act) == 0);

            int is_wall = (act.type >= 1 && act.type <= 11);
            if (theron_v1_track02_actuator_needs_value_fix(act.type, is_wall)) {
                uint8_t orig = (uint8_t)(act.value & 0xFF);
                uint8_t translated = theron_v1_track02_translate_item_id(orig);
                if (translated != orig) fixed++;
            }
        }

        printf("  %s: %u ground_refs → %u chain items (max depth %u), %u actuator value fixes\n",
               names[d], gref_count, total_chain_items, max_chain_depth, fixed);
        printf("    cats:");
        const char *cat_names[] = {
            "door","telep","text","act","monster","weapon","clothing",
            "scroll","potion","chest","misc","unused","unused","unused",
            "missile","cloud"
        };
        for (int c = 0; c < 16; c++) {
            if (cat_visit_counts[c] > 0)
                printf(" %s=%u", cat_names[c], cat_visit_counts[c]);
        }
        printf("\n");

        assert(total_chain_items > 0);
        assert(max_chain_depth < 50);

        free(td);
    }
}

int main(void) {
    printf("test_theron_v1_track02_ground_ref\n");
    test_ref_encode_decode();

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
