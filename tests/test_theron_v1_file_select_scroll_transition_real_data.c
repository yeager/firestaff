#include "theron_v1_raw_loader_trace.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned char *read_file(const char *path, size_t *size)
{
    FILE *file = path ? fopen(path, "rb") : NULL;
    long length;
    unsigned char *bytes;
    *size = 0u;
    if (!file || fseek(file, 0, SEEK_END) != 0 ||
        (length = ftell(file)) <= 0 || fseek(file, 0, SEEK_SET) != 0) {
        if (file) fclose(file);
        return NULL;
    }
    bytes = (unsigned char *)malloc((size_t)length + 1u);
    if (!bytes || fread(bytes, 1u, (size_t)length, file) != (size_t)length) {
        free(bytes); fclose(file); return NULL;
    }
    fclose(file);
    bytes[length] = 0;
    *size = (size_t)length;
    return bytes;
}

int main(void)
{
    const char *paths[9] = {
        getenv("FIRESTAFF_THERON_FILE_SELECT_VDC_TRACE"),
        getenv("FIRESTAFF_THERON_FILE_SELECT_PRE_VDC_STATE"),
        getenv("FIRESTAFF_THERON_FILE_SELECT_PRE_VRAM"),
        getenv("FIRESTAFF_THERON_FILE_SELECT_PRE_VCE"),
        getenv("FIRESTAFF_THERON_FILE_SELECT_PRE_SAT"),
        getenv("FIRESTAFF_THERON_FILE_SELECT_POST_VDC_STATE"),
        getenv("FIRESTAFF_THERON_FILE_SELECT_POST_VRAM"),
        getenv("FIRESTAFF_THERON_FILE_SELECT_POST_VCE"),
        getenv("FIRESTAFF_THERON_FILE_SELECT_POST_SAT")
    };
    unsigned char *data[9] = {0};
    size_t sizes[9] = {0}, i;
    Theron_V1RawLoaderTraceFileSelectEncodedTransportReceipt transport;
    Theron_V1RawLoaderTraceFileSelectScrollReceipt receipt;

    for (i = 0u; i < 9u; ++i) if (!paths[i]) {
        puts("SKIP: authentic file-select scroll inputs are not configured");
        return 77;
    }
    for (i = 0u; i < 9u; ++i) if (!(data[i] = read_file(paths[i], &sizes[i]))) {
        fputs("FAIL: could not read file-select scroll inputs\n", stderr);
        while (i) free(data[--i]);
        return 1;
    }
    memset(&transport, 0, sizeof(transport));
    transport.valid = 1;
    transport.variant = THERON_TRACK02_VARIANT_US_BIN;
    memcpy(transport.track02_md5, THERON_TRACK02_MD5_US_BIN, 33u);
    transport.presentation_frame = 8580u;
    transport.destination_to_vdc_verified = 1;
    transport.vdc_destination_replay_verified = 1;
    if (!theron_v1_raw_loader_trace_bind_file_select_scroll_transition(
            &transport, (const char *)data[0], (const char *)data[1],
            data[2], sizes[2], data[3], sizes[3], data[4], sizes[4],
            (const char *)data[5], data[6], sizes[6], data[7], sizes[7],
            data[8], sizes[8], &receipt) || !receipt.valid ||
        receipt.pre_frame != 8579u || receipt.update_frame != 8580u ||
        receipt.post_frame != 8581u || receipt.pre_byr != 0x00e9u ||
        receipt.post_byr != 0x00e8u || receipt.register_select_pc != 0x4993u ||
        receipt.low_byte_writer_pc != 0x4999u ||
        receipt.high_byte_writer_pc != 0x499fu ||
        receipt.vram_checksum != 0x832b4d13u ||
        receipt.vce_checksum != 0x5376a91bu ||
        receipt.sat_checksum != 0xa8007f15u ||
        receipt.pre_source_checksum != 0xaf183e0du ||
        receipt.post_source_checksum != 0x7622aee1u ||
        receipt.pre_color_checksum != 0x68fe4a69u ||
        receipt.post_color_checksum != 0x8f1cf573u ||
        receipt.pre_nonzero_pixels != 43047u ||
        receipt.post_nonzero_pixels != 43087u ||
        receipt.sprite_pixels != 24576u || receipt.changed_pixels != 12644u ||
        receipt.changed_sprite_pixels != 0u ||
        receipt.changed_min_x != 32u || receipt.changed_max_x != 223u ||
        receipt.changed_min_y != 64u || receipt.changed_max_y != 175u ||
        !receipt.frame_end_markers_verified ||
        !receipt.graphics_snapshots_identical ||
        !receipt.game_byr_write_verified || !receipt.composition_delta_verified ||
        !receipt.sprite_mask_static_verified || receipt.screen_semantics_proven) {
        fputs("FAIL: authentic file-select scroll transition rejected\n", stderr);
        for (i = 0u; i < 9u; ++i) free(data[i]);
        return 1;
    }
    data[2][0] ^= 1u;
    if (theron_v1_raw_loader_trace_bind_file_select_scroll_transition(
            &transport, (const char *)data[0], (const char *)data[1],
            data[2], sizes[2], data[3], sizes[3], data[4], sizes[4],
            (const char *)data[5], data[6], sizes[6], data[7], sizes[7],
            data[8], sizes[8], &receipt)) {
        fputs("FAIL: corrupted pre-scroll VRAM accepted\n", stderr);
        for (i = 0u; i < 9u; ++i) free(data[i]);
        return 1;
    }
    for (i = 0u; i < 9u; ++i) free(data[i]);
    puts("PASS: authentic file-select BYR scroll transition is bound");
    return 0;
}
