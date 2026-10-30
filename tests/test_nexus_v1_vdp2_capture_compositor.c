#include "nexus_v1_vdp2_capture_compositor.h"
#include "nexus_v1_vdp1_capture_compositor.h"
#include "nexus_v1_title_res_corpus_receipt.h"
#include "firestaff_x68k_media_receipt.h"
#include "nexus_v1_viewport.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void wb16(uint8_t *p, unsigned int value)
{
    p[0] = (uint8_t)(value >> 8U);
    p[1] = (uint8_t)value;
}

static void wl16(uint8_t *p, unsigned int value)
{
    p[0] = (uint8_t)value;
    p[1] = (uint8_t)(value >> 8U);
}

static int run_external_bitmap_capture(void)
{
    const char *path = getenv("FIRESTAFF_NEXUS_RUNTIME_CAPTURE");
    const char *frame_text = getenv("FIRESTAFF_NEXUS_RUNTIME_CAPTURE_FRAME");
    FILE *file;
    long file_size;
    uint8_t *capture;
    Nexus_V1_Vdp2BitmapCaptureFramebuffer framebuffer;
    Nexus_V1_SaturnRuntimeCaptureFrameReceipt frame;
    Nexus_V1_SaturnVdp2RegisterReceipt registers;
    Nexus_V1_Vdp2BitmapCaptureReceipt receipt;
    unsigned int frame_index;
    int ok;

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
    memset(&frame, 0, sizeof(frame));
    memset(&registers, 0, sizeof(registers));
    memset(&receipt, 0, sizeof(receipt));
    ok = nexus_v1_vdp2_capture_decode_runtime_frame_nbg1_bitmap(
        &framebuffer, capture, (size_t)file_size, frame_index, &frame,
        &registers, &receipt) && receipt.valid && receipt.capture_only &&
        receipt.nbg1_bitmap_mode && receipt.colour_code_256 &&
        receipt.bitmap_vram_offset == 0U && receipt.transparent_pixels > 0 &&
        !receipt.renderer_permitted;
    free(capture);
    return ok;
}

static uint8_t *read_exact_file(const char *path, size_t expected_size)
{
    FILE *file;
    uint8_t *bytes;
    long size;
    if (!path || !*path) return NULL;
    file = fopen(path, "rb");
    if (!file || fseek(file, 0, SEEK_END) != 0 ||
        (size = ftell(file)) < 0 || (size_t)size != expected_size ||
        fseek(file, 0, SEEK_SET) != 0) {
        if (file) fclose(file);
        return NULL;
    }
    bytes = (uint8_t *)malloc(expected_size);
    if (!bytes || fread(bytes, 1U, expected_size, file) != expected_size) {
        free(bytes);
        fclose(file);
        return NULL;
    }
    fclose(file);
    return bytes;
}

static int has_sha256(const uint8_t *bytes, size_t size,
                      const char *expected_hex)
{
    char actual_hex[65];
    return bytes && expected_hex &&
        firestaff_x68k_media_receipt_sha256_hex(
            bytes, size, actual_hex, sizeof(actual_hex)) == 0 &&
        strcmp(actual_hex, expected_hex) == 0;
}

