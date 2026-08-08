/*
 * dm2_v1_record_pool_pc34_compat.c — DM2 V1 source-ordered c_record pool
 * ownership layer.
 *
 * The DM2 world is owned by the source-ordered c_record model instead of a
 * reduced parallel structure: each of the 16 DB pools owns an exact copy of
 * its source span (plus the proven DB3/DB4 G1-extension continuations), and
 * all address, link, and relocation semantics follow skproject verbatim.
 *
 * Source: skproject/SKULLWIN/c_record.cpp:28-31   (table_recordsizes)
 *         skproject/SKULLWIN/c_record.cpp:44-52   (DM2_GET_ADDRESS_OF_RECORD)
 *         skproject/SKULLWIN/c_record.cpp:54-57   (DM2_GET_NEXT_RECORD_LINK)
 *         skproject/SKULLWIN/c_record.cpp:60-170  (APPEND/CUT list paths)
 *         skproject/SKULLWIN/c_moverec.cpp        (DM2_MOVE_RECORD_TO)
 *         skproject/SKULLWIN/c_dballoc.cpp        (pool ownership)
 *         skproject/SKWIN/SkWinCore.cpp           (READ_DUNGEON_STRUCTURE)
 */

#include "dm2_v1_record_pool_pc34_compat.h"
#include "dm2_v1_dungeon_loader.h"

#include <stdlib.h>
#include <string.h>
#include <limits.h>

/* skproject/SKULLWIN/c_record.cpp:28-31 — table_recordsizes[16], bytes per
 * record for DB0..DB15.  Zero entries (DB11..DB13) have no allocated pool in
 * the source and must never resolve an address. */
static const uint8_t s_table_recordsizes[DM2_V1_RECORD_POOL_COUNT] = {
    4, 6, 4, 8, 16, 4, 4, 4, 4, 8, 4, 0, 0, 0, 8, 4
};

/* skmap.cpp reads these fixed READ_DUNGEON_STRUCTURE spans.  They remain
 * local because the public raw-dungeon receipt deliberately exposes offsets,
 * not a second serialized-layout API. */
enum {
    DM2_V1_RAW_DUNGEON_HEADER_SIZE = 44,
    DM2_V1_RAW_MAP_DESC_SIZE = 16
};

static int16_t dm2_v1_rd16(const uint8_t *p)
{
    return (int16_t)((uint16_t)p[0] | ((uint16_t)p[1] << 8));
}

static void dm2_v1_wr16(uint8_t *p, int16_t v)
{
    uint16_t u = (uint16_t)v;
    p[0] = (uint8_t)(u & 0xffu);
    p[1] = (uint8_t)((u >> 8) & 0xffu);
}

static uint16_t dm2_v1_raw_rd16(const uint8_t *p)
{
    return (uint16_t)p[0] | ((uint16_t)p[1] << 8);
}

static int dm2_v1_sksave_map_owner_ground_index(
    const DM2_V1_SksaveMapOwner *owner, int map, int x, int y,
    size_t *out_index)
{
    const DM2_V1_OriginalRawDungeonReceipt *dungeon;
    size_t column_index_base;
    size_t column_base = 0u;
    size_t object_index;
    size_t i;
    uint8_t tile;

    if (!owner || !owner->valid || !owner->raw_body || !owner->dungeon ||
        !out_index) return 0;
    dungeon = owner->dungeon;
    if (map < 0 || map >= (int)dungeon->map_count || x < 0 || y < 0 ||
        x >= (int)dungeon->map_widths[map] ||
        y >= (int)dungeon->map_heights[map]) return 0;
    tile = owner->raw_body[(size_t)dungeon->map_data_base +
        (size_t)dungeon->map_data_relative_offsets[map] +
        (size_t)x * dungeon->map_heights[map] + (size_t)y];
    if ((tile & 0x10u) == 0u) return -1;
    column_index_base = (size_t)DM2_V1_RAW_DUNGEON_HEADER_SIZE +
        (size_t)dungeon->map_count * DM2_V1_RAW_MAP_DESC_SIZE;
    for (i = 0u; i < (size_t)map; ++i) column_base += dungeon->map_widths[i];
    if (column_base + (size_t)x >= (size_t)dungeon->column_index_count ||
        column_index_base > owner->raw_body_size ||
        (size_t)dungeon->column_index_count >
            (owner->raw_body_size - column_index_base) / 2u) return 0;
    object_index = (size_t)dm2_v1_raw_rd16(owner->raw_body +
        column_index_base + (column_base + (size_t)x) * 2u);
    for (i = 0u; i < (size_t)y; ++i) {
        uint8_t prior = owner->raw_body[(size_t)dungeon->map_data_base +
            (size_t)dungeon->map_data_relative_offsets[map] +
            (size_t)x * dungeon->map_heights[map] + i];
        if ((prior & 0x10u) != 0u) ++object_index;
    }
    if (object_index >= owner->ground_stack_count) return 0;
    *out_index = object_index;
    return 1;
}

