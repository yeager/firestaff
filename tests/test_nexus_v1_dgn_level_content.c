#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "asset_find_by_hash.h"
#include "nexus_v1_dgn.h"
#include "nexus_v1_dungeon.h"

static uint8_t *load_file(const char *path, int *size_out) {
    FILE *f = fopen(path, "rb");
    uint8_t *buf;
    long sz;
    if (!f) return NULL;
    if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return NULL; }
    sz = ftell(f);
    if (sz <= 0 || sz > INT_MAX || fseek(f, 0, SEEK_SET) != 0) {
        fclose(f);
        return NULL;
    }
    buf = (uint8_t *)malloc((size_t)sz);
    if (!buf) { fclose(f); return NULL; }
    if (fread(buf, 1, (size_t)sz, f) != (size_t)sz) {
        free(buf);
        fclose(f);
        return NULL;
    }
    fclose(f);
    *size_out = (int)sz;
    return buf;
}

static int test_all_levels(const char *data_dir) {
    /* Keep this corpus regression on the same authenticated European
     * identities as the production level loader.  Counts alone are not
     * provenance: a same-sized fixture or renamed DGN must not publish
     * floor-item/decoration/sensor census evidence. */
    static const char *const retail_md5[16] = {
        "603ec9c531a92539babdda84ab09e78e",
        "751e1442bf7dccbd41bf146b5be144ab",
        "e2cb85d9fedc27f894a84e0f465fcde1",
        "19637d6b59849565f64565aed786d7ea",
        "85abc1b822e5c66ec4e99f1f676c140e",
        "ed5d54ab0ac1c927c1346dd966c8a5cc",
        "58c336ff6146e7216f0081e726823ea1",
        "c19e6038a017a320515ecbb66f6da197",
        "9bfc31bea631345a3660c2645be0e95b",
        "32a6450f29eb7babd73fcbe7a0310f22",
        "2928440e9c21457929f1323a28a42f70",
        "d7be5cd0d6e5c10afe99ec9950614fad",
        "db1cf70d6730615f73f191fad5e11e32",
        "f8876d0181d79727013236a6b597b99b",
        "a634dd5e95567ecbbbc332350c8cf12b",
        "5e6e237074f1e6b0decc629868a51f3c"
    };
    static const struct {
        int level;
        int items, decors, sensors, alcoves, wdecors, wsensors;
    } expected[16] = {
        { 0,  0, 34,  0,  0,  0,  0},
        { 1,  8, 28,  4,  1, 41, 42},
        { 2, 58,  1, 32,  4,  4, 35},
        { 3, 65, 19, 42, 13,  2, 93},
        { 4, 41,  0, 34,  0,  3, 34},
        { 5, 30,  0, 92,  2,  9, 63},
        { 6, 21,  0,  7,  1,  3, 23},
        { 7, 56,  1, 53,  7,  4, 61},
        { 8, 19,  6, 22, 14, 11, 40},
        { 9, 26,  2,  7,  0,  2, 18},
        {10, 26,  4, 40,  3,  5, 33},
        {11, 31,  0, 49,  3, 10, 44},
        {12, 50,  6, 28,  0,  5, 35},
        {13,  6,  0,  7,  1,  7, 11},
        {14,  1,  0,  4,  2,  2, 14},
        {15,  8,  0, 38,  3,  4, 31},
    };

    int pass = 0, fail = 0;
    int i;
    for (i = 0; i < 16; ++i) {
        char path[1024];
        uint8_t *buf;
        int sz;
        Nexus_V1_DgnDecodeResult res;

        snprintf(path, sizeof(path), "%s/LEV%02d.DGN", data_dir, i);
        buf = load_file(path, &sz);
        if (!buf) {
            printf("SKIP LEV%02d.DGN (not found)\n", i);
            continue;
        }

        if (!asset_file_matches_md5(path, retail_md5[i])) {
            printf("FAIL LEV%02d.DGN retail identity mismatch\n", i);
            fail++;
            free(buf);
            continue;
        }

        if (!nexus_v1_dgn_decode(buf, sz, &res)) {
            printf("FAIL LEV%02d.DGN decode failed\n", i);
            fail++;
            free(buf);
            continue;
        }

        if (i == 1) {
            Nexus_V1_Level level;
            int entry;
            int saw_inscription = 0;
            int saw_champion = 0;
            memset(&level, 0, sizeof(level));
            if (nexus_v1_level_load(&level, buf, sz, i) != 0) {
                printf("FAIL LEV01 Structure1F load for sensor-type regression\n");
                fail++;
                free(buf);
                continue;
            }
            for (entry = 0; entry < level.structure1f_entry_count; ++entry) {
                const Nexus_V1_DgnStructure1FEntry *record =
                    &level.structure1f_entries[entry];
                if (record->family != NEXUS_V1_DGN_STRUCTURE1F_WALL_SENSORS)
                    continue;
                if (record->sensor_type == 0x8bU) saw_inscription = 1;
                if (record->sensor_type == 0x63U) saw_champion = 1;
            }
            if (!saw_inscription || !saw_champion) {
                printf("FAIL LEV01 wall sensor type byte was not retained\n");
                fail++;
                free(buf);
                continue;
            }
        }

        int ok = 1;
        if (res.floor_item_count != expected[i].items) {
            printf("FAIL LEV%02d items: got %d expected %d\n", i, res.floor_item_count, expected[i].items);
            ok = 0;
        }
        if (res.floor_decor_count != expected[i].decors) {
            printf("FAIL LEV%02d decors: got %d expected %d\n", i, res.floor_decor_count, expected[i].decors);
            ok = 0;
        }
        if (res.floor_sensor_count != expected[i].sensors) {
            printf("FAIL LEV%02d sensors: got %d expected %d\n", i, res.floor_sensor_count, expected[i].sensors);
            ok = 0;
        }
        if (res.alcove_count != expected[i].alcoves) {
            printf("FAIL LEV%02d alcoves: got %d expected %d\n", i, res.alcove_count, expected[i].alcoves);
            ok = 0;
        }
        if (res.wall_decor_count != expected[i].wdecors) {
            printf("FAIL LEV%02d wdecors: got %d expected %d\n", i, res.wall_decor_count, expected[i].wdecors);
            ok = 0;
        }
        if (res.wall_sensor_count != expected[i].wsensors) {
            printf("FAIL LEV%02d wsensors: got %d expected %d\n", i, res.wall_sensor_count, expected[i].wsensors);
            ok = 0;
        }

        if (ok) {
            printf("PASS LEV%02d.DGN: %d items, %d decors, %d sensors, %d alcoves, %d wdecors, %d wsensors\n",
                   i, res.floor_item_count, res.floor_decor_count, res.floor_sensor_count,
                   res.alcove_count, res.wall_decor_count, res.wall_sensor_count);
            pass++;
        } else {
            fail++;
        }

        free(buf);
    }

    printf("\n%d passed, %d failed\n", pass, fail);
    return fail == 0 ? 0 : 1;
}

int main(void) {
    const char *dir = getenv("NEXUS_DATA_DIR");
    if (!dir) dir = getenv("HOME");
    if (!dir) {
        printf("SKIP no data dir\n");
        return 0;
    }

    char data_dir[1024];
    if (getenv("NEXUS_DATA_DIR")) {
        snprintf(data_dir, sizeof(data_dir), "%s", dir);
    } else {
        snprintf(data_dir, sizeof(data_dir), "%s/.firestaff/data/nexus", dir);
    }

    return test_all_levels(data_dir);
}
