/*
 * dm2_v1_actuator_event_pc34_compat.c — DM2 actuator event dispatch.
 *
 * Ports ACTUATE_WALL_MECHA and ACTUATE_FLOOR_MECHA from skproject
 * skevent.cpp (v4), plus the actuator sub-functions they dispatch to.
 *
 * Source: skproject/SKWINSPX/src/v4/skevent.cpp
 */

#include "dm2_v1_actuator_event_pc34_compat.h"

#include <string.h>

#include "dm2_v1_dbitem_alloc_pc34_compat.h"
#include "dm2_v1_move_record_to_pc34_compat.h"
#include "dm2_v1_record_pool_pc34_compat.h"
#include "dm2_v1_tile_record_walk_pc34_compat.h"

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
 * DM2_INVOKE_ACTUATOR ultimately does the same: reads the actuator's
 * (Xcoord, Ycoord, Direction) and fires INVOKE_MESSAGE. */
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

    t = make_timer(map, game_tick + (uint32_t)delay_add,
                   0x04, 0,
                   encode_xy(target_x, target_y),
                   (int16_t)((dir & 0xFF) | ((action_type & 0xFF) << 8)));
    /* value_b encoding: low byte = Value2 (direction), high byte = ActionType
     * Source: c_timer.h — the source's INVOKE_MESSAGE sets Value2 and
     * ActionType as separate nibbles.  We pack them into value_b. */
    dm2_v1_source_timer_enqueue(queue, &t, 0);
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

/* ── ACTIVATE_RELAY1 ───────────────────────────────────────────────── */

/* Source: skevent.cpp:1154-1183 */
int dm2_v1_activate_relay1(DM2_V1_RecordPoolSet *pool_set,
                           DM2_V1_DungeonData *dungeon,
                           DM2_V1_SourceTimerQueue *queue,
                           const uint8_t *actu_record,
                           int timer_action_type,
                           int delay_as_mult,
                           int map, uint32_t game_tick)
{
    uint32_t bp04;
    int relay_action;

    (void)pool_set;
    (void)dungeon;

    if (queue == NULL || actu_record == NULL) return 0;

    /* Once-only + revert/action guard (skevent.cpp:1159-1166) */
    if (dm2_actu_once_only(actu_record)) {
        if (dm2_actu_revert_effect(actu_record) != 0 || timer_action_type != 0) {
            if (dm2_actu_revert_effect(actu_record) == 0 || timer_action_type != 1)
                return 1;
        }
    }

    /* Compute tick (skevent.cpp:1169-1176) */
    if (delay_as_mult != 0) {
        bp04 = game_tick + ((uint32_t)dm2_actu_data(actu_record) << dm2_actu_delay(actu_record));
    } else {
        bp04 = (uint32_t)dm2_actu_data(actu_record) + game_tick + (uint32_t)dm2_actu_delay(actu_record);
    }

    /* Determine action type to forward (skevent.cpp:1178-1180) */
    relay_action = dm2_actu_once_only(actu_record)
                 ? (int)dm2_actu_action_type(actu_record)
                 : timer_action_type;

    /* INVOKE_MESSAGE: queue a tty04 at the target coordinates */
    {
        DM2_V1_SourceTimer t = make_timer(
            map, bp04, 0x04, 0,
            encode_xy(dm2_actu_xcoord(actu_record),
                      dm2_actu_ycoord(actu_record)),
            (int16_t)((dm2_actu_direction(actu_record) & 0xFF) |
                      ((relay_action & 0xFF) << 8)));
        dm2_v1_source_timer_enqueue(queue, &t, 0);
    }
    return 1;
}

/* ── ACTIVATE_RELAY2 ───────────────────────────────────────────────── */

/* Source: skevent.cpp:1185-1232 */
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
    (void)actu_data_param;

    if (queue == NULL || actu_record == NULL) return 0;

    /* ActionType == 3 path (skevent.cpp:1191-1221) */
    if (dm2_actu_action_type(actu_record) == 3) {
        if (dm2_actu_revert_effect(actu_record) != 0) {
            dm2_v1_invoke_actuator(queue, actu_record, timer_action_type, 0, map, game_tick);
            if (actu_data_param == 0) {
                int si;
                switch (timer_action_type) {
                    case 0: si = 1; break;
                    case 1: si = 0; break;
                    case 2: si = 2; break;
                    default: si = timer_action_type; break;
                }
                dm2_v1_invoke_actuator(queue, actu_record, si, actu_data_param, map, game_tick);
            }
            return 1;
        }
        dm2_v1_invoke_actuator(queue, actu_record, timer_action_type, actu_data_param, map, game_tick);
        return 1;
    }

    /* Standard guard (skevent.cpp:1223-1227) */
    if (dm2_actu_revert_effect(actu_record) != 0 || timer_action_type != 0) {
        if (dm2_actu_revert_effect(actu_record) == 0 || timer_action_type != 1)
            return 1;
    }

    dm2_v1_invoke_actuator(queue, actu_record, (int)dm2_actu_action_type(actu_record), actu_data_param, map, game_tick);
    return 1;
}