int dm2_v1_sksave_map_owner_init(
    DM2_V1_SksaveMapOwner *owner, const uint8_t *raw_body,
    size_t raw_body_size,
    const DM2_V1_OriginalRawDungeonReceipt *dungeon_receipt)
{
    size_t column_index_base;
    size_t ground_stack_base;
    size_t bytes;
    size_t i;

    if (owner) memset(owner, 0, sizeof(*owner));
    if (!owner || !raw_body || !dungeon_receipt || !dungeon_receipt->valid)
        return 0;
    column_index_base = (size_t)DM2_V1_RAW_DUNGEON_HEADER_SIZE +
        (size_t)dungeon_receipt->map_count * DM2_V1_RAW_MAP_DESC_SIZE;
    ground_stack_base = column_index_base +
        (size_t)dungeon_receipt->column_index_count * 2u;
    bytes = (size_t)dungeon_receipt->ground_stack_count * 2u;
    if (column_index_base > raw_body_size ||
        (size_t)dungeon_receipt->column_index_count >
            (raw_body_size - column_index_base) / 2u ||
        ground_stack_base > raw_body_size || bytes > raw_body_size - ground_stack_base)
        return 0;
    owner->ground_stack_links = (uint16_t *)malloc(bytes ? bytes : 1u);
    if (!owner->ground_stack_links) return 0;
    for (i = 0u; i < (size_t)dungeon_receipt->ground_stack_count; ++i) {
        uint16_t link = dm2_v1_raw_rd16(raw_body + ground_stack_base + i * 2u);
        if (link != 0xfffeu && link != 0xffffu &&
            (dm2_v1_record_handle_pool((int16_t)link) >= DM2_V1_RECORD_POOL_COUNT ||
             dm2_v1_record_handle_index((int16_t)link) >=
                 dungeon_receipt->db_record_counts[dm2_v1_record_handle_pool((int16_t)link)])) {
            free(owner->ground_stack_links);
            memset(owner, 0, sizeof(*owner));
            return 0;
        }
        owner->ground_stack_links[i] = link;
    }
    owner->raw_body = raw_body;
    owner->raw_body_size = raw_body_size;
    owner->dungeon = dungeon_receipt;
    owner->ground_stack_count = dungeon_receipt->ground_stack_count;
    owner->valid = 1;
    return 1;
}

void dm2_v1_sksave_map_owner_free(DM2_V1_SksaveMapOwner *owner)
{
    if (!owner) return;
    free(owner->ground_stack_links);
    memset(owner, 0, sizeof(*owner));
}

int dm2_v1_sksave_map_owner_tile_record_link(
    const DM2_V1_SksaveMapOwner *owner, int map, int x, int y,
    uint16_t *out_link)
{
    size_t index;
    int rc;

    if (out_link) *out_link = 0xfffeu;
    if (!out_link) return 0;
    rc = dm2_v1_sksave_map_owner_ground_index(owner, map, x, y, &index);
    if (rc < 0) return 1;
    if (rc == 0) return 0;
    *out_link = owner->ground_stack_links[index];
    return 1;
}

int dm2_v1_sksave_map_owner_detach_dynamic_records(
    DM2_V1_SksaveMapOwner *owner, DM2_V1_RecordPoolSet *set,
    uint32_t *out_detached_count)
{
    size_t budget = 1u;
    uint32_t detached = 0u;
    int map;
    int pool;

    if (out_detached_count) *out_detached_count = 0u;
    if (!owner || !owner->valid || owner->dynamic_records_detached || !set ||
        !set->valid || set->record_graph_complete != 0) return 0;
    for (pool = 0; pool < DM2_V1_RECORD_POOL_COUNT; ++pool) {
        if (set->pools[pool].record_count > 0)
            budget += (size_t)set->pools[pool].record_count;
    }
    for (map = 0; map < (int)owner->dungeon->map_count; ++map) {
        int x;
        for (x = 0; x < (int)owner->dungeon->map_widths[map]; ++x) {
            int y;
            for (y = 0; y < (int)owner->dungeon->map_heights[map]; ++y) {
                size_t ground_index;
                int rc = dm2_v1_sksave_map_owner_ground_index(owner, map, x, y,
                                                               &ground_index);
                int16_t current;
                int16_t previous = DM2_V1_RECORD_HANDLE_NULL;
                size_t steps = 0u;
                if (rc < 0) continue;
                if (rc == 0) return 0;
                current = (int16_t)owner->ground_stack_links[ground_index];
                while (current != DM2_V1_RECORD_HANDLE_END &&
                       current != DM2_V1_RECORD_HANDLE_NULL) {
                    uint8_t *record;
                    int16_t next;
                    int current_pool = dm2_v1_record_handle_pool(current);
                    if (++steps > budget || current_pool < 0 ||
                        current_pool >= DM2_V1_RECORD_POOL_COUNT ||
                        !(record = dm2_v1_record_pool_address_mut(set, current)) ||
                        !dm2_v1_record_pool_next_link(set, current, &next)) return 0;
                    if (current_pool >= 4) {
                        /* sksvgame.cpp:1138-1150: sever the dynamic record
                         * before CUT_RECORD_FROM rewires its real tile owner. */
                        dm2_v1_wr16(record, DM2_V1_RECORD_HANDLE_END);
                        if (previous == DM2_V1_RECORD_HANDLE_NULL) {
                            owner->ground_stack_links[ground_index] = (uint16_t)next;
                        } else {
                            uint8_t *prev = dm2_v1_record_pool_address_mut(set, previous);
                            if (!prev) return 0;
                            dm2_v1_wr16(prev, next);
                        }
                        ++detached;
                    } else {
                        previous = current;
                    }
                    current = next;
                }
            }
        }
    }
    owner->dynamic_records_detached = 1;
    if (out_detached_count) *out_detached_count = detached;
    return 1;
}

static int dm2_v1_sksave_db3_has_extra_value(const uint8_t *record)
{
    const uint8_t subtype = (uint8_t)(record[2] & 0x7fu);

    /* sksvgame.cpp:1336-1345 — only these c_actuator subtypes carry the
     * nine-bit value ahead of table1d64db[3]'s normal record mask. */
    return subtype == 0x27u || subtype == 0x1bu || subtype == 0x1du ||
           subtype == 0x41u || subtype == 0x2cu || subtype == 0x32u ||
           subtype == 0x30u || subtype == 0x2du;
}

