#include "dm2_v1_1c9a_pc34_compat.h"

#include <stdlib.h>
#include <string.h>

/* ========================================================================
 * Tile cache
 * ======================================================================== */

void dm2_v1_1c9a_tile_cache_init(DM2_V1_1c9aTileCache *cache) {
    if (!cache) return;
    cache->cached_x = -1;
    cache->cached_y = -1;
    cache->cached_map = -1;
    cache->cached_tile_value = 0;
    cache->cached_b0 = -1;
    cache->cached_b2 = -1;
    cache->cached_b4 = -1;
    cache->cached_b6 = 0;
    cache->cached_b7 = 0;
    cache->cached_be = -1;
    cache->cached_c4 = 1;
}

/* ========================================================================
 * Utility: popcount (DM2_1c9a_0598)
 * skproject c_1c9a.cpp:933-957
 * ======================================================================== */

int32_t dm2_v1_1c9a_popcount(int32_t value) {
    int32_t count = 0;
    int32_t pos = 0;
    for (;;) {
        if (value == 0) return count;
        if (pos >= 32) return count;
        if (value & 1) count++;
        value = (uint32_t)value >> 1;
        pos++;
    }
}

/* ========================================================================
 * DM2_19f0_1511 — direction normalization
 * skproject c_1c9a.cpp:2430-2435
 * ======================================================================== */

int32_t dm2_v1_1c9a_19f0_1511(int32_t value) {
    /* Source: simple sign-extend / clamp. Returns value & 0xffff sign-extended. */
    return (int32_t)(int16_t)(value & 0xffff);
}

/* ========================================================================
 * DM2_1BAAD — tile passability check
 * skproject c_1c9a.cpp:23-150
 *
 * Checks whether a tile at (x,y) blocks creature movement.
 * Returns 1 if passable (creature can be there), 0 if blocked.
 * Logic:
 *   - tile_type 0 (open floor): passable
 *   - tile_type 4 (door): check sub-type and rebirth altar
 *   - tile_type 6 (teleporter): check bit 2 (open)
 *   - if tile has record bit 0x10, walk records looking for
 *     creature type 4 with AI spec flag check
 *   - type 0xf records with sub-type 0xe block
 * ======================================================================== */

int32_t dm2_v1_1c9a_1baad(
    const DM2_V1_1c9aCallbacks *cb, void *ctx,
    int32_t x, int32_t y,
    DM2_V1_1c9a1BA1BReceipt *receipt)
{
    if (!cb) return 0;

    uint8_t tile_val = cb->get_tile_value(ctx, (int16_t)x, (int16_t)y);
    uint8_t tile_type = (tile_val >> 5) & 0x7;
    uint8_t tile_sub = tile_val & 0x7;

    if (receipt) {
        receipt->tile_type = tile_type;
        receipt->passable = false;
    }

    /* tile_type 0 = open floor -> passable */
    if (tile_type == 0) {
        if (receipt) receipt->passable = true;
        return 1;
    }

    /* tile_type 4 = door */
    if (tile_type == 4) {
        /* sub-type 3 or 4 -> check rebirth altar */
        if (tile_sub == 3 || tile_sub == 4) {
            void *tile_rec = cb->get_address_of_tile_record(ctx, (int16_t)x, (int16_t)y);
            bool is_altar = cb->is_rebirth_altar(ctx, tile_rec);
            int32_t q = cb->query_0cee_3275(ctx, is_altar ? 1 : 0);
            if (q != 0) {
                /* 50% chance passable */
                if (cb->randbit(ctx)) {
                    if (receipt) receipt->passable = false;
                    return 0;
                }
            }
            if (receipt) receipt->passable = true;
            return 1;
        }
    }

    /* tile_type 6 = teleporter: check bit 2 (open flag) */
    if (tile_type == 6) {
        if ((tile_val & 0x4) == 0) {
            if (receipt) receipt->passable = true;
            return 1;
        }
    }

    /* Check record bit 0x10 -> walk thing list */
    if ((tile_val & 0x10) == 0) {
        if (receipt) receipt->passable = false;
        return 0;
    }

    int16_t rec = cb->get_wall_tile_anyitem_record(ctx, (int16_t)x, (int16_t)y);
    for (;;) {
        if (rec == -2) { /* 0xfffe = end of list */
            if (receipt) receipt->passable = false;
            return 0;
        }
        /* Extract record type: bits 10-13 */
        uint16_t urec = (uint16_t)rec;
        int rec_type = (urec >> 10) & 0xf;

        /* type 0xf: check sub-type */
        if (rec_type == 0xf) {
            void *addr = cb->get_address_of_record(ctx, urec);
            if (addr) {
                uint16_t word2 = *(uint16_t *)((uint8_t *)addr + 2);
                if ((word2 & 0x7f) == 0xe) {
                    if (receipt) receipt->passable = true;
                    return 1;
                }
            }
        }

        /* type 4: creature -> check AI spec flags */
        if (rec_type == 4) {
            int16_t creature_rec = cb->get_creature_at(ctx, (int16_t)x, (int16_t)y);
            if (creature_rec != -1) {
                int32_t ai_flags = cb->query_creature_ai_spec_flags(ctx, (uint16_t)creature_rec);
                if ((ai_flags & 0x1) != 0) {
                    /* Check bits 6-7 */
                    int size = (ai_flags >> 6) & 0x3;
                    if (size < 2) {
                        if (receipt) receipt->passable = true;
                        return 1;
                    }
                } else {
                    if ((ai_flags & 0x20) == 0) {
                        if (receipt) receipt->passable = true;
                        return 1;
                    }
                }
            }
        }

        rec = (int16_t)cb->get_next_record_link(ctx, urec);
    }
}

