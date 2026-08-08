#ifndef FIRESTAFF_DM2_V1_RECORD_POOL_PC34_COMPAT_H
#define FIRESTAFF_DM2_V1_RECORD_POOL_PC34_COMPAT_H

/*
 * dm2_v1_record_pool_pc34_compat.h — DM2 V1 source-ordered c_record pool
 * ownership layer.
 *
 * This module replaces the reduced parallel record view with pools that own
 * their record bytes in the source's exact layout.  Every semantic is locked
 * to skproject/SKULLWIN:
 *
 *   c_record.cpp:28-31   table_recordsizes[16] =
 *                        {4, 6, 4, 8, 16, 4, 4, 4, 4, 8, 4, 0, 0, 0, 8, 4}
 *                        (bytes per record for DB0..DB15; 0 = unallocated DB)
 *   c_record.cpp:44-52   DM2_GET_ADDRESS_OF_RECORD: pool = (r >> 10) & 0xf,
 *                        byte offset = recordsize[pool] * (r & 0x3ff)
 *   c_record.h:8-9       OBJECT_NULL = -1, OBJECT_END_MARKER = -2
 *   c_record.cpp:54-57   DM2_GET_NEXT_RECORD_LINK = first word of record
 *   c_record.cpp:60-120  DM2_APPEND_RECORD_TO list path (x < 0)
 *   c_record.cpp:122+    DM2_CUT_RECORD_FROM list path (x < 0)
 *   c_moverec.cpp        DM2_MOVE_RECORD_TO = cut + append relocation
 *   c_dballoc.cpp        pool allocation/ownership boundaries
 *   SkWinCore.cpp        READ_DUNGEON_STRUCTURE source ordering
 *
 * Fail-closed contract: pools whose source span is not validated are absent
 * (record_size 0 or record_count 0), handles OBJECT_NULL/OBJECT_END_MARKER
 * never resolve, and tile-rooted append/cut paths are rejected until the
 * c_map ground-stack link state is proven.  No record bytes are fabricated.
 */

#include <stdint.h>
#include <stddef.h>

#include "dm2_v1_dungeon_loader.h"
#include "dm2_v1_new_game.h"
#include "dm2_v1_save_read_record_checkcode_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DM2_V1_RECORD_POOL_COUNT 16
#define DM2_V1_RECORD_HANDLE_NULL ((int16_t)-1) /* skproject OBJECT_NULL */
#define DM2_V1_RECORD_HANDLE_END  ((int16_t)-2) /* skproject OBJECT_END_MARKER */
#define DM2_V1_RECORD_INDEX_MASK  0x3ffu        /* c_record.cpp:49 (r & 0x3ff) */

typedef struct {
    uint8_t *bytes;          /* owned copy: record_count * record_size bytes */
    int record_count;        /* declared records in this pool */
    int record_size;         /* bytes per record (table_recordsizes entry) */
    int source_base;         /* byte offset in the source G1 span, -1 absent */
    uint8_t *extension_bytes; /* owned G1-extension continuation, if proven */
    int extension_count;     /* continuation record count, 0 absent */
    int extension_base;      /* source byte offset of continuation, -1 absent */
} DM2_V1_RecordPool;

typedef struct DM2_V1_RecordPoolSet {
    DM2_V1_RecordPool pools[DM2_V1_RECORD_POOL_COUNT];
    int valid;                  /* 1 only after a validated G1 population */
    int record_graph_complete;  /* mirrors the loader's graph state */
} DM2_V1_RecordPoolSet;

/* Receipt for the source-owned direct-root phase of READ_SKSAVE_DUNGEON.
 * Root links are retained for the later champion/hand owner, while the
 * record bytes and list links are written into the authenticated pools. */
#define DM2_V1_SKSAVE_DIRECT_ROOT_MAX 121u
typedef struct {
    int valid;
    uint16_t root_count;
    uint16_t roots[DM2_V1_SKSAVE_DIRECT_ROOT_MAX];
    uint32_t record_count;
    uint32_t possession_continuation_count;
    uint32_t record_hash;
    uint32_t continuation_hash;
    /* Exact shared-SUPPRESS position immediately after the hero/cursor root
     * phase.  The next owner must consume special timer chains, then map
     * chains, before it may read possession continuations. */
    size_t next_stream_offset;
    uint8_t next_stream_bits_remaining;
    uint8_t next_stream_current_byte;
} DM2_V1_SksaveDirectRootReceipt;

