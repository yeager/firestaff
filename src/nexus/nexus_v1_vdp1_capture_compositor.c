#include "nexus_v1_vdp1_capture_compositor.h"

#include <stdlib.h>
#include <string.h>

static uint16_t read_le16(const uint8_t *p)
{
    return (uint16_t)p[0] | ((uint16_t)p[1] << 8);
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
    Nexus_V1_Vdp1TextureCommand command;
    uint16_t palette[16];
    float xy[4][2];
    uint8_t end_rows[256] = {0};
    int index;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    receipt.fallback_visuals_permitted = 0;
    if (!framebuffer || !input || !input->command || !input->texture_span ||
        !input->palette_state || !input->dgn_image || !input->dgn_palette ||
        input->command_size != NEXUS_V1_VDP1_COMMAND_BYTES ||
        input->palette_state_size != 32 || input->dgn_palette_size != 32 ||
        input->texture_span_size <= 0 || input->dgn_image_size <= 0 ||
        input->screen_origin_x < 0 || input->screen_origin_x >= NEXUS_FB_W ||
        input->screen_origin_y < 0 || input->screen_origin_y >= NEXUS_FB_H ||
        input->palette_slot_base < 0 || input->palette_slot_base > 240 ||
        !input->dgn_source_hash_verified ||
        !input->original_saturn_capture_verified ||
        nexus_v1_vdp1_texture_command_parse(input->command,
            input->command_size, &command) != 0 || !command.texture_command ||
        command.colour_mode != 1U || !command.texture_source_range_valid ||
        command.texture_byte_count != (uint32_t)input->texture_span_size ||
        input->texture_span_size != input->dgn_image_size ||
        !swapped_words_equal(input->texture_span, input->texture_span_size,
                             input->dgn_image, input->dgn_image_size) ||
        !swapped_words_equal(input->palette_state, input->palette_state_size,
                             input->dgn_palette, input->dgn_palette_size)) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.command_framed = 1;
    receipt.mode1_lookup = 1;
    receipt.coordinate_words_framed = command.coordinate_words_framed;
    receipt.original_saturn_capture_verified = 1;
    receipt.source_join_verified = 1;
    receipt.palette_join_verified = 1;
    receipt.palette_slot_base = input->palette_slot_base;
    receipt.screen_origin_x = input->screen_origin_x;
    receipt.screen_origin_y = input->screen_origin_y;

    for (index = 0; index < 16; ++index) {
        palette[index] = read_le16(input->palette_state + index * 2);
        framebuffer->palette[input->palette_slot_base + index] =
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
        *out_receipt = receipt;
        return 0;
    }
    (void)composite_triangle(framebuffer, xy, 0, 1, 2,
        input->texture_span, command.texture_width, command.texture_height,
        input->palette_slot_base, command.draw_mode,
        &receipt.written_pixels, &receipt.transparent_pixels,
        &receipt.end_code_pixels, end_rows);
    (void)composite_triangle(framebuffer, xy, 0, 2, 3,
        input->texture_span, command.texture_width, command.texture_height,
        input->palette_slot_base, command.draw_mode,
        &receipt.written_pixels, &receipt.transparent_pixels,
        &receipt.end_code_pixels, end_rows);
    receipt.valid = receipt.written_pixels > 0;
    receipt.renderer_permitted = receipt.valid;
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
    receipt.command_order_verified = 1;
    receipt.end_record_verified = 1;
    for (i = 0; i < input->command_count; ++i) {
        Nexus_V1_Vdp1CaptureCompositeReceipt command_receipt;
        if (!nexus_v1_vdp1_capture_composite_mode1(
                working, &input->commands[i], &command_receipt)) {
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
    }
    if (receipt.command_frames_verified != input->command_count ||
        receipt.source_joins_verified != input->command_count ||
        receipt.palette_joins_verified != input->command_count) {
        free(working);
        *out_receipt = receipt;
        return 0;
    }
    *framebuffer = *working;
    free(working);
    receipt.valid = receipt.written_pixels > 0;
    receipt.renderer_permitted = receipt.valid;
    *out_receipt = receipt;
    return receipt.valid;
}
