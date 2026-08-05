#include "nexus_v1_title.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int failures;

static unsigned char *read_file(const char *root, const char *name,
                                size_t *out_size)
{
    char path[4096];
    FILE *fp;
    long size;
    unsigned char *data;
    if (snprintf(path, sizeof(path), "%s/%s", root, name) >=
        (int)sizeof(path)) return NULL;
    fp = fopen(path, "rb");
    if (!fp || fseek(fp, 0, SEEK_END) != 0) { if (fp) fclose(fp); return NULL; }
    size = ftell(fp);
    if (size <= 0 || fseek(fp, 0, SEEK_SET) != 0) { fclose(fp); return NULL; }
    data = (unsigned char *)malloc((size_t)size);
    if (!data || fread(data, 1, (size_t)size, fp) != (size_t)size) {
        free(data); fclose(fp); return NULL;
    }
    fclose(fp);
    *out_size = (size_t)size;
    return data;
}

static void check(int condition, const char *message)
{
    if (!condition) { fprintf(stderr, "FAIL: %s\n", message); ++failures; }
}

int main(void)
{
    const char *root = getenv("FIRESTAFF_NEXUS_DATA_DIR");
    unsigned char *title_bin;
    unsigned char *title_cg;
    size_t bin_size = 0, cg_size = 0;
    Nexus_TitleScreen title;
    int map;

    if (!root || !*root) root = ".firestaff/data/nexus";
    title_bin = read_file(root, "TITLE.BIN", &bin_size);
    title_cg = read_file(root, "TITLE.CG", &cg_size);
    if (!title_bin || !title_cg || bin_size <= 0x0e278U) {
        free(title_bin); free(title_cg);
        puts("SKIP: Nexus TITLE.BIN/TITLE.CG corpus unavailable");
        return 77;
    }
    memset(&title, 0, sizeof(title));
    check(nexus_v1_title_decode_mapd(
              title_bin + 0x0e278U, bin_size - 0x0e278U,
              title_cg, cg_size, &title) == 1,
          "DMWeb MAPD/TIBG decoder accepts retail TITLE.BIN/TITLE.CG");
    check(title.decoded_map_count == NEXUS_V1_TITLE_MAP_COUNT &&
          title.decoded_map_source_bound == 1,
          "all five retail title maps are source-bound");
    for (map = 0; map < NEXUS_V1_TITLE_MAP_COUNT; ++map) {
        size_t i, nonzero = 0;
        check(title.decoded_map_pixels[map] != NULL,
              "retail title map has decoded pixel storage");
        if (!title.decoded_map_pixels[map]) continue;
        for (i = 0; i < (size_t)NEXUS_V1_TITLE_MAP_WIDTH *
                         NEXUS_V1_TITLE_MAP_HEIGHT; ++i) {
            if (title.decoded_map_pixels[map][i] != 0) ++nonzero;
        }
        check(nonzero > 0, "retail title map contains decoded tile pixels");
    }
    for (map = 0; map < 16; ++map)
        check(title.decoded_map_palette[map] != 0,
              "retail MAPD palette word is populated");
    title.pixels = (unsigned char *)malloc(1U);
    check(title.pixels != NULL, "title presentation sentinel allocates");
    title.width = NEXUS_V1_TITLE_MAP_WIDTH;
    title.height = NEXUS_V1_TITLE_MAP_HEIGHT;
    title.loaded = 1;
    {
        Nexus_V1_TitleRenderPlan plan;
        check(nexus_v1_title_build_render_plan(&title, 0, &plan) == 0,
              "raw TITLE.CG atlas is blocked after MAPD source decode");
    }
    free(title.pixels);
    title.pixels = NULL;
    nexus_title_free(&title);
    free(title_bin); free(title_cg);
    if (failures) return 1;
    puts("Nexus TITLE.BIN MAPD/TIBG real-data checks passed");
    return 0;
}
