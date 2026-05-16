
#include "firestaff_bitmap_proof.h"
#include "firestaff_asset_pipeline.h"
#include "firestaff_graphics_dat_reader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Proof: extract wall bitmap from GRAPHICS.DAT and write as PPM. */

int fs_bitmap_proof_run(const char *data_dir, const char *output_path) {
    FS_AssetBundle bundle;
    FS_GraphicsDat gfx;
    uint32_t palette[256];
    uint8_t pixels[65536];
    int w = 0, h = 0, got, i;
    FILE *f;

    if (!data_dir || !output_path) {
        fprintf(stderr, "Usage: firestaff_bitmap_proof DATA_DIR OUTPUT.ppm\n");
        return 1;
    }

    printf("Loading GRAPHICS.DAT from %s...\n", data_dir);
    if (fs_assets_load_dm1(&bundle, data_dir) < 0) {
        fprintf(stderr, "Failed to load GRAPHICS.DAT\n");
        return 1;
    }

    if (fs_gfx_load(&gfx, bundle.graphics_data, bundle.graphics_size) < 0) {
        fprintf(stderr, "Failed to parse GRAPHICS.DAT\n");
        fs_assets_free(&bundle);
        return 1;
    }

    printf("GRAPHICS.DAT: %d graphics\n", gfx.graphic_count);

    /* Get palette */
    fs_gfx_get_palette(&gfx, palette);

    /* Try several graphic indices to find a visible wall bitmap */
    int test_indices[] = {33, 34, 35, 30, 31, 32, 40, 50, 60, 100, 120, 1, 2, 3};
    int best_idx = -1, best_size = 0;

    for (i = 0; i < (int)(sizeof(test_indices)/sizeof(test_indices[0])); i++) {
        int idx = test_indices[i];
        if (idx >= gfx.graphic_count) continue;
        int tw = 0, th = 0;
        got = fs_gfx_extract_bitmap(&gfx, idx, pixels, sizeof(pixels), &tw, &th);
        if (got > 0 && tw > 4 && th > 4 && tw * th > best_size) {
            best_idx = idx;
            best_size = tw * th;
            w = tw; h = th;
            printf("  Graphic #%d: %dx%d (%d bytes)\n", idx, tw, th, got);
        }
    }

    if (best_idx < 0) {
        fprintf(stderr, "No usable graphic found\n");
        fs_assets_free(&bundle);
        return 1;
    }

    /* Re-extract the best one */
    got = fs_gfx_extract_bitmap(&gfx, best_idx, pixels, sizeof(pixels), &w, &h);
    printf("\nExtracting graphic #%d: %dx%d\n", best_idx, w, h);

    /* Write PPM */
    f = fopen(output_path, "wb");
    if (!f) {
        fprintf(stderr, "Cannot write %s\n", output_path);
        fs_assets_free(&bundle);
        return 1;
    }

    fprintf(f, "P6\n%d %d\n255\n", w, h);
    for (i = 0; i < w * h; i++) {
        uint32_t rgba = palette[pixels[i] & 0xFF];
        uint8_t rgb[3] = {
            (uint8_t)((rgba >> 16) & 0xFF),
            (uint8_t)((rgba >> 8) & 0xFF),
            (uint8_t)(rgba & 0xFF)
        };
        fwrite(rgb, 1, 3, f);
    }
    fclose(f);

    printf("Written: %s (%dx%d)\n", output_path, w, h);
    printf("\nPROOF: GRAPHICS.DAT bitmap extraction works.\n");

    fs_assets_free(&bundle);
    return 0;
}

/* Standalone main for probe */
#ifndef FS_BITMAP_PROOF_NO_MAIN
int main(int argc, char **argv) {
    if (argc < 3) {
        printf("Usage: %s DATA_DIR OUTPUT.ppm\n", argv[0]);
        printf("Example: %s ~/.openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34/DATA wall_proof.ppm\n", argv[0]);
        return 1;
    }
    return fs_bitmap_proof_run(argv[1], argv[2]);
}
#endif
