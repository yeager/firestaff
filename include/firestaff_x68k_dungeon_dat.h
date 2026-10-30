/* Read-only metadata reader for raw DM1 X68000 (X30J) DUNGEON.DAT. */
#ifndef FIRESTAFF_X68K_DUNGEON_DAT_H
#define FIRESTAFF_X68K_DUNGEON_DAT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define FIRESTAFF_X68K_DUNGEON_MAX_MAPS 16u
#define FIRESTAFF_X68K_DUNGEON_THING_TYPES 16u
#define FIRESTAFF_X68K_DUNGEON_THING_END_OF_LIST 0xfffeu

typedef struct {
    uint8_t type;
    uint8_t flags;
} FirestaffX68kDungeonSquare;

typedef struct {
    size_t offset;
    size_t size;
} FirestaffX68kDungeonDatSpan;

typedef struct {
    uint16_t raw_map_offset;
    uint8_t offset_map_x;
    uint8_t offset_map_y;
    uint8_t level;
    uint8_t width;
    uint8_t height;
    uint8_t allowed_creature_type_count;
    uint8_t wall_ornament_count;
    uint8_t floor_ornament_count;
    uint8_t door_ornament_count;
    size_t raw_map_span_size;
} FirestaffX68kDungeonDatMap;

typedef struct {
    const uint8_t *bytes;
    size_t size;
    uint16_t ornament_random_seed;
    uint16_t raw_map_byte_count;
    uint8_t map_count;
    uint16_t text_data_word_count;
    uint16_t initial_party_location;
    uint16_t square_first_thing_count;
    uint16_t thing_count[FIRESTAFF_X68K_DUNGEON_THING_TYPES];
    FirestaffX68kDungeonDatMap maps[FIRESTAFF_X68K_DUNGEON_MAX_MAPS];
    uint16_t column_count;
    FirestaffX68kDungeonDatSpan map_descriptors;
    FirestaffX68kDungeonDatSpan column_sft_counts;
    FirestaffX68kDungeonDatSpan square_first_things;
    FirestaffX68kDungeonDatSpan text_data;
    FirestaffX68kDungeonDatSpan thing_data[FIRESTAFF_X68K_DUNGEON_THING_TYPES];
    FirestaffX68kDungeonDatSpan raw_map_data;
    uint8_t has_trailing_checksum;
    uint16_t trailing_checksum;
} FirestaffX68kDungeonDat;

typedef enum {
    FIRESTAFF_X68K_DUNGEON_MAP_CREATURE_TYPE = 0,
    FIRESTAFF_X68K_DUNGEON_MAP_WALL_ORNAMENT = 1,
    FIRESTAFF_X68K_DUNGEON_MAP_FLOOR_ORNAMENT = 2,
    FIRESTAFF_X68K_DUNGEON_MAP_DOOR_ORNAMENT = 3
} FirestaffX68kDungeonDatMapMetadataKind;

/*
 * Opens only the raw, uncompressed X30J file. An optional 0x8104 compressed
 * envelope is rejected; callers must not pass it to a different-platform
 * decoder. If present, the source-defined trailing byte-sum checksum is
 * validated. All returned spans are offsets into the caller-owned input.
 */
int FirestaffX68kDungeonDat_OpenRaw(const uint8_t *bytes,
                                    size_t size,
                                    FirestaffX68kDungeonDat *out);

/*
 * Read the native column-major square byte. ReDMCSB LOADSAVE.C:2161-2163
 * advances each column by Height+1; DUNGEON.C F0151 returns that byte for
 * in-bounds coordinates.
 */
int FirestaffX68kDungeonDat_GetSquare(const FirestaffX68kDungeonDat *dungeon,
                                      uint8_t map_index,
                                      uint8_t map_x,
                                      uint8_t map_y,
                                      uint8_t *out_square);

/* Decode the source's upper-three-bit square type and lower five-bit flags.
 * Flag meanings depend on the square type (ReDMCSB DEFS.H:1001-1036).
 */
int FirestaffX68kDungeonDat_DecodeSquare(uint8_t raw_square,
                                         FirestaffX68kDungeonSquare *out_square);

/* Resolve the first THING word for a map square. Returns 0 on success,
 * including empty squares (0xfffe); returns -1 for invalid coordinates/data.
 * ReDMCSB DUNGEON.C F0160/F0161 and LOADSAVE.C column-count setup.
 */
int FirestaffX68kDungeonDat_GetSquareFirstThing(
    const FirestaffX68kDungeonDat *dungeon,
    uint8_t map_index,
    uint8_t map_x,
    uint8_t map_y,
    uint16_t *out_thing);

/* Read map-local trailing metadata in ReDMCSB F0174 order: creature types,
 * wall ornaments, floor ornaments, then door ornaments (DUNGEON.C:2755-2760).
 */
int FirestaffX68kDungeonDat_GetMapMetadata(
    const FirestaffX68kDungeonDat *dungeon,
    uint8_t map_index,
    FirestaffX68kDungeonDatMapMetadataKind kind,
    uint8_t item_index,
    uint8_t *out_value);

/* Decode the X30J DUNGEON_HEADER.InitialPartyLocation bit fields. */
int FirestaffX68kDungeonDat_GetInitialPartyLocation(
    const FirestaffX68kDungeonDat *dungeon,
    uint8_t *out_map_x,
    uint8_t *out_map_y,
    uint8_t *out_direction);

#ifdef __cplusplus
}
#endif

#endif
