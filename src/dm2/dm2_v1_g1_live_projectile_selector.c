#include "dm2_v1_dungeon_loader.h"
#include "dm2_v1_record_pool_pc34_compat.h"
#include <string.h>

/* Resolve one linked ObjectID through the active pool snapshot first. The
 * dungeon owner is a fallback only for source DB15 special-object records,
 * which are the immutable effect table and are not allocated as live items.
 * This is deliberately separate from the
 * dungeon-only selector above: runtime-created DB14 records do not belong to
 * the original dungeon record span.
 *
 * SKProject c_record.cpp:44-52::DM2_GET_ADDRESS_OF_RECORD decodes the DB from
 * the ObjectID, then addresses that DB's active record pool. The runtime's
 * SHOOT_CHAMPION_MISSILE owner allocates its DB14 record in this pool set;
 * absent DB5..DB14 handles fail closed instead of reading stale disk data. */
static const uint8_t *dm2_v1_g1_live_resolve_object_record(
    const DM2_V1_DungeonData *dungeon,
    const DM2_V1_RecordPoolSet *record_pools,
    uint16_t object_id,
    int *out_type,
    int *out_index,
    int *out_size)
{
    int type = dm2_v1_record_handle_pool((int16_t)object_id);
    int index = dm2_v1_record_handle_index((int16_t)object_id);
    const uint8_t *record;

    if (out_type) *out_type = -1;
    if (out_index) *out_index = -1;
    if (out_size) *out_size = 0;
    if (type < 0 || type >= DM2_V1_RECORD_POOL_COUNT) return NULL;
    record = dm2_v1_record_pool_address(record_pools, (int16_t)object_id);
    if (record) {
        int size = record_pools->pools[type].record_size;
        if (size < 4) return NULL;
        if (out_type) *out_type = type;
        if (out_index) *out_index = index;
        if (out_size) *out_size = size;
        return record;
    }
    if (type != 15 || !dungeon) return NULL;
    return dm2_v1_dungeon_get_thing_record(
        dungeon, object_id, out_type, out_index, out_size);
}

int dm2_v1_g1_live_flying_item_selector_receipt(
    const DM2_V1_DungeonData *dungeon,
    const DM2_V1_RecordPoolSet *record_pools,
    const DM2_V1_AssetLoader *loader,
    uint16_t missile_object_id,
    DM2_V1_G1DirectMissileReceipt *out_missile,
    DM2_V1_G1FlyingItemSelectorReceipt *out_selector)
{
    static const uint8_t class1_by_type[16] = {
        0x0e, 0x18, 0xff, 0xff, 0x0f, 0x10, 0x11, 0x12,
        0x13, 0x14, 0x15, 0xff, 0xff, 0xff, 0xff, 0x0d };
    const uint8_t *missile_record;
    const uint8_t *object_record = NULL;
    DM2_V1_G1DirectMissileReceipt missile;
    DM2_V1_G1FlyingItemSelectorReceipt selector;
    uint32_t hash = 2166136261u;
    uint16_t object;
    int type = -1, index = -1, size = 0;

    if (out_missile) memset(out_missile, 0, sizeof(*out_missile));
    if (out_selector) memset(out_selector, 0, sizeof(*out_selector));
    memset(&missile, 0, sizeof(missile));
    memset(&selector, 0, sizeof(selector));
    if (!dungeon || !record_pools || !record_pools->valid ||
        !record_pools->record_graph_complete || !loader ||
        record_pools->source_words_big_endian !=
            (dungeon->words_big_endian != 0) ||
        dm2_v1_record_handle_pool((int16_t)missile_object_id) != 14 ||
        !(missile_record = dm2_v1_record_pool_address(
              record_pools, (int16_t)missile_object_id)) ||
        record_pools->pools[14].record_size != 8) {
        return 0;
    }

    /* DME.h::Missile owns bytes w2,b4,b5,w6; w0 is the active pool link and
     * must not be interpreted as part of the visual selector. */
    object = dm2_v1_dungeon_read_record_u16(dungeon, missile_record + 2);
    missile.object_id = missile_object_id;
    missile.missile_object = object;
    missile.energy_remaining = missile_record[4];
    missile.energy_remaining2 = missile_record[5];
    missile.timer_index = dm2_v1_dungeon_read_record_u16(
        dungeon, missile_record + 6);
    for (int byte = 2; byte < 8; ++byte) {
        hash = (hash ^ missile_record[byte]) * 16777619u;
    }
    missile.record_hash = hash ? hash : 1u;
    missile.valid = 1;

    /* SKProject SKULLWIN/c_record.cpp:203::DM2_QUERY_CLS2_FROM_RECORD handles
     * virtual spell ObjectIDs >= 0xff80 before DM2_GET_ADDRESS_OF_RECORD: their
     * selector is the low byte minus 0x80. They are not DB15 pool addresses.
     * Ordinary linked objects continue through the live pool resolver. */
    if (object >= 0xff80u) {
        type = 15;
        index = (int)((object & 0x00ffu) - 0x80u);
    } else {
        object_record = dm2_v1_g1_live_resolve_object_record(
            dungeon, record_pools, object, &type, &index, &size);
        if (!object_record || index < 0 || type < 0 || type > 15 || size < 4)
            return 0;
        if (type == 14) {
            object = dm2_v1_dungeon_read_record_u16(dungeon,
                                                    object_record + 2);
            if (object >= 0xff80u) {
                type = 15;
                index = (int)((object & 0x00ffu) - 0x80u);
                object_record = NULL;
            } else {
                object_record = dm2_v1_g1_live_resolve_object_record(
                    dungeon, record_pools, object, &type, &index, &size);
                if (!object_record || index < 0 || type < 0 ||
                    type > 15 || size < 4) return 0;
            }
        }
    }
    if (class1_by_type[type] == 0xffu) return 0;
    selector.class1 = class1_by_type[type];
    if (type == 15 && object >= 0xff80u)
        selector.class2 = (uint8_t)((object & 0x00ffu) - 0x80u);
    else if (type == 4) selector.class2 = object_record[4];
    else if (type == 5 || type == 6 || type == 10 || type == 15)
        selector.class2 = (uint8_t)(dm2_v1_dungeon_read_record_u16(
            dungeon, object_record + 2) & 0x7fu);
    else if (type == 7) selector.class2 = 0u;
    else return 0;
    selector.record_byte4 = missile.energy_remaining;
    if (selector.class1 == 0x0du) {
        if (missile.energy_remaining == 0xffu ||
            !dm2_v1_query_gdat_entry_data_index(
                loader, 0x0d, selector.class2, 0x0b, 1,
                &selector.image_data_index) || !selector.image_data_index)
            return 0;
        selector.branch_temp_picst = 1;
    }
    hash = 2166136261u;
    hash = (hash ^ missile.record_hash) * 16777619u;
    hash = (hash ^ object) * 16777619u;
    hash = (hash ^ selector.class1) * 16777619u;
    hash = (hash ^ selector.class2) * 16777619u;
    hash = (hash ^ selector.record_byte4) * 16777619u;
    hash = (hash ^ selector.image_data_index) * 16777619u;
    selector.missile_object_id = missile_object_id;
    selector.missile_object = object;
    selector.identity_hash = hash ? hash : 1u;
    selector.valid = 1;
    if (out_missile) *out_missile = missile;
    if (out_selector) *out_selector = selector;
    return 1;
}

