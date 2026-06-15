/*
 * Theron V1 save header rejection regression.
 *
 * No game data is needed. The positive control uses theron_v1_save_to_slot()
 * as the existing valid Theron save header builder, then the negative cases
 * mutate synthetic temp saves so header metadata cannot become a launchable
 * Theron save profile.
 */

#include "theron_v1_dungeon_progression.h"
#include "theron_v1_save_load.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#if defined(_WIN32)
#include <direct.h>
#define TST_MKDIR(path) _mkdir(path)
#define TST_RMDIR(path) _rmdir(path)
#else
#include <unistd.h>
#define TST_MKDIR(path) mkdir(path, 0700)
#define TST_RMDIR(path) rmdir(path)
#endif

static int g_failures = 0;

static void expect_true(int condition, const char *message) {
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        ++g_failures;
    }
}

static int make_temp_dir(char out[512]) {
#if defined(_WIN32)
    const char *tmp = getenv("TEMP");
    snprintf(out, 512, "%s\\firestaff_theron_save_header_%lu",
             tmp ? tmp : ".", (unsigned long)time(NULL));
    return TST_MKDIR(out) == 0;
#else
    snprintf(out, 512, "/tmp/firestaff_theron_save_header_XXXXXX");
    return mkdtemp(out) != NULL;
#endif
}

static void remove_slot_file(const char *root, int slot) {
    char path[512];
    theron_v1_save_slot_path(root, slot, path, sizeof(path));
    if (path[0]) {
        remove(path);
    }
}

static void remove_temp_dir(const char *root) {
    int slot;
    for (slot = 0; slot < THERON_SAVE_SLOT_COUNT; ++slot) {
        remove_slot_file(root, slot);
    }
    TST_RMDIR(root);
}

static int save_valid_slot(const char *root, int slot, const char *label) {
    uint8_t champion_data[THERON_SAVE_CHAMPION_COUNT *
                          THERON_SAVE_CHAMPION_BLOCK_SIZE];
    Theron_DungeonProgression progression;

    memset(champion_data, 0, sizeof(champion_data));
    champion_data[0] = (uint8_t)(0x40 + slot);

    theron_v1_dungeon_progression_init(&progression);
    progression.quest_items_collected = (uint8_t)(1U << (slot % 7));
    progression.current_dungeon = THERON_DUNGEON_2_CRYPT_OF_SHADOWS;

    return theron_v1_save_to_slot(root,
                                  slot,
                                  champion_data,
                                  sizeof(champion_data),
                                  &progression,
                                  label);
}

static int set_deobfuscated_header_byte(const char *root,
                                        int slot,
                                        size_t offset,
                                        uint8_t deobfuscated_value) {
    char path[512];
    FILE *fp;
    uint8_t obfuscated_value;

    theron_v1_save_slot_path(root, slot, path, sizeof(path));
    fp = fopen(path, "r+b");
    if (!fp) {
        return 0;
    }
    if (fseek(fp, (long)offset, SEEK_SET) != 0) {
        fclose(fp);
        return 0;
    }

    obfuscated_value =
        (uint8_t)(deobfuscated_value ^
                  (uint8_t)(THERON_SAVE_OBFUSCATE_SEED + offset));
    if (fwrite(&obfuscated_value, 1, 1, fp) != 1) {
        fclose(fp);
        return 0;
    }
    return fclose(fp) == 0;
}

