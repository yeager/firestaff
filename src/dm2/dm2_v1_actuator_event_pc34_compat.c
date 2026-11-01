/*
 * dm2_v1_actuator_event_pc34_compat.c — DM2 actuator event dispatch.
 *
 * Ports ACTUATE_WALL_MECHA and ACTUATE_FLOOR_MECHA from skproject
 * skevent.cpp (v4), plus the actuator sub-functions they dispatch to.
 *
 * Source: skproject/SKWINSPX/src/v4/skevent.cpp
 */

#include "dm2_v1_actuator_event_pc34_compat.h"

#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include "dm2_v1_caii_alloc_pc34_compat.h"
#include "dm2_v1_dbitem_alloc_pc34_compat.h"
#include "dm2_v1_item_ops_pc34_compat.h"
#include "dm2_v1_move_record_to_pc34_compat.h"
#include "dm2_v1_record_pool_pc34_compat.h"
#include "dm2_v1_skproject_core.h"
#include "dm2_v1_tile_record_walk_pc34_compat.h"

/* Creature-killer types and declaration in dm2_v1_actuator_event_pc34_compat.h */

/* ── Timer construction helpers ─────────────────────────────────────── */

static DM2_V1_SourceTimer make_timer(int map, uint32_t tick,
                                     uint8_t type, uint8_t actor,
                                     int16_t value_a, int16_t value_b)
{
    DM2_V1_SourceTimer t;
    memset(&t, 0, sizeof(t));
    t.ticks_and_map = (tick & DM2_V1_SOURCE_TIMER_TICK_MASK)
                    | ((uint32_t)(map & 0xFF) << 24);
    t.type = type;
    t.actor = actor;
    t.value_a = value_a;
    t.value_b = value_b;
    t.reserved = 0;
    return t;
}

/* Encode x,y into value_a for tty04 timers (xcoordB/ycoordB).
 * Source: c_timer.h setxyA(x, y) puts x in low byte, y in high byte. */
static int16_t encode_xy(int x, int y)
{
    return (int16_t)((uint8_t)x | ((uint8_t)y << 8));
}


/* ── INVOKE_ACTUATOR ───────────────────────────────────────────────── */

/* Source: skevent.cpp — queues a tty04 timer aimed at the actuator's
 * target coordinates.  The source's c_tim_proc.cpp:4367-4392
 * DM2_INVOKE_ACTUATOR reads the actuator's (Xcoord, Ycoord, Direction),
 * adds its stored delay and the caller's extra ticks, then fires the message. */
int dm2_v1_invoke_actuator(DM2_V1_SourceTimerQueue *queue,
                           const uint8_t *actu_record,
                           int action_type, int delay_add,
                           int map, uint32_t game_tick)
{
    DM2_V1_SourceTimer t;
    uint8_t target_x;
    uint8_t target_y;
    uint8_t dir;

    if (queue == NULL || actu_record == NULL) return 0;

    target_x = dm2_actu_xcoord(actu_record);
    target_y = dm2_actu_ycoord(actu_record);
    dir      = dm2_actu_direction(actu_record);

    t = make_timer(map, game_tick + (uint32_t)dm2_actu_delay(actu_record)
                              + (uint32_t)delay_add,
                   0x04, 0,
                   encode_xy(target_x, target_y),
                   (int16_t)((dir & 0xFF) | ((action_type & 0xFF) << 8)));
    /* value_b encoding: low byte = Value2 (direction), high byte = ActionType
     * Source: c_timer.h — the source's INVOKE_MESSAGE sets Value2 and
     * ActionType as separate nibbles.  We pack them into value_b. */
    dm2_v1_source_timer_enqueue(queue, &t, 0);
    return 1;
}

/* ── ACTIVATE_RELAY1 ───────────────────────────────────────────────── */

/* Source: skevent.cpp:1154-1183 — ACTIVATE_RELAY1(Timer *ref, Actuator *pr4,
 * X16 delayAsMult).  Once-only actuators gate on their revert-effect flag
 * and the incoming timer's action type; otherwise queue a tty04
 * INVOKE_MESSAGE timer at the actuator's target coordinates with a delay
 * computed either as a multiplier shift or a plain sum. */
int dm2_v1_activate_relay1(DM2_V1_RecordPoolSet *pool_set,
                           DM2_V1_DungeonData *dungeon,
                           DM2_V1_SourceTimerQueue *queue,
                           const uint8_t *actu_record,
                           int timer_action_type,
                           int delay_as_mult,
                           int map, uint32_t game_tick)
{
    (void)pool_set;
    (void)dungeon;

    if (queue == NULL || actu_record == NULL) return 0;

    if (dm2_actu_once_only(actu_record) != 0) {
        if (dm2_actu_revert_effect(actu_record) != 0 || timer_action_type != 0) {
            if (dm2_actu_revert_effect(actu_record) == 0 || timer_action_type != 1)
                return 1;
        }
    }

    uint16_t actu_data = dm2_actu_data(actu_record);
    uint8_t  delay      = dm2_actu_delay(actu_record);
    uint32_t fire_tick;

    if (delay_as_mult != 0) {
        fire_tick = game_tick + ((uint32_t)actu_data << delay);
    } else {
        fire_tick = (uint32_t)actu_data + game_tick + delay;
    }

    uint8_t target_x = dm2_actu_xcoord(actu_record);
    uint8_t target_y = dm2_actu_ycoord(actu_record);
    uint8_t dir      = dm2_actu_direction(actu_record);
    int action = (dm2_actu_once_only(actu_record) != 0)
                     ? dm2_actu_action_type(actu_record)
                     : timer_action_type;

    DM2_V1_SourceTimer t = make_timer(map, fire_tick, 0x04, 0,
                                       encode_xy(target_x, target_y),
                                       (int16_t)((dir & 0xFF) | ((action & 0xFF) << 8)));
    dm2_v1_source_timer_enqueue(queue, &t, 0);
    return 1;
}

/* ── ACTIVATE_RELAY2 ───────────────────────────────────────────────── */

/* Source: skevent.cpp:1185-1224 — ACTIVATE_RELAY2(Timer *ref, Actuator *pr4,
 * X16 xx).  Dispatches to INVOKE_ACTUATOR with an action type/delay chosen
 * from the actuator's own action type, its revert-effect flag, and the
 * incoming timer's action type. */
int dm2_v1_activate_relay2(DM2_V1_RecordPoolSet *pool_set,
                           DM2_V1_DungeonData *dungeon,
                           DM2_V1_SourceTimerQueue *queue,
                           const uint8_t *actu_record,
                           int timer_action_type,
                           int actu_data_param,
                           int map, uint32_t game_tick)
{
    (void)pool_set;
    (void)dungeon;

    if (queue == NULL || actu_record == NULL) return 0;

    int di = actu_data_param;
    uint8_t actu_action = dm2_actu_action_type(actu_record);

    if (actu_action == 3) {
        if (dm2_actu_revert_effect(actu_record) != 0) {
            dm2_v1_invoke_actuator(queue, actu_record, timer_action_type, 0,
                                    map, game_tick);
            if (di == 0) {
                int si;
                switch (timer_action_type) {
                    case 0: si = 1; break;
                    case 1: si = 0; break;
                    case 2: si = 2; break;
                    default: si = timer_action_type; break;
                }
                dm2_v1_invoke_actuator(queue, actu_record, si, di,
                                        map, game_tick);
            }
            return 1;
        }
        dm2_v1_invoke_actuator(queue, actu_record, timer_action_type, di,
                                map, game_tick);
        return 1;
    }

    if (dm2_actu_revert_effect(actu_record) != 0 || timer_action_type != 0) {
        if (dm2_actu_revert_effect(actu_record) == 0 || timer_action_type != 1)
            return 1;
    }

    dm2_v1_invoke_actuator(queue, actu_record, actu_action, di,
                            map, game_tick);
    return 1;
}

/* ── ACTIVATE_TICK_GENERATOR ───────────────────────────────────────── */

/* Source: skevent.cpp:357-400 */
int dm2_v1_activate_tick_generator(DM2_V1_SourceTimerQueue *queue,
                                   const uint8_t *actu_record,
                                   int16_t record_link,
                                   int map, uint32_t game_tick)
{
    uint16_t si;
    uint16_t actu_data;
    uint8_t  atype;
    DM2_V1_SourceTimer t;

    if (queue == NULL || actu_record == NULL) return 0;

    atype = dm2_actu_type(actu_record);
    switch (atype) {
        case DM2_ACTU_TICK_GENERATOR: si = 1;   break;
        case DM2_ACTU_TICK_GEN_V008:  si = 8;   break;
        case DM2_ACTU_TICK_GEN_V016:  si = 16;  break;
        case DM2_ACTU_TICK_GEN_V032:  si = 32;  break;
        case DM2_ACTU_TICK_GEN_V064:  si = 64;  break;
        case DM2_ACTU_TICK_GEN_V128:  si = 128; break;
        default: return 0;
    }

    actu_data = dm2_actu_data(actu_record);
    if (actu_data == 0) return 1;

    t = make_timer(map,
                   game_tick + (game_tick % ((uint32_t)actu_data * (uint32_t)si)),
                   DM2_V1_TIMER_TICK_GENERATOR, 0,
                   record_link,
                   (int16_t)(si & 0xFF));
    dm2_v1_source_timer_enqueue(queue, &t, 0);
    return 1;
}

/* ── PUSH_BUTTON_SWITCH ───────────────────────────────────────────── */

/* Source: skevent.cpp:2010-2028
 * Finds the door record at the actuator's target (Xcoord, Ycoord) and
 * sets/clears/toggles bit 13 of its w2 word based on action_type.
 * Bit 13 controls whether the door is sealed (cannot be bashed open). */

int dm2_v1_push_button_switch(DM2_V1_RecordPoolSet *pool_set,
                              DM2_V1_DungeonData *dungeon,
                              const uint8_t *actu_record,
                              int map,
                              int action_type,
                              DM2_V1_ActuatorEventReceipt *receipt)
{
    int16_t direct_link;
    uint8_t *door;
    int target_x, target_y;

    if (pool_set == NULL || !pool_set->valid || dungeon == NULL ||
        actu_record == NULL || map < 0 || map >= dungeon->level_count ||
        action_type < DM2_ACTMSG_OPEN_SET || action_type > DM2_ACTMSG_TOGGLE) {
        return 0;
    }

    target_x = (int)dm2_actu_xcoord(actu_record);
    target_y = (int)dm2_actu_ycoord(actu_record);
    if (target_x < 0 || target_y < 0 ||
        target_x >= dungeon->level_widths[map] ||
        target_y >= dungeon->level_heights[map]) {
        return 0;
    }

    /* skevent.cpp:2015 takes GET_ADDRESS_OF_TILE_RECORD directly.  A
     * complete chained walk would be a different operation: it could mutate
     * a later DB0 item even though the source would cast the first record. */
    direct_link = dm2_v1_tile_record_link(dungeon, map, target_x, target_y);
    if (direct_link == DM2_V1_RECORD_HANDLE_NULL ||
        direct_link == DM2_V1_RECORD_HANDLE_END ||
        dm2_v1_record_handle_pool(direct_link) != DM2_DB_DOOR ||
        !(door = dm2_v1_record_pool_address_mut(pool_set, direct_link))) {
        return 0;
    }

    switch (action_type) {
        case DM2_ACTMSG_OPEN_SET: dm2_door_set_bit13(door, 1); break;
        case DM2_ACTMSG_CLOSE_CLEAR: dm2_door_set_bit13(door, 0); break;
        case DM2_ACTMSG_TOGGLE:
            dm2_door_set_bit13(door, (uint8_t)(dm2_door_bit13(door) ^ 1u));
            break;
        default: return 0;
    }
    if (receipt) receipt->door_bit13_toggled++;
    return 1;
}

