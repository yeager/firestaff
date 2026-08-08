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
#include "dm2_v1_save_load_extra_dungeon_data_pc34_compat.h"
#include "dm2_v1_save_timers_pc34_compat.h"
#include "dm2_v1_save_post_load_timer_rebuild_pc34_compat.h"

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

/* Source-owned mutable view of c_map::dm2_v1e038c during
 * READ_SKSAVE_DUNGEON.  The raw SKSAVE body remains immutable; only its
 * ground-stack link span is copied because SKProject removes DB4..DB15 links
 * from that span before it clears the dynamic record pools.  This is not a
 * replacement dungeon model. */
typedef struct {
    const uint8_t *raw_body;
    size_t raw_body_size;
    const DM2_V1_OriginalRawDungeonReceipt *dungeon;
    uint16_t *ground_stack_links;
    size_t ground_stack_count;
    uint8_t *map_tiles;
    size_t map_tiles_size;
    uint16_t map_tile_offsets[DM2_RAW_SKSAVE_MAX_MAPS];
    int current_map;
    int valid;
    int dynamic_records_detached;
} DM2_V1_SksaveMapOwner;

/* The map-facing portion of the eventual single GAME_LOAD transaction. It
 * binds exactly one mutable c_map owner to exactly one c_record pool set;
 * callers must not substitute either member after stream consumption begins. */
typedef struct {
    DM2_V1_SksaveMapOwner *map_owner;
    DM2_V1_RecordPoolSet *record_pools;
} DM2_V1_SksaveMapRestoreContext;

/* Construct the sole mutable owner of the authenticated ground-stack links.
 * `raw_body` is never copied or modified. */
int dm2_v1_sksave_map_owner_init(
    DM2_V1_SksaveMapOwner *owner,
    const uint8_t *raw_body,
    size_t raw_body_size,
    const DM2_V1_OriginalRawDungeonReceipt *dungeon_receipt);

void dm2_v1_sksave_map_owner_free(DM2_V1_SksaveMapOwner *owner);

/* c_map::DM2_GET_TILE_RECORD_LINK against the mutable dm2_v1e038c copy.
 * An unmarked tile returns OBJECT_END_MARKER (0xfffe). */
int dm2_v1_sksave_map_owner_tile_record_link(
    const DM2_V1_SksaveMapOwner *owner,
    int map, int x, int y, uint16_t *out_link);

/* Direct DM2_LoadExtraDungeonCallbacks adapters. They expose the mutable
 * RAM tile copy while retaining the authenticated raw body as provenance. */
int dm2_v1_sksave_map_owner_get_map_count(void *ctx);
void dm2_v1_sksave_map_owner_get_map_dimensions(void *ctx,
                                                 int *width, int *height);
void dm2_v1_sksave_map_owner_change_current_map(void *ctx, int map);
uint8_t dm2_v1_sksave_map_owner_get_tile(void *ctx, int x, int y);
int dm2_v1_sksave_map_owner_set_tile(void *ctx, int x, int y, uint8_t tile);
uint16_t dm2_v1_sksave_map_owner_get_tile_record_link_current(
    void *ctx, int x, int y);

int dm2_v1_sksave_map_restore_context_init(
    DM2_V1_SksaveMapRestoreContext *context,
    DM2_V1_SksaveMapOwner *map_owner,
    DM2_V1_RecordPoolSet *record_pools);

/* Complete adapters for DM2_LoadExtraDungeonCallbacks. `ctx` is a
 * DM2_V1_SksaveMapRestoreContext, not a bare map owner, so the resident-chain
 * callback cannot accidentally decode against another pool. */
int dm2_v1_sksave_map_restore_get_map_count(void *ctx);
void dm2_v1_sksave_map_restore_get_map_dimensions(void *ctx,
                                                   int *width, int *height);
void dm2_v1_sksave_map_restore_change_current_map(void *ctx, int map);
uint8_t dm2_v1_sksave_map_restore_get_tile(void *ctx, int x, int y);
int dm2_v1_sksave_map_restore_set_tile(void *ctx, int x, int y, uint8_t tile);
uint16_t dm2_v1_sksave_map_restore_get_tile_record_link(void *ctx,
                                                          int x, int y);
