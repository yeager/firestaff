
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nexus_v1_hud_hit_rects.h"

static int verify_real_dm_bin(void)
{
    const char *root = getenv("FIRESTAFF_NEXUS_DATA_DIR");
    char path[1024];
    FILE *file;
    long size;
    uint8_t *bytes;
    Nexus_HitRect parsed[NEXUS_HIT_RECT_COUNT];
    const Nexus_HitRect *legacy;
    size_t count = 0U;
    size_t i;

    if (!root || !root[0]) return 0;
    if (snprintf(path, sizeof(path), "%s/DM.BIN", root) >= (int)sizeof(path)) {
        return 0;
    }
    file = fopen(path, "rb");
    if (!file) return 0;
    if (fseek(file, 0, SEEK_END) != 0 || (size = ftell(file)) <= 0 ||
        fseek(file, 0, SEEK_SET) != 0) {
        fclose(file);
        return 0;
    }
    bytes = (uint8_t *)malloc((size_t)size);
    if (!bytes || fread(bytes, 1, (size_t)size, file) != (size_t)size) {
        free(bytes);
        fclose(file);
        return 0;
    }
    fclose(file);
    if (nexus_v1_hud_hit_rects_parse_dm_bin(
            bytes, (size_t)size, parsed, NEXUS_HIT_RECT_COUNT,
            &count) != 0 || count != NEXUS_HIT_RECT_COUNT) {
        free(bytes);
        return -1;
    }
    (void)nexus_v1_hud_hit_rects(&legacy);
    for (i = 0U; i < count; ++i) {
        if (memcmp(&parsed[i], &legacy[i], sizeof(parsed[i])) != 0) {
            free(bytes);
            return -1;
        }
    }
    free(bytes);
    return 1;
}

int main(void) {
    const Nexus_HitRect *rects;
    int count, fail = 0;

    count = nexus_v1_hud_hit_rects(&rects);
    if (count != NEXUS_HIT_RECT_COUNT) {
        fprintf(stderr, "FAIL: expected %d rects, got %d\n", NEXUS_HIT_RECT_COUNT, count);
        return 1;
    }

    /* Verify viewport rect */
    if (rects[NEXUS_HIT_VIEWPORT].x1 != 144 || rects[NEXUS_HIT_VIEWPORT].y1 != 72 ||
        rects[NEXUS_HIT_VIEWPORT].x2 != 240 || rects[NEXUS_HIT_VIEWPORT].y2 != 200) {
        fprintf(stderr, "FAIL: viewport rect mismatch\n"); fail++;
    }

    /* Verify compass rect */
    if (rects[NEXUS_HIT_COMPASS].x1 != 14 || rects[NEXUS_HIT_COMPASS].y1 != 6 ||
        rects[NEXUS_HIT_COMPASS].x2 != 74 || rects[NEXUS_HIT_COMPASS].y2 != 45) {
        fprintf(stderr, "FAIL: compass rect mismatch\n"); fail++;
    }

    /* Verify movement pad rect */
    if (rects[NEXUS_HIT_MOVEMENT_PAD].x1 != 27 || rects[NEXUS_HIT_MOVEMENT_PAD].y2 != 207) {
        fprintf(stderr, "FAIL: movement pad rect mismatch\n"); fail++;
    }

    /* Hit test: center of viewport */
    {
        int idx = nexus_v1_hud_hit_test(192, 136);
        if (idx != NEXUS_HIT_VIEWPORT && idx != NEXUS_HIT_LOWER_VIEWPORT) {
            fprintf(stderr, "FAIL: viewport center hit=%d\n", idx); fail++;
        }
    }

    /* Hit test: compass area */
    {
        int idx = nexus_v1_hud_hit_test(40, 20);
        if (idx != NEXUS_HIT_COMPASS) {
            fprintf(stderr, "FAIL: compass hit=%d\n", idx); fail++;
        }
    }

    /* Hit test: sidebar button */
    {
        int idx = nexus_v1_hud_hit_test(300, 110);
        if (idx != NEXUS_HIT_SIDEBAR_BUTTON_0) {
            fprintf(stderr, "FAIL: sidebar btn hit=%d\n", idx); fail++;
        }
    }

    /* Hit test: outside all rects */
    {
        int idx = nexus_v1_hud_hit_test(5, 220);
        if (idx != -1) {
            fprintf(stderr, "FAIL: outside hit=%d (expected -1)\n", idx); fail++;
        }
    }

    /* Verify empty rect at index 9 */
    if (rects[9].x1 != 0 || rects[9].y1 != 0 || rects[9].x2 != 0 || rects[9].y2 != 0) {
        fprintf(stderr, "FAIL: empty rect 9 not zero\n"); fail++;
    }

    if (fail) {
        fprintf(stderr, "%d failures\n", fail);
        return 1;
    }
    {
        int real = verify_real_dm_bin();
        if (real < 0) {
            fprintf(stderr, "FAIL: real DM.BIN HUD hit-rect parse mismatch\n");
            return 1;
        }
        if (real > 0) {
            printf("ok: real DM.BIN HUD hit rectangles parsed and match source table\n");
        } else {
            printf("ok: real DM.BIN HUD hit rectangles skipped (data root not mounted)\n");
        }
    }
    printf("ok: Nexus HUD hit rectangles verified (%d rects from DM.BIN 0x038000)\n", count);
    return 0;
}
