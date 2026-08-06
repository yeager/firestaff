#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "nexus_v1_hud_layout.h"

static uint64_t fnv1a64(const uint8_t *data, size_t size)
{
    uint64_t hash = UINT64_C(0xcbf29ce484222325);
    size_t i;
    for (i = 0U; i < size; ++i) {
        hash ^= data[i];
        hash *= UINT64_C(0x100000001b3);
    }
    return hash;
}

static size_t count_be32(const uint8_t *data, size_t size, uint32_t value)
{
    size_t i;
    size_t count = 0U;
    for (i = 0U; i + 4U <= size; ++i) {
        uint32_t observed = ((uint32_t)data[i] << 24) |
            ((uint32_t)data[i + 1U] << 16) |
            ((uint32_t)data[i + 2U] << 8) | data[i + 3U];
        if (observed == value) ++count;
    }
    return count;
}

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
    if (size < (long)NEXUS_HUD_LAYOUT_DM_BIN_OFFSET +
            (long)NEXUS_HUD_LAYOUT_ENTRY_COUNT *
                (long)NEXUS_HUD_LAYOUT_ENTRY_BYTES ||
        memcmp(data + NEXUS_HUD_LAYOUT_DM_BIN_OFFSET - 0x10U,
               "yam\\menuctrl.c", 14U) != 0 ||
        fnv1a64(data + NEXUS_HUD_LAYOUT_DM_BIN_OFFSET,
                NEXUS_HUD_LAYOUT_ENTRY_COUNT * NEXUS_HUD_LAYOUT_ENTRY_BYTES) !=
            UINT64_C(0x5fc435070d81882c) ||
        count_be32(data, (size_t)size, UINT32_C(0x060476d0)) != 7U) {
        free(data);
        fprintf(stderr,
                "FAIL: DM.BIN menuctrl disassembly/table anchor mismatch\n");
        return 1;
    }
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
    data[NEXUS_HUD_LAYOUT_DM_BIN_OFFSET + 2U] = 0U;
    data[NEXUS_HUD_LAYOUT_DM_BIN_OFFSET + 3U] = 1U;
    if (nexus_v1_hud_layout_parse_dm_bin(
            data, (size_t)size, entries, NEXUS_HUD_LAYOUT_ENTRY_COUNT,
            &count) == 0) {
        free(data);
        fprintf(stderr, "FAIL: reserved HUD layout word was accepted\n");
        return 1;
    }
    /* Restore the reserved word, then test the independent Saturn display
     * envelope. A non-sentinel off-screen coordinate is not a valid retail
     * HUD placement. */
    data[NEXUS_HUD_LAYOUT_DM_BIN_OFFSET + 2U] = 0U;
    data[NEXUS_HUD_LAYOUT_DM_BIN_OFFSET + 3U] = 0U;
    data[NEXUS_HUD_LAYOUT_DM_BIN_OFFSET + 4U] = 0x01U;
    data[NEXUS_HUD_LAYOUT_DM_BIN_OFFSET + 5U] = 0x41U;
    if (nexus_v1_hud_layout_parse_dm_bin(
            data, (size_t)size, entries, NEXUS_HUD_LAYOUT_ENTRY_COUNT,
            &count) == 0) {
        free(data);
        fprintf(stderr, "FAIL: off-screen HUD layout coordinate was accepted\n");
        return 1;
    }
    free(data);
    puts("PASS: real DM.BIN HUD layout parsed (80 entries)");
    return 0;
}