int dm2_v1_record_pool_restore_raw_sksave_resident_chain(
    DM2_V1_RecordPoolSet *set, DM2_ReadRecordSession *session,
    uint16_t root_link)
{
    const uint8_t *sizes = dm2_v1_save_record_sizes();
    int16_t current = (int16_t)root_link;
    size_t budget = 1u;
    size_t steps = 0u;
    int pool;

    if (!set || !set->valid || set->record_graph_complete != 0 || !session ||
        root_link == 0xfffeu || root_link == 0xffffu) return 0;
    for (pool = 0; pool < 4; ++pool) {
        if (set->pools[pool].record_count > 0)
            budget += (size_t)set->pools[pool].record_count;
    }
    while (current != DM2_V1_RECORD_HANDLE_END &&
           current != DM2_V1_RECORD_HANDLE_NULL) {
        const int type = dm2_v1_record_handle_pool(current);
        const uint8_t *mask;
        uint8_t *record;
        int16_t next;

        if (++steps > budget || type < 0 || type > 3 ||
            !(record = dm2_v1_record_pool_address_mut(set, current)) ||
            !dm2_v1_record_pool_next_link(set, current, &next)) return 0;
        mask = dm2_v1_save_record_mask_for_type(type);
        if (type == 3 && dm2_v1_sksave_db3_has_extra_value(record)) {
            uint8_t value_bytes[2] = { 0u, 0u };
            static const uint8_t nine_bit_mask[2] = { 0xffu, 0x01u };
            uint16_t value;
            if (dm2_suppress_reader_read(&session->reader, nine_bit_mask, 2u,
                                         value_bytes, 0u) != 0) return 0;
            value = (uint16_t)value_bytes[0] |
                    ((uint16_t)value_bytes[1] << 8);
            record[2] = (uint8_t)((record[2] & 0x7fu) |
                                  ((value & 0x01u) << 7));
            record[3] = (uint8_t)(value >> 1);
        }
        if (mask && dm2_suppress_reader_read_preserve(
                &session->reader, mask, sizes[type], record) != 0) return 0;
        current = next;
    }
    return current == DM2_V1_RECORD_HANDLE_END;
}

/* The record-pool owner is also compiled by narrow G1 audit targets that do
 * not link the optional SKSAVE decoder. Keep those targets linkable while a
 * decoder-linked target resolves these weak fail-closed fallbacks with the
 * source implementation. */
__attribute__((weak)) void dm2_v1_read_record_session_init(
    DM2_ReadRecordSession *session, const uint8_t *in_buf, size_t in_size)
{
    if (!session) return;
    memset(session, 0, sizeof(*session));
    session->in_buf = in_buf;
    session->in_size = in_size;
}

__attribute__((weak)) int dm2_v1_read_record_checkcode(
    DM2_ReadRecordSession *session, const DM2_ReadRecordCallbacks *cb,
    uint16_t *owner_link, int map_x, int map_y, int read_sub_chain_info,
    int follow_chain)
{
    (void)session; (void)cb; (void)owner_link; (void)map_x; (void)map_y;
    (void)read_sub_chain_info; (void)follow_chain;
    return -1;
}

typedef struct {
    DM2_V1_RecordPoolSet *set;
    uint32_t record_hash;
    uint32_t record_count;
    uint16_t record_links[4096];
    DM2_ReadRecordCreatureAiFlagsFn ai_fn;
    void *ai_ctx;
} DM2_V1_SksavePoolRestoreContext;

static uint32_t dm2_v1_sksave_hash_bytes(uint32_t hash,
                                         const uint8_t *bytes, size_t size)
{
    size_t i;
    for (i = 0u; i < size; ++i) {
        hash ^= bytes[i];
        hash *= 16777619u;
    }
    return hash;
}

static uint16_t dm2_v1_sksave_pool_alloc(void *context, int record_type)
{
    DM2_V1_SksavePoolRestoreContext *ctx =
        (DM2_V1_SksavePoolRestoreContext *)context;
    DM2_V1_RecordPool *pool;
    int index;

    if (!ctx || !ctx->set || record_type < 4 || record_type >= 16) {
        return 0xfffeu;
    }
    pool = &ctx->set->pools[record_type];
    if (pool->record_size <= 0 || !pool->bytes) return 0xfffeu;
    for (index = 0; index < pool->record_count; ++index) {
        uint8_t *record = pool->bytes +
            (size_t)index * (size_t)pool->record_size;
        if (dm2_v1_rd16(record) == DM2_V1_RECORD_HANDLE_NULL) {
            return (uint16_t)(((uint16_t)record_type << 10) | (uint16_t)index);
        }
    }
    return 0xfffeu;
}

static int dm2_v1_sksave_pool_set_data(void *context, uint16_t record_link,
                                       const uint8_t *data, size_t size)
{
    DM2_V1_SksavePoolRestoreContext *ctx =
        (DM2_V1_SksavePoolRestoreContext *)context;
    DM2_V1_RecordPool *pool;
    uint8_t *record;
    int type = dm2_v1_record_handle_pool((int16_t)record_link);
    int index = dm2_v1_record_handle_index((int16_t)record_link);

    if (!ctx || !ctx->set || !data || type < 4 || type >= 16) return -1;
    pool = &ctx->set->pools[type];
    if (index < 0 || index >= pool->record_count ||
        size != (size_t)pool->record_size) return -1;
    record = pool->bytes + (size_t)index * (size_t)pool->record_size;
    memcpy(record, data, size);
    if (ctx->record_count >= sizeof(ctx->record_links) /
        sizeof(ctx->record_links[0])) return -1;
    ctx->record_hash = dm2_v1_sksave_hash_bytes(
        ctx->record_hash, (const uint8_t *)&record_link, sizeof(record_link));
    ctx->record_hash = dm2_v1_sksave_hash_bytes(ctx->record_hash, data, size);
    ctx->record_links[ctx->record_count] = record_link;
    ++ctx->record_count;
    return 0;
}

