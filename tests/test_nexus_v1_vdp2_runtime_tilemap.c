#include "nexus_v1_vdp2_tilemap_capture_compositor.h"
#include "firestaff_x68k_media_receipt.h"

#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void le16(uint8_t *p, unsigned int value)
{
    p[0] = (uint8_t)value;
    p[1] = (uint8_t)(value >> 8U);
}

static void be16(uint8_t *p, unsigned int value)
{
    p[0] = (uint8_t)(value >> 8U);
    p[1] = (uint8_t)value;
}

static uint8_t *read_exact_bytes(const char *path, size_t expected_size)
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

static int bytes_have_sha256(const uint8_t *bytes, size_t size,
                             const char *expected)
{
    char actual[65];
    return bytes && expected &&
        firestaff_x68k_media_receipt_sha256_hex(
            bytes, size, actual, sizeof(actual)) == 0 &&
        strcmp(actual, expected) == 0;
}

static int capture_plane_have_sha256(
    const Nexus_V1_Vdp2CapturePlane *plane, const char *expected)
{
    const size_t pixels = NEXUS_V1_VDP2_NBG2_CAPTURE_WIDTH *
        NEXUS_V1_VDP2_NBG2_CAPTURE_HEIGHT;
    uint8_t *serialized;
    size_t pixel;
    int matches;
    if (!plane || !expected) return 0;
    serialized = (uint8_t *)malloc(pixels * 4U);
    if (!serialized) return 0;
    for (pixel = 0U; pixel < pixels; ++pixel) {
        uint32_t rgba = plane->rgba_buffer[pixel];
        serialized[pixel * 4U] = plane->index_buffer[pixel];
        serialized[pixel * 4U + 1U] = (uint8_t)(rgba >> 16U);
        serialized[pixel * 4U + 2U] = (uint8_t)(rgba >> 8U);
        serialized[pixel * 4U + 3U] = (uint8_t)rgba;
    }
    matches = bytes_have_sha256(serialized, pixels * 4U, expected);
    free(serialized);
    return matches;
}

static int write_capture_plane(const char *path,
                               const Nexus_V1_Vdp2CapturePlane *plane)
{
    FILE *output;
    size_t pixel;
    if (!path || !*path) return 1;
    output = fopen(path, "wb");
    if (!output) return 0;
    for (pixel = 0U; pixel < NEXUS_V1_VDP2_NBG2_CAPTURE_WIDTH *
                            NEXUS_V1_VDP2_NBG2_CAPTURE_HEIGHT; ++pixel) {
        uint32_t rgba = plane->rgba_buffer[pixel];
        uint8_t encoded[4] = {
            plane->index_buffer[pixel], (uint8_t)(rgba >> 16U),
            (uint8_t)(rgba >> 8U), (uint8_t)rgba
        };
        if (fwrite(encoded, 1U, sizeof(encoded), output) != sizeof(encoded)) {
            fclose(output);
            return 0;
        }
    }
    return fclose(output) == 0;
}

static int write_ymir_priority_stack(const char *path,
    const Nexus_V1_Vdp2YmirPriorityStackPlane *plane)
{
    FILE *output;
    size_t pixel;
    if (!path || !*path) return 1;
    output = fopen(path, "wb");
    if (!output) return 0;
    for (pixel = 0U; pixel < NEXUS_V1_VDP2_CAPTURE_PIXELS; ++pixel) {
        uint32_t rgba = plane->rgba_buffer[pixel];
        uint8_t encoded[4] = {plane->top_layer_buffer[pixel],
            (uint8_t)(rgba >> 16U), (uint8_t)(rgba >> 8U), (uint8_t)rgba};
        if (fwrite(encoded, 1U, sizeof(encoded), output) != sizeof(encoded)) {
            fclose(output);
            return 0;
        }
    }
    return fclose(output) == 0;
}

typedef struct {
    const char *vdp1_path;
    const char *vram_path;
    const char *cram_path;
    const char *registers_path;
    const char *output_path;
    const char *vdp1_sha256;
    const char *vram_sha256;
    const char *cram_sha256;
    const char *registers_sha256;
    const char *reference_path;
    const char *reference_sha256;
    int expected_vdp1_rgb_pixels;
    int expected_vdp1_cram_pixels;
    int expected_nbg0_pixels;
    int expected_nbg1_pixels;
    int expected_nbg2_pixels;
} YmirPriorityStackCapture;