void dm2_v1_sksave_map_restore_set_tile_record_link(void *ctx,
                                                      int x, int y,
                                                      uint16_t link);
int dm2_v1_sksave_map_restore_existing_tile_record_chain(
    void *ctx, DM2_ReadRecordSession *session, uint16_t root_link,
    int x, int y);
/* Source-owned c_querydb teleporter detail for the active saved map. This
 * is the DM2_LoadExtraDungeonCallbacks forward-reference query; it reads
 * only the authentic DB1 link and the mutable c_map tile spans held by the
 * same restore context. */
int dm2_v1_sksave_map_restore_get_teleporter_detail(
    void *ctx, DM2_TeleporterDetail *out, int x, int y);

/* First destructive phase of sksvgame.cpp::DM2_READ_SKSAVE_DUNGEON:
 * unlink every DB4..DB15 record from actual tile chains, preserving DB0..DB3
 * resident roots and setting each detached record link to OBJECT_END_MARKER.
 * The caller may clear dynamic pools only after this succeeds. */
int dm2_v1_sksave_map_owner_detach_dynamic_records(
    DM2_V1_SksaveMapOwner *owner,
    DM2_V1_RecordPoolSet *set,
    uint32_t *out_detached_count);

/* Restore the source SUPPRESS fields of one already resident DB0..DB3 tile
 * chain. The caller obtains `root_link` from DM2_V1_SksaveMapOwner; no
 * record is allocated and no tile link is replaced. This is
 * sksvgame.cpp::DM2_READ_SKSAVE_DUNGEON's non-empty-tile branch, including
 * DB3's eight actuator subtypes with a preceding nine-bit value. */
int dm2_v1_record_pool_restore_raw_sksave_resident_chain(
    DM2_V1_RecordPoolSet *set,
    DM2_ReadRecordSession *session,
    uint16_t root_link);

/* Receipt for the source-owned direct-root phase of READ_SKSAVE_DUNGEON.
 * Root links are retained for the later champion/hand owner, while the
 * record bytes and list links are written into the authenticated pools. */
#define DM2_V1_SKSAVE_DIRECT_ROOT_MAX 121u
/* DM2_2066_062b reads continuation values only after timer and map chains.
 * Retain the source links here, in discovery order, so later GAME_LOAD
 * phases can extend the same list instead of re-walking a reconstructed
 * inventory. This is a bounded receipt, never a possession store or runtime
 * inventory. */
#define DM2_V1_SKSAVE_POSSESSION_LINK_MAX 4096u
typedef struct {
    int valid;
    uint16_t root_count;
    uint16_t roots[DM2_V1_SKSAVE_DIRECT_ROOT_MAX];
    uint32_t record_count;
    uint32_t possession_link_count;
    uint16_t possession_links[DM2_V1_SKSAVE_POSSESSION_LINK_MAX];
    uint32_t possession_continuation_count;
    uint32_t record_hash;
    uint32_t possession_link_hash;
    uint32_t continuation_hash;
    /* Exact shared-SUPPRESS position immediately after the hero/cursor root
     * phase.  The next owner must consume special timer chains, then map
     * chains, before it may read possession continuations. */
    size_t next_stream_offset;
    uint8_t next_stream_bits_remaining;
    uint8_t next_stream_current_byte;
} DM2_V1_SksaveDirectRootReceipt;

/* Apply the exact source-order direct-root result to the already
 * materialised c_hero bytes.  sksvgame.cpp::DM2_READ_SKSAVE_DUNGEON first
 * writes 30 OBJECT_END_MARKER roots per party hero, then the leader-hand
 * root.  This function owns neither a live session nor item bonuses; it is
 * the bounded c_hero/link handoff within the temporary GAME_LOAD
 * transaction. */
