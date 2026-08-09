#include "nexus_v1_vdp1_capture_compositor.h"

#include <stdlib.h>
#include <string.h>

static uint16_t read_le16(const uint8_t *p)
{
    return (uint16_t)p[0] | ((uint16_t)p[1] << 8);
}

static int bytes_all_zero(const uint8_t *bytes, int size)
{
    int i;
    if (!bytes || size <= 0) return 0;
    for (i = 0; i < size; ++i)
        if (bytes[i] != 0U) return 0;
    return 1;
}

static int swapped_words_equal(const uint8_t *capture, int capture_size,
                               const uint8_t *source, int source_size)
{
    int offset;

    if (!capture || !source || capture_size <= 0 ||
        capture_size != source_size || (source_size & 1) != 0) return 0;
    for (offset = 0; offset < source_size; offset += 2) {
        if (capture[offset] != source[offset + 1] ||
            capture[offset + 1] != source[offset]) return 0;
    }
    return 1;
}

static float edge(float ax, float ay, float bx, float by,
                  float px, float py)
{
    return (px - ax) * (by - ay) - (py - ay) * (bx - ax);
}

static uint8_t expand5(uint16_t word, unsigned shift)
{
    uint8_t value = (uint8_t)(((word >> shift) & 0x1fU) << 3U);
    return (uint8_t)(value | (value >> 5U));
}

static uint32_t bgr555_to_rgba(uint16_t word)
{
    /* Saturn VDP1 BGR555 stores R/G/B in bits 4..0/9..5/14..10. */
    return UINT32_C(0xff000000) |
        ((uint32_t)expand5(word, 0U) << 16U) |
        ((uint32_t)expand5(word, 5U) << 8U) |
        (uint32_t)expand5(word, 10U);
}

static int direct_color_triangle(
    Nexus_V1_Vdp1DirectColorFramebuffer *fb, const float xy[4][2],
    int ia, int ib, int ic, const uint8_t *tex, int width, int height,
    uint16_t draw_mode, int *written, int *transparent)
{
    float area;
    float min_x, max_x, min_y, max_y;
    int x, y;

    area = edge(xy[ia][0], xy[ia][1], xy[ib][0], xy[ib][1],
                xy[ic][0], xy[ic][1]);
    if (area == 0.0f) return 0;
    min_x = max_x = xy[ia][0];
    min_y = max_y = xy[ia][1];
    for (x = 0; x < 3; ++x) {
        int index = x == 0 ? ia : (x == 1 ? ib : ic);
        if (xy[index][0] < min_x) min_x = xy[index][0];
        if (xy[index][0] > max_x) max_x = xy[index][0];
        if (xy[index][1] < min_y) min_y = xy[index][1];
        if (xy[index][1] > max_y) max_y = xy[index][1];
    }
    if (max_x < 0.0f || min_x >= (float)NEXUS_FB_W ||
        max_y < 0.0f || min_y >= (float)NEXUS_FB_H) return 0;
    if (min_x < 0.0f) min_x = 0.0f;
    if (min_y < 0.0f) min_y = 0.0f;
    if (max_x > (float)(NEXUS_FB_W - 1)) max_x = (float)(NEXUS_FB_W - 1);
    if (max_y > (float)(NEXUS_FB_H - 1)) max_y = (float)(NEXUS_FB_H - 1);

    for (y = (int)min_y; y <= (int)max_y; ++y) {
        for (x = (int)min_x; x <= (int)max_x; ++x) {
            float px = (float)x + 0.5f;
            float py = (float)y + 0.5f;
            float w0 = edge(xy[ib][0], xy[ib][1], xy[ic][0], xy[ic][1],
                            px, py) / area;
            float w1 = edge(xy[ic][0], xy[ic][1], xy[ia][0], xy[ia][1],
                            px, py) / area;
            float w2 = 1.0f - w0 - w1;
            float u, v;
            int tx, ty, pixel;
            uint16_t word;

            if (w0 < 0.0f || w1 < 0.0f || w2 < 0.0f) continue;
            u = w0 * (ia == 0 ? 0.0f : 1.0f) +
                w1 * (ib == 0 ? 0.0f : 1.0f) +
                w2 * (ic == 0 ? 0.0f : 1.0f);
            v = w0 * (ia == 0 || ia == 1 ? 0.0f : 1.0f) +
                w1 * (ib == 0 || ib == 1 ? 0.0f : 1.0f) +
                w2 * (ic == 0 || ic == 1 ? 0.0f : 1.0f);
            tx = (int)(u * (float)width);
            ty = (int)(v * (float)height);
            if (tx < 0) tx = 0;
            if (ty < 0) ty = 0;
            if (tx >= width) tx = width - 1;
            if (ty >= height) ty = height - 1;
            pixel = ty * width + tx;
            word = read_le16(tex + pixel * 2);
            /* Mednafen src/ss/vdp1.cpp::TexFetch, colour mode 5. */
            if ((draw_mode & 0x0080U) == 0U &&
                (word & UINT16_C(0xc000)) == UINT16_C(0x4000)) {
                ++*transparent;
                continue;
            }
            fb->rgba_buffer[y * NEXUS_FB_W + x] = bgr555_to_rgba(word);
            ++*written;
        }
    }
    return 1;
}