static int run_external_ymir_priority_stack_capture(
    const YmirPriorityStackCapture *capture)
{
    uint8_t *vdp1 = NULL, *vdp1_display = NULL;
    uint8_t *vram = NULL, *cram = NULL, *registers = NULL;
    uint8_t *reference = NULL;
    Nexus_V1_Vdp2YmirPriorityStackPlane *plane = NULL;
    Nexus_V1_Vdp2YmirPriorityStackReceipt receipt;
    int ok = 0;
    if (!capture || !capture->vdp1_path || !capture->vram_path ||
        !capture->cram_path || !capture->registers_path ||
        !capture->output_path || !capture->vdp1_sha256 ||
        !capture->vram_sha256 || !capture->cram_sha256 ||
        !capture->registers_sha256 ||
        ((capture->reference_path == NULL) !=
         (capture->reference_sha256 == NULL)))
        return 0;
    vdp1 = read_exact_bytes(capture->vdp1_path,
                            NEXUS_V1_SATURN_VDP1_VRAM_BYTES);
    vram = read_exact_bytes(capture->vram_path,
                            NEXUS_V1_SATURN_VDP2_VRAM_BYTES);
    cram = read_exact_bytes(capture->cram_path,
                            NEXUS_V1_SATURN_VDP2_CRAM_BYTES);
    registers = read_exact_bytes(capture->registers_path, 512U);
    if (capture->reference_path)
        reference = read_exact_bytes(capture->reference_path,
            NEXUS_V1_VDP2_CAPTURE_PIXELS * 4U);
    vdp1_display = (uint8_t *)calloc(1U, NEXUS_V1_SATURN_VDP1_VRAM_BYTES);
    plane = (Nexus_V1_Vdp2YmirPriorityStackPlane *)malloc(sizeof(*plane));
    if (!vdp1 || !vdp1_display || !vram || !cram || !registers || !plane ||
        (capture->reference_path && !reference) ||
        !bytes_have_sha256(vdp1, NEXUS_V1_SATURN_VDP1_VRAM_BYTES,
                           capture->vdp1_sha256) ||
        !bytes_have_sha256(vram, NEXUS_V1_SATURN_VDP2_VRAM_BYTES,
                           capture->vram_sha256) ||
        !bytes_have_sha256(cram, NEXUS_V1_SATURN_VDP2_CRAM_BYTES,
                           capture->cram_sha256) ||
        !bytes_have_sha256(registers, 512U, capture->registers_sha256) ||
        (reference && !bytes_have_sha256(reference,
            NEXUS_V1_VDP2_CAPTURE_PIXELS * 4U,
            capture->reference_sha256)))
        goto cleanup;
    /* Ymir's --dump-memory VDP1 FBS payload stores the draw framebuffer
     * first and the display framebuffer second. VDP2 samples the latter. */
    memcpy(vdp1_display,
           vdp1 + NEXUS_V1_SATURN_VDP1_VRAM_BYTES / 2U,
           NEXUS_V1_SATURN_VDP1_VRAM_BYTES / 2U);
    if (!nexus_v1_vdp2_capture_compose_ymir_priority_stack(
            plane, vdp1_display, NEXUS_V1_SATURN_VDP1_VRAM_BYTES,
            vram, NEXUS_V1_SATURN_VDP2_VRAM_BYTES,
            cram, NEXUS_V1_SATURN_VDP2_CRAM_BYTES, registers, 512U,
            NEXUS_V1_SATURN_VDP2_REGISTER_ORDER_BIG, 1, 1, &receipt) ||
        !receipt.valid || !receipt.capture_only || receipt.renderer_permitted ||
        !receipt.registers_verified || !receipt.vdp1_framebuffer_verified ||
        !receipt.original_ymir_capture_verified ||
        !receipt.pixel_priority_stack_verified ||
        receipt.color_calculation_cases_unsupported ||
        receipt.shadow_cases_unsupported ||
        receipt.vdp1_rgb_pixels != capture->expected_vdp1_rgb_pixels ||
        receipt.vdp1_cram_pixels != capture->expected_vdp1_cram_pixels ||
        receipt.nbg0_pixels != capture->expected_nbg0_pixels ||
        receipt.nbg1_pixels != capture->expected_nbg1_pixels ||
        receipt.nbg2_pixels != capture->expected_nbg2_pixels ||
        receipt.empty_pixels != 0 ||
        !write_ymir_priority_stack(capture->output_path, plane))
        goto cleanup;
    if (reference) {
        size_t pixel;
        for (pixel = 0U; pixel < NEXUS_V1_VDP2_CAPTURE_PIXELS; ++pixel) {
            uint32_t rgba = plane->rgba_buffer[pixel];
            if ((uint8_t)(rgba >> 16U) != reference[pixel * 4U] ||
                (uint8_t)(rgba >> 8U) != reference[pixel * 4U + 1U] ||
                (uint8_t)rgba != reference[pixel * 4U + 2U])
                goto cleanup;
        }
    }
    ok = 1;
cleanup:
    free(plane);
    free(vdp1_display);
    free(reference);
    free(registers);
    free(cram);
    free(vram);
    free(vdp1);
    return ok;
}

