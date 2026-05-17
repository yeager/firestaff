/*
 * extract_all_sprites.c — Extract all graphics from GRAPHICS.DAT as PNG
 * Usage: extract_all_sprites <GRAPHICS.DAT> [output_dir] [game_name]
 */
#include "firestaff_graphics_dat_reader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

static void mkdirp(const char *path) {
    char tmp[1024];
    snprintf(tmp, sizeof(tmp), "%s", path);
    for (char *p = tmp + 1; *p; p++) {
        if (*p == '/') { *p = 0; mkdir(tmp, 0755); *p = '/'; }
    }
    mkdir(tmp, 0755);
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <GRAPHICS.DAT> [output_dir] [game_name]\n", argv[0]);
        return 1;
    }
    const char *gfx_path = argv[1];
    const char *out_dir = argc > 2 ? argv[2] : "./extracted_sprites";
    const char *game_name = argc > 3 ? argv[3] : "unknown";

    /* Read file */
    FILE *f = fopen(gfx_path, "rb");
    if (!f) { fprintf(stderr, "Cannot open %s\n", gfx_path); return 1; }
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    uint8_t *data = (uint8_t *)malloc((size_t)sz);
    if (!data || (long)fread(data, 1, (size_t)sz, f) != sz) {
        fprintf(stderr, "Read error\n"); fclose(f); return 1;
    }
    fclose(f);

    /* Parse */
    FS_GraphicsDat gfx;
    if (fs_gfx_load(&gfx, data, (int)sz) < 0) {
        fprintf(stderr, "Failed to parse GRAPHICS.DAT\n");
        free(data); return 1;
    }
    printf("Loaded %s: %d graphics\n", gfx_path, gfx.graphic_count);

    /* Get palette */
    uint32_t palette[256];
    memset(palette, 0, sizeof(palette));
    fs_gfx_get_palette(&gfx, palette);

    mkdirp(out_dir);

    /* Open manifest */
    char mpath[1024];
    snprintf(mpath, sizeof(mpath), "%s/manifest.json", out_dir);
    FILE *mf = fopen(mpath, "w");
    if (!mf) { fprintf(stderr, "Cannot create %s\n", mpath); free(data); return 1; }
    fprintf(mf, "{\n  \"game\": \"%s\",\n  \"source\": \"%s\",\n  \"graphic_count\": %d,\n  \"graphics\": [\n",
            game_name, gfx_path, gfx.graphic_count);

    /* Work buffers */
    size_t bufsize = 4 * 1024 * 1024;
    uint8_t *indexed = (uint8_t *)malloc(bufsize);
    uint8_t *rgba = (uint8_t *)malloc(bufsize * 4);
    if (!indexed || !rgba) { fprintf(stderr, "OOM\n"); free(data); return 1; }

    int extracted = 0, skipped = 0;

    for (int i = 0; i < gfx.graphic_count; i++) {
        int w = 0, h = 0;
        int rc = fs_gfx_extract_bitmap(&gfx, i, indexed, (int)bufsize, &w, &h);
        if (rc <= 0 || w <= 0 || h <= 0) {
            w = 0; h = 0;
            rc = fs_gfx_get_bitmap(&gfx, i, indexed, (int)bufsize, &w, &h);
        }

        char filename[256];
        snprintf(filename, sizeof(filename), "sprite_%04d_%dx%d.png", i,
                 w > 0 ? w : gfx.entries[i].width,
                 h > 0 ? h : gfx.entries[i].height);

        int ok = (rc > 0 && w > 0 && h > 0);

        if (i > 0) fprintf(mf, ",\n");
        fprintf(mf, "    {\"index\": %d, \"filename\": \"%s\", \"width\": %d, \"height\": %d, "
                "\"compressed_size\": %d, \"offset\": %d, \"extracted\": %s}",
                i, filename, gfx.entries[i].width, gfx.entries[i].height,
                gfx.entries[i].compressed_size, gfx.entries[i].offset,
                ok ? "true" : "false");

        if (!ok) { skipped++; continue; }

        /* Indexed → RGBA */
        int npix = w * h;
        if (npix > (int)bufsize) { skipped++; continue; }
        for (int p = 0; p < npix; p++) {
            uint32_t c = palette[indexed[p] & 0xFF];
            rgba[p * 4 + 0] = (uint8_t)((c >> 16) & 0xFF);
            rgba[p * 4 + 1] = (uint8_t)((c >> 8)  & 0xFF);
            rgba[p * 4 + 2] = (uint8_t)(c & 0xFF);
            rgba[p * 4 + 3] = 0xFF;
        }

        char out_path[1024];
        snprintf(out_path, sizeof(out_path), "%s/%s", out_dir, filename);
        if (stbi_write_png(out_path, w, h, 4, rgba, w * 4)) {
            extracted++;
        } else {
            fprintf(stderr, "Write failed: %s\n", out_path);
            skipped++;
        }
    }

    fprintf(mf, "\n  ]\n}\n");
    fclose(mf);
    free(indexed); free(rgba); free(data);

    printf("Done: %d extracted, %d skipped\nManifest: %s\n", extracted, skipped, mpath);
    return (extracted > 0) ? 0 : 1;
}
