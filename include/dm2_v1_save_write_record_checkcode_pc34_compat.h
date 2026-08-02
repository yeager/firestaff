#ifndef DM2_V1_SAVE_WRITE_RECORD_CHECKCODE_PC34_COMPAT_H
#define DM2_V1_SAVE_WRITE_RECORD_CHECKCODE_PC34_COMPAT_H

#include "dm2_v1_save_load.h"
#include "dm2_v1_save_record_masks_pc34_compat.h"
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* DM2 WRITE_RECORD_CHECKCODE — recursive record-chain SUPPRESS writer.
 * Source: sksvgame.cpp:1739-1940.
 *
 * Walks a linked list of dungeon records, writing each one through
 * the SUPPRESS codec with per-type masks. Recursively descends into
 * creature possessions, container contents, and type-0xE sub-chains.
 *
 * This implementation uses callbacks for runtime data access to
 * decouple from the live dungeon state. */

#define DM2_RECORD_LINK_END   0xFFFF
#define DM2_RECORD_LINK_NONE  0xFFFE

/* Record data returned by the get_record callback.
 * The caller fills these from DM2_GET_ADDRESS_OF_RECORD. */
typedef struct {
    const uint8_t *data;
    size_t size;
    uint16_t word_00;
    uint16_t word_02;
    uint8_t byte_04;
    uint16_t word_04;
} DM2_WriteRecordData;

/* Timer entry for type-0xF timer scan. */
typedef struct {
    uint8_t type;
    uint16_t record_link;
} DM2_WriteRecordTimer;

/* Callback table for runtime data access. */
typedef struct {
    /* Get record data by record link word.
     * Returns 0 on success, nonzero on failure. */
    int (*get_record)(void *ctx, uint16_t record_link,
                      DM2_WriteRecordData *out);

    /* Get next record link (first word of record).
     * Returns the next link, or DM2_RECORD_LINK_END. */
    uint16_t (*get_next_link)(void *ctx, uint16_t record_link);

    /* Query creature AI spec flags for a record.
     * Returns the flags word (bit 0 checked). */
    int (*query_creature_ai_spec_flags)(void *ctx, uint16_t record_link);

    /* Check if container record represents a map container.
     * Returns nonzero if map container. */
    int (*is_container_map)(void *ctx, uint16_t record_link);

    /* Check if container record represents a moneybox.
     * Returns nonzero if moneybox. */
    int (*is_container_moneybox)(void *ctx, uint16_t record_link);

    /* Add record index to possession indices array.
     * Called for map-container contents and nested type-0xE records. */
    void (*add_possession_index)(void *ctx, uint16_t record_link);

    void *ctx;
} DM2_WriteRecordCallbacks;

/* Mutable save-session state shared across recursive calls. */
typedef struct {
    DM2_SuppressWriter writer;
    uint8_t *out_buf;
    size_t out_cap;
    size_t out_written;
    int creature_count;
    int container_count;
    int *creature_indices;
    size_t creature_indices_cap;
    int *container_indices;
    size_t container_indices_cap;
    int nested_creature;
    int nested_type_0e;
    const DM2_WriteRecordTimer *timers;
    int timer_count;
} DM2_WriteRecordSession;

/* Initialize a write-record session. */
void dm2_v1_write_record_session_init(
    DM2_WriteRecordSession *session,
    uint8_t *out_buf, size_t out_cap,
    int *creature_indices, size_t creature_indices_cap,
    int *container_indices, size_t container_indices_cap,
    const DM2_WriteRecordTimer *timers, int timer_count);

/* Write a record chain starting at record_link.
 * write_sub_chain_info: if nonzero, write 2-bit upper field for non-creature types > 3.
 * follow_chain: if nonzero, follow GET_NEXT_RECORD_LINK to walk the full chain.
 *
 * Returns 0 on success, 1 on SUPPRESS write error, -1 on callback failure.
 * On success, session->out_written is updated with total bytes emitted. */
int dm2_v1_write_record_checkcode(
    DM2_WriteRecordSession *session,
    const DM2_WriteRecordCallbacks *cb,
    uint16_t record_link,
    int write_sub_chain_info,
    int follow_chain);

/* Receipt for completed write. */
typedef struct {
    int valid;
    size_t bytes_written;
    int records_written;
    int creatures_indexed;
    int containers_indexed;
    int timers_matched;
    int possession_indices_added;
    int recursion_depth;
} DM2_WriteRecordCheckcodeReceipt;

#ifdef __cplusplus
}
#endif

#endif /* DM2_V1_SAVE_WRITE_RECORD_CHECKCODE_PC34_COMPAT_H */
