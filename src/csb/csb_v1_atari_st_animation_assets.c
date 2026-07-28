#include "csb_v1_atari_st_animation_assets.h"

#include "csb_v1_atari_st_animation_discovery.h"
#include "csb_v1_animation_script.h"
#include "csb_v1_graphics_atari_st_loader_pc34_compat.h"
#include "csb_v1_startup_img3_decode_pc34_compat.h"

#include <stdio.h>
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

static uint8_t *csb_v1_atari_st_animation_read_file(
    const char *path, size_t *out_size)
{
    FILE *fp;
    long length;
    uint8_t *bytes = NULL;

    if (!path || !out_size || !(fp = fopen(path, "rb"))) return NULL;
    if (fseek(fp, 0L, SEEK_END) != 0 || (length = ftell(fp)) <= 0 ||
        fseek(fp, 0L, SEEK_SET) != 0 ||
        !(bytes = (uint8_t *)malloc((size_t)length)) ||
        fread(bytes, 1u, (size_t)length, fp) != (size_t)length) {
        fclose(fp);
        free(bytes);
        return NULL;
    }
    fclose(fp);
    *out_size = (size_t)length;
    return bytes;
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

typedef struct {
    uint16_t item_index;
    uint16_t type;
    int loaded;
    uint8_t *pixels;
    uint16_t width;
    uint16_t height;
    int32_t box_left;
    int32_t box_top;
    int32_t box_right;
    int32_t box_bottom;
    int16_t transparent_color;
    int16_t bitmap_width;
    int16_t bitmap_height;
    int16_t attribute_1a;
    int16_t attribute_1c;
    int16_t attribute_1e;
} csb_v1_atari_st_animation_slot;

static void csb_v1_atari_st_animation_slot_release(
    csb_v1_atari_st_animation_slot *slot)
{
    if (!slot) return;
    free(slot->pixels);
    memset(slot, 0, sizeof(*slot));
}

static int csb_v1_atari_st_animation_slot_decode_img1(
    const CSB_AtariStLoader *loader, csb_v1_atari_st_animation_slot *slot)
{
    uint8_t *item_bytes = NULL;
    size_t item_size = 0u;
    size_t pixel_count;
    uint16_t width;
    uint16_t height;
    int result = 0;

    if (!loader || !slot || !slot->loaded || slot->type != 0u) return 0;
    if (slot->pixels) return 1;
    if (!csb_v1_atari_st_animation_read_item(loader, slot->item_index,
            &item_bytes, &item_size) || item_size < 4u) goto done;
    width = (uint16_t)(((uint16_t)item_bytes[0] << 8) | item_bytes[1]);
    height = (uint16_t)(((uint16_t)item_bytes[2] << 8) | item_bytes[3]);
    if (width == 0u || height == 0u || width > 320u || height > 200u ||
        height > SIZE_MAX / width) goto done;
    pixel_count = (size_t)width * height;
    slot->pixels = (uint8_t *)malloc(pixel_count);
    if (!slot->pixels || !csb_v1_startup_img3_decode_to_indexed_pc34_compat(
            item_bytes, item_size, width, height, slot->pixels, pixel_count)) {
        free(slot->pixels);
        slot->pixels = NULL;
        goto done;
    }
    slot->width = width;
    slot->height = height;
    slot->box_left = 0;
    slot->box_top = 0;
    slot->box_right = (int32_t)width - 1;
    slot->box_bottom = (int32_t)height - 1;
    slot->transparent_color = -1;
    slot->bitmap_width = (int16_t)width;
    slot->bitmap_height = (int16_t)height;
    result = 1;
done:
    free(item_bytes);
    return result;
}

static int csb_v1_atari_st_animation_slot_copy_image(
    csb_v1_atari_st_animation_slot *destination,
    const csb_v1_atari_st_animation_slot *source, int copy_pixels)
{
    size_t pixel_count;

    if (!destination || !source || !source->pixels || source->width == 0u ||
        source->height == 0u || source->height > SIZE_MAX / source->width)
        return 0;
    pixel_count = (size_t)source->width * source->height;
    csb_v1_atari_st_animation_slot_release(destination);
    destination->pixels = (uint8_t *)calloc(pixel_count, 1u);
    if (!destination->pixels) return 0;
    if (copy_pixels) memcpy(destination->pixels, source->pixels, pixel_count);
    destination->type = 0u;
    destination->loaded = 1;
    destination->width = source->width;
    destination->height = source->height;
    destination->box_left = 0;
    destination->box_top = 0;
    destination->box_right = (int32_t)source->box_right - source->box_left;
    destination->box_bottom = (int32_t)source->box_bottom - source->box_top;
    destination->transparent_color = source->transparent_color;
    destination->bitmap_width = source->bitmap_width;
    destination->bitmap_height = source->bitmap_height;
    destination->attribute_1a = source->attribute_1a;
    destination->attribute_1c = source->attribute_1c;
    destination->attribute_1e = source->attribute_1e;
    return 1;
}

static int csb_v1_atari_st_animation_blit_transparent(
    const csb_v1_atari_st_animation_slot *source,
    csb_v1_atari_st_animation_slot *destination,
    const csb_v1_atari_st_animation_slot *source_box, int32_t x, int32_t y)
{
    int32_t source_top;
    int32_t source_bottom;
    int32_t source_left;
    int32_t source_right;
    int32_t row;

    if (!source || !destination || !source_box || !source->pixels ||
        !destination->pixels ||
        source->width == 0u || source->height == 0u || destination->width == 0u ||
        destination->height == 0u) return 0;
    source_left = source_box->box_left;
    source_right = source_box->box_right;
    source_top = source_box->box_top;
    source_bottom = source_box->box_bottom;
    if (source_left < 0) source_left = 0;
    if (source_top < 0) source_top = 0;
    if (source_right >= source->width) source_right = source->width - 1;
    if (source_bottom >= source->height) source_bottom = source->height - 1;
    if (source_left > source_right || source_top > source_bottom) return 1;
    for (row = source_top; row <= source_bottom; ++row) {
        int32_t column;
        const int32_t dst_y = y + (row - source_top);
        if (dst_y < 0) continue;
        if (dst_y >= destination->height) break;
        for (column = source_left; column <= source_right; ++column) {
            const int32_t dst_x = x + (column - source_left);
            const uint8_t pixel = source->pixels[(size_t)row * source->width +
                                                 column];
            if (dst_x >= 0 && dst_x < destination->width &&
                (destination->transparent_color < 0 ||
                 pixel != (uint8_t)destination->transparent_color)) {
                destination->pixels[(size_t)dst_y * destination->width +
                                    dst_x] = pixel;
            }
        }
    }
    return 1;
}

static int csb_v1_atari_st_animation_slot_has_type(
    const csb_v1_atari_st_animation_slot *slots, uint16_t slot, uint16_t type)
{
    return slots && slot < 256u && slots[slot].loaded && slots[slot].type == type;
}

static int csb_v1_atari_st_animation_palette_item_is_loaded(
    const csb_v1_atari_st_animation_slot *slots, uint16_t item_index)
{
    uint16_t slot;
    for (slot = 0u; slot < 256u; ++slot) {
        if (slots[slot].loaded && slots[slot].type == 1u &&
            slots[slot].item_index == item_index) return 1;
    }
    return 0;
}

static int csb_v1_atari_st_animation_palette_reference_is_loaded(
    const csb_v1_atari_st_animation_slot *slots, uint16_t reference)
{
    return csb_v1_atari_st_animation_slot_has_type(slots, reference, 1u) ||
        csb_v1_atari_st_animation_palette_item_is_loaded(slots, reference) ||
        reference <= 29u;
}

static uint16_t csb_v1_atari_st_animation_palette_item_for_reference(
    const csb_v1_atari_st_animation_slot *slots, uint16_t reference)
{
    uint16_t slot;
    if (csb_v1_atari_st_animation_slot_has_type(slots, reference, 1u))
        return slots[reference].item_index;
    for (slot = 0u; slot < 256u; ++slot) {
        if (slots[slot].loaded && slots[slot].type == 1u &&
            slots[slot].item_index == reference) return reference;
    }
    return reference <= 29u ? reference : 0xffffu;
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

int csb_v1_atari_st_animation_trace_script(
    const char *animate_dat_path, const uint8_t *script, size_t script_size,
    CSB_V1_AtariStAnimationTraceReceipt *out_receipt)
{
    CSB_AtariStLoader loader;
    CSB_V1_AnimationScriptInstruction instructions[
        CSB_V1_ANIMATION_SCRIPT_MAX_INSTRUCTIONS];
    csb_v1_atari_st_animation_slot slots[256];
    uint16_t loop_pc[256];
    size_t instruction_count = 0u;
    size_t pc = 0u;
    uint32_t steps = 0u;
    uint16_t active_screen_slot = 0xffffu;
    uint16_t active_palette_item = 0xffffu;
    int stopped = 0;
    int valid = 1;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!animate_dat_path || !script ||
        csb_v1_animation_script_parse(script, script_size, instructions,
            CSB_V1_ANIMATION_SCRIPT_MAX_INSTRUCTIONS, &instruction_count) !=
            CSB_V1_ANIMATION_SCRIPT_OK) return 0;
    csb_atari_st_graphics_loader_init(&loader);
    if (!csb_atari_st_graphics_loader_open(&loader, animate_dat_path)) return 0;
    memset(slots, 0, sizeof(slots));
    memset(loop_pc, 0, sizeof(loop_pc));

    while (pc < instruction_count && ++steps <= 65536u) {
        const CSB_V1_AnimationScriptInstruction *instruction =
            &instructions[pc++];
        const uint16_t *p = instruction->parameters;

        switch (instruction->opcode) {
            case 1u:
            case 2u:
                stopped = 1;
                break;
            case 3u: /* Load item */
                if (p[0] >= loader.item_count || p[1] >= 256u ||
                    !csb_v1_atari_st_animation_item_type_matches(p[0], p[2])) {
                    valid = 0;
                    break;
                }
                slots[p[1]].item_index = p[0];
                slots[p[1]].type = p[2];
                slots[p[1]].loaded = 1;
                break;
            case 4u: /* Unload */
                if (p[0] >= 256u || !slots[p[0]].loaded) valid = 0;
                else memset(&slots[p[0]], 0, sizeof(slots[p[0]]));
                break;
            case 5u: /* Expand IMG1 into a destination screen slot */
                if (!csb_v1_atari_st_animation_slot_has_type(slots, p[0], 0u) ||
                    p[1] >= 256u) valid = 0;
                else {
                    slots[p[1]] = slots[p[0]];
                    if (out_receipt) out_receipt->expand_count++;
                }
                break;
            case 6u: /* Blit bitmap */
                if (!csb_v1_atari_st_animation_slot_has_type(slots, p[0], 0u) ||
                    !csb_v1_atari_st_animation_slot_has_type(slots, p[1], 0u))
                    valid = 0;
                else if (out_receipt) {
                    /* ANIM.C F0466 waits for VBlank before every blit. */
                    out_receipt->blit_count++;
                    out_receipt->waited_vbl_count++;
                }
                break;
            case 7u: /* Set palette at the next VBlank */
            case 8u: /* Atari ANIM.C waits delay + 1 VBlanks, then sets it */
                /* The original script refers to P4B1 item IDs here. Its
                 * preceding Load commands decide which memory slot owns that
                 * item; e.g. item 2 is loaded in slot 7 then faded as 2. */
                if (!csb_v1_atari_st_animation_palette_reference_is_loaded(slots,
                        p[0]))
                    valid = 0;
                else {
                    active_palette_item =
                        csb_v1_atari_st_animation_palette_item_for_reference(
                            slots, p[0]);
                    if (out_receipt) {
                        if (instruction->opcode == 8u) {
                            out_receipt->fade_count++;
                            out_receipt->waited_vbl_count +=
                                (uint32_t)p[1] + 1u;
                        } else {
                            out_receipt->waited_vbl_count++;
                        }
                    }
                }
                break;
            case 10u: /* Wait VBLs */
                if (out_receipt) out_receipt->waited_vbl_count += p[0];
                break;
            case 11u: /* Wait until VBL */
                if (out_receipt && out_receipt->waited_vbl_count < p[0])
                    out_receipt->waited_vbl_count = p[0];
                break;
            case 12u: /* Play sound */
                if (!csb_v1_atari_st_animation_slot_has_type(slots, p[0], 2u))
                    valid = 0;
                else if (out_receipt) out_receipt->played_sound_count++;
                break;
            case 14u: /* Set screen and palette at VBL */
                if (!csb_v1_atari_st_animation_slot_has_type(slots, p[0], 0u) ||
                    (p[1] != 0xffffu &&
                     !csb_v1_atari_st_animation_palette_reference_is_loaded(slots,
                         p[1]))) {
                    valid = 0;
                } else {
                    if (out_receipt) out_receipt->waited_vbl_count++;
                    active_screen_slot = p[0];
                    if (p[1] != 0xffffu)
                        active_palette_item =
                            csb_v1_atari_st_animation_palette_item_for_reference(
                                slots, p[1]);
                    if (out_receipt) {
                        const uint16_t index = out_receipt->present_count;
                        out_receipt->present_count++;
                        out_receipt->last_presented_image_item = slots[p[0]].item_index;
                        out_receipt->last_presented_palette_item = p[1];
                        if (index < CSB_V1_ATARI_ST_ANIMATION_MAX_PRESENTED_FRAMES) {
                            out_receipt->presented_image_items[index] =
                                slots[p[0]].item_index;
                            out_receipt->presented_palette_items[index] =
                                active_palette_item;
                            out_receipt->presented_vbls[index] =
                                out_receipt->waited_vbl_count;
                        }
                    }
                }
                break;
            case 15u: /* Wait one VBL */
                if (out_receipt) out_receipt->waited_vbl_count++;
                break;
            case 16u: /* FOR: store next instruction address */
                if (p[0] >= 256u || pc > 0xffffu) valid = 0;
                else loop_pc[p[0]] = (uint16_t)pc;
                break;
            case 17u: /* decrement loop counter */
                if (p[0] >= 256u) valid = 0;
                else slots[p[0]].box_left--;
                break;
            case 18u: /* NEXT */
                if (p[0] >= 256u || p[1] >= 256u) valid = 0;
                else if (slots[p[0]].box_left > 0) {
                    if (loop_pc[p[1]] >= instruction_count) valid = 0;
                    else pc = loop_pc[p[1]];
                }
                break;
            case 19u: /* Set box left */
                if (p[0] >= 256u) valid = 0;
                else slots[p[0]].box_left = p[1];
                break;
            case 20u:
                if (p[0] >= 256u) valid = 0;
                else {
                    slots[p[0]].box_left = p[1];
                    slots[p[0]].box_right = p[2];
                }
                break;
            case 21u:
                if (p[0] >= 256u) valid = 0;
                else {
                    slots[p[0]].box_left = p[1];
                    slots[p[0]].box_right = p[2];
                    slots[p[0]].box_top = p[3];
                }
                break;
            case 22u:
                if (p[0] >= 256u) valid = 0;
                else {
                    slots[p[0]].box_left = p[1];
                    slots[p[0]].box_right = p[2];
                    slots[p[0]].box_top = p[3];
                    slots[p[0]].box_bottom = p[4];
                }
                break;
            case 23u:
                if (p[0] >= 256u) valid = 0;
                else {
                    slots[p[0]].box_left = p[1];
                    slots[p[0]].box_right = p[2];
                    slots[p[0]].box_top = p[3];
                    slots[p[0]].box_bottom = p[4];
                    slots[p[0]].transparent_color = (int16_t)p[5];
                }
                break;
            case 24u:
                if (p[0] >= 256u) valid = 0;
                else slots[p[0]].bitmap_width = (int16_t)p[1];
                break;
            case 25u:
                if (p[0] >= 256u) valid = 0;
                else slots[p[0]].bitmap_height = (int16_t)p[1];
                break;
            case 26u:
                if (p[0] >= 256u) valid = 0;
                else slots[p[0]].attribute_1a = (int16_t)p[1];
                break;
            case 27u:
                if (p[0] >= 256u) valid = 0;
                else slots[p[0]].attribute_1c = (int16_t)p[1];
                break;
            case 28u:
                if (p[0] >= 256u) valid = 0;
                else slots[p[0]].attribute_1e = (int16_t)p[1];
                break;
            case 29u: /* allocate copy of image dimensions */
                if (!csb_v1_atari_st_animation_slot_has_type(slots, p[0], 0u) ||
                    p[1] >= 256u) valid = 0;
                else slots[p[1]] = slots[p[0]];
                break;
            case 30u: /* display coordinates */
                if (!csb_v1_atari_st_animation_slot_has_type(slots, p[0], 0u))
                    valid = 0;
                break;
            default:
                /* Opcodes 9, 13 and 20..28 are documented no-op/set forms. */
                break;
        }
        if (!valid || stopped) {
            if (!valid && out_receipt) {
                out_receipt->failed_opcode = instruction->opcode;
                out_receipt->failed_instruction_index = (uint16_t)(pc - 1u);
            }
            break;
        }
    }
    if (out_receipt) {
        out_receipt->executed_instruction_count = steps;
        if (active_screen_slot < 256u && slots[active_screen_slot].loaded &&
            slots[active_screen_slot].type == 0u) {
            out_receipt->final_active_image_item =
                slots[active_screen_slot].item_index;
        } else {
            out_receipt->final_active_image_item = 0xffffu;
        }
        out_receipt->final_palette_item = active_palette_item;
        out_receipt->valid = valid && stopped;
    }
    csb_atari_st_graphics_loader_close(&loader);
    return valid && stopped;
}

int csb_v1_atari_st_animation_decode_frame_at_vbl_indexed(
    const char *animate_dat_path, const uint8_t *script, size_t script_size,
    uint32_t target_vbl,
    uint8_t out_indexed[CSB_V1_ATARI_ST_ANIMATION_INDEXED_BYTES],
    uint8_t out_palette[16][3],
    CSB_V1_AtariStAnimationTraceReceipt *out_receipt)
{
    CSB_AtariStLoader loader;
    CSB_V1_AnimationScriptInstruction instructions[
        CSB_V1_ANIMATION_SCRIPT_MAX_INSTRUCTIONS];
    CSB_V1_AtariStAnimationTraceReceipt trace;
    csb_v1_atari_st_animation_slot slots[256];
    uint16_t loop_pc[256];
    size_t instruction_count = 0u;
    size_t pc = 0u;
    uint32_t vbl_count = 0u;
    uint16_t active_screen_slot = 0xffffu;
    uint16_t active_palette_item = 0xffffu;
    int result = 0;
    uint16_t slot_index;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!animate_dat_path || !script || !out_indexed || !out_palette ||
        !csb_v1_atari_st_animation_trace_script(animate_dat_path, script,
            script_size, &trace) || !trace.valid ||
        csb_v1_animation_script_parse(script, script_size, instructions,
            CSB_V1_ANIMATION_SCRIPT_MAX_INSTRUCTIONS, &instruction_count) !=
            CSB_V1_ANIMATION_SCRIPT_OK) return 0;
    csb_atari_st_graphics_loader_init(&loader);
    if (!csb_atari_st_graphics_loader_open(&loader, animate_dat_path)) return 0;
    memset(slots, 0, sizeof(slots));
    memset(loop_pc, 0, sizeof(loop_pc));

    while (pc < instruction_count) {
        const CSB_V1_AnimationScriptInstruction *instruction =
            &instructions[pc++];
        const uint16_t *p = instruction->parameters;

        switch (instruction->opcode) {
        case 1u:
        case 2u:
            pc = instruction_count;
            break;
        case 3u:
            if (p[0] >= loader.item_count || p[1] >= 256u ||
                !csb_v1_atari_st_animation_item_type_matches(p[0], p[2]))
                goto done;
            csb_v1_atari_st_animation_slot_release(&slots[p[1]]);
            slots[p[1]].item_index = p[0];
            slots[p[1]].type = p[2];
            slots[p[1]].loaded = 1;
            break;
        case 4u:
            if (p[0] >= 256u || !slots[p[0]].loaded) goto done;
            csb_v1_atari_st_animation_slot_release(&slots[p[0]]);
            break;
        case 5u:
            if (p[0] >= 256u || p[1] >= 256u || p[2] >= 256u ||
                !csb_v1_atari_st_animation_slot_decode_img1(&loader,
                    &slots[p[0]]) ||
                !csb_v1_atari_st_animation_slot_copy_image(&slots[p[1]],
                    &slots[p[0]], 1)) goto done;
            /* ANIMATE.FTL's first full-screen expansion targets slot zero,
             * the initial Atari display page. Later opcode 14 switches to
             * slot one explicitly. */
            if (active_screen_slot == 0xffffu && p[1] == 0u)
                active_screen_slot = 0u;
            break;
        case 6u:
            /* F0132_VIDEO_Blit takes the source rectangle from p2 and the
             * destination origin from p3.  The original script sometimes
             * supplies a full-screen box for a smaller source; clipping is
             * therefore part of the source operation, not a replacement
             * heuristic. */
            if (p[0] >= 256u || p[1] >= 256u || p[2] >= 256u || p[3] >= 256u ||
                !csb_v1_atari_st_animation_slot_decode_img1(&loader,
                    &slots[p[0]]) || !slots[p[1]].pixels ||
                !csb_v1_atari_st_animation_blit_transparent(&slots[p[0]],
                    &slots[p[1]], &slots[p[2]], slots[p[3]].box_left,
                    slots[p[3]].box_top)) goto done;
            /* ReDMCSB ANIM.C F0466 synchronizes each bitmap copy to VBlank. */
            vbl_count++;
            break;
        case 7u:
            if (!csb_v1_atari_st_animation_palette_reference_is_loaded(slots,
                    p[0])) goto done;
            active_palette_item =
                csb_v1_atari_st_animation_palette_item_for_reference(slots,
                    p[0]);
            vbl_count++;
            break;
        case 8u:
            if (!csb_v1_atari_st_animation_palette_reference_is_loaded(slots,
                    p[0])) goto done;
            /* ReDMCSB ANIM.C calls F0436 with the script delay. MEDIA772
             * (Atari ST animation) waits inclusively then commits the target
             * palette, rather than using a host interpolation. */
            vbl_count += (uint32_t)p[1] + 1u;
            active_palette_item =
                csb_v1_atari_st_animation_palette_item_for_reference(slots,
                    p[0]);
            break;
        case 10u:
            vbl_count += p[0];
            break;
        case 11u:
            if (vbl_count < p[0]) vbl_count = p[0];
            break;
        case 14u:
            if (p[0] >= 256u || !slots[p[0]].pixels ||
                (p[1] != 0xffffu &&
                 !csb_v1_atari_st_animation_palette_reference_is_loaded(slots,
                     p[1]))) goto done;
            vbl_count++;
            active_screen_slot = p[0];
            if (p[1] != 0xffffu)
                active_palette_item =
                    csb_v1_atari_st_animation_palette_item_for_reference(slots,
                        p[1]);
            break;
        case 15u:
            vbl_count++;
            break;
        case 16u:
            if (p[0] >= 256u || pc > 0xffffu) goto done;
            loop_pc[p[0]] = (uint16_t)pc;
            break;
            case 17u:
            if (p[0] >= 256u) goto done;
            slots[p[0]].box_left--;
            break;
            case 18u:
            if (p[0] >= 256u || p[1] >= 256u) goto done;
            if (slots[p[0]].box_left > 0) {
                if (loop_pc[p[1]] >= instruction_count) goto done;
                pc = loop_pc[p[1]];
            }
            break;
            case 19u:
            if (p[0] >= 256u) goto done;
            slots[p[0]].box_left = p[1];
            break;
            case 20u:
            if (p[0] >= 256u) goto done;
            slots[p[0]].box_left = p[1];
            slots[p[0]].box_right = p[2];
            break;
            case 21u:
            if (p[0] >= 256u) goto done;
            slots[p[0]].box_left = p[1];
            slots[p[0]].box_right = p[2];
            slots[p[0]].box_top = p[3];
            break;
            case 22u:
            if (p[0] >= 256u) goto done;
            slots[p[0]].box_left = p[1];
            slots[p[0]].box_right = p[2];
            slots[p[0]].box_top = p[3];
            slots[p[0]].box_bottom = p[4];
            break;
            case 23u:
            if (p[0] >= 256u) goto done;
            slots[p[0]].box_left = p[1];
            slots[p[0]].box_right = p[2];
            slots[p[0]].box_top = p[3];
            slots[p[0]].box_bottom = p[4];
            slots[p[0]].transparent_color = (int16_t)p[5];
            break;
        case 24u:
            if (p[0] >= 256u) goto done;
            slots[p[0]].bitmap_width = (int16_t)p[1];
            break;
        case 25u:
            if (p[0] >= 256u) goto done;
            slots[p[0]].bitmap_height = (int16_t)p[1];
            break;
        case 26u:
            if (p[0] >= 256u) goto done;
            slots[p[0]].attribute_1a = (int16_t)p[1];
            break;
        case 27u:
            if (p[0] >= 256u) goto done;
            slots[p[0]].attribute_1c = (int16_t)p[1];
            break;
        case 28u:
            if (p[0] >= 256u) goto done;
            slots[p[0]].attribute_1e = (int16_t)p[1];
            break;
        case 29u:
            if (p[0] >= 256u || p[1] >= 256u ||
                !csb_v1_atari_st_animation_slot_decode_img1(&loader,
                    &slots[p[0]]) ||
                !csb_v1_atari_st_animation_slot_copy_image(&slots[p[1]],
                    &slots[p[0]], 0)) goto done;
            break;
        case 30u:
            if (p[0] >= 256u || !slots[p[0]].loaded) goto done;
            slots[p[0]].box_right -= slots[p[0]].box_left;
            slots[p[0]].box_bottom -= slots[p[0]].box_top;
            slots[p[0]].box_left = p[1];
            slots[p[0]].box_top = p[2];
            slots[p[0]].box_right += slots[p[0]].box_left;
            slots[p[0]].box_bottom += slots[p[0]].box_top;
            break;
        default:
            break;
        }
        if (active_screen_slot < 256u && vbl_count >= target_vbl) {
            uint8_t *palette_bytes = NULL;
            size_t palette_size = 0u;
            const size_t screen_size = (size_t)slots[active_screen_slot].width *
                slots[active_screen_slot].height;
            if (slots[active_screen_slot].width !=
                    CSB_V1_ATARI_ST_ANIMATION_WIDTH ||
                slots[active_screen_slot].height !=
                    CSB_V1_ATARI_ST_ANIMATION_HEIGHT ||
                screen_size != CSB_V1_ATARI_ST_ANIMATION_INDEXED_BYTES ||
                active_palette_item == 0xffffu ||
                !csb_v1_atari_st_animation_read_item(&loader,
                    active_palette_item, &palette_bytes, &palette_size) ||
                !csb_v1_atari_st_animation_decode_p4b1_palette(palette_bytes,
                    palette_size, out_palette)) {
                free(palette_bytes);
                goto done;
            }
            memcpy(out_indexed, slots[active_screen_slot].pixels, screen_size);
            free(palette_bytes);
            result = 1;
            break;
        }
    }
done:
    for (slot_index = 0u; slot_index < 256u; ++slot_index)
        csb_v1_atari_st_animation_slot_release(&slots[slot_index]);
    csb_atari_st_graphics_loader_close(&loader);
    if (result && out_receipt) *out_receipt = trace;
    return result;
}

