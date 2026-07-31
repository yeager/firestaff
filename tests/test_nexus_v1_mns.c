#include "nexus_v1_mns.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

static uint8_t *load_file(const char *path, int *out_size) {
    FILE *f = fopen(path, "rb");
    uint8_t *buf;
    long sz;
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    buf = (uint8_t *)malloc((size_t)sz);
    if (!buf) { fclose(f); return NULL; }
    if ((long)fread(buf, 1, (size_t)sz, f) != sz) {
        free(buf); fclose(f); return NULL;
    }
    fclose(f);
    *out_size = (int)sz;
    return buf;
}

static int test_synthetic(void) {
    Nexus_V1_MnsDecodeResult r;
    uint8_t bad[64];
    memset(bad, 0, sizeof(bad));
    if (nexus_v1_mns_decode(bad, 64, &r)) return 1;
    if (nexus_v1_mns_decode(NULL, 0, &r)) return 1;
    printf("  PASS synthetic\n");
    return 0;
}

static int test_all_mns(void) {
    const char *home = getenv("HOME");
    char dirpath[512];
    DIR *d;
    struct dirent *ent;
    int decoded = 0, rendered = 0, fail = 0;

    if (!home) { printf("  SKIP all_mns (no HOME)\n"); return 0; }
    snprintf(dirpath, sizeof(dirpath), "%s/.firestaff/data/nexus", home);
    d = opendir(dirpath);
    if (!d) { printf("  SKIP all_mns (no data dir)\n"); return 0; }

    while ((ent = readdir(d)) != NULL) {
        char path[768];
        const char *name = ent->d_name;
        int len = (int)strlen(name);
        uint8_t *data;
        int size = 0;
        Nexus_V1_MnsDecodeResult result;

        if (len < 5 || strcmp(name + len - 4, ".MNS") != 0) continue;

        snprintf(path, sizeof(path), "%s/%s", dirpath, name);
        data = load_file(path, &size);
        if (!data) continue;

        if (!nexus_v1_mns_decode(data, size, &result)) {
            printf("  FAIL %s: decode failed\n", name);
            free(data);
            ++fail;
            continue;
        }

        printf("  PASS %-14s joints=%d verts=%d faces=%d tex=%d",
               name, result.joint_count, result.total_vertices,
               result.total_faces, result.texture_count);
        if (result.texture_count > 0) {
            int texture_index;
            printf(" tex0=%dx%d hash=0x%08X",
                   result.textures[0].width, result.textures[0].height,
                   result.textures[0].pixel_hash);
            for (texture_index = 0; texture_index < result.texture_count;
                 ++texture_index) {
                uint32_t *pixels = (uint32_t *)malloc(
                    (size_t)result.textures[texture_index].pixel_count *
                    sizeof(*pixels));
                if (!pixels || !nexus_v1_mns_render_texture(
                        data, size, &result, texture_index, pixels,
                        result.textures[texture_index].pixel_count)) {
                    ++fail;
                } else {
                    ++rendered;
                }
                free(pixels);
            }
            printf(" render=%s", fail ? "CHECKED" : "PASS");
        }
        printf("\n");
        decoded++;
        free(data);
    }
    closedir(d);

    printf("  decoded %d MNS files, rendered %d source textures\n",
           decoded, rendered);
    if (decoded == 0) printf("  SKIP (no MNS files found)\n");
    return fail;
}

int main(void) {
    int fail = 0;
    printf("=== Nexus V1 MNS Creature Model Decoder ===\n");
    fail += test_synthetic();
    fail += test_all_mns();
    printf("summary: fail=%d\n", fail);
    return fail ? 1 : 0;
}