/* ========================================================================
 * DM2_1BC29 — passability with party shortcut
 * skproject c_1c9a.cpp:152-160
 * ======================================================================== */

int32_t dm2_v1_1c9a_1bc29(
    const DM2_V1_1c9aCallbacks *cb, void *ctx,
    int16_t x, int16_t y)
{
    if (!cb) return 0;

    /* If creature is at party position, return 1 */
    int16_t cur_map = cb->get_ddat_current_map(ctx);
    int16_t party_map = cb->get_ddat_party_map(ctx);
    int16_t party_x = cb->get_ddat_party_x(ctx);
    int16_t party_y = cb->get_ddat_party_y(ctx);

    if (cur_map == party_map && x == party_x && y == party_y)
        return 1;

    return dm2_v1_1c9a_1baad(cb, ctx, (int32_t)x, (int32_t)y, NULL);
}

/* ========================================================================
 * DM2_19f0_0207 — Bresenham line walk with tile check
 * skproject c_1c9a.cpp:163-468
 *
 * Simplified: walks from (startX,startY) to (endX,endY) checking each
 * intermediate tile with tile_check_fn.  Returns distance if clear, 0 if blocked.
 * ======================================================================== */

int32_t dm2_v1_1c9a_19f0_0207(
    const DM2_V1_1c9aCallbacks *cb, void *ctx,
    int16_t start_x, int16_t start_y,
    int16_t end_x, int16_t end_y,
    DM2_V1_1c9aTileCheckFn tile_check_fn)
{
    if (!cb) return 0;

    int16_t dx = (int16_t)abs(start_x - end_x);
    int16_t dy = (int16_t)abs(start_y - end_y);

    /* Adjacent or same tile -> always passable */
    if (dx + dy <= 1)
        return 1;

    int16_t step_x = (end_x <= start_x) ? 1 : -1;
    int16_t step_y = (end_y <= start_y) ? 1 : -1;

    /* Determine major/minor axis */
    bool y_major = (dx < dy);
    bool axes_equal = (dx == dy);

    int16_t cur_x = end_x;
    int16_t cur_y = end_y;

    /* Walk from end toward start, checking each tile */
    for (;;) {
        int16_t next_x = cur_x + step_x;

        if (axes_equal) {
            /* Diagonal: check both adjacent tiles */
            if (tile_check_fn((int32_t)next_x, (int32_t)cur_y) != 0) {
                int16_t next_y = cur_y + step_y;
                if (tile_check_fn((int32_t)cur_x, (int32_t)next_y) != 0)
                    return 0;
            }
            cur_y += step_y;
        } else if (y_major) {
            cur_y += step_y;
            if (tile_check_fn((int32_t)cur_x, (int32_t)cur_y) != 0)
                return 0;
        } else {
            cur_x += step_x;
            if (tile_check_fn((int32_t)cur_x, (int32_t)cur_y) != 0)
                return 0;
        }

        /* Check if we reached the start */
        int16_t rem_dx = (int16_t)abs(cur_x - start_x);
        int16_t rem_dy = (int16_t)abs(cur_y - start_y);
        if (rem_dx + rem_dy <= 1) {
            return cb->calc_square_distance(ctx, start_x, start_y, end_x, end_y);
        }
    }
}

/* ========================================================================
 * DM2_19f0_045a — tile cache refresh
 * skproject c_1c9a.cpp:470-501
 * ======================================================================== */