int nexus_v1_vdp1_capture_decode_direct_color(
    Nexus_V1_Vdp1DirectColorFramebuffer *framebuffer,
    const Nexus_V1_Vdp1CaptureCompositeInput *input,
    Nexus_V1_Vdp1DirectColorCaptureReceipt *out_receipt)
{
    Nexus_V1_Vdp1DirectColorCaptureReceipt receipt;
    Nexus_V1_Vdp1DirectColorFramebuffer *working;
    Nexus_V1_Vdp1TextureCommand command;
    float xy[4][2];

    memset(&receipt, 0, sizeof(receipt));
    receipt.capture_only = 1;
    receipt.fallback_visuals_permitted = 0;
    if (!out_receipt) return 0;
    if (!framebuffer || !input || !input->command || !input->texture_span ||
        input->command_size != NEXUS_V1_VDP1_COMMAND_BYTES ||
        input->texture_span_size <= 0 || input->screen_origin_x < 0 ||
        input->screen_origin_x >= NEXUS_FB_W || input->screen_origin_y < 0 ||
        input->screen_origin_y >= NEXUS_FB_H ||
        !input->original_saturn_capture_verified ||
        nexus_v1_vdp1_texture_command_parse(input->command,
            input->command_size, &command) != 0 || !command.texture_command ||
        command.colour_mode != 5U || !command.texture_source_range_valid ||
        command.texture_byte_count != (uint32_t)input->texture_span_size ||
        (input->texture_span_size & 1) != 0) {
        *out_receipt = receipt;
        return 0;
    }
    working = (Nexus_V1_Vdp1DirectColorFramebuffer *)malloc(sizeof(*working));
    if (!working) {
        *out_receipt = receipt;
        return 0;
    }
    *working = *framebuffer;
    receipt.command_framed = 1;
    receipt.direct_color_mode = 1;
    receipt.source_word_order_verified = 1;
    receipt.coordinate_words_framed = command.coordinate_words_framed;
    receipt.original_saturn_capture_verified = 1;
    xy[0][0] = (float)input->screen_origin_x + (float)command.xa;
    xy[0][1] = (float)input->screen_origin_y + (float)command.ya;
    xy[1][0] = (float)input->screen_origin_x + (float)command.xb;
    xy[1][1] = (float)input->screen_origin_y + (float)command.yb;
    xy[2][0] = (float)input->screen_origin_x + (float)command.xc;
    xy[2][1] = (float)input->screen_origin_y + (float)command.yc;
    xy[3][0] = (float)input->screen_origin_x + (float)command.xd;
    xy[3][1] = (float)input->screen_origin_y + (float)command.yd;
    if (xy[0][0] == xy[1][0] && xy[0][1] == xy[1][1]) {
        free(working);
        *out_receipt = receipt;
        return 0;
    }
    (void)direct_color_triangle(working, xy, 0, 1, 2, input->texture_span,
        command.texture_width, command.texture_height, command.draw_mode,
        &receipt.written_pixels, &receipt.transparent_pixels);
    (void)direct_color_triangle(working, xy, 0, 2, 3, input->texture_span,
        command.texture_width, command.texture_height, command.draw_mode,
        &receipt.written_pixels, &receipt.transparent_pixels);
    receipt.valid = receipt.written_pixels > 0;
    /* No DGN owner/material receipt is accepted by this API. */
    receipt.renderer_permitted = 0;
    if (receipt.valid) *framebuffer = *working;
    free(working);
    *out_receipt = receipt;
    return receipt.valid;
}

static int composite_triangle(Nexus_Framebuffer *fb,
                              const float xy[4][2],
                              int ia, int ib, int ic,
                              const uint8_t *tex, int width, int height,
                              int palette_base,
                              uint16_t draw_mode, int *written,
                              int *transparent, int *end_code,
                              uint8_t end_rows[256])
{
    float area;
    float min_x, max_x, min_y, max_y;
    int x, y;

    area = edge(xy[ia][0], xy[ia][1], xy[ib][0], xy[ib][1],
                xy[ic][0], xy[ic][1]);
    if (area == 0.0f) return 0;
    min_x = xy[ia][0];
    max_x = xy[ia][0];
    min_y = xy[ia][1];
    max_y = xy[ia][1];
    for (x = 0; x < 3; ++x) {
        int index = x == 0 ? ia : (x == 1 ? ib : ic);
        if (xy[index][0] < min_x) min_x = xy[index][0];
        if (xy[index][0] > max_x) max_x = xy[index][0];
        if (xy[index][1] < min_y) min_y = xy[index][1];
        if (xy[index][1] > max_y) max_y = xy[index][1];
    }
    if (max_x < 0.0f || min_x >= (float)NEXUS_FB_W ||
        max_y < 0.0f || min_y >= (float)NEXUS_FB_H) return 0;
    if (min_x < 0.0f) min_x = 0.0f;
    if (min_y < 0.0f) min_y = 0.0f;
    if (max_x > (float)(NEXUS_FB_W - 1)) max_x = (float)(NEXUS_FB_W - 1);
    if (max_y > (float)(NEXUS_FB_H - 1)) max_y = (float)(NEXUS_FB_H - 1);

    for (y = (int)min_y; y <= (int)max_y; ++y) {
        for (x = (int)min_x; x <= (int)max_x; ++x) {
            float px = (float)x + 0.5f;
            float py = (float)y + 0.5f;
            float w0 = edge(xy[ib][0], xy[ib][1], xy[ic][0], xy[ic][1],
                            px, py) / area;
            float w1 = edge(xy[ic][0], xy[ic][1], xy[ia][0], xy[ia][1],
                            px, py) / area;
            float w2 = 1.0f - w0 - w1;
            float u, v;
            int tx, ty, pixel, index;
            uint8_t texel;

            if (w0 < 0.0f || w1 < 0.0f || w2 < 0.0f) continue;
            u = w0 * (ia == 0 ? 0.0f : ia == 1 ? 1.0f : 1.0f) +
                w1 * (ib == 0 ? 0.0f : ib == 1 ? 1.0f : 1.0f) +
                w2 * (ic == 0 ? 0.0f : ic == 1 ? 1.0f : 1.0f);
            v = w0 * (ia == 0 || ia == 1 ? 0.0f : 1.0f) +
                w1 * (ib == 0 || ib == 1 ? 0.0f : 1.0f) +
                w2 * (ic == 0 || ic == 1 ? 0.0f : 1.0f);
            tx = (int)(u * (float)width);
            ty = (int)(v * (float)height);
            if (tx < 0) tx = 0;
            if (ty < 0) ty = 0;
            if (tx >= width) tx = width - 1;
            if (ty >= height) ty = height - 1;
            pixel = ty * width + tx;
            texel = (pixel & 1) == 0 ? (uint8_t)(tex[pixel >> 1] >> 4U) :
                (uint8_t)(tex[pixel >> 1] & 0x0fU);
            if (end_rows[ty]) continue;
            if (texel == 0U && (draw_mode & 0x0040U) == 0U) {
                ++*transparent;
                continue;
            }
            if (texel == 0x0fU && (draw_mode & 0x0080U) == 0U) {
                end_rows[ty] = 1U;
                ++*end_code;
                continue;
            }
            index = y * NEXUS_FB_W + x;
            fb->color_buffer[index] = (uint8_t)(palette_base + texel);
            fb->z_buffer[index] = 0.0f;
            ++*written;
        }
    }
    return 1;
}

