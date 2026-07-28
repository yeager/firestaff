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
} csb_v1_atari_st_animation_slot;

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
    uint16_t values[256];
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
    memset(values, 0, sizeof(values));
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
                else if (out_receipt) out_receipt->blit_count++;
                break;
            case 7u: /* Set palette */
            case 8u: /* Fade palette */
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
                    if (instruction->opcode == 8u && out_receipt)
                        out_receipt->fade_count++;
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
                if (p[0] >= 256u || values[p[0]] == 0u) valid = 0;
                else values[p[0]]--;
                break;
            case 18u: /* NEXT */
                if (p[0] >= 256u || p[1] >= 256u) valid = 0;
                else if (values[p[0]] != 0u) {
                    if (loop_pc[p[1]] >= instruction_count) valid = 0;
                    else pc = loop_pc[p[1]];
                }
                break;
            case 19u: /* set loop counter */
                if (p[0] >= 256u) valid = 0;
                else values[p[0]] = p[1];
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
