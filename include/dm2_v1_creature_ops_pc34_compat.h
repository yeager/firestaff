#ifndef FIRESTAFF_DM2_V1_CREATURE_OPS_PC34_COMPAT_H
#define FIRESTAFF_DM2_V1_CREATURE_OPS_PC34_COMPAT_H

/*
 * dm2_v1_creature_ops_pc34_compat.h — DM2 V1 creature operations from
 * skproject/SKULLWIN/c_creature.cpp.
 *
 * Callback-based implementations of:
 *   DM2_APPLY_CREATURE_POISON_RESISTANCE  c_creature.cpp:34
 *   DM2_ROTATE_CREATURE                   c_creature.cpp:58
 *   DM2_CREATURE_CAN_HANDLE_ITEM_IN      c_creature.cpp:104
 *   DM2_CONFUSE_CREATURE                  c_creature.cpp:1308
 *   DM2_CREATURE_KILL_ON_TIMER_POSITION   c_creature.cpp:2352
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ---- DM2_APPLY_CREATURE_POISON_RESISTANCE (c_creature.cpp:34) ----
 * Applies creature's poison resistance to incoming damage.
 * Returns reduced poison amount, 0 if immune. */
typedef struct {
    const uint8_t *(*query_ai_spec)(void *ctx, uint16_t creature_type);
    uint16_t (*rand_dir)(void *ctx);
} DM2_V1_CreaturePoisonCallbacks;

int16_t dm2_v1_apply_creature_poison_resistance(
    uint16_t creature_type, int16_t poison_amount,
    const DM2_V1_CreaturePoisonCallbacks *cb, void *ctx);

/* ---- DM2_ROTATE_CREATURE (c_creature.cpp:58) ----
 * Rotates a creature record's facing direction.
 * mode 0: add delta to current facing.
 * mode != 0: set to absolute direction.
 * Also rotates all items in creature's possession chain if AI flag 0x01 set. */
typedef struct {
    uint8_t *(*get_record_address)(void *ctx, uint16_t record_word);
    uint16_t (*query_ai_spec_flags)(void *ctx, uint16_t record_word);
    uint8_t *(*get_next_record_address)(void *ctx, uint16_t record_word);
} DM2_V1_CreatureRotateCallbacks;

void dm2_v1_rotate_creature(
    uint16_t creature_record, int mode, int direction,
    const DM2_V1_CreatureRotateCallbacks *cb, void *ctx);

/* ---- DM2_CREATURE_CAN_HANDLE_ITEM_IN (c_creature.cpp:104) ----
 * Walk a record chain, find the first item (types 5..13 or type 9)
 * matching a direction filter that the creature can handle.
 * direction_filter 0xFF = any direction.
 * Returns the matching record word, or 0xFFFE (END) if none. */
typedef struct {
    int16_t (*get_next_record_link)(void *ctx, uint16_t record_word);
    int (*creature_can_handle_it)(void *ctx, uint16_t record_word,
                                  int16_t creature_type);
} DM2_V1_CreatureHandleCallbacks;

int16_t dm2_v1_creature_can_handle_item_in(
    int16_t creature_type, int16_t first_record, uint8_t direction_filter,
    const DM2_V1_CreatureHandleCallbacks *cb, void *ctx);

/* ---- DM2_CONFUSE_CREATURE (c_creature.cpp:1308) ----
 * Sets confusion flag on a creature. Reads byte at offset+0x11 of the
 * creature record and sets bit 0x04. */
int dm2_v1_confuse_creature(uint8_t *creature_record);

/* ---- DM2_CREATURE_KILL_ON_TIMER_POSITION (c_creature.cpp:2352) ----
 * Checks if a creature should die based on timer position.
 * Just calls DM2_WOUND_CREATURE(record) when the creature's HP <= 0. */

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_CREATURE_OPS_PC34_COMPAT_H */