int dm2_v1_push_button_switch_chain(
    DM2_V1_RecordPoolSet *pool_set,
    DM2_V1_DungeonData *dungeon,
    int map, int x, int y, int action_type,
    DM2_V1_ActuatorEventReceipt *receipt)
{
    int capacity = 0;
    int count = 0;
    int16_t link;
    int16_t *actuators = NULL;
    DM2_V1_ActuatorEventReceipt local;

    memset(&local, 0, sizeof(local));
    if (!pool_set || !pool_set->valid || !dungeon || map < 0 ||
        map >= dungeon->level_count || x < 0 || y < 0 ||
        x >= dungeon->level_widths[map] || y >= dungeon->level_heights[map] ||
        action_type < DM2_ACTMSG_OPEN_SET || action_type > DM2_ACTMSG_TOGGLE) {
        if (receipt) *receipt = local;
        return 0;
    }
    for (int db = 0; db < DM2_V1_RECORD_POOL_COUNT; ++db) {
        const DM2_V1_RecordPool *pool = &pool_set->pools[db];
        if (pool->record_count < 0 || pool->extension_count < 0 ||
            capacity > INT_MAX - pool->record_count - pool->extension_count) {
            if (receipt) *receipt = local;
            return 0;
        }
        capacity += pool->record_count + pool->extension_count;
    }
    if (capacity <= 0) {
        if (receipt) *receipt = local;
        return 0;
    }
    actuators = calloc((size_t)capacity, sizeof(*actuators));
    if (!actuators) {
        if (receipt) *receipt = local;
        return 0;
    }
    link = (int16_t)dm2_v1_dungeon_get_first_thing(dungeon, map, x, y);
    while (link != DM2_V1_RECORD_HANDLE_END) {
        const uint8_t *actuator;
        int16_t next;
        int target_x;
        int target_y;
        int16_t direct_link;

        if (link == DM2_V1_RECORD_HANDLE_NULL || count >= capacity ||
            dm2_v1_record_handle_pool(link) != DM2_DB_ACTUATOR ||
            !(actuator = dm2_v1_record_pool_address(pool_set, link)) ||
            dm2_actu_type(actuator) != DM2_ACTU_PUSH_BUTTON_SWITCH ||
            !dm2_v1_record_pool_next_link(pool_set, link, &next)) {
            local.fail_closed = count != 0;
            free(actuators);
            if (receipt) *receipt = local;
            return 0;
        }
        target_x = (int)dm2_actu_xcoord(actuator);
        target_y = (int)dm2_actu_ycoord(actuator);
        direct_link = dm2_v1_tile_record_link(
            dungeon, map, target_x, target_y);
        if (target_x < 0 || target_y < 0 ||
            target_x >= dungeon->level_widths[map] ||
            target_y >= dungeon->level_heights[map] ||
            direct_link == DM2_V1_RECORD_HANDLE_NULL ||
            direct_link == DM2_V1_RECORD_HANDLE_END ||
            dm2_v1_record_handle_pool(direct_link) != DM2_DB_DOOR ||
            !dm2_v1_record_pool_address(pool_set, direct_link)) {
            local.fail_closed = 1;
            free(actuators);
            if (receipt) *receipt = local;
            return 0;
        }
        actuators[count++] = link;
        link = next;
    }
    if (count == 0) {
        free(actuators);
        if (receipt) *receipt = local;
        return 0;
    }
    for (int i = 0; i < count; ++i) {
        const uint8_t *actuator = dm2_v1_record_pool_address(
            pool_set, actuators[i]);
        if (!dm2_v1_push_button_switch(pool_set, dungeon, actuator, map,
                                       action_type, &local)) {
            local.fail_closed = 1;
            free(actuators);
            if (receipt) *receipt = local;
            return 0;
        }
        ++local.push_button_invoked;
    }
    local.valid = 1;
    free(actuators);
    if (receipt) *receipt = local;
    return 1;
}

/* ── CREATURE_GENERATOR ───────────────────────────────────────────── */

/* Source: skevent.cpp:1740-1759 (dispatch), skcrture.cpp:6311-6362
 * (ALLOC_NEW_CREATURE).  The source allocation is not just a DB4 record
 * insertion: it obtains its AI row, health multiplier, RAND02 direction,
 * animation state and timer ownership through the live game state.  The
 * compatibility API therefore deliberately does not manufacture a creature
 * from an actuator's type/coordinates alone. */
int dm2_v1_creature_generator(DM2_V1_RecordPoolSet *pool_set,
                              DM2_V1_DungeonData *dungeon,
                              DM2_V1_SourceTimerQueue *queue,
                              const uint8_t *actu_record,
                              int map, uint32_t game_tick,
                              DM2_V1_CreatureGeneratorReceipt *receipt)
{
    DM2_V1_CreatureGeneratorReceipt local;

    memset(&local, 0, sizeof(local));
    if (receipt == NULL) receipt = &local;
    (void)pool_set;
    (void)dungeon;
    (void)queue;
    (void)actu_record;
    (void)map;
    (void)game_tick;
    receipt->fail_closed = 1;
    return 0;
}

/* ── ACTIVATE_SHOOTER ─────────────────────────────────────────────── */

/* Source: skevent.cpp:1536-1631; sktimprc.cpp:1611-1800.
 * Shooter types 0x07/0x08/0x0E fire a single projectile;
 * 0x09/0x0A/0x0F fire dual projectiles.
 * Item shooters (0x0E/0x0F) pick items from the source tile.
 * Others allocate via ALLOC_NEW_DBITEM or use missile records.
 *
 * This compatibility helper only reports source launch facts and remains
 * fail-closed without a runtime transaction. The live wall-mecha path is
 * separately owned by activate_shooter_live above. */

int dm2_v1_activate_shooter(DM2_V1_RecordPoolSet *pool_set,
                            DM2_V1_DungeonData *dungeon,
                            DM2_V1_SourceTimerQueue *queue,
                            const uint8_t *actu_record,
                            int timer_x, int timer_y,
                            int timer_direction,
                            int map, uint32_t game_tick,
                            DM2_V1_ShooterReceipt *receipt)
{
    DM2_V1_ShooterReceipt local;

    memset(&local, 0, sizeof(local));
    if (receipt == NULL) receipt = &local;

    if (actu_record != NULL) {
        const int type = (int)dm2_actu_type(actu_record);
        const int data = (int)dm2_actu_data(actu_record);
        const int direction = timer_direction & 3;
        const int launch_direction = (direction + 2) & 3;

        /* sktimprc.cpp derives vb_20/vb_30 from word_at(actuator, 6)
         * after shifting by four; DM2_SHOOT_ITEM receives these as the
         * source payload bytes.  Its first target is one tile along the
         * timer direction, while projectile facing starts two quadrants
         * away.  Single shooters then add RAND01; record that fact without
         * inventing a random result while the owner transaction is closed. */
        receipt->source_fields_decoded = 1;
        receipt->actuator_type = type;
        receipt->actuator_data = data;
        receipt->timer_x = timer_x;
        receipt->timer_y = timer_y;
        receipt->timer_direction = direction;
        receipt->launch_x = timer_x + dm2_x_delta(direction);
        receipt->launch_y = timer_y + dm2_y_delta(direction);
        receipt->launch_direction = launch_direction;
        receipt->second_launch_direction = (launch_direction + 1) & 3;
        receipt->payload_low = (int)((dm2_actu_w6(actu_record) >> 4) & 0xffu);
        receipt->payload_high = (int)((dm2_actu_w6(actu_record) >> 12) & 0xffu);
        receipt->projectile_energy = 100;
        receipt->single_projectile = DM2_SHOOTER_SINGLE_TYPES(type) ? 1 : 0;
        receipt->launch_direction_randomized = receipt->single_projectile;
    }

    /* Runtime dispatch uses activate_shooter_live with the session RNG and
     * the outer wall-mecha transaction. Keep this fact-only adapter closed
     * for standalone callers that cannot own DB14, tile-list and timer state. */
    (void)pool_set;
    (void)dungeon;
    (void)queue;
    (void)timer_x;
    (void)timer_y;
    (void)timer_direction;
    (void)map;
    (void)game_tick;
    receipt->fail_closed = 1;
    receipt->valid = 1;
    return 0;
}

/* ── ACTIVATE_ITEM_TELEPORT ───────────────────────────────────────── */

/* Source: skevent.cpp:1346-1500.
 * Moves items from source tile to destination tile based on actuator's
 * data filter.  capture=1 reverses source/dest.  recycler uses an
 * item bitmask.  Items are filtered by DB type (weapon..miscellaneous)
 * and by direction (wall) or all directions (floor). */

typedef struct {
    DM2_V1_RecordPoolSet *pool_set;
    DM2_V1_DungeonData *dungeon;
    const DM2_V1_AssetLoader *asset_loader;
    DM2_V1_SourceTimerQueue *queue;
    const uint8_t *actu_record;
    int src_x, src_y, src_dir;
    int dst_x, dst_y, dst_dir;
    int is_floor;
    int recycler;
    int only_first_item;
    uint16_t filter_data;
    uint8_t item_mask[64];
    int item_mask_valid;
    int map;
    uint32_t game_tick;
    DM2_V1_DropRng *rng;
    DM2_V1_ReviveChampionFn revive_champion;
    void *revive_ctx;
    DM2_V1_ItemTeleportReceipt *receipt;
} ItemTeleportCtx;

static uint16_t item_teleport_rd16(const DM2_V1_RecordPoolSet *pools,
                                   const uint8_t *bytes)
{
    if (pools->source_words_big_endian)
        return (uint16_t)(((uint16_t)bytes[0] << 8) | bytes[1]);
    return (uint16_t)(bytes[0] | ((uint16_t)bytes[1] << 8));
}

static void item_teleport_wr16(const DM2_V1_RecordPoolSet *pools,
                               uint8_t *bytes, uint16_t value)
{
    if (pools->source_words_big_endian) {
        bytes[0] = (uint8_t)(value >> 8);
        bytes[1] = (uint8_t)value;
    } else {
        bytes[0] = (uint8_t)value;
        bytes[1] = (uint8_t)(value >> 8);
    }
}

/* SKW_MOVE_ITEM_TO applies the actuator ActionType to charged items before
 * relocating them: 0 fills, 1 removes 15 charges, and 2 adds RAND16(max+1).
 * Record words follow the admitted platform byte order. */
static int item_teleport_adjust_charge(ItemTeleportCtx *ctx, int16_t handle)
{
    uint8_t cls1 = 0xffu, cls2 = 0xffu;
    DM2_V1_SkprojectQueryCls1Receipt cls1_receipt;
    DM2_V1_SkprojectQueryCls2Receipt cls2_receipt;
    uint8_t *record;
    uint16_t word, mask, type;
    int shift, max_charge, current, delta, updated, mode;
    memset(&cls1_receipt, 0, sizeof(cls1_receipt));
    memset(&cls2_receipt, 0, sizeof(cls2_receipt));
    if (!dm2_v1_skproject_query_cls1_from_record_ex(
            (uint16_t)handle, ctx->pool_set, &cls1, &cls1_receipt) ||
        !cls1_receipt.valid ||
        !dm2_v1_skproject_query_cls2_from_record(
            (uint16_t)handle, ctx->pool_set, &cls2, &cls2_receipt) ||
        !cls2_receipt.valid || cls2 == 0xffu) return 0;
    if (cls1 == 0x15u && cls2 == 0u) return 1;

    mode = (int)((item_teleport_rd16(
        ctx->pool_set, ctx->actu_record + 4) >> 3) & 7u);
    max_charge = dm2_v1_get_max_charge((uint16_t)handle);
    if (mode == 0) delta = max_charge;
    else if (mode == 1) delta = -15;
    else if (mode == 2) {
        if (!ctx->rng) return 0;
        delta = (int)dm2_v1_drops_rand16(
            ctx->rng, (uint16_t)(max_charge + 1));
    } else {
        /* SKW_MOVE_ITEM_TO skips charge adjustment for ActionType > 2 but
         * still relocates the item. */
        return 1;
    }
    if (max_charge == 0) return 1;

    record = dm2_v1_record_pool_address_mut(ctx->pool_set, handle);
    if (!record) return 0;
    type = (uint16_t)dm2_v1_record_handle_pool(handle);
    if (type == 0x05) { shift = 10; mask = 0x3c00u; }
    else if (type == 0x06) { shift = 9; mask = 0x1e00u; }
    else if (type == 0x0a) { shift = 14; mask = 0xc000u; }
    else return 1;
    word = item_teleport_rd16(ctx->pool_set, record + 2);
    current = (int)((word & mask) >> shift);
    updated = current + delta;
    if (updated < 0) updated = 0;
    if (updated > max_charge) updated = max_charge;
    word = (uint16_t)((word & (uint16_t)~mask) |
                      (((uint16_t)updated << shift) & mask));
    item_teleport_wr16(ctx->pool_set, record + 2, word);
    ctx->receipt->charged_items++;
    return 1;
}

static int item_teleport_cut_possession(ItemTeleportCtx *ctx,
                                        int16_t owner_handle,
                                        int16_t handle,
                                        int16_t previous_handle,
                                        int16_t saved_next)
{
    uint8_t *owner_pool = dm2_v1_record_pool_address_mut(
        ctx->pool_set, owner_handle);
    uint8_t *owner_raw = (uint8_t *)dm2_v1_dungeon_get_thing_record(
        ctx->dungeon, (uint16_t)owner_handle & 0x3fffu, NULL, NULL, NULL);
    uint8_t *item_pool = dm2_v1_record_pool_address_mut(ctx->pool_set,
                                                        handle);
    uint8_t *item_raw = (uint8_t *)dm2_v1_dungeon_get_thing_record(
        ctx->dungeon, (uint16_t)handle & 0x3fffu, NULL, NULL, NULL);
    uint16_t head;

    if (!owner_pool || !owner_raw || !item_pool || !item_raw) return 0;
    head = item_teleport_rd16(ctx->pool_set, owner_pool + 2);
    if (previous_handle == (int16_t)0xffff) {
        if ((head & 0x3fffu) != ((uint16_t)handle & 0x3fffu)) return 0;
        head = (uint16_t)saved_next;
    } else {
        uint8_t *previous_pool = dm2_v1_record_pool_address_mut(
            ctx->pool_set, previous_handle);
        uint8_t *previous_raw = (uint8_t *)dm2_v1_dungeon_get_thing_record(
            ctx->dungeon, (uint16_t)previous_handle & 0x3fffu,
            NULL, NULL, NULL);
        if (!previous_pool || !previous_raw) return 0;
        item_teleport_wr16(ctx->pool_set, previous_pool,
                           (uint16_t)saved_next);
        memcpy(previous_raw, previous_pool, 2u);
    }
    item_teleport_wr16(ctx->pool_set, owner_pool + 2, head);
    memcpy(owner_raw + 2, owner_pool + 2, 2u);
    item_teleport_wr16(ctx->pool_set, item_pool, 0xfffeu);
    memcpy(item_raw, item_pool, 2u);
    return 1;
}

