/*
 * theron_v1_vram_trace_loader.c — Load real PCE VRAM/VCE snapshots
 *
 * Loads binary VRAM (64KB) and VCE (1KB) dumps captured from Mednafen
 * save states into the viewport's tile/palette system. This enables
 * rendering with authentic game data instead of synthetic placeholders.
 *
 * VRAM layout (word-addressed, 64KB = 32K words):
 *   $0000-$07FF: BAT (Background Attribute Table) — 64×32 tile map
 *   $0800-$7EFF: BG tile data (4bpp, 32 bytes per 8×8 tile)
 *   $7F00-$7FFF: SAT (Sprite Attribute Table) — 64 entries
 *
 * VCE layout: 512 × 16-bit LE words, BGR333 format.
 */

#include "theron_v1_vram_trace_loader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int theron_v1_vram_trace_load_raw(Theron_V1_Viewport *vp,
                                  const uint8_t *vram_data, int vram_size,
                                  const uint8_t *vce_data, int vce_size) {
    if (!vp || !vram_data || !vce_data) return -1;
    if (vram_size < THERON_VRAM_SIZE || vce_size < THERON_VCE_SIZE) return -1;

    if (!vp->vram_trace_data) {
        vp->vram_trace_data = (uint8_t *)malloc(THERON_VRAM_SIZE);
        if (!vp->vram_trace_data) return -1;
    }
    if (!vp->vce_trace_data) {
        vp->vce_trace_data = (uint8_t *)malloc(THERON_VCE_SIZE);
        if (!vp->vce_trace_data) {
            free(vp->vram_trace_data);
            vp->vram_trace_data = NULL;
            return -1;
        }
    }

    memcpy(vp->vram_trace_data, vram_data, THERON_VRAM_SIZE);
    memcpy(vp->vce_trace_data, vce_data, THERON_VCE_SIZE);

    for (int i = 0; i < 2048; ++i)
        vp->bat_atlas_indices[i] = -1;

    tqr_palette_load_group(&vp->palette, vce_data, 0, 512);

    vp->vram_trace_loaded = 1;
    return 0;
}

int theron_v1_vram_trace_load_files(Theron_V1_Viewport *vp,
                                    const char *vram_path,
                                    const char *vce_path) {
    if (!vp || !vram_path || !vce_path) return -1;

    FILE *fv = fopen(vram_path, "rb");
    if (!fv) return -1;
    fseek(fv, 0, SEEK_END);
    long vram_sz = ftell(fv);
    if (vram_sz < THERON_VRAM_SIZE) { fclose(fv); return -1; }
    fseek(fv, 0, SEEK_SET);

    uint8_t *vram = (uint8_t *)malloc(THERON_VRAM_SIZE);
    if (!vram) { fclose(fv); return -1; }
    if (fread(vram, 1, THERON_VRAM_SIZE, fv) != THERON_VRAM_SIZE) {
        free(vram); fclose(fv); return -1;
    }
    fclose(fv);

    FILE *fc = fopen(vce_path, "rb");
    if (!fc) { free(vram); return -1; }
    fseek(fc, 0, SEEK_END);
    long vce_sz = ftell(fc);
    if (vce_sz < THERON_VCE_SIZE) { free(vram); fclose(fc); return -1; }
    fseek(fc, 0, SEEK_SET);

    uint8_t *vce = (uint8_t *)malloc(THERON_VCE_SIZE);
    if (!vce) { free(vram); fclose(fc); return -1; }
    if (fread(vce, 1, THERON_VCE_SIZE, fc) != THERON_VCE_SIZE) {
        free(vram); free(vce); fclose(fc); return -1;
    }
    fclose(fc);

    int rc = theron_v1_vram_trace_load_raw(vp, vram, THERON_VRAM_SIZE,
                                           vce, THERON_VCE_SIZE);
    free(vram);
    free(vce);
    return rc;
}

int theron_v1_vram_trace_load_tqtr(Theron_V1_Viewport *vp,
                                   const char *tqtr_path) {
    if (!vp || !tqtr_path) return -1;

    FILE *f = fopen(tqtr_path, "rb");
    if (!f) return -1;

    /* TQTR header: "TQTR" (4) + version (4) + screen_type (4) +
     * vram_size (4) + vce_size (4) + cdram_size (4) + sysram_size (4) = 28 */
    uint8_t hdr[28];
    if (fread(hdr, 1, 28, f) != 28) { fclose(f); return -1; }
    if (memcmp(hdr, "TQTR", 4) != 0) { fclose(f); return -1; }

    uint32_t vram_sz = hdr[12] | (hdr[13] << 8) | (hdr[14] << 16) | (hdr[15] << 24);
    uint32_t vce_sz  = hdr[16] | (hdr[17] << 8) | (hdr[18] << 16) | (hdr[19] << 24);

    if (vram_sz < THERON_VRAM_SIZE || vce_sz < THERON_VCE_SIZE) {
        fclose(f); return -1;
    }

    uint8_t *vram = (uint8_t *)malloc(THERON_VRAM_SIZE);
    uint8_t *vce  = (uint8_t *)malloc(THERON_VCE_SIZE);
    if (!vram || !vce) { free(vram); free(vce); fclose(f); return -1; }

    if (fread(vram, 1, THERON_VRAM_SIZE, f) != THERON_VRAM_SIZE ||
        fread(vce, 1, THERON_VCE_SIZE, f) != THERON_VCE_SIZE) {
        free(vram); free(vce); fclose(f); return -1;
    }
    fclose(f);

    int rc = theron_v1_vram_trace_load_raw(vp, vram, THERON_VRAM_SIZE,
                                           vce, THERON_VCE_SIZE);
    free(vram);
    free(vce);
    return rc;
}