static int dm2_v1_sksave_pool_append(void *context, uint16_t new_link,
                                     uint16_t *owner_link, int map_x, int map_y)
{
    DM2_V1_SksavePoolRestoreContext *ctx =
        (DM2_V1_SksavePoolRestoreContext *)context;
    uint16_t cursor;
    unsigned int guard = 0u;

    if (!ctx || !ctx->set || !owner_link || map_x != -1 || map_y != 0) {
        return -1;
    }
    dm2_v1_wr16((uint8_t *)dm2_v1_record_pool_address_mut(
                    ctx->set, (int16_t)new_link), DM2_V1_RECORD_HANDLE_END);
    cursor = *owner_link;
    if (cursor == (uint16_t)DM2_V1_RECORD_HANDLE_END) {
        *owner_link = new_link;
        return 0;
    }
    while (cursor != (uint16_t)DM2_V1_RECORD_HANDLE_END && guard++ < 1024u) {
        uint8_t *record = dm2_v1_record_pool_address_mut(
            ctx->set, (int16_t)cursor);
        uint16_t next;
        if (!record) return -1;
        next = (uint16_t)dm2_v1_rd16(record);
        if (next == (uint16_t)DM2_V1_RECORD_HANDLE_NULL) return -1;
        if (next == (uint16_t)DM2_V1_RECORD_HANDLE_END) {
            dm2_v1_wr16(record, (int16_t)new_link);
            return 0;
        }
        cursor = next;
    }
    return -1;
}

static int dm2_v1_sksave_pool_child_owner(void *context, uint16_t record_link,
                                          uint16_t **out_owner_link)
{
    DM2_V1_SksavePoolRestoreContext *ctx =
        (DM2_V1_SksavePoolRestoreContext *)context;
    uint8_t *record;
    int type = dm2_v1_record_handle_pool((int16_t)record_link);

    if (!ctx || !ctx->set || !out_owner_link || type < 4 || type >= 16) {
        return -1;
    }
    record = dm2_v1_record_pool_address_mut(ctx->set, (int16_t)record_link);
    if (!record || ctx->set->pools[type].record_size < 4) return -1;
    *out_owner_link = (uint16_t *)(void *)(record + 2);
    return 0;
}

static void dm2_v1_sksave_pool_add_possession(void *context,
                                              uint16_t record_link)
{
    (void)context;
    (void)record_link;
}

static int dm2_v1_sksave_pool_query_ai(void *context, uint16_t record_link,
                                       uint8_t creature_type,
                                       uint16_t *out_flags)
{
    DM2_V1_SksavePoolRestoreContext *ctx =
        (DM2_V1_SksavePoolRestoreContext *)context;
    if (!ctx || !ctx->ai_fn) return -1;
    return ctx->ai_fn(ctx->ai_ctx, record_link, creature_type, out_flags);
}

int dm2_v1_record_pool_record_size(int pool)
{
    if (pool < 0 || pool >= DM2_V1_RECORD_POOL_COUNT) {
        return 0;
    }
    return (int)s_table_recordsizes[pool];
}

int dm2_v1_record_handle_pool(int16_t handle)
{
    /* c_record.cpp:46 — idx = (r >> 10) & 0xf; direction bits 14-15 shift
     * out exactly like the source's unsigned narrowing. */
    return (int)(((uint16_t)handle >> 10) & 0xfu);
}

int dm2_v1_record_handle_index(int16_t handle)
{
    /* c_record.cpp:49 — factor = r & 0x3ff. */
    return (int)((uint16_t)handle & DM2_V1_RECORD_INDEX_MASK);
}

uint8_t *dm2_v1_record_pool_address_mut(DM2_V1_RecordPoolSet *set,
                                        int16_t handle)
{
    int pool;
    int index;
    const DM2_V1_RecordPool *p;

    if (set == NULL || !set->valid ||
        handle == DM2_V1_RECORD_HANDLE_NULL ||
        handle == DM2_V1_RECORD_HANDLE_END) {
        return NULL;
    }
    pool = dm2_v1_record_handle_pool(handle);
    index = dm2_v1_record_handle_index(handle);
    p = &set->pools[pool];
    if (p->record_size <= 0 || index < 0) {
        return NULL;
    }
    /* c_record.cpp:48-51 — ofs = recordsize[idx] * (r & 0x3ff) added to the
     * pool base as a byte offset.  PC G1's validated DB3/DB4 continuation
     * keeps the same ObjectID index space after the declared pool span; the
     * loader owns that continuation separately because it is not contiguous
     * with p->bytes. */
    if (index < p->record_count) {
        if (p->bytes == NULL) {
            return NULL;
        }
        return p->bytes + (size_t)index * (size_t)p->record_size;
    }
    if (p->extension_bytes == NULL || p->extension_count <= 0 ||
        index - p->record_count >= p->extension_count) {
        return NULL;
    }
    return p->extension_bytes +
           (size_t)(index - p->record_count) * (size_t)p->record_size;
}

const uint8_t *dm2_v1_record_pool_address(const DM2_V1_RecordPoolSet *set,
                                          int16_t handle)
{
    return dm2_v1_record_pool_address_mut((DM2_V1_RecordPoolSet *)set, handle);
}

int dm2_v1_record_pool_next_link(const DM2_V1_RecordPoolSet *set,
                                 int16_t handle,
                                 int16_t *out_next)
{
    const uint8_t *addr;

    if (out_next == NULL) {
        return 0;
    }
    addr = dm2_v1_record_pool_address(set, handle);
    if (addr == NULL) {
        return 0;
    }
    /* c_record.cpp:56 — DM2_GET_NEXT_RECORD_LINK reads the record's first
     * word. */
    *out_next = dm2_v1_rd16(addr);
    return 1;
}

