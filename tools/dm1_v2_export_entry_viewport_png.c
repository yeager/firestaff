#include "../dm1_v2_viewport_renderer_pc34.h"

#include <stdio.h>
#include <stdlib.h>


static const char* dm1_default_dungeon_dat_path(void) {
    static char path[1024];
    const char* home = getenv("HOME");
    if (!home || !home[0]) return NULL;
    snprintf(path, sizeof(path), "%s/.openclaw/data/firestaff-original-games/DM/_canonical/dm1/DUNGEON.DAT", home);
    return path;
}

static unsigned char* read_file_bytes(const char* path, int* outSize) {
    FILE* f = fopen(path, "rb");
    unsigned char* data = NULL;
    long size = 0;
    if (outSize) *outSize = 0;
    if (!f) return NULL;
    if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return NULL; }
    size = ftell(f);
    if (size <= 0) { fclose(f); return NULL; }
    if (fseek(f, 0, SEEK_SET) != 0) { fclose(f); return NULL; }
    data = (unsigned char*)malloc((size_t)size);
    if (!data) { fclose(f); return NULL; }
    if (fread(data, 1, (size_t)size, f) != (size_t)size) {
        free(data);
        fclose(f);
        return NULL;
    }
    fclose(f);
    if (outSize) *outSize = (int)size;
    return data;
}

int main(int argc, char** argv) {
    const char* dungeonPath = getenv("DM1_PC34_DUNGEON_DAT");
    const char* outPath = argc > 1 ? argv[1] : "parity-evidence/verification/pass285_dm1_v2_firestaff_entry_viewport_224x136.png";
    int size = 0;
    unsigned char* bytes;
    DM1_V2_DungeonDatState dungeon;
    DM1_V2_ViewportCompositionInput input;
    DM1_V2_ViewportState viewport;
    if (!dungeonPath) dungeonPath = dm1_default_dungeon_dat_path();
    bytes = read_file_bytes(dungeonPath, &size);
    if (!bytes) {
        fprintf(stderr, "failed to read DUNGEON.DAT: %s\n", dungeonPath);
        return 1;
    }
    if (!dm1_v2_vp_dungeon_dat_init(&dungeon, bytes, size)) {
        fprintf(stderr, "failed to decode DUNGEON.DAT: %s\n", dungeonPath);
        free(bytes);
        return 1;
    }
    if (!dm1_v2_vp_build_composition_from_dungeon(&dungeon, 0, 1, 3, 2, &input)) {
        fprintf(stderr, "failed to build entry composition map=0 x=1 y=3 dir=2\n");
        free(bytes);
        return 1;
    }
    dm1_v2_vp_init(&viewport);
    if (!dm1_v2_vp_render_composition_flat(&viewport, &input)) {
        fprintf(stderr, "failed to render entry composition\n");
        free(bytes);
        return 1;
    }
    if (!dm1_v2_vp_write_png_rgba(outPath, &viewport.framebuffer[0][0], DM1_V2_VIEWPORT_W, DM1_V2_VIEWPORT_H, DM1_V2_VIEWPORT_W)) {
        fprintf(stderr, "failed to write PNG: %s\n", outPath);
        free(bytes);
        return 1;
    }
    printf("dm1_v2_entry_viewport_png: %s map=0 x=1 y=3 dir=2 width=%d height=%d parityClaim=0\n", outPath, DM1_V2_VIEWPORT_W, DM1_V2_VIEWPORT_H);
    free(bytes);
    return 0;
}