int nexus_v1_vdp1_capture_composite_mode1(
    Nexus_Framebuffer *framebuffer,
    const Nexus_V1_Vdp1CaptureCompositeInput *input,
    Nexus_V1_Vdp1CaptureCompositeReceipt *out_receipt)
{
    Nexus_V1_Vdp1CaptureCompositeReceipt receipt;
    Nexus_Framebuffer *working;
    Nexus_V1_Vdp1TextureCommand command;
    uint16_t palette[16];
    float xy[4][2];
    uint8_t end_rows[256] = {0};
    int index;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    receipt.fallback_visuals_permitted = 0;
    if (!framebuffer || !input || !input->command || !input->texture_span ||
        (!input->transparent_capture_noop_verified &&
         (!input->palette_state || !input->dgn_image || !input->dgn_palette)) ||
        input->command_size != NEXUS_V1_VDP1_COMMAND_BYTES ||
        (!input->transparent_capture_noop_verified &&
         (input->palette_state_size != 32 || input->dgn_palette_size != 32 ||
          input->dgn_image_size <= 0)) ||
        input->texture_span_size <= 0 ||
        input->screen_origin_x < 0 || input->screen_origin_x >= NEXUS_FB_W ||
        input->screen_origin_y < 0 || input->screen_origin_y >= NEXUS_FB_H ||
        input->palette_slot_base < 0 || input->palette_slot_base > 240 ||
        (!input->transparent_capture_noop_verified &&
         !input->dgn_source_hash_verified) ||
        !input->original_saturn_capture_verified ||
        nexus_v1_vdp1_texture_command_parse(input->command,
            input->command_size, &command) != 0 || !command.texture_command ||
        command.colour_mode != 1U || !command.texture_source_range_valid ||
        command.texture_byte_count != (uint32_t)input->texture_span_size ||
        (!input->transparent_capture_noop_verified &&
         (input->texture_span_size != input->dgn_image_size ||
          !swapped_words_equal(input->texture_span, input->texture_span_size,
                               input->dgn_image, input->dgn_image_size) ||
          !swapped_words_equal(input->palette_state, input->palette_state_size,
                               input->dgn_palette, input->dgn_palette_size))) ||
        (input->transparent_capture_noop_verified &&
         ((command.draw_mode & UINT16_C(0x0040)) != 0U ||
          !bytes_all_zero(input->texture_span, input->texture_span_size)))) {
        *out_receipt = receipt;
        return 0;
    }
    /* Keep a single-command replay atomic as well as the sequence replay.
     * A valid command can still produce no visible pixel when its captured
     * coordinates are outside the host framebuffer; palette and pixels must
     * not be published for that failed receipt. */
    working = (Nexus_Framebuffer *)malloc(sizeof(*working));
    if (!working) {
        *out_receipt = receipt;
        return 0;
    }
    *working = *framebuffer;
    receipt.command_framed = 1;
    receipt.mode1_lookup = 1;
    receipt.coordinate_words_framed = command.coordinate_words_framed;
    receipt.original_saturn_capture_verified = 1;
    receipt.source_join_verified = input->transparent_capture_noop_verified ? 0 : 1;
    receipt.palette_join_verified = input->transparent_capture_noop_verified ? 0 : 1;
    receipt.transparent_noop_verified = input->transparent_capture_noop_verified ? 1 : 0;
    receipt.palette_slot_base = input->palette_slot_base;
    receipt.screen_origin_x = input->screen_origin_x;
    receipt.screen_origin_y = input->screen_origin_y;

    if (input->transparent_capture_noop_verified) {
        receipt.transparent_pixels = command.texture_width * command.texture_height;
        receipt.valid = 1;
        receipt.renderer_permitted = 0;
        free(working);
        *out_receipt = receipt;
        return 1;
    }
    for (index = 0; index < 16; ++index) {
        palette[index] = read_le16(input->palette_state + index * 2);
        working->palette[input->palette_slot_base + index] =
            bgr555_to_rgba(palette[index]);
    }
    xy[0][0] = (float)input->screen_origin_x + (float)command.xa;
    xy[0][1] = (float)input->screen_origin_y + (float)command.ya;
    xy[1][0] = (float)input->screen_origin_x + (float)command.xb;
    xy[1][1] = (float)input->screen_origin_y + (float)command.yb;
    xy[2][0] = (float)input->screen_origin_x + (float)command.xc;
    xy[2][1] = (float)input->screen_origin_y + (float)command.yc;
    xy[3][0] = (float)input->screen_origin_x + (float)command.xd;
    xy[3][1] = (float)input->screen_origin_y + (float)command.yd;
    if (xy[0][0] == xy[1][0] && xy[0][1] == xy[1][1]) {
        if (input->capture_allow_zero_pixel_command) {
            receipt.valid = 1;
            receipt.renderer_permitted = 0;
            free(working);
            *out_receipt = receipt;
            return 1;
        }
        free(working);
        *out_receipt = receipt;
        return 0;
    }
    (void)composite_triangle(working, xy, 0, 1, 2,
        input->texture_span, command.texture_width, command.texture_height,
        input->palette_slot_base, command.draw_mode,
        &receipt.written_pixels, &receipt.transparent_pixels,
        &receipt.end_code_pixels, end_rows);
    (void)composite_triangle(working, xy, 0, 2, 3,
        input->texture_span, command.texture_width, command.texture_height,
        input->palette_slot_base, command.draw_mode,
        &receipt.written_pixels, &receipt.transparent_pixels,
        &receipt.end_code_pixels, end_rows);
    receipt.valid = receipt.written_pixels > 0 ||
        input->capture_allow_zero_pixel_command;
    receipt.renderer_permitted = receipt.written_pixels > 0;
    if (receipt.valid) *framebuffer = *working;
    free(working);
    *out_receipt = receipt;
    return receipt.valid;
}