int dm2_v1_record_pool_append_to_list(DM2_V1_RecordPoolSet *set,
                                      int16_t *list_head_io,
                                      int16_t record)
{
    uint8_t *record_addr;
    int16_t cursor;

    if (set == NULL || list_head_io == NULL) {
        return 0;
    }
    /* c_record.cpp:68-70 — null/end records are never appended. */
    if (record == DM2_V1_RECORD_HANDLE_NULL ||
        record == DM2_V1_RECORD_HANDLE_END) {
        return 0;
    }
    record_addr = dm2_v1_record_pool_address_mut(set, record);
    if (record_addr == NULL) {
        return 0;
    }
    /* c_record.cpp:71 — *dest = OBJECT_END_MARKER terminates the appended
     * record's own link before chaining. */
    dm2_v1_wr16(record_addr, DM2_V1_RECORD_HANDLE_END);

    /* c_record.cpp:75-79 (x < 0 path) — walk to the last link slot and
     * store the appended record there. */
    cursor = *list_head_io;
    if (cursor == DM2_V1_RECORD_HANDLE_END ||
        cursor == DM2_V1_RECORD_HANDLE_NULL) {
        *list_head_io = record;
        return 1;
    }
    for (;;) {
        uint8_t *cursor_addr =
            dm2_v1_record_pool_address_mut(set, cursor);
        int16_t next;

        if (cursor_addr == NULL) {
            return 0; /* unresolvable chain: fail-closed, no mutation */
        }
        next = dm2_v1_rd16(cursor_addr);
        if (next == DM2_V1_RECORD_HANDLE_END) {
            dm2_v1_wr16(cursor_addr, record);
            return 1;
        }
        if (next == DM2_V1_RECORD_HANDLE_NULL) {
            return 0;
        }
        cursor = next;
    }
}

int dm2_v1_record_pool_cut_from_list(DM2_V1_RecordPoolSet *set,
                                     int16_t *list_head_io,
                                     int16_t record)
{
    int16_t cursor;
    int16_t previous;

    if (set == NULL || list_head_io == NULL ||
        record == DM2_V1_RECORD_HANDLE_NULL ||
        record == DM2_V1_RECORD_HANDLE_END) {
        return 0;
    }
    cursor = *list_head_io;
    previous = DM2_V1_RECORD_HANDLE_NULL;
    while (cursor != DM2_V1_RECORD_HANDLE_END &&
           cursor != DM2_V1_RECORD_HANDLE_NULL) {
        const uint8_t *cursor_addr =
            dm2_v1_record_pool_address(set, cursor);
        int16_t next;

        if (cursor_addr == NULL) {
            return 0;
        }
        next = dm2_v1_rd16(cursor_addr);
        if (cursor == record) {
            /* c_record.cpp:122+ list path — splice the successor into the
             * predecessor's link slot (or the list head). */
            if (previous == DM2_V1_RECORD_HANDLE_NULL) {
                *list_head_io = next;
            } else {
                uint8_t *prev_addr =
                    dm2_v1_record_pool_address_mut(set, previous);
                if (prev_addr == NULL) {
                    return 0;
                }
                dm2_v1_wr16(prev_addr, next);
            }
            return 1;
        }
        previous = cursor;
        cursor = next;
    }
    return 0;
}

int dm2_v1_record_pool_cut_from_tile(DM2_V1_RecordPoolSet *set,
                                     DM2_V1_DungeonData *dungeon,
                                     int map, int x, int y,
                                     int16_t record)
{
    int first;
    int16_t head;

    if (set == NULL || dungeon == NULL ||
        record == DM2_V1_RECORD_HANDLE_NULL ||
        record == DM2_V1_RECORD_HANDLE_END) {
        return 0;
    }
    first = dm2_v1_dungeon_get_first_thing(dungeon, map, x, y);
    if (first < 0) {
        return 0;
    }
    head = (int16_t)first;
    if (!dm2_v1_record_pool_cut_from_list(set, &head, record)) {
        return 0;
    }
    dm2_v1_dungeon_set_first_thing(dungeon, map, x, y, (uint16_t)head);
    return 1;
}

int dm2_v1_record_pool_relocate(DM2_V1_RecordPoolSet *set,
                                int16_t *from_head_io,
                                int16_t *to_head_io,
                                int16_t record)
{
    /* c_moverec.cpp DM2_MOVE_RECORD_TO is cut-then-append.  Only the list
     * paths are admitted here; tile-rooted relocation (c_map ground stacks)
     * stays rejected until that link state is proven. */
    if (!dm2_v1_record_pool_cut_from_list(set, from_head_io, record)) {
        return 0;
    }
    if (!dm2_v1_record_pool_append_to_list(set, to_head_io, record)) {
        return 0;
    }
    return 1;
}

int dm2_v1_record_pool_set_init_from_dungeon(DM2_V1_RecordPoolSet *set,
                                             const DM2_V1_DungeonData *d)
{
    DM2_V1_G1RecordPoolEvidence evidence;
    int cursor_ok = 1;

    if (set == NULL) {
        return 0;
    }
    memset(set, 0, sizeof(*set));
    if (d == NULL || d->raw_data == NULL ||
        !dm2_v1_dungeon_collect_g1_record_pool_evidence(d, &evidence) ||
        !evidence.available) {
        return 0;
    }

    for (int type = 0; type < DM2_V1_RECORD_POOL_COUNT; ++type) {
        DM2_V1_RecordPool *p = &set->pools[type];
        int count = d->thing_type_counts[type];
        int size = (int)s_table_recordsizes[type];
        long bytes;

        p->record_size = size;
        p->source_base = evidence.candidate_pool_bases[type];
        if (count <= 0 || size <= 0 || p->source_base < 0) {
            continue;
        }
        bytes = (long)count * size;
        if (p->source_base + bytes > d->raw_size) {
            cursor_ok = 0;
            break;
        }
        p->bytes = (uint8_t *)malloc((size_t)bytes);
        if (p->bytes == NULL) {
            dm2_v1_record_pool_set_free(set);
            return 0;
        }
        memcpy(p->bytes, d->raw_data + p->source_base, (size_t)bytes);
        p->record_count = count;

        /* Proven PC G1 continuation runs (DB3 at the ObjectID ceiling, then
         * DB4 at 300 rows) extend the same source pools; see
         * dm2_v1_dungeon_loader.c dm2_v1_configure_pc_g1_extension_records
         * and skproject c_record.cpp pool addressing. */
        if (d->g1_extension_record_bases[type] >= 0 &&
            d->g1_extension_record_counts[type] > 0) {
            long ext_bytes = (long)d->g1_extension_record_counts[type] * size;
            if (d->g1_extension_record_bases[type] + ext_bytes > d->raw_size) {
                cursor_ok = 0;
                break;
            }
            p->extension_bytes = (uint8_t *)malloc((size_t)ext_bytes);
            if (p->extension_bytes == NULL) {
                dm2_v1_record_pool_set_free(set);
                return 0;
            }
            memcpy(p->extension_bytes,
                   d->raw_data + d->g1_extension_record_bases[type],
                   (size_t)ext_bytes);
            p->extension_count = d->g1_extension_record_counts[type];
            p->extension_base = d->g1_extension_record_bases[type];
        }
    }
    if (!cursor_ok) {
        dm2_v1_record_pool_set_free(set);
        return 0;
    }
    set->valid = 1;
    set->record_graph_complete = d->record_graph_complete;
    return 1;
}

