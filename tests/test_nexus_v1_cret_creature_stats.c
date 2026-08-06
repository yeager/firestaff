
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nexus_v1_creatures.h"

#define DM_BIN_AI_TABLE_OFFSET 0x0383A8U

static uint32_t read_be32(const uint8_t *p)
{
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
           ((uint32_t)p[2] << 8) | (uint32_t)p[3];
}

static int verify_retail_ai_table(const char *data_dir,
                                  const Nexus_V1_CreatureManager *mgr)
{
    char path[512];
    uint8_t bytes[NEXUS_CRET_COUNT * sizeof(uint32_t)];
    FILE *file;
    int i;

    if (snprintf(path, sizeof(path), "%s/DM.BIN", data_dir) >= (int)sizeof(path) ||
        !(file = fopen(path, "rb"))) {
        puts("SKIP: retail Nexus DM.BIN is not mounted for AI table check");
        return 0;
    }
    if (fseek(file, (long)DM_BIN_AI_TABLE_OFFSET, SEEK_SET) != 0 ||
        fread(bytes, 1, sizeof(bytes), file) != sizeof(bytes)) {
        fclose(file);
        fprintf(stderr, "FAIL: could not read DM.BIN AI table at 0x%06X\n",
                DM_BIN_AI_TABLE_OFFSET);
        return 1;
    }
    fclose(file);
    for (i = 0; i < NEXUS_CRET_COUNT; ++i) {
        uint32_t actual = read_be32(bytes + i * sizeof(uint32_t));
        if (actual != mgr->types[i].ai_func_ptr) {
            fprintf(stderr, "FAIL: AI pointer %d: got 0x%08X, expected 0x%08X\n",
                    i, mgr->types[i].ai_func_ptr, actual);
            return 1;
        }
    }
    printf("AI dispatch: all %d pointers match DM.BIN 0x%06X\n",
           NEXUS_CRET_COUNT, DM_BIN_AI_TABLE_OFFSET);
    return 0;
}

static const struct {
    const char *name;
    int hp, damage, armor, speed;
} expected[] = {
    {"Ant Man",         0x78, 0x5A, 0x19, 0x96},
    {"Mummy",           0x78, 0x3C, 0x0F, 0x78},
    {"Ghost",           0x5A, 0x50, 0x55, 0x5A},
    {"Vexirk",          0x5A, 0x5A, 0x1E, 0x78},
    {"Giggler",         0x5A, 0x5A, 0x07, 0x5A},
    {"Golem",           0x78, 0x78, 0x3C, 0x78},
    {"Hell Hound",      0x5A, 0x64, 0x4B, 0x96},
    {"Gold Dragon",     0x5A, 0xB4, 0x46, 0x5A},
    {"Silver Dragon",   0x5A, 0xB4, 0x46, 0x5A},
    {"Red Dragon",      0x96, 0x82, 0x41, 0x96},
    {"Last Monster",    0x96, 0x82, 0x41, 0x96},
    {"Oitu",            0x78, 0xC8, 0x55, 0x78},
    {"Rat",             0x78, 0x6E, 0x28, 0x78},
    {"Rock Pile",       0x78, 0x62, 0x2D, 0x5A},
    {"Screamer",        0x5A, 0x6E, 0x28, 0x5A},
    {"Scorpion",        0x96, 0x64, 0x23, 0x5A},
    {"Floor Snake",     0xB4, 0x46, 0x0F, 0xB4},
    {"Wall Snake",      0x78, 0x78, 0x2B, 0x5A},
    {"Skeleton Sword",  0x96, 0x64, 0x5F, 0xD2},
    {"Skeleton Shield", 0x78, 0x64, 0x5F, 0x78},
    {"Worm",            0x5A, 0x78, 0x5F, 0x5A},
    {"Green Dragon",    0x5A, 0x55, 0x50, 0x5A},
    {"Red Drake",       0x5A, 0x6E, 0x28, 0x78},
    {"Dragon Zombie",   0x5A, 0xDC, 0x41, 0x78},
    {"Mini Dragon",     0x3C, 0xFA, 0x5A, 0x5A},
    {"Borketh",         0x3C, 0xFA, 0x5A, 0x5A},
    {"Chaos",           0x3C, 0xFF, 0x5A, 0x5A},
    {"Lord Rib",        0x5A, 0x01, 0x0A, 0x5A},
    {"Big Worm",        0x5A, 0x5F, 0x64, 0x5A},
    {"Obake",           0x5A, 0xFF, 0x82, 0x96},
};

