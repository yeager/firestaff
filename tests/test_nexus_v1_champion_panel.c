
#include <stdio.h>
#include "nexus_v1_champion_panel.h"

int main(void) {
    int fail = 0;
    const Nexus_PanelRect* r;

    /* Stat bar table — 12 entries from DM.BIN 0x038428 */
    r = nexus_v1_stat_bar_rects();
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
    r = nexus_v1_inv_slot_rects();
    if (r[0].x1 != 192 || r[0].y1 != 152 || r[0].action != 0x29) {
        fprintf(stderr, "FAIL: inv slot [0]\n"); fail++;
    }
    if (r[3].x1 != 208 || r[3].y1 != 168 || r[3].param != 3) {
        fprintf(stderr, "FAIL: inv slot [3]\n"); fail++;
    }

    /* Equipment slots — 8 entries around champion portrait */
    r = nexus_v1_equip_slot_rects();
    if (r[0].x1 != 257 || r[0].action != 0x2C || r[0].param != 0) {
        fprintf(stderr, "FAIL: equip slot [0]\n"); fail++;
    }
    if (r[7].x1 != 252 || r[7].y1 != 136 || r[7].param != 7) {
        fprintf(stderr, "FAIL: equip slot [7]\n"); fail++;
    }

    /* Hit test — click inside stat bar 0 */
    {
        const Nexus_PanelRect* hit = nexus_v1_panel_hit_test(
            nexus_v1_stat_bar_rects(), NEXUS_STAT_BAR_RECT_COUNT, 40, 30);
        if (!hit || hit->param != 0) {
            fprintf(stderr, "FAIL: hit test stat bar 0\n"); fail++;
        }
    }

    /* Hit test — click inside equip slot 5 */
    {
        const Nexus_PanelRect* hit = nexus_v1_panel_hit_test(
            nexus_v1_equip_slot_rects(), NEXUS_EQUIP_SLOT_RECT_COUNT, 280, 140);
        if (!hit || hit->param != 5) {
            fprintf(stderr, "FAIL: hit test equip slot 5\n"); fail++;
        }
    }

    /* Hit test — miss */
    {
        const Nexus_PanelRect* hit = nexus_v1_panel_hit_test(
            nexus_v1_inv_slot_rects(), NEXUS_INV_SLOT_RECT_COUNT, 0, 0);
        if (hit) {
            fprintf(stderr, "FAIL: hit test should miss\n"); fail++;
        }
    }

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