static int run_external_ymir_priority_stack(void)
{
    static const YmirPriorityStackCapture frame239 = {
        NULL, NULL, NULL, NULL, NULL,
        "82caef1162741e5baca99e3a8747abb62bd0568df84c75cc0a781176b6636f04",
        "148eebc95b80465a278b7f6d9c1025ee378ec6d288563de3e52ba8af986d94cc",
        "f97f5ebf7bcfab263514895097ec40e6c5dca6dd91fe878f1691e9f204d8515d",
        "308b3c23bb1db102d3833801a836f0141540fe0870069abb37d2041e57f9b555",
        NULL, NULL, 59884U, 491U, 3954U, 148U, 7203U
    };
    static const YmirPriorityStackCapture frame397 = {
        NULL, NULL, NULL, NULL, NULL,
        "82caef1162741e5baca99e3a8747abb62bd0568df84c75cc0a781176b6636f04",
        "148eebc95b80465a278b7f6d9c1025ee378ec6d288563de3e52ba8af986d94cc",
        "76c0f491634b7ddcb86f0fe688e35e1e08e0c594640b068bb175abecec669c77",
        "e853e43bfc7d73d3425c56bf9c5258217c6db63d12859d4cbf587662be0799cd",
        NULL, NULL, 59884U, 491U, 3954U, 148U, 7203U
    };
    static const YmirPriorityStackCapture frame2173 = {
        NULL, NULL, NULL, NULL, NULL,
        "82caef1162741e5baca99e3a8747abb62bd0568df84c75cc0a781176b6636f04",
        "972bf6209ec8543a0665b5077a1c8f069ec213cea100be623b591268e1a03e31",
        "244e7d84c910f5555f12250dde6ccf698171682588ba88253fa73fb2478006de",
        "e853e43bfc7d73d3425c56bf9c5258217c6db63d12859d4cbf587662be0799cd",
        NULL, NULL, 59884U, 491U, 3954U, 148U, 7203U
    };
    static const YmirPriorityStackCapture frame300 = {
        NULL, NULL, NULL, NULL, NULL,
        "82caef1162741e5baca99e3a8747abb62bd0568df84c75cc0a781176b6636f04",
        "6a5660371b4814156efdb6d62f80b884b2515b1014b075366666aa1d071dd711",
        "faf2c8f3adbd4d89a562e28b744f772683cbbc822e0413e2088298a51a35f85a",
        "e26d0632deaad24ae9672eff670536749cf86ae34fdf39b7ad8e06999ee9d607",
        NULL,
        "0428f9e456ea7b211657a5a6b012f08f72ded4c73a75327edc4dc90d3e607273",
        65500U, 491U, 2110U, 74U, 3505U
    };
    YmirPriorityStackCapture capture;
    int ran = 0;
    if (getenv("FIRESTAFF_NEXUS_YMIR_VDP1_FBS")) {
        capture = frame239;
        capture.vdp1_path = getenv("FIRESTAFF_NEXUS_YMIR_VDP1_FBS");
        capture.vram_path = getenv("FIRESTAFF_NEXUS_YMIR_NBG2_VRAM");
        capture.cram_path = getenv("FIRESTAFF_NEXUS_YMIR_NBG2_CRAM");
        capture.registers_path = getenv("FIRESTAFF_NEXUS_YMIR_NBG2_REGS");
        capture.output_path = getenv("FIRESTAFF_NEXUS_YMIR_VDP2_STACK_CAPTURE_OUT");
        if (!run_external_ymir_priority_stack_capture(&capture)) return 0;
        ran = 1;
    }
    if (getenv("FIRESTAFF_NEXUS_YMIR_FRAME397_VDP1_FBS")) {
        capture = frame397;
        capture.vdp1_path = getenv("FIRESTAFF_NEXUS_YMIR_FRAME397_VDP1_FBS");
        capture.vram_path = getenv("FIRESTAFF_NEXUS_YMIR_FRAME397_VDP2_VRAM");
        capture.cram_path = getenv("FIRESTAFF_NEXUS_YMIR_FRAME397_VDP2_CRAM");
        capture.registers_path = getenv("FIRESTAFF_NEXUS_YMIR_FRAME397_VDP2_REGS");
        capture.output_path = getenv("FIRESTAFF_NEXUS_YMIR_FRAME397_STACK_CAPTURE_OUT");
        if (!run_external_ymir_priority_stack_capture(&capture)) return 0;
        ran = 1;
    }
    if (getenv("FIRESTAFF_NEXUS_YMIR_FRAME300_VDP1_FBS")) {
        capture = frame300;
        capture.vdp1_path = getenv("FIRESTAFF_NEXUS_YMIR_FRAME300_VDP1_FBS");
        capture.vram_path = getenv("FIRESTAFF_NEXUS_YMIR_FRAME300_VDP2_VRAM");
        capture.cram_path = getenv("FIRESTAFF_NEXUS_YMIR_FRAME300_VDP2_CRAM");
        capture.registers_path = getenv("FIRESTAFF_NEXUS_YMIR_FRAME300_VDP2_REGS");
        capture.output_path = getenv("FIRESTAFF_NEXUS_YMIR_FRAME300_STACK_OUT");
        capture.reference_path = getenv("FIRESTAFF_NEXUS_YMIR_FRAME300_RENDERED");
        capture.reference_sha256 = frame300.reference_sha256;
        if (!run_external_ymir_priority_stack_capture(&capture)) return 0;
        ran = 1;
    }
    if (getenv("FIRESTAFF_NEXUS_YMIR_FRAME2173_VDP1_FBS")) {
        capture = frame2173;
        capture.vdp1_path = getenv("FIRESTAFF_NEXUS_YMIR_FRAME2173_VDP1_FBS");
        capture.vram_path = getenv("FIRESTAFF_NEXUS_YMIR_FRAME2173_VDP2_VRAM");
        capture.cram_path = getenv("FIRESTAFF_NEXUS_YMIR_FRAME2173_VDP2_CRAM");
        capture.registers_path = getenv("FIRESTAFF_NEXUS_YMIR_FRAME2173_VDP2_REGS");
        capture.output_path = getenv("FIRESTAFF_NEXUS_YMIR_FRAME2173_STACK_OUT");
        if (!run_external_ymir_priority_stack_capture(&capture)) return 0;
        ran = 1;
    }
    return ran;
}