/* Exact skproject table_recordsizes entry (bytes).  Returns 0 for the
 * unallocated DB11/DB12/DB13 pools and for out-of-range pool ids. */
int dm2_v1_record_pool_record_size(int pool);

/* Handle decode, c_record.cpp:44-52.  Direction bits 14-15 are masked out
 * exactly like DM2_GET_ADDRESS_OF_RECORD's (r >> 10) & 0xf. */
int dm2_v1_record_handle_pool(int16_t handle);
int dm2_v1_record_handle_index(int16_t handle);

/* DM2_GET_ADDRESS_OF_RECORD equivalent: bounded pointer into the owned pool
 * copy.  NULL for null/end handles, absent or zero-sized pools, and indexes
 * outside the declared record count and any loader-proven G1 continuation
 * (fail-closed). */
const uint8_t *dm2_v1_record_pool_address(const DM2_V1_RecordPoolSet *set,
                                          int16_t handle);
uint8_t *dm2_v1_record_pool_address_mut(DM2_V1_RecordPoolSet *set,
                                        int16_t handle);

/* DM2_GET_NEXT_RECORD_LINK equivalent: the record's first word is the next
 * link.  Returns 0 and leaves *out_next untouched when the handle cannot
 * resolve (fail-closed). */
int dm2_v1_record_pool_next_link(const DM2_V1_RecordPoolSet *set,
                                 int16_t handle,
                                 int16_t *out_next);

/* DM2_APPEND_RECORD_TO list path (x < 0): append `record` at the end of the
 * link list rooted at *list_head_io.  The appended record's own link word
 * becomes OBJECT_END_MARKER before it is chained.  Rejects null/end records
 * and unresolvable list heads. */
int dm2_v1_record_pool_append_to_list(DM2_V1_RecordPoolSet *set,
                                      int16_t *list_head_io,
                                      int16_t record);

/* DM2_CUT_RECORD_FROM list path (x < 0): unlink `record` from the list
 * rooted at *list_head_io.  Returns 1 when the record was found and cut,
 * 0 otherwise.  The cut record's own link word is not rewritten, matching
 * the source (the caller re-appends or deallocates). */
int dm2_v1_record_pool_cut_from_list(DM2_V1_RecordPoolSet *set,
                                     int16_t *list_head_io,
                                     int16_t record);

/* DM2_MOVE_RECORD_TO list relocation boundary (c_moverec.cpp): cut from the
 * source list, then append to the destination list.  Tile-rooted relocation
 * stays rejected until c_map ground-stack link state is proven. */
int dm2_v1_record_pool_relocate(DM2_V1_RecordPoolSet *set,
                                int16_t *from_head_io,
                                int16_t *to_head_io,
                                int16_t record);

/* DM2_CUT_RECORD_FROM tile path (x >= 0): unlink `record` from the tile's
 * thing chain at (map, x, y).  Uses dm2_v1_dungeon_get_first_thing /
 * dm2_v1_dungeon_set_first_thing for head mutation.  Returns 1 on success. */
int dm2_v1_record_pool_cut_from_tile(DM2_V1_RecordPoolSet *set,
                                     DM2_V1_DungeonData *dungeon,
                                     int map, int x, int y,
                                     int16_t record);

struct dm2_dungeon_world;
/* Populate the pool set from a world whose G1 record pools validated
 * (dm2_world_has_verified_g1_record_pools).  Copies the exact source spans
 * declared by the loader's candidate evidence plus the proven DB3/DB4 G1
 * extension continuations.  Returns 1 on full population, 0 fail-closed. */
int dm2_v1_record_pool_set_init_from_world(DM2_V1_RecordPoolSet *set,
                                           const struct dm2_dungeon_world *world);

