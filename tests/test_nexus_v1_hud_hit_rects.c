#include <stdio.h>
#include <stdlib.h>

#include "nexus_v1_hud_hit_rects.h"

int main(void)
{
    const char *root = getenv("FIRESTAFF_NEXUS_DATA_DIR");
    char path[1024];
    FILE *file;
    long size;
    uint8_t *data;
    Nexus_HitRect rects[NEXUS_HIT_RECT_COUNT];
    size_t count = 0U;

    if (!root || !root[0]) {
        puts("SKIP: FIRESTAFF_NEXUS_DATA_DIR is not mounted");
        return 77;
    }
    if (snprintf(path, sizeof(path), "%s/DM.BIN", root) >= (int)sizeof(path) ||
        !(file = fopen(path, "rb")) || fseek(file, 0, SEEK_END) != 0 ||
        (size = ftell(file)) <= 0 || fseek(file, 0, SEEK_SET) != 0) {
        if (file) fclose(file);
        puts("SKIP: real DM.BIN is unavailable");
        return 77;
    }
    data = (uint8_t *)malloc((size_t)size);
    if (!data || fread(data, 1, (size_t)size, file) != (size_t)size) {
        free(data);
        fclose(file);
        puts("SKIP: real DM.BIN could not be read");
        return 77;
    }
    fclose(file);
    if (nexus_v1_hud_hit_rects_parse_dm_bin(
            data, (size_t)size, rects, NEXUS_HIT_RECT_COUNT, &count) != 0 ||
        count != NEXUS_HIT_RECT_COUNT ||
        rects[NEXUS_HIT_VIEWPORT].x1 != 144 ||
        rects[NEXUS_HIT_VIEWPORT].y1 != 72 ||
        rects[NEXUS_HIT_VIEWPORT].x2 != 240 ||
        rects[NEXUS_HIT_VIEWPORT].y2 != 200 ||
        rects[NEXUS_HIT_COMPASS].x1 != 14 ||
        rects[NEXUS_HIT_COMPASS].y1 != 6 ||
        rects[NEXUS_HIT_COMPASS].x2 != 74 ||
        rects[NEXUS_HIT_COMPASS].y2 != 45 ||
        rects[NEXUS_HIT_MOVEMENT_PAD].x1 != 27 ||
        rects[NEXUS_HIT_MOVEMENT_PAD].y2 != 207) {
        free(data);
        fprintf(stderr, "FAIL: real DM.BIN HUD hit rectangles mismatch\n");
        return 1;
    }
    data[NEXUS_HIT_RECT_DM_BIN_OFFSET + 4U] = 0x01U;
    data[NEXUS_HIT_RECT_DM_BIN_OFFSET + 5U] = 0x41U;
    if (nexus_v1_hud_hit_rects_parse_dm_bin(
            data, (size_t)size, rects, NEXUS_HIT_RECT_COUNT, &count) == 0) {
        free(data);
        fprintf(stderr, "FAIL: out-of-screen HUD rectangle was accepted\n");
        return 1;
    }
    /* A signed negative origin is also outside the Saturn display envelope;
     * it must not pass merely because the far corner is in range. */
    data[NEXUS_HIT_RECT_DM_BIN_OFFSET + 4U] = 0x00U;
    data[NEXUS_HIT_RECT_DM_BIN_OFFSET + 5U] = 0x0aU;
    data[NEXUS_HIT_RECT_DM_BIN_OFFSET + 0U] = 0xffU;
    data[NEXUS_HIT_RECT_DM_BIN_OFFSET + 1U] = 0xffU;
    if (nexus_v1_hud_hit_rects_parse_dm_bin(
            data, (size_t)size, rects, NEXUS_HIT_RECT_COUNT, &count) == 0) {
        free(data);
        fprintf(stderr, "FAIL: negative-origin HUD rectangle was accepted\n");
        return 1;
    }
    /* A zero-width region is not a usable Saturn hit rectangle and must not
     * enter the authenticated HUD geometry receipt. */
    data[NEXUS_HIT_RECT_DM_BIN_OFFSET + 0U] = 0x00U;
    data[NEXUS_HIT_RECT_DM_BIN_OFFSET + 1U] = 0x0eU;
    data[NEXUS_HIT_RECT_DM_BIN_OFFSET + 4U] = 0x00U;
    data[NEXUS_HIT_RECT_DM_BIN_OFFSET + 5U] = 0x0eU;
    if (nexus_v1_hud_hit_rects_parse_dm_bin(
            data, (size_t)size, rects, NEXUS_HIT_RECT_COUNT, &count) == 0) {
        free(data);
        fprintf(stderr, "FAIL: zero-width HUD rectangle was accepted\n");
        return 1;
    }
    free(data);
    puts("PASS: real DM.BIN HUD hit rectangles parsed (40 entries)");
    return 0;
}
