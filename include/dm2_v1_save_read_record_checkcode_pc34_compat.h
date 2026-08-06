#ifndef DM2_V1_SAVE_READ_RECORD_CHECKCODE_PC34_COMPAT_H
#define DM2_V1_SAVE_READ_RECORD_CHECKCODE_PC34_COMPAT_H

/* DM2 READ_RECORD_CHECKCODE — recursive record-chain SUPPRESS reader.
 * Source: sksvgame.cpp:1476-1738 (inverse of WRITE_RECORD_CHECKCODE).
 *
 * Reads SUPPRESS-encoded record data and populates record buffers
 * via callbacks. Mirror of the writer in
 * dm2_v1_save_write_record_checkcode_pc34_compat.h. */

#include "dm2_v1_save_load.h"
#include "dm2_v1_save_record_masks_pc34_compat.h"
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Callback to allocate a new record and return its link word.
 * record_type: 0-15 record type.
 * Returns the allocated record link, or DM2_RECORD_LINK_NONE on failure. */
typedef uint16_t (*DM2_ReadRecordAllocFn)(void *ctx, int record_type);

/* Callback to set record data after SUPPRESS decode.
 * record_link: the allocated link word.
 * data: decoded record bytes.
 * size: byte count.
 * Returns 0 on success. */
typedef int (*DM2_ReadRecordSetDataFn)(void *ctx, uint16_t record_link,
                                       const uint8_t *data, size_t size);

/* Source: SKWINSPX/src/v5/skrecord.cpp::DM2_APPEND_RECORD_TO.
 * The allocator owns both the record's initial end marker and insertion into
 * either a parent record-link field or a tile chain. `owner_link` is the
 * exact `i16* ebxpw` destination from DM2_READ_RECORD_CHECKCODE; it is NULL
 * only for a tile root, which is addressed by map_x/map_y. */
typedef int (*DM2_ReadRecordAppendFn)(void *ctx, uint16_t new_link,
                                      uint16_t *owner_link,
                                      int map_x, int map_y);

/* Return the `uw_02` field of an allocated record.  READ_RECORD_CHECKCODE
 * initializes that field to OBJECT_END_MARKER before recursively restoring
 * creature/container/type-0xE subchains. */
typedef int (*DM2_ReadRecordChildOwnerFn)(void *ctx, uint16_t record_link,
                                          uint16_t **out_owner_link);

/* Source: sksvgame.cpp::DM2_ADD_INDEX_TO_POSSESSION_INDICES. Map containers
 * and creature-owned type 0xE records retain their source-owned possession
 * continuation through this callback instead of manufacturing a chain. */
typedef void (*DM2_ReadRecordAddPossessionIndexFn)(void *ctx,
                                                    uint16_t record_link);

/* Resolve the original AI-spec flags for a just-allocated DB4 creature.
 * SKProject c_savegame.cpp::DM2_READ_RECORD_CHECKCODE selects v1d648f rather
 * than the default v1d647f mask when QUERY_CREATURE_AI_SPEC_FLAGS(record)
 * has bit 0 set.  The creature type is byte 4, read before its body mask.
 * Return zero only when the result comes from the authenticated CREATURES →
 * CREATURE_AI GDAT chain.  A missing provider is deliberately not equivalent
 * to zero flags: accepting that would desynchronise the shared SKSAVE stream.
 */
typedef int (*DM2_ReadRecordCreatureAiFlagsFn)(void *ctx,
                                               uint16_t record_link,
                                               uint8_t creature_type,
                                               uint16_t *out_flags);

typedef struct {
    DM2_ReadRecordAllocFn alloc_record;
    DM2_ReadRecordSetDataFn set_data;
    DM2_ReadRecordAppendFn append_record;
    DM2_ReadRecordChildOwnerFn child_owner;
    DM2_ReadRecordAddPossessionIndexFn add_possession_index;
    DM2_ReadRecordCreatureAiFlagsFn query_creature_ai_flags;
    void *ctx;
} DM2_ReadRecordCallbacks;

typedef struct {
    DM2_SuppressReader reader;
    const uint8_t *in_buf;
    size_t in_size;
    size_t in_consumed;
    int records_read;
    int creatures_read;
    int containers_read;
    int map_containers_read;
    int possessions_read;
    int nested_creature;
    int nested_type_0e;
    int error;
} DM2_ReadRecordSession;

void dm2_v1_read_record_session_init(
    DM2_ReadRecordSession *session,
    const uint8_t *in_buf, size_t in_size);

/* Read a record chain from SUPPRESS-encoded data.
 * read_sub_chain_info: if nonzero, read 2-bit upper field for non-creature types > 3.
 * follow_chain: if nonzero, keep reading until terminator bit.
 *
 * Returns 0 on success, nonzero on error. */
int dm2_v1_read_record_checkcode(
    DM2_ReadRecordSession *session,
    const DM2_ReadRecordCallbacks *cb,
    uint16_t *owner_link,
    int map_x, int map_y,
    int read_sub_chain_info,
    int follow_chain);

#ifdef __cplusplus
}
#endif

#endif /* DM2_V1_SAVE_READ_RECORD_CHECKCODE_PC34_COMPAT_H */
