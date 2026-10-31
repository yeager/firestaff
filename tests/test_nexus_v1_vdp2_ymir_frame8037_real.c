#include "firestaff_x68k_media_receipt.h"
#include "nexus_v1_vdp2_capture_compositor.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FRAME_WIDTH 320U
#define FRAME_HEIGHT 224U
#define FRAME_PIXELS (FRAME_WIDTH * FRAME_HEIGHT)
#define FBS_BYTES (512U * 256U * 2U)
#define VRAM_BYTES (512U * 1024U)
#define CRAM_BYTES (4U * 1024U)
#define REGISTER_BYTES 512U
#define RGBX_BYTES (FRAME_PIXELS * 4U)

typedef struct { uint8_t *data; size_t size; } Blob;

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

static void free_blob(Blob *blob)
{
    free(blob->data);
    blob->data = NULL;
    blob->size = 0U;
}

static int has_sha256(const Blob *blob, const char *expected)
{
    char actual[65];
    return blob->data && firestaff_x68k_media_receipt_sha256_hex(
        blob->data, blob->size, actual, sizeof(actual)) == 0 &&
        strcmp(actual, expected) == 0;
}

static int read_capture_file(const char *root, const char *name, Blob *blob)
{
    char path[4096];
    int length = snprintf(path, sizeof(path), "%s/%s", root, name);
    if (length < 0 || (size_t)length >= sizeof(path)) return 0;
    *blob = read_file(path);
    return blob->data != NULL;
}

static int read_reference_file(const char *root, const char *name, Blob *blob)
{
    char path[4096];
    int length = snprintf(path, sizeof(path), "%s/%s", root, name);
    if (length < 0 || (size_t)length >= sizeof(path)) return 0;
    *blob = read_file(path);
    return blob->data != NULL;
}

static int contains(const Blob *blob, const char *text)
{
    return blob->data && strstr((const char *)blob->data, text) != NULL;
}

