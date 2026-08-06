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
    size_t vram_nonzero;
    size_t vce_nonzero;

    if (!vram_path || !vram_path[0] || !vce_path || !vce_path[0]) {
        puts("SKIP: THERON_VRAM_SNAPSHOT and THERON_VCE_SNAPSHOT are not set");
        return 77;
    }
    memset(&viewport, 0, sizeof(viewport));
    if (theron_v1_vram_trace_load_files(&viewport, vram_path, vce_path) != 0 ||
        !viewport.vram_trace_loaded || !viewport.vram_trace_data ||
        !viewport.vce_trace_data) {
        fprintf(stderr, "FAIL: real Mednafen VDC/VCE snapshot was not loaded\n");
        return 1;
    }
    vram_nonzero = nonzero_bytes(viewport.vram_trace_data, THERON_VRAM_SIZE);
    vce_nonzero = nonzero_bytes(viewport.vce_trace_data, THERON_VCE_SIZE);
    loaded = theron_v1_vram_trace_populate_tiles(&viewport, 0, 64, 32);
    if (vram_nonzero == 0u || vce_nonzero == 0u || loaded <= 0 ||
        viewport.palette.tile_count <= 0) {
        fprintf(stderr, "FAIL: authentic snapshot contains no usable BAT/tile binding\n");
        theron_v1_vram_trace_unload(&viewport);
        return 1;
    }
    printf("PASS: vram_nonzero=%zu vce_nonzero=%zu bat_tiles=%d "
           "palette_entries=512 pixels=source_only\n",
           vram_nonzero, vce_nonzero, loaded);
    theron_v1_vram_trace_unload(&viewport);
    return 0;
}