static void expect_slot_launchable(const char *root, int slot) {
    uint8_t champion_data[THERON_SAVE_CHAMPION_COUNT *
                          THERON_SAVE_CHAMPION_BLOCK_SIZE];
    Theron_DungeonProgression progression;
    Theron_SaveSlot info;
    Theron_SaveSlot slots[THERON_SAVE_SLOT_COUNT];
    int count;

    memset(champion_data, 0, sizeof(champion_data));
    memset(&progression, 0, sizeof(progression));
    memset(&info, 0, sizeof(info));
    memset(slots, 0, sizeof(slots));

    expect_true(theron_v1_save_verify_slot(root, slot) == 1,
                "positive control save verifies");
    expect_true(theron_v1_save_load_from_slot(root,
                                              slot,
                                              champion_data,
                                              sizeof(champion_data),
                                              &progression,
                                              sizeof(progression),
                                              &info) == 0,
                "positive control save loads");
    expect_true(info.valid == 1,
                "positive control save produces a launchable profile");

    count = theron_v1_save_enum_slots(root, slots, THERON_SAVE_SLOT_COUNT);
    expect_true(count > slot, "positive control slot is enumerated");
    expect_true(slots[slot].valid == 1,
                "positive control slot is marked available");
}

static void expect_slot_not_launchable(const char *root,
                                       int slot,
                                       const char *case_name) {
    uint8_t champion_data[THERON_SAVE_CHAMPION_COUNT *
                          THERON_SAVE_CHAMPION_BLOCK_SIZE];
    Theron_DungeonProgression progression;
    Theron_SaveSlot info;
    Theron_SaveSlot slots[THERON_SAVE_SLOT_COUNT];
    int count;

    memset(champion_data, 0, sizeof(champion_data));
    memset(&progression, 0, sizeof(progression));
    memset(&info, 0, sizeof(info));
    memset(slots, 0, sizeof(slots));

    expect_true(theron_v1_save_verify_slot(root, slot) == 0,
                case_name);
    expect_true(theron_v1_save_load_from_slot(root,
                                              slot,
                                              champion_data,
                                              sizeof(champion_data),
                                              &progression,
                                              sizeof(progression),
                                              &info) != 0,
                "invalid Theron save header is rejected on load");
    expect_true(info.valid == 0,
                "invalid Theron save header does not produce profile metadata");

    count = theron_v1_save_enum_slots(root, slots, THERON_SAVE_SLOT_COUNT);
    expect_true(count > slot, "invalid slot is still represented in enumeration");
    expect_true(slots[slot].valid == 0,
                "invalid Theron save header is marked unavailable");
}

int main(void) {
    char temp_dir[512];

    if (!make_temp_dir(temp_dir)) {
        perror("make temp dir");
        return 1;
    }

    expect_true(save_valid_slot(temp_dir, 0, "valid control") == 0,
                "valid Theron save fixture written");
    expect_slot_launchable(temp_dir, 0);

    expect_true(save_valid_slot(temp_dir, 1, "bad magic") == 0,
                "bad-magic fixture base written");
    expect_true(set_deobfuscated_header_byte(temp_dir,
                                             1,
                                             THERON_SAVE_OFF_MAGIC,
                                             (uint8_t)'X'),
                "bad-magic fixture mutated");
    expect_slot_not_launchable(temp_dir,
                               1,
                               "bad Theron save magic does not verify");

    expect_true(save_valid_slot(temp_dir, 2, "bad version") == 0,
                "bad-version fixture base written");
    expect_true(set_deobfuscated_header_byte(temp_dir,
                                             2,
                                             THERON_SAVE_OFF_VERSION,
                                             2),
                "bad-version fixture mutated");
    expect_slot_not_launchable(temp_dir,
                               2,
                               "unsupported Theron save version does not verify");

    expect_true(save_valid_slot(temp_dir, 3, "bad checksum") == 0,
                "checksum-mismatch fixture base written");
    expect_true(set_deobfuscated_header_byte(temp_dir,
                                             3,
                                             THERON_SAVE_OFF_QUEST_ITEMS,
                                             0x7f),
                "checksum-mismatch fixture mutated");
    expect_slot_not_launchable(temp_dir,
                               3,
                               "Theron save header/footer mismatch does not verify");

    remove_temp_dir(temp_dir);

    if (g_failures) {
        return 1;
    }
    puts("ok: invalid Theron save headers stay unavailable and non-launchable");
    return 0;
}