/* ── PUSH_BUTTON_SWITCH ───────────────────────────────────────────── */

/* Source: skevent.cpp:2010-2028
 * Finds the door record at the actuator's target (Xcoord, Ycoord) and
 * sets/clears/toggles bit 13 of its w2 word based on action_type.
 * Bit 13 controls whether the door is sealed (cannot be bashed open). */

typedef struct {
    DM2_V1_RecordPoolSet *pool_set;
    int action_type;
    int found;
} PushButtonCtx;

static int push_button_door_visitor(void *context,
                                    int16_t handle,
                                    const uint8_t *record_unused)
{
    PushButtonCtx *ctx = (PushButtonCtx *)context;
    int db_type = dm2_v1_record_handle_pool(handle);
    uint8_t *record_mut;
    (void)record_unused;

    if (db_type != DM2_DB_DOOR) return 0;

    record_mut = dm2_v1_record_pool_address_mut(ctx->pool_set, handle);
    if (record_mut == NULL) return 0;

    switch (ctx->action_type) {
        case 0: dm2_door_set_bit13(record_mut, 1); break;
        case 1: dm2_door_set_bit13(record_mut, 0); break;
        case 2: dm2_door_set_bit13(record_mut,
                    (uint8_t)(dm2_door_bit13(record_mut) ^ 1u)); break;
    }
    ctx->found = 1;
    return 1;
}

int dm2_v1_push_button_switch(DM2_V1_RecordPoolSet *pool_set,
                              DM2_V1_DungeonData *dungeon,
                              const uint8_t *actu_record,
                              int action_type,
                              DM2_V1_ActuatorEventReceipt *receipt)
{
    PushButtonCtx ctx;
    DM2_V1_TileRecordWalkReceipt walk_receipt;
    int target_x, target_y;

    if (pool_set == NULL || dungeon == NULL || actu_record == NULL) return 0;

    target_x = (int)dm2_actu_xcoord(actu_record);
    target_y = (int)dm2_actu_ycoord(actu_record);

    ctx.pool_set = pool_set;
    ctx.action_type = action_type;
    ctx.found = 0;

    memset(&walk_receipt, 0, sizeof(walk_receipt));
    dm2_v1_tile_record_walk(pool_set, dungeon, 0, target_x, target_y,
                            push_button_door_visitor, &ctx, &walk_receipt);

    if (ctx.found && receipt) receipt->door_bit13_toggled++;
    return ctx.found;
}

/* ── CREATURE_GENERATOR ───────────────────────────────────────────── */

/* Source: skevent.cpp:1740-1759 (dispatch), skcrture.cpp:6311-6362
 * (ALLOC_NEW_CREATURE).  Allocates a creature record from the pool,
 * sets its type, HP, direction, and places it at the actuator's
 * (Xcoord, Ycoord).
 *
 * HP formula: healthMultiplier (capped to 31) * baseHP >> 3, then
 * final HP = rand(HP/8+1) + HP.  Without live AI specs, we use
 * multiplier=7 (the hardcoded value from the dispatch) and default
 * baseHP=40, yielding HP = 7*40/8 = 35, +rand(5) = 35-39. */