static int run_external_ymir_mapd_nbg1_pass(void)
{
    const char *vram_path = getenv("FIRESTAFF_NEXUS_YMIR_MAPD_NBG1_VRAM");
    const char *cram_path = getenv("FIRESTAFF_NEXUS_YMIR_MAPD_NBG1_CRAM");
    const char *registers_path = getenv("FIRESTAFF_NEXUS_YMIR_MAPD_NBG1_REGS");
    const char *output_path = getenv("FIRESTAFF_NEXUS_YMIR_MAPD_NBG1_OUT");
    uint8_t *vram = NULL, *cram = NULL, *registers = NULL;
    Nexus_V1_Vdp2CapturePlane *plane = NULL;
    Nexus_V1_Vdp2Nbg1CapturePlaneReceipt receipt;
    int ok = 0;

    if (!vram_path && !cram_path && !registers_path) return 1;
    vram = read_exact_bytes(vram_path, NEXUS_V1_SATURN_VDP2_VRAM_BYTES);
    cram = read_exact_bytes(cram_path, NEXUS_V1_SATURN_VDP2_CRAM_BYTES);
    registers = read_exact_bytes(registers_path, 512U);
    plane = (Nexus_V1_Vdp2CapturePlane *)malloc(sizeof(*plane));
    if (!vram || !cram || !registers || !plane ||
        !bytes_have_sha256(vram, NEXUS_V1_SATURN_VDP2_VRAM_BYTES,
            "7367347e6adcb72ac09c946d03a15e1486e12f32593cee7bf3992e58da3b3d9f") ||
        !bytes_have_sha256(cram, NEXUS_V1_SATURN_VDP2_CRAM_BYTES,
            "db282593790dec4a2bf8f76293c0458a1a140adc3351650bd56a417f0359e0de") ||
        !bytes_have_sha256(registers, 512U,
            "774fa7f2e815b14770b83feb83e8c15406041a931e65ed465b1062944a811dd1"))
        goto cleanup;
    if (!nexus_v1_vdp2_capture_decode_ymir_nbg1_tilemap_plane(
            plane, vram, NEXUS_V1_SATURN_VDP2_VRAM_BYTES,
            cram, NEXUS_V1_SATURN_VDP2_CRAM_BYTES,
            registers, 512U, NEXUS_V1_SATURN_VDP2_REGISTER_ORDER_BIG,
            1, &receipt) || !receipt.valid || !receipt.capture_only ||
        receipt.renderer_permitted || !receipt.registers_verified ||
        !receipt.original_ymir_capture_verified || !receipt.nbg1_enabled ||
        !receipt.nbg1_tilemap_mode || !receipt.pnd_size_two_words ||
        receipt.colour_code_256 || receipt.bits_per_pixel != 4 ||
        receipt.name_table_vram_offset != 0x5c000 ||
        receipt.map_page_columns != 64 || receipt.map_page_rows != 64 ||
        receipt.visible_columns != 40 || receipt.visible_rows != 28 ||
        receipt.tiles_decoded != 1120 || receipt.written_pixels != 71680 ||
        receipt.transparent_pixels != 0 ||
        !capture_plane_have_sha256(plane,
            "a7d55feabe7ec09beb568808cb7674274f04b7e90be3758f8d91ed8aad2a0dea") ||
        !write_capture_plane(output_path, plane))
        goto cleanup;
    ok = 1;
cleanup:
    free(plane);
    free(registers);
    free(cram);
    free(vram);
    return ok;
}

