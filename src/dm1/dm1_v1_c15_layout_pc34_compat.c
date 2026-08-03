#include "dm1_v1_c15_layout_pc34_compat.h"

#include <string.h>

uint32_t dm1_v1_c15_layout_fingerprint_pc34(const unsigned char* bytes,
                                             size_t byte_count)
{
    uint32_t hash = 2166136261u;
    size_t i;
    if (!bytes && byte_count) return 0u;
    for (i = 0; i < byte_count; ++i) {
        hash ^= bytes[i];
        hash *= 16777619u;
    }
    return hash;
}

int dm1_v1_c15_pool_reserve_pc34(
    struct DungeonThings_Compat* things,
    DM1_C15PoolReservationPc34* out_reservation)
{
    unsigned short thing;
    unsigned char* raw;
    int index;

    if (!things || !out_reservation || !things->loaded ||
        !things->rawThingData[THING_TYPE_EXPLOSION] ||
        !things->explosions || things->thingCounts[THING_TYPE_EXPLOSION] <= 0 ||
        things->explosionCount != things->thingCounts[THING_TYPE_EXPLOSION]) {
        return 0;
    }

    memset(out_reservation, 0, sizeof(*out_reservation));
    for (index = 0; index < things->thingCounts[THING_TYPE_EXPLOSION]; ++index) {
        raw = things->rawThingData[THING_TYPE_EXPLOSION] + (size_t)index * 4u;
        if (raw[0] == (unsigned char)(THING_NONE & 0xffu) &&
            raw[1] == (unsigned char)(THING_NONE >> 8)) {
            break;
        }
    }
    if (index >= things->thingCounts[THING_TYPE_EXPLOSION]) return 0;

    memcpy(out_reservation->raw, raw, sizeof(out_reservation->raw));
    out_reservation->decoded = things->explosions[index];
    thing = F0516_DUNGEON_GetUnusedThing_Compat(
        things, THING_TYPE_EXPLOSION, NULL, NULL);
    if (thing == THING_NONE || THING_GET_TYPE(thing) != THING_TYPE_EXPLOSION) {
        memset(out_reservation, 0, sizeof(*out_reservation));
        return 0;
    }
    if (THING_GET_INDEX(thing) != index) {
        /* F0516 must select the same first unused C15 row we snapshotted. */
        return 0;
    }

    raw = things->rawThingData[THING_TYPE_EXPLOSION] + (size_t)index * 4u;
    if (raw[0] != (unsigned char)(THING_ENDOFLIST & 0xffu) ||
        raw[1] != (unsigned char)(THING_ENDOFLIST >> 8) ||
        things->explosions[index].next != THING_ENDOFLIST) {
        memcpy(raw, out_reservation->raw, sizeof(out_reservation->raw));
        things->explosions[index] = out_reservation->decoded;
        memset(out_reservation, 0, sizeof(*out_reservation));
        return 0;
    }

    out_reservation->things = things;
    out_reservation->thing = thing;
    out_reservation->active = 1;
    return 1;
}

int dm1_v1_c15_pool_rollback_pc34(
    DM1_C15PoolReservationPc34* reservation)
{
    unsigned char* raw;
    int index;

    if (!reservation || !reservation->active || !reservation->things ||
        THING_GET_TYPE(reservation->thing) != THING_TYPE_EXPLOSION) {
        return 0;
    }
    if (reservation->linked &&
        (!reservation->dungeon ||
         !F0515_DUNGEON_UnlinkThingFromList_Compat(
             reservation->dungeon, reservation->things, reservation->thing,
             THING_ENDOFLIST, reservation->mapIndex, reservation->mapX,
             reservation->mapY))) {
        return 0;
    }
    index = THING_GET_INDEX(reservation->thing);
    if (!reservation->things->loaded ||
        !reservation->things->rawThingData[THING_TYPE_EXPLOSION] ||
        !reservation->things->explosions || index < 0 ||
        index >= reservation->things->thingCounts[THING_TYPE_EXPLOSION] ||
        index >= reservation->things->explosionCount) {
        return 0;
    }

    raw = reservation->things->rawThingData[THING_TYPE_EXPLOSION] +
        (size_t)index * 4u;
    memcpy(raw, reservation->raw, sizeof(reservation->raw));
    reservation->things->explosions[index] = reservation->decoded;
    reservation->linked = 0;
    reservation->active = 0;
    return 1;
}