static int item_teleport_champion_index(ItemTeleportCtx *ctx,
                                        int16_t handle)
{
    const uint8_t *record = dm2_v1_record_pool_address(ctx->pool_set, handle);
    uint16_t word;
    int type = dm2_v1_record_handle_pool(handle);
    if (!record) return -1;
    word = item_teleport_rd16(ctx->pool_set, record + 2);
    if (type == 5) return (int)(((uint32_t)word * 4u) >> 12);
    if (type == 6) return (int)(((uint32_t)word * 8u) >> 12);
    if (type == 10) return (int)(word >> 14);
    return -1;
}

static int item_teleport_revive_champion(ItemTeleportCtx *ctx,
                                         int16_t handle,
                                         int16_t possession_owner,
                                         int16_t possession_previous,
                                         int16_t saved_next)
{
    int champion_index = item_teleport_champion_index(ctx, handle);
    uint8_t *pool_record;
    uint8_t *raw_record;
    if (!ctx->revive_champion || champion_index < 0 || champion_index > 3) {
        ctx->receipt->fail_closed = 1;
        return 1;
    }
    if (possession_owner != (int16_t)0xffff) {
        if (!item_teleport_cut_possession(ctx, possession_owner, handle,
                                          possession_previous, saved_next)) {
            ctx->receipt->fail_closed = 1;
            return 1;
        }
    } else {
        DM2_V1_MoveRecordToReceipt cut_receipt;
        memset(&cut_receipt, 0, sizeof(cut_receipt));
        if (!dm2_v1_move_record_to(
                ctx->pool_set, ctx->dungeon, ctx->queue, handle,
                (int16_t)ctx->src_x, (int16_t)ctx->src_y,
                -1, -1, -1, ctx->map, ctx->game_tick, &cut_receipt) ||
            cut_receipt.fail_closed) {
            ctx->receipt->fail_closed = 1;
            return 1;
        }
    }
    if (!dm2_v1_record_pool_dealloc_record(ctx->pool_set, handle)) {
        ctx->receipt->fail_closed = 1;
        return 1;
    }
    pool_record = dm2_v1_record_pool_address_mut(ctx->pool_set, handle);
    raw_record = (uint8_t *)dm2_v1_dungeon_get_thing_record(
        ctx->dungeon, (uint16_t)handle & 0x3fffu, NULL, NULL, NULL);
    if (!pool_record || !raw_record) {
        ctx->receipt->fail_closed = 1;
        return 1;
    }
    memcpy(raw_record, pool_record, 2u);
    if (!ctx->revive_champion(ctx->revive_ctx, (int16_t)champion_index)) {
        ctx->receipt->fail_closed = 1;
        return 1;
    }
    ctx->receipt->champions_revived++;
    ctx->receipt->last_champion_index = (int16_t)champion_index;
    return 1;
}

static int item_teleport_process_item(ItemTeleportCtx *ctx,
                                      int16_t handle,
                                      int16_t possession_owner,
                                      int16_t possession_previous,
                                      int16_t saved_next)
{
    int db_type = dm2_v1_record_handle_pool(handle);
    int item_dir = (int)(((uint16_t)handle >> 14) & 3u);
    uint16_t distinctive = 0xffffu;
    int exact_type_match = 0;

    /* Source item range: dbWeapon=5 through dbMiscellaneous_item=10. */
    if (db_type < DM2_DB_WEAPON || db_type > DM2_DB_MISC_ITEM) {
        return 0;
    }

    /* Direction filter: wall actuators match direction only */
    if (ctx->is_floor == 0 && item_dir != ctx->src_dir) {
        return 0;
    }
    if (ctx->recycler && !ctx->item_mask_valid) return 0;

    /* skevent.cpp:1400-1420 / SKW_ACTIVATE_ITEM_TELEPORT: compare
     * GET_DISTINCTIVE_ITEMTYPE with ActuatorData; 0x1ff accepts every
     * item. Recycler instead indexes QUERY_CREATURES_ITEM_MASK by the
     * distinctive type. */
    if (ctx->recycler || ctx->filter_data != 0x1ffu) {
        uint8_t cls2 = 0xffu;
        DM2_V1_SkprojectQueryCls2Receipt cls2_receipt;
        DM2_V1_SkprojectDistinctiveItemtypeReceipt type_receipt;
        memset(&cls2_receipt, 0, sizeof(cls2_receipt));
        memset(&type_receipt, 0, sizeof(type_receipt));
        if (!dm2_v1_skproject_query_cls2_from_record(
                (uint16_t)handle, ctx->pool_set, &cls2, &cls2_receipt) ||
            !cls2_receipt.valid || cls2 == 0xffu ||
            !dm2_v1_skproject_get_distinctive_itemtype(
                (uint16_t)handle, cls2, &distinctive, &type_receipt) ||
            !type_receipt.valid) {
            ctx->receipt->item_type_lookup_failed++;
            ctx->receipt->fail_closed = 1;
            return 1;
        }
        if (ctx->recycler) {
            if (distinctive >= 512u ||
                (ctx->item_mask[distinctive >> 3] &
                 (uint8_t)(1u << (distinctive & 7u))) == 0u) return 0;
        } else if (distinctive == ctx->filter_data) {
            exact_type_match = 1;
        } else if (ctx->filter_data != 0x1ffu) {
            return 0;
        }
    }

    /* SKWINDOS dm2byg.cpp:3390, SKWINSPX v4 skevent.cpp:1435 and v5
     * bitem.cpp:862 all revive a matching champion token only when the
     * actuator's once-only bit is clear. */
    if (exact_type_match && !ctx->recycler &&
        dm2_actu_once_only(ctx->actu_record) == 0) {
        uint8_t cls1 = 0xffu, cls2 = 0xffu;
        DM2_V1_SkprojectQueryCls1Receipt cls1_receipt;
        DM2_V1_SkprojectQueryCls2Receipt cls2_receipt;
        memset(&cls1_receipt, 0, sizeof(cls1_receipt));
        memset(&cls2_receipt, 0, sizeof(cls2_receipt));
        if (dm2_v1_skproject_query_cls1_from_record_ex(
                (uint16_t)handle, ctx->pool_set, &cls1, &cls1_receipt) &&
            cls1_receipt.valid && cls1 == 0x15u &&
            dm2_v1_skproject_query_cls2_from_record(
                (uint16_t)handle, ctx->pool_set, &cls2, &cls2_receipt) &&
            cls2_receipt.valid && cls2 == 0u) {
            ctx->receipt->items_counted++;
            return item_teleport_revive_champion(
                ctx, handle, possession_owner, possession_previous,
                saved_next);
        }
    }

    ctx->receipt->items_counted++;

    if (!item_teleport_adjust_charge(ctx, handle)) {
        ctx->receipt->fail_closed = 1;
        return 1;
    }

    /* SKW_MOVE_ITEM_TO cuts tile items from the source tile before appending
     * them. A recursively visited possession item is cut from its owning
     * creature's +2 link instead. */
    {
        DM2_V1_MoveRecordToReceipt move_receipt;
        int destination_direction = ctx->dst_dir;
        int destination_tile_value = dm2_v1_dungeon_c_map_get_tile_value(
            ctx->dungeon, ctx->map, ctx->dst_x, ctx->dst_y);
        if (destination_tile_value < 0) {
            ctx->receipt->fail_closed = 1;
            return 1;
        }
        if ((((unsigned int)destination_tile_value & 0xffu) >> 5) != 0u) {
            if (!ctx->rng) {
                ctx->receipt->fail_closed = 1;
                return 1;
            }
            destination_direction = (int)dm2_v1_drops_rand16(
                ctx->rng, 4u);
        }
        if (possession_owner != (int16_t)0xffff) {
            if (!item_teleport_cut_possession(
                    ctx, possession_owner, handle, possession_previous,
                    saved_next)) {
                ctx->receipt->fail_closed = 1;
                return 1;
            }
        } else if (ctx->src_x >= 0 && ctx->src_y >= 0) {
            memset(&move_receipt, 0, sizeof(move_receipt));
            int cut = dm2_v1_move_record_to(
                ctx->pool_set, ctx->dungeon, ctx->queue, handle,
                (int16_t)ctx->src_x, (int16_t)ctx->src_y,
                -1, -1, -1, ctx->map, ctx->game_tick, &move_receipt);
            if (!cut || move_receipt.fail_closed) {
                ctx->receipt->fail_closed = 1;
                return 1;
            }
        }
        memset(&move_receipt, 0, sizeof(move_receipt));
        int moved = dm2_v1_move_record_to(
            ctx->pool_set, ctx->dungeon, ctx->queue, handle,
            -1, -1, (int16_t)ctx->dst_x, (int16_t)ctx->dst_y,
            (int16_t)destination_direction, ctx->map, ctx->game_tick,
            &move_receipt);
        if (!moved || move_receipt.fail_closed) {
            ctx->receipt->fail_closed = 1;
            return 1;
        }
        ctx->receipt->items_moved++;
    }

    if (ctx->only_first_item && ctx->receipt->items_moved > 0) {
        return 1;
    }
    return 0;
}

static int item_teleport_walk_possessions(ItemTeleportCtx *ctx,
                                          int16_t creature_handle,
                                          unsigned depth)
{
    uint8_t *creature_pool;
    uint16_t cursor;
    int16_t previous = (int16_t)0xffff;
    size_t budget = 1u;
    int type;

    if (depth > 64u) {
        ctx->receipt->fail_closed = 1;
        return 1;
    }
    creature_pool = dm2_v1_record_pool_address_mut(ctx->pool_set,
                                                    creature_handle);
    if (!creature_pool) {
        ctx->receipt->fail_closed = 1;
        return 1;
    }
    for (type = 0; type < DM2_V1_RECORD_POOL_COUNT; ++type) {
        budget += ctx->pool_set->pools[type].record_count;
        budget += ctx->pool_set->pools[type].extension_count;
    }
    cursor = item_teleport_rd16(ctx->pool_set, creature_pool + 2);
    while (cursor != 0xfffeu) {
        uint8_t *record;
        int16_t next;
        int db_type;
        if (cursor == 0xffffu || budget-- == 0u) {
            ctx->receipt->fail_closed = 1;
            return 1;
        }
        record = dm2_v1_record_pool_address_mut(ctx->pool_set,
                                                 (int16_t)cursor);
        if (!record || !dm2_v1_record_pool_next_link(
                ctx->pool_set, (int16_t)cursor, &next)) {
            ctx->receipt->fail_closed = 1;
            return 1;
        }
        db_type = dm2_v1_record_handle_pool((int16_t)cursor);
        if (db_type == DM2_DB_CREATURE) {
            int moved_before = ctx->receipt->items_moved;
            ctx->receipt->creatures_checked++;
            (void)item_teleport_walk_possessions(
                ctx, (int16_t)cursor, depth + 1u);
            if (ctx->receipt->fail_closed ||
                (ctx->only_first_item &&
                 ctx->receipt->items_moved > moved_before))
                return 1;
        } else if (db_type >= DM2_DB_WEAPON &&
                   db_type <= DM2_DB_MISC_ITEM) {
            int moved_before = ctx->receipt->items_moved;
            int stop = item_teleport_process_item(
                ctx, (int16_t)cursor, creature_handle, previous, next);
            if (ctx->receipt->fail_closed || stop) return 1;
            if (ctx->receipt->items_moved == moved_before)
                previous = (int16_t)cursor;
        } else {
            previous = (int16_t)cursor;
        }
        cursor = (uint16_t)next;
    }
    return 0;
}

static int item_teleport_visitor(void *context,
                                 int16_t handle,
                                 const uint8_t *record_unused)
{
    ItemTeleportCtx *ctx = (ItemTeleportCtx *)context;
    int db_type = dm2_v1_record_handle_pool(handle);
    (void)record_unused;

    if (db_type == DM2_DB_CREATURE && !ctx->recycler) {
        int moved_before = ctx->receipt->items_moved;
        ctx->receipt->creatures_checked++;
        (void)item_teleport_walk_possessions(ctx, handle, 0u);
        return ctx->receipt->fail_closed ||
               (ctx->only_first_item &&
                ctx->receipt->items_moved > moved_before);
    }
    return item_teleport_process_item(
        ctx, handle, (int16_t)0xffff, (int16_t)0xffff,
        (int16_t)0xffff);
}

