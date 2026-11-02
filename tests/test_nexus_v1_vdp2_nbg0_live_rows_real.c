#include "firestaff_x68k_media_receipt.h"
#include "nexus_v1_vdp2_capture_compositor.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct { uint8_t *data; size_t size; } Blob;

static Blob read_file(const char *path)
{
    Blob b = {NULL, 0U}; FILE *f; long n;
    if (!path || !(f = fopen(path, "rb"))) return b;
    if (fseek(f, 0, SEEK_END) || (n = ftell(f)) < 0 || fseek(f, 0, SEEK_SET)) { fclose(f); return b; }
    b.data = (uint8_t *)malloc((size_t)n + 1U);
    if (!b.data || fread(b.data, 1U, (size_t)n, f) != (size_t)n) { free(b.data); b.data = NULL; fclose(f); return b; }
    fclose(f); b.data[n] = 0U; b.size = (size_t)n; return b;
}

static int hash_is(const Blob *b, const char *expected)
{
    char actual[65];
    return b->data && firestaff_x68k_media_receipt_sha256_hex(b->data, b->size, actual, sizeof(actual)) == 0 && strcmp(actual, expected) == 0;
}

static Blob read_capture_file(const char *root, const char *relative)
{
    size_t n = strlen(root) + strlen(relative) + 2U;
    char *path = (char *)malloc(n);
    Blob b = {NULL, 0U};
    if (!path) return b;
    snprintf(path, n, "%s/%s", root, relative);
    b = read_file(path);
    free(path);
    return b;
}

int main(void)
{
    const char *root = getenv("FIRESTAFF_NEXUS_NBG0_LIVE_CAPTURE_DIR");
    const char *replay_path = getenv("FIRESTAFF_NEXUS_NBG0_LIVE_REPLAY_RGB");
    Blob fbs = {0}, vram = {0}, cram = {0}, regs = {0};
    Nexus_V1_Vdp2YmirMenuCaptureFramebuffer *fb = NULL;
    Nexus_V1_Vdp2YmirNbg0ScanlineCaptureInput input;
    Nexus_V1_Vdp2YmirNbg0ScanlineCaptureReceipt receipt;
    const Nexus_V1_Vdp2YmirNbg0RowState rows[] = {
        {0U, 224U, 0x1f79U, 0x0bcdU, 0x00cdU, 0x00cdU}
    };
    FILE *out = NULL; size_t p; int rc = 1;
    if (!root || !*root) {
        puts("SKIP: set FIRESTAFF_NEXUS_NBG0_LIVE_CAPTURE_DIR");
        return 77;
    }
    fbs = read_capture_file(root, "dumps/vdp1-fbs.bin");
    vram = read_capture_file(root, "dumps/vdp2-vram.bin");
    cram = read_capture_file(root, "dumps/vdp2-cram.bin");
    regs = read_capture_file(root, "dumps/vdp2-registers.bin");
    fb = (Nexus_V1_Vdp2YmirMenuCaptureFramebuffer *)malloc(sizeof(*fb));
    if (!fbs.data || fbs.size != 524288U || !vram.data || vram.size != 524288U || !cram.data || cram.size != 4096U || !regs.data || regs.size != 512U || !fb ||
        !hash_is(&fbs, "0570c79b067adde55161ba2e16c09a628dcdf99b7b49f168d803c948b4479420") ||
        !hash_is(&vram, "2b6fe0392b1ff23c0c55f08423d179a74d7f481990e37aadd4e6bfd657d9cf45") ||
        !hash_is(&cram, "db282593790dec4a2bf8f76293c0458a1a140adc3351650bd56a417f0359e0de") ||
        !hash_is(&regs, "2d3ed242492f06aa8bac0aa90acfc823c488a6d07ec76d5932882233e80d2ff4") ||
        !hash_is(&regs, "2d3ed242492f06aa8bac0aa90acfc823c488a6d07ec76d5932882233e80d2ff4")) {
        fprintf(stderr, "FAIL: authenticated NBG0 live-row capture size/hash\n"); goto done;
    }
    memset(&input, 0, sizeof(input));
    input.vdp1_display_framebuffer = fbs.data + 512U*256U*2U;
    input.vdp1_display_framebuffer_size = 512U*256U*2U;
    input.vdp2_vram = vram.data; input.vdp2_vram_size = vram.size;
    input.vdp2_cram = cram.data; input.vdp2_cram_size = cram.size;
    input.vdp2_registers = regs.data; input.vdp2_registers_size = regs.size;
    input.register_byte_order = NEXUS_V1_SATURN_VDP2_REGISTER_ORDER_BIG;
    input.row_states = rows; input.row_state_count = sizeof(rows)/sizeof(rows[0]);
    input.original_ymir_capture_verified = 1; input.row_states_verified = 1;
    memset(&receipt, 0, sizeof(receipt));
    if (nexus_v1_vdp2_capture_replay_ymir_nbg0_scanlines(fb, &input, &receipt) || receipt.valid) {
        fprintf(stderr, "FAIL: scanline replay accepted an unattested enabled NBG1 layer\n");
        goto done;
    }
    input.nbg1_transparency_verified = 1;
    if (!nexus_v1_vdp2_capture_replay_ymir_nbg0_scanlines(fb, &input, &receipt) || !receipt.valid || !receipt.capture_only || receipt.renderer_permitted || !receipt.nbg1_transparency_verified || receipt.uncovered_rows != 0 || receipt.nbg0_pixels != 21455 || receipt.vdp1_pixels != 50225) {
        fprintf(stderr, "FAIL: live-row replay receipt nbg0=%d vdp1=%d uncovered=%d\n", receipt.nbg0_pixels, receipt.vdp1_pixels, receipt.uncovered_rows); goto done;
    }
    if (!replay_path || !(out = fopen(replay_path, "wb"))) { fprintf(stderr, "FAIL: replay output path\n"); goto done; }
    for (p = 0; p < 320U*224U; ++p) {
        uint32_t c = fb->rgba_buffer[p]; uint8_t rgb[3] = {(uint8_t)(c>>16U), (uint8_t)(c>>8U), (uint8_t)c};
        if (fwrite(rgb, 1U, 3U, out) != 3U) { fclose(out); out = NULL; goto done; }
    }
    if (fclose(out)) { out = NULL; goto done; } out = NULL;
    printf("PASS: live NBG0 rows composed: NBG0=%d VDP1=%d rows=0..223; capture-only\n", receipt.nbg0_pixels, receipt.vdp1_pixels);
    rc = 0;
done:
    if (out) fclose(out);
    free(fb); free(fbs.data); free(vram.data); free(cram.data); free(regs.data);
    return rc;
}