int nexus_v1_vdp1_capture_composite_mode1_sequence(
    Nexus_Framebuffer *framebuffer,
    const Nexus_V1_Vdp1CaptureSequenceInput *input,
    Nexus_V1_Vdp1CaptureSequenceReceipt *out_receipt)
{
    Nexus_V1_Vdp1CaptureSequenceReceipt receipt;
    Nexus_Framebuffer *working;
    int i;

    memset(&receipt, 0, sizeof(receipt));
    if (!out_receipt) return 0;
    if (!framebuffer || !input || !input->commands ||
        input->command_count <= 0 || input->command_count > 256 ||
        !input->system_clip_state_verified ||
        !input->local_coordinate_state_verified ||
        !input->display_origin_state_verified ||
        !input->command_order_verified || !input->end_record_verified) {
        *out_receipt = receipt;
        return 0;
    }
    working = (Nexus_Framebuffer *)malloc(sizeof(*working));
    if (!working) {
        *out_receipt = receipt;
        return 0;
    }
    *working = *framebuffer;
    receipt.sequence_state_verified = 1;
    receipt.command_count = input->command_count;
    receipt.display_origin_verified = 1;
    receipt.display_origin_x = input->display_origin_x;
    receipt.display_origin_y = input->display_origin_y;
    receipt.command_order_verified = 1;
    receipt.end_record_verified = 1;
    for (i = 0; i < input->command_count; ++i) {
        Nexus_V1_Vdp1CaptureCompositeReceipt command_receipt;
        if (!nexus_v1_vdp1_capture_composite_mode1(
                working, &input->commands[i], &command_receipt)) {
            receipt.command_frames_verified += command_receipt.command_framed;
            receipt.source_joins_verified += command_receipt.source_join_verified;
            receipt.palette_joins_verified += command_receipt.palette_join_verified;
            receipt.transparent_pixels += command_receipt.transparent_pixels;
            if (input->commands[i].capture_allow_zero_pixel_command) {
                ++receipt.capture_gap_commands;
                continue;
            }
            free(working);
            *out_receipt = receipt;
            return 0;
        }
        if (input->commands[i].screen_origin_x != input->display_origin_x ||
            input->commands[i].screen_origin_y != input->display_origin_y) {
            if (input->commands[i].capture_allow_zero_pixel_command) {
                ++receipt.capture_gap_commands;
                continue;
            }
            free(working);
            *out_receipt = receipt;
            return 0;
        }
        receipt.command_frames_verified += command_receipt.command_framed;
        receipt.source_joins_verified += command_receipt.source_join_verified;
        receipt.palette_joins_verified += command_receipt.palette_join_verified;
        receipt.written_pixels += command_receipt.written_pixels;
        receipt.transparent_pixels += command_receipt.transparent_pixels;
        receipt.end_code_pixels += command_receipt.end_code_pixels;
        receipt.transparent_noop_commands +=
            command_receipt.transparent_noop_verified;
    }
    if (receipt.command_frames_verified + receipt.capture_gap_commands !=
            input->command_count ||
        receipt.source_joins_verified + receipt.transparent_noop_commands +
            receipt.capture_gap_commands != input->command_count ||
        receipt.palette_joins_verified + receipt.transparent_noop_commands +
            receipt.capture_gap_commands !=
            input->command_count) {
        free(working);
        *out_receipt = receipt;
        return 0;
    }
    *framebuffer = *working;
    free(working);
    receipt.valid = receipt.written_pixels > 0 ||
        (input->command_count > 0 && receipt.capture_gap_commands > 0);
    receipt.renderer_permitted = receipt.written_pixels > 0;
    *out_receipt = receipt;
    return receipt.valid;
}

