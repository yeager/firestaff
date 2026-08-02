#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nexus_v1_dgn.h"

static uint8_t *load_file(const char *path, int *size_out) {
    FILE *f = fopen(path, "rb");
    uint8_t *buf;
    long sz;
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    buf = (uint8_t *)malloc((size_t)sz);
    if (!buf) { fclose(f); return NULL; }
    fread(buf, 1, (size_t)sz, f);
    fclose(f);
    *size_out = (int)sz;
    return buf;
}

static int test_all_levels(const char *data_dir) {
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

        if (!nexus_v1_dgn_decode(buf, sz, &res)) {
            printf("FAIL LEV%02d.DGN decode failed\n", i);
            fail++;
            free(buf);
            continue;
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
