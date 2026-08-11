#include "nexus_v1_vdp1_capture_compositor.h"
#include "nexus_v1_vdp1_dgn_material_resolver.h"
#include "nexus_v1_viewport.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void wl16(uint8_t *p, unsigned int value)
{
    p[0] = (uint8_t)value;
    p[1] = (uint8_t)(value >> 8);
}

typedef struct {
    uint8_t dgn_image[4];
    uint8_t dgn_palette[32];
    int reject;
} SequenceMaterialFixture;

static int resolve_sequence_material(
    const uint8_t *vdp1_vram, int vdp1_vram_size,
    const uint8_t *command, int command_size,
    const Nexus_V1_Vdp1TextureCommand *parsed,
    uint32_t command_byte_offset,
    Nexus_V1_Vdp1CaptureCompositeInput *out_input,
    void *context)
{
    SequenceMaterialFixture *fixture = (SequenceMaterialFixture *)context;
    int i;

    (void)command;
    (void)command_size;
    (void)command_byte_offset;
    if (!fixture || fixture->reject || !vdp1_vram ||
        vdp1_vram_size != (int)NEXUS_V1_VDP1_VRAM_BYTES || !parsed ||
        !out_input || parsed->colour_mode != 1U ||
        parsed->texture_byte_count != sizeof(fixture->dgn_image) ||
        parsed->texture_source_byte_offset != 0x100U) return 0;
    for (i = 0; i < (int)sizeof(fixture->dgn_image); i += 2) {
        fixture->dgn_image[i] = vdp1_vram[0x100 + i + 1];
        fixture->dgn_image[i + 1] = vdp1_vram[0x100 + i];
    }
    for (i = 0; i < (int)sizeof(fixture->dgn_palette); i += 2) {
        fixture->dgn_palette[i] = vdp1_vram[0x200 + i + 1];
        fixture->dgn_palette[i + 1] = vdp1_vram[0x200 + i];
    }
    memset(out_input, 0, sizeof(*out_input));
    out_input->dgn_image = fixture->dgn_image;
    out_input->dgn_image_size = sizeof(fixture->dgn_image);
    out_input->dgn_palette = fixture->dgn_palette;
    out_input->dgn_palette_size = sizeof(fixture->dgn_palette);
    out_input->dgn_source_hash_verified = 1;
    out_input->palette_slot_base = 16;
    return 1;
}

static int run_external_direct_color_capture(void)
{
    const char *path = getenv("FIRESTAFF_NEXUS_RUNTIME_CAPTURE");
    const char *frame_text = getenv("FIRESTAFF_NEXUS_RUNTIME_CAPTURE_FRAME");
    FILE *file;
    long file_size;
    uint8_t *capture;
    Nexus_V1_SaturnRuntimeCaptureFrameReceipt frame;
    Nexus_V1_Vdp1CommandSequenceReceipt sequence;
    Nexus_V1_Vdp1DirectColorFramebuffer framebuffer;
    Nexus_V1_Vdp1DirectColorCaptureReceipt receipt;
    Nexus_Viewport viewport;
    Nexus_V1_SaturnRuntimeCaptureFrameReceipt viewport_frame;
    Nexus_V1_Vdp1CommandSequenceReceipt viewport_sequence;
    Nexus_V1_Vdp1DirectColorCaptureReceipt viewport_receipt;
    unsigned int frame_index;
    int found;

    if (!path || !*path || !frame_text || !*frame_text) return 1;
    frame_index = (unsigned int)strtoul(frame_text, NULL, 0);
    file = fopen(path, "rb");
    if (!file || fseek(file, 0, SEEK_END) != 0 ||
        (file_size = ftell(file)) <= 0 || fseek(file, 0, SEEK_SET) != 0) {
        if (file) fclose(file);
        return 0;
    }
    capture = (uint8_t *)malloc((size_t)file_size);
    if (!capture || fread(capture, 1U, (size_t)file_size, file) !=
            (size_t)file_size) {
        free(capture);
        fclose(file);
        return 0;
    }
    fclose(file);
    memset(&framebuffer, 0, sizeof(framebuffer));
    memset(&receipt, 0, sizeof(receipt));
    found = nexus_v1_vdp1_capture_decode_direct_color_runtime_frame(
        &framebuffer, capture, (size_t)file_size, frame_index, &frame,
        &sequence, &receipt) && receipt.valid && receipt.capture_only &&
        receipt.direct_color_mode && receipt.source_word_order_verified &&
        !receipt.renderer_permitted && receipt.written_pixels > 0 &&
        receipt.command_byte_offset != 0U;
    nexus_viewport_init(&viewport);
    memset(&viewport_frame, 0, sizeof(viewport_frame));
    memset(&viewport_sequence, 0, sizeof(viewport_sequence));
    memset(&viewport_receipt, 0, sizeof(viewport_receipt));
    found = found &&
        nexus_viewport_replay_vdp1_direct_color_capture(
            &viewport, capture, (size_t)file_size, frame_index,
            &viewport_frame, &viewport_sequence, &viewport_receipt) &&
        viewport_receipt.valid && viewport_receipt.capture_only &&
        !viewport_receipt.renderer_permitted &&
        viewport_receipt.written_pixels > 0 &&
        viewport.last_vdp1_direct_color_capture_receipt.valid;
    free(capture);
    return found;
}