static int run_external_ymir_capture_planes(void)
{
    const char *vram_path = getenv("FIRESTAFF_NEXUS_YMIR_NBG2_VRAM");
    const char *cram_path = getenv("FIRESTAFF_NEXUS_YMIR_NBG2_CRAM");
    const char *registers_path = getenv("FIRESTAFF_NEXUS_YMIR_NBG2_REGS");
    const char *output_path = getenv("FIRESTAFF_NEXUS_YMIR_NBG2_CAPTURE_OUT");
    const char *nbg0_output_path = getenv("FIRESTAFF_NEXUS_YMIR_NBG0_CAPTURE_OUT");
    const char *nbg1_output_path = getenv("FIRESTAFF_NEXUS_YMIR_NBG1_CAPTURE_OUT");
    uint8_t *vram = NULL;
    uint8_t *cram = NULL;
    uint8_t *registers = NULL;
    Nexus_V1_Vdp2CapturePlane *plane = NULL;
    Nexus_V1_Vdp2Nbg2CapturePlaneReceipt receipt;
    Nexus_V1_Vdp2Nbg0CapturePlaneReceipt nbg0_receipt;
    Nexus_V1_Vdp2Nbg1CapturePlaneReceipt nbg1_receipt;
    size_t pixel;
    int ok = 0;

    if (!vram_path && !cram_path && !registers_path) return 1;
    vram = read_exact_bytes(vram_path, NEXUS_V1_SATURN_VDP2_VRAM_BYTES);
    cram = read_exact_bytes(cram_path, NEXUS_V1_SATURN_VDP2_CRAM_BYTES);
    registers = read_exact_bytes(registers_path, 512U);
    plane = (Nexus_V1_Vdp2CapturePlane *)malloc(sizeof(*plane));
    if (!vram || !cram || !registers || !plane ||
        !bytes_have_sha256(vram, NEXUS_V1_SATURN_VDP2_VRAM_BYTES,
            "148eebc95b80465a278b7f6d9c1025ee378ec6d288563de3e52ba8af986d94cc") ||
        !bytes_have_sha256(cram, NEXUS_V1_SATURN_VDP2_CRAM_BYTES,
            "f97f5ebf7bcfab263514895097ec40e6c5dca6dd91fe878f1691e9f204d8515d") ||
        !bytes_have_sha256(registers, 512U,
            "308b3c23bb1db102d3833801a836f0141540fe0870069abb37d2041e57f9b555")) {
        goto cleanup;
    }
    if (!nexus_v1_vdp2_capture_decode_ymir_nbg2_tilemap_plane(
            plane, vram, NEXUS_V1_SATURN_VDP2_VRAM_BYTES,
            cram, NEXUS_V1_SATURN_VDP2_CRAM_BYTES,
            registers, 512U, NEXUS_V1_SATURN_VDP2_REGISTER_ORDER_BIG,
            1, &receipt) || !receipt.valid || !receipt.capture_only ||
        receipt.renderer_permitted || !receipt.registers_verified ||
        !receipt.original_ymir_capture_verified || !receipt.nbg2_tilemap_mode ||
        !receipt.pnd_size_two_words || !receipt.colour_code_256 ||
        receipt.name_table_vram_offset != 0x60000 ||
        receipt.map_page_columns != 64 || receipt.map_page_rows != 64 ||
        receipt.visible_columns != 40 || receipt.visible_rows != 28 ||
        receipt.tiles_decoded != 1120 || receipt.written_pixels != 11232 ||
        receipt.transparent_pixels != 60448) goto cleanup;
    for (pixel = 0U; pixel < NEXUS_V1_VDP2_NBG2_CAPTURE_WIDTH *
                            NEXUS_V1_VDP2_NBG2_CAPTURE_HEIGHT; ++pixel) {
        if ((plane->index_buffer[pixel] == 0U) !=
            (plane->rgba_buffer[pixel] == 0U)) goto cleanup;
    }
    if (!write_capture_plane(output_path, plane)) goto cleanup;
    if (!nexus_v1_vdp2_capture_decode_ymir_nbg0_bitmap_plane(
            plane, vram, NEXUS_V1_SATURN_VDP2_VRAM_BYTES,
            cram, NEXUS_V1_SATURN_VDP2_CRAM_BYTES,
            registers, 512U, NEXUS_V1_SATURN_VDP2_REGISTER_ORDER_BIG,
            1, &nbg0_receipt) || !nbg0_receipt.valid ||
        !nbg0_receipt.capture_only || nbg0_receipt.renderer_permitted ||
        !nbg0_receipt.registers_verified ||
        !nbg0_receipt.original_ymir_capture_verified ||
        !nbg0_receipt.nbg0_bitmap_mode || !nbg0_receipt.colour_code_256 ||
        nbg0_receipt.bitmap_vram_offset != 0 ||
        nbg0_receipt.written_pixels != 3968 ||
        nbg0_receipt.transparent_pixels != 67712) goto cleanup;
    for (pixel = 0U; pixel < NEXUS_V1_VDP2_NBG2_CAPTURE_WIDTH *
                            NEXUS_V1_VDP2_NBG2_CAPTURE_HEIGHT; ++pixel) {
        if ((plane->index_buffer[pixel] == 0U) !=
            (plane->rgba_buffer[pixel] == 0U)) goto cleanup;
    }
    if (!write_capture_plane(nbg0_output_path, plane)) goto cleanup;
    if (!nexus_v1_vdp2_capture_decode_ymir_nbg1_tilemap_plane(
            plane, vram, NEXUS_V1_SATURN_VDP2_VRAM_BYTES,
            cram, NEXUS_V1_SATURN_VDP2_CRAM_BYTES,
            registers, 512U, NEXUS_V1_SATURN_VDP2_REGISTER_ORDER_BIG,
            1, &nbg1_receipt) || !nbg1_receipt.valid ||
        !nbg1_receipt.capture_only || nbg1_receipt.renderer_permitted ||
        !nbg1_receipt.registers_verified ||
        !nbg1_receipt.original_ymir_capture_verified ||
        !nbg1_receipt.nbg1_tilemap_mode || !nbg1_receipt.pnd_size_two_words ||
        !nbg1_receipt.colour_code_256 ||
        nbg1_receipt.name_table_vram_offset != 0x5c000 ||
        nbg1_receipt.map_page_columns != 64 || nbg1_receipt.map_page_rows != 64 ||
        nbg1_receipt.visible_columns != 40 || nbg1_receipt.visible_rows != 28 ||
        nbg1_receipt.tiles_decoded != 1120 || nbg1_receipt.written_pixels != 148 ||
        nbg1_receipt.transparent_pixels != 71532) goto cleanup;
    if (!write_capture_plane(nbg1_output_path, plane)) goto cleanup;
    ok = 1;
cleanup:
    free(plane);
    free(registers);
    free(cram);
    free(vram);
    return ok;
}

