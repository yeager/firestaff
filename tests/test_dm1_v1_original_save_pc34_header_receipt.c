#include "dm1_v1_original_save_pc34_handoff.h"

#include <stdio.h>
#include <string.h>

int main(void)
{
    unsigned char bytes[65536];
    size_t size = 0u;
    DM1OriginalSavePC34FixtureSpec spec;
    Dm1V1OriginalSavePc34HeaderDecodeReceipt receipt;

    memset(&spec, 0, sizeof(spec));
    spec.maximum_active_group_count = 1;
    spec.event_maximum_count = 1;
    spec.game_id = 0x44334d31u;
    if (dm1_v1_original_save_pc34_build_handoff_fixture_bytes(
            &spec, bytes, sizeof(bytes), &size) != SAVEGAME_OK ||
        !dm1_v1_original_save_pc34_decode_header_receipt(
            bytes, size, &receipt) || !receipt.header_size_ok ||
        !receipt.decoded || !receipt.checksum_ok) {
        return 1;
    }
    if (receipt.platform != SAVEGAME_PC34_PLATFORM_PC ||
        receipt.dungeon_id != SAVEGAME_PC34_DUNGEON_ID_DM) {
        return 1;
    }
    bytes[0] ^= 1u;
    if (dm1_v1_original_save_pc34_decode_header_receipt(
            bytes, size, &receipt) || receipt.checksum_ok) {
        return 1;
    }
    puts("ok: PC34 F0429 header receipt accepts fixture and rejects checksum corruption");
    return 0;
}