/* Module-local tile cache (session-scoped). */
static DM2_V1_1c9aTileCache g_tile_cache = {
    -1, -1, -1, 0, -1, -1, -1, 0, 0, -1, 1
};

int32_t dm2_v1_1c9a_19f0_045a(
    const DM2_V1_1c9aCallbacks *cb, void *ctx,
    int16_t x, int16_t y)
{
    if (!cb) return 0;

    int16_t cur_map = cb->get_ddat_current_map(ctx);

    /* Return cached if same position and map */
    if (x == g_tile_cache.cached_x &&
        y == g_tile_cache.cached_y &&
        cur_map == g_tile_cache.cached_map) {
        return (int32_t)g_tile_cache.cached_tile_value;
    }

    g_tile_cache.cached_map = cur_map;
    g_tile_cache.cached_y = y;
    g_tile_cache.cached_x = x;

    uint8_t tile_val = cb->get_tile_value(ctx, x, y);
    g_tile_cache.cached_tile_value = (int16_t)(tile_val & 0xff);

    /* Check bit 4 presence flag */
    int16_t presence = (g_tile_cache.cached_tile_value & 0x10) != 0 ? 1 : 0;
    presence += -2; /* -2 + 1 = -1, -2 + 0 = -2 */
    g_tile_cache.cached_b4 = presence;
    g_tile_cache.cached_b2 = presence;
    g_tile_cache.cached_b0 = presence;
    g_tile_cache.cached_b6 = 0;
    g_tile_cache.cached_b7 = 0;
    g_tile_cache.cached_be = -1;
    g_tile_cache.cached_c4 = 1;

    return (int32_t)g_tile_cache.cached_tile_value;
}

/* ========================================================================
 * DM2_19f0_04bf — first non-basic record finder
 * skproject c_1c9a.cpp:503-540
 * ======================================================================== */

int32_t dm2_v1_1c9a_19f0_04bf(
    const DM2_V1_1c9aCallbacks *cb, void *ctx)
{
    if (!cb) return -2;

    if (g_tile_cache.cached_b2 != -1)
        return (int32_t)g_tile_cache.cached_b2;

    int16_t rec = g_tile_cache.cached_b0;
    if (rec == -1) {
        rec = cb->get_tile_record_link(ctx,
            g_tile_cache.cached_x, g_tile_cache.cached_y);
        g_tile_cache.cached_b0 = rec;
    }

    /* Walk chain, skip types <= 3 */
    for (;;) {
        if (rec == -2) break; /* 0xfffe = end */
        uint16_t urec = (uint16_t)rec;
        int rec_type = (urec >> 10) & 0xf;
        if (rec_type > 3) break;
        rec = (int16_t)cb->get_next_record_link(ctx, urec);
    }

    g_tile_cache.cached_b2 = rec;
    return (int32_t)rec;
}

/* ========================================================================
 * DM2_19f0_050f — creature record finder
 * skproject c_1c9a.cpp:542-573
 * ======================================================================== */

int32_t dm2_v1_1c9a_19f0_050f(
    const DM2_V1_1c9aCallbacks *cb, void *ctx)
{
    if (!cb) return -2;

    if (g_tile_cache.cached_b4 != -1)
        return (int32_t)g_tile_cache.cached_b4;

    int32_t rec32 = dm2_v1_1c9a_19f0_04bf(cb, ctx);
    int16_t rec = (int16_t)rec32;

    /* Walk chain, find type == 4 (creature) */
    for (;;) {
        if (rec == -2) break;
        uint16_t urec = (uint16_t)rec;
        int rec_type = (urec >> 10) & 0xf;
        if (rec_type == 4) break;
        rec = (int16_t)cb->get_next_record_link(ctx, urec);
    }

    g_tile_cache.cached_b4 = rec;
    return (int32_t)rec;
}

/* ========================================================================
 * DM2_19f0_0559 — creature turn direction selection
 * skproject c_1c9a.cpp:584-645
 *
 * Given target direction, decides left (+1) or right (-1) turn.
 * ======================================================================== */