static int run_external_tilemap_capture(void)
{
    const char *path = getenv("FIRESTAFF_NEXUS_RUNTIME_CAPTURE");
    const char *frame_text = getenv("FIRESTAFF_NEXUS_RUNTIME_CAPTURE_FRAME");
    FILE *file;
    long file_size;
    uint8_t *capture;
    Nexus_Framebuffer framebuffer;
    Nexus_V1_SaturnRuntimeCaptureFrameReceipt frame;
    Nexus_V1_SaturnVdp2RegisterReceipt registers;
    Nexus_V1_Vdp2TilemapCaptureReceipt receipt;
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
    if (!nexus_v1_saturn_runtime_capture_frame(
            capture, (size_t)file_size, frame_index, &frame) ||
        !frame.valid) {
        free(capture);
        return 0;
    }
    {
        Nexus_V1_Vdp2TilemapCaptureInput bounds_input;
        memset(&bounds_input, 0, sizeof(bounds_input));
        bounds_input.capture_name_table = frame.vdp2_vram;
        bounds_input.capture_name_table_size = 4;
        bounds_input.capture_character_generator = frame.vdp2_vram;
        bounds_input.capture_character_generator_size = 64;
        bounds_input.capture_cram = frame.vdp2_cram;
        bounds_input.capture_cram_size = (int)frame.vdp2_cram_size;
        bounds_input.vdp2_registers = frame.vdp2_registers;
        bounds_input.vdp2_registers_size = (int)frame.vdp2_register_size;
        bounds_input.register_byte_order = frame.vdp2_word_order;
        bounds_input.source_name_table = frame.vdp2_vram;
        bounds_input.source_name_table_size = 4;
        bounds_input.source_character_generator = frame.vdp2_vram;
        bounds_input.source_character_generator_size = 64;
        bounds_input.source_cram = frame.vdp2_cram;
        bounds_input.source_cram_size = (int)frame.vdp2_cram_size;
        bounds_input.map_columns = 1;
        bounds_input.map_rows = 1;
        bounds_input.destination_x = INT_MAX;
        if (nexus_v1_vdp2_capture_composite_nbg1_tilemap(
                &framebuffer, &bounds_input, &receipt) || receipt.valid ||
            receipt.renderer_permitted) {
            free(capture);
            return 0;
        }
        bounds_input.destination_x = 0;
        bounds_input.destination_y = INT_MAX;
        if (nexus_v1_vdp2_capture_composite_nbg1_tilemap(
                &framebuffer, &bounds_input, &receipt) || receipt.valid ||
            receipt.renderer_permitted) {
            free(capture);
            return 0;
        }
    }
    nexus_fb_init(&framebuffer);
    nexus_fb_clear(&framebuffer);
    if (nexus_v1_vdp2_capture_decode_runtime_frame_nbg1_tilemap(
            &framebuffer, capture, (size_t)file_size, frame_index,
            INT_MAX, 0, 1, 1, 0, 0, &frame, &registers, &receipt) ||
        nexus_v1_vdp2_capture_decode_runtime_frame_nbg1_tilemap(
            &framebuffer, capture, (size_t)file_size, frame_index,
            0, INT_MAX, 1, 1, 0, 0, &frame, &registers, &receipt) ||
        nexus_v1_vdp2_capture_decode_runtime_frame_nbg1_tilemap(
            &framebuffer, capture, (size_t)file_size, frame_index,
            0, 0, 1, 1, INT_MAX, 0, &frame, &registers, &receipt) ||
        nexus_v1_vdp2_capture_decode_runtime_frame_nbg1_tilemap(
            &framebuffer, capture, (size_t)file_size, frame_index,
            0, 0, 1, 1, 0, INT_MAX, &frame, &registers, &receipt)) {
        free(capture);
        return 0;
    }
    memset(&frame, 0, sizeof(frame));
    memset(&registers, 0, sizeof(registers));
    memset(&receipt, 0, sizeof(receipt));
    ok = nexus_v1_vdp2_capture_decode_runtime_frame_nbg1_tilemap(
        &framebuffer, capture, (size_t)file_size, frame_index,
        0, 0, 64, 64, 0, 0, &frame, &registers, &receipt) &&
        frame.valid && registers.valid && registers.nbg1_enabled &&
        !registers.nbg1_bitmap_mode && receipt.valid && receipt.capture_only &&
        !receipt.renderer_permitted && receipt.original_saturn_capture_verified &&
        receipt.tiles_decoded == 64;
    if (!ok) {
        fprintf(stderr,
            "NBG1 capture decode rejected: frame=%u frame_valid=%d "
            "registers_valid=%d enabled=%d bitmap=%d bpp=%d "
            "receipt_valid=%d capture_only=%d renderer=%d tiles=%d "
            "transparent=%d written=%d\n",
            frame_index, frame.valid, registers.valid, registers.nbg1_enabled,
            registers.nbg1_bitmap_mode, registers.nbg1_colour_code,
            receipt.valid, receipt.capture_only, receipt.renderer_permitted,
            receipt.tiles_decoded, receipt.transparent_pixels,
            receipt.written_pixels);
    }
    free(capture);
    return ok;
}

