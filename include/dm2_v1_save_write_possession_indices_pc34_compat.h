#ifndef DM2_V1_SAVE_WRITE_POSSESSION_INDICES_PC34_COMPAT_H
#define DM2_V1_SAVE_WRITE_POSSESSION_INDICES_PC34_COMPAT_H

#include "dm2_v1_save_write_record_checkcode_pc34_compat.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* DM2 WRITE_POSSESSION_INDICES — deferred possession link writer.
 * Source: sksvgame.cpp:1684-1713.
 *
 * Walks the possession index array built during WRITE_RECORD_CHECKCODE,
 * resolves each container/type-0xE record's contents link to a 10-bit
 * creature or container index, and writes it via SUPPRESS. */

/* Callback to resolve a record link to its contents index.
 * record_link: the full record link word from the possession array.
 * Returns the resolved 10-bit index, or -1 if the record type should be skipped. */
typedef struct {
    int (*resolve_possession_index)(void *ctx, uint16_t record_link);
    void *ctx;
} DM2_WritePossessionCallbacks;

/* Write all deferred possession indices.
 * possession_links: array of record link words collected during WRITE_RECORD_CHECKCODE.
 * count: number of entries.
 *
 * Returns 0 on success, 1 on SUPPRESS write error. */
int dm2_v1_write_possession_indices(
    DM2_WriteRecordSession *session,
    const DM2_WritePossessionCallbacks *cb,
    const uint16_t *possession_links,
    int count);

#ifdef __cplusplus
}
#endif

#endif /* DM2_V1_SAVE_WRITE_POSSESSION_INDICES_PC34_COMPAT_H */