int csb_v1_atari_st_animation_render_final_rgba(
    const char *animate_dat_path, const uint8_t *script, size_t script_size,
    uint8_t *out_rgba, size_t out_rgba_size,
    CSB_V1_AtariStAnimationTraceReceipt *out_receipt)
{
    CSB_V1_AtariStAnimationTraceReceipt trace;

    memset(&trace, 0, sizeof(trace));
    if (!csb_v1_atari_st_animation_trace_script(animate_dat_path, script,
            script_size, &trace) || !trace.valid ||
        !csb_v1_atari_st_animation_item_type_matches(
            trace.final_active_image_item, 0u) ||
        !csb_v1_atari_st_animation_item_type_matches(
            trace.final_palette_item, 1u) ||
        !csb_v1_atari_st_animation_render_rgba(animate_dat_path,
            trace.final_active_image_item, trace.final_palette_item, out_rgba,
            out_rgba_size)) return 0;
    if (out_receipt) *out_receipt = trace;
    return 1;
}

int csb_v1_atari_st_animation_render_presented_rgba(
    const char *animate_dat_path, const uint8_t *script, size_t script_size,
    uint16_t presentation_index, uint8_t *out_rgba, size_t out_rgba_size,
    CSB_V1_AtariStAnimationTraceReceipt *out_receipt)
{
    CSB_V1_AtariStAnimationTraceReceipt trace;

    memset(&trace, 0, sizeof(trace));
    if (!csb_v1_atari_st_animation_trace_script(animate_dat_path, script,
            script_size, &trace) || !trace.valid ||
        presentation_index >= trace.present_count ||
        presentation_index >= CSB_V1_ATARI_ST_ANIMATION_MAX_PRESENTED_FRAMES ||
        !csb_v1_atari_st_animation_item_type_matches(
            trace.presented_image_items[presentation_index], 0u) ||
        !csb_v1_atari_st_animation_item_type_matches(
            trace.presented_palette_items[presentation_index], 1u) ||
        !csb_v1_atari_st_animation_render_rgba(animate_dat_path,
            trace.presented_image_items[presentation_index],
            trace.presented_palette_items[presentation_index], out_rgba,
            out_rgba_size)) return 0;
    if (out_receipt) *out_receipt = trace;
    return 1;
}

