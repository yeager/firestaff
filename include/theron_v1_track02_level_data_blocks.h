#ifndef THERON_V1_TRACK02_LEVEL_DATA_BLOCKS_H
#define THERON_V1_TRACK02_LEVEL_DATA_BLOCKS_H

#include <stdint.h>
#include <stddef.h>
#include "theron_v1_track02.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Sources: US Track 02 BIN (MD5 f23601102138f87c33025877767ebf76), JP
 * Track 02 BIN (MD5 b7afb338ad31be1025b53f9aff12d73a), and the direct ISO
 * projections TQUS19.iso/TQJP19.iso (hash-gated by their media receipts).
 *
 * 7 per-level graphics/tile banks (~256KB each = one PCE HuCard bank).
 * Identified by byte-exact 232-byte shared prologue signature.
 * Each block starts with a 0xF0-byte prologue (0xE8 bytes shared resource
 * table + 8 bytes per-level metadata), followed by level-specific tile
 * pattern data for the PCE VDC (4bpp 8x8 tiles).
 *
 * These are NOT the dungeon map layout (wall/floor topology) — that data
 * format is still unidentified within Track 02.
 *
 * UD offsets discovered via full-track signature scan. Direct ISO projection
 * offsets are the same authenticated spans after the 225-sector pregap is
 * removed; they are separate descriptors because the final level span and
 * container identity differ. */

#define THERON_TRACK02_LEVEL_COUNT  7u
#define THERON_TRACK02_LEVEL_PROLOGUE_SIZE  0xF0u
#define THERON_TRACK02_LEVEL_SHARED_PROLOGUE_SIZE  0xE8u
#define THERON_TRACK02_LEVEL_RESOURCE_HEADER_SIZE  6u

typedef struct {
    uint32_t ud_offset;
    uint8_t  per_level_meta[8];
} Theron_LevelDataBlockDesc;

/* Exact later-level byte window from the authenticated user-data image. The
 * compressed payload is borrowed from the caller's buffer; its byte-level
 * HuC6280 lift is available below, while level/object/tile meaning remains
 * runtime-capture gated. */
typedef struct {
    int valid;
    Theron_Track02Variant variant;
    unsigned int level;
    uint32_t block_ud_offset;
    uint32_t compressed_ud_offset;
    uint32_t resource_end_ud_offset;
    size_t compressed_bytes;
    uint16_t resource_length;
    size_t resource_bitstream_bytes;
    uint32_t compressed_fnv1a;
    uint32_t shared_prologue_fnv1a;
    uint8_t resource_header[THERON_TRACK02_LEVEL_RESOURCE_HEADER_SIZE];
    int resource_header_verified;
    uint8_t per_level_meta[8];
    const uint8_t *compressed;
    const uint8_t *resource_bitstream;
} Theron_LevelDataBlockReceipt;

/* The retail helper is a HuC6280 variable-width bit reader plus a pointer
 * table of output addresses.  The stage-2 handler supplies the HuC6280 MPR
 * mappings around that helper; this logical lift deliberately takes the
 * already-flattened source resource and destination window as arguments.
 * It therefore decodes bytes without inventing a level/map/tile meaning. */
typedef enum {
    THERON_HUC6280_DECODE_UNAVAILABLE = 0,
    THERON_HUC6280_DECODE_READY = 1,
    THERON_HUC6280_DECODE_INVALID_ARGUMENT = 2,
    THERON_HUC6280_DECODE_TRUNCATED = 3,
    THERON_HUC6280_DECODE_POINTER_TABLE = 4,
    THERON_HUC6280_DECODE_DESTINATION = 5,
    THERON_HUC6280_DECODE_UNSUPPORTED = 6
} Theron_Huc6280DecodeStatus;

typedef struct {
    Theron_Huc6280DecodeStatus status;
    uint16_t resource_length;
    size_t resource_bitstream_bytes;
    size_t bits_consumed;
    size_t output_bytes;
    size_t pointer_entries;
    size_t tokens;
    size_t literal_tokens;
    size_t backreference_tokens;
    size_t width_markers;
    uint8_t final_bit_width;
} Theron_Huc6280DecodeReceipt;

/* Decode one authentic $23ad resource frame. `resource` starts at the six
 * bytes read by the retail routine: the little-endian length word is at
 * +2, and the bitstream starts at +6. `destination_address` is the CPU
 * address corresponding to destination[0]. `pointer_table` contains the
 * output-address entries used by the $2496 back-reference path; entries
 * supplied by the caller before the call are preserved until the retail
 * loop overwrites them. */
int theron_v1_huc6280_decode_resource(
    const uint8_t *resource, size_t resource_bytes,
    uint8_t *destination, size_t destination_capacity,
    uint16_t destination_address,
    uint16_t *pointer_table, size_t pointer_table_capacity,
    size_t pointer_table_seed_count,
    Theron_Huc6280DecodeReceipt *out);

const Theron_LevelDataBlockDesc *theron_v1_track02_level_data_block(unsigned int level);

const Theron_LevelDataBlockDesc *theron_v1_track02_level_data_block_for_variant(
    Theron_Track02Variant variant, unsigned int level);

/* Reads one complete later-level block from a normalized 2048-byte
 * user-data image. The span ends at the next authenticated block or at the
 * end of the supplied image for level seven. No decompression or tile/object
 * semantics are assigned. */
int theron_v1_track02_level_data_block_read(
    const uint8_t *user_data, size_t user_data_size,
    Theron_Track02Variant variant, unsigned int level,
    Theron_LevelDataBlockReceipt *out);

#ifdef __cplusplus
}
#endif

#endif /* THERON_V1_TRACK02_LEVEL_DATA_BLOCKS_H */
