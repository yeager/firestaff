#include "nexus_v1_dgn_texture_decode.h"
#include "asset_find_by_hash.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint16_t read_be16(const uint8_t *p)
{
    return (uint16_t)(((uint16_t)p[0] << 8) | p[1]);
}

static uint32_t read_be32(const uint8_t *p)
{
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
           ((uint32_t)p[2] << 8) | p[3];
}

#define DGN_BLOCK_SIZE 2048U

static const char *expected_dgn_md5(int level_index)
{
    static const char *const hashes[16] = {
        "603ec9c531a92539babdda84ab09e78e", "751e1442bf7dccbd41bf146b5be144ab",
        "e2cb85d9fedc27f894a84e0f465fcde1", "19637d6b59849565f64565aed786d7ea",
        "85abc1b822e5c66ec4e99f1f676c140e", "ed5d54ab0ac1c927c1346dd966c8a5cc",
        "58c336ff6146e7216f0081e726823ea1", "c19e6038a017a320515ecbb66f6da197",
        "9bfc31bea631345a3660c2645be0e95b", "32a6450f29eb7babd73fcbe7a0310f22",
        "2928440e9c21457929f1323a28a42f70", "d7be5cd0d6e5c10afe99ec9950614fad",
        "db1cf70d6730615f73f191fad5e11e32", "f8876d0181d79727013236a6b597b99b",
        "a634dd5e95567ecbbbc332350c8cf12b", "5e6e237074f1e6b0decc629868a51f3c"
    };
    return level_index >= 0 && level_index < 16 ? hashes[level_index] : NULL;
}

int main(void)
{
    const char *dir = getenv("FIRESTAFF_NEXUS_DATA_DIR");
    char default_dir[1024];
    char path[1024];
    FILE *f;
    long size;
    uint8_t *data;
    uint8_t *pixels;
    uint16_t palette[16];
    Nexus_V1_DgnTextureDecodeReceipt r;
    int rc, level_index, image_id, decoded = 0, indexed4 = 0, direct555 = 0;
    size_t nonzero_pixels = 0U;
    size_t nonzero_palette_words = 0U;
    const size_t pixel_capacity = 1024U * 1024U;
    if (!dir || !dir[0]) {
        const char *home = getenv("HOME");
        if (home) {
            snprintf(default_dir, sizeof(default_dir), "%s/.firestaff/data/nexus", home);
            dir = default_dir;
        }
    }
    if (!dir || !dir[0]) { puts("SKIP: FIRESTAFF_NEXUS_DATA_DIR is not set"); return 77; }
    pixels = (uint8_t *)malloc(pixel_capacity);
    if (!pixels) return 1;

    for (level_index = 0; level_index < 16; ++level_index) {
        uint32_t structure2_offset;
        uint32_t structure2_useful;
        int texture_count = 0;
        snprintf(path, sizeof(path), "%s/LEV%02d.DGN", dir, level_index);
        f = fopen(path, "rb");
        if (!f || fseek(f, 0, SEEK_END) != 0) {
            if (f) fclose(f);
            free(pixels);
            fprintf(stderr, "FAIL: LEV%02d.DGN missing\n", level_index);
            return 1;
        }
        size = ftell(f);
        rewind(f);
        data = (uint8_t *)malloc((size_t)size);
        if (!data || fread(data, 1, (size_t)size, f) != (size_t)size) {
            free(data); fclose(f); free(pixels); return 1;
        }
        fclose(f);
        if (!asset_file_matches_md5(path, expected_dgn_md5(level_index))) {
            fprintf(stderr, "FAIL: LEV%02d.DGN is not the verified retail source\n",
                    level_index);
            free(data); free(pixels); return 1;
        }
        if (size < 0x20) {
            fprintf(stderr, "FAIL: LEV%02d.DGN header is truncated\n", level_index);
            free(data); free(pixels); return 1;
        }
        structure2_offset = (uint32_t)read_be16(data + 0x14) * DGN_BLOCK_SIZE;
        structure2_useful = read_be32(data + 0x18);
        while (texture_count < 512 &&
               structure2_offset + (uint32_t)(texture_count + 1) * 20U <=
                   structure2_offset + structure2_useful &&
               read_be16(data + structure2_offset + (uint32_t)texture_count * 20U) !=
                   0xffffU) {
            ++texture_count;
        }
        if (texture_count <= 0) {
            fprintf(stderr, "FAIL: LEV%02d.DGN has no parsed Structure2 descriptors\n", level_index);
            free(data); free(pixels); return 1;
        }
        for (image_id = 0; image_id < texture_count; ++image_id) {
            const uint8_t *descriptor =
                data + structure2_offset + (uint32_t)image_id * 20U;
            memset(pixels, 0, pixel_capacity);
            rc = nexus_v1_dgn_texture_decode(data, (int)size, image_id,
                                             pixels, (int)pixel_capacity,
                                             palette, 16, &r);
            if (rc != NEXUS_V1_DGN_TEXTURE_DECODE_OK || r.source_verified ||
                !r.decoded || r.width != read_be16(descriptor + 6) ||
                r.height != read_be16(descriptor + 8) ||
                r.encoding != read_be16(descriptor + 2)) {
                fprintf(stderr, "FAIL: LEV%02d image=%d decode=%s w=%u/%u h=%u/%u enc=%04x/%04x source=%d decoded=%d\n",
                        level_index, image_id,
                        nexus_v1_dgn_texture_decode_status_name(
                            (Nexus_V1_DgnTextureDecodeStatus)rc),
                        r.width, read_be16(descriptor + 6), r.height,
                        read_be16(descriptor + 8), r.encoding,
                        read_be16(descriptor + 2), r.source_verified,
                        r.decoded);
                free(data); free(pixels); return 1;
            }
            if (r.indexed4) {
                if (!r.palette_decoded || r.palette_entries != 16) {
                    fprintf(stderr, "FAIL: LEV%02d image=%d missing indexed palette\n",
                            level_index, image_id);
                    free(data); free(pixels); return 1;
                }
                for (rc = 0; rc < 16; ++rc) {
                    if (palette[rc] != 0U) ++nonzero_palette_words;
                }
                ++indexed4;
            }
            if (r.direct_color_555) ++direct555;
            {
                size_t output_bytes = r.indexed4
                    ? (size_t)r.width * (size_t)r.height
                    : (size_t)r.width * (size_t)r.height * 2U;
                size_t output_index;
                for (output_index = 0U; output_index < output_bytes;
                     ++output_index) {
                    if (pixels[output_index] != 0U) ++nonzero_pixels;
                }
            }
            ++decoded;
        }
        free(data);
    }
    free(pixels);
    if (decoded != 1678 || indexed4 != 1553 || direct555 != 125 ||
        nonzero_pixels == 0U || nonzero_palette_words == 0U) {
        fprintf(stderr, "FAIL: real texture census decoded=%d indexed4=%d direct555=%d nonzero_pixels=%zu nonzero_palette_words=%zu\n",
                decoded, indexed4, direct555, nonzero_pixels,
                nonzero_palette_words);
        return 1;
    }
    printf("test_nexus_v1_dgn_texture_decode: PASS levels=16 textures=%d indexed4=%d direct555=%d\n",
           decoded, indexed4, direct555);
    return 0;
}