int main(void)
{
    const char *capture_root = getenv("FIRESTAFF_NEXUS_YMIR_FRAME8037_CAPTURE_DIR");
    const char *reference_root = getenv("FIRESTAFF_NEXUS_YMIR_FRAME8037_REFERENCE_DIR");
    static const char *const pass_names[2] = {
        "vdp1-8037-vdp2-9118-320x224.rgbx8888le",
        "vdp1-8037-vdp2-9119-320x224.rgbx8888le"
    };
    static const char *const pass_sha[2] = {
        "0a37fe5d4c31821001f0b6973e8c768150df3a35429b581ebd8131fa34802602",
        "0a37fe5d4c31821001f0b6973e8c768150df3a35429b581ebd8131fa34802602"
    };
    Blob fbs = {0}, vram = {0}, cram = {0}, registers = {0};
    Blob bundle = {0}, trace = {0}, register_receipt = {0};
    Blob passes[2] = {{0}};
    Nexus_V1_Vdp2YmirMenuCaptureFramebuffer *framebuffer = NULL;
    Nexus_V1_Vdp2YmirMenuCaptureInput input;
    Nexus_V1_Vdp2YmirMenuCaptureReceipt receipt;
    unsigned int pass, pixel, exact = 0U;
    int result = 1;

    if (!capture_root || !*capture_root || !reference_root || !*reference_root) {
        fprintf(stderr, "SKIP: set both authentic frame-8037 capture directories\n");
        return 77;
    }
    if (!read_capture_file(capture_root, "vdp1-fbs.bin", &fbs) ||
        !read_capture_file(capture_root, "vdp2-vram.bin", &vram) ||
        !read_capture_file(capture_root, "vdp2-cram.bin", &cram) ||
        !read_capture_file(capture_root, "vdp2-registers.bin", &registers) ||
        !read_capture_file(capture_root, "vdp1-capture-bundle.txt", &bundle) ||
        !read_capture_file(capture_root, "vdp1-command-trace.csv", &trace) ||
        !read_capture_file(capture_root, "vdp-register-snapshot.txt", &register_receipt)) {
        fprintf(stderr, "missing authentic frame-8037 Ymir capture file\n");
        goto done;
    }
    if (fbs.size != FBS_BYTES * 2U || vram.size != VRAM_BYTES ||
        cram.size != CRAM_BYTES || registers.size != REGISTER_BYTES ||
        !has_sha256(&fbs, "2dc15ef07aaacd89a00af18fe3e0bf3677bdb626f3a852f37820eb29bc9ce1ce") ||
        !has_sha256(&vram, "de1a77427dc62457eb4b59fa0424fd25e9768ce229505445311929d3bdae405c") ||
        !has_sha256(&cram, "db282593790dec4a2bf8f76293c0458a1a140adc3351650bd56a417f0359e0de") ||
        !has_sha256(&registers, "e69d03094ef856922413e073cb9078015e52849786268d6421c7df00a655965e") ||
        !has_sha256(&bundle, "10e9dd1ef6bff3ca95ffb7a57265819d92ab46621ab157d19b32478af360b874") ||
        !has_sha256(&trace, "f15eb2038f2195c7d7cc64947e5d50b679b8280073e2aadbcec4d72b9a5ccf29") ||
        !has_sha256(&register_receipt, "4181505354f1ba4f4c9615ca9a337f2dcd9fc30a4448869d06b43e5d15ee0cc6")) {
        fprintf(stderr, "frame-8037 capture size or SHA-256 mismatch\n");
        goto done;
    }
    if (!contains(&bundle, "capture_frame_index=8037\n") ||
        !contains(&bundle, "capture_next_command_index=178\n") ||
        !contains(&bundle, "trace_file_complete=1\n") ||
        !contains(&bundle, "trace_rows=329584\n") ||
        !contains(&bundle, "trace_rows_for_capture_frame=178\n") ||
        !contains(&bundle, "trace_dropped_total=0\n") ||
        !contains(&register_receipt, "vdp1_frame_index=8037\n") ||
        !contains(&register_receipt, "vdp1_next_command_index=178\n")) {
        fprintf(stderr, "capture receipts do not bind complete frame 8037\n");
        goto done;
    }
    for (pass = 0U; pass < 2U; ++pass) {
        if (!read_reference_file(reference_root, pass_names[pass], &passes[pass]) ||
            passes[pass].size != RGBX_BYTES ||
            !has_sha256(&passes[pass], pass_sha[pass])) {
            fprintf(stderr, "missing or invalid Ymir VDP2 pass reference %s\n",
                    pass_names[pass]);
            goto done;
        }
    }
    if (memcmp(passes[0].data, passes[1].data, RGBX_BYTES) != 0) {
        fprintf(stderr, "the two authenticated VDP2 passes differ\n");
        goto done;
    }

    framebuffer = (Nexus_V1_Vdp2YmirMenuCaptureFramebuffer *)
        malloc(sizeof(*framebuffer));
    if (!framebuffer) { fprintf(stderr, "out of memory\n"); goto done; }
    memset(&input, 0, sizeof(input));
    /* The capture exports draw then display FBS; use the displayed bank. */
    input.vdp1_display_framebuffer = fbs.data + FBS_BYTES;
    input.vdp1_display_framebuffer_size = FBS_BYTES;
    input.vdp2_vram = vram.data;
    input.vdp2_vram_size = vram.size;
    input.vdp2_cram = cram.data;
    input.vdp2_cram_size = cram.size;
    input.vdp2_registers = registers.data;
    input.vdp2_registers_size = registers.size;
    input.register_byte_order = NEXUS_V1_SATURN_VDP2_REGISTER_ORDER_BIG;
    input.original_ymir_capture_verified = 1;
    input.command_trace_complete_verified = 1;
    if (!nexus_v1_vdp2_capture_replay_ymir_title_menu(
            framebuffer, &input, &receipt) || !receipt.valid ||
        !receipt.capture_only || receipt.renderer_permitted ||
        !receipt.registers_verified ||
        !receipt.command_trace_complete_verified ||
        !receipt.original_ymir_capture_verified ||
        receipt.width != (int)FRAME_WIDTH || receipt.height != (int)FRAME_HEIGHT ||
        receipt.vdp1_direct_pixels + receipt.vdp1_palette_pixels != (int)FRAME_PIXELS ||
        receipt.vdp1_direct_pixels != 70185 ||
        receipt.vdp1_palette_pixels != 1495 ||
        receipt.nbg0_overlay_pixels <= 0) {
        fprintf(stderr, "frame-8037 replay rejected: valid=%d direct=%d palette=%d NBG0=%d renderer=%d\n",
                receipt.valid, receipt.vdp1_direct_pixels,
                receipt.vdp1_palette_pixels, receipt.nbg0_overlay_pixels,
                receipt.renderer_permitted);
        goto done;
    }
    for (pixel = 0U; pixel < FRAME_PIXELS; ++pixel) {
        uint32_t rgba = framebuffer->rgba_buffer[pixel];
        const uint8_t actual[3] = {
            (uint8_t)(rgba >> 16U), (uint8_t)(rgba >> 8U), (uint8_t)rgba
        };
        if (memcmp(actual, passes[0].data + (size_t)pixel * 4U, 3U) != 0 ||
            memcmp(actual, passes[1].data + (size_t)pixel * 4U, 3U) != 0) {
            fprintf(stderr, "frame-8037 RGB mismatch at x=%u y=%u\n",
                    pixel % FRAME_WIDTH, pixel / FRAME_WIDTH);
            goto done;
        }
        ++exact;
    }
    printf("PASS: authentic Nexus frame 8037 capture-only composition matches both Ymir passes exactly (%u/%u RGB pixels; direct=%d palette=%d NBG0-overlay=%d); DGN ownership=unclaimed; renderer_permitted=false\n",
           exact, FRAME_PIXELS, receipt.vdp1_direct_pixels,
           receipt.vdp1_palette_pixels, receipt.nbg0_overlay_pixels);
    result = 0;
done:
    free(framebuffer);
    free_blob(&fbs); free_blob(&vram); free_blob(&cram); free_blob(&registers);
    free_blob(&bundle); free_blob(&trace); free_blob(&register_receipt);
    free_blob(&passes[0]); free_blob(&passes[1]);
    return result;
}
