#ifndef FIRESTAFF_DM2_V1_CCM_ADVANCED_PC34_COMPAT_H
#define FIRESTAFF_DM2_V1_CCM_ADVANCED_PC34_COMPAT_H

/*
 * dm2_v1_ccm_advanced_pc34_compat.h — algorithmic parity ports of six
 * skproject CCM (Creature Command Machine) handler bodies that
 * dm2_v1_ccm.c's dispatcher previously only receipted as flag writes:
 *
 *   DM2_CREATURE_CCM03            c_creature.cpp:1609
 *   DM2_CREATURE_JUMPS            c_creature.cpp:1636
 *   DM2_CREATURE_TAKES_ITEM       c_creature.cpp:2176
 *   DM2_CREATURE_PUTS_DOWN_ITEM   c_creature.cpp:2284
 *   DM2_CREATURE_TRANSFORM        c_creature.cpp:2637
 *   DM2_CREATURE_EXPLODE_OR_SUMMON c_creature.cpp:2762
 *
 * skproject's register emulation (c_nreg/RG*, DOWNCAST, s350/ddat global
 * structs) addresses the live creature record and session globals by raw
 * byte offset.  This port keeps the same branch structure and operand
 * order but replaces every external read/write and every sub-routine
 * call (DM2_CREATURE_WALK_NOW, DM2_MOVE_RECORD_TO, DM2_CREATE_CLOUD,
 * DM2_CREATE_MINION, DM2_QUEUE_NOISE_GEN1/2, DM2_CREATURE_GO_THERE,
 * record-chain walk/cut/append, ...) with an explicit callback so the
 * host (dm2_v1_creature.c or the CCM dispatcher) supplies the concrete
 * record/session binding.  Nothing here fabricates state: every callback
 * is mandatory (NULL callback fails closed, receipt->valid stays 0).
 *
 * Direction delta tables (table1d27fc/table1d2804, c_moverec.cpp:77-79
 * and callers) are the standard DM engine 4-direction offset pair
 * (N/E/S/W); they are reproduced verbatim as dm2_v1_ccm_dir_dx/dy.
 *
 * V1 invariant preserved: these functions mutate only the caller-owned
 * state reached through the callback struct; they never touch party
 * HP/mana/food/water/direction/position directly.
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Direction deltas — table1d27fc (dx) / table1d2804 (dy), 4 directions
 * (0=N,1=E,2=S,3=W), c_moverec.cpp:77-79. */
extern const int16_t dm2_v1_ccm_dir_dx[4];
extern const int16_t dm2_v1_ccm_dir_dy[4];

/* ── Shared record accessors ───────────────────────────────────────
 * Every handler below reads/writes the live creature record through
 * this small set of accessors, matching the source's DOWNCAST(c_creature,
 * s350.creatures) + byte_at(record, offset) idiom. */
