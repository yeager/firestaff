/* DM2 save: DM2_STORE_EXTRA_DUNGEON_DATA tile-header SUPPRESS encoding.
 * Source: sksvgame.cpp:1958-2041.
 *
 * The full function walks every map, every tile, writes a SUPPRESS-encoded
 * tile header, then calls DM2_WRITE_RECORD_CHECKCODE for the tile's thing
 * chain. This module implements the tile-header portion; the record-chain
 * walk requires the full record addressing system and is deferred. */

#include "dm2_v1_save_dungeon_data_pc34_compat.h"

int dm2_v1_save_tile_suppress_size(uint8_t tile_type_3bit)
{
    switch (tile_type_3bit & 0x07) {
    case 0: return 0;
    case 1: return 0;
    case 2: return 8;
    case 3: return 0;
    case 4: return 7;
    case 5: return 8;
    case 6: return 4;
    case 7: return 0;
    default: return 0;
    }
}

int dm2_v1_save_teleporter_is_forward_ref(int current_map, int target_map)
{
    return (target_map >= 0 && current_map >= 0 && target_map < current_map)
           ? 0 : (target_map > current_map ? 1 : 0);
}