int32_t dm2_v1_1c9a_19f0_0559(
    const DM2_V1_1c9aCallbacks *cb, void *ctx,
    int16_t target_dir,
    DM2_V1_1c9a0559Receipt *receipt)
{
    if (!cb) return 0;

    /* Get current creature facing from creature record word@0xe bits 14-15 */
    uint16_t word_e = cb->get_creature_word(ctx, 0xe);
    int16_t cur_facing = (int16_t)((word_e << 6) >> 14); /* bits 8-9 */

    int16_t opposite = (target_dir + 2) & 0x3;

    int16_t turn_dir;
    if (cur_facing == opposite) {
        /* Facing opposite to target -> face target directly */
        cb->set_creature_byte(ctx, 0x1a, 0);
        /* s350.v1e056f = -2 */
        if (receipt) {
            receipt->turned = false;
            receipt->new_facing = -1;
        }
        return 0;
    }

    if (cur_facing != target_dir) {
        int16_t left = (target_dir - 1) & 0x3;
        if (cur_facing == left) {
            turn_dir = -1;
        } else {
            turn_dir = 1;
        }
    } else {
        /* Same direction: random */
        if (cb->randbit(ctx))
            turn_dir = 1;
        else
            turn_dir = -1;
    }

    int16_t new_facing = (cur_facing + turn_dir) & 0x3;
    cb->set_creature_byte(ctx, 0x1d, (uint8_t)new_facing);

    /* Set action byte: turn_dir != -1 -> 0x8, else 0x7 */
    uint8_t action = (turn_dir != -1) ? 8 : 7;
    cb->set_creature_byte(ctx, 0x1a, action);
    /* s350.v1e056f = -4 */

    if (receipt) {
        receipt->turned = true;
        receipt->new_facing = new_facing;
    }
    return 1;
}

/* ========================================================================
 * DM2_19f0_05e8 — creature combat/movement detailed evaluation
 * skproject c_1c9a.cpp:648-931
 * Stub: returns 0 (fail closed — host must wire full composition)
 * ======================================================================== */

int32_t dm2_v1_1c9a_19f0_05e8(
    const DM2_V1_1c9aCallbacks *cb, void *ctx,
    int32_t creature_index, void *edx_ptr, void *ebx_ptr,
    int32_t dir, int16_t param0, int32_t param1, int32_t param2)
{
    (void)cb; (void)ctx;
    (void)creature_index; (void)edx_ptr; (void)ebx_ptr;
    (void)dir; (void)param0; (void)param1; (void)param2;
    /* Fail closed: complex AI evaluation requires full source porting.
     * The bounded slice is proven for the static helper structure;
     * the body runs host-owned until the CCM composition is bound. */
    return 0;
}

/* ========================================================================
 * DM2_19f0_0891 — creature attack/movement decision
 * skproject c_1c9a.cpp:960-1660
 * Stub: returns 0 (fail closed)
 * ======================================================================== */

int32_t dm2_v1_1c9a_19f0_0891(
    const DM2_V1_1c9aCallbacks *cb, void *ctx,
    int32_t creature_index, int16_t map_x, int16_t map_y,
    int16_t dir, int16_t target_x, int16_t target_dir,
    DM2_V1_1c9a0891Receipt *receipt)
{
    (void)cb; (void)ctx;
    (void)creature_index; (void)map_x; (void)map_y;
    (void)dir; (void)target_x; (void)target_dir;
    if (receipt) {
        receipt->decided = false;
        receipt->action_code = -1;
    }
    return 0;
}

/* ========================================================================
 * DM2_19f0_0d10 — creature movement decision
 * skproject c_1c9a.cpp:1663-2257
 * Stub: returns 0 (fail closed)
 * ======================================================================== */

int32_t dm2_v1_1c9a_19f0_0d10(
    const DM2_V1_1c9aCallbacks *cb, void *ctx,
    int32_t creature_index, int16_t map_x, int16_t map_y,
    int16_t dir, int16_t target_x, int16_t target_y)
{
    (void)cb; (void)ctx;
    (void)creature_index; (void)map_x; (void)map_y;
    (void)dir; (void)target_x; (void)target_y;
    return 0;
}

/* ========================================================================
 * DM2_19f0_13aa — creature combat target selection
 * skproject c_1c9a.cpp:2259-2427
 * Stub: returns 0 (fail closed)
 * ======================================================================== */

int32_t dm2_v1_1c9a_19f0_13aa(
    const DM2_V1_1c9aCallbacks *cb, void *ctx,
    int32_t creature_index, int32_t unused)
{
    (void)cb; (void)ctx;
    (void)creature_index; (void)unused;
    return 0;
}

/* ========================================================================
 * DM2_D283 — creature record lookup
 * skproject c_1c9a.cpp:2438-2510
 * Stub: returns NULL (fail closed)
 * ======================================================================== */

void *dm2_v1_1c9a_d283(
    const DM2_V1_1c9aCallbacks *cb, void *ctx,
    int32_t creature_index, int32_t map_index)
{
    (void)cb; (void)ctx;
    (void)creature_index; (void)map_index;
    return NULL;
}