void theron_v1_vram_trace_unload(Theron_V1_Viewport *vp) {
    if (!vp) return;
    free(vp->vram_trace_data);
    free(vp->vce_trace_data);
    vp->vram_trace_data = NULL;
    vp->vce_trace_data = NULL;
    vp->vram_trace_loaded = 0;
    for (int i = 0; i < 2048; ++i)
        vp->bat_atlas_indices[i] = -1;
    tqr_palette_free_tiles(&vp->palette);
}

int theron_v1_vram_trace_populate_tiles(Theron_V1_Viewport *vp,
                                        int bat_start_word,
                                        int bat_w, int bat_h) {
    if (!vp || !vp->vram_trace_loaded || !vp->vram_trace_data) return -1;
    /* The PCE BAT occupies 2048 words (64 columns × 32 rows).  The
     * current atlas population still loads the complete authenticated BG
     * tile span, but reject an invalid caller window before treating the
     * trace as a usable VRAM binding. */
    if (bat_start_word < 0 || bat_w <= 0 || bat_h <= 0 ||
        bat_w > 64 || bat_h > 32 || bat_start_word >= 2048 ||
        bat_start_word + (bat_h - 1) * 64 + (bat_w - 1) >= 2048) {
        return -1;
    }

    const uint8_t *vram = vp->vram_trace_data;
    int tile_map[2048][TQR_PALETTE_GROUPS];

    /* A BAT word is the source-owned VDC mapping: bits 0..10 select the
     * background tile and bits 12..15 select its palette group.  Keep one
     * atlas entry per (tile, palette) pair because the same source tile can
     * be displayed with multiple VCE groups.  Do not scan the whole VRAM or
     * invent group 0 for every non-zero tile; that loses the actual scene
     * binding carried by the captured BAT. */
    for (int tile = 0; tile < 2048; ++tile) {
        for (int group = 0; group < TQR_PALETTE_GROUPS; ++group) {
            tile_map[tile][group] = -1;
        }
    }

    /* BG tiles start at VRAM word $0800 = byte offset $1000 */
    int tile_base_byte = 0x1000;
    tqr_palette_free_tiles(&vp->palette);
    for (int i = 0; i < 2048; ++i)
        vp->bat_atlas_indices[i] = -1;

    int loaded = 0;
    for (int y = 0; y < bat_h; ++y) {
        for (int x = 0; x < bat_w; ++x) {
            int bat_word = bat_start_word + y * 64 + x;
            uint16_t bat = (uint16_t)vram[bat_word * 2] |
                           ((uint16_t)vram[bat_word * 2 + 1] << 8);
            int tile_index = (int)(bat & 0x07FFu);
            int pal_group = (int)((bat >> 12) & 0x0Fu);
            int off = tile_base_byte + tile_index * THERON_VRAM_TILE_BYTES;
            int atlas_index;

            if (off < tile_base_byte ||
                off + THERON_VRAM_TILE_BYTES > 0xFE00 ||
                tile_map[tile_index][pal_group] >= 0) {
                if (off >= tile_base_byte &&
                    off + THERON_VRAM_TILE_BYTES <= 0xFE00 &&
                    tile_map[tile_index][pal_group] >= 0) {
                    vp->bat_atlas_indices[bat_word] =
                        (int16_t)tile_map[tile_index][pal_group];
                }
                continue;
            }
            atlas_index = tqr_tile_load_from_data(
                &vp->palette, vram + off, 4, pal_group, "vram_trace");
            if (atlas_index < 0) return -1;
            vp->palette.tiles[atlas_index].vram_index = (uint16_t)tile_index;
            tile_map[tile_index][pal_group] = atlas_index;
            vp->bat_atlas_indices[bat_word] = (int16_t)atlas_index;
            ++loaded;
        }
    }

    return loaded;
}

int theron_v1_vram_trace_bat_atlas_index(const Theron_V1_Viewport *vp,
                                         int bat_word) {
    if (!vp || !vp->vram_trace_loaded || bat_word < 0 || bat_word >= 2048)
        return -1;
    return vp->bat_atlas_indices[bat_word];
}

int theron_v1_vram_trace_render_bat_preview(Theron_V1_Viewport *vp,
                                            int bat_start_word,
                                            int bat_w,
                                            int bat_h,
                                            int dst_x,
                                            int dst_y) {
    int copied = 0;

    if (!vp || !vp->vram_trace_loaded || !vp->fb.data ||
        bat_start_word < 0 || bat_w <= 0 || bat_h <= 0 ||
        bat_w > 64 || bat_h > 32 || bat_start_word >= 2048 ||
        bat_start_word + (bat_h - 1) * 64 + (bat_w - 1) >= 2048 ||
        dst_x < 0 || dst_y < 0 || dst_x + bat_w * TQR_TILE_DIM > vp->fb.w ||
        dst_y + bat_h * TQR_TILE_DIM > vp->fb.h) {
        return -1;
    }

    for (int y = 0; y < bat_h; ++y) {
        for (int x = 0; x < bat_w; ++x) {
            int bat_word = bat_start_word + y * 64 + x;
            int atlas_index = vp->bat_atlas_indices[bat_word];
            const TQR_Tile *tile;

            if (atlas_index < 0 || atlas_index >= vp->palette.tile_count)
                continue;
            tile = &vp->palette.tiles[atlas_index];
            if (!tile->data) continue;
            for (int row = 0; row < TQR_TILE_DIM; ++row) {
                memcpy(vp->fb.data +
                           (dst_y + y * TQR_TILE_DIM + row) * vp->fb.stride +
                           dst_x + x * TQR_TILE_DIM,
                       tile->data + row * TQR_TILE_DIM,
                       TQR_TILE_DIM);
            }
            ++copied;
        }
    }
    return copied;
}