static int dm2_v1_activate_item_teleport_impl(
                                  DM2_V1_RecordPoolSet *pool_set,
                                  DM2_V1_DungeonData *dungeon,
                                  const DM2_V1_AssetLoader *asset_loader,
                                  DM2_V1_SourceTimerQueue *queue,
                                  const uint8_t *actu_record,
                                  int timer_x, int timer_y,
                                  int timer_direction,
                                  int timer_action_type,
                                  int is_floor, int recycler,
                                  int capture, int only_first_item,
                                  int map, uint32_t game_tick,
                                  DM2_V1_DropRng *rng,
                                  DM2_V1_ReviveChampionFn revive_champion,
                                  void *revive_ctx,
                                  DM2_V1_ItemTeleportReceipt *receipt)
{
    DM2_V1_ItemTeleportReceipt local;
    ItemTeleportCtx ctx;
    DM2_V1_TileRecordWalkReceipt walk_receipt;

    memset(&local, 0, sizeof(local));
    if (receipt == NULL) {
        receipt = &local;
    } else {
        memset(receipt, 0, sizeof(*receipt));
    }
    memset(&ctx, 0, sizeof(ctx));
    receipt->last_champion_index = -1;

    if (pool_set == NULL || dungeon == NULL || actu_record == NULL) {
        receipt->fail_closed = 1;
        return 0;
    }

    /* Revert/action guard (skevent.cpp:1379-1383) */
    if (dm2_actu_revert_effect(actu_record) != 0 || timer_action_type != 0) {
        if (dm2_actu_revert_effect(actu_record) == 0 || timer_action_type != 1) {
            receipt->valid = 1;
            return 1;
        }
    }

    /* Source/dest depends on capture flag (skevent.cpp:1360-1377) */
    if (capture != 0) {
        ctx.src_x = (int)dm2_actu_xcoord(actu_record);
        ctx.src_y = (int)dm2_actu_ycoord(actu_record);
        ctx.src_dir = (int)dm2_actu_direction(actu_record);
        ctx.dst_x = timer_x;
        ctx.dst_y = timer_y;
        ctx.dst_dir = timer_direction;
    } else {
        ctx.src_x = timer_x;
        ctx.src_y = timer_y;
        ctx.src_dir = timer_direction;
        ctx.dst_x = (int)dm2_actu_xcoord(actu_record);
        ctx.dst_y = (int)dm2_actu_ycoord(actu_record);
        ctx.dst_dir = (int)dm2_actu_direction(actu_record);
    }

    /* skevent.cpp:1413 and c_querydb.cpp:1045: obtain the exact GDAT
     * item-type mask from the actuator's creature index and Delay field. */
    if (recycler != 0) {
        DM2_V1_CreaturesItemMaskReceipt mask_receipt;
        int creature_index = (int)(dm2_actu_data(actu_record) & 0xffu);
        int text_field_base = (int)dm2_actu_delay(actu_record) * 3 + 0x10;
        memset(&mask_receipt, 0, sizeof(mask_receipt));
        if (!dm2_v1_query_creatures_item_mask_receipt(
                asset_loader, creature_index, text_field_base, 0,
                &mask_receipt) || !mask_receipt.accepted) {
            /* c_querydb.cpp zeroes the destination mask before asking GDAT.
             * The source caller ignores a missing-text result; if Firestaff
             * cannot produce an authenticated mask, fail closed to that
             * empty-mask no-op instead of accepting every item. */
            receipt->recycler_mask_missing = 1;
            memset(ctx.item_mask, 0, sizeof(ctx.item_mask));
        } else {
            memcpy(ctx.item_mask, mask_receipt.mask, sizeof(ctx.item_mask));
            ctx.item_mask_valid = 1;
        }
    }

    ctx.pool_set = pool_set;
    ctx.dungeon = dungeon;
    ctx.asset_loader = asset_loader;
    ctx.queue = queue;
    ctx.actu_record = actu_record;
    ctx.is_floor = is_floor;
    ctx.recycler = recycler != 0;
    ctx.only_first_item = only_first_item;
    ctx.filter_data = dm2_actu_data(actu_record);
    ctx.map = map;
    ctx.game_tick = game_tick;
    ctx.rng = rng;
    ctx.revive_champion = revive_champion;
    ctx.revive_ctx = revive_ctx;
    ctx.receipt = receipt;

    memset(&walk_receipt, 0, sizeof(walk_receipt));
    dm2_v1_tile_record_walk(pool_set, dungeon, map, ctx.src_x, ctx.src_y,
                            item_teleport_visitor, &ctx, &walk_receipt);

    if (receipt->fail_closed) return 0;
    receipt->valid = 1;
    return 1;
}

int dm2_v1_activate_item_teleport(DM2_V1_RecordPoolSet *pool_set,
                                  DM2_V1_DungeonData *dungeon,
                                  const DM2_V1_AssetLoader *asset_loader,
                                  DM2_V1_SourceTimerQueue *queue,
                                  const uint8_t *actu_record,
                                  int timer_x, int timer_y,
                                  int timer_direction,
                                  int timer_action_type,
                                  int is_floor, int recycler,
                                  int capture, int only_first_item,
                                  int map, uint32_t game_tick,
                                  DM2_V1_DropRng *rng,
                                  DM2_V1_ItemTeleportReceipt *receipt)
{
    return dm2_v1_activate_item_teleport_impl(
        pool_set, dungeon, asset_loader, queue, actu_record,
        timer_x, timer_y, timer_direction, timer_action_type,
        is_floor, recycler, capture, only_first_item, map, game_tick,
        rng, NULL, NULL, receipt);
}

/* ── Wall mecha record walk context ────────────────────────────────── */

typedef struct {
    DM2_V1_RecordPoolSet *pool_set;
    DM2_V1_DungeonData *dungeon;
    DM2_V1_CaiiArray *caii;
    const DM2_V1_AssetLoader *asset_loader;
    DM2_V1_SourceTimerQueue *queue;
    int map;
    int x;             /* timer X coordinate (tile walk origin) */
    int y;             /* timer Y coordinate (tile walk origin) */
    int action_type;   /* timer ActionType (from value_b high byte) */
    int direction;     /* timer Value2 (from value_b low byte) */
    uint32_t game_tick;
    DM2_V1_GlobVarState *global_state;
    const DM2_V1_CreatureKillerCallbacks *creature_killer_cb;
    DM2_V1_GetOrnateAnimLenFn get_ornate_anim_len;
    void *ornate_ctx;
    DM2_V1_QueueOrnateActivationSoundFn queue_ornate_sound;
    void *ornate_sound_ctx;
    DM2_V1_QueueShooterNoiseFn queue_shooter_noise;
    void *shooter_sound_ctx;
    DM2_V1_DropRng *rng;
    DM2_V1_ReviveChampionFn revive_champion;
    void *revive_ctx;
    DM2_V1_ActuatorEventReceipt *receipt;
} WallMechaCtx;

static void shooter_wr16(const DM2_V1_RecordPoolSet *pools,
                         uint8_t *p, uint16_t value)
{
    if (pools->source_words_big_endian) {
        p[0] = (uint8_t)(value >> 8);
        p[1] = (uint8_t)value;
    } else {
        p[0] = (uint8_t)value;
        p[1] = (uint8_t)(value >> 8);
    }
}

/* Source: SKWINSPX/src/v4/skevent.cpp:1536-1631 and skgame.cpp:440-501;
 * SKWINSPX/src/v5/sktimprc.cpp:1611-1812 and bitem.cpp:1043-1138;
 * SKWINDOS/dm2byg.cpp:2765-2841 and dm2byc.cpp:359-424. The runtime wall
 * owner supplies its transaction snapshot, session RNG and source noise
 * callback. */
static int activate_shooter_live(DM2_V1_RecordPoolSet *pools,
                                 DM2_V1_DungeonData *dungeon,
                                 DM2_V1_SourceTimerQueue *queue,
                                 DM2_V1_DropRng *rng,
                                 DM2_V1_QueueShooterNoiseFn queue_noise,
                                 void *shooter_sound_ctx,
                                 const uint8_t *actuator,
                                 int timer_x, int timer_y, int timer_dir,
                                 int map, uint32_t game_tick,
                                 DM2_V1_ShooterReceipt *receipt)
{
    int type, data, low, high, single, dir, launch_dir, launch_x, launch_y;
    int16_t objects[2] = {-1, -1};
    int object_count = 1;
    int budget = 0;
    int16_t cursor;
    if (!pools || !pools->valid || !dungeon || !queue || !rng ||
        !actuator || !receipt || map < 0 || map >= dungeon->level_count ||
        timer_x < 0 || timer_y < 0 ||
        timer_x >= dungeon->level_widths[map] ||
        timer_y >= dungeon->level_heights[map]) return 0;
    type = dm2_actu_type(actuator);
    data = dm2_actu_data(actuator);
    low = (int)((dm2_actu_w6(actuator) >> 4) & 0xffu);
    high = (int)((dm2_actu_w6(actuator) >> 12) & 0xffu);
    dir = timer_dir & 3;
    launch_dir = (dir + 2) & 3;
    single = (type == DM2_ACTU_SOME_SHOOTER ||
              type == DM2_ACTU_MISSILE_SHOOTER ||
              type == DM2_ACTU_ITEM_SHOOTER);
    if (!(single || type == DM2_ACTU_WEAPON_SHOOTER ||
          type == DM2_ACTU_MISSILE_SHOOTER_2 ||
          type == DM2_ACTU_ITEM_SHOOTER_X2)) return 0;
    receipt->source_fields_decoded = 1;
    receipt->actuator_type = type;
    receipt->actuator_data = data;
    receipt->timer_x = timer_x;
    receipt->timer_y = timer_y;
    receipt->timer_direction = dir;
    receipt->payload_low = low;
    receipt->payload_high = high;
    receipt->projectile_energy = 100;
    receipt->single_projectile = single;

    if (type == DM2_ACTU_ITEM_SHOOTER ||
        type == DM2_ACTU_ITEM_SHOOTER_X2) {
        int found = 0;
        cursor = (int16_t)dm2_v1_dungeon_get_first_thing(
            dungeon, map, timer_x, timer_y);
        for (int db = 0; db < DM2_V1_RECORD_POOL_COUNT; ++db)
            budget += pools->pools[db].record_count +
                      pools->pools[db].extension_count;
        while (cursor != DM2_V1_RECORD_HANDLE_END && cursor >= 0 &&
               budget-- > 0) {
            int16_t next;
            int record_db = dm2_v1_record_handle_pool(cursor);
            if (record_db > DM2_DB_ACTUATOR &&
                (dm2_handle_dir(cursor) == dir ||
                 dm2_handle_dir(cursor) == ((dir + 1) & 3))) {
                objects[found++] = cursor;
                if (found == 2) break;
            }
            if (!dm2_v1_record_pool_next_link(pools, cursor, &next)) return 0;
            cursor = next;
        }
        if (found == 0) return 1; /* Source no-op: no directional item. */
        if (type == DM2_ACTU_ITEM_SHOOTER_X2 && found > 1) {
            object_count = 2;
            single = 0;
        } else {
            single = 1;
        }
        if (!dm2_v1_record_pool_cut_from_tile(
                pools, dungeon, map, timer_x, timer_y, objects[0])) return 0;
        receipt->source_items_cut++;
        if (object_count == 2) {
            if (!dm2_v1_record_pool_cut_from_tile(
                    pools, dungeon, map, timer_x, timer_y,
                    objects[1])) return 0;
            receipt->source_items_cut++;
        }
    } else if (type == DM2_ACTU_MISSILE_SHOOTER ||
               type == DM2_ACTU_MISSILE_SHOOTER_2) {
        objects[0] = (int16_t)(0xff80u + (uint16_t)data);
        if (!single) {
            objects[1] = (int16_t)(0xff80u + (uint16_t)data);
            object_count = 2;
        }
    } else {
        objects[0] = dm2_v1_alloc_new_dbitem(pools, (uint16_t)data);
        if (objects[0] < 0) return 1; /* Source allocation no-op. */
        if (!single) {
            objects[1] = dm2_v1_alloc_new_dbitem(pools, (uint16_t)data);
            if (objects[1] < 0) single = 1;
            else object_count = 2;
        }
    }

    if (single) {
        launch_dir = (launch_dir + (int)dm2_v1_drops_randbit(rng)) & 3;
        receipt->launch_direction_randomized = 1;
    }
    receipt->single_projectile = single;
    receipt->launch_x = timer_x + dm2_x_delta(dir);
    receipt->launch_y = timer_y + dm2_y_delta(dir);
    receipt->launch_direction = launch_dir;
    receipt->second_launch_direction = (launch_dir + 1) & 3;
    launch_x = receipt->launch_x;
    launch_y = receipt->launch_y;
    if (launch_x < 0 || launch_y < 0 ||
        launch_x >= dungeon->level_widths[map] ||
        launch_y >= dungeon->level_heights[map]) return 0;

    for (int i = 0; i < object_count; ++i) {
        int projectile_dir = i == 0 ? launch_dir : ((launch_dir + 1) & 3);
        int16_t db14 = dm2_v1_record_pool_alloc_new_record(pools, 14u);
        uint8_t *missile = db14 >= 0
            ? dm2_v1_record_pool_address_mut(pools, db14) : NULL;
        int16_t head;
        int tile_raw;
        DM2_V1_SourceTimer follow;
        if (db14 < 0) {
            /* SKULLWIN/c_item.cpp:1058-1068 and SPX v4 skgame.cpp:451-459:
             * if the DB14 missile record cannot be allocated, ordinary
             * items are placed on the launch tile facing the projectile
             * direction. Cloud objects have no such fallback. The caller's
             * wall-mecha snapshot owns rollback if MOVE_RECORD_TO cannot
             * complete this source fallback. */
            if (dm2_v1_record_handle_pool(objects[i]) != 0x0f) {
                DM2_V1_MoveRecordToReceipt move;
                memset(&move, 0, sizeof(move));
                if (!dm2_v1_move_record_to(
                        pools, dungeon, queue, objects[i], -1, 0,
                        (int16_t)launch_x, (int16_t)launch_y,
                        (int16_t)projectile_dir, map, game_tick, &move) ||
                    move.fail_closed)
                    return 0;
                receipt->allocation_fallback_items_moved++;
            }
            continue;
        }
        if (!missile || pools->pools[14].record_size < 8)
            return 0;
        /* c_item.cpp:1097-1100 — DM2_SHOOT_ITEM requests GEN2 noise before
         * publishing the missile record/timer. Noise is best-effort: the
         * source routine returns void and continues when sound admission is
         * unavailable. A missing sound queue must not suppress the shot. */
        if (queue_noise &&
            queue_noise(shooter_sound_ctx, (uint16_t)objects[i], map,
                        launch_x, launch_y, game_tick))
            receipt->sound_queued++;
        shooter_wr16(pools, missile, 0xfffeu);
        shooter_wr16(pools, missile + 2, (uint16_t)objects[i]);
        missile[4] = (uint8_t)low;
        missile[5] = 100u;
        shooter_wr16(pools, missile + 6, 0u);
        tile_raw = dm2_v1_dungeon_get_tile_raw(
            dungeon, map, launch_x, launch_y);
        head = (int16_t)dm2_v1_dungeon_get_first_thing(
            dungeon, map, launch_x, launch_y);
        if (head < 0 && (tile_raw < 0 || (tile_raw & 0x10) == 0)) {
            if (dm2_v1_dungeon_insert_first_thing_empty_tile(
                    dungeon, map, launch_x, launch_y, (uint16_t)db14) != 0)
                return 0;
        } else if (head < 0) {
            if (dm2_v1_dungeon_set_first_thing(
                    dungeon, map, launch_x, launch_y, (uint16_t)db14) != 0)
                return 0;
        } else {
            if (!dm2_v1_record_pool_append_to_list(pools, &head, db14) ||
                dm2_v1_dungeon_set_first_thing(
                    dungeon, map, launch_x, launch_y, (uint16_t)head) != 0)
                return 0;
        }
        memset(&follow, 0, sizeof(follow));
        follow.ticks_and_map = ((uint32_t)(map & 0xff) << 24) |
            ((game_tick + 1u) & DM2_V1_SOURCE_TIMER_TICK_MASK);
        follow.type = 0x1eu;
        follow.value_a = (int16_t)(((uint16_t)db14 & 0x3fffu) |
                                   ((uint16_t)projectile_dir << 14));
        follow.value_b = (int16_t)((launch_x & 0x1f) |
            ((launch_y & 0x1f) << 5) | ((projectile_dir & 3) << 10) |
            (((i == 0 ? high : dir) & 0xf) << 12));
        if (dm2_v1_source_timer_enqueue(queue, &follow, 0) !=
            DM2_V1_SOURCE_TIMER_OK) return 0;
        ++receipt->projectiles_created;
        ++receipt->continuation_timers_queued;
    }
    receipt->valid = 1;
    return 1;
}

