#include "firestaff_x68k_media_receipt.h"
#include "nexus_v1_vdp2_capture_compositor.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FRAME59_WIDTH 320U
#define FRAME59_HEIGHT 224U
#define FRAME59_PIXELS (FRAME59_WIDTH * FRAME59_HEIGHT)
#define FRAME59_RGB_BYTES (FRAME59_PIXELS * 3U)
#define VDP1_FBS_PAIR_BYTES (512U * 256U * 2U * 2U)

typedef struct {
    uint8_t *data;
    size_t size;
} Blob;

static Blob read_file(const char *path)
{
    Blob blob = {NULL, 0U};
    FILE *file = fopen(path, "rb");
    long length;

    if (!file || fseek(file, 0, SEEK_END) != 0 ||
        (length = ftell(file)) < 0 || fseek(file, 0, SEEK_SET) != 0) {
        if (file) fclose(file);
        return blob;
    }
    blob.data = (uint8_t *)malloc((size_t)length + 1U);
    if (!blob.data || fread(blob.data, 1U, (size_t)length, file) !=
            (size_t)length) {
        free(blob.data);
        blob.data = NULL;
        fclose(file);
        return blob;
    }
    fclose(file);
    blob.data[length] = 0U;
    blob.size = (size_t)length;
    return blob;
}

static Blob read_capture_file(const char *root, const char *relative_path)
{
    char path[4096];
    int length = snprintf(path, sizeof(path), "%s/%s", root, relative_path);
    Blob missing = {NULL, 0U};
    if (length < 0 || (size_t)length >= sizeof(path)) return missing;
    return read_file(path);
}

static int has_sha256(const Blob *blob, const char *expected)
{
    char actual[65];
    if (!blob->data || firestaff_x68k_media_receipt_sha256_hex(
            blob->data, blob->size, actual, sizeof(actual)) != 0) {
        fprintf(stderr, "FAIL: frame-59 evidence file is missing or unreadable\n");
        return 0;
    }
    if (strcmp(actual, expected) != 0) {
        fprintf(stderr, "FAIL: frame-59 evidence hash %s, expected %s\n",
                actual, expected);
        return 0;
    }
    return 1;
}

static void free_blob(Blob *blob)
{
    free(blob->data);
    blob->data = NULL;
    blob->size = 0U;
}

static int write_rgb_output(const char *path,
    const Nexus_V1_Vdp2YmirMenuCaptureFramebuffer *framebuffer)
{
    FILE *file;
    size_t pixel;
    if (!path || !*path) return 1;
    file = fopen(path, "wb");
    if (!file) return 0;
    for (pixel = 0U; pixel < FRAME59_PIXELS; ++pixel) {
        uint32_t rgba = framebuffer->rgba_buffer[pixel];
        uint8_t rgb[3] = {(uint8_t)(rgba >> 16U),
                          (uint8_t)(rgba >> 8U), (uint8_t)rgba};
        if (fwrite(rgb, 1U, sizeof(rgb), file) != sizeof(rgb)) {
            fclose(file);
            return 0;
        }
    }
    return fclose(file) == 0;
}

