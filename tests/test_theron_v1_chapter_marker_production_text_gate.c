#include "theron_v1_chapter_marker.h"
#include "theron_v1_world.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RAW_SECTOR_SIZE 2352u
#define USER_SECTOR_SIZE 2048u
#define USER_DATA_OFFSET 16u

int theron_v1_save_enum_slots(const char *save_root,
                              Theron_SaveSlot *slots,
                              int max_slots) {
    (void)save_root;
    (void)slots;
    (void)max_slots;
    return 0;
}

int theron_v1_world_quest_item_name_raw(
    const Theron_V1_World *world,
    unsigned int quest_index,
    const uint8_t **out_bytes,
    size_t *out_size) {
    if (out_bytes) *out_bytes = NULL;
    if (out_size) *out_size = 0u;
    if (!world || !out_bytes || !out_size || quest_index >= 7u)
        return 0;
    return theron_v1_track02_quest_item_name_raw(
        &world->track02_item_names[quest_index], out_bytes, out_size);
}

static uint8_t *load_user_data(const char *path, size_t *out_size) {
    FILE *file = fopen(path, "rb");
    long raw_size;
    size_t sectors, i;
    uint8_t *raw, *user_data;
    if (!file || fseek(file, 0, SEEK_END) != 0 ||
        (raw_size = ftell(file)) <= 0 || fseek(file, 0, SEEK_SET) != 0) {
        if (file) fclose(file);
        return NULL;
    }
    raw = (uint8_t *)malloc((size_t)raw_size);
    if (!raw || fread(raw, 1u, (size_t)raw_size, file) != (size_t)raw_size) {
        free(raw);
        fclose(file);
        return NULL;
    }
    fclose(file);
    sectors = (size_t)raw_size / RAW_SECTOR_SIZE;
    user_data = (uint8_t *)malloc(sectors * USER_SECTOR_SIZE);
    if (!user_data) {
        free(raw);
        return NULL;
    }
    for (i = 0u; i < sectors; ++i)
        memcpy(user_data + i * USER_SECTOR_SIZE,
               raw + i * RAW_SECTOR_SIZE + USER_DATA_OFFSET,
               USER_SECTOR_SIZE);
    free(raw);
    *out_size = sectors * USER_SECTOR_SIZE;
    return user_data;
}

static int bind_real_bank(Theron_V1_World *world,
                          const char *path,
                          int variant) {
    Theron_Track02ItemNameSource source;
    size_t user_data_size = 0u;
    uint8_t *user_data = load_user_data(path, &user_data_size);
    int ok = user_data && theron_v1_track02_decode_item_name_source(
        user_data, user_data_size, variant, 1u, &source);
    if (ok) world->track02_item_names[0] = source;
    free(user_data);
    return ok;
}

int main(void) {
    Theron_V1_BootProfile profile;
    Theron_DungeonProgression progression;
    Theron_ChapterMarker marker;
    Theron_V1_World *world;
    const char *home = getenv("HOME");
    char path[1024];

    memset(&profile, 0, sizeof(profile));
    profile.assets_verified = 1;
    theron_v1_dungeon_progression_init(&progression);
    progression.quest_items_collected = THERON_QUEST_ITEM_1_SHIELD_DEFIANT;
    if (theron_v1_chapter_marker_compute(
            &profile, &progression, NULL, &marker) != 0 ||
        strstr(marker.quest_summary, "source name unavailable") == NULL ||
        strstr(marker.quest_summary, "Shield Defiant") != NULL ||
        strstr(marker.quest_summary, "Taza") != NULL ||
        strstr(marker.quest_summary, "Soulcage") != NULL ||
        strstr(marker.quest_summary, "Retaliator") != NULL) {
        fputs("FAIL: production chapter marker published static quest text\n",
              stderr);
        return 1;
    }
    if (!home || !(world = (Theron_V1_World *)calloc(1u, sizeof(*world))))
        return 1;
    world->progression = progression;
    world->progression.quest_items_collected = 0u;
    snprintf(path, sizeof(path), "%s/.firestaff/data/theron/TQUS02.bin", home);
    {
        FILE *source = fopen(path, "rb");
        if (!source) {
            free(world);
            puts("SKIP: authentic US and JP Track 02 files are not staged");
            return 77;
        }
        fclose(source);
    }
    if (!bind_real_bank(world, path, 2) ||
        theron_v1_chapter_marker_compute_world(
            &profile, world, NULL, &marker) != 0 ||
        strstr(marker.quest_summary, "next: SHIELD DEFIANT") == NULL) {
        fputs("FAIL: production marker did not publish authentic US name\n",
              stderr);
        free(world);
        return 1;
    }
    snprintf(path, sizeof(path), "%s/.firestaff/data/theron/TQJP02.bin", home);
    {
        FILE *source = fopen(path, "rb");
        if (!source) {
            free(world);
            puts("SKIP: authentic US and JP Track 02 files are not staged");
            return 77;
        }
        fclose(source);
    }
    if (!bind_real_bank(world, path, 1) ||
        theron_v1_chapter_marker_compute_world(
            &profile, world, NULL, &marker) != 0 ||
        strstr(marker.quest_summary, "source name unavailable") == NULL ||
        strstr(marker.quest_summary, "SHIELD DEFIANT") != NULL) {
        fputs("FAIL: production marker treated JP Shift-JIS as host text\n",
              stderr);
        free(world);
        return 1;
    }
    free(world);
    puts("PASS: production marker uses real US text and gates JP Shift-JIS");
    return 0;
}