/* Visitor callback for dm2_v1_tile_record_walk.
 * Ports the body of ACTUATE_WALL_MECHA's for-loop (skevent.cpp:1643-2063). */
static int wall_mecha_visitor(void *context,
                              int16_t handle,
                              const uint8_t *record)
{
    WallMechaCtx *ctx = (WallMechaCtx *)context;
    DM2_V1_ActuatorEventReceipt *r = ctx->receipt;
    int db_type;
    uint8_t atype;
    uint16_t actu_data;
    uint8_t *record_mut;

    r->records_visited++;

    db_type = dm2_v1_record_handle_pool(handle);

    /* Text records (dbText = 1): toggle visibility (skevent.cpp:1646-1688) */
    if (db_type == DM2_DB_TEXT) {
        if (dm2_handle_dir(handle) == ctx->direction) {
            r->text_records_toggled++;
        }
        return 0; /* continue */
    }

    /* Only actuator records (dbActuator = 3) are dispatched */
    if (db_type != DM2_DB_ACTUATOR) return 0;

    /* Direction filter (skevent.cpp:2400-2414) */
    atype = dm2_actu_type(record);
    if (dm2_handle_dir(handle) != ctx->direction) {
        return 0;
    }

    actu_data = dm2_actu_data(record);
    r->actuators_dispatched++;

    /* Need mutable pointer for counter/tick-gen/ornate operations */
    record_mut = dm2_v1_record_pool_address_mut(ctx->pool_set, handle);

    switch (atype) {

    /* ── WORK_TIMER (0x31) skevent.cpp:1717-1739 ─────────────────── */
    case DM2_ACTU_WORK_TIMER:
        if (dm2_actu_active_status(record) != 0) break;
        if (record_mut) {
            DM2_V1_SourceTimer t = make_timer(ctx->map,
                ctx->game_tick + actu_data,
                0x5B, 0, handle, 0);
            dm2_v1_source_timer_enqueue(ctx->queue, &t, 0);
            dm2_actu_set_active_status(record_mut, 1);
        }
        if (dm2_actu_revert_effect(record) != 0 || ctx->action_type != 0) {
            if (dm2_actu_revert_effect(record) == 0 || ctx->action_type != 1)
                break;
        }
        {
            int fwd = dm2_actu_once_only(record)
                    ? (int)dm2_actu_action_type(record)
                    : ctx->action_type;
            dm2_v1_invoke_actuator(ctx->queue, record, fwd, 0,
                                   ctx->map, ctx->game_tick);
        }
        r->work_timer_invoked++;
        break;

    /* ── CREATURE_GENERATOR (0x2e) skevent.cpp:1740-1759 ─────────── */
    case DM2_ACTU_CREATURE_GENERATOR: {
        DM2_V1_CreatureGeneratorReceipt cg_receipt;
        if (ctx->action_type != 0) break;
        r->creature_generator_invoked++;
        memset(&cg_receipt, 0, sizeof(cg_receipt));
        dm2_v1_creature_generator(ctx->pool_set, ctx->dungeon, ctx->queue,
                                   record, ctx->map, ctx->game_tick,
                                   &cg_receipt);
        if (cg_receipt.fail_closed) r->fail_closed++;
        break;
    }

    /* ── COUNTER (0x1d) skevent.cpp:1834-1863 ────────────────────── */
    case DM2_ACTU_COUNTER: {
        int bp0c;
        int bp0e;
        if (!record_mut) { r->fail_closed++; break; }
        bp0c = (actu_data == 0 || (actu_data & 256) != 0) ? 1 : 0;
        if (ctx->action_type == 1) {
            dm2_actu_set_data(record_mut, (uint16_t)(actu_data + 1));
        } else if (ctx->action_type == 0) {
            if (dm2_actu_once_only(record) == 0 || actu_data != 0) {
                dm2_actu_set_data(record_mut, (uint16_t)(actu_data - 1));
            }
        }
        actu_data = dm2_actu_data(record_mut);
        bp0e = (actu_data == 0 || (actu_data & 256) != 0) ? 1 : 0;
        if (bp0e == bp0c) break;
        if (dm2_actu_action_type(record) == 3) {
            dm2_v1_invoke_actuator(ctx->queue, record_mut,
                (dm2_actu_revert_effect(record) == bp0e) ? 1 : 0,
                0, ctx->map, ctx->game_tick);
        } else if (bp0e != 0) {
            dm2_v1_invoke_actuator(ctx->queue, record_mut,
                (int)dm2_actu_action_type(record),
                0, ctx->map, ctx->game_tick);
        }
        r->counter_invoked++;
        break;
    }

    /* ── FINITE_RELAY (0x2d) skevent.cpp:1864-1888 ───────────────── */
    case DM2_ACTU_FINITE_RELAY:
        if (actu_data > 0 && actu_data <= 400) {
            if (record_mut) {
                dm2_actu_set_data(record_mut, (uint16_t)(actu_data - 1));
            }
            dm2_v1_invoke_actuator(ctx->queue, record,
                ctx->action_type, 0, ctx->map, ctx->game_tick);
        }
        /* Probabilistic path (400-500) omitted: requires RAND16 */
        r->relay1_invoked++;
        break;

    /* ── TICK_GENERATOR family (0x1e,0x33-0x37) skevent.cpp:1889-1901 */
    case DM2_ACTU_TICK_GENERATOR:
    case DM2_ACTU_TICK_GEN_V008:
    case DM2_ACTU_TICK_GEN_V016:
    case DM2_ACTU_TICK_GEN_V032:
    case DM2_ACTU_TICK_GEN_V064:
    case DM2_ACTU_TICK_GEN_V128:
        if (record_mut) {
            uint8_t new_once = dm2_toggle_actuator_message(
                ctx->action_type, dm2_actu_once_only(record));
            dm2_actu_set_once_only(record_mut, new_once);
            if (dm2_actu_active_status(record_mut) != 0 || new_once == 0)
                break;
            dm2_v1_activate_tick_generator(ctx->queue, record_mut,
                handle, ctx->map, ctx->game_tick);
        }
        r->tick_generator_invoked++;
        break;

    /* ── RELAY_1 (0x20), RELAY_3 (0x45) skevent.cpp:1902-1906 ────── */
    case DM2_ACTU_RELAY_1:
    case DM2_ACTU_RELAY_3:
        dm2_v1_activate_relay1(ctx->pool_set, ctx->dungeon, ctx->queue,
            record, ctx->action_type,
            (atype == DM2_ACTU_RELAY_3) ? 1 : 0,
            ctx->map, ctx->game_tick);
        r->relay1_invoked++;
        break;

    /* ── RELAY_2 (0x3d) skevent.cpp:1907-1910 ───────────────────── */
    case DM2_ACTU_RELAY_2:
        dm2_v1_activate_relay2(ctx->pool_set, ctx->dungeon, ctx->queue,
            record, ctx->action_type, (int)actu_data,
            ctx->map, ctx->game_tick);
        r->relay2_invoked++;
        break;

    /* ── SHOOTERS (0x07-0x0a,0x0e,0x0f) skevent.cpp:1911-1919 ───── */
    case DM2_ACTU_SOME_SHOOTER:
    case DM2_ACTU_MISSILE_SHOOTER:
    case DM2_ACTU_WEAPON_SHOOTER:
    case DM2_ACTU_MISSILE_SHOOTER_2:
    case DM2_ACTU_ITEM_SHOOTER:
    case DM2_ACTU_ITEM_SHOOTER_X2: {
        DM2_V1_ShooterReceipt sh_receipt;
        int shooter_ok;
        memset(&sh_receipt, 0, sizeof(sh_receipt));
        /* SKWIN/SkWinCore.cpp ACTUATE_WALL_MECHA (around line 62036)
         * passes Timer::XcoordB/YcoordB to ACTIVATE_SHOOTER (line 61938).
         * These are the activated wall tile in ctx->x/y; Actuator::w6 is
         * shooter payload and must not be decoded as a map position. */
        if (ctx->rng) {
            shooter_ok = activate_shooter_live(
                ctx->pool_set, ctx->dungeon, ctx->queue, ctx->rng,
                ctx->queue_shooter_noise, ctx->shooter_sound_ctx, record,
                ctx->x, ctx->y,
                ctx->direction, ctx->map, ctx->game_tick, &sh_receipt);
        } else {
            shooter_ok = dm2_v1_activate_shooter(
                ctx->pool_set, ctx->dungeon, ctx->queue, record,
                ctx->x, ctx->y,
                ctx->direction, ctx->map, ctx->game_tick, &sh_receipt);
        }
        if (!shooter_ok) sh_receipt.fail_closed = 1;
        r->shooter_invoked++;
        if (sh_receipt.fail_closed) r->fail_closed++;
        break;
    }

    /* ── CROSS_MAP (0x16) skevent.cpp:1920-1933 ─────────────────── */
    case DM2_ACTU_CROSS_MAP: {
        int target_map = (int)(actu_data & 63);
        DM2_V1_SourceTimer t = make_timer(target_map,
            ctx->game_tick, 0x04, 0,
            encode_xy(dm2_actu_xcoord(record),
                      dm2_actu_ycoord(record)),
            (int16_t)(((actu_data >> 6) & 3) |
                      ((ctx->action_type & 0xFF) << 8)));
        dm2_v1_source_timer_enqueue(ctx->queue, &t, 0);
        r->cross_map_invoked++;
        break;
    }

    /* ── THE_END (0x12) c_tim_proc.cpp:2212-2220 (switch case 11)
     * Sets ddat.v1e0240 = 1 (game exit flag) and calls PREPARE_EXIT.
     * Receipt: the_end_triggered = 1 signals the runtime to end the game. */
    case DM2_ACTU_THE_END:
        r->the_end_triggered++;
        break;

    /* ── ORNATE_ANIMATOR_2 (0x32) skevent.cpp:1940-1943 ──────────── */
    case DM2_ACTU_ORNATE_ANIMATOR_2:
        if (dm2_actu_active_status(record) == 0 && record_mut) {
            dm2_actu_set_active_status(record_mut, 1);
            dm2_actu_set_data(record_mut, 0);
            {
                DM2_V1_SourceTimer t = make_timer(ctx->map,
                    ctx->game_tick + 1,
                    DM2_V1_TIMER_ORNATE_ANIMATOR, 0,
                    handle, 1 /* isWall=1 */);
                dm2_v1_source_timer_enqueue(ctx->queue, &t, 0);
            }
            if (ctx->queue_ornate_sound)
                ctx->queue_ornate_sound(ctx->ornate_sound_ctx, handle,
                    record_mut, ctx->map, ctx->x, ctx->y,
                    ctx->game_tick, 1);
        }
        if (dm2_actu_once_only(record)) {
            dm2_v1_activate_relay2(ctx->pool_set, ctx->dungeon, ctx->queue,
                record, ctx->action_type, 0, ctx->map, ctx->game_tick);
        }
        r->ornate_animator_invoked++;
        break;

    /* ── ORNATE_STEP_ANIMATOR (0x41) skevent.cpp:1944-1949 ───────── */
    case DM2_ACTU_ORNATE_STEP_ANIMATOR:
        if (record_mut) {
            uint16_t new_data = (ctx->action_type == 0)
                              ? (uint16_t)(actu_data + 1)
                              : (uint16_t)(actu_data - 1);
            if (ctx->get_ornate_anim_len) {
                int16_t anim_len = ctx->get_ornate_anim_len(
                    ctx->ornate_ctx, record, 1);
                if (anim_len > 0)
                    new_data = (uint16_t)(new_data % (uint16_t)anim_len);
            }
            dm2_actu_set_data(record_mut, new_data & 0x1FF);
        }
        break;

    /* ── ORNATE_ANIMATOR / continuous (0x2c) skevent.cpp:1950-1953 ── */
    case DM2_ACTU_ORNATE_ANIMATOR: {
        r->ornate_animator_invoked++;
        if (!record_mut || !ctx->get_ornate_anim_len) {
            r->fail_closed++;
            break;
        }
        uint16_t prev_once = dm2_actu_once_only(record);
        dm2_actu_set_once_only(record_mut,
            dm2_toggle_actuator_message(ctx->action_type,
                dm2_actu_once_only(record)));
        if (dm2_actu_once_only(record_mut) != prev_once) {
            int16_t anim_len = ctx->get_ornate_anim_len(
                ctx->ornate_ctx, record, 1);
            if (anim_len < 1) anim_len = 1;
            if (dm2_actu_once_only(record_mut) == 1) {
                if (dm2_actu_active_status(record) == 0) {
                    dm2_actu_set_active_status(record_mut, 1);
                    /* SKWINSPX/src/v4/skevent.cpp:1307 sets
                     * glbDoLightCheck when this continuous animator starts
                     * on the player's map. The runtime owner applies the
                     * map gate and consumes this request after timer order. */
                    r->ornate_light_check_requested = 1;
                    uint16_t phase = (uint16_t)(
                        ((anim_len - (ctx->game_tick % (uint32_t)anim_len))
                         % (uint32_t)anim_len) & 0xFF);
                    dm2_actu_set_data(record_mut,
                        (actu_data & 0x100) | phase);
                    if (ctx->queue_ornate_sound)
                        ctx->queue_ornate_sound(ctx->ornate_sound_ctx,
                            handle, record_mut, ctx->map, ctx->x, ctx->y,
                            ctx->game_tick, 1);
                }
            } else {
                int16_t di = (int16_t)(((actu_data & 0xFF) + ctx->game_tick)
                              % (uint32_t)anim_len);
                if (di == 0) {
                    dm2_actu_set_active_status(record_mut, 0);
                    /* Source line 1319 requests the same check only when
                     * the stop phase is exactly zero. */
                    r->ornate_light_check_requested = 1;
                } else {
                    di = anim_len - di;
                    DM2_V1_SourceTimer t = make_timer(ctx->map,
                        ctx->game_tick + (uint32_t)di,
                        0x59, 0, handle, 0);
                    dm2_v1_source_timer_enqueue(ctx->queue, &t, 0);
                }
            }
        }
        if (dm2_actu_revert_effect(record) != 0 &&
            dm2_actu_action_type(record) == 3) {
            dm2_v1_invoke_actuator(ctx->queue, record,
                ctx->action_type, 0, ctx->map, ctx->game_tick);
        }
        break;
    }

    /* ── SWITCH_SIGN_CREATURE (0x26) skevent.cpp:1954-1957 ───────── */
    case DM2_ACTU_SWITCH_SIGN_CREATURE:
        if (record_mut) {
            dm2_actu_set_once_only(record_mut,
                dm2_toggle_actuator_message(ctx->action_type,
                    dm2_actu_once_only(record)));
        }
        break;

    /* ── PLACED_ITEM_TELEPORTER (0x3b) skevent.cpp:1958-1961 ────── */
    case DM2_ACTU_PLACED_ITEM_TELEPORTER: {
        DM2_V1_ItemTeleportReceipt it_receipt;
        int16_t si = dm2_v1_tile_record_link(ctx->dungeon, ctx->map,
            (int)dm2_actu_xcoord(record), (int)dm2_actu_ycoord(record));
        memset(&it_receipt, 0, sizeof(it_receipt));
        dm2_v1_activate_item_teleport_impl(ctx->pool_set, ctx->dungeon,
            ctx->asset_loader, ctx->queue,
            record, (int)dm2_actu_xcoord(record), (int)dm2_actu_ycoord(record),
            ctx->direction, ctx->action_type,
            0, 0, 0, 0, ctx->map, ctx->game_tick, ctx->rng, ctx->revive_champion,
            ctx->revive_ctx, &it_receipt);
        r->item_teleport_invoked++;
        if (it_receipt.fail_closed) r->fail_closed++;
        (void)si;
        break;
    }

    /* ── ITEM_CAPTURE (0x47) skevent.cpp:1962-1965 ──────────────── */
    case DM2_ACTU_ITEM_CAPTURE: {
        DM2_V1_ItemTeleportReceipt it_receipt;
        memset(&it_receipt, 0, sizeof(it_receipt));
        dm2_v1_activate_item_teleport_impl(ctx->pool_set, ctx->dungeon,
            ctx->asset_loader, ctx->queue,
            record, (int)dm2_actu_xcoord(record), (int)dm2_actu_ycoord(record),
            ctx->direction, ctx->action_type,
            0, 0, 1, 0, ctx->map, ctx->game_tick, ctx->rng, ctx->revive_champion,
            ctx->revive_ctx, &it_receipt);
        r->item_teleport_invoked++;
        if (it_receipt.fail_closed) r->fail_closed++;
        break;
    }

    /* ── ITEM_RECYCLER (0x40) skevent.cpp:1966-1969 ─────────────── */
    case DM2_ACTU_ITEM_RECYCLER: {
        DM2_V1_ItemTeleportReceipt it_receipt;
        memset(&it_receipt, 0, sizeof(it_receipt));
        dm2_v1_activate_item_teleport_impl(ctx->pool_set, ctx->dungeon,
            ctx->asset_loader, ctx->queue,
            record, (int)dm2_actu_xcoord(record), (int)dm2_actu_ycoord(record),
            ctx->direction, ctx->action_type,
            0, 1, 0, 0, ctx->map, ctx->game_tick, ctx->rng, ctx->revive_champion,
            ctx->revive_ctx, &it_receipt);
        r->item_teleport_invoked++;
        if (it_receipt.fail_closed) r->fail_closed++;
        break;
    }

    /* ── ITEM_TELEPORT_UNK (0x48) skevent.cpp:1970-1973 ─────────── */
    case DM2_ACTU_ITEM_TELEPORT_UNK: {
        DM2_V1_ItemTeleportReceipt it_receipt;
        memset(&it_receipt, 0, sizeof(it_receipt));
        dm2_v1_activate_item_teleport_impl(ctx->pool_set, ctx->dungeon,
            ctx->asset_loader, ctx->queue,
            record, (int)dm2_actu_xcoord(record), (int)dm2_actu_ycoord(record),
            ctx->direction, ctx->action_type,
            0, 0, 0, 1, ctx->map, ctx->game_tick, ctx->rng, ctx->revive_champion,
            ctx->revive_ctx, &it_receipt);
        r->item_teleport_invoked++;
        if (it_receipt.fail_closed) r->fail_closed++;
        break;
    }

    /* ── ITEM_CAPTURE_CREATURE (0x49) skevent.cpp:1974-1977 ─────── */
    case DM2_ACTU_ITEM_CAPTURE_CREATURE: {
        DM2_V1_ItemTeleportReceipt it_receipt;
        memset(&it_receipt, 0, sizeof(it_receipt));
        dm2_v1_activate_item_teleport_impl(ctx->pool_set, ctx->dungeon,
            ctx->asset_loader, ctx->queue,
            record, (int)dm2_actu_xcoord(record), (int)dm2_actu_ycoord(record),
            ctx->direction, ctx->action_type,
            0, 0, 1, 1, ctx->map, ctx->game_tick, ctx->rng, ctx->revive_champion,
            ctx->revive_ctx, &it_receipt);
        r->item_teleport_invoked++;
        if (it_receipt.fail_closed) r->fail_closed++;
        break;
    }

    /* ── ITEM_GENERATOR (0x3c) skevent.cpp:1978-1997 ────────────── */
    case DM2_ACTU_ITEM_GENERATOR: {
        r->item_generator_invoked++;
        /* ReDMCSB's ITEM_GENERATOR reaches ALLOC_NEW_DBITEM with a live
         * dballoc handler and source-owned item state.  `actu_data` alone
         * cannot prove an item record, quantity or placement, so do not
         * create the former generic record here. */
        r->fail_closed++;
        break;
    }

    /* ── INVERSE_FLAG (0x43) skevent.cpp:2002-2005 ──────────────── */
    case DM2_ACTU_INVERSE_FLAG: {
        int fwd_action = dm2_actu_once_only(record)
            ? (int)dm2_actu_action_type(record) : ctx->action_type;
        const int global_operation =
            (dm2_actu_revert_effect(record) ? 3 : 0) + ctx->action_type;
        if (!ctx->global_state) {
            r->fail_closed++;
            break;
        }
        (void)dm2_v1_update_glob_var_direct(
            ctx->global_state, (int16_t)dm2_actu_data(record),
            (int16_t)global_operation, 1);
        dm2_v1_invoke_actuator(ctx->queue, record,
            fwd_action, 0, ctx->map, ctx->game_tick);
        r->inverse_flag_invoked++;
        break;
    }

    /* ── TEST_FLAG (0x44) skevent.cpp:2006-2009 ─────────────────── */
    case DM2_ACTU_TEST_FLAG: {
        int fwd_action = dm2_actu_once_only(record)
            ? (int)dm2_actu_action_type(record) : ctx->action_type;
        int should_forward;
        int32_t gv_val;
        const int revert = dm2_actu_revert_effect(record) != 0;
        if (!ctx->global_state) {
            r->fail_closed++;
            break;
        }
        gv_val = dm2_v1_get_glob_var(
            ctx->global_state, dm2_actu_data(record));
        should_forward = ((revert != (gv_val != 0u)) &&
                          (ctx->action_type == 0 || ctx->action_type == 2)) ||
                         ((revert == (gv_val != 0u)) &&
                          ctx->action_type == 1);
        if (should_forward) {
            dm2_v1_invoke_actuator(ctx->queue, record,
                fwd_action, 0, ctx->map, ctx->game_tick);
        }
        r->test_flag_invoked++;
        break;
    }

    /* ── PUSH_BUTTON_SWITCH (0x46) skevent.cpp:2010-2028 ────────── */
    case DM2_ACTU_PUSH_BUTTON_SWITCH:
        r->push_button_invoked++;
        dm2_v1_push_button_switch(ctx->pool_set, ctx->dungeon,
                                   record, ctx->map, ctx->action_type, r);
        break;

    default:
        r->unknown_type_skipped++;
        break;
    }

    return 0; /* continue walking */
}