int dm2_v1_creature_generator(DM2_V1_RecordPoolSet *pool_set,
                              DM2_V1_DungeonData *dungeon,
                              DM2_V1_SourceTimerQueue *queue,
                              const uint8_t *actu_record,
                              int map, uint32_t game_tick,
                              DM2_V1_CreatureGeneratorReceipt *receipt)
{
    DM2_V1_CreatureGeneratorReceipt local;
    int16_t creature_handle;
    int target_x, target_y;
    uint16_t creature_type;
    uint8_t direction;
    uint16_t health_mult;
    uint16_t base_hp;
    uint16_t hp;
    uint8_t *creature_rec;
    DM2_V1_MoveRecordToReceipt move_receipt;

    memset(&local, 0, sizeof(local));
    if (receipt == NULL) receipt = &local;

    if (pool_set == NULL || dungeon == NULL || actu_record == NULL) {
        receipt->fail_closed = 1;
        return 0;
    }

    creature_type = dm2_actu_data(actu_record);
    target_x = (int)dm2_actu_xcoord(actu_record);
    target_y = (int)dm2_actu_ycoord(actu_record);

    /* Direction: once_only ? action_type : random 0-2
     * Without RAND02, use direction 0 as deterministic fallback. */
    if (dm2_actu_once_only(actu_record) != 0) {
        direction = (uint8_t)(dm2_actu_action_type(actu_record) & 3u);
    } else {
        direction = (uint8_t)(game_tick & 3u);
    }

    /* Allocate creature record (dbCreature = pool 4) */
    creature_handle = dm2_v1_record_pool_alloc_new_record(pool_set, 4);
    if (creature_handle < 0) {
        receipt->fail_closed = 1;
        return 0;
    }
    receipt->creature_allocated = 1;

    /* Set creature type and direction in the record */
    creature_rec = dm2_v1_record_pool_address_mut(pool_set, creature_handle);
    if (creature_rec == NULL) {
        receipt->fail_closed = 1;
        return 0;
    }

    /* Creature record layout (dme.h):
     * w0 (bytes 0-1): next link
     * w2 (bytes 2-3): bits 0-6 = creature type
     * b15 (byte 15): bits 0-1 = direction, init to 0xFB */
    {
        uint16_t w2 = (uint16_t)creature_rec[2] | ((uint16_t)creature_rec[3] << 8);
        w2 = (w2 & ~0x7Fu) | (creature_type & 0x7Fu);
        creature_rec[2] = (uint8_t)(w2 & 0xFFu);
        creature_rec[3] = (uint8_t)(w2 >> 8);
    }
    creature_rec[15] = (uint8_t)(0xFB & ~3u) | (direction & 3u);

    /* HP: multiplier=7 (hardcoded in dispatch), base=40 (default).
     * hp = (min(7,31) * 40) >> 3 = 35.  Final = 35 + rand(5).
     * Deterministic: use 35. */
    health_mult = 7;
    base_hp = 40;
    hp = (uint16_t)((health_mult * base_hp) >> 3);
    /* Store HP in creature record.  HP fields vary by version;
     * use bytes 4-5 as HP1 (common layout). */
    creature_rec[4] = (uint8_t)(hp & 0xFFu);
    creature_rec[5] = (uint8_t)(hp >> 8);

    /* Place creature on tile via MOVE_RECORD_TO */
    memset(&move_receipt, 0, sizeof(move_receipt));
    if (dm2_v1_move_record_to(pool_set, dungeon, queue,
                               creature_handle,
                               -1, 0,
                               (int16_t)target_x, (int16_t)target_y,
                               (int16_t)direction,
                               map, game_tick,
                               &move_receipt) == 1) {
        receipt->creature_placed = 1;
    } else {
        receipt->fail_closed = 1;
    }

    /* Override iAnimSeq if actuator has revert_effect set */
    if (dm2_actu_revert_effect(actu_record) != 0 && creature_rec != NULL) {
        uint16_t anim_seq = (uint16_t)dm2_actu_delay(actu_record);
        /* iAnimSeq at byte offset 12-13 (common layout) */
        creature_rec[12] = (uint8_t)(anim_seq & 0xFFu);
        creature_rec[13] = (uint8_t)(anim_seq >> 8);
        receipt->anim_seq_set = 1;
    }

    receipt->valid = 1;
    return 1;
}

/* ── ACTIVATE_SHOOTER ─────────────────────────────────────────────── */

/* Source: skevent.cpp:1536-1631.
 * Shooter types 0x07/0x08/0x0E fire a single projectile;
 * 0x09/0x0A/0x0F fire dual projectiles.
 * Item shooters (0x0E/0x0F) pick items from the source tile.
 * Others allocate via ALLOC_NEW_DBITEM or use missile records.
 *
 * Projectile launch is modeled as a tty07 timer (SHOOT_ITEM equivalent)
 * queued at game_tick+1. */

