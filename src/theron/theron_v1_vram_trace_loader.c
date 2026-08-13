/*
 * theron_v1_vram_trace_loader.c — Load real PCE VRAM/VCE snapshots
 *
 * Loads binary VRAM (64KB) and VCE (1KB) dumps captured from Mednafen
 * save states into the viewport's tile/palette system. This enables
 * rendering with authentic game data instead of synthetic placeholders.
 *
 * VRAM layout (word-addressed, 64KB = 32K words):
 *   The authenticated dungeon snapshot has its BAT at $0000 and its tile
 *   patterns in the remaining VRAM. Older fixture snapshots use a $1000
 *   tile-data base. The loader tries the fixture layout first and falls back
 *   to the observed source layout only when the BAT yields no valid tiles.
 *
 * VCE layout: 512 × 16-bit LE words, BGR333 format.
 */

#include "theron_v1_vram_trace_loader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint32_t theron_vram_trace_fnv1a_file(const char *path) {
    FILE *file;
    uint8_t buffer[4096];
    size_t count;
    uint32_t hash = 0x811c9dc5u;

    if (!path || !(file = fopen(path, "rb"))) return 0u;
    while ((count = fread(buffer, 1u, sizeof(buffer), file)) != 0u) {
        for (size_t i = 0u; i < count; ++i) {
            hash ^= buffer[i];
            hash *= 0x01000193u;
        }
    }
    if (ferror(file)) hash = 0u;
    fclose(file);
    return hash;
}