static int run_external_ymir_menu_capture(void)
{
    const char *fbs_path = getenv("FIRESTAFF_NEXUS_YMIR_FBS");
    const char *vram_path = getenv("FIRESTAFF_NEXUS_YMIR_VDP2_VRAM");
    const char *cram_path = getenv("FIRESTAFF_NEXUS_YMIR_CRAM");
    const char *registers_path = getenv("FIRESTAFF_NEXUS_YMIR_REGS");
    const char *output_path = getenv("FIRESTAFF_NEXUS_YMIR_REPLAY_OUT");
    const char *capture_attested = getenv("FIRESTAFF_NEXUS_YMIR_CAPTURE_ATTESTED");
    const char *trace_attested = getenv("FIRESTAFF_NEXUS_YMIR_TRACE_ATTESTED");
    const char *vdp1_vram_path = getenv("FIRESTAFF_NEXUS_YMIR_VDP1_VRAM");
    const char *vdp1_reconstructed_path =
        getenv("FIRESTAFF_NEXUS_YMIR_VDP1_RECONSTRUCTED_VRAM");
    const char *title_path = getenv("FIRESTAFF_NEXUS_YMIR_COMMAND171_TITLE");
    const char *expected_rgb_path =
        getenv("FIRESTAFF_NEXUS_YMIR_COMMAND171_EXPECTED_RGB");
    uint8_t *fbs = NULL;
    uint8_t *vram = NULL;
    uint8_t *vdp1_vram = NULL;
    uint8_t *vdp1_reconstructed_vram = NULL;
    uint8_t *title = NULL;
    uint8_t *expected_rgb = NULL;
    uint8_t *cram = NULL;
    uint8_t *registers = NULL;
    Nexus_V1_Vdp2YmirMenuCaptureFramebuffer *framebuffer = NULL;
    Nexus_V1_Vdp2YmirMenuCaptureInput input;
    Nexus_V1_Vdp2YmirMenuCaptureReceipt receipt;
    FILE *output = NULL;
    size_t i;
    int ok = 0;
    int overlay_requested = (vdp1_vram_path && *vdp1_vram_path) ||
        (vdp1_reconstructed_path && *vdp1_reconstructed_path) ||
        (title_path && *title_path) ||
        (expected_rgb_path && *expected_rgb_path);

    if (!fbs_path && !vram_path && !cram_path && !registers_path &&
        !output_path && !overlay_requested) return 1;
    fbs = read_exact_file(fbs_path,
                          NEXUS_V1_VDP1_YMIR_FRAMEBUFFER_BYTES * 2U);
    vram = read_exact_file(vram_path, NEXUS_V1_SATURN_VDP2_VRAM_BYTES);
    cram = read_exact_file(cram_path, NEXUS_V1_SATURN_VDP2_CRAM_BYTES);
    registers = read_exact_file(registers_path, 512U);
    if (overlay_requested) {
        vdp1_vram = read_exact_file(vdp1_vram_path,
                                    NEXUS_V1_SATURN_VDP1_VRAM_BYTES);
        vdp1_reconstructed_vram = read_exact_file(vdp1_reconstructed_path,
                                    NEXUS_V1_SATURN_VDP1_VRAM_BYTES);
        title = read_exact_file(title_path, 112216U);
        expected_rgb = read_exact_file(expected_rgb_path,
            NEXUS_V1_VDP2_YMIR_MENU_WIDTH *
            NEXUS_V1_VDP2_YMIR_MENU_HEIGHT * 3U);
    }
    framebuffer = (Nexus_V1_Vdp2YmirMenuCaptureFramebuffer *)
        malloc(sizeof(*framebuffer));
    if (!fbs || !vram || !cram || !registers || !framebuffer ||
        (overlay_requested && (!vdp1_vram || !vdp1_reconstructed_vram ||
                               !title || !expected_rgb)) ||
        !output_path || !*output_path) goto cleanup;
    if (!has_sha256(fbs, NEXUS_V1_VDP1_YMIR_FRAMEBUFFER_BYTES * 2U,
            "8852b60717358bb10416cec2ebf8d8c6c049025fb78f1d621e938e3305b51c25") ||
        !has_sha256(vram, NEXUS_V1_SATURN_VDP2_VRAM_BYTES,
            "de1a77427dc62457eb4b59fa0424fd25e9768ce229505445311929d3bdae405c") ||
        !has_sha256(cram, NEXUS_V1_SATURN_VDP2_CRAM_BYTES,
            "db282593790dec4a2bf8f76293c0458a1a140adc3351650bd56a417f0359e0de") ||
        !has_sha256(registers, 512U,
            "e69d03094ef856922413e073cb9078015e52849786268d6421c7df00a655965e"))
        goto cleanup;
    if (overlay_requested &&
        (!has_sha256(vdp1_vram, NEXUS_V1_SATURN_VDP1_VRAM_BYTES,
            "9974b1716c4ee91aa1d44d3ed2705cc09ac27b17c642e68bdaf96293fd36c4e7") ||
         !has_sha256(vdp1_reconstructed_vram,
            NEXUS_V1_SATURN_VDP1_VRAM_BYTES,
            "90b85d0358c7605c8186828d2e42ab46412f504f12c40348197a46db8f770913") ||
         !has_sha256(expected_rgb,
            NEXUS_V1_VDP2_YMIR_MENU_WIDTH *
            NEXUS_V1_VDP2_YMIR_MENU_HEIGHT * 3U,
            "05e8aaaa5cf7459a9ff93589c45ddc681be837748c0a344461f246cd61b18875")))
        goto cleanup;
    memset(&input, 0, sizeof(input));
    /* Ymir writes draw then display framebuffer; consume the latter. */
    input.vdp1_display_framebuffer = fbs +
        NEXUS_V1_VDP1_YMIR_FRAMEBUFFER_BYTES;
    input.vdp1_display_framebuffer_size = NEXUS_V1_VDP1_YMIR_FRAMEBUFFER_BYTES;
    input.vdp2_vram = vram;
    input.vdp2_vram_size = NEXUS_V1_SATURN_VDP2_VRAM_BYTES;
    input.vdp2_cram = cram;
    input.vdp2_cram_size = NEXUS_V1_SATURN_VDP2_CRAM_BYTES;
    input.vdp2_registers = registers;
    input.vdp2_registers_size = 512U;
    input.register_byte_order = NEXUS_V1_SATURN_VDP2_REGISTER_ORDER_BIG;
    if (overlay_requested) {
        Nexus_V1_Vdp1TextureCommand command;
        char title_sha256[65];
        const size_t command_offset = 0x1560U;
        const size_t title_offset = 0x1f26U;
        size_t i;
        /* Normalize only the traced command record for Firestaff's parser.
         * Keep texture bytes in the captured bus order: swapping the whole
         * VRAM snapshot also swaps neighboring 4bpp texel bytes. */
        memcpy(vdp1_vram + command_offset,
               vdp1_reconstructed_vram + command_offset,
               NEXUS_V1_VDP1_COMMAND_BYTES);
        if (nexus_v1_vdp1_texture_command_parse(
                vdp1_vram + command_offset,
                (int)NEXUS_V1_VDP1_COMMAND_BYTES, &command) != 0 ||
            !command.texture_command || command.command_type != 0U ||
            command.colour_mode != 0U ||
            command.texture_source_byte_offset != 0x415a0U ||
            command.texture_byte_count != 1008U ||
            title_offset + command.texture_byte_count > 112216U) {
            goto cleanup;
        }
        if (firestaff_x68k_media_receipt_sha256_hex(
                title, 112216U, title_sha256, sizeof(title_sha256)) != 0 ||
            (strcmp(title_sha256, NEXUS_V1_TITLE_BIN_SHA256) != 0 &&
             strcmp(title_sha256, NEXUS_V1_TITLE_BIN_ENGLISH_SHA256) != 0 &&
             strcmp(title_sha256, NEXUS_V1_TITLE_BIN_FRENCH_SHA256) != 0)) {
            goto cleanup;
        }
        for (i = 0U; i < command.texture_byte_count; i += 2U) {
            if (vdp1_vram[command.texture_source_byte_offset + i] !=
                    title[title_offset + i] ||
                vdp1_vram[command.texture_source_byte_offset + i + 1U] !=
                    title[title_offset + i + 1U]) goto cleanup;
        }
        input.vdp1_vram = vdp1_vram;
        input.vdp1_vram_size = NEXUS_V1_SATURN_VDP1_VRAM_BYTES;
        input.vdp1_command_byte_offset = (uint32_t)command_offset;
        input.vdp1_command_source_join_verified = 1;
    }
    input.original_ymir_capture_verified = capture_attested &&
        strcmp(capture_attested, "1") == 0;
    input.command_trace_complete_verified = trace_attested &&
        strcmp(trace_attested, "1") == 0;
    if (!nexus_v1_vdp2_capture_replay_ymir_title_menu(
            framebuffer, &input, &receipt) || !receipt.valid ||
        !receipt.capture_only || receipt.renderer_permitted ||
        !receipt.registers_verified || !receipt.command_trace_complete_verified ||
        !receipt.original_ymir_capture_verified ||
        receipt.width != (int)NEXUS_V1_VDP2_YMIR_MENU_WIDTH ||
        receipt.height != (int)NEXUS_V1_VDP2_YMIR_MENU_HEIGHT ||
        receipt.vdp1_direct_pixels + receipt.vdp1_palette_pixels !=
            (int)(NEXUS_V1_VDP2_YMIR_MENU_WIDTH *
                  NEXUS_V1_VDP2_YMIR_MENU_HEIGHT) ||
        receipt.nbg0_overlay_pixels <= 0) {
        fprintf(stderr, "Ymir menu receipt valid=%d registers=%d direct=%d "
                "palette=%d nbg0=%d capture=%d trace=%d\n",
                receipt.valid, receipt.registers_verified,
                receipt.vdp1_direct_pixels, receipt.vdp1_palette_pixels,
                receipt.nbg0_overlay_pixels,
                receipt.original_ymir_capture_verified,
                receipt.command_trace_complete_verified);
        goto cleanup;
    }
    if (overlay_requested &&
        (!receipt.vdp1_command_overlaid ||
         !receipt.vdp1_command_source_join_verified ||
         receipt.vdp1_command_pixels != 963)) goto cleanup;
    if (overlay_requested) {
        size_t pixel;
        for (pixel = 0U; pixel < NEXUS_V1_VDP2_YMIR_MENU_WIDTH *
                                NEXUS_V1_VDP2_YMIR_MENU_HEIGHT; ++pixel) {
            uint32_t composed = framebuffer->rgba_buffer[pixel];
            const uint8_t channels[3] = {
                (uint8_t)(composed >> 16U),
                (uint8_t)(composed >> 8U),
                (uint8_t)composed
            };
            unsigned int channel;
            for (channel = 0U; channel < 3U; ++channel) {
                int difference = (int)channels[channel] -
                    (int)expected_rgb[pixel * 3U + channel];
                if (difference < -8 || difference > 8) goto cleanup;
            }
        }
    }
    input.command_trace_complete_verified = 0;
    if (nexus_v1_vdp2_capture_replay_ymir_title_menu(
            framebuffer, &input, &receipt) || receipt.valid ||
        receipt.renderer_permitted) goto cleanup;
    input.command_trace_complete_verified = 1;
    registers[0x2cU] = 0x01U;
    if (nexus_v1_vdp2_capture_replay_ymir_title_menu(
            framebuffer, &input, &receipt) || receipt.valid ||
        receipt.renderer_permitted) goto cleanup;
    registers[0x2cU] = 0U;
    if (!nexus_v1_vdp2_capture_replay_ymir_title_menu(
            framebuffer, &input, &receipt) || !receipt.valid ||
        receipt.renderer_permitted) goto cleanup;
    output = fopen(output_path, "wb");
    if (!output) goto cleanup;
    for (i = 0; i < NEXUS_V1_VDP2_YMIR_MENU_WIDTH *
                    NEXUS_V1_VDP2_YMIR_MENU_HEIGHT; ++i) {
        uint32_t pixel = framebuffer->rgba_buffer[i];
        uint8_t rgb[3];
        rgb[0] = (uint8_t)(pixel >> 16U);
        rgb[1] = (uint8_t)(pixel >> 8U);
        rgb[2] = (uint8_t)pixel;
        if (fwrite(rgb, 1U, sizeof(rgb), output) != sizeof(rgb)) goto cleanup;
    }
    if (fclose(output) != 0) {
        output = NULL;
        goto cleanup;
    }
    output = NULL;
    ok = 1;
cleanup:
    if (output) fclose(output);
    free(framebuffer);
    free(title);
    free(expected_rgb);
    free(vdp1_reconstructed_vram);
    free(vdp1_vram);
    free(registers);
    free(cram);
    free(vram);
    free(fbs);
    return ok;
}