int dm2_v1_sksave_apply_direct_roots_to_heroes(
    DM2_V1_Hero *heroes, size_t hero_capacity, uint16_t hero_count,
    const DM2_V1_SksaveDirectRootReceipt *roots,
    uint16_t *out_leader_hand_root, uint32_t *out_root_hash);

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

/* Populate the pool set directly from authenticated dungeon data. The legacy
 * PC G1 extension route requires its candidate-pool evidence; the canonical
 * 44-map File_header route requires its own runtime-map receipt and copies
 * the exact declared DB spans. The two layouts are never reinterpreted as
 * one another. Returns 1 on full population, 0 fail-closed. */
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

typedef enum {
    DM2_V1_SKSAVE_PREFLIGHT_FAILURE_NONE = 0,
    DM2_V1_SKSAVE_PREFLIGHT_FAILURE_PREPARE,
    DM2_V1_SKSAVE_PREFLIGHT_FAILURE_SPECIAL_TIMERS,
    DM2_V1_SKSAVE_PREFLIGHT_FAILURE_MAPS,
    DM2_V1_SKSAVE_PREFLIGHT_FAILURE_POSSESSIONS,
    DM2_V1_SKSAVE_PREFLIGHT_FAILURE_TIMER_REBUILD
} DM2_V1_SksavePreflightFailureStage;

/* Source-order preflight through DM2_2066_197c.  It owns a temporary raw
 * c_record pool and therefore never publishes a partial runtime session.
 * The caller supplies c_hex2a.w_00 (`savegamew7`) from the same verified
 * 42-byte SKSAVE header that owns `raw_body`. */
typedef struct {
    int valid;
    DM2_V1_SksavePreflightFailureStage failure_stage;
    uint16_t hero_count;
    uint16_t timer_count;
    uint16_t special_chain_count;
    uint16_t maps_loaded;
    uint32_t tiles_loaded;
    uint32_t map_record_chains_loaded;
    uint32_t teleporter_forward_refs_skipped;
    int16_t map_failure_map;
    int16_t map_failure_x;
    int16_t map_failure_y;
    uint16_t map_failure_root_link;
    int16_t map_failure_record_type;
    int16_t map_failure_record_reason;
    uint32_t possession_link_count;
    uint32_t possession_continuation_count;
    int16_t timer_queue_count;
    int16_t timer_free_head;
    uint16_t hero_timeridx_cleared;
    uint16_t hero_timeridx_set;
    uint16_t ornate_timer_backlinks_set;
    uint16_t direct_root_count;
    uint16_t leader_hand_root_link;
    uint32_t timer_hash;
    uint32_t heroes_hash;
    uint32_t direct_root_hash;
    uint32_t timer_queue_hash;
    uint32_t record_hash;
    uint32_t continuation_hash;
    size_t next_stream_offset;
    uint8_t next_stream_bits_remaining;
    uint8_t next_stream_current_byte;
} DM2_V1_SksaveSpecialTimerReceipt;

/* Preflight the source-ordered prefix through DM2_2066_197c.  It creates one
 * temporary raw c_map/c_record/c_tim owner, detaches dynamic tile records,
 * restores direct hero/cursor roots, consumes timer types 0x3c and 0x3d,
 * then walks every map through READ_SKSAVE_DUNGEON and restores the final
 * DM2_2066_062b possession continuations into those temporary record owners,
 * then reconstructs the source timer heap/free list and applies the proven
 * DM2_3a15_020f timer back-links. The temporary owner
 * is always discarded, never a partial Resume session. */
int dm2_v1_record_pool_preflight_raw_sksave_special_timer_chains(
    const uint8_t *raw_body,
    size_t raw_body_size,
    const DM2_V1_OriginalRawSaveStateReceipt *state_receipt,
    uint16_t savegamew7,
    DM2_ReadRecordCreatureAiFlagsFn query_creature_ai_flags,
    void *query_creature_ai_flags_ctx,
    DM2_V1_SksaveSpecialTimerReceipt *out_receipt);

void dm2_v1_record_pool_set_free(DM2_V1_RecordPoolSet *set);

const char *dm2_v1_record_pool_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_RECORD_POOL_PC34_COMPAT_H */