/* ── ACTUATE_WALL_MECHA ────────────────────────────────────────────── */

int dm2_v1_actuate_wall_mecha_with_rng_and_revive(
                              DM2_V1_RecordPoolSet *pool_set,
                              DM2_V1_DungeonData *dungeon,
                              DM2_V1_CaiiArray *caii,
                              const DM2_V1_AssetLoader *asset_loader,
                              DM2_V1_SourceTimerQueue *queue,
                              int map, int x, int y,
                              int action_type, int direction,
                              uint32_t game_tick,
                              DM2_V1_GlobVarState *global_state,
                              const DM2_V1_CreatureKillerCallbacks *creature_killer_cb,
                              DM2_V1_GetOrnateAnimLenFn get_ornate_anim_len,
                              void *ornate_ctx,
                              DM2_V1_QueueOrnateActivationSoundFn queue_ornate_sound,
                              void *ornate_sound_ctx,
                              DM2_V1_QueueShooterNoiseFn queue_shooter_noise,
                              void *shooter_sound_ctx,
                              DM2_V1_DropRng *rng,
                              DM2_V1_ReviveChampionFn revive_champion,
                              void *revive_ctx,
                              DM2_V1_ActuatorEventReceipt *receipt)
{
    WallMechaCtx ctx;
    DM2_V1_TileRecordWalkReceipt walk_receipt;

    if (receipt == NULL) return 0;
    memset(receipt, 0, sizeof(*receipt));

    if (pool_set == NULL || dungeon == NULL || queue == NULL) {
        receipt->fail_closed = 1;
        return 0;
    }

    ctx.pool_set    = pool_set;
    ctx.dungeon     = dungeon;
    ctx.asset_loader = asset_loader;
    ctx.caii        = caii;
    ctx.queue       = queue;
    ctx.map         = map;
    ctx.x           = x;
    ctx.y           = y;
    ctx.action_type = action_type;
    ctx.direction   = direction;
    ctx.game_tick   = game_tick;
    ctx.global_state      = global_state;
    ctx.creature_killer_cb = creature_killer_cb;
    ctx.get_ornate_anim_len = get_ornate_anim_len;
    ctx.ornate_ctx  = ornate_ctx;
    ctx.queue_ornate_sound = queue_ornate_sound;
    ctx.ornate_sound_ctx = ornate_sound_ctx;
    ctx.queue_shooter_noise = queue_shooter_noise;
    ctx.shooter_sound_ctx = shooter_sound_ctx;
    ctx.rng = rng;
    ctx.revive_champion = revive_champion;
    ctx.revive_ctx = revive_ctx;
    ctx.receipt     = receipt;

    memset(&walk_receipt, 0, sizeof(walk_receipt));
    dm2_v1_tile_record_walk(pool_set, dungeon, map, x, y,
                            wall_mecha_visitor, &ctx, &walk_receipt);

    receipt->valid = 1;
    return 1;
}

