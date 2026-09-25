#include "theron_v1_vram_trace_loader.h"
#include "theron_v1_boot.h"
#include "theron_v1_world.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static size_t nonzero_bytes(const unsigned char *data, size_t count) {
    size_t i;
    size_t nonzero = 0;
    for (i = 0; i < count; ++i)
        if (data[i] != 0u) ++nonzero;
    return nonzero;
}

static int write_source_bmp(const char *path,
                            const Theron_V1_Viewport *viewport) {
    FILE *file;
    uint8_t header[54] = {0};
    int y;

    if (!path || !path[0] || !viewport || !viewport->fb.data) return 0;
    file = fopen(path, "wb");
    if (!file) return 0;
    header[0] = 'B';
    header[1] = 'M';
    {
        uint32_t size = 54u + (uint32_t)(viewport->fb.w * viewport->fb.h * 3);
        header[2] = (uint8_t)size; header[3] = (uint8_t)(size >> 8);
        header[4] = (uint8_t)(size >> 16); header[5] = (uint8_t)(size >> 24);
    }
    header[10] = 54;
    header[14] = 40;
    header[18] = (uint8_t)viewport->fb.w;
    header[19] = (uint8_t)(viewport->fb.w >> 8);
    header[22] = (uint8_t)viewport->fb.h;
    header[23] = (uint8_t)(viewport->fb.h >> 8);
    header[26] = 1;
    header[28] = 24;
    if (fwrite(header, 1, sizeof(header), file) != sizeof(header)) {
        fclose(file);
        return 0;
    }
    for (y = viewport->fb.h - 1; y >= 0; --y) {
        int x;
        for (x = 0; x < viewport->fb.w; ++x) {
            uint8_t index = viewport->fb.data[y * viewport->fb.stride + x];
            uint16_t source = viewport->host_palette_source_indices[index];
            uint32_t rgba = viewport->palette.entries[source].rgba;
            uint8_t pixel[3] = {
                (uint8_t)(rgba & 0xffu),
                (uint8_t)((rgba >> 8) & 0xffu),
                (uint8_t)((rgba >> 16) & 0xffu)
            };
            if (fwrite(pixel, 1, sizeof(pixel), file) != sizeof(pixel)) {
                fclose(file);
                return 0;
            }
        }
    }
    return fclose(file) == 0;
}