int dm2_v1_record_pool_set_init_from_raw_sksave(
    DM2_V1_RecordPoolSet *set,
    const uint8_t *raw_body,
    size_t raw_body_size,
    const DM2_V1_OriginalRawDungeonReceipt *dungeon_receipt)
{
    int pool;

    if (set == NULL) {
        return 0;
    }
    memset(set, 0, sizeof(*set));
    if (raw_body == NULL || dungeon_receipt == NULL ||
        !dungeon_receipt->valid ||
        dungeon_receipt->suppress_state_offset == 0u ||
        dungeon_receipt->suppress_state_offset > raw_body_size) {
        return 0;
    }

    /* READ_DUNGEON_STRUCTURE has already laid out every DB pool before
     * DM2_GAME_LOAD starts the one shared SUPPRESS stream.  Do not derive
     * offsets from a convenient local cursor: the raw-dungeon receipt owns
     * each original pool boundary and its hash. */
    for (pool = 0; pool < DM2_V1_RECORD_POOL_COUNT; ++pool) {
        DM2_V1_RecordPool *target = &set->pools[pool];
        const int record_size = (int)s_table_recordsizes[pool];
        const size_t count = dungeon_receipt->db_record_counts[pool];
        const size_t offset = dungeon_receipt->db_pool_offsets[pool];
        const size_t bytes = count * (size_t)record_size;

        target->record_size = record_size;
        target->source_base = -1;
        target->extension_base = -1;

        /* DB11..DB13 are unallocated by c_record.cpp.  A source receipt
         * naming records in one of those pools is contradictory, not an
         * invitation to invent a storage width. */
        if (record_size == 0) {
            if (count != 0u) {
                dm2_v1_record_pool_set_free(set);
                return 0;
            }
            continue;
        }
        if (count == 0u) {
            continue;
        }
        if (count > SIZE_MAX / (size_t)record_size ||
            offset > (size_t)INT_MAX ||
            offset > dungeon_receipt->suppress_state_offset ||
            bytes > dungeon_receipt->suppress_state_offset - offset ||
            offset > raw_body_size || bytes > raw_body_size - offset) {
            dm2_v1_record_pool_set_free(set);
            return 0;
        }
        /* The receipt's pool identity prevents a structurally plausible
         * truncated body from being materialized as a live DB baseline. */
        {
            uint32_t hash = 2166136261u;
            size_t i;
            for (i = 0u; i < bytes; ++i) {
                hash ^= raw_body[offset + i];
                hash *= 16777619u;
            }
            if (hash != dungeon_receipt->db_pool_hashes[pool]) {
                dm2_v1_record_pool_set_free(set);
                return 0;
            }
        }
        target->bytes = (uint8_t *)malloc(bytes);
        if (target->bytes == NULL) {
            dm2_v1_record_pool_set_free(set);
            return 0;
        }
        memcpy(target->bytes, raw_body + offset, bytes);
        target->record_count = (int)count;
        target->source_base = (int)offset;
    }

    set->valid = 1;
    /* The raw prefix is only the exact DB baseline.  GAME_LOAD still calls
     * DM2_READ_SKSAVE_DUNGEON to clear/rebuild dynamic records and attach
     * every saved link, so this must never pass a complete-graph gate. */
    set->record_graph_complete = 0;
    return 1;
}

int dm2_v1_record_pool_clear_raw_sksave_dynamic_records(
    DM2_V1_RecordPoolSet *set,
    const DM2_V1_OriginalRawDungeonReceipt *dungeon_receipt)
{
    int pool;

    if (set == NULL || dungeon_receipt == NULL || !set->valid ||
        set->record_graph_complete != 0 || !dungeon_receipt->valid) {
        return 0;
    }

    /* Validate the full baseline first.  The source loop is destructive, so
     * a contradictory receipt must not partially clear a caller's pool set. */
    for (pool = 0; pool < DM2_V1_RECORD_POOL_COUNT; ++pool) {
        const DM2_V1_RecordPool *source = &set->pools[pool];
        const int record_size = (int)s_table_recordsizes[pool];
        const uint16_t count = dungeon_receipt->db_record_counts[pool];

        if (source->record_size != record_size ||
            source->record_count != (int)count ||
            source->extension_bytes != NULL || source->extension_count != 0 ||
            source->extension_base != -1) {
            return 0;
        }
        if (record_size == 0) {
            if (count != 0u || source->bytes != NULL) return 0;
        } else if (count == 0u) {
            if (source->bytes != NULL) return 0;
        } else if (source->bytes == NULL) {
            return 0;
        }
    }

    /* SKProject sksvgame.cpp:1151-1164.  DB0..DB3 remain resident because
     * their tile-chain ownership was retained during the preceding map walk.
     * The dynamic pools are made available to c_record::ALLOC_NEW_RECORD by
     * writing only GenericRecord::w0 = OBJECT_NULL; the rest of each source
     * record is intentionally left alone until the source SUPPRESS masks
     * overwrite it during READ_RECORD_CHECKCODE. */
    for (pool = 4; pool < DM2_V1_RECORD_POOL_COUNT; ++pool) {
        DM2_V1_RecordPool *target = &set->pools[pool];
        int index;

        for (index = 0; index < target->record_count; ++index) {
            uint8_t *record = target->bytes +
                (size_t)index * (size_t)target->record_size;
            dm2_v1_wr16(record, DM2_V1_RECORD_HANDLE_NULL);
        }
    }
    return 1;
}