/* Populate the pool set directly from dungeon data whose G1 candidate
 * evidence validates (dm2_v1_dungeon_collect_g1_record_pool_evidence).
 * Same population contract as init_from_world; the world wrapper only
 * adds its own verification gate.  Returns 1 on full population, 0
 * fail-closed. */
int dm2_v1_record_pool_set_init_from_dungeon(
    DM2_V1_RecordPoolSet *set,
    const DM2_V1_DungeonData *dungeon);

/* Materialize the DB0..DB15 pool image that SKProject has just installed by
 * READ_DUNGEON_STRUCTURE while loading an original raw SKSave body.  The
 * caller supplies the body after its 42-byte c_hex2a header and the receipt
 * issued by dm2_v1_original_raw_sksave_dungeon_receipt().
 *
 * This is intentionally the pre-DM2_READ_SKSAVE_DUNGEON state: it copies
 * the exact source pool spans and their declared capacities, but does not
 * claim that record links, tile chains, hero possessions or timers have been
 * restored.  In particular record_graph_complete remains zero.  The next
 * GAME_LOAD stage must perform SKProject's remove/clear/reallocate order
 * before a save can become playable.
 *
 * Source: SKWINSPX/src/v5/sksvgame.cpp::DM2_GAME_LOAD (1482-1526),
 *         ::DM2_READ_SKSAVE_DUNGEON (1108-1400), and
 *         c_record.cpp::DM2_GET_ADDRESS_OF_RECORD (44-57).
 * Returns 1 only when every declared non-empty DB span is authenticated and
 * copied.  On failure `set` is cleared; no caller-provided bytes are used as
 * a fallback. */
int dm2_v1_record_pool_set_init_from_raw_sksave(
    DM2_V1_RecordPoolSet *set,
    const uint8_t *raw_body,
    size_t raw_body_size,
    const DM2_V1_OriginalRawDungeonReceipt *dungeon_receipt);

/* Execute the DB4..DB15 clearing phase of
 * DM2_READ_SKSAVE_DUNGEON after the caller has removed every dynamic record
 * from its source-owned map chains.  SKProject leaves DB0..DB3 in place,
 * then writes OBJECT_NULL to the first word of every DB4..DB15 record before
 * READ_RECORD_CHECKCODE reallocates hero, party and tile chains.
 *
 * `dungeon_receipt` must be the same authenticated raw baseline used to
 * create `set`; mismatched counts, unallocated DBs, absent spans, a complete
 * graph, or invalid ownership reject without modifying the set.  This helper
 * does not itself detach tile links and therefore cannot publish a restored
 * game. Source: SKWINSPX/src/v5/sksvgame.cpp::DM2_READ_SKSAVE_DUNGEON
 * lines 1128-1176. */
int dm2_v1_record_pool_clear_raw_sksave_dynamic_records(
    DM2_V1_RecordPoolSet *set,
    const DM2_V1_OriginalRawDungeonReceipt *dungeon_receipt);

/* Decode the source direct-root stream after the authenticated DB-clear
 * phase. This is the production pool owner for SKProject's
 * READ_RECORD_CHECKCODE hero/cursor roots; it does not attach tile roots or
 * publish a playable session.  The returned reader boundary is deliberately
 * before special timer chains and map chains.  SKProject reads possession
 * continuations only after both of those phases, so this helper must not
 * consume them. `query_creature_ai_flags` must resolve the authenticated
 * CREATURES[type] -> v1d296c source row, or the whole transaction fails. */
int dm2_v1_record_pool_restore_raw_sksave_direct_roots(
    DM2_V1_RecordPoolSet *set,
    const uint8_t *raw_body,
    size_t raw_body_size,
    const DM2_V1_OriginalRawSaveStateReceipt *state_receipt,
    DM2_ReadRecordCreatureAiFlagsFn query_creature_ai_flags,
    void *query_creature_ai_flags_ctx,
    DM2_V1_SksaveDirectRootReceipt *out_receipt);

void dm2_v1_record_pool_set_free(DM2_V1_RecordPoolSet *set);

const char *dm2_v1_record_pool_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_RECORD_POOL_PC34_COMPAT_H */