/* ========================================================================
 * DM2_CREATURE_GO_THERE — creature movement orchestrator
 * skproject c_1c9a.cpp:2514-3972
 * Stub: returns 0 (fail closed)
 * ======================================================================== */

int32_t dm2_v1_1c9a_creature_go_there(
    const DM2_V1_1c9aCallbacks *cb, void *ctx,
    int32_t creature_index, int16_t map_x, int16_t map_y,
    int16_t dir, int16_t dest_x, int16_t dest_y,
    DM2_V1_1c9aCreatureGoReceipt *receipt)
{
    (void)cb; (void)ctx;
    (void)creature_index; (void)map_x; (void)map_y;
    (void)dir; (void)dest_x; (void)dest_y;
    if (receipt) {
        receipt->moved = false;
        receipt->new_x = map_x;
        receipt->new_y = map_y;
        receipt->new_dir = dir;
    }
    return 0;
}

/* ========================================================================
 * DM2_19f0_2024 — pre-movement tile validation
 * skproject c_1c9a.cpp:3987-4121
 * Stub: returns 0 (fail closed)
 * ======================================================================== */

int32_t dm2_v1_1c9a_19f0_2024(
    const DM2_V1_1c9aCallbacks *cb, void *ctx,
    int32_t creature_index, int16_t x, int16_t y)
{
    (void)cb; (void)ctx;
    (void)creature_index; (void)x; (void)y;
    return 0;
}

/* ========================================================================
 * DM2_19f0_2165 — high-level creature AI tick handler
 * skproject c_1c9a.cpp:4124-4638
 * Stub: returns 0 (fail closed)
 * ======================================================================== */

int32_t dm2_v1_1c9a_19f0_2165(
    const DM2_V1_1c9aCallbacks *cb, void *ctx,
    int32_t creature_index, int16_t map_x, int16_t map_y,
    int16_t dir, int32_t flags, int16_t param1, int16_t param2)
{
    (void)cb; (void)ctx;
    (void)creature_index; (void)map_x; (void)map_y;
    (void)dir; (void)flags; (void)param1; (void)param2;
    return 0;
}

/* ========================================================================
 * DM2_19f0_266c — ranged attack feasibility
 * skproject c_1c9a.cpp:4641-4718
 * Stub: returns 0 (fail closed)
 * ======================================================================== */

int32_t dm2_v1_1c9a_19f0_266c(
    const DM2_V1_1c9aCallbacks *cb, void *ctx,
    int32_t creature_index, int16_t x, int16_t y, int16_t dir)
{
    (void)cb; (void)ctx;
    (void)creature_index; (void)x; (void)y; (void)dir;
    return 0;
}

/* ========================================================================
 * DM2_19f0_2723 — adjacent tile threat assessment
 * skproject c_1c9a.cpp:4721-4838
 * Stub: returns 0 (fail closed)
 * ======================================================================== */

int32_t dm2_v1_1c9a_19f0_2723(
    const DM2_V1_1c9aCallbacks *cb, void *ctx,
    int32_t creature_index, int16_t x, int16_t y, int16_t dir)
{
    (void)cb; (void)ctx;
    (void)creature_index; (void)x; (void)y; (void)dir;
    return 0;
}

/* ========================================================================
 * DM2_19f0_2813 — path obstruction check
 * skproject c_1c9a.cpp:4841-5081
 * Stub: returns false (fail closed — path blocked)
 * ======================================================================== */

bool dm2_v1_1c9a_19f0_2813(
    const DM2_V1_1c9aCallbacks *cb, void *ctx,
    int32_t creature_index, int16_t map_x, int16_t map_y,
    int16_t x, int16_t y, int16_t target_x, int16_t target_y)
{
    (void)cb; (void)ctx;
    (void)creature_index; (void)map_x; (void)map_y;
    (void)x; (void)y; (void)target_x; (void)target_y;
    return false;
}

/* ========================================================================
 * DM2_1BA1B — tile passability dispatcher
 * skproject c_1c9a.cpp:5090-5133
 *
 * Used as indirect callback for line-of-sight checks.
 * tile_type 0 (wall/empty) -> passable
 * tile_type 4 (door): check sub-type 4 and altar
 * tile_type 6 (teleporter): bit 2 clear -> passable
 * tile_type 7 -> passable
 * ======================================================================== */