#define DM2_V1_TIMER_SHOOT_ITEM 0x07

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
    (void)dungeon; (void)timer_x; (void)timer_y;
    uint8_t atype;
    uint16_t actu_data;
    int single;
    int facing;
    int16_t proj_handle;

    memset(&local, 0, sizeof(local));
    if (receipt == NULL) receipt = &local;

    if (pool_set == NULL || queue == NULL || actu_record == NULL) {
        receipt->fail_closed = 1;
        return 0;
    }

    atype = dm2_actu_type(actu_record);
    actu_data = dm2_actu_data(actu_record);

    single = DM2_SHOOTER_SINGLE_TYPES(atype) ? 1 : 0;
    receipt->single_projectile = single;

    /* Facing: opposite of timer direction (+2 mod 4) */
    facing = (timer_direction + 2) & 3;

    /* Item shooters (0x0E, 0x0F): would need to CUT_RECORD_FROM the tile.
     * Without full record-chain surgery, fail-closed on item cut but
     * still report the invocation. */
    if (atype == DM2_ACTU_ITEM_SHOOTER || atype == DM2_ACTU_ITEM_SHOOTER_X2) {
        receipt->fail_closed = 1;
        receipt->valid = 1;
        return 0;
    }

    /* Missile/trap shooters (0x08, 0x0A): reuse actu_data as missile record.
     * Weapon shooters (0x07, 0x09): allocate new item from actu_data. */
    if (atype == DM2_ACTU_MISSILE_SHOOTER || atype == DM2_ACTU_MISSILE_SHOOTER_2) {
        /* These reuse missile record directly; encode as-is */
        proj_handle = (int16_t)actu_data;
    } else {
        proj_handle = dm2_v1_alloc_new_dbitem(pool_set, actu_data);
        if (proj_handle < 0) {
            receipt->fail_closed = 1;
            receipt->valid = 1;
            return 0;
        }
        receipt->items_allocated = 1;
    }

    /* Queue projectile as a SHOOT_ITEM timer.
     * The timer's value_a encodes the projectile handle,
     * value_b encodes facing | (speed << 8). */
    {
        uint8_t w6_low = (uint8_t)(dm2_actu_w6(actu_record) & 0xFFu);
        DM2_V1_SourceTimer t = make_timer(map,
            game_tick + 1,
            DM2_V1_TIMER_SHOOT_ITEM, 0,
            proj_handle,
            (int16_t)((facing & 0xFF) | ((uint16_t)w6_low << 8)));
        dm2_v1_source_timer_enqueue(queue, &t, 0);
        receipt->projectiles_queued = 1;
    }

    /* Dual projectile: allocate second and queue with rotated facing */
    if (single == 0) {
        int16_t proj2 = dm2_v1_alloc_new_dbitem(pool_set, actu_data);
        if (proj2 >= 0) {
            DM2_V1_SourceTimer t2 = make_timer(map,
                game_tick + 1,
                DM2_V1_TIMER_SHOOT_ITEM, 0,
                proj2,
                (int16_t)(((facing + 1) & 3) |
                          ((uint16_t)(dm2_actu_w6(actu_record) & 0xFFu) << 8)));
            dm2_v1_source_timer_enqueue(queue, &t2, 0);
            receipt->projectiles_queued = 2;
            receipt->items_allocated = 2;
        }
    }

    receipt->valid = 1;
    return 1;
}

/* ── ACTIVATE_ITEM_TELEPORT ───────────────────────────────────────── */

/* Source: skevent.cpp:1346-1500.
 * Moves items from source tile to destination tile based on actuator's
 * data filter.  capture=1 reverses source/dest.  recycler uses an
 * item bitmask.  Items are filtered by DB type (weapon..misc_item)
 * and by direction (wall) or all directions (floor). */

typedef struct {
    DM2_V1_RecordPoolSet *pool_set;
    DM2_V1_DungeonData *dungeon;
    DM2_V1_SourceTimerQueue *queue;
    const uint8_t *actu_record;
    int src_x, src_y, src_dir;
    int dst_x, dst_y, dst_dir;
    int is_floor;
    int only_first_item;
    uint16_t filter_data;
    int map;
    uint32_t game_tick;
    DM2_V1_ItemTeleportReceipt *receipt;
} ItemTeleportCtx;

