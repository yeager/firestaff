#ifndef FIRESTAFF_DM2_V1_DBITEM_ALLOC_PC34_COMPAT_H
#define FIRESTAFF_DM2_V1_DBITEM_ALLOC_PC34_COMPAT_H

/*
 * dm2_v1_dbitem_alloc_pc34_compat.h — DM2 V1 ALLOC_NEW_DBITEM drop path
 * over the DM2-002 record pool (DM2-006 follow-up).
 *
 * Binds the skproject drop-creation chain verbatim so resolved creature
 * drops become real DB item records:
 *
 *   c_record.cpp:367-401   DM2_GET_ITEMDB_OF_ITEMSPEC_ACTUATOR: itemspec
 *                          & 0x1ff; >>7 selects dbWeapon(5)/dbCloth(6)/
 *                          dbMisc(10); group 3 splits 0x1fc scroll(7),
 *                          >=0x1e0 container(9), >=0x1b0 creature(4),
 *                          else potion(8); >0x1fc invalid.
 *   c_record.cpp:403-444   DM2_GET_ITEMTYPE_OF_ITEMSPEC_ACTUATOR: group
 *                          base subtraction (0x00/0x80/0x100; group 3:
 *                          0x1fc->0, 0x1e0/0x1b0/0x180 bases).
 *   c_record.cpp:1076-1139 DM2_ALLOC_NEW_RECORD: forward pool scan for a
 *                          slot whose w0 is OBJECT_NULL, 0x800A bones map
 *                          to dbMisc without the 3-record reserve, plain
 *                          dbMisc reserves the last 3 records, the slot is
 *                          zeroed and w0 becomes OBJECT_END_MARKER
 *                          (dbContainer also w2), handle = (db << 10) |
 *                          index.  The source's
 *                          DM2_RECYCLE_A_RECORD_FROM_THE_WORLD fallback is
 *                          a full world walk and stays unproven: an
 *                          exhausted pool returns OBJECT_NULL fail-closed.
 *   c_record.cpp:284-345   DM2_SET_ITEMTYPE: per-DB itemtype write
 *                          (db5/6/10 word@2 low 7 bits, db8 word@2 high
 *                          7 bits, db9 container charge split over word@4,
 *                          db4 byte@4, db7 scroll no-op), guarded by
 *                          handle < 0xff80 and db 4..10.
 *   c_record.cpp:1142-1165 DM2_ALLOC_NEW_DBITEM: itemspec -> db |
 *                          (itemspec & 0x8000) -> ALLOC_NEW_RECORD ->
 *                          SET_ITEMTYPE.
 *   c_record.cpp:1568-1634 DM2_DROP_CREATURE_POSSESSION generated-drops
 *                          loop: per admitted slot, per item ALLOC_NEW_
 *                          DBITEM (OBJECT_NULL breaks the slot loop
 *                          WITHOUT a direction draw), then the direction
 *                          draw — (party_dir + RANDBIT) & 3 on the party
 *                          cell, RANDDIR elsewhere — then MOVE_RECORD_TO
 *                          with the direction folded into the record word
 *                          (dir << 14 | handle & 0x3fff).  The bounded
 *                          MOVE_RECORD_TO from-nowhere path (xposFrom=-1)
 *                          is cut-skip + DM2_APPEND_RECORD_TO; tile-rooted
 *                          ground-stack mutation stays unproven, so the
 *                          destination list head is caller-owned.
 */

#include <stdint.h>

#include "dm2_v1_drops.h"
#include "dm2_v1_record_pool_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

/* c_record.cpp:1076-1139 — allocate a zeroed record in pool
 * (db_request & 0x7fff); db_request 0x800A selects dbMisc without the
 * 3-record reserve.  Returns the direction-free handle (db << 10 | index)
 * or DM2_V1_RECORD_HANDLE_NULL when the pool is absent, the db is out of
 * range, or the pool is exhausted (recycle unproven, fail-closed). */
int16_t dm2_v1_record_pool_alloc_new_record(DM2_V1_RecordPoolSet *set,
                                            uint16_t db_request);

/* c_record.cpp:284-345 — per-DB itemtype write.  Returns 1 when the
 * source path executed (scroll is a deliberate no-op), 0 for the source
 * guards (OBJECT_NULL, handle >= 0xff80, db outside 4..10) or an
 * unresolvable record. */
int dm2_v1_record_pool_set_itemtype(DM2_V1_RecordPoolSet *set,
                                    int16_t record, uint8_t itemtype);

/* c_record.cpp:1142-1165 — ALLOC_NEW_DBITEM: allocate + SET_ITEMTYPE.
 * Returns the direction-free handle or DM2_V1_RECORD_HANDLE_NULL. */
int16_t dm2_v1_alloc_new_dbitem(DM2_V1_RecordPoolSet *set,
                                uint16_t itemspec);

typedef struct {
    int slot_field;       /* GDAT field 0x0A..0x14 */
    int item_ordinal;     /* 0-based within the slot's count loop */
    uint16_t itemspec;    /* slot word >> 7 */
    int item_db;          /* resolved DB 4..10, -1 when unresolvable */
    int item_type;        /* resolved item type, -1 when unresolvable */
    int16_t record;       /* allocated handle incl. direction bits, -1 fail */
    int alloc_failed;     /* OBJECT_NULL: source breaks the slot loop */
    int at_party_cell;    /* drop cell == party cell */
    int direction_rand;   /* raw RANDBIT/RANDDIR draw, -1 when not drawn */
    int direction;        /* final direction 0..3, -1 when not drawn */
    int placed;           /* appended to the destination list */
} DM2_V1_DropPlacedItem;

/* c_record.cpp:1568-1634 generated-drops loop: resolve each admitted slot
 * (word 0 skipped, base=(w&15)+1, extra=(w&0x70)>>4 with RAND16(extra+1),
 * itemspec = w>>7 — the same source order as
 * dm2_v1_drops_resolve_source_slots) and, per item, ALLOC_NEW_DBITEM +
 * the source direction draw + the bounded from-nowhere MOVE_RECORD_TO
 * (append to *ground_head_io, which the caller owns; pass an
 * OBJECT_END_MARKER head for an empty cell stack).
 *
 * RNG order is the source's interleaved order: slot extra roll, then that
 * slot's per-item direction draws, then the next slot.  Returns the number
 * of placed items; *out_item_count receives the total executed per-item
 * iterations (including alloc failures).  out_items receipts up to
 * max_items iterations; execution continues past the receipt capacity so
 * the RNG stream stays source-exact. */
int dm2_v1_drops_place_source_slots(
    DM2_V1_RecordPoolSet *set,
    const uint16_t slot_words[DM2_DROP_SLOT_COUNT],
    DM2_V1_DropRng *rng,
    int party_x, int party_y, int party_dir,
    int drop_x, int drop_y,
    int16_t *ground_head_io,
    DM2_V1_DropPlacedItem *out_items,
    int max_items,
    int *out_item_count);

const char *dm2_v1_dbitem_alloc_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_DBITEM_ALLOC_PC34_COMPAT_H */
