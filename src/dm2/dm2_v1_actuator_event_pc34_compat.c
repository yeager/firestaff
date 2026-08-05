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

#include "dm2_v1_caii_alloc_pc34_compat.h"
#include "dm2_v1_dbitem_alloc_pc34_compat.h"
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

    /* Item shooters (0x0E, 0x0F): walk the tile chain at (timer_x, timer_y)
     * to find the first item record (db pool > 3) matching the shooter
     * direction, cut it from the tile, and fire it as the projectile.
     * Source: c_tim_proc.cpp:1697-1764, DM2_ACTIVATE_SHOOTER item path. */
    if (atype == DM2_ACTU_ITEM_SHOOTER || atype == DM2_ACTU_ITEM_SHOOTER_X2) {
        int16_t tile_head;
        int16_t cursor;
        int16_t found = DM2_V1_RECORD_HANDLE_END;
        int16_t found2 = DM2_V1_RECORD_HANDLE_END;
        int dir_match = (int)(actu_data >> 8) & 3;
        long walk_limit = 1024;
        int first;

        (void)dir_match;

        if (dungeon == NULL) {
            receipt->fail_closed = 1;
            receipt->valid = 1;
            return 0;
        }

        first = dm2_v1_dungeon_get_first_thing(dungeon, map, timer_x, timer_y);
        if (first < 0) {
            receipt->valid = 1;
            return 1;
        }
        tile_head = (int16_t)first;
        cursor = tile_head;

        while (cursor != DM2_V1_RECORD_HANDLE_END &&
               cursor != DM2_V1_RECORD_HANDLE_NULL && walk_limit-- > 0) {
            int pool = dm2_v1_record_handle_pool(cursor);
            int16_t next;
            if (pool > 3) {
                int rec_dir = (cursor >> 14) & 3;
                int match_dir = facing;
                int match_dir2 = (facing + 1) & 3;
                if (rec_dir == match_dir || rec_dir == match_dir2) {
                    if (found == DM2_V1_RECORD_HANDLE_END) {
                        found = cursor;
                    } else if (single == 0 &&
                               found2 == DM2_V1_RECORD_HANDLE_END) {
                        found2 = cursor;
                        break;
                    }
                }
            }
            if (!dm2_v1_record_pool_next_link(pool_set, cursor, &next))
                break;
            cursor = next;
        }

        if (found == DM2_V1_RECORD_HANDLE_END) {
            receipt->valid = 1;
            return 1;
        }

        dm2_v1_record_pool_cut_from_tile(pool_set, dungeon,
                                         map, timer_x, timer_y, found);
        receipt->items_cut = 1;
        proj_handle = found & 0x3FFF;

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

        if (single == 0 && found2 != DM2_V1_RECORD_HANDLE_END) {
            dm2_v1_record_pool_cut_from_tile(pool_set, dungeon,
                                             map, timer_x, timer_y, found2);
            receipt->items_cut = 2;
            {
                uint8_t w6_low = (uint8_t)(dm2_actu_w6(actu_record) & 0xFFu);
                DM2_V1_SourceTimer t2 = make_timer(map,
                    game_tick + 1,
                    DM2_V1_TIMER_SHOOT_ITEM, 0,
                    (int16_t)(found2 & 0x3FFF),
                    (int16_t)(((facing + 1) & 3) |
                              ((uint16_t)w6_low << 8)));
                dm2_v1_source_timer_enqueue(queue, &t2, 0);
                receipt->projectiles_queued = 2;
            }
        }

        receipt->valid = 1;
        return 1;
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

    /* Recycler path: QUERY_CREATURES_ITEM_MASK builds a bitmask of item
     * types a creature accepts.  Without GDAT text queries, use a
     * permissive mask (all item types pass).  Source: c_querydb.cpp:1045. */
    if (recycler != 0) {
        uint8_t item_mask[64];
        memset(item_mask, 0xFF, sizeof(item_mask));
        ctx.filter_data = 0x1FFU;
        receipt->recycler_permissive_mask = 1;
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
    DM2_V1_CaiiArray *caii;
    DM2_V1_SourceTimerQueue *queue;
    int map;
    int x;             /* timer X coordinate (tile walk origin) */
    int y;             /* timer Y coordinate (tile walk origin) */
    int action_type;   /* timer ActionType (from value_b high byte) */
    int direction;     /* timer Value2 (from value_b low byte) */
    uint32_t game_tick;
    uint16_t *global_words;
    uint16_t global_word_count;
    const DM2_V1_CreatureKillerCallbacks *creature_killer_cb;
    DM2_V1_GetOrnateAnimLenFn get_ornate_anim_len;
    void *ornate_ctx;
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
                    uint16_t phase = (uint16_t)(
                        ((anim_len - (ctx->game_tick % (uint32_t)anim_len))
                         % (uint32_t)anim_len) & 0xFF);
                    dm2_actu_set_data(record_mut,
                        (actu_data & 0x100) | phase);
                }
            } else {
                int16_t di = (int16_t)(((actu_data & 0xFF) + ctx->game_tick)
                              % (uint32_t)anim_len);
                if (di == 0) {
                    dm2_actu_set_active_status(record_mut, 0);
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
        if (ctx->global_words && ctx->global_word_count > 0) {
            DM2_V1_SkprojectUpdateGlobVarReceipt gv_receipt;
            dm2_v1_skproject_update_glob_var(
                dm2_actu_data(record), (uint16_t)fwd_action, 0,
                ctx->global_words, ctx->global_word_count, &gv_receipt);
        }
        dm2_v1_invoke_actuator(ctx->queue, record,
            fwd_action, 0, ctx->map, ctx->game_tick);
        r->inverse_flag_invoked++;
        break;
    }

    /* ── TEST_FLAG (0x44) skevent.cpp:2006-2009 ─────────────────── */
    case DM2_ACTU_TEST_FLAG: {
        int fwd_action = dm2_actu_once_only(record)
            ? (int)dm2_actu_action_type(record) : ctx->action_type;
        int should_forward = 1;
        if (ctx->global_words && ctx->global_word_count > 0) {
            uint16_t gv_val = 0;
            DM2_V1_SkprojectGetGlobVarReceipt gv_receipt;
            dm2_v1_skproject_get_glob_var(
                dm2_actu_data(record),
                ctx->global_words, ctx->global_word_count,
                &gv_val, &gv_receipt);
            if (gv_val == 0)
                should_forward = 0;
        }
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
                              DM2_V1_CaiiArray *caii,
                              DM2_V1_SourceTimerQueue *queue,
                              int map, int x, int y,
                              int action_type, int direction,
                              uint32_t game_tick,
                              uint16_t *global_words,
                              uint16_t global_word_count,
                              DM2_V1_GetOrnateAnimLenFn get_ornate_anim_len,
                              void *ornate_ctx,
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
    ctx.caii        = caii;
    ctx.queue       = queue;
    ctx.map         = map;
    ctx.x           = x;
    ctx.y           = y;
    ctx.action_type = action_type;
    ctx.direction   = direction;
    ctx.game_tick   = game_tick;
    ctx.global_words      = global_words;
    ctx.global_word_count = global_word_count;
    ctx.creature_killer_cb = NULL;
    ctx.get_ornate_anim_len = get_ornate_anim_len;
    ctx.ornate_ctx  = ornate_ctx;
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
                    uint16_t phase = (uint16_t)(
                        ((anim_len - (ctx->game_tick % (uint32_t)anim_len))
                         % (uint32_t)anim_len) & 0xFF);
                    dm2_actu_set_data(record_mut,
                        (dm2_actu_data(record) & 0x100) | phase);
                }
            } else {
                int16_t di = (int16_t)(((dm2_actu_data(record) & 0xFF)
                              + ctx->game_tick) % (uint32_t)anim_len);
                if (di == 0) {
                    dm2_actu_set_active_status(record_mut, 0);
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
    case DM2_ACTU_INVERSE_FLAG: {
        int fwd_action = dm2_actu_once_only(record)
            ? (int)dm2_actu_action_type(record) : ctx->action_type;
        if (ctx->global_words && ctx->global_word_count > 0) {
            DM2_V1_SkprojectUpdateGlobVarReceipt gv_receipt;
            dm2_v1_skproject_update_glob_var(
                dm2_actu_data(record), (uint16_t)fwd_action, 0,
                ctx->global_words, ctx->global_word_count, &gv_receipt);
        }
        dm2_v1_invoke_actuator(ctx->queue, record,
            fwd_action, 0, ctx->map, ctx->game_tick);
        r->inverse_flag_invoked++;
        break;
    }

    /* ── TEST_FLAG (0x44) skevent.cpp:2242-2245 ─────────────────── */
    case DM2_ACTU_TEST_FLAG: {
        int fwd_action = dm2_actu_once_only(record)
            ? (int)dm2_actu_action_type(record) : ctx->action_type;
        int should_forward = 1;
        if (ctx->global_words && ctx->global_word_count > 0) {
            uint16_t gv_val = 0;
            DM2_V1_SkprojectGetGlobVarReceipt gv_receipt;
            dm2_v1_skproject_get_glob_var(
                dm2_actu_data(record),
                ctx->global_words, ctx->global_word_count,
                &gv_val, &gv_receipt);
            if (gv_val == 0)
                should_forward = 0;
        }
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

int dm2_v1_actuate_floor_mecha(DM2_V1_RecordPoolSet *pool_set,
                               DM2_V1_DungeonData *dungeon,
                               DM2_V1_CaiiArray *caii,
                               DM2_V1_SourceTimerQueue *queue,
                               int map, int x, int y,
                               int action_type, int direction,
                               uint32_t game_tick,
                               uint16_t *global_words,
                               uint16_t global_word_count,
                               DM2_V1_GetOrnateAnimLenFn get_ornate_anim_len,
                               void *ornate_ctx,
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
    ctx.caii        = caii;
    ctx.queue       = queue;
    ctx.map         = map;
    ctx.x           = x;
    ctx.y           = y;
    ctx.action_type = action_type;
    ctx.direction   = direction;
    ctx.game_tick   = game_tick;
    ctx.global_words      = global_words;
    ctx.global_word_count = global_word_count;
    ctx.creature_killer_cb = NULL;
    ctx.get_ornate_anim_len = get_ornate_anim_len;
    ctx.ornate_ctx  = ornate_ctx;
    ctx.receipt     = receipt;

    memset(&walk_receipt, 0, sizeof(walk_receipt));
    dm2_v1_tile_record_walk(pool_set, dungeon, map, x, y,
                            floor_mecha_visitor, &ctx, &walk_receipt);

    receipt->valid = 1;
    return 1;
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
    int16_t dx_abs, dy_abs, origin_x, origin_y, span_x, span_y;
    int16_t ix, iy;

    if (!receipt) return 0;
    memset(receipt, 0, sizeof(*receipt));
    if (!cb || !cb->get_creature_at || !cb->creature_type_matches) return 0;

    dx_abs = (int16_t)((actu_x - timer_x) < 0 ? -(actu_x - timer_x) : (actu_x - timer_x));
    dy_abs = (int16_t)((actu_y - timer_y) < 0 ? -(actu_y - timer_y) : (actu_y - timer_y));
    origin_x = (int16_t)(actu_x - dx_abs);
    origin_y = (int16_t)(actu_y - dy_abs);
    span_x = (int16_t)(2 * dx_abs + 1);
    span_y = (int16_t)(2 * dy_abs + 1);

    for (iy = span_y - 1; iy >= 0; iy--) {
        for (ix = span_x - 1; ix >= 0; ix--) {
            int16_t cx = (int16_t)(origin_x + ix);
            int16_t cy = (int16_t)(origin_y + iy);
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