int main(void) {
    Nexus_V1_CreatureManager mgr;
    Nexus_V1_CreatureManager bytes_mgr;
    const char *data_dir;
    char path[512];
    FILE *bytes_file;
    long bytes_size;
    uint8_t *bytes;
    int bound, bytes_bound, i, fail = 0;

    data_dir = getenv("FIRESTAFF_NEXUS_DATA_DIR");
    if (!data_dir) data_dir = getenv("NEXUS_DATA_DIR");
    if (!data_dir) {
        fprintf(stderr, "SKIP: no data dir\n");
        return 0;
    }

    snprintf(path, sizeof(path), "%s/RLOWFIX.BIN", data_dir);

    nexus_v1_creatures_init(&mgr);
    bound = nexus_v1_creatures_load_cret(&mgr, path);
    if (bound == 0) {
        fprintf(stderr, "SKIP: RLOWFIX.BIN not found or CRET section missing\n");
        return 0;
    }
    bytes_file = fopen(path, "rb");
    if (!bytes_file || fseek(bytes_file, 0, SEEK_END) != 0 ||
        (bytes_size = ftell(bytes_file)) <= 0 ||
        fseek(bytes_file, 0, SEEK_SET) != 0) {
        if (bytes_file) fclose(bytes_file);
        return 1;
    }
    bytes = (uint8_t *)malloc((size_t)bytes_size);
    if (!bytes || fread(bytes, 1U, (size_t)bytes_size, bytes_file) !=
            (size_t)bytes_size) {
        free(bytes);
        fclose(bytes_file);
        return 1;
    }
    fclose(bytes_file);
    nexus_v1_creatures_init(&bytes_mgr);
    bytes_bound = nexus_v1_creatures_load_cret_bytes(
        &bytes_mgr, bytes, (size_t)bytes_size);
    free(bytes);
    if (bytes_bound != bound || bytes_bound != NEXUS_CRET_COUNT ||
        bytes_mgr.types[0].health != 0x78 ||
        bytes_mgr.types[26].ranged_type != 0) {
        fprintf(stderr, "FAIL: bytes-based CRET binding diverged from file route\n");
        return 1;
    }

    printf("CRET: bound %d creatures\n", bound);
    if (bound != 30) {
        fprintf(stderr, "FAIL: expected 30 creatures, got %d\n", bound);
        return 1;
    }

    for (i = 0; i < 30; i++) {
        Nexus_CreatureType *t = &mgr.types[i];
        if (t->health != expected[i].hp) {
            fprintf(stderr, "FAIL: %s HP: got %d, expected %d\n",
                    expected[i].name, t->health, expected[i].hp);
            fail++;
        }
        if (t->attack != expected[i].damage) {
            fprintf(stderr, "FAIL: %s attack: got %d, expected %d\n",
                    expected[i].name, t->attack, expected[i].damage);
            fail++;
        }
        if (t->defense != expected[i].armor) {
            fprintf(stderr, "FAIL: %s defense: got %d, expected %d\n",
                    expected[i].name, t->defense, expected[i].armor);
            fail++;
        }
        if (t->speed != expected[i].speed) {
            fprintf(stderr, "FAIL: %s speed: got %d, expected %d\n",
                    expected[i].name, t->speed, expected[i].speed);
            fail++;
        }
        if (!t->cret_bound) {
            fprintf(stderr, "FAIL: %s cret_bound not set\n", expected[i].name);
            fail++;
        }
        if (!fail)
            printf("  [%2d] %-16s HP=%3d ATK=%3d DEF=%3d SPD=%3d OK\n",
                   i, t->name, t->health, t->attack, t->defense, t->speed);
    }

    fail += verify_retail_ai_table(data_dir, &mgr);

    /* Verify new CRET fields for known creatures */
    {
        Nexus_CreatureType *antman = &mgr.types[0];
        Nexus_CreatureType *golem = &mgr.types[5];
        Nexus_CreatureType *chaos = &mgr.types[26];
        if (antman->detection_range != 43) {
            fprintf(stderr, "FAIL: AntMan detection=%d, expected 43\n", antman->detection_range); fail++;
        }
        if (antman->ai_type != 13) {
            fprintf(stderr, "FAIL: AntMan ai_type=%d, expected 13\n", antman->ai_type); fail++;
        }
        if (antman->poison != 0) {
            fprintf(stderr, "FAIL: AntMan poison=%d, expected 0\n", antman->poison); fail++;
        }
        if (golem->behav_flags != 48) {
            fprintf(stderr, "FAIL: Golem behav_flags=%d, expected 48\n", golem->behav_flags); fail++;
        }
        if (chaos->ai_type != 4) {
            fprintf(stderr, "FAIL: Chaos ai_type=%d, expected 4\n", chaos->ai_type); fail++;
        }
        if (chaos->ranged_type != 0) {
            fprintf(stderr, "FAIL: Chaos ranged=%d, expected 0\n", chaos->ranged_type); fail++;
        }
        if (!fail)
            printf("New CRET fields (detection, AI, poison, behavior) verified\n");
    }

    if (fail) {
        fprintf(stderr, "%d stat mismatches\n", fail);
        return 1;
    }
    printf("All 30 creature CRET stats verified against RLOWFIX.BIN\n");
    return 0;
}