int32_t dm2_v1_1c9a_1ba1b(
    const DM2_V1_1c9aCallbacks *cb, void *ctx,
    int32_t x, int32_t y,
    DM2_V1_1c9a1BA1BReceipt *receipt)
{
    if (!cb) return 0;

    uint8_t tile_val = cb->get_tile_value(ctx, (int16_t)x, (int16_t)y);
    uint8_t tile_type = (tile_val >> 5) & 0x7;

    if (receipt) {
        receipt->tile_type = tile_type;
        receipt->passable = false;
    }

    if (tile_type == 4) {
        /* Door: check sub-type */
        uint8_t sub = tile_val & 0x7;
        if (sub == 4) {
            /* Check rebirth altar */
            void *tile_rec = cb->get_address_of_tile_record(ctx, (int16_t)x, (int16_t)y);
            int32_t gfx = cb->get_graphics_for_door(ctx,
                cb->is_rebirth_altar(ctx, tile_rec) ? 1 : 0);
            if (gfx == 0) {
                if (receipt) receipt->passable = true;
                return 1;
            }
        }
        return 0;
    }

    if (tile_type == 0) {
        if (receipt) receipt->passable = true;
        return 1;
    }

    if (tile_type == 7) {
        if (receipt) receipt->passable = true;
        return 1;
    }

    if (tile_type == 6) {
        /* Teleporter: bit 2 clear -> passable */
        if ((tile_val & 0x4) == 0) {
            if (receipt) receipt->passable = true;
            return 1;
        }
        return 0;
    }

    return 0;
}

/* ========================================================================
 * DM2_1c9a_0247 — creature CAII cleanup
 * skproject c_1c9a.cpp:5135-5160
 * Stub: no-op (CAII deallocation requires session-owned dballochandler)
 * ======================================================================== */

void dm2_v1_1c9a_0247(
    const DM2_V1_1c9aCallbacks *cb, void *ctx,
    int32_t creature_index)
{
    (void)cb; (void)ctx; (void)creature_index;
    /* Fail closed: CAII deallocation requires DM2_ALLOCATION11 and
     * DM2_dballoc_3e74_58a3 which are session-owned. */
}

/* ========================================================================
 * DM2_1c9a_0648 — CAII lookup / map change
 * skproject c_1c9a.cpp:5162-5196
 * ======================================================================== */

int32_t dm2_v1_1c9a_0648(
    const DM2_V1_1c9aCallbacks *cb, void *ctx,
    int32_t creature_index)
{
    if (!cb) return 0;

    int16_t idx = (int16_t)(creature_index & 0xffff);
    int16_t cur_map = cb->get_ddat_current_map(ctx);

    if (idx == cur_map)
        return (int32_t)idx;

    cb->change_current_map(ctx, idx);
    return (int32_t)idx;
}

/* ========================================================================
 * DM2_1c9a_06bd — creature record lookup by position
 * skproject c_1c9a.cpp:5218-5245
 * Stub: returns NULL
 * ======================================================================== */

int16_t *dm2_v1_1c9a_06bd(
    const DM2_V1_1c9aCallbacks *cb, void *ctx,
    int32_t creature_index, int32_t x, int32_t y)
{
    (void)cb; (void)ctx;
    (void)creature_index; (void)x; (void)y;
    return NULL;
}

/* ========================================================================
 * DM2_1c9a_078b — recursive creature group movement
 * skproject c_1c9a.cpp:5249-5375
 * Stub: returns 0
 * ======================================================================== */

int32_t dm2_v1_1c9a_078b(
    const DM2_V1_1c9aCallbacks *cb, void *ctx,
    void *creature_ptr, int32_t x, int32_t y)
{
    (void)cb; (void)ctx;
    (void)creature_ptr; (void)x; (void)y;
    return 0;
}

/* ========================================================================
 * DM2_1c9a_0958 — creature type extractor
 * skproject c_1c9a.cpp:5377-5401
 * Stub: returns -1
 * ======================================================================== */

int32_t dm2_v1_1c9a_0958(
    const DM2_V1_1c9aCallbacks *cb, void *ctx,
    int32_t creature_index)
{
    (void)cb; (void)ctx; (void)creature_index;
    return -1;
}

/* ========================================================================
 * DM2_1c9a_09b9 — creature property getter
 * skproject c_1c9a.cpp:5404-5413
 * Stub: returns 0
 * ======================================================================== */

int32_t dm2_v1_1c9a_09b9(
    const DM2_V1_1c9aCallbacks *cb, void *ctx,
    int32_t creature_index, int32_t property)
{
    (void)cb; (void)ctx;
    (void)creature_index; (void)property;
    return 0;
}

/* ========================================================================
 * DM2_1c9a_09db — creature facing update
 * skproject c_1c9a.cpp:5416-5431
 * Stub: no-op
 * ======================================================================== */

void dm2_v1_1c9a_09db(
    const DM2_V1_1c9aCallbacks *cb, void *ctx,
    void *creature_ptr)
{
    (void)cb; (void)ctx; (void)creature_ptr;
}