int nexus_v1_vdp1_capture_replay_vram_sequence(
    Nexus_Framebuffer *framebuffer,
    const Nexus_V1_Vdp1CaptureVramSequenceInput *input,
    Nexus_V1_Vdp1CaptureVramSequenceReceipt *out_receipt)
{
    Nexus_V1_Vdp1CaptureVramSequenceReceipt receipt;
    Nexus_V1_Vdp1CaptureSequenceInput replay_input;
    Nexus_V1_Vdp1CaptureCompositeInput commands[
        NEXUS_V1_VDP1_SEQUENCE_MAX_COMMANDS];
    int i;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    receipt.semantic_admission_blocked = 1;
    memset(&replay_input, 0, sizeof(replay_input));
    memset(commands, 0, sizeof(commands));
    if (!framebuffer || !input || !input->vdp1_vram ||
        input->vdp1_vram_size != (int)NEXUS_V1_VDP1_VRAM_BYTES ||
        !input->original_saturn_capture_verified || !input->resolve_material ||
        !nexus_v1_vdp1_command_sequence_frame(
            &(Nexus_V1_Vdp1CommandSequenceInput){
                input->vdp1_vram, input->vdp1_vram_size, input->copr_word},
            &receipt.command_sequence) ||
        !receipt.command_sequence.valid || !receipt.command_sequence.complete ||
        !receipt.command_sequence.display_origin_verified ||
        receipt.command_sequence.draw_count <= 0 ||
        receipt.command_sequence.draw_count >
            NEXUS_V1_VDP1_SEQUENCE_MAX_COMMANDS) {
        *out_receipt = receipt;
        return 0;
    }

    for (i = 0; i < receipt.command_sequence.command_count; ++i) {
        uint32_t offset = receipt.command_sequence.command_byte_offsets[i];
        uint32_t palette_offset;
        Nexus_V1_Vdp1TextureCommand parsed;
        const uint8_t *command = input->vdp1_vram + offset;
        Nexus_V1_Vdp1CaptureCompositeInput *resolved;

        if (nexus_v1_vdp1_texture_command_parse(
                command, NEXUS_V1_VDP1_COMMAND_BYTES, &parsed) != 0) {
            if (input->mode1_only_capture) {
                ++receipt.skipped_non_draw_commands;
                continue;
            }
            *out_receipt = receipt;
            return 0;
        }
        if (!parsed.texture_command || parsed.end_command) {
            if (input->mode1_only_capture)
                ++receipt.skipped_non_draw_commands;
            continue;
        }
        ++receipt.draw_commands_seen;
        if (receipt.draw_commands_seen >
                NEXUS_V1_VDP1_SEQUENCE_MAX_COMMANDS) {
            ++receipt.unresolved_draw_commands;
            *out_receipt = receipt;
            return 0;
        }
        if (!parsed.texture_source_range_valid ||
            parsed.texture_source_byte_end > NEXUS_V1_VDP1_VRAM_BYTES) {
            if (input->mode1_only_capture) {
                ++receipt.unowned_mode1_draw_commands;
                continue;
            }
            ++receipt.unresolved_draw_commands;
            *out_receipt = receipt;
            return 0;
        }
        if (parsed.colour_mode != 1U) {
            if (!input->mode1_only_capture) {
                ++receipt.unresolved_draw_commands;
                *out_receipt = receipt;
                return 0;
            }
            ++receipt.unowned_non_mode1_draw_commands;
            continue;
        }
        /* CMDCOLR is a VDP1 word address after the documented <<2
         * conversion; this byte-buffer API therefore uses <<3. */
        palette_offset = (((uint32_t)parsed.colour_control & ~UINT32_C(3)) << 3U);
        if (palette_offset > NEXUS_V1_VDP1_VRAM_BYTES - 32U) {
            if (input->mode1_only_capture) {
                ++receipt.unowned_mode1_draw_commands;
                continue;
            }
            ++receipt.unresolved_draw_commands;
            *out_receipt = receipt;
            return 0;
        }
        resolved = &commands[receipt.draw_commands_resolved];
        if (!input->resolve_material(
                input->vdp1_vram, input->vdp1_vram_size, command,
                NEXUS_V1_VDP1_COMMAND_BYTES, &parsed, offset, resolved,
                input->resolver_context)) {
            if (input->mode1_only_capture) {
                ++receipt.unowned_mode1_draw_commands;
                continue;
            }
            ++receipt.unresolved_draw_commands;
            *out_receipt = receipt;
            return 0;
        }
        ++receipt.draw_commands_resolved;
        resolved->command = command;
        resolved->command_size = NEXUS_V1_VDP1_COMMAND_BYTES;
        resolved->texture_span = input->vdp1_vram +
            parsed.texture_source_byte_offset;
        resolved->texture_span_size = (int)parsed.texture_byte_count;
        resolved->palette_state = input->vdp1_vram + palette_offset;
        resolved->palette_state_size = 32;
        resolved->original_saturn_capture_verified = 1;
        resolved->capture_allow_zero_pixel_command = input->mode1_only_capture;
        resolved->screen_origin_x = receipt.command_sequence.display_origin_x;
        resolved->screen_origin_y = receipt.command_sequence.display_origin_y;
    }
    if ((input->mode1_only_capture &&
         receipt.draw_commands_seen + receipt.skipped_non_draw_commands !=
             receipt.command_sequence.command_count) ||
        (!input->mode1_only_capture &&
         receipt.draw_commands_seen != receipt.command_sequence.draw_count) ||
        receipt.draw_commands_resolved +
            receipt.unowned_non_mode1_draw_commands +
            receipt.unowned_mode1_draw_commands !=
            receipt.draw_commands_seen) {
        *out_receipt = receipt;
        return 0;
    }

    replay_input.commands = commands;
    replay_input.command_count = receipt.draw_commands_resolved;
    replay_input.system_clip_state_verified =
        receipt.command_sequence.system_clip_count > 0;
    replay_input.local_coordinate_state_verified =
        receipt.command_sequence.local_coordinate_count > 0;
    replay_input.display_origin_state_verified =
        receipt.command_sequence.display_origin_verified;
    replay_input.display_origin_x = receipt.command_sequence.display_origin_x;
    replay_input.display_origin_y = receipt.command_sequence.display_origin_y;
    replay_input.command_order_verified =
        receipt.command_sequence.command_order_verified;
    replay_input.end_record_verified =
        receipt.command_sequence.end_record_verified;
    if (!replay_input.system_clip_state_verified && input->mode1_only_capture) {
        receipt.system_clip_state_missing = 1;
        /* The bounded compositor still needs a clipping envelope. Use its
         * framebuffer bounds only for capture replay and keep renderer
         * permission closed via the explicit receipt above. */
        replay_input.system_clip_state_verified = 1;
    }
    if (!replay_input.system_clip_state_verified && !input->mode1_only_capture)
        replay_input.system_clip_state_verified = 0;
    if ((!replay_input.system_clip_state_verified && !input->mode1_only_capture) ||
        !replay_input.local_coordinate_state_verified ||
        !nexus_v1_vdp1_capture_composite_mode1_sequence(
            framebuffer, &replay_input, &receipt.replay)) {
        *out_receipt = receipt;
        return 0;
    }
    if (receipt.system_clip_state_missing)
        receipt.replay.renderer_permitted = 0;
    receipt.valid = 1;
    *out_receipt = receipt;
    return 1;
}