int dm1_v1_c15_pool_initialize_and_link_pc34(
    DM1_C15PoolReservationPc34* reservation,
    struct DungeonDatState_Compat* dungeon,
    int explosion_type,
    int attack,
    int centered,
    int cell,
    int map_index,
    int map_x,
    int map_y)
{
    unsigned char* raw;
    unsigned short linked_thing;
    int index;

    if (!reservation || !reservation->active || !reservation->things ||
        !dungeon || !dungeon->tilesLoaded ||
        explosion_type < 0 || explosion_type > 0x7f || attack < 0 ||
        attack > 0xff || (centered != 0 && centered != 1) ||
        cell < 0 || cell > 3) {
        if (reservation && reservation->active) {
            (void)dm1_v1_c15_pool_rollback_pc34(reservation);
        }
        return 0;
    }
    index = THING_GET_INDEX(reservation->thing);
    if (THING_GET_TYPE(reservation->thing) != THING_TYPE_EXPLOSION ||
        index < 0 || index >= reservation->things->explosionCount ||
        !reservation->things->rawThingData[THING_TYPE_EXPLOSION]) {
        (void)dm1_v1_c15_pool_rollback_pc34(reservation);
        return 0;
    }

    raw = reservation->things->rawThingData[THING_TYPE_EXPLOSION] +
        (size_t)index * 4u;
    raw[2] = (unsigned char)(explosion_type | (centered << 7));
    raw[3] = (unsigned char)attack;
    reservation->things->explosions[index].type = (unsigned char)explosion_type;
    reservation->things->explosions[index].centered = (unsigned char)centered;
    reservation->things->explosions[index].attack = (unsigned char)attack;

    linked_thing = (unsigned short)(reservation->thing | ((unsigned short)cell << 14));
    if (!F0514_DUNGEON_LinkThingToList_Compat(
            dungeon, reservation->things, linked_thing, THING_ENDOFLIST,
            map_index, map_x, map_y)) {
        (void)dm1_v1_c15_pool_rollback_pc34(reservation);
        return 0;
    }

    reservation->dungeon = dungeon;
    reservation->thing = linked_thing;
    reservation->mapIndex = map_index;
    reservation->mapX = map_x;
    reservation->mapY = map_y;
    reservation->linked = 1;
    return 1;
}

int dm1_v1_c15_c25_publish_pc34(
    DM1_C15PoolReservationPc34* reservation,
    struct DungeonDatState_Compat* dungeon,
    int explosion_type,
    int attack,
    int centered,
    int cell,
    int map_index,
    int map_x,
    int map_y,
    uint32_t fire_at_tick,
    int priority,
    DM1_C15C25PublicationReceiptPc34* out_receipt)
{
    DM1_C15C25PublicationReceiptPc34 receipt;
    int index;

    if (!out_receipt || fire_at_tick > 0x00ffffffu || priority < 0 ||
        priority > 0xff || map_index < 0 || map_index > 0xff ||
        map_x < 0 || map_x > 0xff || map_y < 0 || map_y > 0xff) {
        if (reservation && reservation->active) {
            (void)dm1_v1_c15_pool_rollback_pc34(reservation);
        }
        return 0;
    }
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (!dm1_v1_c15_pool_initialize_and_link_pc34(
            reservation, dungeon, explosion_type, attack, centered, cell,
            map_index, map_x, map_y)) {
        return 0;
    }
    index = THING_GET_INDEX(reservation->thing);
    if (index < 0 || !reservation->things ||
        index >= reservation->things->thingCounts[THING_TYPE_EXPLOSION]) {
        (void)dm1_v1_c15_pool_rollback_pc34(reservation);
        return 0;
    }

    memset(&receipt, 0, sizeof(receipt));
    receipt.mapTime = ((uint32_t)map_index << 24) | fire_at_tick;
    receipt.slot = reservation->thing;
    receipt.mapX = (uint8_t)map_x;
    receipt.mapY = (uint8_t)map_y;
    receipt.priority = (uint8_t)priority;
    receipt.c15Fingerprint = dm1_v1_c15_layout_fingerprint_pc34(
        reservation->things->rawThingData[THING_TYPE_EXPLOSION] +
            (size_t)index * 4u, 4u);
    receipt.active = 1;
    if (!receipt.c15Fingerprint || !dm1_v1_c15_c25_receipt_is_live_pc34(
            &receipt, dungeon, reservation->things)) {
        (void)dm1_v1_c15_pool_rollback_pc34(reservation);
        return 0;
    }
    *out_receipt = receipt;
    return 1;
}

