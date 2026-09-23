/*
 * Write a real Theron Track 02 VDC/VCE capture as a native 256x224 BMP.
 *
 * This is deliberately a screen-space capture tool. It uses the authenticated
 * VRAM/VCE file identities and the source BAT/atlas route, but does not claim
 * the unresolved T520 dungeon-square or perspective consumer.
 *
 * Usage:
 *   firestaff_theron_v1_authenticated_capture_bmp VRAM.bin VCE.bin OUT.bmp
 *   firestaff_theron_v1_authenticated_capture_bmp --research-raw \
 *       VRAM.bin VCE.bin VDC.state SAT.bin OUT.bmp
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
    FILE *file = NULL;
    const int width = vp->vdc_state_loaded ? vp->vdc_display_width
                                           : TQR_VIEWPORT_W;
    const int height = vp->vdc_state_loaded ? vp->vdc_display_height
                                            : TQR_VIEWPORT_H;
    const int row_bytes = width * 3;
    const int stride = (row_bytes + 3) & ~3;
    const uint32_t pixel_bytes = (uint32_t)(stride * height);
    const uint32_t file_bytes = 14u + 40u + pixel_bytes;
    uint8_t *row;

    if (!path || !vp || !vp->fb.data || width <= 0 || height <= 0 ||
        vp->fb.w < width || vp->fb.h < height || vp->fb.stride < width) {
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
    write_u32_le(file, (uint32_t)width);
    write_u32_le(file, (uint32_t)height);
    write_u16_le(file, 1);
    write_u16_le(file, 24);
    write_u32_le(file, 0);
    write_u32_le(file, pixel_bytes);
    write_u32_le(file, 2835);
    write_u32_le(file, 2835);
    write_u32_le(file, 0);
    write_u32_le(file, 0);

    for (int y = height - 1; y >= 0; --y) {
        memset(row, 0, (size_t)stride);
        for (int x = 0; x < width; ++x) {
            uint16_t source = vp->fb.data[y * vp->fb.stride + x];
            if (vp->vdc_state_loaded)
                source = vp->host_palette_source_indices[source];
            uint32_t rgba = vp->palette.entries[
                source & (TQR_PALETTE_SIZE - 1)].rgba;
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

static int load_research_snapshot(Theron_V1_Viewport *vp,
                                  const char *vram_path,
                                  const char *vce_path,
                                  const char *vdc_path,
                                  const char *sat_path) {
    FILE *file = NULL;
    char header[64];
    char row[256];
    unsigned bxr, byr, mwr, hsr, hdr, vsr, vdr, vcr, cr;
    static const uint16_t widths[4] = {32u, 64u, 128u, 128u};

    if (theron_v1_vram_trace_load_files(vp, vram_path, vce_path) != 0 ||
        !(file = fopen(vdc_path, "r")) ||
        !fgets(header, sizeof(header), file) ||
        strcmp(header, "FIRESTAFF_THERON_VDC_STATE_V1\n") != 0 ||
        !fgets(row, sizeof(row), file) ||
        sscanf(row,
               "vdc=0 bxr=%4x byr=%4x mwr=%4x hsr=%4x hdr=%4x vsr=%4x vdr=%4x vcr=%4x cr=%4x",
               &bxr, &byr, &mwr, &hsr, &hdr, &vsr, &vdr, &vcr, &cr) != 9) {
        if (file) fclose(file);
        return -1;
    }
    fclose(file);
    vp->vdc_bxr = (uint16_t)bxr;
    vp->vdc_byr = (uint16_t)byr;
    vp->vdc_mwr = (uint16_t)mwr;
    vp->vdc_hdr = (uint16_t)hdr;
    vp->vdc_vdr = (uint16_t)vdr;
    vp->vdc_cr = (uint16_t)cr;
    vp->vdc_bat_width = widths[(mwr >> 4) & 3u];
    vp->vdc_bat_height = (mwr & 0x40u) ? 64u : 32u;
    vp->vdc_display_width = (uint16_t)(((hdr & 0x7fu) + 1u) * 8u);
    vp->vdc_display_height = (uint16_t)((vdr & 0x01ffu) + 1u);
    vp->vdc_state_loaded = 1;
    vp->vdc_sat_trace_data = (uint8_t *)malloc(THERON_VDC_SAT_SIZE);
    if (!vp->vdc_sat_trace_data || !(file = fopen(sat_path, "rb")) ||
        fread(vp->vdc_sat_trace_data, 1, THERON_VDC_SAT_SIZE, file) !=
            THERON_VDC_SAT_SIZE || fgetc(file) != EOF) {
        if (file) fclose(file);
        return -1;
    }
    fclose(file);
    vp->vdc_sat_loaded = 1;
    (void)hsr;
    (void)vsr;
    (void)vcr;
    return 0;
}

int main(int argc, char **argv) {
    Theron_V1_Viewport vp;
    uint8_t *framebuffer;
    int populated;
    int result;

    const int research = argc == 7 && strcmp(argv[1], "--research-raw") == 0;
    const char *vram_path;
    const char *vce_path;
    const char *out_path;

    if ((!research && argc != 4) || (research && argc != 7)) {
        fprintf(stderr,
                "usage: %s VRAM.bin VCE.bin OUT.bmp\n"
                "       %s --research-raw VRAM.bin VCE.bin VDC.state SAT.bin OUT.bmp\n",
                argv[0], argv[0]);
        return 2;
    }
    vram_path = research ? argv[2] : argv[1];
    vce_path = research ? argv[3] : argv[2];
    out_path = research ? argv[6] : argv[3];
    memset(&vp, 0, sizeof(vp));
    framebuffer = (uint8_t *)calloc(
        (size_t)(research ? 320 : TQR_FB_W) * TQR_FB_H, 1u);
    if (!framebuffer) return 1;
    vp.fb.data = framebuffer;
    vp.fb.w = research ? 320 : TQR_FB_W;
    vp.fb.h = TQR_FB_H;
    vp.fb.stride = vp.fb.w;

    if ((research ? load_research_snapshot(
                        &vp, vram_path, vce_path, argv[4], argv[5])
                  : theron_v1_vram_trace_load_known_capture_files(
                        &vp, vram_path, vce_path)) != 0) {
        free(framebuffer);
        return 1;
    }
    populated = theron_v1_vram_trace_populate_tiles(
        &vp, 0, research ? vp.vdc_bat_width : 64,
        research ? vp.vdc_bat_height : 32);
    result = populated > 0 &&
             theron_v1_vram_trace_render_authenticated_screen(&vp) > 0 &&
             write_bmp(out_path, &vp) == 0 ? 0 : 1;
    theron_v1_vram_trace_unload(&vp);
    free(framebuffer);
    return result;
}