int nexus_v1_vdp1_capture_replay_runtime_frame(
    Nexus_Framebuffer *framebuffer,
    const uint8_t *capture_bytes, size_t capture_byte_count,
    unsigned int frame_index,
    Nexus_V1_Vdp1CaptureSequenceMaterialResolver resolve_material,
    void *resolver_context,
    Nexus_V1_SaturnRuntimeCaptureFrameReceipt *out_frame_receipt,
    Nexus_V1_Vdp1CaptureVramSequenceReceipt *out_replay_receipt)
{
    Nexus_V1_SaturnRuntimeCaptureFrameReceipt frame;
    Nexus_V1_Vdp1CaptureVramSequenceReceipt replay;
    Nexus_V1_Vdp1CaptureVramSequenceInput input;

    memset(&frame, 0, sizeof(frame));
    memset(&replay, 0, sizeof(replay));
    if (out_frame_receipt) *out_frame_receipt = frame;
    if (out_replay_receipt) *out_replay_receipt = replay;
    if (!nexus_v1_saturn_runtime_capture_frame(
            capture_bytes, capture_byte_count, frame_index, &frame) ||
        !frame.valid || !frame.vdp1_state_valid || !frame.vdp1_state_present ||
        !frame.vdp1_vram || !resolve_material) {
        if (out_frame_receipt) *out_frame_receipt = frame;
        return 0;
    }
    memset(&input, 0, sizeof(input));
    input.vdp1_vram = frame.vdp1_vram;
    input.vdp1_vram_size = (int)frame.vdp1_vram_size;
    input.copr_word = frame.copr_word;
    input.original_saturn_capture_verified = 1;
    input.resolve_material = resolve_material;
    input.resolver_context = resolver_context;
    if (!nexus_v1_vdp1_capture_replay_vram_sequence(
            framebuffer, &input, &replay)) {
        if (out_frame_receipt) *out_frame_receipt = frame;
        if (out_replay_receipt) *out_replay_receipt = replay;
        return 0;
    }
    if (out_frame_receipt) *out_frame_receipt = frame;
    if (out_replay_receipt) *out_replay_receipt = replay;
    return 1;
}

