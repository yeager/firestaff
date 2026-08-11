#include "csb_v1_x68k_graphics_handoff.h"

#include <stdio.h>
#include <stdlib.h>

static int read_file(const char *path, uint8_t **out, size_t *out_size) {
    FILE *file = NULL;
    long length;
    uint8_t *bytes = NULL;
    if (!path || !out || !out_size || !(file = fopen(path, "rb")) ||
        fseek(file, 0, SEEK_END) != 0 || (length = ftell(file)) <= 0 ||
        fseek(file, 0, SEEK_SET) != 0 || !(bytes = malloc((size_t)length)) ||
        fread(bytes, 1u, (size_t)length, file) != (size_t)length) {
        if (file) fclose(file);
        free(bytes);
        return 0;
    }
    fclose(file);
    *out = bytes;
    *out_size = (size_t)length;
    return 1;
}

int main(void) {
    const char *path = getenv("FIRESTAFF_CSB_X68K_HDM");
    const uint8_t invalid[4] = {0u, 0u, 0u, 0u};
    CSB_V1_X68kGraphicsReceipt receipt;
    CSB_V1_X68kGraphicsItem first;
    uint8_t pixels[640u * 400u];
    uint16_t width = 0u;
    uint16_t height = 0u;
    uint16_t item13_width = 0u;
    uint16_t item13_height = 0u;

    if (csb_v1_x68k_hdm_graphics_receipt(invalid, sizeof(invalid), &receipt) ||
        csb_v1_x68k_hdm_graphics_item(invalid, sizeof(invalid), 0u, &first) ||
        csb_v1_x68k_hdm_graphics_decode_item(invalid, sizeof(invalid), 0u,
                                              pixels, sizeof(pixels), &width,
                                              &height)) {
        puts("test_csb_v1_x68k_graphics_handoff: invalid media accepted");
        return 1;
    }
    if (!path || !path[0]) {
        puts("test_csb_v1_x68k_graphics_handoff: SKIP FIRESTAFF_CSB_X68K_HDM unset");
        return 0;
    }
    {
        uint8_t *hdm = NULL;
        size_t hdm_size = 0u;
        if (!read_file(path, &hdm, &hdm_size) ||
            !csb_v1_x68k_hdm_graphics_receipt(hdm, hdm_size, &receipt) ||
            receipt.graphics_byte_count != 373583u || receipt.item_count != 732u ||
            receipt.direct_item_count != 732u ||
            !csb_v1_x68k_hdm_graphics_item(hdm, hdm_size, 0u, &first) ||
            first.data_offset != 5860u || first.stored_byte_count == 0u ||
            first.stored_byte_count != first.decoded_byte_count ||
            csb_v1_x68k_hdm_graphics_item(hdm, hdm_size, 732u, &first) ||
            !csb_v1_x68k_hdm_graphics_decode_item(hdm, hdm_size, 13u,
                                                   pixels, sizeof(pixels),
                                                   &width, &height) ||
            width != 96u || height != 41u) {
            free(hdm);
            puts("test_csb_v1_x68k_graphics_handoff: original media mismatch");
            return 1;
        }
        item13_width = width;
        item13_height = height;
        if (csb_v1_x68k_hdm_graphics_decode_item(hdm, hdm_size, 13u,
                                                  pixels, 0u, &width,
                                                  &height)) {
            free(hdm);
            puts("test_csb_v1_x68k_graphics_handoff: undersized output accepted");
            return 1;
        }
        free(hdm);
    }
    if (item13_width != 96u || item13_height != 41u) {
        puts("test_csb_v1_x68k_graphics_handoff: item13 receipt changed");
        return 1;
    }
    puts("test_csb_v1_x68k_graphics_handoff: PASS item13=96x41");
    return 0;
}
