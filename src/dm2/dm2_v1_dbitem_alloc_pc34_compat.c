/*
 * dm2_v1_dbitem_alloc_pc34_compat.c — DM2 V1 ALLOC_NEW_DBITEM drop path
 * over the DM2-002 record pool (DM2-006 follow-up).
 *
 * Source: skproject/SKULLWIN/c_record.cpp:367-401   (GET_ITEMDB_OF_ITEMSPEC_
 *                                                     ACTUATOR)
 *         skproject/SKULLWIN/c_record.cpp:403-444   (GET_ITEMTYPE_OF_ITEMSPEC_
 *                                                     ACTUATOR)
 *         skproject/SKULLWIN/c_record.cpp:1076-1139 (ALLOC_NEW_RECORD)
 *         skproject/SKULLWIN/c_record.cpp:284-345   (SET_ITEMTYPE)
 *         skproject/SKULLWIN/c_record.cpp:1142-1165 (ALLOC_NEW_DBITEM)
 *         skproject/SKULLWIN/c_record.cpp:1537-1634 (DROP_CREATURE_POSSESSION
 *                                                     generated-drops loop)
 *         skproject/SKULLWIN/c_random.cpp:13-47     (RAND16/RANDBIT/RANDDIR)
 */

#include "dm2_v1_dbitem_alloc_pc34_compat.h"
#include "dm2_v1_record_ops_pc34_compat.h"

#include <string.h>

#define DM2_V1_DB_CREATURE 4
#define DM2_V1_DB_WEAPON 5
#define DM2_V1_DB_CLOTH 6
#define DM2_V1_DB_SCROLL 7
#define DM2_V1_DB_POTION 8
#define DM2_V1_DB_CONTAINER 9
#define DM2_V1_DB_MISC 10

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

int16_t dm2_v1_record_pool_alloc_new_record(DM2_V1_RecordPoolSet *set,
                                            uint16_t db_request)
{
    DM2_V1_RecordPool *pool;
    int db;
    int scan_count;
    int index;

    if (set == NULL || !set->valid) {
        return DM2_V1_RECORD_HANDLE_NULL;
    }

    /* c_record.cpp:1086-1101 — db = db_request with bit 15 cleared
     * (RG1Bhi &= 0x7f); the bones request 0x800A maps to dbMisc WITHOUT
     * the reserve, plain dbMisc reserves the last 3 records. */
    db = (int)(db_request & 0x7fffu);
    if (db < 0 || db >= DM2_V1_RECORD_POOL_COUNT) {
        return DM2_V1_RECORD_HANDLE_NULL;
    }
    pool = &set->pools[db];
    if (pool->record_size <= 0 || pool->bytes == NULL) {
        return DM2_V1_RECORD_HANDLE_NULL;
    }
    scan_count = pool->record_count;
    if (db_request == 0x800au) {
        /* bones: full pool, db already resolved to dbMisc below */
    } else if (db == DM2_V1_DB_MISC) {
        scan_count -= 3;
    }
    if (scan_count < 0) {
        scan_count = 0;
    }

    /* c_record.cpp:1105-1130 — forward scan for a slot whose first word is
     * OBJECT_NULL (0xffff); the allocated index is the number of in-use
     * records skipped.  Pool exhaustion hits the source's
     * DM2_RECYCLE_A_RECORD_FROM_THE_WORLD, a full world walk that stays
     * unproven: fail-closed OBJECT_NULL here. */
    for (index = 0; index < scan_count; ++index) {
        uint8_t *addr =
            pool->bytes + (size_t)index * (size_t)pool->record_size;
        if (dm2_v1_rd16(addr) != DM2_V1_RECORD_HANDLE_NULL) {
            continue;
        }
        /* c_record.cpp:1131-1139 — zero the slot, terminate its own link;
         * dbContainer also terminates its content link (w2). */
        memset(addr, 0, (size_t)pool->record_size);
        dm2_v1_wr16(addr, DM2_V1_RECORD_HANDLE_END);
        if (db == DM2_V1_DB_CONTAINER) {
            dm2_v1_wr16(addr + 2, DM2_V1_RECORD_HANDLE_END);
        }
        return (int16_t)((db << 10) | (index & 0x3ff));
    }
    return DM2_V1_RECORD_HANDLE_NULL;
}