int dm1_v1_c15_c25_receipt_is_live_pc34(
    const DM1_C15C25PublicationReceiptPc34* receipt,
    const struct DungeonDatState_Compat* dungeon,
    const struct DungeonThings_Compat* things)
{
    unsigned short thing;
    const unsigned char* raw;
    int index;
    int matches = 0;
    int safety = 0;

    if (!receipt || !receipt->active || !dungeon || !things ||
        !things->loaded || !dungeon->maps || !dungeon->tiles ||
        !things->rawThingData[THING_TYPE_EXPLOSION] ||
        THING_GET_TYPE(receipt->slot) != THING_TYPE_EXPLOSION ||
        (int)(receipt->mapTime >> 24) >= dungeon->header.mapCount ||
        receipt->mapX >= dungeon->maps[receipt->mapTime >> 24].width ||
        receipt->mapY >= dungeon->maps[receipt->mapTime >> 24].height) {
        return 0;
    }
    index = THING_GET_INDEX(receipt->slot);
    if (index < 0 || index >= things->thingCounts[THING_TYPE_EXPLOSION]) {
        return 0;
    }
    raw = things->rawThingData[THING_TYPE_EXPLOSION] + (size_t)index * 4u;
    if (dm1_v1_c15_layout_fingerprint_pc34(raw, 4u) !=
        receipt->c15Fingerprint) {
        return 0;
    }
    thing = F0511_DUNGEON_GetSquareFirstThing_Compat(
        dungeon, things, (int)(receipt->mapTime >> 24), receipt->mapX,
        receipt->mapY);
    while (thing != THING_NONE && thing != THING_ENDOFLIST && safety++ < 64) {
        if (thing == receipt->slot) ++matches;
        thing = F0512_DUNGEON_GetThingNext_Compat(things, thing);
    }
    return matches == 1;
}

int dm1_v1_f0221_fluxcage_on_square_pc34(
    const struct DungeonDatState_Compat* dungeon,
    const struct DungeonThings_Compat* things,
    int map_index,
    int map_x,
    int map_y,
    int* out_has_fluxcage)
{
    const unsigned char* raw;
    unsigned short thing;
    int index;
    int safety = 0;

    if (out_has_fluxcage) *out_has_fluxcage = 0;
    if (!dungeon || !things || !out_has_fluxcage ||
        map_index < 0 || map_index >= dungeon->header.mapCount) {
        return 0;
    }
    if (!things->loaded ||
        !things->rawThingData[THING_TYPE_EXPLOSION] || !things->explosions ||
        things->thingCounts[THING_TYPE_EXPLOSION] != things->explosionCount) {
        return 1;
    }
    thing = F0511_DUNGEON_GetSquareFirstThing_Compat(
        dungeon, things, map_index, map_x, map_y);
    while (thing != THING_NONE && thing != THING_ENDOFLIST && safety++ < 64) {
        if (THING_GET_TYPE(thing) == THING_TYPE_EXPLOSION) {
            index = THING_GET_INDEX(thing);
            if (index < 0 || index >= things->explosionCount) return 0;
            raw = things->rawThingData[THING_TYPE_EXPLOSION] +
                (size_t)index * 4u;
            if ((unsigned short)(raw[0] | ((unsigned short)raw[1] << 8)) !=
                    things->explosions[index].next ||
                (raw[2] & 0x7fu) != things->explosions[index].type ||
                ((raw[2] >> 7) & 1u) != things->explosions[index].centered ||
                raw[3] != things->explosions[index].attack) {
                return 0;
            }
            if (things->explosions[index].type == 50) {
                *out_has_fluxcage = 1;
                return 1;
            }
        }
        thing = F0512_DUNGEON_GetThingNext_Compat(things, thing);
    }
    return safety < 64;
}
