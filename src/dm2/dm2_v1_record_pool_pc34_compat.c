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
