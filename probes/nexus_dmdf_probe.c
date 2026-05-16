
#include "nexus_v1_dmdf_model.h"
#include "nexus_v1_engine.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Probe: verify DMDF model parser against real .MNS files. */

static const char *g_test_models[] = {
    "SCORPION.MNS", "MUMMY.MNS", "DRAGON.MNS", "SKELETON.MNS",
    "GHOST.MNS", "WORM.MNS", NULL
};

int main(int argc, char **argv) {
    const char *data_dir = (argc > 1) ? argv[1] : NULL;
    Nexus_V1_Engine engine;
    int i;

    if (!data_dir) {
        const char *home = getenv("HOME");
        static char buf[512];
        if (home) { snprintf(buf, sizeof(buf), "%s/.firestaff/data/nexus", home); data_dir = buf; }
    }

    printf("=== Nexus DMDF Model Probe ===\n");

    if (nexus_v1_init(&engine, data_dir) < 0) {
        printf("No Nexus data — probe skipped\n");
        return 0;
    }

    for (i = 0; g_test_models[i]; i++) {
        int size = 0;
        uint8_t *data = nexus_v1_read_file(&engine, g_test_models[i], &size);
        if (!data) {
            printf("%s: not found\n", g_test_models[i]);
            continue;
        }

        printf("\n%s: %d bytes\n", g_test_models[i], size);

        if (nexus_v1_dmdf_is_valid(data, size)) {
            Nexus_V1_Model model;
            if (nexus_v1_dmdf_load(&model, data, size, g_test_models[i]) >= 0) {
                printf("  Magic: DMDF, size=%u, sections=%u\n",
                    model.header.file_size, model.header.section_count);
                printf("  Data offset: %u\n", model.header.data_offset);
                if (model.vertex_count > 0)
                    printf("  Vertices: %d, Faces: %d\n",
                        model.vertex_count, model.header.face_count);
                nexus_v1_dmdf_free(&model);
            }
        } else {
            printf("  Not DMDF format (first 4: %02x%02x%02x%02x)\n",
                data[0], data[1], data[2], data[3]);
        }
        free(data);
    }

    nexus_v1_shutdown(&engine);
    printf("\n=== Probe complete ===\n");
    return 0;
}

