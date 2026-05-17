/*
 * extract_all_sprites.c — Extract all graphics from DM1 PC-34 GRAPHICS.DAT
 * Uses IMG3 decompressor (F0689_IMG_ExpandGraphicToBitmap_Compat).
 * Usage: extract_all_sprites <GRAPHICS.DAT> [output_dir] [game_name]
 */
#include "firestaff_graphics_dat_reader.h"
#include "image_expand_pc34_compat.h"
#include "image_backend_pc34_compat.h"

/* Globals needed by image_backend_pc34_compat.c */
unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;
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

    FS_GraphicsDat gfx;
    if (fs_gfx_load(&gfx, data, (int)sz) < 0) {
        fprintf(stderr, "Failed to parse GRAPHICS.DAT\n");
        free(data); return 1;
    }
    printf("Loaded %s: %d graphics\n", gfx_path, gfx.graphic_count);

    uint32_t palette[256];
    memset(palette, 0, sizeof(palette));
    fs_gfx_get_palette(&gfx, palette);

    mkdirp(out_dir);

    char mpath[1024];
    snprintf(mpath, sizeof(mpath), "%s/manifest.json", out_dir);
    FILE *mf = fopen(mpath, "w");
    if (!mf) { fprintf(stderr, "Cannot create %s\n", mpath); free(data); return 1; }
    fprintf(mf, "{\n  \"game\": \"%s\",\n  \"source\": \"%s\",\n  \"graphic_count\": %d,\n  \"graphics\": [\n",
            game_name, gfx_path, gfx.graphic_count);

    /* Large output buffer for expanded bitmap (320x200 max typical) */
    size_t bmpsize = 512 * 512;
    uint8_t *bitmap = (uint8_t *)calloc(1, bmpsize);
    uint8_t *rgba = (uint8_t *)malloc(bmpsize * 4);
    if (!bitmap || !rgba) { fprintf(stderr, "OOM\n"); free(data); return 1; }

    int extracted = 0, skipped = 0;

    for (int i = 0; i < gfx.graphic_count; i++) {
        int w = gfx.entries[i].width;
        int h = gfx.entries[i].height;
        int off = gfx.entries[i].offset;

        char filename[256];
        snprintf(filename, sizeof(filename), "sprite_%04d_%dx%d.png", i, w, h);

        int ok = 0;
        if (w > 0 && h > 0 && w <= 512 && h <= 512 && off >= 0 && off < (int)sz) {
            /* The raw graphic data at offset includes 4-byte w/h header
             * for the IMG3 decompressor. Point directly at the graphic. */
            const uint8_t *src = data + off;
            memset(bitmap, 0, bmpsize);

            /* Use IMG3 decompressor — it reads w/h from first 4 bytes of src */
            F0689_IMG_ExpandGraphicToBitmap_Compat(src, bitmap);

            /* IMG3 outputs packed nibbles: 2 pixels per byte.
             * Even pixel offset → high nibble, odd → low nibble.
             * Stride is EVEN_INTEGER(width) for row alignment. */
            int stride = (w % 2 == 0) ? w : (w + 1); /* EVEN_INTEGER */
            int npix_bytes = (stride * h + 1) / 2; /* packed bytes */
            if (npix_bytes > 0 && npix_bytes <= (int)bmpsize) {
                for (int y = 0; y < h; y++) {
                    for (int x = 0; x < w; x++) {
                        int pixidx = y * stride + x;
                        int byteidx = pixidx / 2;
                        uint8_t nibble;
                        if (pixidx % 2 == 0)
                            nibble = (bitmap[byteidx] >> 4) & 0x0F;
                        else
                            nibble = bitmap[byteidx] & 0x0F;
                        int di = (y * w + x) * 4;
                        uint32_t c = palette[nibble];
                        rgba[di + 0] = (uint8_t)((c >> 16) & 0xFF);
                        rgba[di + 1] = (uint8_t)((c >> 8) & 0xFF);
                        rgba[di + 2] = (uint8_t)(c & 0xFF);
                        rgba[di + 3] = 0xFF;
                    }
                }

                char out_path[1024];
                snprintf(out_path, sizeof(out_path), "%s/%s", out_dir, filename);
                if (stbi_write_png(out_path, w, h, 4, rgba, w * 4)) {
                    ok = 1;
                    extracted++;
                }
            }
        }

        if (i > 0) fprintf(mf, ",\n");
        fprintf(mf, "    {\"index\": %d, \"filename\": \"%s\", \"width\": %d, \"height\": %d, "
                "\"compressed_size\": %d, \"offset\": %d, \"extracted\": %s}",
                i, filename, w, h,
                gfx.entries[i].compressed_size, off,
                ok ? "true" : "false");

        if (!ok) skipped++;
    }

    fprintf(mf, "\n  ]\n}\n");
    fclose(mf);
    free(bitmap); free(rgba); free(data);

    printf("Done: %d extracted, %d skipped\nManifest: %s\n", extracted, skipped, mpath);
    return (extracted > 0) ? 0 : 1;
}