int dm2_v1_record_pool_set_itemtype(DM2_V1_RecordPoolSet *set,
                                    int16_t record, uint8_t itemtype)
{
    uint8_t *addr;
    int db;

    if (set == NULL || !set->valid) {
        return 0;
    }
    /* c_record.cpp:290-291 source guards: OBJECT_NULL or handle >= 0xff80
     * returns without a write. */
    if (record == DM2_V1_RECORD_HANDLE_NULL ||
        (uint16_t)record >= 0xff80u) {
        return 0;
    }
    addr = dm2_v1_record_pool_address_mut(set, record);
    if (addr == NULL) {
        return 0;
    }
    /* c_record.cpp:294-297 — db = (handle & 0x3c00) >> 10; only db 4..10
     * take an itemtype. */
    db = (int)(((uint16_t)record & 0x3c00u) >> 10);
    if (db < 4 || db > 10) {
        return 0;
    }
    switch (db - 4) {
        case 0: /* db4 Creature: byte@4 = type (c_record.cpp:306-309) */
            addr[4] = itemtype;
            return 1;
        case 1: /* db5 Weapon, db6 Cloth, db10 Misc: word@2 low 7 bits */
        case 2:
        case 6:
            addr[2] &= 0x80u;
            {
                uint16_t w = (uint16_t)dm2_v1_rd16(addr + 2);
                w |= (uint16_t)(itemtype & 0x7fu);
                dm2_v1_wr16(addr + 2, (int16_t)w);
            }
            return 1;
        case 3: /* db7 Scroll: source returns without a write */
            return 1;
        case 4: /* db8 Potion: word@2 high 7 bits (byte@3) */
            addr[3] &= 0x80u;
            {
                uint16_t w = (uint16_t)dm2_v1_rd16(addr + 2);
                w |= (uint16_t)((itemtype & 0x7fu) << 8);
                dm2_v1_wr16(addr + 2, (int16_t)w);
            }
            return 1;
        case 5: /* db9 Container: charge split over word@4
                 * (c_record.cpp:325-341) */
            {
                uint16_t w = (uint16_t)dm2_v1_rd16(addr + 4);
                addr[4] &= 0xf9u;
                w = (uint16_t)dm2_v1_rd16(addr + 4);
                w |= (uint16_t)(((itemtype / 8u) & 3u) << 1);
                addr[5] &= 0x1fu;
                w = (uint16_t)((w & 0x1fffu) |
                               ((uint16_t)(itemtype & 7u) << 13));
                dm2_v1_wr16(addr + 4, (int16_t)w);
                if ((w & 0x6u) == 2u) {
                    dm2_v1_wr16(addr + 6, DM2_V1_RECORD_HANDLE_NULL);
                }
            }
            return 1;
        default:
            return 0;
    }
}

int16_t dm2_v1_alloc_new_dbitem(DM2_V1_RecordPoolSet *set,
                                uint16_t itemspec)
{
    int db;
    int16_t record;

    if (set == NULL || !set->valid) {
        return DM2_V1_RECORD_HANDLE_NULL;
    }
    /* c_record.cpp:1148-1154 — db | (itemspec & 0x8000). */
    db = dm2_v1_get_itemdb_of_itemspec_actuator(itemspec);
    if (db < 0) {
        return DM2_V1_RECORD_HANDLE_NULL;
    }
    record = dm2_v1_record_pool_alloc_new_record(
        set, (uint16_t)(db | (itemspec & 0x8000u)));
    if (record == DM2_V1_RECORD_HANDLE_NULL) {
        return DM2_V1_RECORD_HANDLE_NULL;
    }
    /* c_record.cpp:1156-1162 — SET_ITEMTYPE on the fresh record. */
    (void)dm2_v1_record_pool_set_itemtype(
        set, record,
        (uint8_t)dm2_v1_get_itemtype_of_itemspec_actuator(itemspec));
    return record;
}