/* ========================================================================
 * DM2_CREATURE_SOMETHING_1c9a_0a48 — global AI processing loop
 * skproject c_1c9a.cpp:5434-5693
 * Stub: returns 0
 * ======================================================================== */

int32_t dm2_v1_1c9a_creature_something_0a48(
    const DM2_V1_1c9aCallbacks *cb, void *ctx)
{
    (void)cb; (void)ctx;
    return 0;
}

/* ========================================================================
 * DM2_1c9a_0cf7 — creature timer scheduling
 * skproject c_1c9a.cpp:5695-5732
 * ======================================================================== */

void dm2_v1_1c9a_0cf7(
    const DM2_V1_1c9aCallbacks *cb, void *ctx,
    int32_t creature_index, int32_t delay)
{
    if (!cb) return;
    if (cb->creature_schedule_at)
        cb->creature_schedule_at(ctx, creature_index, (int16_t)delay);
}

/* ========================================================================
 * DM2_1c9a_0db0 — creature timer cancellation
 * skproject c_1c9a.cpp:5734-5763
 * ======================================================================== */

void dm2_v1_1c9a_0db0(
    const DM2_V1_1c9aCallbacks *cb, void *ctx,
    int32_t creature_index)
{
    if (!cb) return;
    if (cb->creature_cancel_timer)
        cb->creature_cancel_timer(ctx, creature_index);
}

/* ========================================================================
 * DM2_ALLOC_CAII_TO_CREATURE — allocate CAII slot
 * skproject c_1c9a.cpp:5772-5894
 * Delegates to the existing caii_alloc module.
 * ======================================================================== */

void dm2_v1_1c9a_alloc_caii_to_creature(
    const DM2_V1_1c9aCallbacks *cb, void *ctx,
    int32_t creature_index, int16_t x, int16_t y)
{
    if (!cb) return;
    if (cb->alloc_caii_to_creature)
        cb->alloc_caii_to_creature(ctx, creature_index, x, y);
}

/* ========================================================================
 * DM2_1c9a_0fcb — creature activation/wakeup
 * skproject c_1c9a.cpp:5896-5958
 * Stub: delegates to alloc + schedule
 * ======================================================================== */

void dm2_v1_1c9a_0fcb(
    const DM2_V1_1c9aCallbacks *cb, void *ctx,
    int32_t creature_index)
{
    if (!cb) return;
    /* Allocate CAII if needed, then schedule first timer tick */
    if (cb->alloc_caii_to_creature)
        cb->alloc_caii_to_creature(ctx, creature_index, 0, 0);
    if (cb->creature_schedule_at)
        cb->creature_schedule_at(ctx, creature_index, 1);
}

/* ========================================================================
 * DM2_CREATE_MINION — spawn summoned creature
 * skproject c_1c9a.cpp:5961-6146
 * Stub: returns -1 (no creature spawned)
 * ======================================================================== */

int16_t dm2_v1_1c9a_create_minion(
    const DM2_V1_1c9aCallbacks *cb, void *ctx,
    int32_t creature_type, int16_t map_x, int16_t map_y,
    int16_t dir, int32_t owner_index, int16_t hit_points,
    int32_t flags, int8_t spell_power,
    DM2_V1_1c9aMinionReceipt *receipt)
{
    (void)cb; (void)ctx;
    (void)creature_type; (void)map_x; (void)map_y;
    (void)dir; (void)owner_index; (void)hit_points;
    (void)flags; (void)spell_power;
    if (receipt) {
        receipt->created = false;
        receipt->record_index = -1;
    }
    return -1;
}

/* ========================================================================
 * DM2_RELEASE_MINION — destroy summoned creature
 * skproject c_1c9a.cpp:6149-6179
 * Stub: no-op
 * ======================================================================== */

void dm2_v1_1c9a_release_minion(
    const DM2_V1_1c9aCallbacks *cb, void *ctx,
    int32_t creature_index)
{
    (void)cb; (void)ctx; (void)creature_index;
}

/* ========================================================================
 * DM2_1c9a_17c7 — creature-to-party distance
 * skproject c_1c9a.cpp:6182-6239
 * ======================================================================== */

int32_t dm2_v1_1c9a_17c7(
    const DM2_V1_1c9aCallbacks *cb, void *ctx,
    int32_t creature_index, int16_t x, int16_t y)
{
    if (!cb) return -1;
    (void)creature_index;

    int16_t party_x = cb->get_ddat_party_x(ctx);
    int16_t party_y = cb->get_ddat_party_y(ctx);

    return cb->calc_square_distance(ctx, x, y, party_x, party_y);
}