typedef struct {
    void *ctx;

    /* byte@0x1f — per-opcode phase counter ("b_1f"). */
    uint8_t (*get_phase)(void *ctx);
    void    (*set_phase)(void *ctx, uint8_t phase);

    /* byte@0x1a — creature command state (b_1a). */
    uint8_t (*get_state)(void *ctx);
    void    (*set_state)(void *ctx, uint8_t state);

    /* byte@0x1e — creature type / GDAT cls index. */
    uint8_t (*get_creature_type)(void *ctx);

    /* byte@0x1c — creature "level"/kind used as an item-class filter
     * (0xff = no filter). */
    uint8_t (*get_item_class_filter)(void *ctx);

    /* byte@0x1b — facing direction (0..3). */
    uint8_t (*get_facing)(void *ctx);
    void    (*set_facing)(void *ctx, uint8_t facing);

    /* word@0x18 packed rotation/position word: bits[4:0] hold the
     * source's 0..0x1f positional index; bits above 5 hold a rotation
     * phase.  Exposed as raw get/set so JUMPS can reproduce the
     * source's shift/mask sequence exactly. */
    uint16_t (*get_pos_word)(void *ctx);
    void     (*set_pos_word)(void *ctx, uint16_t value);

    /* s350.v1e0562 — creature's live map coordinate. */
    void (*get_xy)(void *ctx, int16_t *out_x, int16_t *out_y);
    void (*set_xy)(void *ctx, int16_t x, int16_t y);

    /* s350.v1e054c / v1e054e / v1e0571 — current map id / possession
     * chain head / destination level. */
    int32_t (*get_current_map)(void *ctx);
    uint16_t (*get_possession_head)(void *ctx);
    uint8_t (*get_dest_level)(void *ctx);

    /* RNG: DM2_RAND()/DM2_RAND16(n)/DM2_RANDDIR(). */
    int32_t (*rand_raw)(void *ctx);
    int16_t (*rand16)(void *ctx, int16_t upper_exclusive);
    int16_t (*randdir)(void *ctx);

    /* s350.v1e0552 AI spec byte@0x6 — power/hp field feeding the
     * EXPLODE_OR_SUMMON self-destruct cloud magnitude roll and the
     * TRANSFORM/EXPLODE_OR_SUMMON rebind's rand16 spread. */
    uint16_t (*get_ai_spec_power)(void *ctx);

    /* DM2_CREATURE_WALK_NOW (c_creature.cpp:1774) — returns the source
     * i32 result (0 = moved, nonzero = blocked/pending). */
    int32_t (*walk_now)(void *ctx);

    /* DM2_CREATURE_CCM06 (c_creature.cpp:1572) — random small facing
     * jitter; no return value in the source. */
    void (*ccm06)(void *ctx);

    /* DM2_CREATURE_GO_THERE / DM2_MOVE_RECORD_TO — 1 on success,
     * matching the source's nonzero-success convention used by JUMPS'
     * final phase and TAKES_ITEM/PUTS_DOWN_ITEM. */
    int (*move_record_to)(void *ctx, int16_t record, int16_t from_x,
                          int16_t from_y, int16_t dest_x, int16_t dest_y,
                          int16_t dest_dir);

    /* DM2_1c9a_0648 (c_1c9a.cpp:5161) — transition-cache poke run after
     * JUMPS lands the party on a teleporter-adjusted tile. */
    void (*transition_cache_poke)(void *ctx, uint8_t dest_level);

    /* DM2_CREATE_CLOUD (visual/damage cloud spawn). */
    void (*create_cloud)(void *ctx, int16_t sprite, int16_t param,
                         int16_t x, int16_t y, int16_t dir_or_flag);

    /* DM2_CREATE_MINION — returns the new record word, or -1 fail. */
    int16_t (*create_minion)(void *ctx, uint8_t creature_type,
                             uint8_t summon_kind, int16_t facing,
                             int16_t x, int16_t y, int32_t level,
                             uint16_t unused_ffff, uint8_t owner_flag);

    /* DM2_IS_CREATURE_ALLOWED_ON_LEVEL. */
    int (*is_creature_allowed_on_level)(void *ctx, int32_t map, uint8_t level);

    /* DM2_DELETE_CREATURE_RECORD — self-deletion fallback when a
     * transform/summon lands the creature on a disallowed level. */
    void (*delete_creature_record)(void *ctx, int16_t x, int16_t y);

    /* DM2_CREATURE_KILL_ON_TIMER_POSITION — companion opcode reused by
     * EXPLODE_OR_SUMMON's self-destruct branch when byte@0x1e != 0. */
    void (*kill_on_timer_position)(void *ctx);

    /* Item record-chain primitives for TAKES_ITEM / PUTS_DOWN_ITEM. */
    uint16_t (*get_wall_tile_anyitem_record)(void *ctx, int16_t x, int16_t y);
    uint16_t (*get_next_record_link)(void *ctx, uint16_t record);
    uint8_t  (*get_record_db_class)(void *ctx, uint16_t record); /* bits 10-13 */
    int      (*creature_can_handle_it)(void *ctx, uint16_t record,
                                       uint8_t creature_type);
    uint16_t (*can_handle_item_in)(void *ctx, uint8_t creature_type,
                                   uint16_t first_record, uint8_t dir_filter);
    void     (*append_record_to)(void *ctx, uint16_t record,
                                 uint16_t *chain_head);
    void     (*cut_record_from)(void *ctx, uint16_t record,
                                uint16_t *chain_head);
    uint8_t  (*query_cls1_from_record)(void *ctx, uint16_t record);
    uint8_t  (*query_cls2_from_record)(void *ctx, uint16_t record);

    /* Sound: DM2_QUEUE_NOISE_GEN1/2.  Bound directly onto
     * dm2_v1_sound_queue_noise_gen1/2's parameter order; ctx wraps the
     * caller's DM2_V1_SoundQueueState + env. */
    void (*queue_noise_gen1)(void *ctx, int8_t cls1, int8_t cls2,
                             int8_t cls3, int16_t ecxw, int16_t volume,
                             int16_t x, int16_t y, int16_t delay_mode);
    void (*queue_noise_gen2)(void *ctx, int8_t cls1, int8_t cls2,
                             int8_t cls3, int8_t cls_alt, int16_t x,
                             int16_t y, int16_t ecxw, int16_t volume,
                             int16_t delay_mode);

    /* v350.v1e0570 — "needs redraw / party moved" host flag. */
    void (*mark_needs_redraw)(void *ctx);
} DM2_V1_CCMAdvancedCallbacks;

/* Generic receipt: 1 == the handler body ran to completion against a
 * fully-populated callback set; 0 == fail-closed (missing callback or
 * bad argument), matching this codebase's receipt convention. */
typedef struct {
    int valid;
    int32_t result; /* the source i32 return value where the source has one */
} DM2_V1_CCMAdvancedReceipt;

/* DM2_CREATURE_CCM03 (c_creature.cpp:1609).
 * phase 0: run CCM06, result 0. phase 1: result = WALK_NOW().
 * Any other phase keeps the source's uninitialised-then-0 fallthrough.
 * byte@0x1f is incremented unconditionally on every call. */
