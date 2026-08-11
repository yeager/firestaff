#include "nexus_v1_viewport.h"
#include "nexus_v1_vdp1_dgn_material_resolver.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint8_t *read_file(const char *path, size_t *out_size)
{
    FILE *file;
    long size;
    uint8_t *data;
    if (!path || !out_size) return NULL;
    file = fopen(path, "rb");
    if (!file || fseek(file, 0, SEEK_END) != 0 ||
        (size = ftell(file)) <= 0 || fseek(file, 0, SEEK_SET) != 0) {
        if (file) fclose(file);
        return NULL;
    }
    data = (uint8_t *)malloc((size_t)size);
    if (!data || fread(data, 1U, (size_t)size, file) != (size_t)size) {
        free(data);
        fclose(file);
        return NULL;
    }
    fclose(file);
    *out_size = (size_t)size;
    return data;
}

static int written_pixels(const Nexus_Framebuffer *framebuffer)
{
    int i;
    int count = 0;
    if (!framebuffer) return 0;
    for (i = 0; i < NEXUS_FB_W * NEXUS_FB_H; ++i)
        if (framebuffer->z_buffer[i] < 1e30f) ++count;
    return count;
}

int main(void)
{
    const char *capture_path = getenv("FIRESTAFF_NEXUS_RUNTIME_CAPTURE");
    const char *dgn_path = getenv("FIRESTAFF_NEXUS_DGN_SOURCE");
    const char *frame_text = getenv("FIRESTAFF_NEXUS_RUNTIME_CAPTURE_FRAME");
    Nexus_Viewport viewport;
    Nexus_V1_Vdp1DgnMaterialResolverInput resolver;
    Nexus_V1_SaturnRuntimeCaptureFrameReceipt frame;
    Nexus_V1_SaturnVdp2RegisterReceipt registers;
    Nexus_V1_Vdp2TilemapCaptureReceipt vdp2;
    Nexus_V1_Vdp1CaptureVramSequenceReceipt vdp1;
    Nexus_V1_Vdp12CaptureCompositionReceipt composition;
    uint8_t *capture;
    uint8_t *dgn;
    size_t capture_size;
    size_t dgn_size;
    unsigned int frame_index;

    if (!capture_path || !dgn_path || !frame_text) {
        puts("test_nexus_v1_vdp12_runtime_composition: SKIP (external capture not configured)");
        return 0;
    }
    capture = read_file(capture_path, &capture_size);
    dgn = read_file(dgn_path, &dgn_size);
    if (!capture || !dgn) {
        free(capture);
        free(dgn);
        fprintf(stderr, "FAIL: runtime composition input files\n");
        return 1;
    }
    frame_index = (unsigned int)strtoul(frame_text, NULL, 0);
    memset(&resolver, 0, sizeof(resolver));
    resolver.dgn_bytes = dgn;
    resolver.dgn_byte_count = (int)dgn_size;
    resolver.source_hash_verified = 1;
    resolver.palette_slot_base = 16;
    nexus_viewport_init(&viewport);
    if (!nexus_viewport_replay_vdp12_runtime_frame_mode1_over_tilemap(
            &viewport, capture, capture_size, frame_index,
            nexus_v1_vdp1_dgn_material_resolver, &resolver,
            0, 0, 64, 64, 0, 0,
            &frame, &registers, &vdp2, &vdp1, &composition) ||
        !frame.valid || !registers.valid || !vdp2.valid || !vdp2.capture_only ||
        vdp2.renderer_permitted || !vdp1.valid || !vdp1.replay.valid ||
        vdp1.replay.source_joins_verified < 190 ||
        !composition.valid || !composition.layer_order_verified ||
        !composition.vdp1_over_vdp2 || composition.renderer_permitted ||
        composition.vdp2_written_pixels <= 0 ||
        composition.vdp1_written_pixels <= 0 || written_pixels(&viewport.fb) <= 0) {
        free(capture);
        free(dgn);
        fprintf(stderr, "FAIL: authenticated VDP2-under-VDP1 runtime composition\n");
        return 1;
    }
    free(capture);
    free(dgn);
    puts("test_nexus_v1_vdp12_runtime_composition: PASS");
    return 0;
}
