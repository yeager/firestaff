#include "nexus_v1_dgn_texture_decode.h"

#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    const char *dir = getenv("FIRESTAFF_NEXUS_DATA_DIR");
    char default_dir[1024];
    char path[1024];
    FILE *f;
    long size;
    uint8_t *data, pixels[64 * 64 * 2];
    uint16_t palette[16];
    Nexus_V1_DgnTextureDecodeReceipt r;
    int rc;
    if (!dir || !dir[0]) {
        const char *home = getenv("HOME");
        if (home) {
            snprintf(default_dir, sizeof(default_dir), "%s/.firestaff/data/nexus", home);
            dir = default_dir;
        }
    }
    if (!dir || !dir[0]) { puts("SKIP: FIRESTAFF_NEXUS_DATA_DIR is not set"); return 77; }
    snprintf(path, sizeof(path), "%s/LEV00.DGN", dir);
    f = fopen(path, "rb");
    if (!f || fseek(f, 0, SEEK_END) != 0) { if (f) fclose(f); puts("SKIP: LEV00.DGN missing"); return 77; }
    size = ftell(f); rewind(f); data = (uint8_t *)malloc((size_t)size);
    if (!data || fread(data, 1, (size_t)size, f) != (size_t)size) { free(data); fclose(f); return 1; }
    fclose(f);
    rc = nexus_v1_dgn_texture_decode(data, (int)size, 1, pixels, sizeof(pixels), palette, 16, &r);
    if (rc != NEXUS_V1_DGN_TEXTURE_DECODE_OK || !r.source_verified || !r.decoded ||
        !r.indexed4 || r.width != 0x30 || r.height != 0x30 || r.image_bytes != 0x480U ||
        !r.palette_decoded || pixels[0] > 15U) { free(data); fprintf(stderr, "FAIL: real Structure2 texture decode (%s) w=%u h=%u bytes=%u idx=%d pal=%d p0=%u\n", nexus_v1_dgn_texture_decode_status_name((Nexus_V1_DgnTextureDecodeStatus)rc), r.width, r.height, r.image_bytes, r.indexed4, r.palette_decoded, pixels[0]); return 1; }
    rc = nexus_v1_dgn_texture_decode(data, (int)size, 0, pixels, sizeof(pixels), palette, 16, &r);
    free(data);
    if (rc != NEXUS_V1_DGN_TEXTURE_DECODE_OK || !r.direct_color_555 || r.width != 0x20 || r.height != 0x20) { fprintf(stderr, "FAIL: real direct-color texture decode\n"); return 1; }
    puts("test_nexus_v1_dgn_texture_decode: PASS");
    return 0;
}
