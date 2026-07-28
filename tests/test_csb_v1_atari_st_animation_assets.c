#include "csb_v1_atari_st_animation_assets.h"

#include <stdio.h>
#include <stdlib.h>

static int failures;

#define CHECK(condition, message) do { \
    if (condition) printf("PASS: %s\n", message); \
    else { printf("FAIL: %s\n", message); ++failures; } \
} while (0)

static uint8_t *read_file(const char *path, size_t *out_size)
{
    FILE *fp;
    long length;
    uint8_t *bytes;
    if (!path || !out_size || !(fp = fopen(path, "rb"))) return NULL;
    if (fseek(fp, 0L, SEEK_END) != 0 || (length = ftell(fp)) <= 0 ||
        fseek(fp, 0L, SEEK_SET) != 0 ||
        !(bytes = (uint8_t *)malloc((size_t)length)) ||
        fread(bytes, 1u, (size_t)length, fp) != (size_t)length) {
        fclose(fp);
        return NULL;
    }
    fclose(fp);
    *out_size = (size_t)length;
    return bytes;
}

int main(void)
{
    const uint8_t palette_bytes[32] = {
        0x00, 0x00, 0x07, 0x65, 0x04, 0x44, 0x00, 0x02
    };
    uint8_t palette[16][3];
    const char *dat_path = getenv("FIRESTAFF_CSB_ANIMATE_DAT");
    const char *script_path = getenv("FIRESTAFF_CSB_ANIMATE_SCR");

    CHECK(csb_v1_atari_st_animation_decode_p4b1_palette(palette_bytes,
              sizeof(palette_bytes), palette) && palette[0][0] == 0u &&
              palette[1][0] == 255u && palette[1][1] == 218u &&
              palette[1][2] == 182u,
          "P4B1 Atari ST palette words become RGB888");

    if (dat_path && script_path && dat_path[0] && script_path[0]) {
        size_t script_size = 0u;
        uint8_t *script = read_file(script_path, &script_size);
        uint8_t *rgba = (uint8_t *)malloc(CSB_V1_ATARI_ST_ANIMATION_RGBA_BYTES);
        CSB_V1_AtariStAnimationAssetReceipt receipt;
        CHECK(script != NULL, "real Atari ANIMATE.SCR is readable");
        if (script) {
            CHECK(csb_v1_atari_st_animation_validate_assets(dat_path, script,
                      script_size, &receipt) && receipt.valid &&
                      receipt.data_item_count == 87u &&
                      receipt.script_instruction_count == 288u &&
                      receipt.palette_load_count > 0u &&
                      receipt.image_load_count > 0u &&
                      receipt.sound_load_count == 4u,
                  "real script references only documented ANIMATE.DAT item families");
        }
        CHECK(rgba != NULL && csb_v1_atari_st_animation_render_rgba(dat_path,
                  30u, 0u, rgba, CSB_V1_ATARI_ST_ANIMATION_RGBA_BYTES) &&
                  rgba[3] == 255u,
              "real Atari IMG1 title frame renders with its P4B1 palette");
        free(script);
        free(rgba);
    }
    return failures == 0 ? 0 : 1;
}
