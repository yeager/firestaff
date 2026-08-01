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
    if (p->record_size <= 0 || p->bytes == NULL ||
        index < 0 || index >= p->record_count) {
        return NULL;
    }
    /* c_record.cpp:48-51 — ofs = recordsize[idx] * (r & 0x3ff) added to the
     * pool base as a byte offset. */
    return p->bytes + (size_t)index * (size_t)p->record_size;
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