int dm2_v1_ccm_advanced_ccm03(const DM2_V1_CCMAdvancedCallbacks *cb,
                              DM2_V1_CCMAdvancedReceipt *out);

/* DM2_CREATURE_JUMPS (c_creature.cpp:1636).
 * phase 0: call WALK_NOW (preserving the "next AI dir" byte around it,
 *   which the source explicitly stashes/restores), apply the
 *   dir-delta rotation to the packed 0x18 word, advance to phase 1.
 * phase 1: call WALK_NOW; advance to phase 2 on success else phase 3.
 * phase 2: advance to phase 3, MOVE_RECORD_TO the party onto the
 *   creature's current tile, poke the transition cache, return 0.
 * phase >=3: no-op, return 0. */
int dm2_v1_ccm_advanced_jumps(const DM2_V1_CCMAdvancedCallbacks *cb,
                              DM2_V1_CCMAdvancedReceipt *out);

/* DM2_CREATURE_TAKES_ITEM (c_creature.cpp:2176).
 * Walk the party tile's item chain (GET_WALL_TILE_ANYITEM_RECORD /
 * GET_NEXT_RECORD_LINK).  Skip DB-class 4 (creature) records outright;
 * for others, skip when the creature's item-class filter (byte@0x1c,
 * 0xff = any) doesn't match the record's DB class.  The first surviving
 * record the creature CAN_HANDLE_IT is moved onto the creature (fail
 * with 0xffffffff dest per source) and appended to its possession
 * chain; the loop then stops unless the creature's "keep picking up"
 * flag is clear (source byte@0x1e & 0x40).  Returns 1 if nothing was
 * ever taken, 0 otherwise (matching the source's inverted counter
 * check). */
int dm2_v1_ccm_advanced_takes_item(const DM2_V1_CCMAdvancedCallbacks *cb,
                                   DM2_V1_CCMAdvancedReceipt *out);

/* DM2_CREATURE_PUTS_DOWN_ITEM (c_creature.cpp:2284).
 * Repeatedly find an item in the creature's possession chain the
 * creature CAN_HANDLE (CAN_HANDLE_ITEM_IN, no direction filter), cut it
 * from the chain, tag its DB-class field with the creature's item-class
 * filter, and MOVE_RECORD_TO the party's tile.  The first item dropped
 * in a call queues a "thud" noise (QUEUE_NOISE_GEN2).  The loop
 * continues while the creature's "keep dropping" flag (byte@0x1e &
 * 0x40) is set; each iteration is bounded to avoid runaway chains. */
int dm2_v1_ccm_advanced_puts_down_item(const DM2_V1_CCMAdvancedCallbacks *cb,
                                       DM2_V1_CCMAdvancedReceipt *out);

/* DM2_CREATURE_TRANSFORM (c_creature.cpp:2637).
 * byte@0x1a != 0x3b (i.e. opcode 0x3c, "already transforming"): spawn a
 * single damage cloud and queue a transform noise.
 * byte@0x1a == 0x3b (opcode 0x3b, "start transforming"): spawn
 * RANDDIR()+1 damage clouds around the creature, plus a phase-scaled
 * damage cloud once byte@0x1f (the pre-increment phase) >= 1; once the
 * phase reaches 3 the creature is switched to state 0x3c, its phase is
 * reset, its AI spec/animation are rebound to its current creature
 * type, and — if the resulting creature type is not allowed on the
 * destination level — the creature record is deleted.  byte@0x1f is
 * incremented unconditionally (matching the source, which reads the
 * pre-increment value into vb_04 before bumping it). */
int dm2_v1_ccm_advanced_transform(const DM2_V1_CCMAdvancedCallbacks *cb,
                                  DM2_V1_CCMAdvancedReceipt *out);

/* DM2_CREATURE_EXPLODE_OR_SUMMON (c_creature.cpp:2762).
 * byte@0x20 selects the sub-mode (mirrors opcode 0x3d/0x3e/0x3f/0x40
 * routing through the same three branches):
 *   0: self-destruct — spawn a randomized damage cloud, then run
 *      KILL_ON_TIMER_POSITION if byte@0x1e != 0. Returns 0.
 *   1: summon — DM2_CREATE_MINION at a random-direction offset;
 *      returns 1 iff creation failed (-1), else 0.
 *   2: rebind — switch state to 0x11, reset phase, rebind AI
 *      spec/animation to the creature's type; delete the record if the
 *      resulting type is disallowed on the destination level.
 *      Returns 0.
 *   other: returns 0 (source "no branch taken"). */
int dm2_v1_ccm_advanced_explode_or_summon(
    const DM2_V1_CCMAdvancedCallbacks *cb,
    uint8_t mode /* byte@0x20 */,
    DM2_V1_CCMAdvancedReceipt *out);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_CCM_ADVANCED_PC34_COMPAT_H */