int nexus_v1_vdp1_capture_replay_runtime_frame_mode1_sequence(
    Nexus_Framebuffer *framebuffer,
    const uint8_t *capture_bytes, size_t capture_byte_count,
    unsigned int frame_index,
    Nexus_V1_Vdp1CaptureSequenceMaterialResolver resolve_material,
    void *resolver_context,
    Nexus_V1_SaturnRuntimeCaptureFrameReceipt *out_frame_receipt,
    Nexus_V1_Vdp1CaptureVramSequenceReceipt *out_replay_receipt)
{
    Nexus_V1_SaturnRuntimeCaptureFrameReceipt frame;
    Nexus_V1_Vdp1CaptureVramSequenceReceipt replay;
    Nexus_V1_Vdp1CaptureVramSequenceInput input;

    memset(&frame, 0, sizeof(frame));
    memset(&replay, 0, sizeof(replay));
    if (out_frame_receipt) *out_frame_receipt = frame;
    if (out_replay_receipt) *out_replay_receipt = replay;
    if (!framebuffer || !capture_bytes || !resolve_material ||
        !nexus_v1_saturn_runtime_capture_frame(
            capture_bytes, capture_byte_count, frame_index, &frame) ||
        !frame.valid || !frame.vdp1_state_valid || !frame.vdp1_state_present ||
        !frame.vdp1_vram) {
        if (out_frame_receipt) *out_frame_receipt = frame;
        return 0;
    }
    memset(&input, 0, sizeof(input));
    input.vdp1_vram = frame.vdp1_vram;
    input.vdp1_vram_size = (int)frame.vdp1_vram_size;
    input.copr_word = frame.copr_word;
    input.original_saturn_capture_verified = 1;
    input.resolve_material = resolve_material;
    input.resolver_context = resolver_context;
    input.mode1_only_capture = 1;
    if (!nexus_v1_vdp1_capture_replay_vram_sequence(
            framebuffer, &input, &replay)) {
        if (out_frame_receipt) *out_frame_receipt = frame;
        if (out_replay_receipt) *out_replay_receipt = replay;
        return 0;
    }
    if (out_frame_receipt) *out_frame_receipt = frame;
    if (out_replay_receipt) *out_replay_receipt = replay;
    return 1;
}

int nexus_v1_vdp1_capture_decode_direct_color_runtime_frame(
    Nexus_V1_Vdp1DirectColorFramebuffer *framebuffer,
    const uint8_t *capture_bytes, size_t capture_byte_count,
    unsigned int frame_index,
    Nexus_V1_SaturnRuntimeCaptureFrameReceipt *out_frame_receipt,
    Nexus_V1_Vdp1CommandSequenceReceipt *out_sequence_receipt,
    Nexus_V1_Vdp1DirectColorCaptureReceipt *out_direct_receipt)
{
    Nexus_V1_SaturnRuntimeCaptureFrameReceipt frame;
    Nexus_V1_Vdp1CommandSequenceReceipt sequence;
    Nexus_V1_Vdp1DirectColorCaptureReceipt direct;
    Nexus_V1_Vdp1CommandSequenceInput sequence_input;
    Nexus_V1_Vdp1CaptureCompositeInput input;
    int i;

    memset(&frame, 0, sizeof(frame));
    memset(&sequence, 0, sizeof(sequence));
    memset(&direct, 0, sizeof(direct));
    direct.capture_only = 1;
    direct.fallback_visuals_permitted = 0;
    if (out_frame_receipt) *out_frame_receipt = frame;
    if (out_sequence_receipt) *out_sequence_receipt = sequence;
    if (out_direct_receipt) *out_direct_receipt = direct;
    if (!framebuffer || !capture_bytes ||
        !nexus_v1_saturn_runtime_capture_frame(
            capture_bytes, capture_byte_count, frame_index, &frame) ||
        !frame.valid || !frame.vdp1_state_valid || !frame.vdp1_state_present ||
        !frame.vdp1_vram || frame.vdp1_vram_size != NEXUS_V1_VDP1_VRAM_BYTES) {
        if (out_frame_receipt) *out_frame_receipt = frame;
        return 0;
    }
    sequence_input.vdp1_vram = frame.vdp1_vram;
    sequence_input.vdp1_vram_size = (int)frame.vdp1_vram_size;
    sequence_input.copr_word = frame.copr_word;
    if (!nexus_v1_vdp1_command_sequence_frame(&sequence_input, &sequence) ||
        !sequence.valid || !sequence.complete ||
        !sequence.display_origin_verified || sequence.draw_count <= 0) {
        if (out_frame_receipt) *out_frame_receipt = frame;
        if (out_sequence_receipt) *out_sequence_receipt = sequence;
        return 0;
    }
    for (i = 0; i < sequence.command_count; ++i) {
        Nexus_V1_Vdp1TextureCommand command;
        uint32_t offset = sequence.command_byte_offsets[i];

        if (nexus_v1_vdp1_texture_command_parse(
                frame.vdp1_vram + offset,
                NEXUS_V1_VDP1_COMMAND_BYTES, &command) != 0 ||
            !command.texture_command || command.colour_mode != 5U) continue;
        memset(&input, 0, sizeof(input));
        input.command = frame.vdp1_vram + offset;
        input.command_size = NEXUS_V1_VDP1_COMMAND_BYTES;
        input.texture_span = frame.vdp1_vram +
            command.texture_source_byte_offset;
        input.texture_span_size = (int)command.texture_byte_count;
        input.original_saturn_capture_verified = 1;
        input.screen_origin_x = sequence.display_origin_x;
        input.screen_origin_y = sequence.display_origin_y;
        if (nexus_v1_vdp1_capture_decode_direct_color(
                framebuffer, &input, &direct)) {
            direct.command_byte_offset = offset;
            if (out_frame_receipt) *out_frame_receipt = frame;
            if (out_sequence_receipt) *out_sequence_receipt = sequence;
            if (out_direct_receipt) *out_direct_receipt = direct;
            return 1;
        }
        break;
    }
    if (out_frame_receipt) *out_frame_receipt = frame;
    if (out_sequence_receipt) *out_sequence_receipt = sequence;
    if (out_direct_receipt) *out_direct_receipt = direct;
    return 0;
}