int theron_v1_vram_trace_load_raw(Theron_V1_Viewport *vp,
                                  const uint8_t *vram_data, int vram_size,
                                  const uint8_t *vce_data, int vce_size) {
    if (!vp || !vram_data || !vce_data) return -1;
    /* A raw snapshot is a complete VDC/VCE capture, not a prefix view into a
     * larger container.  The file-backed path already enforces these exact
     * lengths; keep the in-memory API equally strict so callers cannot
     * accidentally admit concatenated or container-tainted bytes. */
    if (vram_size != THERON_VRAM_SIZE || vce_size != THERON_VCE_SIZE) return -1;

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
    vp->vce_palette_relation_verified = 0;
    vp->bat_palette_group_mask = 0;

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
    /* The capture contract is an exact raw VDC snapshot.  Do not silently
     * truncate a concatenated/contaminated file into an apparently valid
     * source bank. */
    if (vram_sz != THERON_VRAM_SIZE) { fclose(fv); return -1; }
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
    if (vce_sz != THERON_VCE_SIZE) {
        free(vram);
        fclose(fc);
        return -1;
    }
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

int theron_v1_vram_trace_load_verified_files(
    Theron_V1_Viewport *vp,
    const char *vram_path,
    const char *vce_path,
    uint32_t expected_vram_fnv1a,
    uint32_t expected_vce_fnv1a) {
    if (!vp || !vram_path || !vce_path || expected_vram_fnv1a == 0u ||
        expected_vce_fnv1a == 0u ||
        theron_vram_trace_fnv1a_file(vram_path) != expected_vram_fnv1a ||
        theron_vram_trace_fnv1a_file(vce_path) != expected_vce_fnv1a) {
        return -1;
    }
    return theron_v1_vram_trace_load_files(vp, vram_path, vce_path);
}

int theron_v1_vram_trace_load_known_capture_files(
    Theron_V1_Viewport *vp,
    const char *vram_path,
    const char *vce_path) {
    /* Complete FNV-1a identities from authenticated external captures.  The
     * old pair is retained for backwards-compatible capture replay.  The
     * later pairs are the US dungeon, US interactive, JP startup and US
     * cold-start screen-space receipts respectively.  Hash admission is the
     * only new capability here; no VDC frame is interpreted as a dungeon
     * square or object record. */
    static const struct {
        uint32_t vram;
        uint32_t vce;
    } known[] = {
        {0xf11c6b2au, 0xea83f117u},
        {0x5c830cc2u, 0x6fb303b5u},
        {0x4f15b98cu, 0x71cc9b11u},
        {0x8ae1e419u, 0x4e48c361u},
        {0x1a37c99bu, 0x71cc9b11u},
        /* 2026-08-09 authenticated active-dungeon screen capture. */
        {0x105dcffbu, 0xea83f117u},
        /* 2026-08-10 corrected cold-start transport capture.  This pair
         * owns only the captured VDC/VCE bitmap and palette banks; the
         * dungeon square/material consumer remains deliberately closed. */
        {0x4a2186a2u, 0xaa11c4f2u},
        /* 2026-08-10 manual dungeon capture from the authenticated US
         * Track 02/System Card session.  Its VCE snapshot is the same
         * source-owned palette bank, while the VRAM image is a later
         * screen-state snapshot. */
        {0x5d20ebc7u, 0xea83f117u},
        /* 2026-08-11 bounded replay from the authenticated US Track 02
         * medium/System Card pair.  This is a new screen-space VDC image
         * with the previously admitted source VCE bank; it authorizes only
         * bitmap/tile/palette replay, never square or object semantics. */
        {0x42a483acu, 0x6fb303b5u},
        /* 2026-08-12 autoload combat capture.  The session is negative for
         * gameplay handoff, but its complete VDC/VCE snapshots are real
         * source-owned screen bytes and may be replayed screen-space only. */
        {0x411960ebu, 0x6fb303b5u},
        /* 2026-08-09 clean external replay.  The transition receipt for
         * this run is intentionally negative, so this pair authorizes only
         * the complete source VDC/VCE screen route, not level or gameplay
         * ownership. */
        {0xa449538au, 0xea83f117u},
        /* 2026-08-13 authenticated CUE/state replay through the patched
         * Mednafen capture producer.  This is a distinct live VRAM image
         * paired with the already admitted source VCE bank; it remains a
         * screen-space-only route. */
        {0x8165c4d4u, 0xea83f117u}
    };

    if (!vp || !vram_path || !vce_path) return -1;
    for (size_t i = 0u; i < sizeof(known) / sizeof(known[0]); ++i) {
        if (theron_v1_vram_trace_load_verified_files(
                vp, vram_path, vce_path, known[i].vram, known[i].vce) == 0) {
            return 0;
        }
    }
    return -1;
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
        /* TQTR may carry an extended VRAM segment.  The VCE segment starts
         * after the complete declared VRAM span, not after the 64 KiB slice
         * Firestaff consumes.  Skipping the extension keeps the palette
         * aligned with the source container instead of admitting shifted
         * colours as a seemingly valid capture. */
        (vram_sz > THERON_VRAM_SIZE &&
         fseek(f, (long)(vram_sz - THERON_VRAM_SIZE), SEEK_CUR) != 0) ||
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
    vp->vce_palette_relation_verified = 0;
    vp->bat_palette_group_mask = 0;
    for (int i = 0; i < 2048; ++i)
        vp->bat_atlas_indices[i] = -1;
    tqr_palette_free_tiles(&vp->palette);
}

static int theron_v1_vram_trace_populate_tiles_with_base(
    Theron_V1_Viewport *vp, int bat_start_word, int bat_w, int bat_h,
    int tile_base_byte) {
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

    if (tile_base_byte < 0 || tile_base_byte >= THERON_VRAM_SIZE)
        return -1;

    tqr_palette_free_tiles(&vp->palette);
    for (int i = 0; i < 2048; ++i)
        vp->bat_atlas_indices[i] = -1;

    int loaded = 0;
    uint16_t palette_group_mask = 0;
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
            palette_group_mask |= (uint16_t)(1u << pal_group);
            vp->palette.tiles[atlas_index].vram_index = (uint16_t)tile_index;
            tile_map[tile_index][pal_group] = atlas_index;
            vp->bat_atlas_indices[bat_word] = (int16_t)atlas_index;
            ++loaded;
        }
    }

    /* The VCE snapshot is the exact source of the 16-entry groups selected
     * by the BAT words above.  Recheck the native little-endian words before
     * publishing the relation so a future caller cannot accidentally treat
     * a decoded tile atlas with an unrelated palette as source-bound. */
    if (loaded > 0 && vp->vce_trace_data) {
        int relation_ok = 1;
        for (int group = 0; group < TQR_PALETTE_GROUPS && relation_ok; ++group) {
            if ((palette_group_mask & (uint16_t)(1u << group)) == 0) continue;
            for (int color = 0; color < TQR_PALETTE_GROUP_SIZE; ++color) {
                int entry = group * TQR_PALETTE_GROUP_SIZE + color;
                uint16_t native = (uint16_t)vp->vce_trace_data[entry * 2] |
                                   ((uint16_t)vp->vce_trace_data[entry * 2 + 1] << 8);
                if (vp->palette.entries[entry].bgr333 != native) {
                    relation_ok = 0;
                    break;
                }
            }
        }
        vp->bat_palette_group_mask = palette_group_mask;
        vp->vce_palette_relation_verified = relation_ok;
    }

    return loaded;
}