int dm2_v1_actuate_wall_mecha_with_rng(
                              DM2_V1_RecordPoolSet *pool_set,
                              DM2_V1_DungeonData *dungeon,
                              DM2_V1_CaiiArray *caii,
                              const DM2_V1_AssetLoader *asset_loader,
                              DM2_V1_SourceTimerQueue *queue,
                              int map, int x, int y,
                              int action_type, int direction,
                              uint32_t game_tick,
                              DM2_V1_GlobVarState *global_state,
                              const DM2_V1_CreatureKillerCallbacks *creature_killer_cb,
                              DM2_V1_GetOrnateAnimLenFn get_ornate_anim_len,
                              void *ornate_ctx,
                              DM2_V1_QueueOrnateActivationSoundFn queue_ornate_sound,
                              void *ornate_sound_ctx,
                              DM2_V1_QueueShooterNoiseFn queue_shooter_noise,
                              void *shooter_sound_ctx,
                              DM2_V1_DropRng *rng,
                              DM2_V1_ActuatorEventReceipt *receipt)
{
    return dm2_v1_actuate_wall_mecha_with_rng_and_revive(
        pool_set, dungeon, caii, asset_loader, queue, map, x, y,
        action_type, direction, game_tick, global_state, creature_killer_cb,
        get_ornate_anim_len, ornate_ctx, queue_ornate_sound,
        ornate_sound_ctx, queue_shooter_noise, shooter_sound_ctx, rng,
        NULL, NULL, receipt);
}

int dm2_v1_actuate_wall_mecha(DM2_V1_RecordPoolSet *pool_set,
                              DM2_V1_DungeonData *dungeon,
                              DM2_V1_CaiiArray *caii,
                              const DM2_V1_AssetLoader *asset_loader,
                              DM2_V1_SourceTimerQueue *queue,
                              int map, int x, int y,
                              int action_type, int direction,
                              uint32_t game_tick,
                              DM2_V1_GlobVarState *global_state,
                              const DM2_V1_CreatureKillerCallbacks *creature_killer_cb,
                              DM2_V1_GetOrnateAnimLenFn get_ornate_anim_len,
                              void *ornate_ctx,
                              DM2_V1_QueueOrnateActivationSoundFn queue_ornate_sound,
                              void *ornate_sound_ctx,
                              DM2_V1_ActuatorEventReceipt *receipt)
{
    return dm2_v1_actuate_wall_mecha_with_rng(
        pool_set, dungeon, caii, asset_loader, queue, map, x, y,
        action_type, direction, game_tick, global_state, creature_killer_cb,
        get_ornate_anim_len, ornate_ctx, queue_ornate_sound, ornate_sound_ctx,
        NULL, NULL, NULL, receipt);
}

/* ── Floor mecha visitor ───────────────────────────────────────────── */

static int floor_mecha_visitor(void *context,
                               int16_t handle,
                               const uint8_t *record)
{
    WallMechaCtx *ctx = (WallMechaCtx *)context;
    DM2_V1_ActuatorEventReceipt *r = ctx->receipt;
    int db_type;
    uint8_t atype;
    uint8_t *record_mut;

    r->records_visited++;
    db_type = dm2_v1_record_handle_pool(handle);

    /* Text records: toggle visibility (skevent.cpp:2085-2102) */
    if (db_type == DM2_DB_TEXT) {
        r->text_records_toggled++;
        return 0;
    }

    if (db_type != DM2_DB_ACTUATOR) return 0;

    atype = dm2_actu_type(record);
    r->actuators_dispatched++;
    record_mut = dm2_v1_record_pool_address_mut(ctx->pool_set, handle);

    switch (atype) {

    /* ── CONTINUOUS_ORNATE_ANIMATOR (0x2c) skevent.cpp:2117-2119 ── */
    case DM2_ACTU_ORNATE_ANIMATOR: {
        r->ornate_animator_invoked++;
        if (!record_mut || !ctx->get_ornate_anim_len) {
            r->fail_closed++;
            break;
        }
        uint16_t prev_once = dm2_actu_once_only(record);
        dm2_actu_set_once_only(record_mut,
            dm2_toggle_actuator_message(ctx->action_type,
                dm2_actu_once_only(record)));
        if (dm2_actu_once_only(record_mut) != prev_once) {
            int16_t anim_len = ctx->get_ornate_anim_len(
                ctx->ornate_ctx, record, 0);
            if (anim_len < 1) anim_len = 1;
            if (dm2_actu_once_only(record_mut) == 1) {
                if (dm2_actu_active_status(record) == 0) {
                    dm2_actu_set_active_status(record_mut, 1);
                    /* SKWINSPX/src/v4/skevent.cpp:1307 sets
                     * glbDoLightCheck when this continuous animator starts
                     * on the player's map. */
                    r->ornate_light_check_requested = 1;
                    uint16_t phase = (uint16_t)(
                        ((anim_len - (ctx->game_tick % (uint32_t)anim_len))
                         % (uint32_t)anim_len) & 0xFF);
                    dm2_actu_set_data(record_mut,
                        (dm2_actu_data(record) & 0x100) | phase);
                    if (ctx->queue_ornate_sound)
                        ctx->queue_ornate_sound(ctx->ornate_sound_ctx,
                            handle, record_mut, ctx->map, ctx->x, ctx->y,
                            ctx->game_tick, 0);
                }
            } else {
                int16_t di = (int16_t)(((dm2_actu_data(record) & 0xFF)
                              + ctx->game_tick) % (uint32_t)anim_len);
                if (di == 0) {
                    dm2_actu_set_active_status(record_mut, 0);
                    /* Source line 1319 requests the same check only when
                     * the stop phase is exactly zero. */
                    r->ornate_light_check_requested = 1;
                } else {
                    di = anim_len - di;
                    DM2_V1_SourceTimer t = make_timer(ctx->map,
                        ctx->game_tick + (uint32_t)di,
                        0x59, 0, handle, 0);
                    dm2_v1_source_timer_enqueue(ctx->queue, &t, 0);
                }
            }
        }
        if (dm2_actu_revert_effect(record) != 0 &&
            dm2_actu_action_type(record) == 3) {
            dm2_v1_invoke_actuator(ctx->queue, record,
                ctx->action_type, 0, ctx->map, ctx->game_tick);
        }
        break;
    }

    /* ── RELAY_1 (0x20), RELAY_3 (0x45) skevent.cpp:2120-2124 ──── */
    case DM2_ACTU_RELAY_1:
    case DM2_ACTU_RELAY_3:
        dm2_v1_activate_relay1(ctx->pool_set, ctx->dungeon, ctx->queue,
            record, ctx->action_type,
            (atype == DM2_ACTU_RELAY_3) ? 1 : 0,
            ctx->map, ctx->game_tick);
        r->relay1_invoked++;
        break;

    /* ── RELAY_2 (0x3d) skevent.cpp:2125-2128 ────────────────────── */
    case DM2_ACTU_RELAY_2:
        dm2_v1_activate_relay2(ctx->pool_set, ctx->dungeon, ctx->queue,
            record, ctx->action_type, (int)dm2_actu_data(record),
            ctx->map, ctx->game_tick);
        r->relay2_invoked++;
        break;

    /* ── CROSS_SCENE (0x27) skevent.cpp:2112-2115 ────────────── */
    case DM2_ACTU_CROSS_SCENE:
        if (record_mut) {
            dm2_actu_set_once_only(record_mut,
                dm2_toggle_actuator_message(ctx->action_type,
                    dm2_actu_once_only(record)));
        }
        r->cross_scene_invoked++;
        break;

    /* ── CREATURE_KILLER (0x0b) + CREATURE_AI_STATE (0x28)
     *    skevent.cpp:2129-2141 — both call ACTIVATE_CREATURE_KILLER. */
    case DM2_ACTU_CREATURE_KILLER:
    case DM2_ACTU_CREATURE_AI_STATE: {
        uint16_t ck_data = dm2_actu_data(record);
        DM2_V1_CreatureKillerReceipt ck_receipt;
        r->creature_killer_invoked++;
        dm2_v1_activate_creature_killer(
            ctx->creature_killer_cb,
            (int16_t)(ck_data & 0x0F),
            (int16_t)((ck_data >> 4) & 0x1F),
            (int16_t)ctx->x, (int16_t)ctx->y,
            (int16_t)dm2_actu_xcoord(record),
            (int16_t)dm2_actu_ycoord(record),
            (int16_t)atype, (int16_t)ctx->action_type,
            &ck_receipt);
        if (!ck_receipt.valid) r->fail_closed++;
        break;
    }

    /* ── CREATURE_DIRECTION (0x42) skevent.cpp:2234-2236 ─────── */
    /* Source: _3a15_0d5c is a TODO/no-op in skproject. */
    case DM2_ACTU_CREATURE_DIRECTION:
        r->creature_direction_invoked++;
        break;

    /* ── CREATURE_ANIMATOR (0x3a) c_tim_proc.cpp:3177-3184 ───── */
    case DM2_ACTU_CREATURE_ANIMATOR:
        r->creature_animator_invoked++;
        if (ctx->caii) {
            DM2_V1_CaiiAnimateActivationReceipt anim;
            memset(&anim, 0, sizeof(anim));
            dm2_v1_caii_animate_activation(
                ctx->pool_set, (const DM2_V1_DungeonData *)ctx->dungeon,
                ctx->caii, ctx->queue, ctx->map, (unsigned long)ctx->game_tick,
                ctx->x, ctx->y, &anim);
        } else {
            r->fail_closed++;
        }
        break;

    /* ── ORNATE_ANIMATOR_2 (0x32) skevent.cpp:2206-2209 ──────────── */
    case DM2_ACTU_ORNATE_ANIMATOR_2:
        if (dm2_actu_active_status(record) == 0 && record_mut) {
            dm2_actu_set_active_status(record_mut, 1);
            dm2_actu_set_data(record_mut, 0);
            {
                DM2_V1_SourceTimer t = make_timer(ctx->map,
                    ctx->game_tick + 1,
                    DM2_V1_TIMER_ORNATE_ANIMATOR, 0,
                    handle, 0 /* isWall=0 */);
                dm2_v1_source_timer_enqueue(ctx->queue, &t, 0);
            }
            if (ctx->queue_ornate_sound)
                ctx->queue_ornate_sound(ctx->ornate_sound_ctx, handle,
                    record_mut, ctx->map, ctx->x, ctx->y,
                    ctx->game_tick, 0);
        }
        if (dm2_actu_once_only(record)) {
            dm2_v1_activate_relay2(ctx->pool_set, ctx->dungeon, ctx->queue,
                record, ctx->action_type, 0, ctx->map, ctx->game_tick);
        }
        r->ornate_animator_invoked++;
        break;

    /* ── PLACED_ITEM_TELEPORTER (0x3b) skevent.cpp:2214-2217 ────── */
    case DM2_ACTU_PLACED_ITEM_TELEPORTER: {
        DM2_V1_ItemTeleportReceipt it_receipt;
        memset(&it_receipt, 0, sizeof(it_receipt));
        dm2_v1_activate_item_teleport_impl(ctx->pool_set, ctx->dungeon,
            ctx->asset_loader, ctx->queue,
            record, (int)dm2_actu_xcoord(record), (int)dm2_actu_ycoord(record),
            ctx->direction, ctx->action_type,
            1, 0, 0, 0, ctx->map, ctx->game_tick, ctx->rng, ctx->revive_champion,
            ctx->revive_ctx, &it_receipt);
        r->item_teleport_invoked++;
        if (it_receipt.fail_closed) r->fail_closed++;
        break;
    }

    /* ── ITEM_CAPTURE (0x47) skevent.cpp:2218-2221 ─────────────── */
    case DM2_ACTU_ITEM_CAPTURE: {
        DM2_V1_ItemTeleportReceipt it_receipt;
        memset(&it_receipt, 0, sizeof(it_receipt));
        dm2_v1_activate_item_teleport_impl(ctx->pool_set, ctx->dungeon,
            ctx->asset_loader, ctx->queue,
            record, (int)dm2_actu_xcoord(record), (int)dm2_actu_ycoord(record),
            ctx->direction, ctx->action_type,
            1, 0, 1, 0, ctx->map, ctx->game_tick, ctx->rng, ctx->revive_champion,
            ctx->revive_ctx, &it_receipt);
        r->item_teleport_invoked++;
        if (it_receipt.fail_closed) r->fail_closed++;
        break;
    }

    /* ── ITEM_RECYCLER (0x40) skevent.cpp:2222-2225 ────────────── */
    case DM2_ACTU_ITEM_RECYCLER: {
        DM2_V1_ItemTeleportReceipt it_receipt;
        memset(&it_receipt, 0, sizeof(it_receipt));
        dm2_v1_activate_item_teleport_impl(ctx->pool_set, ctx->dungeon,
            ctx->asset_loader, ctx->queue,
            record, (int)dm2_actu_xcoord(record), (int)dm2_actu_ycoord(record),
            ctx->direction, ctx->action_type,
            1, 1, 0, 0, ctx->map, ctx->game_tick, ctx->rng, ctx->revive_champion,
            ctx->revive_ctx, &it_receipt);
        r->item_teleport_invoked++;
        if (it_receipt.fail_closed) r->fail_closed++;
        break;
    }

    /* ── ITEM_TELEPORT_UNK (0x48) skevent.cpp:2226-2229 ────────── */
    case DM2_ACTU_ITEM_TELEPORT_UNK: {
        DM2_V1_ItemTeleportReceipt it_receipt;
        memset(&it_receipt, 0, sizeof(it_receipt));
        dm2_v1_activate_item_teleport_impl(ctx->pool_set, ctx->dungeon,
            ctx->asset_loader, ctx->queue,
            record, (int)dm2_actu_xcoord(record), (int)dm2_actu_ycoord(record),
            ctx->direction, ctx->action_type,
            1, 0, 0, 1, ctx->map, ctx->game_tick, ctx->rng, ctx->revive_champion,
            ctx->revive_ctx, &it_receipt);
        r->item_teleport_invoked++;
        if (it_receipt.fail_closed) r->fail_closed++;
        break;
    }

    /* ── ITEM_CAPTURE_CREATURE (0x49) skevent.cpp:2230-2233 ─────── */
    case DM2_ACTU_ITEM_CAPTURE_CREATURE: {
        DM2_V1_ItemTeleportReceipt it_receipt;
        memset(&it_receipt, 0, sizeof(it_receipt));
        dm2_v1_activate_item_teleport_impl(ctx->pool_set, ctx->dungeon,
            ctx->asset_loader, ctx->queue,
            record, (int)dm2_actu_xcoord(record), (int)dm2_actu_ycoord(record),
            ctx->direction, ctx->action_type,
            1, 0, 1, 1, ctx->map, ctx->game_tick, ctx->rng, ctx->revive_champion,
            ctx->revive_ctx, &it_receipt);
        r->item_teleport_invoked++;
        if (it_receipt.fail_closed) r->fail_closed++;
        break;
    }

    /* ── INVERSE_FLAG (0x43) skevent.cpp:2238-2241 ──────────────── */
    case DM2_ACTU_INVERSE_FLAG: {
        int fwd_action = dm2_actu_once_only(record)
            ? (int)dm2_actu_action_type(record) : ctx->action_type;
        const int global_operation =
            (dm2_actu_revert_effect(record) ? 3 : 0) + ctx->action_type;
        if (!ctx->global_state) {
            r->fail_closed++;
            break;
        }
        (void)dm2_v1_update_glob_var_direct(
            ctx->global_state, (int16_t)dm2_actu_data(record),
            (int16_t)global_operation, 1);
        dm2_v1_invoke_actuator(ctx->queue, record,
            fwd_action, 0, ctx->map, ctx->game_tick);
        r->inverse_flag_invoked++;
        break;
    }

    /* ── TEST_FLAG (0x44) skevent.cpp:2242-2245 ─────────────────── */
    case DM2_ACTU_TEST_FLAG: {
        int fwd_action = dm2_actu_once_only(record)
            ? (int)dm2_actu_action_type(record) : ctx->action_type;
        int should_forward;
        int32_t gv_val;
        const int revert = dm2_actu_revert_effect(record) != 0;
        if (!ctx->global_state) {
            r->fail_closed++;
            break;
        }
        gv_val = dm2_v1_get_glob_var(
            ctx->global_state, dm2_actu_data(record));
        should_forward = ((revert != (gv_val != 0u)) &&
                          (ctx->action_type == 0 || ctx->action_type == 2)) ||
                         ((revert == (gv_val != 0u)) &&
                          ctx->action_type == 1);
        if (should_forward) {
            dm2_v1_invoke_actuator(ctx->queue, record,
                fwd_action, 0, ctx->map, ctx->game_tick);
        }
        r->test_flag_invoked++;
        break;
    }

    default:
        r->unknown_type_skipped++;
        break;
    }

    return 0;
}

