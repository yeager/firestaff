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
    const char *paths[4] = {
        getenv("FIRESTAFF_THERON_FILE_SELECT_VDC_STATE"),
        getenv("FIRESTAFF_THERON_FILE_SELECT_VRAM"),
        getenv("FIRESTAFF_THERON_FILE_SELECT_VCE"),
        getenv("FIRESTAFF_THERON_FILE_SELECT_SAT")
    };
    unsigned char *data[4] = {0};
    size_t sizes[4] = {0}, i;
    Theron_V1RawLoaderTraceFileSelectEncodedTransportReceipt transport;
    Theron_V1RawLoaderTraceFileSelectSatFrameReceipt receipt;

    for (i = 0u; i < 4u; ++i) if (!paths[i]) {
        puts("SKIP: authentic file-select SAT frame inputs are not configured");
        return 77;
    }
    for (i = 0u; i < 4u; ++i) if (!(data[i] = read_file(paths[i], &sizes[i]))) {
        fputs("FAIL: could not read file-select SAT frame inputs\n", stderr);
        while (i) free(data[--i]);
        return 1;
    }
    memset(&transport, 0, sizeof(transport));
    transport.valid = 1;
    transport.variant = THERON_TRACK02_VARIANT_US_BIN;
    memcpy(transport.track02_md5, THERON_TRACK02_MD5_US_BIN, 33u);
    transport.presentation_frame = 8580u;
    transport.vdc_payload_checksum = 0xa8007f15u;
    transport.first_vram_word = 0x0800u;
    transport.last_vram_word = 0x08ffu;
    transport.destination_to_vdc_verified = 1;
    transport.vdc_destination_replay_verified = 1;
    if (!theron_v1_raw_loader_trace_bind_file_select_sat_frame(
            &transport, (const char *)data[0], data[1], sizes[1],
            data[2], sizes[2], data[3], sizes[3], &receipt) ||
        !receipt.valid || receipt.frame != 8580u ||
        receipt.first_vram_word != 0x0800u ||
        receipt.last_vram_word != 0x08ffu || receipt.byte_count != 512u ||
        receipt.staging_checksum != 0xa8007f15u ||
        receipt.vram_checksum != 0x832b4d13u ||
        receipt.vce_checksum != 0x5376a91bu ||
        receipt.sat_checksum != 0xa8007f15u ||
        receipt.nonzero_sat_entries != 18u ||
        receipt.visible_sat_entries != 12u ||
        receipt.first_sprite_pattern != 0x0108u ||
        receipt.last_sprite_pattern != 0x010fu ||
        receipt.sprite_pattern_bytes != 1024u ||
        receipt.sprite_pattern_checksum != 0xba5526c5u ||
        receipt.bat_reference_count != 0u ||
        receipt.frame_pixels != 61440u ||
        receipt.nonzero_frame_pixels != 43087u ||
        receipt.background_source_pixels != 18511u ||
        receipt.sprite_source_pixels != 24576u ||
        receipt.unique_source_indices != 40u ||
        receipt.frame_source_checksum != 0x7622aee1u ||
        receipt.frame_color_checksum != 0x8f1cf573u ||
        receipt.sprite_source_index != 0x0101u ||
        receipt.sprite_color != 0x0000u ||
        receipt.sprite_min_x != 32u || receipt.sprite_max_x != 223u ||
        receipt.sprite_min_y != 0u || receipt.sprite_max_y != 239u ||
        receipt.byr != 0x00e8u ||
        receipt.mwr != 0x0010u || receipt.cr != 0x00c8u ||
        !receipt.atomic_snapshot_verified ||
        !receipt.vram_sat_identity_verified ||
        !receipt.sat_record_layout_verified ||
        !receipt.sprite_pattern_range_verified ||
        !receipt.frame_composition_verified ||
        !receipt.vce_color_composition_verified ||
        receipt.sprite_or_screen_semantics_proven) {
        fputs("FAIL: authentic file-select SAT frame rejected\n", stderr);
        for (i = 0u; i < 4u; ++i) free(data[i]);
        return 1;
    }
    data[3][0] ^= 1u;
    if (theron_v1_raw_loader_trace_bind_file_select_sat_frame(
            &transport, (const char *)data[0], data[1], sizes[1],
            data[2], sizes[2], data[3], sizes[3], &receipt)) {
        fputs("FAIL: corrupted file-select SAT accepted\n", stderr);
        for (i = 0u; i < 4u; ++i) free(data[i]);
        return 1;
    }
    data[3][0] ^= 1u;
    data[2][514] ^= 1u;
    if (theron_v1_raw_loader_trace_bind_file_select_sat_frame(
            &transport, (const char *)data[0], data[1], sizes[1],
            data[2], sizes[2], data[3], sizes[3], &receipt)) {
        fputs("FAIL: corrupted file-select VCE accepted\n", stderr);
        for (i = 0u; i < 4u; ++i) free(data[i]);
        return 1;
    }
    for (i = 0u; i < 4u; ++i) free(data[i]);
    puts("PASS: authentic file-select VRAM staging is SAT-bound");
    return 0;
}
