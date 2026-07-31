#include "nexus_v1_champions.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    const char *path = "/Users/bosse/.firestaff/data/nexus/RLOWFIX.BIN";
    FILE *f = fopen(path, "rb");
    long size;
    uint8_t *bytes;
    Nexus_V1_ChampionPool pool;
    if (!f) { puts("SKIP: local Nexus RLOWFIX.BIN not present"); return 0; }
    if (fseek(f, 0, SEEK_END) != 0) return 1;
    size = ftell(f);
    if (size <= 0 || fseek(f, 0, SEEK_SET) != 0) return 1;
    bytes = (uint8_t *)malloc((size_t)size);
    if (!bytes || fread(bytes, 1, (size_t)size, f) != (size_t)size) return 1;
    fclose(f);
    if (!nexus_v1_champions_init_from_rlowfix(&pool, bytes, (size_t)size)) return 1;
    if (pool.champion_count != NEXUS_NEXUS_PLRD_CHAMPION_COUNT) return 1;
    if (strcmp(pool.champions[0].name_jp, "アレックス") != 0 ||
        pool.champions[0].health != 50 || pool.champions[0].stamina != 57 ||
        pool.champions[0].mana != 13 || pool.champions[19].health != 125 ||
        pool.champions[19].wizard_level != 2) return 1;
    free(bytes);
    puts("test_nexus_v1_champion_plrd: PASS");
    return 0;
}