int theron_v1_vram_trace_populate_tiles(Theron_V1_Viewport *vp,
                                        int bat_start_word,
                                        int bat_w, int bat_h) {
    int loaded = theron_v1_vram_trace_populate_tiles_with_base(
        vp, bat_start_word, bat_w, bat_h, 0x1000);

    if (loaded != 0) return loaded;
    /* The real dungeon capture has BAT words at $0000 whose tile indices
     * address patterns from VRAM byte zero.  Only use this source-observed
     * route when the historical fixture base produced no admissible tile;
     * never silently merge the two layouts. */
    return theron_v1_vram_trace_populate_tiles_with_base(
        vp, bat_start_word, bat_w, bat_h, 0);
}

int theron_v1_vram_trace_bat_atlas_index(const Theron_V1_Viewport *vp,
                                         int bat_word) {
    if (!vp || !vp->vram_trace_loaded || bat_word < 0 || bat_word >= 2048)
        return -1;
    return vp->bat_atlas_indices[bat_word];
}

int theron_v1_vram_trace_palette_relation_verified(
    const Theron_V1_Viewport *vp) {
    return vp && vp->vram_trace_loaded &&
           vp->vce_palette_relation_verified &&
           vp->bat_palette_group_mask != 0;
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
            uint8_t decoded_tile[64];

            if (atlas_index < 0 || atlas_index >= vp->palette.tile_count)
                continue;
            tile = &vp->palette.tiles[atlas_index];
            if (!tile->data) continue;
            /* The atlas keeps the source-owned PCE planar bytes.  Decode
             * them before preview/presentation; treating the 32 raw 4bpp
             * bytes as 64 indexed pixels produces a plausible-looking but
             * incorrect screen and bypasses the real bitmap decoder. */
            tqr_decode_tile(decoded_tile, tile->data, tile->bpp);
            for (int row = 0; row < TQR_TILE_DIM; ++row) {
                uint8_t *dst = vp->fb.data +
                    (dst_y + y * TQR_TILE_DIM + row) * vp->fb.stride +
                    dst_x + x * TQR_TILE_DIM;
                const uint8_t *src = decoded_tile + row * TQR_TILE_DIM;
                uint8_t palette_base = (uint8_t)(tile->pal_group *
                                                 TQR_PALETTE_GROUP_SIZE);
                for (int px = 0; px < TQR_TILE_DIM; ++px) {
                    /* BAT bits 12..15 select the VCE group.  Keep that
                     * source-owned group in the indexed frame; copying only
                     * the four-bit tile value silently collapsed every real
                     * palette group into group zero. */
                    dst[px] = (uint8_t)(palette_base + src[px]);
                }
            }
            ++copied;
        }
    }
    return copied;
}

int theron_v1_vram_trace_render_authenticated_screen(Theron_V1_Viewport *vp) {
    if (!vp || !vp->vram_trace_loaded || !vp->fb.data ||
        vp->fb.w < TQR_VIEWPORT_W || vp->fb.h < TQR_VIEWPORT_H) {
        return -1;
    }
    /* The capture is a native 256x224 VDC screen. BAT cells are laid out in
     * the source's 64-cell stride; the admitted screen window is the first
     * 32 columns by 28 rows. Keep this separate from the unresolved
     * T520/T600 square-to-tile consumer. */
    return theron_v1_vram_trace_render_bat_preview(
        vp, 0, TQR_VIEWPORT_W / TQR_TILE_DIM,
        TQR_VIEWPORT_H / TQR_TILE_DIM, 0, 0);
}