int main(void)
{
    const char *root = getenv("FIRESTAFF_NEXUS_YMIR_FRAME59_CAPTURE_DIR");
    const char *output_path = getenv("FIRESTAFF_NEXUS_YMIR_FRAME59_REPLAY_OUT");
    Blob fbs = {NULL, 0U}, vram = {NULL, 0U}, cram = {NULL, 0U};
    Blob registers = {NULL, 0U}, native_rgb = {NULL, 0U};
    Blob screenshot = {NULL, 0U}, bundle = {NULL, 0U}, trace = {NULL, 0U};
    Blob register_snapshot = {NULL, 0U};
    Nexus_V1_Vdp2YmirMenuCaptureFramebuffer *framebuffer = NULL;
    Nexus_V1_Vdp2YmirTitlePromptCaptureInput input;
    Nexus_V1_Vdp2YmirTitlePromptCaptureReceipt receipt;
    size_t pixel;
    int result = 1;

    if (!root || !*root) {
        fprintf(stderr, "SKIP: set FIRESTAFF_NEXUS_YMIR_FRAME59_CAPTURE_DIR\n");
        return 77;
    }
    fbs = read_capture_file(root, "dumps/vdp1-fbs.bin");
    vram = read_capture_file(root, "dumps/vdp2-vram.bin");
    cram = read_capture_file(root, "dumps/vdp2-cram.bin");
    registers = read_capture_file(root, "dumps/vdp2-registers.bin");
    bundle = read_capture_file(root, "dumps/vdp1-capture-bundle.txt");
    trace = read_capture_file(root, "dumps/vdp1-command-trace.csv");
    register_snapshot = read_capture_file(root,
        "dumps/vdp-register-snapshot.txt");
    native_rgb = read_capture_file(root,
        "screenshots/vdp12-title-frame59-native.rgb");
    screenshot = read_capture_file(root,
        "screenshots/DUNGEON MASTER NEXUS [T-9111G]-20261030T085957.601.png");
    framebuffer = (Nexus_V1_Vdp2YmirMenuCaptureFramebuffer *)
        malloc(sizeof(*framebuffer));
    if (!fbs.data || !vram.data || !cram.data || !registers.data ||
        !bundle.data || !trace.data || !register_snapshot.data ||
        !native_rgb.data || !screenshot.data || !framebuffer ||
        fbs.size != VDP1_FBS_PAIR_BYTES || vram.size != 512U * 1024U ||
        cram.size != 4096U || registers.size != 512U ||
        native_rgb.size != FRAME59_RGB_BYTES ||
        !has_sha256(&fbs,
            "01c47c584b2bb9e64892db2d42fc3fb231d3edd2b01f9eec8d562b7ed70681a9") ||
        !has_sha256(&vram,
            "de1a77427dc62457eb4b59fa0424fd25e9768ce229505445311929d3bdae405c") ||
        !has_sha256(&cram,
            "db282593790dec4a2bf8f76293c0458a1a140adc3351650bd56a417f0359e0de") ||
        !has_sha256(&registers,
            "79efa616c2db727749aa9018864d1ad32c1583935652fafb16534ba69f8e1f2d") ||
        !has_sha256(&bundle,
            "4f9829e24380ae088a12277060759248eae9cf6142e0f23abe737cfa12622dbc") ||
        !has_sha256(&trace,
            "2e8e02f4570e8cc8eac2859857af4e7b915849f1d8c3f4ddb9b9872748f38036") ||
        !has_sha256(&register_snapshot,
            "e80c6bd419de86bb33da83edcb5fe770337c5a5760c94dd957ad00c3bee81fa2") ||
        !has_sha256(&native_rgb,
            "4a742307a5b31af17e0aff1809bc62513fde6ae56b3cd3eb3b9542b05e6b96fe") ||
        !has_sha256(&screenshot,
            "2fa29fd409034fbbe404d698d33ee5f12e0c4c55caeebc9c7533ffcb540a0624")) {
        fprintf(stderr, "frame-59 evidence sizes: FBS=%lu VRAM=%lu CRAM=%lu "
                "registers=%lu native-RGB=%lu screenshot=%lu\n",
                (unsigned long)fbs.size, (unsigned long)vram.size,
                (unsigned long)cram.size, (unsigned long)registers.size,
                (unsigned long)native_rgb.size,
                (unsigned long)screenshot.size);
        fprintf(stderr, "FAIL: frame-59 authentic capture or reference hash/size\n");
        goto done;
    }

    memset(&input, 0, sizeof(input));
    input.vdp1_framebuffers = fbs.data;
    input.vdp1_framebuffers_size = fbs.size;
    input.vdp2_vram = vram.data;
    input.vdp2_vram_size = vram.size;
    input.vdp2_cram = cram.data;
    input.vdp2_cram_size = cram.size;
    input.vdp2_registers = registers.data;
    input.vdp2_registers_size = registers.size;
    input.register_byte_order = NEXUS_V1_SATURN_VDP2_REGISTER_ORDER_BIG;
    input.original_ymir_capture_verified = 1;
    input.nbg0_over_vdp1_order_verified = 1;
    memset(&receipt, 0, sizeof(receipt));
    if (!nexus_v1_vdp2_capture_replay_ymir_title_prompt(
            framebuffer, &input, &receipt) || !receipt.valid ||
        !receipt.capture_only || receipt.renderer_permitted ||
        !receipt.registers_verified ||
        !receipt.original_ymir_capture_verified ||
        !receipt.nbg0_over_vdp1_order_verified ||
        receipt.width != (int)FRAME59_WIDTH ||
        receipt.height != (int)FRAME59_HEIGHT ||
        receipt.nbg0_overlay_pixels != 18513 ||
        receipt.vdp1_background_pixels != 53167) {
        fprintf(stderr, "FAIL: frame-59 title prompt composition receipt\n");
        goto done;
    }
    for (pixel = 0U; pixel < FRAME59_PIXELS; ++pixel) {
        uint32_t rgba = framebuffer->rgba_buffer[pixel];
        if ((uint8_t)(rgba >> 16U) != native_rgb.data[pixel * 3U] ||
            (uint8_t)(rgba >> 8U) != native_rgb.data[pixel * 3U + 1U] ||
            (uint8_t)rgba != native_rgb.data[pixel * 3U + 2U]) {
            fprintf(stderr, "FAIL: frame-59 native RGB mismatch at pixel %lu\n",
                    (unsigned long)pixel);
            goto done;
        }
    }
    if (!write_rgb_output(output_path, framebuffer)) {
        fprintf(stderr, "FAIL: could not write frame-59 replay output\n");
        goto done;
    }
    printf("PASS: authentic Nexus frame 59 NBG0-over-VDP1 capture replay "
           "matches %u native RGB pixels exactly; NBG0=%d VDP1=%d; "
           "renderer_permitted=false\n", (unsigned int)FRAME59_PIXELS,
           receipt.nbg0_overlay_pixels, receipt.vdp1_background_pixels);
    result = 0;

done:
    free(framebuffer);
    free_blob(&register_snapshot);
    free_blob(&trace);
    free_blob(&bundle);
    free_blob(&screenshot);
    free_blob(&native_rgb);
    free_blob(&registers);
    free_blob(&cram);
    free_blob(&vram);
    free_blob(&fbs);
    return result;
}