int dm2_v1_record_pool_restore_raw_sksave_direct_roots(
    DM2_V1_RecordPoolSet *set,
    const uint8_t *raw_body,
    size_t raw_body_size,
    const DM2_V1_OriginalRawSaveStateReceipt *state_receipt,
    DM2_ReadRecordCreatureAiFlagsFn query_creature_ai_flags,
    void *query_creature_ai_flags_ctx,
    DM2_V1_SksaveDirectRootReceipt *out_receipt)
{
    DM2_V1_SksavePoolRestoreContext context;
    DM2_ReadRecordCallbacks callbacks;
    DM2_ReadRecordSession session;
    uint16_t roots[DM2_V1_SKSAVE_DIRECT_ROOT_MAX];
    size_t root_count;
    size_t root;

#define DM2_V1_SKSAVE_ROOT_ABORT() do { \
        /* The preceding map-detach phase cannot be replayed from this
         * direct-root helper, so a decode error must not leave a partly
         * restored pool available to a caller. */ \
        dm2_v1_record_pool_set_free(set); \
    } while (0)

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!set || !set->valid || set->record_graph_complete != 0 ||
        !raw_body || !state_receipt || !state_receipt->valid ||
        !query_creature_ai_flags ||
        state_receipt->record_link_bitstream_offset > raw_body_size ||
        state_receipt->record_link_bitstream_bits_remaining > 7u) {
        return 0;
    }
    root_count = (size_t)state_receipt->champion_count * 30u + 1u;
    if (root_count == 0u || root_count > DM2_V1_SKSAVE_DIRECT_ROOT_MAX) {
        return 0;
    }

    memset(&context, 0, sizeof(context));
    context.set = set;
    context.record_hash = 2166136261u;
    context.ai_fn = query_creature_ai_flags;
    context.ai_ctx = query_creature_ai_flags_ctx;
    memset(&callbacks, 0, sizeof(callbacks));
    callbacks.alloc_record = dm2_v1_sksave_pool_alloc;
    callbacks.set_data = dm2_v1_sksave_pool_set_data;
    callbacks.append_record = dm2_v1_sksave_pool_append;
    callbacks.child_owner = dm2_v1_sksave_pool_child_owner;
    callbacks.add_possession_index = dm2_v1_sksave_pool_add_possession;
    callbacks.query_creature_ai_flags = dm2_v1_sksave_pool_query_ai;
    callbacks.ctx = &context;

    memset(&session, 0, sizeof(session));
    dm2_v1_read_record_session_init(&session, raw_body, raw_body_size);
    session.reader.position = state_receipt->record_link_bitstream_offset;
    session.reader.bits_remaining =
        state_receipt->record_link_bitstream_bits_remaining;
    if (session.reader.bits_remaining != 0u) {
        if (session.reader.position == 0u) { DM2_V1_SKSAVE_ROOT_ABORT(); return 0; }
        session.reader.current_byte = (uint8_t)(
            raw_body[session.reader.position - 1u] <<
            (8u - session.reader.bits_remaining));
    }
    for (root = 0u; root < root_count; ++root) {
        roots[root] = (uint16_t)DM2_V1_RECORD_HANDLE_END;
        if (dm2_v1_read_record_checkcode(&session, &callbacks, &roots[root],
                                         -1, 0, 0, 0) != 0 || session.error) {
            DM2_V1_SKSAVE_ROOT_ABORT();
            return 0;
        }
    }

    if (out_receipt) {
        out_receipt->valid = 1;
        out_receipt->root_count = (uint16_t)root_count;
        memcpy(out_receipt->roots, roots, root_count * sizeof(roots[0]));
        out_receipt->record_count = context.record_count;
        /* Source order: DM2_READ_SKSAVE_DUNGEON first restores the direct
         * hero/cursor roots, then special timer chains and every map chain.
         * DM2_2066_062b consumes possession continuations only afterwards
         * (SKProject sksvgame.cpp:1178-1400).  Reading them here used map
         * bits as fake continuations and corrupted the shared stream. */
        out_receipt->possession_continuation_count = 0u;
        out_receipt->record_hash = context.record_hash;
        out_receipt->continuation_hash = 0u;
        out_receipt->next_stream_offset = session.reader.position;
        out_receipt->next_stream_bits_remaining = session.reader.bits_remaining;
        out_receipt->next_stream_current_byte = session.reader.current_byte;
    }
#undef DM2_V1_SKSAVE_ROOT_ABORT
    return 1;
}