static int item_teleport_visitor(void *context,
                                 int16_t handle,
                                 const uint8_t *record_unused)
{
    ItemTeleportCtx *ctx = (ItemTeleportCtx *)context;
    int db_type = dm2_v1_record_handle_pool(handle);
    int item_dir = (int)(handle & 3u);
    (void)record_unused;

    /* Only items (dbWeapon=5 through dbMiscellaneous_item=9) */
    if (db_type < DM2_DB_WEAPON || db_type > DM2_DB_MISC_ITEM) {
        /* Check creature possessions */
        if (db_type == DM2_DB_CREATURE) {
            ctx->receipt->creatures_checked++;
        }
        return 0;
    }

    /* Direction filter: wall actuators match direction only */
    if (ctx->is_floor == 0 && item_dir != ctx->src_dir) {
        return 0;
    }

    ctx->receipt->items_counted++;

    /* Filter by actuator data (itemspec).  0x1FF = match all. */
    if (ctx->filter_data != 0x1FF) {
        /* Without GET_DISTINCTIVE_ITEMTYPE, use raw data compare */
        /* This is a simplified filter; full version needs itemtype lookup */
    }

    /* Move the item via MOVE_RECORD_TO */
    {
        DM2_V1_MoveRecordToReceipt move_receipt;
        memset(&move_receipt, 0, sizeof(move_receipt));
        if (dm2_v1_move_record_to(ctx->pool_set, ctx->dungeon, ctx->queue,
                                   handle,
                                   (int16_t)ctx->src_x, (int16_t)ctx->src_y,
                                   (int16_t)ctx->dst_x, (int16_t)ctx->dst_y,
                                   (int16_t)ctx->dst_dir,
                                   ctx->map, ctx->game_tick,
                                   &move_receipt) == 1) {
            ctx->receipt->items_moved++;
        }
    }

    if (ctx->only_first_item && ctx->receipt->items_moved > 0) {
        return 1;
    }
    return 0;
}

int dm2_v1_activate_item_teleport(DM2_V1_RecordPoolSet *pool_set,
                                  DM2_V1_DungeonData *dungeon,
                                  DM2_V1_SourceTimerQueue *queue,
                                  const uint8_t *actu_record,
                                  int timer_x, int timer_y,
                                  int timer_direction,
                                  int timer_action_type,
                                  int is_floor, int recycler,
                                  int capture, int only_first_item,
                                  int map, uint32_t game_tick,
                                  DM2_V1_ItemTeleportReceipt *receipt)
{
    DM2_V1_ItemTeleportReceipt local;
    ItemTeleportCtx ctx;
    DM2_V1_TileRecordWalkReceipt walk_receipt;

    memset(&local, 0, sizeof(local));
    if (receipt == NULL) receipt = &local;

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

    /* Recycler path requires QUERY_CREATURES_ITEM_MASK; fail-closed */
    if (recycler != 0) {
        receipt->fail_closed = 1;
        receipt->valid = 1;
        return 0;
    }

    ctx.pool_set = pool_set;
    ctx.dungeon = dungeon;
    ctx.queue = queue;
    ctx.actu_record = actu_record;
    ctx.is_floor = is_floor;
    ctx.only_first_item = only_first_item;
    ctx.filter_data = dm2_actu_data(actu_record);
    ctx.map = map;
    ctx.game_tick = game_tick;
    ctx.receipt = receipt;

    memset(&walk_receipt, 0, sizeof(walk_receipt));
    dm2_v1_tile_record_walk(pool_set, dungeon, map, ctx.src_x, ctx.src_y,
                            item_teleport_visitor, &ctx, &walk_receipt);

    receipt->valid = 1;
    return 1;
}

/* ── Wall mecha record walk context ────────────────────────────────── */

