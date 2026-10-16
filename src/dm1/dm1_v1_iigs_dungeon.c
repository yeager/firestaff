#include "dm1_v1_iigs_dungeon.h"
#include "dungeon_decompressor_ftl.h"
#include "memory_dungeon_dat_pc34_compat.h"

#include <string.h>

static uint16_t iigs_world_u16(const uint8_t *p) {
    return (uint16_t)((uint16_t)p[0] | ((uint16_t)p[1] << 8));
}
static uint32_t iigs_world_u32(const uint8_t *p) {
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

int dm1_v1_iigs_dungeon_query(const uint8_t *data, size_t size,
                             size_t *out_size) {
    uint32_t declared;
    if (out_size) *out_size=0;
    /* ReDMCSB DEFS.H:981–998 COMPRESSED_DUNGEON_HEADER/DUNGEON_HEADER;
     * LOADSAVE.C F0434:1853–1923, GS MEDIA388 reads the 8-byte native LE
     * wrapper and invokes DECOMPDU.C F0455 on the following frequency table.
     * Minimum payload is the 20-byte F0455 table; minimum decoded structure
     * is header + one map descriptor + checksum. */
    if (!data || !out_size || size<28U ||
        size>DM1_V1_IIGS_DUNGEON_MAX_BYTES ||
        iigs_world_u16(data)!=0x8104U) return 0;
    declared=iigs_world_u32(data+2U);
    if (declared<DUNGEON_HEADER_SIZE+DUNGEON_MAP_DESC_SIZE+2U ||
        declared>DM1_V1_IIGS_DUNGEON_MAX_BYTES) return 0;
    *out_size=declared;
    return 1;
}

int dm1_v1_iigs_dungeon_decode(const uint8_t *data, size_t size,
    uint8_t *out, size_t capacity, size_t *out_size,
    DM1_V1_IigsDungeonReceipt *receipt) {
    struct DungeonDatState_Compat state;
    struct DungeonThings_Compat things;
    DM1_V1_IigsDungeonReceipt result;
    size_t decoded;
    uintptr_t input_address=(uintptr_t)data, output_address=(uintptr_t)out;
    int valid=0;
    if (out_size) *out_size=0;
    if (receipt) memset(receipt,0,sizeof(*receipt));
    if (!out || !out_size || !dm1_v1_iigs_dungeon_query(data,size,&decoded) ||
        capacity<decoded) return 0;
    if ((output_address>=input_address && output_address-input_address<size) ||
        (input_address>output_address && input_address-output_address<decoded)) return 0;
    if (!ftl_decompress_dungeon(data+8U,size-8U,out,(long)decoded)) return 0;
    memset(&state,0,sizeof(state)); memset(&things,0,sizeof(things));
    /* ReDMCSB LOADSAVE.C F0434:1937–2088 reads header, maps, column index,
     * square heads, text, all Thing arrays, raw map bytes, then checksum.
     * READWRIT.C F0421:265–293 sums unsigned bytes modulo 65536. Reuse the
     * shared bounded LE body parser rather than another section/Thing table.
     * Native GS bitfields use this LE layout (DEFS.H MAP:1046–1116).
     * The two original GS GAME/BONUS bodies include the checksum; this API
     * deliberately requires it, although F0434 also tolerated absent EOF
     * trailers. No generic/save checksum policy is changed here. */
    if (!F0504_DUNGEON_LoadTailBuffer_Compat(out,(int)decoded,&state,&things)) goto cleanup;
    memset(&result,0,sizeof(result));
    result.decoded_size=decoded;
    result.dungeon_id=iigs_world_u16(data+6U);
    result.map_count=state.header.mapCount;
    result.start_map_index=0;
    result.start_x=(uint8_t)(state.header.initialPartyLocation&31U);
    result.start_y=(uint8_t)((state.header.initialPartyLocation>>5)&31U);
    result.start_direction=(uint8_t)((state.header.initialPartyLocation>>10)&3U);
    result.first_map_width=state.maps[0].width;
    result.first_map_height=state.maps[0].height;
    if (result.start_x>=result.first_map_width || result.start_y>=result.first_map_height)
        goto cleanup;
    result.raw_map_data_byte_count=state.header.rawMapDataByteCount;
    result.text_data_word_count=state.header.textDataWordCount;
    result.square_first_thing_count=state.header.squareFirstThingCount;
    result.column_count=state.dungeonColumnCount;
    if (receipt) *receipt=result;
    *out_size=decoded;
    valid=1;
cleanup:
    F0504_DUNGEON_FreeThingData_Compat(&things);
    F0500_DUNGEON_FreeDatHeader_Compat(&state);
    return valid;
}