/* ── ACTUATE_FLOOR_MECHA ───────────────────────────────────────────── */

int dm2_v1_actuate_floor_mecha_with_revive(
                               DM2_V1_RecordPoolSet *pool_set,
                               DM2_V1_DungeonData *dungeon,
                               DM2_V1_CaiiArray *caii,
                               const DM2_V1_AssetLoader *asset_loader,
                               DM2_V1_SourceTimerQueue *queue,
                               int map, int x, int y,
                               int action_type, int direction,
                               uint32_t game_tick,
                               DM2_V1_GlobVarState *global_state,
                               const DM2_V1_CreatureKillerCallbacks *creature_killer_cb,
                               DM2_V1_GetOrnateAnimLenFn get_ornate_anim_len,
                               void *ornate_ctx,
                               DM2_V1_QueueOrnateActivationSoundFn queue_ornate_sound,
                               void *ornate_sound_ctx,
                               DM2_V1_DropRng *rng,
                               DM2_V1_ReviveChampionFn revive_champion,
                               void *revive_ctx,
                               DM2_V1_ActuatorEventReceipt *receipt)
{
    WallMechaCtx ctx;
    DM2_V1_TileRecordWalkReceipt walk_receipt;

    if (receipt == NULL) return 0;
    memset(receipt, 0, sizeof(*receipt));

    if (pool_set == NULL || dungeon == NULL || queue == NULL) {
        receipt->fail_closed = 1;
        return 0;
    }

    ctx.pool_set    = pool_set;
    ctx.dungeon     = dungeon;
    ctx.asset_loader = asset_loader;
    ctx.caii        = caii;
    ctx.queue       = queue;
    ctx.map         = map;
    ctx.x           = x;
    ctx.y           = y;
    ctx.action_type = action_type;
    ctx.direction   = direction;
    ctx.game_tick   = game_tick;
    ctx.global_state      = global_state;
    ctx.creature_killer_cb = creature_killer_cb;
    ctx.get_ornate_anim_len = get_ornate_anim_len;
    ctx.ornate_ctx  = ornate_ctx;
    ctx.queue_ornate_sound = queue_ornate_sound;
    ctx.ornate_sound_ctx = ornate_sound_ctx;
    ctx.rng = rng;
    ctx.revive_champion = revive_champion;
    ctx.revive_ctx = revive_ctx;
    ctx.receipt     = receipt;

    memset(&walk_receipt, 0, sizeof(walk_receipt));
    dm2_v1_tile_record_walk(pool_set, dungeon, map, x, y,
                            floor_mecha_visitor, &ctx, &walk_receipt);

    receipt->valid = 1;
    return 1;
}

int dm2_v1_actuate_floor_mecha(DM2_V1_RecordPoolSet *pool_set,
                               DM2_V1_DungeonData *dungeon,
                               DM2_V1_CaiiArray *caii,
                               const DM2_V1_AssetLoader *asset_loader,
                               DM2_V1_SourceTimerQueue *queue,
                               int map, int x, int y,
                               int action_type, int direction,
                               uint32_t game_tick,
                               DM2_V1_GlobVarState *global_state,
                               const DM2_V1_CreatureKillerCallbacks *creature_killer_cb,
                               DM2_V1_GetOrnateAnimLenFn get_ornate_anim_len,
                               void *ornate_ctx,
                               DM2_V1_QueueOrnateActivationSoundFn queue_ornate_sound,
                               void *ornate_sound_ctx,
                               DM2_V1_DropRng *rng,
                               DM2_V1_ActuatorEventReceipt *receipt)
{
    return dm2_v1_actuate_floor_mecha_with_revive(
        pool_set, dungeon, caii, asset_loader, queue, map, x, y,
        action_type, direction, game_tick, global_state, creature_killer_cb,
        get_ornate_anim_len, ornate_ctx, queue_ornate_sound, ornate_sound_ctx,
        rng, NULL, NULL, receipt);
}

/* ── ACTIVATE_CREATURE_KILLER — c_tim_proc.cpp:2907-3007 ──────────── */
int dm2_v1_activate_creature_killer(
    const DM2_V1_CreatureKillerCallbacks *cb,
    int16_t actu_data_lo, int16_t actu_data_hi,
    int16_t actu_x, int16_t actu_y,
    int16_t timer_x, int16_t timer_y,
    int16_t actu_type, int16_t action_type,
    DM2_V1_CreatureKillerReceipt *receipt)
{
    int dx_abs, dy_abs, origin_x, origin_y, span_x, span_y;
    int ix, iy;

    if (!receipt) return 0;
    memset(receipt, 0, sizeof(*receipt));
    if (!cb || !cb->get_creature_at || !cb->creature_type_matches) return 0;

    /* c_tim_proc.cpp:2925-2934 truncates the differences to signed 16-bit
     * before taking their absolute values. Keep the accumulator wide so
     * the -32768 case becomes +32768 instead of overflowing. */
    dx_abs = (int)(int16_t)(actu_x - timer_x);
    dy_abs = (int)(int16_t)(actu_y - timer_y);
    if (dx_abs < 0) dx_abs = -dx_abs;
    if (dy_abs < 0) dy_abs = -dy_abs;
    origin_x = (int)actu_x - dx_abs;
    origin_y = (int)actu_y - dy_abs;
    span_x = 2 * dx_abs + 1;
    span_y = 2 * dy_abs + 1;

    for (iy = span_y - 1; iy >= 0; iy--) {
        for (ix = span_x - 1; ix >= 0; ix--) {
            int cx = origin_x + ix;
            int cy = origin_y + iy;
            uint16_t creature_rec;

            if (cx < 0 || cx >= cb->map_width) continue;
            if (cy < 0 || cy >= cb->map_height) continue;
            receipt->cells_scanned++;

            creature_rec = cb->get_creature_at(cb->ctx, cx, cy);
            if (creature_rec == 0xFFFFu) continue;
            receipt->creatures_found++;

            if (!cb->creature_type_matches(cb->ctx, creature_rec, (uint16_t)actu_data_hi))
                continue;

            if (actu_type == 0x0b) {
                if (actu_data_lo == 0) continue;
                if (actu_data_lo == 1) continue;
                if (actu_data_lo != 2) {
                    receipt->valid = 1;
                    return 1;
                }
                if (cb->set_creature_ai_state) {
                    cb->set_creature_ai_state(cb->ctx, creature_rec,
                                              cx, cy, 0x13, 1);
                    receipt->ai_stops++;
                }
            } else if (actu_type == 0x28) {
                int32_t damage = (int32_t)(uint16_t)actu_data_lo;
                if (action_type != 0)
                    damage |= 0x8000;
                if (cb->attack_creature) {
                    cb->attack_creature(cb->ctx, creature_rec,
                                        cx, cy, damage, 0x64, 0);
                    receipt->attacks++;
                }
            }
        }
    }

    receipt->valid = 1;
    return 1;
}