int main(void) {
    const char *vram_path = getenv("THERON_VRAM_SNAPSHOT");
    const char *vce_path = getenv("THERON_VCE_SNAPSHOT");
    const char *vdc_path = getenv("THERON_VDC_STATE_SNAPSHOT");
    const char *sat_path = getenv("THERON_VDC_SAT_SNAPSHOT");
    const char *vdc_io_path = getenv("THERON_VDC_IO_TRACE");
    const char *capture_root = getenv("THERON_CAPTURE_ROOT");
    Theron_V1_Viewport viewport;
    int loaded;
    int preview_cells;
    int screen_cells;
    unsigned char m11_framebuffer[320u * 200u] = {0};
    unsigned char expected_screen[320u * 224u];
    size_t preview_nonzero;
    size_t presented_nonzero;
    size_t boot_presented_nonzero;
    size_t vram_nonzero;
    size_t vce_nonzero;
    size_t sprite_pixels = 0u;

    if ((!capture_root || !capture_root[0]) &&
        (!vram_path || !vram_path[0] || !vce_path || !vce_path[0] ||
         !vdc_path || !vdc_path[0] || !sat_path || !sat_path[0] ||
         !vdc_io_path || !vdc_io_path[0])) {
        puts("SKIP: atomic Theron VRAM/VCE/VDC-state/SAT/VDC-I/O bundle is not set");
        return 77;
    }
    if (!capture_root || !capture_root[0]) {
#ifdef _WIN32
        _putenv_s("FIRESTAFF_THERON_VRAM_SNAPSHOT", vram_path);
        _putenv_s("FIRESTAFF_THERON_VCE_SNAPSHOT", vce_path);
        _putenv_s("FIRESTAFF_THERON_VDC_STATE_SNAPSHOT", vdc_path);
        _putenv_s("FIRESTAFF_THERON_VDC_SAT_SNAPSHOT", sat_path);
        _putenv_s("FIRESTAFF_THERON_VDC_IO_TRACE", vdc_io_path);
#else
        setenv("FIRESTAFF_THERON_VRAM_SNAPSHOT", vram_path, 1);
        setenv("FIRESTAFF_THERON_VCE_SNAPSHOT", vce_path, 1);
        setenv("FIRESTAFF_THERON_VDC_STATE_SNAPSHOT", vdc_path, 1);
        setenv("FIRESTAFF_THERON_VDC_SAT_SNAPSHOT", sat_path, 1);
        setenv("FIRESTAFF_THERON_VDC_IO_TRACE", vdc_io_path, 1);
#endif
    }
    memset(&viewport, 0, sizeof(viewport));
    if (!(capture_root && capture_root[0]
              ? theron_vp_init_from_data_dir(&viewport, capture_root)
              : theron_vp_init(&viewport)) ||
        !viewport.vram_trace_loaded ||
        !viewport.vram_trace_data || !viewport.vce_trace_data) {
        fprintf(stderr, "FAIL: production viewport did not initialize or bind real VRAM/VCE\n");
        return 1;
    }
    if (!viewport.synthetic_rendering_blocked) {
        fprintf(stderr, "FAIL: real capture did not keep unbound semantic rendering blocked\n");
        theron_vp_free(&viewport);
        return 1;
    }
    if (getenv("THERON_EXPECT_INPUT_SCREEN_RELATION")) {
        int expect_left = strcmp(
            getenv("THERON_EXPECT_INPUT_SCREEN_RELATION"), "left") == 0;
        uint16_t expected_mask = expect_left ? 0x0080u : 0x0020u;
        uint16_t expected_result = expect_left ? 0x0037u : 0x003du;
        if (
        (!viewport.vdc_input_screen_relation_verified ||
         viewport.vdc_source_input_mask != expected_mask ||
         viewport.vdc_source_input_result != expected_result ||
         viewport.vdc_source_input_hold_frames != 10u)) {
        fprintf(stderr,
                "FAIL: authenticated directional input was not bound to its screen\n");
        theron_vp_free(&viewport);
        return 1;
        }
    }
    vram_nonzero = nonzero_bytes(viewport.vram_trace_data, THERON_VRAM_SIZE);
    vce_nonzero = nonzero_bytes(viewport.vce_trace_data, THERON_VCE_SIZE);
    loaded = theron_v1_vram_trace_populate_tiles(
        &viewport, 0, viewport.vdc_bat_width, viewport.vdc_bat_height);
    if (vram_nonzero == 0u || vce_nonzero == 0u || loaded <= 0 ||
        viewport.palette.tile_count <= 0 ||
        !theron_v1_vram_trace_palette_relation_verified(&viewport)) {
        fprintf(stderr, "FAIL: authentic snapshot contains no usable BAT/tile binding\n");
        theron_vp_free(&viewport);
        return 1;
    }
    preview_cells = loaded;
    if (theron_v1_vram_trace_render_authenticated_screen(&viewport) <= 0) {
        fprintf(stderr, "FAIL: register-bound BAT preview produced no frame\n");
        theron_vp_free(&viewport);
        return 1;
    }
    preview_nonzero = nonzero_bytes(viewport.fb.data,
                                    (size_t)viewport.fb.stride * viewport.fb.h);
    if (preview_cells <= 0 || preview_nonzero == 0u) {
        fprintf(stderr, "FAIL: authentic BAT preview produced no pixels\n");
        theron_vp_free(&viewport);
        return 1;
    }
    memset(viewport.fb.data, 0,
           (size_t)viewport.fb.stride * (size_t)viewport.fb.h);
    screen_cells = theron_v1_vram_trace_render_authenticated_screen(&viewport);
    if (screen_cells <= 0 ||
        nonzero_bytes(viewport.fb.data,
                      (size_t)viewport.fb.stride * viewport.fb.h) == 0u) {
        fprintf(stderr, "FAIL: authenticated native screen produced no pixels\n");
        theron_vp_free(&viewport);
        return 1;
    }
    for (size_t p = 0u;
         p < (size_t)viewport.vdc_display_width * viewport.vdc_display_height;
         ++p) {
        if (viewport.vdc_source_frame[p] >= 0x100u) ++sprite_pixels;
    }
    if (sprite_pixels == 0u || viewport.host_palette_source_count == 0u ||
        viewport.host_palette_source_count > 256u) {
        fprintf(stderr,
                "FAIL: authenticated SAT produced no losslessly mapped sprite pixels\n");
        theron_vp_free(&viewport);
        return 1;
    }
    memcpy(expected_screen, viewport.fb.data,
           (size_t)viewport.fb.stride * (size_t)viewport.fb.h);
    memset(viewport.fb.data, 0,
           (size_t)viewport.fb.stride * (size_t)viewport.fb.h);
    theron_vp_render_dungeon(&viewport, NULL);
    if (memcmp(expected_screen, viewport.fb.data,
               (size_t)viewport.fb.stride * (size_t)viewport.fb.h) != 0) {
        fprintf(stderr,
                "FAIL: production dungeon route diverged from authenticated screen route\n");
        theron_vp_free(&viewport);
        return 1;
    }
    theron_vp_present(&viewport, &viewport.palette,
                      m11_framebuffer, 320, 200);
    presented_nonzero = nonzero_bytes(m11_framebuffer,
                                      sizeof(m11_framebuffer));
    if (presented_nonzero == 0u) {
        fprintf(stderr, "FAIL: authentic BAT frame was not presented to M11\n");
        theron_vp_free(&viewport);
        return 1;
    }

    /* Regression for the production boot facade: an authenticated capture
     * is permitted to reach the source-only presenter even when the real
     * Track 02 bundle has no decoded generated tile/material bank.  This
     * must not reopen synthetic square-to-tile semantics. */
    {
        Theron_V1_World world;
        TrAssetBundle assets;
        unsigned char boot_framebuffer[320u * 200u] = {0};

        theron_v1_world_init(&world);
        memset(&assets, 0, sizeof(assets));
        if (!theron_v1_boot_runtime_render_frame(
                &world, &viewport, &assets, 0, 0,
                boot_framebuffer, 320, 200)) {
            fprintf(stderr,
                    "FAIL: authenticated capture was blocked by boot facade\n");
            theron_vp_free(&viewport);
            return 1;
        }
        if (memcmp(expected_screen, viewport.fb.data,
                   (size_t)viewport.fb.stride * (size_t)viewport.fb.h) != 0) {
            fprintf(stderr,
                    "FAIL: boot facade overlaid unverified UI on authentic screen\n");
            theron_vp_free(&viewport);
            return 1;
        }
        boot_presented_nonzero = nonzero_bytes(boot_framebuffer,
                                                sizeof(boot_framebuffer));
        if (boot_presented_nonzero == 0u) {
            fprintf(stderr,
                    "FAIL: boot facade presented an empty capture frame\n");
            theron_vp_free(&viewport);
            return 1;
        }
    }
    if (getenv("THERON_VRAM_CAPTURE_BMP") &&
        !write_source_bmp(getenv("THERON_VRAM_CAPTURE_BMP"), &viewport)) {
        fprintf(stderr, "FAIL: requested source-backed BMP could not be written\n");
        theron_vp_free(&viewport);
        return 1;
    }
    printf("PASS: vram_nonzero=%zu vce_nonzero=%zu bat_tiles=%d "
           "preview_cells=%d preview_nonzero=%zu presented_nonzero=%zu "
           "boot_presented_nonzero=%zu palette_entries=512 "
           "host_entries=%u sprite_pixels=%zu "
           "pixels=source_only\n",
           vram_nonzero, vce_nonzero, loaded, preview_cells, preview_nonzero,
           presented_nonzero, boot_presented_nonzero,
           viewport.host_palette_source_count, sprite_pixels);
    theron_vp_free(&viewport);
    return 0;
}