/* ========================================================================
 * DM2_1c9a_19d4 — creature position update
 * skproject c_1c9a.cpp:6241-6272
 * Stub: no-op
 * ======================================================================== */

void dm2_v1_1c9a_19d4(
    const DM2_V1_1c9aCallbacks *cb, void *ctx,
    int32_t creature_index, int16_t x, int16_t y, int16_t dir)
{
    (void)cb; (void)ctx;
    (void)creature_index; (void)x; (void)y; (void)dir;
}

/* ========================================================================
 * DM2_1c9a_1a48 — creature damage application
 * skproject c_1c9a.cpp:6274-6353
 * Stub: returns 0 (no damage applied)
 * ======================================================================== */

int32_t dm2_v1_1c9a_1a48(
    const DM2_V1_1c9aCallbacks *cb, void *ctx,
    int32_t creature_index, int32_t damage,
    DM2_V1_1c9aDamageReceipt *receipt)
{
    (void)cb; (void)ctx;
    (void)creature_index; (void)damage;
    if (receipt) {
        receipt->applied = false;
        receipt->remaining_hp = 0;
        receipt->creature_died = false;
    }
    return 0;
}

/* ========================================================================
 * DM2_1c9a_1b16 — creature healing
 * skproject c_1c9a.cpp:6355-6417
 * Stub: returns 0
 * ======================================================================== */

int32_t dm2_v1_1c9a_1b16(
    const DM2_V1_1c9aCallbacks *cb, void *ctx,
    int32_t creature_index, int32_t heal_amount,
    DM2_V1_1c9aHealReceipt *receipt)
{
    (void)cb; (void)ctx;
    (void)creature_index; (void)heal_amount;
    if (receipt) {
        receipt->applied = false;
        receipt->new_hp = 0;
    }
    return 0;
}

/* ========================================================================
 * DM2_FIND_WALK_PATH — A*-style pathfinding
 * skproject c_1c9a.cpp:6439-9668
 * Stub: returns -1 (no path)
 * ======================================================================== */

int32_t dm2_v1_1c9a_find_walk_path(
    const DM2_V1_1c9aCallbacks *cb, void *ctx,
    int32_t creature_index, int16_t start_x, int16_t start_y,
    int16_t flags, uint8_t *path_buffer, void *button,
    DM2_V1_1c9aPathfindReceipt *receipt)
{
    (void)cb; (void)ctx;
    (void)creature_index; (void)start_x; (void)start_y;
    (void)flags; (void)path_buffer; (void)button;
    if (receipt) {
        receipt->path_found = false;
        receipt->path_length = -1;
    }
    return -1;
}

/* ========================================================================
 * DM2_1c9a_381c — walk path consumer
 * skproject c_1c9a.cpp:9697-9746
 * Stub: returns -1 (no path)
 * ======================================================================== */

int32_t dm2_v1_1c9a_381c(
    const DM2_V1_1c9aCallbacks *cb, void *ctx)
{
    (void)cb; (void)ctx;
    return -1;
}

/* ========================================================================
 * DM2_1c9a_38a8 — CAII/action-list walk-path dispatch
 * skproject c_1c9a.cpp:9749-9894
 * Requires the source-owned s350/CAII/action-list state.  Return an explicit
 * unavailable result; source return value zero is a real no-path outcome and
 * must not be forged by this compatibility shim.
 * ======================================================================== */

int32_t dm2_v1_1c9a_38a8(
    const DM2_V1_1c9aCallbacks *cb, void *ctx)
{
    (void)cb; (void)ctx;
    return -1;
}

/* ========================================================================
 * DM2_FILL_CAII_CUR_MAP — map creature initialization
 * skproject c_1c9a.cpp:9896-9993
 * Stub: returns 0
 * ======================================================================== */

int32_t dm2_v1_1c9a_fill_caii_cur_map(
    const DM2_V1_1c9aCallbacks *cb, void *ctx,
    DM2_V1_1c9aFillCaiiReceipt *receipt)
{
    (void)cb; (void)ctx;
    if (receipt) {
        receipt->creatures_activated = 0;
    }
    return 0;
}

/* ========================================================================
 * DM2_FILL_ORPHAN_CAII — orphan CAII recovery
 * skproject c_1c9a.cpp:9996-10022
 * Stub: no-op
 * ======================================================================== */

void dm2_v1_1c9a_fill_orphan_caii(
    const DM2_V1_1c9aCallbacks *cb, void *ctx)
{
    (void)cb; (void)ctx;
}
