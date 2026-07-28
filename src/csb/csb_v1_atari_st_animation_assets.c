#include "csb_v1_atari_st_animation_assets.h"

#include "csb_v1_animation_script.h"
#include "csb_v1_graphics_atari_st_loader_pc34_compat.h"
#include "csb_v1_startup_img3_decode_pc34_compat.h"

#include <stdlib.h>
#include <string.h>

static int csb_v1_atari_st_animation_item_type_matches(
    uint16_t item_index, uint16_t item_type)
{
    if (item_type == 1u) return item_index <= 29u;
    if (item_type == 0u)
        return (item_index >= 30u && item_index <= 36u) ||
            (item_index >= 75u && item_index <= 84u);
    if (item_type == 2u) return item_index == 85u || item_index == 86u;
    return 0;
}

static int csb_v1_atari_st_animation_read_item(
    const CSB_AtariStLoader *loader, uint16_t item_index,
    uint8_t **out_bytes, size_t *out_size)
{
    uint8_t *bytes;
    size_t size;

    if (!loader || !out_bytes || !out_size || item_index >= loader->item_count)
        return 0;
    size = loader->items[item_index].decompressed_size;
    if (size == 0u) return 0;
    bytes = (uint8_t *)malloc(size);
    if (!bytes) return 0;
    if (csb_atari_st_graphics_loader_read_item(loader, item_index, bytes, size) !=
        (int)size) {
        free(bytes);
        return 0;
    }
    *out_bytes = bytes;
    *out_size = size;
    return 1;
}

int csb_v1_atari_st_animation_decode_p4b1_palette(
    const uint8_t *bytes, size_t byte_count, uint8_t out_rgb[16][3])
{
    size_t index;

    if (!bytes || !out_rgb || byte_count != 32u) return 0;
    for (index = 0u; index < 16u; ++index) {
        uint16_t word = (uint16_t)(((uint16_t)bytes[index * 2u] << 8) |
            bytes[index * 2u + 1u]);
        out_rgb[index][0] = (uint8_t)(((word >> 8) & 7u) * 255u / 7u);
        out_rgb[index][1] = (uint8_t)(((word >> 4) & 7u) * 255u / 7u);
        out_rgb[index][2] = (uint8_t)((word & 7u) * 255u / 7u);
    }
    return 1;
}

int csb_v1_atari_st_animation_validate_assets(
    const char *animate_dat_path, const uint8_t *script, size_t script_size,
    CSB_V1_AtariStAnimationAssetReceipt *out_receipt)
{
    CSB_AtariStLoader loader;
    CSB_V1_AnimationScriptInstruction instructions[
        CSB_V1_ANIMATION_SCRIPT_MAX_INSTRUCTIONS];
    size_t instruction_count = 0u;
    size_t index;
    int valid = 1;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!animate_dat_path || !script ||
        csb_v1_animation_script_parse(script, script_size, instructions,
            CSB_V1_ANIMATION_SCRIPT_MAX_INSTRUCTIONS, &instruction_count) !=
            CSB_V1_ANIMATION_SCRIPT_OK) return 0;
    csb_atari_st_graphics_loader_init(&loader);
    if (!csb_atari_st_graphics_loader_open(&loader, animate_dat_path)) return 0;

    for (index = 0u; index < instruction_count; ++index) {
        const CSB_V1_AnimationScriptInstruction *instruction =
            &instructions[index];
        if (instruction->opcode != 3u) continue;
        if (instruction->parameter_count != 3u ||
            instruction->parameters[0] >= loader.item_count ||
            !csb_v1_atari_st_animation_item_type_matches(
                instruction->parameters[0], instruction->parameters[2])) {
            valid = 0;
            if (out_receipt) out_receipt->invalid_load_count++;
            continue;
        }
        if (out_receipt) {
            if (instruction->parameters[2] == 0u) out_receipt->image_load_count++;
            else if (instruction->parameters[2] == 1u) out_receipt->palette_load_count++;
            else out_receipt->sound_load_count++;
        }
    }
    if (out_receipt) {
        out_receipt->data_item_count = loader.item_count;
        out_receipt->script_instruction_count = (uint16_t)instruction_count;
        out_receipt->valid = valid && out_receipt->invalid_load_count == 0u;
    }
    csb_atari_st_graphics_loader_close(&loader);
    return valid;
}

int csb_v1_atari_st_animation_render_rgba(
    const char *animate_dat_path, uint16_t image_item, uint16_t palette_item,
    uint8_t *out_rgba, size_t out_rgba_size)
{
    CSB_AtariStLoader loader;
    uint8_t *image_bytes = NULL;
    uint8_t *palette_bytes = NULL;
    uint8_t indexed[CSB_V1_ATARI_ST_ANIMATION_WIDTH *
                    CSB_V1_ATARI_ST_ANIMATION_HEIGHT];
    uint8_t palette[16][3];
    size_t image_size = 0u;
    size_t palette_size = 0u;
    size_t index;
    int result = 0;

    if (!animate_dat_path || !out_rgba ||
        out_rgba_size < CSB_V1_ATARI_ST_ANIMATION_RGBA_BYTES ||
        !csb_v1_atari_st_animation_item_type_matches(image_item, 0u) ||
        !csb_v1_atari_st_animation_item_type_matches(palette_item, 1u)) return 0;
    csb_atari_st_graphics_loader_init(&loader);
    if (!csb_atari_st_graphics_loader_open(&loader, animate_dat_path) ||
        !csb_v1_atari_st_animation_read_item(&loader, image_item, &image_bytes,
            &image_size) ||
        !csb_v1_atari_st_animation_read_item(&loader, palette_item, &palette_bytes,
            &palette_size) ||
        !csb_v1_atari_st_animation_decode_p4b1_palette(palette_bytes, palette_size,
            palette) ||
        !csb_v1_startup_img3_decode_to_indexed_pc34_compat(image_bytes, image_size,
            CSB_V1_ATARI_ST_ANIMATION_WIDTH, CSB_V1_ATARI_ST_ANIMATION_HEIGHT,
            indexed, sizeof(indexed))) goto done;

    for (index = 0u; index < sizeof(indexed); ++index) {
        uint8_t color = (uint8_t)(indexed[index] & 15u);
        out_rgba[index * 4u] = palette[color][0];
        out_rgba[index * 4u + 1u] = palette[color][1];
        out_rgba[index * 4u + 2u] = palette[color][2];
        out_rgba[index * 4u + 3u] = 255u;
    }
    result = 1;
done:
    free(image_bytes);
    free(palette_bytes);
    csb_atari_st_graphics_loader_close(&loader);
    return result;
}