static int run_external_dgn_mode1_capture(void)
{
    const char *capture_path = getenv("FIRESTAFF_NEXUS_RUNTIME_CAPTURE");
    const char *dgn_path = getenv("FIRESTAFF_NEXUS_DGN_SOURCE");
    const char *frame_text = getenv("FIRESTAFF_NEXUS_RUNTIME_CAPTURE_FRAME");
    FILE *capture_file;
    FILE *dgn_file;
    long capture_size;
    long dgn_size;
    uint8_t *capture;
    uint8_t *dgn;
    Nexus_V1_Vdp1DgnMaterialResolverInput resolver;
    Nexus_Framebuffer framebuffer;
    Nexus_V1_SaturnRuntimeCaptureFrameReceipt frame;
    Nexus_V1_Vdp1CaptureVramSequenceReceipt replay;
    unsigned int frame_index;
    int ok;

    if (!capture_path || !*capture_path || !dgn_path || !*dgn_path ||
        !frame_text || !*frame_text) return 1;
    frame_index = (unsigned int)strtoul(frame_text, NULL, 0);
    capture_file = fopen(capture_path, "rb");
    dgn_file = fopen(dgn_path, "rb");
    if (!capture_file || !dgn_file || fseek(capture_file, 0, SEEK_END) != 0 ||
        (capture_size = ftell(capture_file)) <= 0 ||
        fseek(capture_file, 0, SEEK_SET) != 0 ||
        fseek(dgn_file, 0, SEEK_END) != 0 || (dgn_size = ftell(dgn_file)) <= 0 ||
        fseek(dgn_file, 0, SEEK_SET) != 0) {
        if (capture_file) fclose(capture_file);
        if (dgn_file) fclose(dgn_file);
        return 0;
    }
    capture = (uint8_t *)malloc((size_t)capture_size);
    dgn = (uint8_t *)malloc((size_t)dgn_size);
    if (!capture || !dgn || fread(capture, 1U, (size_t)capture_size,
                                  capture_file) != (size_t)capture_size ||
        fread(dgn, 1U, (size_t)dgn_size, dgn_file) != (size_t)dgn_size) {
        free(capture);
        free(dgn);
        fclose(capture_file);
        fclose(dgn_file);
        return 0;
    }
    fclose(capture_file);
    fclose(dgn_file);
    memset(&resolver, 0, sizeof(resolver));
    resolver.dgn_bytes = dgn;
    resolver.dgn_byte_count = (int)dgn_size;
    resolver.source_hash_verified = 1;
    nexus_fb_init(&framebuffer);
    nexus_fb_clear(&framebuffer);
    memset(&frame, 0, sizeof(frame));
    memset(&replay, 0, sizeof(replay));
    ok = nexus_v1_vdp1_capture_replay_runtime_frame_mode1_sequence(
        &framebuffer, capture, (size_t)capture_size, frame_index,
        nexus_v1_vdp1_dgn_material_resolver, &resolver, &frame, &replay) &&
        frame.valid && replay.valid && replay.command_sequence.valid &&
        replay.draw_commands_seen == 235 &&
        replay.draw_commands_resolved == 218 &&
        replay.unowned_non_mode1_draw_commands == 1 &&
        replay.unowned_mode1_draw_commands == 16 &&
        replay.skipped_non_draw_commands == 7 &&
        replay.replay.valid && replay.replay.source_joins_verified == 218 &&
        replay.replay.palette_joins_verified == 218 &&
        !replay.system_clip_state_missing &&
        replay.system_clip_state_verified && replay.system_clip_x == 319 &&
        replay.system_clip_y == 223 && replay.replay.renderer_permitted;
    free(capture);
    free(dgn);
    return ok;
}