int main(void)
{
    uint8_t *bitmap = (uint8_t *)calloc(1, NEXUS_V1_VDP2_NBG1_BITMAP_BYTES);
    uint8_t *cram = (uint8_t *)calloc(1, 4096);
    uint8_t registers[NEXUS_V1_VDP2_REGISTERS_BYTES] = {0};
    Nexus_V1_Vdp2CaptureCompositeInput input;
    Nexus_V1_Vdp2CaptureCompositeReceipt receipt;
    Nexus_Viewport viewport;
    int i;

    if (!bitmap || !cram) return 1;
    wb16(registers + 0x20, 0x0002);
    wb16(registers + 0x28, 0x1211);
    wb16(registers + 0x2c, 0x0000);
    for (i = 0; i < 16; ++i) {
        bitmap[20 * 512 + 30 + i] = (uint8_t)(i + 1);
        wb16(cram + i * 2, 0x8000U | (unsigned int)i);
    }
    memset(&input, 0, sizeof(input));
    input.capture_bitmap = bitmap;
    input.capture_bitmap_size = NEXUS_V1_VDP2_NBG1_BITMAP_BYTES;
    input.capture_cram = cram;
    input.capture_cram_size = 4096;
    input.vdp2_registers = registers;
    input.vdp2_registers_size = sizeof(registers);
    input.register_byte_order = NEXUS_V1_SATURN_VDP2_REGISTER_ORDER_BIG;
    input.source_bitmap = bitmap;
    input.source_bitmap_size = NEXUS_V1_VDP2_NBG1_BITMAP_BYTES;
    input.source_palette = cram;
    input.source_palette_size = NEXUS_V1_VDP2_NBG1_PALETTE_BYTES;
    input.source_hash_verified = 1;
    input.original_saturn_capture_verified = 1;
    input.transparent_index_zero_verified = 1;
    input.source_x = 30;
    input.source_y = 20;
    input.destination_x = 40;
    input.destination_y = 50;
    input.width = 16;
    input.height = 1;
    input.palette_base = 0;
    nexus_viewport_init(&viewport);
    if (!nexus_viewport_replay_vdp2_nbg1_capture(&viewport, &input,
                                                 &receipt) ||
        !receipt.valid || receipt.written_pixels != 16 ||
        viewport.fb.color_buffer[50 * NEXUS_FB_W + 40] != 1U ||
        viewport.fb.palette[1] != UINT32_C(0xff080000)) {
        fprintf(stderr, "FAIL: authenticated VDP2 NBG1 replay\n");
        free(bitmap);
        free(cram);
        return 1;
    }
    input.original_saturn_capture_verified = 0;
    if (nexus_v1_vdp2_capture_composite_nbg1_bitmap(
            &viewport.fb, &input, &receipt) || receipt.renderer_permitted) {
        fprintf(stderr, "FAIL: unauthenticated VDP2 replay was admitted\n");
        free(bitmap);
        free(cram);
        return 1;
    }
    /* The same source-bound bitmap lane must accept the native
     * little-endian register serialization used by later frames. */
    memset(registers, 0, sizeof(registers));
    wl16(registers + 0x00, 0x8020);
    wl16(registers + 0x20, 0x0002);
    wl16(registers + 0x28, 0x1211);
    wl16(registers + 0x2c, 0x0000);
    input.original_saturn_capture_verified = 1;
    input.register_byte_order = NEXUS_V1_SATURN_VDP2_REGISTER_ORDER_LITTLE;
    if (!nexus_v1_vdp2_capture_composite_nbg1_bitmap(
            &viewport.fb, &input, &receipt) || !receipt.valid) {
        fprintf(stderr, "FAIL: little-endian VDP2 NBG1 replay\n");
        free(bitmap);
        free(cram);
        return 1;
    }
    /* UNKNOWN keeps the historical register scorer and must retain the
     * historical big-endian CRAM decoding too. */
    input.register_byte_order = NEXUS_V1_SATURN_VDP2_REGISTER_ORDER_UNKNOWN;
    memset(registers, 0, sizeof(registers));
    wb16(registers + 0x20, 0x0002);
    wb16(registers + 0x28, 0x1211);
    wb16(registers + 0x2c, 0x0000);
    if (!nexus_v1_vdp2_capture_composite_nbg1_bitmap(
            &viewport.fb, &input, &receipt) || !receipt.valid ||
        viewport.fb.palette[1] != UINT32_C(0xff080000)) {
        fprintf(stderr, "FAIL: legacy VDP2 palette order changed\n");
        free(bitmap);
        free(cram);
        return 1;
    }
    memset(registers, 0, sizeof(registers));
    wb16(registers + 0x20, 0x0002);
    wb16(registers + 0x28, 0x1211);
    wb16(registers + 0x2c, 0x0000);
    wb16(registers + 0x2c, 0x0000);
    wb16(registers + NEXUS_V1_VDP2_CRAOFA_OFFSET, 0x0010);
    input.register_byte_order = NEXUS_V1_SATURN_VDP2_REGISTER_ORDER_BIG;
    if (nexus_v1_vdp2_capture_composite_nbg1_bitmap(
            &viewport.fb, &input, &receipt) || receipt.renderer_permitted) {
        fprintf(stderr, "FAIL: unbound NBG1 CRAM offset was admitted\n");
        free(bitmap);
        free(cram);
        return 1;
    }
    input.original_saturn_capture_verified = 1;
    wb16(registers + 0x2c, 0x0100);
    if (nexus_v1_vdp2_capture_composite_nbg1_bitmap(
            &viewport.fb, &input, &receipt) || receipt.renderer_permitted) {
        fprintf(stderr, "FAIL: NBG0 palette bits were treated as NBG1\n");
        free(bitmap);
        free(cram);
        return 1;
    }
    free(bitmap);
    free(cram);
    if (getenv("FIRESTAFF_NEXUS_RUNTIME_CAPTURE") &&
        getenv("FIRESTAFF_NEXUS_RUNTIME_CAPTURE_FRAME") &&
        !run_external_bitmap_capture()) {
        fprintf(stderr, "FAIL: external VDP2 NBG1 bitmap capture decode\n");
        return 1;
    }
    if ((getenv("FIRESTAFF_NEXUS_YMIR_FBS") ||
         getenv("FIRESTAFF_NEXUS_YMIR_VDP2_VRAM") ||
         getenv("FIRESTAFF_NEXUS_YMIR_CRAM") ||
         getenv("FIRESTAFF_NEXUS_YMIR_REGS") ||
         getenv("FIRESTAFF_NEXUS_YMIR_REPLAY_OUT")) &&
        !run_external_ymir_menu_capture()) {
        fprintf(stderr, "FAIL: external full-frame Ymir title-menu replay\n");
        return 1;
    }
    puts("test_nexus_v1_vdp2_capture_compositor: PASS");
    return 0;
}
