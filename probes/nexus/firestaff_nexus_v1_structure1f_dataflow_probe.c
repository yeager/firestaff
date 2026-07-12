/*
 * Original Nexus DGN Structure1F direct-coordinate dataflow receipt.
 *
 * This reads the real LEV corpus and compares only documented direct 64x64
 * coordinate bytes with Firestaff's typed Structure1F records. It does not
 * assign object, sensor, trigger, draw, or gameplay semantics to a record.
 */

#include "nexus_v1_dungeon.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int failures;

static void check(int condition, const char *message) {
    if (condition) printf("PASS: %s\n", message);
    else { fprintf(stderr, "FAIL: %s\n", message); ++failures; }
}

static int read_file(const char *path, uint8_t **out_data, int *out_size) {
    FILE *file;
    long length;
    uint8_t *data;

    if (!path || !out_data || !out_size) return 0;
    *out_data = NULL;
    *out_size = 0;
    file = fopen(path, "rb");
    if (!file || fseek(file, 0L, SEEK_END) != 0 ||
        (length = ftell(file)) <= 0L || fseek(file, 0L, SEEK_SET) != 0) {
        if (file) fclose(file);
        return 0;
    }
    data = (uint8_t *)malloc((size_t)length);
    if (!data || fread(data, 1U, (size_t)length, file) != (size_t)length) {
        free(data);
        fclose(file);
        return 0;
    }
    fclose(file);
    *out_data = data;
    *out_size = (int)length;
    return 1;
}

int main(int argc, char **argv) {
    const char *data_dir = argc > 1 ? argv[1] : NULL;
    char default_dir[1024];
    const char *home;
    int level_index;
    int levels_loaded = 0;
    int direct_record_count = 0;
    int direct_record_mismatch_count = 0;

    if (!data_dir) {
        home = getenv("HOME");
        if (!home || snprintf(default_dir, sizeof(default_dir),
                              "%s/.firestaff/data/nexus", home) <= 0) {
            puts("SKIP: no Nexus data directory argument or HOME");
            return 0;
        }
        data_dir = default_dir;
    }
    for (level_index = 0; level_index < 16; ++level_index) {
        char path[1200];
        uint8_t *data = NULL;
        int size = 0;
        Nexus_V1_DgnStructure1Layout layout;
        Nexus_V1_Level level;
        int family;
        int typed_index = 0;

        memset(&layout, 0, sizeof(layout));
        memset(&level, 0, sizeof(level));
        if (snprintf(path, sizeof(path), "%s/LEV%02d.DGN", data_dir,
                     level_index) <= 0 || !read_file(path, &data, &size)) {
            fprintf(stderr, "FAIL: LEV%02d.DGN is unavailable\n", level_index);
            ++failures;
            continue;
        }
        check(nexus_v1_dgn_structure1_layout(&layout, data, size) == 0 &&
                  layout.valid && layout.structure1f.valid &&
                  nexus_v1_level_load(&level, data, size, level_index) == 0 &&
                  level.structure1f_entry_count ==
                      layout.structure1f.total_entry_count,
              "real DGN Structure1F layout reaches typed runtime records");
        for (family = 0; family < NEXUS_DGN_STRUCTURE1F_FAMILY_COUNT; ++family) {
            int record;
            const uint8_t *raw = data + layout.structure1_offset +
                layout.structure1f.family_offset[family];
            for (record = 0; record < layout.structure1f.family_count[family];
                 ++record, ++typed_index) {
                const uint8_t *source = raw + record *
                    layout.structure1f.family_record_size[family];
                const Nexus_V1_DgnStructure1FEntry *typed =
                    &level.structure1f_entries[typed_index];
                if (family > NEXUS_V1_DGN_STRUCTURE1F_FLOOR_SENSORS) continue;
                ++direct_record_count;
                if (typed->family != (Nexus_V1_DgnStructure1FFamily)family ||
                    typed->x != source[1] || typed->y != source[2] ||
                    typed->x >= NEXUS_MAX_MAP_SIZE ||
                    typed->y >= NEXUS_MAX_MAP_SIZE) {
                    ++direct_record_mismatch_count;
                }
            }
        }
        ++levels_loaded;
        free(data);
    }
    check(levels_loaded == 16, "all 16 original LEV DGN files load");
    check(direct_record_count > 0 && direct_record_mismatch_count == 0,
          "Structure1F direct coordinates match typed runtime records");
    printf("Nexus DGN Structure1F direct flow: levels=%d direct-records=%d "
           "mismatches=%d; object/sensor/render-proof=0\n",
           levels_loaded, direct_record_count, direct_record_mismatch_count);
    return failures == 0 ? 0 : 1;
}
