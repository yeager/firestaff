
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nexus_v1_champion_panel.h"
#include "nexus_v1_engine.h"

int main(void) {
    int fail = 0;
    const char *root = getenv("FIRESTAFF_NEXUS_DATA_DIR");
    char path[1024];
    FILE *file;
    long size;
    uint8_t *data;
    Nexus_PanelRect stat_bars[NEXUS_STAT_BAR_RECT_COUNT];
    Nexus_PanelRect inv_slots[NEXUS_INV_SLOT_RECT_COUNT];
    Nexus_PanelRect equip_slots[NEXUS_EQUIP_SLOT_RECT_COUNT];
    const Nexus_PanelRect* r;

    if (!root || !root[0]) root = ".firestaff/data/nexus";
    if (snprintf(path, sizeof(path), "%s/DM.BIN", root) >= (int)sizeof(path))
        return 77;
    file = fopen(path, "rb");
    if (!file || fseek(file, 0L, SEEK_END) != 0 || (size = ftell(file)) <= 0 ||
        fseek(file, 0L, SEEK_SET) != 0) {
        if (file) fclose(file);
        puts("SKIP: retail Nexus DM.BIN is not mounted");
        return 77;
    }
    data = (uint8_t *)malloc((size_t)size);
    if (!data || fread(data, 1U, (size_t)size, file) != (size_t)size) {
        free(data);
        fclose(file);
        return 77;
    }
    fclose(file);
    if (nexus_v1_champion_panel_parse_dm_bin(
            data, (size_t)size, stat_bars, inv_slots, equip_slots) != 0) {
        free(data);
        fprintf(stderr, "FAIL: retail DM.BIN panel tables did not parse\n");
        return 1;
    }

    {
        Nexus_V1_Engine engine;
        Nexus_PanelRect engine_stats[NEXUS_STAT_BAR_RECT_COUNT];
        Nexus_PanelRect engine_inv[NEXUS_INV_SLOT_RECT_COUNT];
        Nexus_PanelRect engine_equip[NEXUS_EQUIP_SLOT_RECT_COUNT];
        memset(&engine, 0, sizeof(engine));
        if (nexus_v1_champion_panel_geometry_ready(&engine) ||
            nexus_v1_champion_panel_geometry(
                &engine, engine_stats, engine_inv, engine_equip) == 0) {
            free(data);
            fprintf(stderr, "FAIL: uninitialized engine exposed HUD geometry\n");
            return 1;
        }
        if (nexus_v1_init(&engine, root) != 0 ||
            !nexus_v1_champion_panel_geometry_ready(&engine) ||
            nexus_v1_champion_panel_geometry(
                &engine, engine_stats, engine_inv, engine_equip) != 0 ||
            memcmp(engine_stats, stat_bars, sizeof(stat_bars)) != 0 ||
            memcmp(engine_inv, inv_slots, sizeof(inv_slots)) != 0 ||
            memcmp(engine_equip, equip_slots, sizeof(equip_slots)) != 0) {
            nexus_v1_shutdown(&engine);
            free(data);
            fprintf(stderr, "FAIL: initialized engine did not expose retail HUD geometry\n");
            return 1;
        }
        nexus_v1_shutdown(&engine);
    }

    /* Stat bar table — 12 entries from DM.BIN 0x038428 */
    r = stat_bars;
    if (r[0].x1 != 32 || r[0].y1 != 25 || r[0].x2 != 48 || r[0].y2 != 41 ||
        r[0].action != 0x2D || r[0].param != 0) {
        fprintf(stderr, "FAIL: stat bar [0]\n"); fail++;
    }
    if (r[2].param != -1 || r[5].param != -2 ||
        r[8].param != -3 || r[11].param != -4) {
        fprintf(stderr, "FAIL: stat bar group selectors\n"); fail++;
    }
    if (r[10].x1 != 240 || r[10].x2 != 256 || r[10].param != 7) {
        fprintf(stderr, "FAIL: stat bar [10] champion 7\n"); fail++;
    }

    /* Inventory grid — 4 entries, 2x2 at (192,152)-(224,184) */
    r = inv_slots;
    if (r[0].x1 != 192 || r[0].y1 != 152 || r[0].action != 0x29) {
        fprintf(stderr, "FAIL: inv slot [0]\n"); fail++;
    }
    if (r[3].x1 != 208 || r[3].y1 != 168 || r[3].param != 3) {
        fprintf(stderr, "FAIL: inv slot [3]\n"); fail++;
    }

    /* Equipment slots — 8 entries around champion portrait */
    r = equip_slots;
    if (r[0].x1 != 257 || r[0].action != 0x2C || r[0].param != 0) {
        fprintf(stderr, "FAIL: equip slot [0]\n"); fail++;
    }
    if (r[7].x1 != 252 || r[7].y1 != 136 || r[7].param != 7) {
        fprintf(stderr, "FAIL: equip slot [7]\n"); fail++;
    }

    /* Hit test — click inside stat bar 0 */
    {
        const Nexus_PanelRect* hit = nexus_v1_panel_hit_test(
            stat_bars, NEXUS_STAT_BAR_RECT_COUNT, 40, 30);
        if (!hit || hit->param != 0) {
            fprintf(stderr, "FAIL: hit test stat bar 0\n"); fail++;
        }
    }

    /* Hit test — click inside equip slot 5 */
    {
        const Nexus_PanelRect* hit = nexus_v1_panel_hit_test(
            equip_slots, NEXUS_EQUIP_SLOT_RECT_COUNT, 280, 140);
        if (!hit || hit->param != 5) {
            fprintf(stderr, "FAIL: hit test equip slot 5\n"); fail++;
        }
    }

    /* Hit test — miss */
    {
        const Nexus_PanelRect* hit = nexus_v1_panel_hit_test(
            inv_slots, NEXUS_INV_SLOT_RECT_COUNT, 0, 0);
        if (hit) {
            fprintf(stderr, "FAIL: hit test should miss\n"); fail++;
        }
    }

    free(data);

    /* NULL safety */
    {
        const Nexus_PanelRect* hit = nexus_v1_panel_hit_test(NULL, 0, 10, 10);
        if (hit) {
            fprintf(stderr, "FAIL: NULL safety\n"); fail++;
        }
    }

    if (fail) {
        fprintf(stderr, "%d failures\n", fail);
        return 1;
    }
    printf("ok: Nexus champion panel rects verified (3 groups + hit tests)\n");
    return 0;
}