typedef struct {
    DM2_V1_RecordPoolSet *pool_set;
    DM2_V1_DungeonData *dungeon;
    DM2_V1_SourceTimerQueue *queue;
    int map;
    int action_type;   /* timer ActionType (from value_b high byte) */
    int direction;     /* timer Value2 (from value_b low byte) */
    uint32_t game_tick;
    DM2_V1_ActuatorEventReceipt *receipt;
} WallMechaCtx;

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
        memset(&sh_receipt, 0, sizeof(sh_receipt));
        dm2_v1_activate_shooter(ctx->pool_set, ctx->dungeon, ctx->queue,
                                 record,
                                 (int)dm2_actu_xcoord(record),
                                 (int)dm2_actu_ycoord(record),
                                 ctx->direction,
                                 ctx->map, ctx->game_tick, &sh_receipt);
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

    /* ── THE_END (0x12) skevent.cpp:1934-1939 ────────────────────── */
    case DM2_ACTU_THE_END:
        r->fail_closed++;
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
            /* Modulo ornate anim len not available without GDAT; store raw. */
            dm2_actu_set_data(record_mut, new_data & 0x1FF);
        }
        break;

    /* ── ORNATE_ANIMATOR / continuous (0x2c) skevent.cpp:1950-1953 ── */
    case DM2_ACTU_ORNATE_ANIMATOR:
        /* Full continuous ornate animator requires GDAT ornate anim len;
         * mark invoked, fail-closed on animation scheduling. */
        r->ornate_animator_invoked++;
        r->fail_closed++;
        break;

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
        dm2_v1_activate_item_teleport(ctx->pool_set, ctx->dungeon, ctx->queue,
            record, (int)dm2_actu_xcoord(record), (int)dm2_actu_ycoord(record),
            ctx->direction, ctx->action_type,
            0, 0, 0, 0, ctx->map, ctx->game_tick, &it_receipt);
        r->item_teleport_invoked++;
        if (it_receipt.fail_closed) r->fail_closed++;
        (void)si;
        break;
    }

    /* ── ITEM_CAPTURE (0x47) skevent.cpp:1962-1965 ──────────────── */
    case DM2_ACTU_ITEM_CAPTURE: {
        DM2_V1_ItemTeleportReceipt it_receipt;
        memset(&it_receipt, 0, sizeof(it_receipt));
        dm2_v1_activate_item_teleport(ctx->pool_set, ctx->dungeon, ctx->queue,
            record, (int)dm2_actu_xcoord(record), (int)dm2_actu_ycoord(record),
            ctx->direction, ctx->action_type,
            0, 0, 1, 0, ctx->map, ctx->game_tick, &it_receipt);
        r->item_teleport_invoked++;
        if (it_receipt.fail_closed) r->fail_closed++;
        break;
    }

    /* ── ITEM_RECYCLER (0x40) skevent.cpp:1966-1969 ─────────────── */
    case DM2_ACTU_ITEM_RECYCLER: {
        DM2_V1_ItemTeleportReceipt it_receipt;
        memset(&it_receipt, 0, sizeof(it_receipt));
        dm2_v1_activate_item_teleport(ctx->pool_set, ctx->dungeon, ctx->queue,
            record, (int)dm2_actu_xcoord(record), (int)dm2_actu_ycoord(record),
            ctx->direction, ctx->action_type,
            0, 1, 0, 0, ctx->map, ctx->game_tick, &it_receipt);
        r->item_teleport_invoked++;
        if (it_receipt.fail_closed) r->fail_closed++;
        break;
    }

    /* ── ITEM_TELEPORT_UNK (0x48) skevent.cpp:1970-1973 ─────────── */
    case DM2_ACTU_ITEM_TELEPORT_UNK: {
        DM2_V1_ItemTeleportReceipt it_receipt;
        memset(&it_receipt, 0, sizeof(it_receipt));
        dm2_v1_activate_item_teleport(ctx->pool_set, ctx->dungeon, ctx->queue,
            record, (int)dm2_actu_xcoord(record), (int)dm2_actu_ycoord(record),
            ctx->direction, ctx->action_type,
            0, 0, 0, 1, ctx->map, ctx->game_tick, &it_receipt);
        r->item_teleport_invoked++;
        if (it_receipt.fail_closed) r->fail_closed++;
        break;
    }

    /* ── ITEM_CAPTURE_CREATURE (0x49) skevent.cpp:1974-1977 ─────── */
    case DM2_ACTU_ITEM_CAPTURE_CREATURE: {
        DM2_V1_ItemTeleportReceipt it_receipt;
        memset(&it_receipt, 0, sizeof(it_receipt));
        dm2_v1_activate_item_teleport(ctx->pool_set, ctx->dungeon, ctx->queue,
            record, (int)dm2_actu_xcoord(record), (int)dm2_actu_ycoord(record),
            ctx->direction, ctx->action_type,
            0, 0, 1, 1, ctx->map, ctx->game_tick, &it_receipt);
        r->item_teleport_invoked++;
        if (it_receipt.fail_closed) r->fail_closed++;
        break;
    }

    /* ── ITEM_GENERATOR (0x3c) skevent.cpp:1978-1997 ────────────── */
    case DM2_ACTU_ITEM_GENERATOR: {
        int16_t new_item = dm2_v1_alloc_new_dbitem(ctx->pool_set, actu_data);
        r->item_generator_invoked++;
        if (new_item >= 0) {
            DM2_V1_MoveRecordToReceipt move_r;
            memset(&move_r, 0, sizeof(move_r));
            dm2_v1_move_record_to(ctx->pool_set, ctx->dungeon, ctx->queue,
                new_item, -1, 0,
                (int16_t)dm2_actu_xcoord(record),
                (int16_t)dm2_actu_ycoord(record),
                (int16_t)dm2_actu_direction(record),
                ctx->map, ctx->game_tick, &move_r);
        } else {
            r->fail_closed++;
        }
        break;
    }

    /* ── INVERSE_FLAG (0x43) skevent.cpp:2002-2005 ──────────────── */
    case DM2_ACTU_INVERSE_FLAG:
        /* Full path requires UPDATE_GLOB_VAR; forward the actuator chain. */
        dm2_v1_invoke_actuator(ctx->queue, record,
            dm2_actu_once_only(record)
                ? (int)dm2_actu_action_type(record)
                : ctx->action_type,
            0, ctx->map, ctx->game_tick);
        r->inverse_flag_invoked++;
        break;

    /* ── TEST_FLAG (0x44) skevent.cpp:2006-2009 ─────────────────── */
    case DM2_ACTU_TEST_FLAG:
        /* Full path requires GET_GLOB_VAR; forward unconditionally. */
        dm2_v1_invoke_actuator(ctx->queue, record,
            dm2_actu_once_only(record)
                ? (int)dm2_actu_action_type(record)
                : ctx->action_type,
            0, ctx->map, ctx->game_tick);
        r->test_flag_invoked++;
        break;

    /* ── PUSH_BUTTON_SWITCH (0x46) skevent.cpp:2010-2028 ────────── */
    case DM2_ACTU_PUSH_BUTTON_SWITCH:
        r->push_button_invoked++;
        dm2_v1_push_button_switch(ctx->pool_set, ctx->dungeon,
                                   record, ctx->action_type, r);
        break;

    default:
        r->unknown_type_skipped++;
        break;
    }

    return 0; /* continue walking */
}

