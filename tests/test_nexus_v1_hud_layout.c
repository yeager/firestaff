
#include <stdio.h>
#include "nexus_v1_hud_layout.h"

int main(void) {
    const Nexus_HudElement *elems;
    const uint16_t *hp;
    int count, fail = 0;

    count = nexus_v1_hud_layout(&elems);
    if (count != 80) {
        fprintf(stderr, "FAIL: expected 80 entries, got %d\n", count);
        return 1;
    }

    /* Verify viewport entry */
    if (elems[0].element_id != NEXUS_HUD_VIEWPORT || elems[0].x != 256 || elems[0].y != 125) {
        fprintf(stderr, "FAIL: viewport entry mismatch\n"); fail++;
    }

    /* Verify HP bar entries 12-15 */
    if (elems[12].element_id != NEXUS_HUD_HP_BAR || elems[12].x != 64 || elems[12].y != 4) {
        fprintf(stderr, "FAIL: HP bar 0 mismatch\n"); fail++;
    }
    if (elems[15].element_id != NEXUS_HUD_HP_BAR || elems[15].x != 256) {
        fprintf(stderr, "FAIL: HP bar 3 mismatch\n"); fail++;
    }

    /* Verify spell panel entry 16 */
    if (elems[16].element_id != NEXUS_HUD_SPELL_PANEL || elems[16].x != 288 || elems[16].y != 48) {
        fprintf(stderr, "FAIL: spell panel mismatch\n"); fail++;
    }

    /* Verify sentinel at entry 19 */
    if (elems[19].element_id != 0xFFFF) {
        fprintf(stderr, "FAIL: sentinel at 19 mismatch\n"); fail++;
    }

    /* Verify champion portraits */
    if (elems[4].element_id != NEXUS_HUD_CHAMPION_1 || elems[4].x != 272 || elems[4].y != 9) {
        fprintf(stderr, "FAIL: champion 1 portrait mismatch\n"); fail++;
    }

    /* Verify HP bar position accessor */
    hp = nexus_v1_hud_hp_bar_positions();
    if (hp[0] != 64 || hp[1] != 128 || hp[2] != 192 || hp[3] != 256) {
        fprintf(stderr, "FAIL: HP bar positions mismatch\n"); fail++;
    }

    /* Verify inventory grid entries 31-34 */
    if (elems[31].element_id != NEXUS_HUD_INV_SLOT || elems[31].x != 142 || elems[31].y != 86) {
        fprintf(stderr, "FAIL: inv slot 0 mismatch\n"); fail++;
    }

    if (fail) {
        fprintf(stderr, "%d failures\n", fail);
        return 1;
    }
    printf("ok: Nexus HUD layout table verified (80 entries, %d groups)\n", count);
    return 0;
}
