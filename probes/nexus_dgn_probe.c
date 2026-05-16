
#include "nexus_v1_dungeon.h"
#include "nexus_v1_engine.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Probe: verify DGN parser against real Nexus level data.
 * Reads LEV00.DGN and reports header interpretation. */

int main(int argc, char **argv) {
    const char *data_dir = (argc > 1) ? argv[1] : NULL;
    Nexus_V1_Engine engine;
    uint8_t *data;
    int size = 0;
    Nexus_V1_Level level;

    if (!data_dir) {
        /* Try default */
        const char *home = getenv("HOME");
        static char buf[512];
        if (home) {
            snprintf(buf, sizeof(buf), "%s/.firestaff/data/nexus", home);
            data_dir = buf;
        }
    }

    printf("=== Nexus DGN Probe ===\n");
    printf("Data dir: %s\n", data_dir ? data_dir : "(none)");

    if (nexus_v1_init(&engine, data_dir) < 0) {
        printf("No Nexus data found — probe skipped\n");
        return 0;
    }

    /* Read LEV00.DGN */
    data = nexus_v1_read_file(&engine, "LEV00.DGN", &size);
    if (!data) {
        printf("Cannot read LEV00.DGN\n");
        nexus_v1_shutdown(&engine);
        return 1;
    }

    printf("LEV00.DGN: %d bytes\n", size);
    printf("First 16 bytes: ");
    for (int i = 0; i < 16 && i < size; i++)
        printf("%02x ", data[i]);
    printf("\n");

    /* Try parsing */
    if (nexus_v1_level_load(&level, data, size, 0) >= 0) {
        printf("Parsed: %dx%d, geometry=%d bytes\n",
            level.width, level.height, level.geometry_size);

        /* Count non-zero squares */
        int open = 0, walls = 0;
        for (int y = 0; y < level.height; y++)
            for (int x = 0; x < level.width; x++) {
                if (level.squares[y][x] != 0) open++;
                else walls++;
            }
        printf("Squares: %d open, %d walls\n", open, walls);
    }

    free(data);
    nexus_v1_shutdown(&engine);
    printf("=== Probe complete ===\n");
    return 0;
}