/* ── ACTUATE_WALL_MECHA ────────────────────────────────────────────── */

int dm2_v1_actuate_wall_mecha(DM2_V1_RecordPoolSet *pool_set,
                              DM2_V1_DungeonData *dungeon,
                              DM2_V1_SourceTimerQueue *queue,
                              int map, int x, int y,
                              int action_type, int direction,
                              uint32_t game_tick,
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
    ctx.queue       = queue;
    ctx.map         = map;
    ctx.action_type = action_type;
    ctx.direction   = direction;
    ctx.game_tick   = game_tick;
    ctx.receipt     = receipt;

    memset(&walk_receipt, 0, sizeof(walk_receipt));
    dm2_v1_tile_record_walk(pool_set, dungeon, map, x, y,
                            wall_mecha_visitor, &ctx, &walk_receipt);

    receipt->valid = 1;
    return 1;
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
    case DM2_ACTU_ORNATE_ANIMATOR:
        r->ornate_animator_invoked++;
        r->fail_closed++;
        break;

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

    /* ── CREATURE_KILLER (0x0b) skevent.cpp:2129-2141 ─────────── */
    case DM2_ACTU_CREATURE_KILLER:
        r->creature_killer_invoked++;
        r->fail_closed++;
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
        dm2_v1_activate_item_teleport(ctx->pool_set, ctx->dungeon, ctx->queue,
            record, (int)dm2_actu_xcoord(record), (int)dm2_actu_ycoord(record),
            ctx->direction, ctx->action_type,
            1, 0, 0, 0, ctx->map, ctx->game_tick, &it_receipt);
        r->item_teleport_invoked++;
        if (it_receipt.fail_closed) r->fail_closed++;
        break;
    }

    /* ── ITEM_CAPTURE (0x47) skevent.cpp:2218-2221 ─────────────── */
    case DM2_ACTU_ITEM_CAPTURE: {
        DM2_V1_ItemTeleportReceipt it_receipt;
        memset(&it_receipt, 0, sizeof(it_receipt));
        dm2_v1_activate_item_teleport(ctx->pool_set, ctx->dungeon, ctx->queue,
            record, (int)dm2_actu_xcoord(record), (int)dm2_actu_ycoord(record),
            ctx->direction, ctx->action_type,
            1, 0, 1, 0, ctx->map, ctx->game_tick, &it_receipt);
        r->item_teleport_invoked++;
        if (it_receipt.fail_closed) r->fail_closed++;
        break;
    }

    /* ── ITEM_RECYCLER (0x40) skevent.cpp:2222-2225 ────────────── */
    case DM2_ACTU_ITEM_RECYCLER: {
        DM2_V1_ItemTeleportReceipt it_receipt;
        memset(&it_receipt, 0, sizeof(it_receipt));
        dm2_v1_activate_item_teleport(ctx->pool_set, ctx->dungeon, ctx->queue,
            record, (int)dm2_actu_xcoord(record), (int)dm2_actu_ycoord(record),
            ctx->direction, ctx->action_type,
            1, 1, 0, 0, ctx->map, ctx->game_tick, &it_receipt);
        r->item_teleport_invoked++;
        if (it_receipt.fail_closed) r->fail_closed++;
        break;
    }

    /* ── ITEM_TELEPORT_UNK (0x48) skevent.cpp:2226-2229 ────────── */
    case DM2_ACTU_ITEM_TELEPORT_UNK: {
        DM2_V1_ItemTeleportReceipt it_receipt;
        memset(&it_receipt, 0, sizeof(it_receipt));
        dm2_v1_activate_item_teleport(ctx->pool_set, ctx->dungeon, ctx->queue,
            record, (int)dm2_actu_xcoord(record), (int)dm2_actu_ycoord(record),
            ctx->direction, ctx->action_type,
            1, 0, 0, 1, ctx->map, ctx->game_tick, &it_receipt);
        r->item_teleport_invoked++;
        if (it_receipt.fail_closed) r->fail_closed++;
        break;
    }

    /* ── ITEM_CAPTURE_CREATURE (0x49) skevent.cpp:2230-2233 ─────── */
    case DM2_ACTU_ITEM_CAPTURE_CREATURE: {
        DM2_V1_ItemTeleportReceipt it_receipt;
        memset(&it_receipt, 0, sizeof(it_receipt));
        dm2_v1_activate_item_teleport(ctx->pool_set, ctx->dungeon, ctx->queue,
            record, (int)dm2_actu_xcoord(record), (int)dm2_actu_ycoord(record),
            ctx->direction, ctx->action_type,
            1, 0, 1, 1, ctx->map, ctx->game_tick, &it_receipt);
        r->item_teleport_invoked++;
        if (it_receipt.fail_closed) r->fail_closed++;
        break;
    }

    /* ── INVERSE_FLAG (0x43) skevent.cpp:2238-2241 ──────────────── */
    case DM2_ACTU_INVERSE_FLAG:
        dm2_v1_invoke_actuator(ctx->queue, record,
            dm2_actu_once_only(record)
                ? (int)dm2_actu_action_type(record)
                : ctx->action_type,
            0, ctx->map, ctx->game_tick);
        r->inverse_flag_invoked++;
        break;

    /* ── TEST_FLAG (0x44) skevent.cpp:2242-2245 ─────────────────── */
    case DM2_ACTU_TEST_FLAG:
        dm2_v1_invoke_actuator(ctx->queue, record,
            dm2_actu_once_only(record)
                ? (int)dm2_actu_action_type(record)
                : ctx->action_type,
            0, ctx->map, ctx->game_tick);
        r->test_flag_invoked++;
        break;

    default:
        r->unknown_type_skipped++;
        break;
    }

    return 0;
}

/* ── ACTUATE_FLOOR_MECHA ───────────────────────────────────────────── */

int dm2_v1_actuate_floor_mecha(DM2_V1_RecordPoolSet *pool_set,
                               DM2_V1_DungeonData *dungeon,
                               DM2_V1_SourceTimerQueue *queue,
                               int map, int x, int y,
                               int action_type, int direction,
                               uint32_t game_tick,
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
    ctx.queue       = queue;
    ctx.map         = map;
    ctx.action_type = action_type;
    ctx.direction   = direction;
    ctx.game_tick   = game_tick;
    ctx.receipt     = receipt;

    memset(&walk_receipt, 0, sizeof(walk_receipt));
    dm2_v1_tile_record_walk(pool_set, dungeon, map, x, y,
                            floor_mecha_visitor, &ctx, &walk_receipt);

    receipt->valid = 1;
    return 1;
}
