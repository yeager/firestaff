#include "nexus_v1_sound.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int failures;

#define CHECK(condition, message) do { \
    if (!(condition)) { \
        fprintf(stderr, "FAIL: %s\\n", message); \
        failures++; \
    } \
} while (0)

static unsigned char *read_file(const char *path, int *out_size) {
    FILE *file;
    long size;
    unsigned char *data;

    *out_size = 0;
    file = fopen(path, "rb");
    if (!file || fseek(file, 0, SEEK_END) != 0 ||
        (size = ftell(file)) <= 0 || fseek(file, 0, SEEK_SET) != 0) {
        if (file) fclose(file);
        return NULL;
    }
    data = (unsigned char *)malloc((size_t)size);
    if (!data || fread(data, 1, (size_t)size, file) != (size_t)size) {
        free(data);
        fclose(file);
        return NULL;
    }
    fclose(file);
    *out_size = (int)size;
    return data;
}

static void test_retail_map_requires_ff_ff_terminator(void) {
    static const unsigned char sal_data[] = { 0 };
    unsigned char malformed_map[26];
    Nexus_SoundEngine sound;
    Nexus_SoundMapWindow window;

    memset(malformed_map, 0, sizeof(malformed_map));
    /* One observed-shaped record followed by a false FF 00 terminator. */
    malformed_map[0] = 0x20;
    malformed_map[1] = 0x00;
    malformed_map[2] = 0xb0;
    malformed_map[6] = 0x05;
    malformed_map[7] = 0x40;
    malformed_map[24] = 0xff;
    malformed_map[25] = 0x00;

    CHECK(nexus_sound_init(&sound) == 0,
          "malformed retail MAP sound engine initializes");
    CHECK(nexus_sound_load_canonical_level(&sound, 0, sal_data,
                                            (int)sizeof(sal_data),
                                            malformed_map,
                                            (int)sizeof(malformed_map), 1, 1) == 0,
          "malformed retail MAP loads as source bytes");
    CHECK(sound.map_record_table_supported == 0,
          "retail MAP rejects FF xx as a terminator");
    CHECK(nexus_sound_map_lookup_raw_selector(&sound, 0x20, &window) == -1,
          "malformed retail MAP cannot expose a partial window");
    nexus_sound_shutdown(&sound);
}

int main(void) {
    const char *data_dir = getenv("FIRESTAFF_NEXUS_DATA_DIR");
    int level;
    int record_total = 0;

    if (!data_dir || !data_dir[0]) {
        fprintf(stderr, "SKIP: FIRESTAFF_NEXUS_DATA_DIR is not configured\\n");
        return 77;
    }

    for (level = 0; level < 16; ++level) {
        char sal_path[1024];
        char map_path[1024];
        unsigned char *sal_data;
        unsigned char *map_data;
        int sal_size;
        int map_size;
        int record;
        Nexus_SoundEngine sound;

        snprintf(sal_path, sizeof(sal_path), "%s/SNDLEV%02d.SAL", data_dir,
                 level);
        snprintf(map_path, sizeof(map_path), "%s/SNDLEV%02d.MAP", data_dir,
                 level);
        sal_data = read_file(sal_path, &sal_size);
        map_data = read_file(map_path, &map_size);
        CHECK(sal_data != NULL && map_data != NULL, "retail SAL/MAP pair opens");
        if (!sal_data || !map_data) {
            free(sal_data);
            free(map_data);
            continue;
        }

        CHECK(nexus_sound_init(&sound) == 0, "sound engine initializes");
        CHECK(nexus_sound_load_canonical_level(&sound, level,
                                                sal_data, sal_size,
                                                map_data, map_size, 1, 1) == 0,
              "retail SAL/MAP pair loads");
        CHECK(sound.map_record_table_supported,
              "retail MAP has a terminated record table");
        CHECK(sound.map_record_count > 0 &&
              sound.map_record_count <= NEXUS_SFX_MAP_MAX_RECORDS,
              "retail MAP record count stays bounded");
        record_total += sound.map_record_count;
        CHECK(sound.map_out_of_bounds_record_count == 0,
              "every retail MAP window is inside its SAL bank");
        CHECK(sound.map_record_terminator_offset ==
              sound.map_record_count * 8,
              "retail MAP terminator follows complete eight-byte records");

        for (record = 0; record < sound.map_record_count; ++record) {
            Nexus_SoundMapWindow window;
            const Nexus_SoundMapWindow *stored = &sound.map_records[record];
            if (stored->data_id == 0) {
                CHECK(nexus_sound_map_lookup_raw_selector(&sound, stored->selector,
                                                           &window) == 0,
                      "retail DataID 0 selector resolves to a bounded MAP window");
                CHECK(window.selector == stored->selector &&
                      window.attribute == stored->attribute &&
                      window.sal_offset == stored->sal_offset &&
                      window.sal_size == stored->sal_size,
                      "retail DataID 0 route preserves MAP fields");
            }
        }
        {
            Nexus_SoundMapWindow window;
            CHECK(nexus_sound_map_lookup_raw_selector(&sound, 0xff, &window) == -1,
                  "missing selector does not synthesize a SAL window");
        }

        nexus_sound_shutdown(&sound);
        free(sal_data);
        free(map_data);
    }

    CHECK(record_total == 154,
          "retail SNDLEV00-15 MAP corpus exposes 154 bounded records");

    test_retail_map_requires_ff_ff_terminator();

    return failures ? 1 : 0;
}
