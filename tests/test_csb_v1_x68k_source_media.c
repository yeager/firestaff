#include "csb_v1_x68k_source_media.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    const uint8_t invalid[64] = {0u};
    CSB_V1_X68kSourceMediaReceipt receipt;

    if (csb_v1_x68k_hdm_source_media_receipt(
            NULL, 0u, &receipt) != CSB_V1_X68K_SOURCE_MEDIA_ERR_ARGUMENT ||
        csb_v1_x68k_hdm_source_media_receipt(
            invalid, sizeof(invalid), &receipt) != CSB_V1_X68K_SOURCE_MEDIA_ERR_HDM)
        return 1;
    if (!path || !path[0]) {
        puts("test_csb_v1_x68k_source_media: SKIP FIRESTAFF_CSB_X68K_HDM unset");
        return 0;
    }
    {
        uint8_t *hdm = NULL;
        size_t hdm_size = 0u;
        if (!read_file(path, &hdm, &hdm_size) ||
            csb_v1_x68k_hdm_source_media_receipt(hdm, hdm_size, &receipt) !=
                CSB_V1_X68K_SOURCE_MEDIA_OK ||
            strcmp(receipt.hdm_sha256,
                   "e912addf1881b6c2b3cde4207507061a43459748082c75953cbc3c305fdf24e1") != 0 ||
            receipt.graphics.graphics_byte_count != 373583u ||
            receipt.graphics.item_count != 732u ||
            receipt.dungeon_level_count != 2u || receipt.dungeon_square_bytes != 1u ||
            receipt.initial_party_level != 0 || receipt.initial_party_x != 9 ||
            receipt.initial_party_y != 0 || receipt.initial_party_direction != 2 ||
            receipt.entrance_music.track_count != 9u ||
            receipt.entrance_music.event_count != 3205u ||
            receipt.autoexec.command_count != 3u ||
            strcmp(receipt.autoexec.commands[0], "CK") != 0 ||
            strcmp(receipt.autoexec.commands[1], "VIDSET") != 0 ||
            strcmp(receipt.autoexec.commands[2], "CHAOS_STRIKES_BACK") != 0 ||
            receipt.program.text_bytes != 9020u || receipt.program.entry_offset != 64u ||
            !receipt.x68000_identity_bound || !receipt.shared_graphics_layout_only ||
            receipt.authenticity_claimed || receipt.native_runtime_launch_permitted) {
            free(hdm);
            return 1;
        }
        free(hdm);
    }
    puts("test_csb_v1_x68k_source_media: PASS");
    return 0;
}
