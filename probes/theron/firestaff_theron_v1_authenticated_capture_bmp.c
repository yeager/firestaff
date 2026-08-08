/*
 * Write a real Theron Track 02 VDC/VCE capture as a native 256x224 BMP.
 *
 * This is deliberately a screen-space capture tool. It uses the authenticated
 * VRAM/VCE file identities and the source BAT/atlas route, but does not claim
 * the unresolved T520 dungeon-square or perspective consumer.
 *
 * Usage:
 *   firestaff_theron_v1_authenticated_capture_bmp VRAM.bin VCE.bin OUT.bmp
 */

#include "theron_v1_vram_trace_loader.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void write_u16_le(FILE *file, uint16_t value) {
    uint8_t bytes[2] = {(uint8_t)value, (uint8_t)(value >> 8)};
    (void)fwrite(bytes, 1, sizeof(bytes), file);
}

static void write_u32_le(FILE *file, uint32_t value) {
    uint8_t bytes[4] = {
        (uint8_t)value, (uint8_t)(value >> 8),
        (uint8_t)(value >> 16), (uint8_t)(value >> 24)
    };
    (void)fwrite(bytes, 1, sizeof(bytes), file);
}

static int write_bmp(const char *path, const Theron_V1_Viewport *vp) {
    FILE *file;
    const int row_bytes = TQR_VIEWPORT_W * 3;
    const int stride = (row_bytes + 3) & ~3;
    const uint32_t pixel_bytes = (uint32_t)(stride * TQR_VIEWPORT_H);
    const uint32_t file_bytes = 14u + 40u + pixel_bytes;
    uint8_t *row;

    if (!path || !vp || !vp->fb.data || vp->fb.w < TQR_VIEWPORT_W ||
        vp->fb.h < TQR_VIEWPORT_H || vp->fb.stride < TQR_VIEWPORT_W) {
        return -1;
    }
    file = fopen(path, "wb");
    if (!file) return -1;
    row = (uint8_t *)calloc((size_t)stride, 1u);
    if (!row) {
        fclose(file);
        return -1;
    }

    (void)fwrite("BM", 1, 2, file);
    write_u32_le(file, file_bytes);
    write_u16_le(file, 0);
    write_u16_le(file, 0);
    write_u32_le(file, 54);
    write_u32_le(file, 40);
    write_u32_le(file, TQR_VIEWPORT_W);
    write_u32_le(file, TQR_VIEWPORT_H);
    write_u16_le(file, 1);
    write_u16_le(file, 24);
    write_u32_le(file, 0);
    write_u32_le(file, pixel_bytes);
    write_u32_le(file, 2835);
    write_u32_le(file, 2835);
    write_u32_le(file, 0);
    write_u32_le(file, 0);

    for (int y = TQR_VIEWPORT_H - 1; y >= 0; --y) {
        memset(row, 0, (size_t)stride);
        for (int x = 0; x < TQR_VIEWPORT_W; ++x) {
            uint32_t rgba = vp->palette.entries[
                vp->fb.data[y * vp->fb.stride + x] &
                (TQR_PALETTE_SIZE - 1)].rgba;
            row[x * 3 + 0] = (uint8_t)rgba;
            row[x * 3 + 1] = (uint8_t)(rgba >> 8);
            row[x * 3 + 2] = (uint8_t)(rgba >> 16);
        }
        if (fwrite(row, 1, (size_t)stride, file) != (size_t)stride) {
            free(row);
            fclose(file);
            return -1;
        }
    }
    free(row);
    return fclose(file) == 0 ? 0 : -1;
}

int main(int argc, char **argv) {
    Theron_V1_Viewport vp;
    uint8_t *framebuffer;
    int populated;
    int result;

    if (argc != 4) {
        fprintf(stderr, "usage: %s VRAM.bin VCE.bin OUT.bmp\n", argv[0]);
        return 2;
    }
    memset(&vp, 0, sizeof(vp));
    framebuffer = (uint8_t *)calloc(TQR_FB_W * TQR_FB_H, 1u);
    if (!framebuffer) return 1;
    vp.fb.data = framebuffer;
    vp.fb.w = TQR_FB_W;
    vp.fb.h = TQR_FB_H;
    vp.fb.stride = TQR_FB_W;

    if (theron_v1_vram_trace_load_verified_files(
            &vp, argv[1], argv[2], 0xf8ab6c1bu, 0xea83f117u) != 0) {
        free(framebuffer);
        return 1;
    }
    populated = theron_v1_vram_trace_populate_tiles(&vp, 0, 64, 32);
    result = populated > 0 &&
             theron_v1_vram_trace_render_authenticated_screen(&vp) > 0 &&
             write_bmp(argv[3], &vp) == 0 ? 0 : 1;
    theron_v1_vram_trace_unload(&vp);
    free(framebuffer);
    return result;
}
