
#include <stdio.h>
#include <string.h>
#include "nexus_v1_automap.h"

static int g_fail;
static void expect(int c, const char *m) {
    if (!c) { fprintf(stderr, "FAIL: %s\n", m); g_fail++; }
}

int main(void) {
    /* Test 1: default palette has working values */
    {
        Nexus_AutomapRenderConfig cfg;
        nexus_v1_automap_default_config(&cfg);
        expect(cfg.cell_size == 4, "default cell_size is 4");
        expect(cfg.wall_color != 0, "default wall_color is set");
        expect(cfg.floor_color != 0, "default floor_color is set");
        expect(cfg.party_color != 0, "default party_color is set");
        expect(cfg.bg_color != 0, "default bg_color is set");
    }

    /* Test 2: explored floor cell is rendered */
    {
        Nexus_Automap map;
        uint8_t sq[NEXUS_MAX_MAP_SIZE][NEXUS_MAX_MAP_SIZE];
        Nexus_AutomapRenderConfig cfg;
        uint32_t pixels[16 * 16];
        int drawn;
        nexus_v1_automap_init(&map);
        memset(sq, 0, sizeof(sq));
        sq[2][3] = 1;
        nexus_v1_automap_reveal(&map, 0, 3, 2);
        nexus_v1_automap_default_config(&cfg);
        cfg.cell_size = 2;
        memset(pixels, 0x5a, sizeof(pixels));
        drawn = nexus_v1_automap_render(&map, sq, -1, -1, &cfg,
                                         pixels, 16, 16);
        expect(drawn > 0, "render produces nonzero drawn pixels");
    }

    /* Test 3: unexplored cell is cleared to zero (render clears framebuffer) */
    {
        Nexus_Automap map;
        uint8_t sq[NEXUS_MAX_MAP_SIZE][NEXUS_MAX_MAP_SIZE];
        Nexus_AutomapRenderConfig cfg;
        uint32_t pixels[16 * 16];
        nexus_v1_automap_init(&map);
        memset(sq, 1, sizeof(sq));
        nexus_v1_automap_default_config(&cfg);
        cfg.cell_size = 1;
        memset(pixels, 0x5a, sizeof(pixels));
        nexus_v1_automap_render(&map, sq, -1, -1, &cfg, pixels, 16, 16);
        expect(pixels[0] == 0, "render clears unexplored cells to zero");
    }

    /* Test 4: explored wall cell renders with wall color */
    {
        Nexus_Automap map;
        uint8_t sq[NEXUS_MAX_MAP_SIZE][NEXUS_MAX_MAP_SIZE];
        Nexus_AutomapRenderConfig cfg;
        uint32_t pixels[16 * 16];
        nexus_v1_automap_init(&map);
        memset(sq, 0, sizeof(sq));
        nexus_v1_automap_reveal(&map, 0, 1, 1);
        nexus_v1_automap_default_config(&cfg);
        cfg.cell_size = 1;
        memset(pixels, 0x5a, sizeof(pixels));
        nexus_v1_automap_render(&map, sq, -1, -1, &cfg, pixels, 16, 16);
        expect(pixels[1 * 16 + 1] == cfg.wall_color, "wall cell renders with wall color");
    }

    /* Test 5: party marker renders at party position */
    {
        Nexus_Automap map;
        uint8_t sq[NEXUS_MAX_MAP_SIZE][NEXUS_MAX_MAP_SIZE];
        Nexus_AutomapRenderConfig cfg;
        uint32_t pixels[32 * 32];
        nexus_v1_automap_init(&map);
        memset(sq, 1, sizeof(sq));
        nexus_v1_automap_reveal(&map, 0, 3, 3);
        nexus_v1_automap_default_config(&cfg);
        cfg.cell_size = 2;
        memset(pixels, 0, sizeof(pixels));
        nexus_v1_automap_render(&map, sq, 3, 3, &cfg, pixels, 32, 32);
        /* Party marker center: cx = 3*2 + 1 = 7, cy = 3*2 + 1 = 7 */
        expect(pixels[7 * 32 + 7] == cfg.party_color,
               "party marker renders at party position");
    }

    /* Test 6: NULL safety */
    {
        expect(nexus_v1_automap_render(NULL, NULL, 0, 0, NULL, NULL, 0, 0) == 0,
               "NULL render returns 0");
        nexus_v1_automap_default_config(NULL);
        expect(1, "NULL config safe");
    }

    /* Test 7: zero-size buffer */
    {
        Nexus_Automap map;
        Nexus_AutomapRenderConfig cfg;
        nexus_v1_automap_init(&map);
        nexus_v1_automap_default_config(&cfg);
        expect(nexus_v1_automap_render(&map, NULL, 0, 0, &cfg, NULL, 0, 0) == 0,
               "zero-size buffer returns 0");
    }

    if (g_fail) {
        fprintf(stderr, "%d failures\n", g_fail);
        return 1;
    }
    printf("ok: Nexus automap renderer verified\n");
    return 0;
}
