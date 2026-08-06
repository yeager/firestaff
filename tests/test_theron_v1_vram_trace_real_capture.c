#include "theron_v1_vram_trace_loader.h"

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

int main(void) {
    const char *vram_path = getenv("THERON_VRAM_SNAPSHOT");
    const char *vce_path = getenv("THERON_VCE_SNAPSHOT");
    Theron_V1_Viewport viewport;
    int loaded;
    int preview_cells;
    size_t preview_nonzero;
    size_t vram_nonzero;
    size_t vce_nonzero;

    if (!vram_path || !vram_path[0] || !vce_path || !vce_path[0]) {
        puts("SKIP: THERON_VRAM_SNAPSHOT and THERON_VCE_SNAPSHOT are not set");
        return 77;
    }
    setenv("FIRESTAFF_THERON_VRAM_SNAPSHOT", vram_path, 1);
    setenv("FIRESTAFF_THERON_VCE_SNAPSHOT", vce_path, 1);
    memset(&viewport, 0, sizeof(viewport));
    if (!theron_vp_init(&viewport) || !viewport.vram_trace_loaded ||
        !viewport.vram_trace_data || !viewport.vce_trace_data) {
        fprintf(stderr, "FAIL: production viewport did not initialize\n");
        return 1;
    }
    if (!viewport.synthetic_rendering_blocked) {
        fprintf(stderr, "FAIL: real capture did not keep unbound semantic rendering blocked\n");
        theron_vp_free(&viewport);
        return 1;
    }
    vram_nonzero = nonzero_bytes(viewport.vram_trace_data, THERON_VRAM_SIZE);
    vce_nonzero = nonzero_bytes(viewport.vce_trace_data, THERON_VCE_SIZE);
    loaded = theron_v1_vram_trace_populate_tiles(&viewport, 0, 64, 32);
    if (vram_nonzero == 0u || vce_nonzero == 0u || loaded <= 0 ||
        viewport.palette.tile_count <= 0) {
        fprintf(stderr, "FAIL: authentic snapshot contains no usable BAT/tile binding\n");
        theron_vp_free(&viewport);
        return 1;
    }
    preview_cells = theron_v1_vram_trace_render_bat_preview(
        &viewport, 0, 32, 28, 0, 0);
    preview_nonzero = nonzero_bytes(viewport.fb.data,
                                    (size_t)viewport.fb.stride * viewport.fb.h);
    if (preview_cells <= 0 || preview_nonzero == 0u) {
        fprintf(stderr, "FAIL: authentic BAT preview produced no pixels\n");
        theron_vp_free(&viewport);
        return 1;
    }
    printf("PASS: vram_nonzero=%zu vce_nonzero=%zu bat_tiles=%d "
           "preview_cells=%d preview_nonzero=%zu palette_entries=512 "
           "pixels=source_only\n",
           vram_nonzero, vce_nonzero, loaded, preview_cells, preview_nonzero);
    theron_vp_free(&viewport);
    return 0;
}
