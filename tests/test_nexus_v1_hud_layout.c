#include <stdio.h>
#include <stdlib.h>

#include "nexus_v1_hud_layout.h"

int main(void)
{
    const char *root = getenv("FIRESTAFF_NEXUS_DATA_DIR");
    char path[1024];
    FILE *file;
    long size;
    uint8_t *data;
    Nexus_HudElement entries[NEXUS_HUD_LAYOUT_ENTRY_COUNT];
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
    if (nexus_v1_hud_layout_parse_dm_bin(
            data, (size_t)size, entries, NEXUS_HUD_LAYOUT_ENTRY_COUNT,
            &count) != 0 || count != NEXUS_HUD_LAYOUT_ENTRY_COUNT ||
        entries[0].element_id != NEXUS_HUD_VIEWPORT ||
        entries[0].x != 256U || entries[0].y != 125U ||
        entries[12].element_id != NEXUS_HUD_HP_BAR ||
        entries[12].x != 64U || entries[12].y != 4U ||
        entries[15].x != 256U || entries[16].element_id != NEXUS_HUD_SPELL_PANEL ||
        entries[16].x != 288U || entries[16].y != 48U ||
        entries[19].element_id != 0xFFFFU) {
        free(data);
        fprintf(stderr, "FAIL: real DM.BIN HUD layout mismatch\n");
        return 1;
    }
    free(data);
    puts("PASS: real DM.BIN HUD layout parsed (80 entries)");
    return 0;
}