int dm2_v1_record_pool_preflight_raw_sksave_special_timer_chains(
    const uint8_t *raw_body,
    size_t raw_body_size,
    const DM2_V1_OriginalRawSaveStateReceipt *state_receipt,
    uint16_t savegamew7,
    DM2_ReadRecordCreatureAiFlagsFn query_creature_ai_flags,
    void *query_creature_ai_flags_ctx,
    DM2_V1_SksaveSpecialTimerReceipt *out_receipt)
{
    DM2_V1_RecordPoolSet pools;
    DM2_V1_SksaveMapOwner map_owner;
    DM2_V1_SksaveDirectRootReceipt roots;
    DM2_V1_SksavePoolRestoreContext context;
    DM2_ReadRecordCallbacks callbacks;
    DM2_ReadRecordSession session;
    DM2_V1_SaveTimerRecord *timers = NULL;
    DM2_V1_OriginalRawTimerStreamReceipt timer_stream;
    uint16_t special_count = 0u;
    uint32_t timer_hash = 2166136261u;
    uint16_t i;
    int ok = 0;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    memset(&pools, 0, sizeof(pools));
    memset(&map_owner, 0, sizeof(map_owner));
    memset(&roots, 0, sizeof(roots));
    memset(&context, 0, sizeof(context));
    memset(&callbacks, 0, sizeof(callbacks));
    memset(&session, 0, sizeof(session));
    memset(&timer_stream, 0, sizeof(timer_stream));
    if (!raw_body || !state_receipt || !state_receipt->valid ||
        !query_creature_ai_flags || !out_receipt || savegamew7 == 0u ||
        state_receipt->timer_count > 4096u ||
        !dm2_v1_record_pool_set_init_from_raw_sksave(
            &pools, raw_body, raw_body_size, &state_receipt->dungeon) ||
        !dm2_v1_sksave_map_owner_init(
            &map_owner, raw_body, raw_body_size, &state_receipt->dungeon) ||
        !dm2_v1_sksave_map_owner_detach_dynamic_records(
            &map_owner, &pools, NULL) ||
        !dm2_v1_record_pool_clear_raw_sksave_dynamic_records(
            &pools, &state_receipt->dungeon) ||
        !dm2_v1_record_pool_restore_raw_sksave_direct_roots(
            &pools, raw_body, raw_body_size, state_receipt,
            query_creature_ai_flags, query_creature_ai_flags_ctx, &roots)) {
        goto done;
    }
    if (state_receipt->timer_count != 0u) {
        timers = (DM2_V1_SaveTimerRecord *)calloc(
            state_receipt->timer_count, sizeof(*timers));
        if (!timers || !dm2_v1_original_raw_sksave_decode_timer_stream(
                raw_body, raw_body_size, state_receipt,
                (uint8_t *)timers, state_receipt->timer_count,
                &timer_stream) || !timer_stream.valid) {
            goto done;
        }
    }
    dm2_v1_read_record_session_init(&session, raw_body, raw_body_size);
    session.reader.position = roots.next_stream_offset;
    session.reader.bits_remaining = roots.next_stream_bits_remaining;
    session.reader.current_byte = roots.next_stream_current_byte;
    context.set = &pools;
    context.record_hash = 2166136261u;
    context.ai_fn = query_creature_ai_flags;
    context.ai_ctx = query_creature_ai_flags_ctx;
    callbacks.alloc_record = dm2_v1_sksave_pool_alloc;
    callbacks.set_data = dm2_v1_sksave_pool_set_data;
    callbacks.append_record = dm2_v1_sksave_pool_append;
    callbacks.child_owner = dm2_v1_sksave_pool_child_owner;
    callbacks.add_possession_index = dm2_v1_sksave_pool_add_possession;
    callbacks.query_creature_ai_flags = dm2_v1_sksave_pool_query_ai;
    callbacks.ctx = &context;
    if (dm2_v1_read_special_timer_record_chains(
            &session, &callbacks, timers, state_receipt->timer_count,
            savegamew7, &special_count) != 0 || session.error) {
        goto done;
    }
    for (i = 0u; i < state_receipt->timer_count; ++i) {
        timer_hash = dm2_v1_sksave_hash_bytes(
            timer_hash, timers[i].bytes, sizeof(timers[i].bytes));
    }
    out_receipt->valid = 1;
    out_receipt->timer_count = state_receipt->timer_count;
    out_receipt->special_chain_count = special_count;
    out_receipt->timer_hash = timer_hash ? timer_hash : 1u;
    out_receipt->record_hash = context.record_hash;
    out_receipt->next_stream_offset = session.reader.position;
    out_receipt->next_stream_bits_remaining = session.reader.bits_remaining;
    out_receipt->next_stream_current_byte = session.reader.current_byte;
    ok = 1;
done:
    free(timers);
    dm2_v1_sksave_map_owner_free(&map_owner);
    dm2_v1_record_pool_set_free(&pools);
    return ok;
}

void dm2_v1_record_pool_set_free(DM2_V1_RecordPoolSet *set)
{
    if (set == NULL) {
        return;
    }
    for (int i = 0; i < DM2_V1_RECORD_POOL_COUNT; ++i) {
        free(set->pools[i].bytes);
        free(set->pools[i].extension_bytes);
        set->pools[i].bytes = NULL;
        set->pools[i].extension_bytes = NULL;
        set->pools[i].record_count = 0;
        set->pools[i].extension_count = 0;
        set->pools[i].source_base = -1;
        set->pools[i].extension_base = -1;
    }
    set->valid = 0;
    set->record_graph_complete = 0;
}

const char *dm2_v1_record_pool_source_evidence(void)
{
    return
        "DM2 V1 c_record pool ownership — skproject source-lock\n"
        "Source: skproject/SKULLWIN/c_record.cpp:28-31 (table_recordsizes)\n"
        "Source: skproject/SKULLWIN/c_record.cpp:44-52 (GET_ADDRESS_OF_RECORD)\n"
        "Source: skproject/SKULLWIN/c_record.cpp:54-57 (GET_NEXT_RECORD_LINK)\n"
        "Source: skproject/SKULLWIN/c_record.cpp:60-170 (APPEND/CUT list paths)\n"
        "Source: skproject/SKULLWIN/c_moverec.cpp (DM2_MOVE_RECORD_TO)\n"
        "Source: skproject/SKULLWIN/c_dballoc.cpp (pool ownership)\n"
        "Source: skproject/SKWIN/SkWinCore.cpp (READ_DUNGEON_STRUCTURE)\n";
}