int nexus_v1_vdp1_capture_replay_runtime_frame_mode1_material(
    Nexus_Framebuffer *framebuffer,
    const uint8_t *capture_bytes, size_t capture_byte_count,
    unsigned int frame_index,
    Nexus_V1_Vdp1CaptureSequenceMaterialResolver resolve_material,
    void *resolver_context,
    Nexus_V1_SaturnRuntimeCaptureFrameReceipt *out_frame_receipt,
    Nexus_V1_Vdp1CommandSequenceReceipt *out_sequence_receipt,
    Nexus_V1_Vdp1CaptureCompositeReceipt *out_composite_receipt,
    uint32_t *out_command_byte_offset)
{
    Nexus_V1_SaturnRuntimeCaptureFrameReceipt frame;
    Nexus_V1_Vdp1CommandSequenceReceipt sequence;
    Nexus_V1_Vdp1CaptureCompositeReceipt composite;
    Nexus_V1_Vdp1CommandSequenceInput sequence_input;
    int i;

    memset(&frame, 0, sizeof(frame));
    memset(&sequence, 0, sizeof(sequence));
    memset(&composite, 0, sizeof(composite));
    if (out_frame_receipt) *out_frame_receipt = frame;
    if (out_sequence_receipt) *out_sequence_receipt = sequence;
    if (out_composite_receipt) *out_composite_receipt = composite;
    if (out_command_byte_offset) *out_command_byte_offset = 0U;
    if (!framebuffer || !capture_bytes || !resolve_material ||
        !nexus_v1_saturn_runtime_capture_frame(
            capture_bytes, capture_byte_count, frame_index, &frame) ||
        !frame.valid || !frame.vdp1_state_valid || !frame.vdp1_state_present ||
        !frame.vdp1_vram || frame.vdp1_vram_size != NEXUS_V1_VDP1_VRAM_BYTES) {
        if (out_frame_receipt) *out_frame_receipt = frame;
        return 0;
    }
    sequence_input.vdp1_vram = frame.vdp1_vram;
    sequence_input.vdp1_vram_size = (int)frame.vdp1_vram_size;
    sequence_input.copr_word = frame.copr_word;
    if (!nexus_v1_vdp1_command_sequence_frame(&sequence_input, &sequence) ||
        !sequence.valid || !sequence.complete ||
        !sequence.display_origin_verified || sequence.draw_count <= 0) {
        if (out_frame_receipt) *out_frame_receipt = frame;
        if (out_sequence_receipt) *out_sequence_receipt = sequence;
        return 0;
    }
    for (i = 0; i < sequence.command_count; ++i) {
        Nexus_V1_Vdp1TextureCommand parsed;
        Nexus_V1_Vdp1CaptureCompositeInput input;
        uint32_t offset = sequence.command_byte_offsets[i];
        uint32_t palette_offset;
        const uint8_t *command = frame.vdp1_vram + offset;

        if (nexus_v1_vdp1_texture_command_parse(
                command, NEXUS_V1_VDP1_COMMAND_BYTES, &parsed) != 0 ||
            !parsed.texture_command || parsed.colour_mode != 1U ||
            !parsed.texture_source_range_valid ||
            parsed.texture_source_byte_end > NEXUS_V1_VDP1_VRAM_BYTES) continue;
        palette_offset = (((uint32_t)parsed.colour_control & ~UINT32_C(3)) << 3U);
        if (palette_offset > NEXUS_V1_VDP1_VRAM_BYTES - 32U) continue;
        memset(&input, 0, sizeof(input));
        if (!resolve_material(frame.vdp1_vram, (int)frame.vdp1_vram_size,
                              command, NEXUS_V1_VDP1_COMMAND_BYTES, &parsed,
                              offset, &input, resolver_context)) continue;
        input.command = command;
        input.command_size = NEXUS_V1_VDP1_COMMAND_BYTES;
        input.texture_span = frame.vdp1_vram +
            parsed.texture_source_byte_offset;
        input.texture_span_size = (int)parsed.texture_byte_count;
        input.palette_state = frame.vdp1_vram + palette_offset;
        input.palette_state_size = 32;
        input.original_saturn_capture_verified = 1;
        input.screen_origin_x = sequence.display_origin_x;
        input.screen_origin_y = sequence.display_origin_y;
        if (!nexus_v1_vdp1_capture_composite_mode1(
                framebuffer, &input, &composite)) continue;
        if (out_frame_receipt) *out_frame_receipt = frame;
        if (out_sequence_receipt) *out_sequence_receipt = sequence;
        if (out_composite_receipt) *out_composite_receipt = composite;
        if (out_command_byte_offset) *out_command_byte_offset = offset;
        return 1;
    }
    if (out_frame_receipt) *out_frame_receipt = frame;
    if (out_sequence_receipt) *out_sequence_receipt = sequence;
    if (out_composite_receipt) *out_composite_receipt = composite;
    return 0;
}