int main(void)
{
    uint8_t command[NEXUS_V1_VDP1_COMMAND_BYTES] = {0};
    uint8_t texture[32] = {0};
    uint8_t direct_texture[128] = {0};
    uint8_t dgn_image[32];
    uint8_t dgn_direct_image[128];
    uint8_t palette_state[32] = {0};
    uint8_t dgn_palette[32];
    Nexus_Framebuffer framebuffer;
    Nexus_Viewport viewport;
    Nexus_V1_Vdp1CaptureCompositeInput input;
    Nexus_V1_Vdp1CaptureCompositeReceipt receipt;
    Nexus_V1_Vdp1DirectColorFramebuffer direct_framebuffer;
    Nexus_V1_Vdp1DirectColorCaptureReceipt direct_receipt;
    Nexus_V1_Vdp1CaptureSequenceInput sequence_input;
    Nexus_V1_Vdp1CaptureSequenceReceipt sequence_receipt;
    Nexus_V1_Vdp1CaptureVramSequenceInput vram_sequence_input;
    Nexus_V1_Vdp1CaptureVramSequenceReceipt vram_sequence_receipt;
    SequenceMaterialFixture material_fixture;
    Nexus_Framebuffer before_rejected_vram_replay;
    uint8_t *vdp1_vram;
    Nexus_Framebuffer before_failed_replay;
    int i;

    /* 16x4 mode-1 quad, signed coordinates around the Saturn display
     * centre. Capture lanes are little-endian word images; DGN source lanes
     * are their canonical big-endian word order. */
    wl16(command + 0, 0x1002U);
    wl16(command + 4, 1U << 3);
    wl16(command + 6, 0x3278U);
    wl16(command + 8, 0x0040U);
    wl16(command + 10, 0x0402U);
    wl16(command + 12, 0xfff0U);
    wl16(command + 14, 0xfff0U);
    wl16(command + 16, 0x0010U);
    wl16(command + 18, 0xfff0U);
    wl16(command + 20, 0x0010U);
    wl16(command + 22, 0x0010U);
    wl16(command + 24, 0xfff0U);
    wl16(command + 26, 0x0010U);
    for (i = 0; i < (int)sizeof(texture); ++i) texture[i] = 0x12U;
    for (i = 0; i < (int)sizeof(texture); i += 2) {
        dgn_image[i] = texture[i + 1];
        dgn_image[i + 1] = texture[i];
    }
    for (i = 0; i < 16; ++i) {
        wl16(palette_state + i * 2, (unsigned int)(0x8000U + i));
        dgn_palette[i * 2] = palette_state[i * 2 + 1];
        dgn_palette[i * 2 + 1] = palette_state[i * 2];
    }
    memset(&input, 0, sizeof(input));
    input.command = command;
    input.command_size = sizeof(command);
    input.texture_span = texture;
    input.texture_span_size = sizeof(texture);
    input.palette_state = palette_state;
    input.palette_state_size = sizeof(palette_state);
    input.dgn_image = dgn_image;
    input.dgn_image_size = sizeof(dgn_image);
    input.dgn_palette = dgn_palette;
    input.dgn_palette_size = sizeof(dgn_palette);
    input.dgn_source_hash_verified = 1;
    input.original_saturn_capture_verified = 1;
    input.screen_origin_x = 160;
    input.screen_origin_y = 112;
    input.palette_slot_base = 32;

    nexus_fb_init(&framebuffer);
    nexus_fb_clear(&framebuffer);
    memset(&receipt, 0, sizeof(receipt));
    if (!nexus_v1_vdp1_capture_composite_mode1(
            &framebuffer, &input, &receipt) || !receipt.valid ||
        !receipt.source_join_verified || !receipt.palette_join_verified ||
        !receipt.renderer_permitted || receipt.written_pixels <= 0 ||
        framebuffer.color_buffer[112 * NEXUS_FB_W + 160] != 33U) {
        fprintf(stderr, "FAIL: authenticated VDP1 capture composite\n");
        return 1;
    }
    /* Mode 5 is a direct 16-bit RGB stream, not a CLUT texture.  Its
     * capture decoder must preserve the word semantics but remain capture-
     * only until a DGN owner/material receipt exists. */
    for (i = 0; i < (int)sizeof(direct_texture); i += 2)
        wl16(direct_texture + i, (i < 32) ? 0x4000U : 0x001fU);
    for (i = 0; i < (int)sizeof(direct_texture); i += 2) {
        dgn_direct_image[i] = direct_texture[i + 1];
        dgn_direct_image[i + 1] = direct_texture[i];
    }
    wl16(command + 4, 5U << 3);
    input.texture_span = direct_texture;
    input.texture_span_size = sizeof(direct_texture);
    input.dgn_direct_image = dgn_direct_image;
    input.dgn_direct_image_size = sizeof(dgn_direct_image);
    input.dgn_direct_source_hash_verified = 1;
    memset(&direct_framebuffer, 0, sizeof(direct_framebuffer));
    memset(&direct_receipt, 0, sizeof(direct_receipt));
    if (!nexus_v1_vdp1_capture_decode_direct_color(
            &direct_framebuffer, &input, &direct_receipt) ||
        !direct_receipt.valid || !direct_receipt.capture_only ||
        !direct_receipt.direct_color_mode ||
        !direct_receipt.source_word_order_verified ||
        !direct_receipt.source_join_verified ||
        direct_receipt.renderer_permitted ||
        direct_receipt.written_pixels <= 0 ||
        direct_receipt.transparent_pixels <= 0) {
        fprintf(stderr, "FAIL: authenticated direct-color capture decode\n");
        return 1;
    }
    input.original_saturn_capture_verified = 0;
    if (nexus_v1_vdp1_capture_decode_direct_color(
            &direct_framebuffer, &input, &direct_receipt) ||
        direct_receipt.renderer_permitted) {
        fprintf(stderr, "FAIL: unauthenticated direct-color capture admitted\n");
        return 1;
    }
    input.original_saturn_capture_verified = 1;
    /* Restore the indexed mode-1 fixture for the remaining compositor tests. */
    wl16(command + 4, 1U << 3);
    input.texture_span = texture;
    input.texture_span_size = sizeof(texture);
    nexus_viewport_init(&viewport);
    memset(&receipt, 0, sizeof(receipt));
    if (!nexus_viewport_replay_vdp1_capture(&viewport, &input, &receipt) ||
        !receipt.valid || receipt.written_pixels <= 0 ||
        viewport.last_vdp1_capture_receipt.written_pixels !=
            receipt.written_pixels) {
        fprintf(stderr, "FAIL: viewport VDP1 capture replay adapter\n");
        return 1;
    }
    /* ECD is active in this command: an F texel ends its source row. */
    texture[0] = 0x1fU;
    for (i = 0; i < (int)sizeof(texture); i += 2) {
        dgn_image[i] = texture[i + 1];
        dgn_image[i + 1] = texture[i];
    }
    memset(&receipt, 0, sizeof(receipt));
    if (!nexus_v1_vdp1_capture_composite_mode1(
            &framebuffer, &input, &receipt) || receipt.end_code_pixels <= 0 ||
        receipt.written_pixels <= 0) {
        fprintf(stderr, "FAIL: VDP1 end-code row termination\n");
        return 1;
    }
    input.original_saturn_capture_verified = 0;
    if (nexus_v1_vdp1_capture_composite_mode1(
            &framebuffer, &input, &receipt) || receipt.renderer_permitted) {
        fprintf(stderr, "FAIL: unauthenticated VDP1 composite was admitted\n");
        return 1;
    }
    input.original_saturn_capture_verified = 1;
    /* A valid but fully off-screen command must not publish its palette or
     * leave partial pixels behind when the bounded replay returns failure. */
    before_failed_replay = framebuffer;
    wl16(command + 12, 0x0100U);
    wl16(command + 14, 0x0100U);
    wl16(command + 16, 0x0110U);
    wl16(command + 18, 0x0100U);
    wl16(command + 20, 0x0110U);
    wl16(command + 22, 0x0110U);
    wl16(command + 24, 0x0100U);
    wl16(command + 26, 0x0110U);
    if (nexus_v1_vdp1_capture_composite_mode1(
            &framebuffer, &input, &receipt) ||
        memcmp(&before_failed_replay, &framebuffer,
               sizeof(framebuffer)) != 0) {
        fprintf(stderr, "FAIL: failed VDP1 replay mutated framebuffer\n");
        return 1;
    }
    /* Restore the in-frame command before exercising sequence composition. */
    wl16(command + 12, 0xfff0U);
    wl16(command + 14, 0xfff0U);
    wl16(command + 16, 0x0010U);
    wl16(command + 18, 0xfff0U);
    wl16(command + 20, 0x0010U);
    wl16(command + 22, 0x0010U);
    wl16(command + 24, 0xfff0U);
    wl16(command + 26, 0x0010U);
    memset(&sequence_input, 0, sizeof(sequence_input));
    sequence_input.commands = &input;
    sequence_input.command_count = 1;
    sequence_input.system_clip_state_verified = 1;
    sequence_input.local_coordinate_state_verified = 1;
    sequence_input.display_origin_state_verified = 1;
    sequence_input.display_origin_x = 160;
    sequence_input.display_origin_y = 112;
    sequence_input.command_order_verified = 1;
    sequence_input.end_record_verified = 1;
    memset(&sequence_receipt, 0, sizeof(sequence_receipt));
    if (!nexus_v1_vdp1_capture_composite_mode1_sequence(
            &framebuffer, &sequence_input, &sequence_receipt) ||
        !sequence_receipt.valid || !sequence_receipt.sequence_state_verified ||
        sequence_receipt.command_frames_verified != 1) {
        fprintf(stderr, "FAIL: authenticated VDP1 sequence composite\n");
        return 1;
    }
    nexus_viewport_init(&viewport);
    if (!nexus_viewport_replay_vdp1_capture_sequence(
            &viewport, &sequence_input, &sequence_receipt) ||
        viewport.last_vdp1_sequence_receipt.command_count != 1) {
        fprintf(stderr, "FAIL: viewport VDP1 sequence replay adapter\n");
        return 1;
    }
    sequence_input.display_origin_x = 159;
    if (nexus_v1_vdp1_capture_composite_mode1_sequence(
            &framebuffer, &sequence_input, &sequence_receipt)) {
        fprintf(stderr, "FAIL: VDP1 sequence accepted mismatched origin\n");
        return 1;
    }
    sequence_input.display_origin_x = 160;
    sequence_input.end_record_verified = 0;
    if (nexus_v1_vdp1_capture_composite_mode1_sequence(
            &framebuffer, &sequence_input, &sequence_receipt)) {
        fprintf(stderr, "FAIL: incomplete VDP1 sequence admitted\n");
        return 1;
    }

    /* The production-facing adapter builds the replay inputs from one
     * authenticated VRAM/CMDLINK chain. The resolver is deliberately the
     * only owner of DGN source bytes; an unresolved draw must not mutate the
     * framebuffer. */
    vdp1_vram = (uint8_t *)calloc(1U, NEXUS_V1_VDP1_VRAM_BYTES);
    if (!vdp1_vram) return 1;
    wl16(vdp1_vram + 0, 0x0009U);       /* system clip */
    wl16(vdp1_vram + 32, 0x000aU);      /* local coordinate */
    wl16(vdp1_vram + 44, 160U);
    wl16(vdp1_vram + 46, 112U);
    wl16(vdp1_vram + 64, 0x0002U);      /* mode-1 draw */
    wl16(vdp1_vram + 68, 1U << 3);
    wl16(vdp1_vram + 70, 0x0040U);      /* CLUT at 0x200 (byte offset) */
    wl16(vdp1_vram + 72, 0x0020U);      /* texture at 0x100 */
    wl16(vdp1_vram + 74, 0x0101U);      /* 8x1 */
    wl16(vdp1_vram + 76, 0xfffcU);
    wl16(vdp1_vram + 78, 0xfffcU);
    wl16(vdp1_vram + 80, 0x0004U);
    wl16(vdp1_vram + 82, 0xfffcU);
    wl16(vdp1_vram + 84, 0x0004U);
    wl16(vdp1_vram + 86, 0x0004U);
    wl16(vdp1_vram + 88, 0xfffcU);
    wl16(vdp1_vram + 90, 0x0004U);
    wl16(vdp1_vram + 96, 0x8000U);      /* END */
    memset(vdp1_vram + 0x100, 0x11, 4);
    for (i = 0; i < 16; ++i) wl16(vdp1_vram + 0x200 + i * 2,
                                   0x8000U + (unsigned)i);
    memset(&material_fixture, 0, sizeof(material_fixture));
    memset(&vram_sequence_input, 0, sizeof(vram_sequence_input));
    vram_sequence_input.vdp1_vram = vdp1_vram;
    vram_sequence_input.vdp1_vram_size = NEXUS_V1_VDP1_VRAM_BYTES;
    vram_sequence_input.copr_word = 12U;
    vram_sequence_input.original_saturn_capture_verified = 1;
    vram_sequence_input.resolve_material = resolve_sequence_material;
    vram_sequence_input.resolver_context = &material_fixture;
    nexus_fb_init(&framebuffer);
    nexus_fb_clear(&framebuffer);
    memset(&vram_sequence_receipt, 0, sizeof(vram_sequence_receipt));
    if (!nexus_v1_vdp1_capture_replay_vram_sequence(
            &framebuffer, &vram_sequence_input, &vram_sequence_receipt) ||
        !vram_sequence_receipt.valid ||
        vram_sequence_receipt.draw_commands_seen != 1 ||
        vram_sequence_receipt.draw_commands_resolved != 1 ||
        vram_sequence_receipt.replay.valid <= 0) {
        free(vdp1_vram);
        fprintf(stderr, "FAIL: VDP1 VRAM sequence adapter\n");
        return 1;
    }
    before_rejected_vram_replay = framebuffer;
    material_fixture.reject = 1;
    if (nexus_v1_vdp1_capture_replay_vram_sequence(
            &framebuffer, &vram_sequence_input, &vram_sequence_receipt) ||
        memcmp(&before_rejected_vram_replay, &framebuffer,
               sizeof(framebuffer)) != 0 ||
        vram_sequence_receipt.unresolved_draw_commands != 1) {
        free(vdp1_vram);
        fprintf(stderr, "FAIL: unresolved VDP1 draw was admitted\n");
        return 1;
    }
    free(vdp1_vram);
    if (getenv("FIRESTAFF_NEXUS_RUNTIME_CAPTURE") &&
        getenv("FIRESTAFF_NEXUS_RUNTIME_CAPTURE_FRAME") &&
        !getenv("FIRESTAFF_NEXUS_DGN_SOURCE") &&
        !run_external_direct_color_capture()) {
        fprintf(stderr, "FAIL: external direct-color capture decode\n");
        return 1;
    }
    if (getenv("FIRESTAFF_NEXUS_DGN_SOURCE") &&
        !run_external_dgn_mode1_capture()) {
        fprintf(stderr, "FAIL: external DGN mode-1 capture replay\n");
        return 1;
    }
    puts("test_nexus_v1_vdp1_capture_compositor: PASS");
    return 0;
}