int csb_v1_atari_st_animation_decode_presented_indexed(
    const char *animate_dat_path, const uint8_t *script, size_t script_size,
    uint16_t presentation_index,
    uint8_t out_indexed[CSB_V1_ATARI_ST_ANIMATION_INDEXED_BYTES],
    uint8_t out_palette[16][3],
    CSB_V1_AtariStAnimationTraceReceipt *out_receipt)
{
    CSB_V1_AtariStAnimationTraceReceipt trace;
    CSB_AtariStLoader loader;
    uint8_t *image_bytes = NULL;
    uint8_t *palette_bytes = NULL;
    size_t image_size = 0u;
    size_t palette_size = 0u;
    int result = 0;

    if (!out_indexed || !out_palette ||
        !csb_v1_atari_st_animation_trace_script(animate_dat_path, script,
            script_size, &trace) || !trace.valid ||
        presentation_index >= trace.present_count ||
        presentation_index >= CSB_V1_ATARI_ST_ANIMATION_MAX_PRESENTED_FRAMES ||
        !csb_v1_atari_st_animation_item_type_matches(
            trace.presented_image_items[presentation_index], 0u) ||
        !csb_v1_atari_st_animation_item_type_matches(
            trace.presented_palette_items[presentation_index], 1u)) return 0;
    csb_atari_st_graphics_loader_init(&loader);
    if (!csb_atari_st_graphics_loader_open(&loader, animate_dat_path) ||
        !csb_v1_atari_st_animation_read_item(&loader,
            trace.presented_image_items[presentation_index], &image_bytes,
            &image_size) ||
        !csb_v1_atari_st_animation_read_item(&loader,
            trace.presented_palette_items[presentation_index], &palette_bytes,
            &palette_size) ||
        !csb_v1_atari_st_animation_decode_p4b1_palette(palette_bytes,
            palette_size, out_palette) ||
        !csb_v1_startup_img3_decode_to_indexed_pc34_compat(image_bytes,
            image_size, CSB_V1_ATARI_ST_ANIMATION_WIDTH,
            CSB_V1_ATARI_ST_ANIMATION_HEIGHT, out_indexed,
            CSB_V1_ATARI_ST_ANIMATION_INDEXED_BYTES)) goto done;
    result = 1;
done:
    free(image_bytes);
    free(palette_bytes);
    csb_atari_st_graphics_loader_close(&loader);
    if (result && out_receipt) *out_receipt = trace;
    return result;
}