int dm2_v1_drops_place_source_slots(
    DM2_V1_RecordPoolSet *set,
    const uint16_t slot_words[DM2_DROP_SLOT_COUNT],
    DM2_V1_DropRng *rng,
    int party_x, int party_y, int party_dir,
    int drop_x, int drop_y,
    int16_t *ground_head_io,
    DM2_V1_DropPlacedItem *out_items,
    int max_items,
    int *out_item_count)
{
    int placed = 0;
    int iterations = 0;
    int at_party;

    if (out_item_count != NULL) {
        *out_item_count = 0;
    }
    if (set == NULL || !set->valid || slot_words == NULL || rng == NULL) {
        return 0;
    }
    at_party = (drop_x == party_x && drop_y == party_y) ? 1 : 0;

    /* c_record.cpp:1568-1634 — slots 0x0A..0x14 ascending; per admitted
     * slot the count roll is consumed first, then the per-item loop runs
     * ALLOC_NEW_DBITEM -> direction draw -> MOVE_RECORD_TO, so the RNG
     * stream interleaves exactly like the source. */
    for (int slot = 0; slot < DM2_DROP_SLOT_COUNT; ++slot) {
        uint16_t word = slot_words[slot];
        int count;
        int extra_range;
        uint16_t itemspec;

        if (word == 0) {
            continue; /* source `continue` on a zero slot word */
        }
        count = (int)(word & 0x0fu) + 1;
        extra_range = (int)((word & 0x0070u) >> 4);
        if (extra_range != 0) {
            count += (int)dm2_v1_drops_rand16(rng,
                                              (uint16_t)(extra_range + 1));
        }
        itemspec = (uint16_t)(word >> 7);

        for (int ordinal = 0; ordinal < count; ++ordinal) {
            DM2_V1_DropPlacedItem item;
            int16_t record;
            int16_t placed_record;

            memset(&item, 0, sizeof(item));
            item.slot_field = DM2_DROP_SLOT_FIRST + slot;
            item.item_ordinal = ordinal;
            item.itemspec = itemspec;
            item.item_db = -1;
            item.item_type = -1;
            item.record = DM2_V1_RECORD_HANDLE_NULL;
            item.at_party_cell = at_party;
            item.direction_rand = -1;
            item.direction = -1;

            /* c_record.cpp:1603-1608 — ALLOC_NEW_DBITEM; OBJECT_NULL
             * breaks the slot loop BEFORE the direction draw. */
            record = dm2_v1_alloc_new_dbitem(set, itemspec);
            if (record == DM2_V1_RECORD_HANDLE_NULL) {
                item.alloc_failed = 1;
                if (out_items != NULL && iterations < max_items) {
                    out_items[iterations] = item;
                }
                ++iterations;
                break;
            }
            item.item_db = dm2_v1_get_itemdb_of_itemspec_actuator(itemspec);
            item.item_type =
                dm2_v1_get_itemtype_of_itemspec_actuator(itemspec);

            /* c_record.cpp:1609-1622 — direction draw: on the party cell
             * (party_dir + RANDBIT) & 3, elsewhere RANDDIR. */
            if (at_party) {
                item.direction_rand = (int)dm2_v1_drops_randbit(rng);
                item.direction = (party_dir + item.direction_rand) & 3;
            } else {
                item.direction_rand = (int)dm2_v1_drops_randdir(rng);
                item.direction = item.direction_rand;
            }

            /* c_record.cpp:1623-1631 — the record word carries the
             * direction (dir << 14 | handle & 0x3fff); the bounded
             * from-nowhere MOVE_RECORD_TO appends to the caller-owned
             * destination list (tile-rooted mutation unproven). */
            placed_record = (int16_t)(((uint16_t)item.direction << 14) |
                                      ((uint16_t)record & 0x3fffu));
            item.record = placed_record;
            if (ground_head_io != NULL &&
                dm2_v1_record_pool_append_to_list(set, ground_head_io,
                                                  placed_record)) {
                item.placed = 1;
                ++placed;
            }
            if (out_items != NULL && iterations < max_items) {
                out_items[iterations] = item;
            }
            ++iterations;
        }
    }

    if (out_item_count != NULL) {
        *out_item_count = iterations;
    }
    return placed;
}

const char *dm2_v1_dbitem_alloc_source_evidence(void)
{
    return
        "DM2 V1 ALLOC_NEW_DBITEM drop path — skproject source-lock\n"
        "Source: skproject/SKULLWIN/c_record.cpp:367-401 (GET_ITEMDB_OF_ITEMSPEC_ACTUATOR)\n"
        "Source: skproject/SKULLWIN/c_record.cpp:403-444 (GET_ITEMTYPE_OF_ITEMSPEC_ACTUATOR)\n"
        "Source: skproject/SKULLWIN/c_record.cpp:1076-1139 (ALLOC_NEW_RECORD, bones 0x800A, dbMisc reserve)\n"
        "Source: skproject/SKULLWIN/c_record.cpp:284-345 (SET_ITEMTYPE per-DB writes)\n"
        "Source: skproject/SKULLWIN/c_record.cpp:1142-1165 (ALLOC_NEW_DBITEM)\n"
        "Source: skproject/SKULLWIN/c_record.cpp:1537-1634 (DROP_CREATURE_POSSESSION generated-drops loop)\n"
        "Source: skproject/SKULLWIN/c_random.cpp:13-47 (RAND16/RANDBIT/RANDDIR LCG)\n";
}
