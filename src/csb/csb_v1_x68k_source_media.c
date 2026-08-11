#include "csb_v1_x68k_source_media.h"

#include "firestaff_x68k_media_receipt.h"

#include <string.h>

int csb_v1_x68k_hdm_source_media_receipt(
    const uint8_t *hdm, size_t hdm_size,
    CSB_V1_X68kSourceMediaReceipt *out_receipt) {
    CSB_V1_X68kSourceMediaReceipt receipt;
    CSB_V1_DungeonData dungeon;

    if (!out_receipt) return CSB_V1_X68K_SOURCE_MEDIA_ERR_ARGUMENT;
    memset(out_receipt, 0, sizeof(*out_receipt));
    memset(&receipt, 0, sizeof(receipt));
    memset(&dungeon, 0, sizeof(dungeon));
    if (!hdm) return CSB_V1_X68K_SOURCE_MEDIA_ERR_ARGUMENT;
    if (!csb_v1_x68k_hdm_probe(hdm, hdm_size, &receipt.media))
        return CSB_V1_X68K_SOURCE_MEDIA_ERR_HDM;
    if (!csb_v1_x68k_hdm_graphics_receipt(hdm, hdm_size, &receipt.graphics))
        return CSB_V1_X68K_SOURCE_MEDIA_ERR_GRAPHICS;
    if (csb_v1_x68k_hdm_load_dungeon(&dungeon, hdm, hdm_size, NULL) !=
        CSB_V1_X68K_DUNGEON_HANDOFF_OK)
        return CSB_V1_X68K_SOURCE_MEDIA_ERR_DUNGEON;
    receipt.dungeon_level_count = (uint16_t)dungeon.level_count;
    receipt.dungeon_square_bytes = (uint16_t)dungeon.square_bytes;
    if (!csb_v1_dungeon_initial_party_pose_pc34(
            &dungeon, &receipt.initial_party_level, &receipt.initial_party_x,
            &receipt.initial_party_y, &receipt.initial_party_direction)) {
        csb_v1_dungeon_free(&dungeon);
        return CSB_V1_X68K_SOURCE_MEDIA_ERR_DUNGEON;
    }
    csb_v1_dungeon_free(&dungeon);
    if (!csb_v1_x68k_enter_sng_probe_hdm(hdm, hdm_size,
                                          &receipt.entrance_music))
        return CSB_V1_X68K_SOURCE_MEDIA_ERR_ENTRANCE_MUSIC;
    if (firestaff_x68k_media_receipt_sha256_hex(
            hdm, hdm_size, receipt.hdm_sha256, sizeof(receipt.hdm_sha256)) != 0)
        return CSB_V1_X68K_SOURCE_MEDIA_ERR_HASH;
    receipt.x68000_identity_bound = 1;
    receipt.shared_graphics_layout_only = 1;
    receipt.authenticity_claimed = 0;
    receipt.native_runtime_launch_permitted = 0;
    *out_receipt = receipt;
    return CSB_V1_X68K_SOURCE_MEDIA_OK;
}