int csb_v1_atari_st_animation_render_final_from_root_rgba(
    const char *search_root, const char *cache_root, uint8_t *out_rgba,
    size_t out_rgba_size, CSB_V1_AtariStAnimationTraceReceipt *out_receipt)
{
    CSB_V1_AtariStAnimationDiscoveryReceipt discovery;
    char script_path[ASSET_PATH_MAX];
    char data_path[ASSET_PATH_MAX];
    uint8_t *script;
    size_t script_size = 0u;
    int result;

    if (!csb_v1_atari_st_animation_discover(search_root, &discovery) ||
        !csb_v1_atari_st_animation_materialize(&discovery, cache_root,
            script_path, data_path) ||
        !(script = csb_v1_atari_st_animation_read_file(script_path,
            &script_size))) return 0;
    result = csb_v1_atari_st_animation_render_final_rgba(data_path, script,
        script_size, out_rgba, out_rgba_size, out_receipt);
    free(script);
    return result;
}

int csb_v1_atari_st_animation_render_presented_from_root_rgba(
    const char *search_root, const char *cache_root,
    uint16_t presentation_index, uint8_t *out_rgba, size_t out_rgba_size,
    CSB_V1_AtariStAnimationTraceReceipt *out_receipt)
{
    CSB_V1_AtariStAnimationDiscoveryReceipt discovery;
    char script_path[ASSET_PATH_MAX];
    char data_path[ASSET_PATH_MAX];
    uint8_t *script;
    size_t script_size = 0u;
    int result;

    if (!csb_v1_atari_st_animation_discover(search_root, &discovery) ||
        !csb_v1_atari_st_animation_materialize(&discovery, cache_root,
            script_path, data_path) ||
        !(script = csb_v1_atari_st_animation_read_file(script_path,
            &script_size))) return 0;
    result = csb_v1_atari_st_animation_render_presented_rgba(data_path, script,
        script_size, presentation_index, out_rgba, out_rgba_size, out_receipt);
    free(script);
    return result;
}

