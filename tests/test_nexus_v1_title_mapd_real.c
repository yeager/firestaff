#include "nexus_v1_title.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int failures;

static unsigned long long fnv1a64(const unsigned char *data, size_t size)
{
    unsigned long long hash = 1469598103934665603ULL;
    size_t i;
    for (i = 0; i < size; ++i) {
        hash ^= data[i];
        hash *= 1099511628211ULL;
    }
    return hash;
}

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
              title_bin + 0x0e278U, 0x8c70U,
              title_cg, cg_size, &title) == 0,
          "truncated MAPD palette table is rejected before palette reads");
    check(nexus_v1_title_decode_mapd(
              title_bin + 0x0e278U, bin_size - 0x0e278U,
              title_cg, cg_size, &title) == 1,
          "DMWeb MAPD/TIBG decoder accepts retail TITLE.BIN/TITLE.CG");
    check(title.decoded_map_count == NEXUS_V1_TITLE_MAP_COUNT &&
          title.decoded_map_source_bound == 1,
          "all five retail title maps are source-bound");
    for (map = 0; map < NEXUS_V1_TITLE_MAP_COUNT; ++map) {
        static const unsigned long long expected_pixel_fnv[] = {
            0x58181742b519fa49ULL,
            0x9d2997248a8f998fULL,
            0xd11379ef24d10f7eULL,
            0x0311888616d3d656ULL,
            0xe25c8744a702bfcdULL
        };
        size_t i, nonzero = 0;
        check(title.decoded_map_source_offsets[map] ==
                  0x40U + (uint32_t)map * 0x1c04U &&
              title.decoded_map_cell_bytes[map] ==
                  NEXUS_V1_TITLE_MAP_CELL_BYTES &&
              title.decoded_map_tile_min[map] <=
                  title.decoded_map_tile_max[map] &&
              title.decoded_map_tile_max[map] < 5249U &&
              title.decoded_map_cell_fnv1a64[map] != 0U,
              "retail title map retains bounded raw cell provenance");
        check(title.decoded_map_pixels[map] != NULL,
              "retail title map has decoded pixel storage");
        if (!title.decoded_map_pixels[map]) continue;
        for (i = 0; i < (size_t)NEXUS_V1_TITLE_MAP_WIDTH *
                         NEXUS_V1_TITLE_MAP_HEIGHT; ++i) {
            if (title.decoded_map_pixels[map][i] != 0) ++nonzero;
        }
        check(nonzero > 0, "retail title map contains decoded tile pixels");
        check(fnv1a64(title.decoded_map_pixels[map],
                      (size_t)NEXUS_V1_TITLE_MAP_WIDTH *
                          (size_t)NEXUS_V1_TITLE_MAP_HEIGHT) ==
                  expected_pixel_fnv[map],
              "retail title map pixel receipt is stable");
    }
    for (map = 0; map < 16; ++map)
        check(title.decoded_map_palette[map] != 0,
              "retail MAPD palette word is populated");
    {
        unsigned char *broken_mapd = (unsigned char *)malloc(0x8c74U);
        check(broken_mapd != NULL, "title MAPD regression buffer allocates");
        if (broken_mapd) {
            memcpy(broken_mapd, title_bin + 0x0e278U, 0x8c74U);
            broken_mapd[0x40U + 0x1c04U] = 0;
            broken_mapd[0x40U + 0x1c04U + 1U] = 0;
            check(nexus_v1_title_decode_mapd(
                      broken_mapd, 0x8c74U, title_cg, cg_size, &title) == 0,
                  "corrupt later title map is rejected");
            check(title.decoded_map_count == 0 &&
                      title.decoded_map_source_bound == 0 &&
                      title.decoded_map_pixels[0] == NULL,
                  "failed title decode clears earlier map allocations and metadata");
            check(nexus_v1_title_decode_mapd(
                      broken_mapd, 4U, title_cg, cg_size, &title) == 0 &&
                      title.decoded_map_count == 0 &&
                      title.decoded_map_pixels[0] == NULL,
                  "undersized title input clears stale decoded state");
            free(broken_mapd);
        }
    }
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
