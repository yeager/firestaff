
#include <stdio.h>
#include <stdlib.h>
#include "nexus_v1_hud_layout.h"

static int verify_real_dm_bin(void)
{
    const char *root = getenv("FIRESTAFF_NEXUS_DATA_DIR");
    char path[1024];
    FILE *file;
    long size;
    uint8_t *bytes;
    Nexus_HudElement parsed[NEXUS_HUD_LAYOUT_ENTRY_COUNT];
    const Nexus_HudElement *legacy;
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
    if (nexus_v1_hud_layout_parse_dm_bin(
            bytes, (size_t)size, parsed, NEXUS_HUD_LAYOUT_ENTRY_COUNT,
            &count) != 0 || count != NEXUS_HUD_LAYOUT_ENTRY_COUNT) {
        free(bytes);
        return -1;
    }
    (void)nexus_v1_hud_layout(&legacy);
    for (i = 0U; i < count; ++i) {
        if (parsed[i].element_id != legacy[i].element_id ||
            parsed[i].x != legacy[i].x || parsed[i].y != legacy[i].y) {
            free(bytes);
            return -1;
        }
    }
    free(bytes);
    return 1;
}

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
    {
        int real = verify_real_dm_bin();
        if (real < 0) {
            fprintf(stderr, "FAIL: real DM.BIN HUD layout parse mismatch\n");
            return 1;
        }
        if (real > 0) {
            printf("ok: real DM.BIN HUD layout parsed and matches source table\n");
        } else {
            printf("ok: real DM.BIN HUD layout skipped (data root not mounted)\n");
        }
    }
    printf("ok: Nexus HUD layout table verified (80 entries, %d groups)\n", count);
    return 0;
}