int csb_v1_atari_st_animation_decode_presented_from_root_indexed(
    const char *search_root, const char *cache_root,
    uint16_t presentation_index,
    uint8_t out_indexed[CSB_V1_ATARI_ST_ANIMATION_INDEXED_BYTES],
    uint8_t out_palette[16][3],
    CSB_V1_AtariStAnimationTraceReceipt *out_receipt)
{
    CSB_V1_AtariStAnimationDiscoveryReceipt discovery;
    char script_path[ASSET_PATH_MAX];
    char data_path[ASSET_PATH_MAX];
    uint8_t *script;
    size_t script_size = 0u;
    int result;

    if (!csb_v1_atari_st_animation_discover(search_root, &discovery) ||
        !csb_v1_atari_st_animation_materialize(&discovery, cache_root,
            script_path, data_path) ||
        !(script = csb_v1_atari_st_animation_read_file(script_path,
            &script_size))) return 0;
    result = csb_v1_atari_st_animation_decode_presented_indexed(data_path,
        script, script_size, presentation_index, out_indexed, out_palette,
        out_receipt);
    free(script);
    return result;
}

int csb_v1_atari_st_animation_decode_frame_at_vbl_from_root_indexed(
    const char *search_root, const char *cache_root, uint32_t target_vbl,
    uint8_t out_indexed[CSB_V1_ATARI_ST_ANIMATION_INDEXED_BYTES],
    uint8_t out_palette[16][3],
    CSB_V1_AtariStAnimationTraceReceipt *out_receipt)
{
    CSB_V1_AtariStAnimationDiscoveryReceipt discovery;
    char script_path[ASSET_PATH_MAX];
    char data_path[ASSET_PATH_MAX];
    uint8_t *script;
    size_t script_size = 0u;
    int result;

    if (!csb_v1_atari_st_animation_discover(search_root, &discovery) ||
        !csb_v1_atari_st_animation_materialize(&discovery, cache_root,
            script_path, data_path) ||
        !(script = csb_v1_atari_st_animation_read_file(script_path,
            &script_size))) return 0;
    result = csb_v1_atari_st_animation_decode_frame_at_vbl_indexed(data_path,
        script, script_size, target_vbl, out_indexed, out_palette, out_receipt);
    free(script);
    return result;
}

int csb_v1_atari_st_animation_trace_from_root(
    const char *search_root, const char *cache_root,
    CSB_V1_AtariStAnimationTraceReceipt *out_receipt)
{
    CSB_V1_AtariStAnimationDiscoveryReceipt discovery;
    char script_path[ASSET_PATH_MAX];
    char data_path[ASSET_PATH_MAX];
    uint8_t *script;
    size_t script_size = 0u;
    int result;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!csb_v1_atari_st_animation_discover(search_root, &discovery) ||
        !csb_v1_atari_st_animation_materialize(&discovery, cache_root,
            script_path, data_path) ||
        !(script = csb_v1_atari_st_animation_read_file(script_path,
            &script_size))) return 0;
    result = csb_v1_atari_st_animation_trace_script(data_path, script,
        script_size, out_receipt);
    free(script);
    return result;
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