int main(void)
{
    const size_t prefix = sizeof(NEXUS_V1_SATURN_RUNTIME_CAPTURE_MAGIC) - 1U;
    const size_t vdp1_marker = sizeof(NEXUS_V1_SATURN_VDP1_RAW_MAGIC_V1) - 1U;
    const size_t vdp2_marker = sizeof(NEXUS_V1_SATURN_VDP2_RAW_MAGIC) - 1U;
    const size_t blob_size = prefix + sizeof("frame=0\n") - 1U + vdp1_marker +
        NEXUS_V1_SATURN_VDP1_PAYLOAD_BYTES + vdp2_marker +
        NEXUS_V1_SATURN_VDP2_PAYLOAD_BYTES;
    uint8_t *blob = (uint8_t *)calloc(1U, blob_size);
    uint8_t *legacy_blob = NULL;
    uint8_t source_name[4] = {0, 0, 0, 0};
    uint8_t source_character[64];
    uint8_t source_cram[NEXUS_V1_SATURN_VDP2_CRAM_BYTES];
    Nexus_V1_Vdp2RuntimeTilemapBinding binding;
    Nexus_V1_Vdp2TilemapCaptureReceipt receipt;
    Nexus_V1_SaturnVdp2RegisterReceipt registers;
    Nexus_Framebuffer framebuffer;
    size_t offset = 0U;
    size_t vdp2_marker_offset;
    uint8_t *vdp2_payload;
    uint8_t *vdp2_vram;
    uint8_t *vdp2_regs;
    int i;

    if (!blob) return 1;
    memcpy(blob + offset, NEXUS_V1_SATURN_RUNTIME_CAPTURE_MAGIC, prefix);
    offset += prefix;
    memcpy(blob + offset, "frame=0\n", sizeof("frame=0\n") - 1U);
    offset += sizeof("frame=0\n") - 1U;
    memcpy(blob + offset, NEXUS_V1_SATURN_VDP1_RAW_MAGIC_V1, vdp1_marker);
    offset += vdp1_marker + NEXUS_V1_SATURN_VDP1_PAYLOAD_BYTES;
    vdp2_marker_offset = offset;
    memcpy(blob + offset, NEXUS_V1_SATURN_VDP2_RAW_MAGIC, vdp2_marker);
    offset += vdp2_marker;
    vdp2_payload = blob + offset;
    vdp2_regs = vdp2_payload;
    vdp2_vram = vdp2_payload + NEXUS_V1_SATURN_VDP2_REG_BYTES;
    le16(vdp2_regs + 0x00, 0x8020U);
    le16(vdp2_regs + 0x20, 0x0002U);
    le16(vdp2_regs + 0x28, 0x1000U);
    le16(vdp2_regs + 0xe4, 0x0000U);
    le16(vdp2_vram + 0, 0U);
    le16(vdp2_vram + 2, 0U);
    for (i = 0; i < 64; ++i) {
        source_character[i] = 0x11U;
        vdp2_vram[0x100 + i] = 0x11U;
    }
    le16(vdp2_vram + 0, 0U);
    memcpy(source_cram,
           vdp2_vram + NEXUS_V1_SATURN_VDP2_VRAM_BYTES,
           sizeof(source_cram));
    memset(&binding, 0, sizeof(binding));
    binding.capture_name_table_offset = 0U;
    binding.capture_character_generator_offset = 0x100U;
    binding.capture_character_generator_size = sizeof(source_character);
    binding.source_name_table = source_name;
    binding.source_name_table_size = sizeof(source_name);
    binding.source_character_generator = source_character;
    binding.source_character_generator_size = sizeof(source_character);
    binding.source_cram = source_cram;
    binding.source_cram_size = sizeof(source_cram);
    binding.map_columns = 1;
    binding.map_rows = 1;
    binding.destination_x = 20;
    binding.destination_y = 30;
    binding.source_hash_verified = 1;
    binding.transparent_index_zero_verified = 1;
    nexus_fb_init(&framebuffer);
    nexus_fb_clear(&framebuffer);
    if (!nexus_v1_vdp2_capture_replay_runtime_frame_nbg1_tilemap(
            &framebuffer, blob, blob_size, 0U, &binding, &receipt) ||
        !receipt.valid || receipt.layer_registers_verified != 1 ||
        receipt.tiles_decoded != 1 || framebuffer.color_buffer[
            30 * NEXUS_FB_W + 20] != 0x11U) {
        fprintf(stderr, "FAIL: raw Saturn VDP2 tilemap replay\n");
        free(blob);
        return 1;
    }
    /* The same frame must also be decodable as a raw capture-only consumer;
     * this path intentionally does not grant source or host-composition
     * admission. */
    /* VDP2 character numbers advance in 32-byte units, including at 8bpp. */
    le16(vdp2_vram + 2, 8U);
    le16(vdp2_regs + 0x3c, 0U);
    le16(vdp2_regs + 0x44, 0U);
    le16(vdp2_regs + 0x46, 0U);
    nexus_fb_clear(&framebuffer);
    if (!nexus_v1_vdp2_capture_decode_runtime_frame_nbg1_tilemap(
            &framebuffer, blob, blob_size, 0U, 0, 0, 1, 1, 25, 35,
            NULL, NULL, &receipt) || !receipt.valid || !receipt.capture_only ||
        receipt.renderer_permitted || framebuffer.color_buffer[
            35 * NEXUS_FB_W + 25] != 0x11U) {
        fprintf(stderr, "FAIL: raw Saturn VDP2 tilemap capture decode\n");
        free(blob);
        return 1;
    }
    /* NBG1 SCXIN1 (0x80) and SCYIN1 (0x84) offset the name-table lookup.
     * Put the only visible character at map cell (1,1) and require an 8x8
     * source scroll to select it. */
    le16(vdp2_vram + 2, 0U);
    le16(vdp2_vram + (65U * 4U), 0U);
    le16(vdp2_vram + (65U * 4U) + 2U, 8U);
    le16(vdp2_regs + 0x80U, 8U);
    le16(vdp2_regs + 0x84U, 8U);
    nexus_fb_clear(&framebuffer);
    memset(&registers, 0, sizeof(registers));
    memset(&receipt, 0, sizeof(receipt));
    if (!nexus_v1_vdp2_capture_decode_runtime_frame_nbg1_tilemap(
            &framebuffer, blob, blob_size, 0U, 0, 0, 1, 1, 27, 37,
            NULL, &registers, &receipt) || !receipt.valid ||
        registers.nbg1_scroll_x != 8U || registers.nbg1_scroll_y != 8U ||
        framebuffer.color_buffer[37 * NEXUS_FB_W + 27] != 0x11U) {
        fprintf(stderr, "FAIL: NBG1 scroll is not applied to tile fetch\n");
        free(blob);
        return 1;
    }
    le16(vdp2_regs + 0x80U, 0U);
    le16(vdp2_regs + 0x84U, 0U);
    /* The upstream Mednafen candidate serializes every Saturn word in
     * big-endian bus order. Exercise the same capture-only decoder with that
     * representation so the local little-endian fixture cannot mask a
     * producer-order regression. */
    be16(vdp2_regs + 0x00, 0x8020U);
    be16(vdp2_regs + 0x20, 0x0002U);
    be16(vdp2_regs + 0x28, 0x1000U);
    be16(vdp2_regs + 0xe4, 0x0000U);
    be16(vdp2_regs + 0x3c, 0x0000U);
    be16(vdp2_regs + 0x44, 0x0000U);
    be16(vdp2_regs + 0x46, 0x0000U);
    be16(vdp2_vram + 0, 0U);
    be16(vdp2_vram + 2, 8U);
    nexus_fb_clear(&framebuffer);
    memset(&registers, 0, sizeof(registers));
    memset(&receipt, 0, sizeof(receipt));
    if (!nexus_v1_vdp2_capture_decode_runtime_frame_nbg1_tilemap(
            &framebuffer, blob, blob_size, 0U, 0, 0, 1, 1, 26, 36,
            NULL, &registers, &receipt) || !receipt.valid ||
        registers.byte_order != NEXUS_V1_SATURN_VDP2_REGISTER_ORDER_BIG ||
        !registers.nbg1_enabled || receipt.renderer_permitted ||
        framebuffer.color_buffer[36 * NEXUS_FB_W + 26] != 0x11U) {
        fprintf(stderr, "FAIL: big-endian Mednafen VDP2 tilemap capture decode\n");
        free(blob);
        return 1;
    }
    /* Early V2 captures carried one redundant draw-buffer byte before the
     * VDP2 marker. The transport reader must retain it without shifting VDP1
     * or VDP2 regions. */
    legacy_blob = (uint8_t *)calloc(1U, blob_size + 1U);
    if (!legacy_blob) {
        free(blob);
        return 1;
    }
    memcpy(legacy_blob, blob, vdp2_marker_offset);
    legacy_blob[vdp2_marker_offset] = 1U;
    memcpy(legacy_blob + vdp2_marker_offset + 1U,
           blob + vdp2_marker_offset, blob_size - vdp2_marker_offset);
    {
        Nexus_V1_SaturnRuntimeCaptureFrameReceipt frame_receipt;
        if (!nexus_v1_saturn_runtime_capture_frame(
                legacy_blob, blob_size + 1U, 0U, &frame_receipt) ||
            !frame_receipt.valid || !frame_receipt.vdp1_draw_which ||
            *frame_receipt.vdp1_draw_which != 1U) {
            fprintf(stderr, "FAIL: legacy draw-buffer transport\n");
            free(legacy_blob);
            free(blob);
            return 1;
        }
    }
    free(legacy_blob);
    free(blob);
    if (getenv("FIRESTAFF_NEXUS_RUNTIME_CAPTURE") &&
        getenv("FIRESTAFF_NEXUS_RUNTIME_CAPTURE_FRAME") &&
        !run_external_tilemap_capture()) {
        fprintf(stderr, "FAIL: external VDP2 NBG1 tilemap capture decode\n");
        return 1;
    }
    if ((getenv("FIRESTAFF_NEXUS_YMIR_NBG2_VRAM") ||
         getenv("FIRESTAFF_NEXUS_YMIR_NBG2_CRAM") ||
         getenv("FIRESTAFF_NEXUS_YMIR_NBG2_REGS")) &&
        !run_external_ymir_capture_planes()) {
        fprintf(stderr, "FAIL: authentic Ymir NBG0/NBG2 capture decode\n");
        return 1;
    }
    if ((getenv("FIRESTAFF_NEXUS_YMIR_MAPD_NBG1_VRAM") ||
         getenv("FIRESTAFF_NEXUS_YMIR_MAPD_NBG1_CRAM") ||
         getenv("FIRESTAFF_NEXUS_YMIR_MAPD_NBG1_REGS")) &&
        !run_external_ymir_mapd_nbg1_pass()) {
        fprintf(stderr, "FAIL: authentic Ymir MAPD NBG1 4bpp plane decode\n");
        return 1;
    }
    if ((getenv("FIRESTAFF_NEXUS_YMIR_VDP1_FBS") ||
         getenv("FIRESTAFF_NEXUS_YMIR_FRAME397_VDP1_FBS") ||
         getenv("FIRESTAFF_NEXUS_YMIR_FRAME300_VDP1_FBS")) &&
        !run_external_ymir_priority_stack()) {
        fprintf(stderr, "FAIL: authentic Ymir VDP1/VDP2 priority composition\n");
        return 1;
    }
    puts("test_nexus_v1_vdp2_runtime_tilemap: PASS");
    return 0;
}
