#ifndef THERON_V1_TRACK02_CREATURE_SPAWN_H
#define THERON_V1_TRACK02_CREATURE_SPAWN_H

#include <stdint.h>
#include <stddef.h>

#include "theron_v1_dungeon_handoff.h"
#include "theron_v1_track02_spawn_binding.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Source: US Track 02 BIN (MD5 f23601102138f87c33025877767ebf76).
 *
 * Theron's Quest does NOT have a DM1-style G0243_CreatureInfo per-type
 * stat table.  Instead it uses a CATEGORY-BASED formula system where
 * creature TYPE determines appearance (sprite descriptor) while creature
 * CATEGORY (0-3) determines combat stats via formulas with random
 * multipliers at spawn time.
 *
 * Spawn code at UD 0x0870E5 (PCE $B0E5), identical across all 7 dungeon
 * banks.  HP capped at 900 (0x0384), same as DM1. */

#define THERON_CREATURE_SPAWN_CATEGORY_COUNT  4
#define THERON_CREATURE_HP_CAP                900

/* Category branch constants observed in disassembly:
 *   Cat 0: dice param = 4
 *   Cat 1: multiplier = 21 (0x15)
 *   Cat 2: multiplier = 25 (0x19), then *1.5
 *   Cat 3: dice param = 5, scaling + 1.5*adj */
typedef struct {
    uint8_t  category;
    uint8_t  multiplier;     /* base multiplier (21 or 25), or 0 for dice-based */
    uint8_t  dice_param;     /* random dice parameter (4 or 5), or 0 for mult-based */
    uint8_t  uses_1_5x;      /* 1 if result is scaled by 1.5 */
} Theron_SpawnCategoryFormula;

const Theron_SpawnZoneDesc *theron_v1_track02_spawn_zone(unsigned int creature_index);
const Theron_SpawnCategoryFormula *theron_v1_track02_spawn_formula(unsigned int category);
size_t theron_v1_track02_spawn_zone_count(void);

const Theron_CreaturePointerEntry *theron_v1_track02_creature_pointer(unsigned int index);

/* Authenticated source decode for the pointer/regular-spawn records.  The
 * input is the complete raw MODE1/2352 US Track 02 BIN, not a host-generated
 * table and not a stripped user-data buffer.  The decoder verifies the exact
 * US Track 02 MD5 before it reads the source bytes at UD 0x274000. The JP BIN
 * has a different layout at these addresses and is rejected until its own
 * source offsets are authenticated. */
/* Returns 1 only for a complete, hash-verified retail BIN whose pointer and
 * regular-spawn records can be read safely.  The output is cleared on every
 * failure.  This binds source records only; it does not authorize RNG,
 * creature AI, combat, generator timing, T700, or T900 semantics. */
int theron_v1_track02_decode_spawn_source(
    const uint8_t *raw_track02,
    size_t raw_track02_bytes,
    Theron_V1Track02Variant variant,
    Theron_Track02SpawnSource *out);

/* Reserved result shape for a future captured spawn consumer.  No host seed
 * may stand in for the original RNG.  Until the HuC6280 RNG return contract
 * and its consumers are captured, the function below always fails closed and
 * clears this structure. */
typedef struct {
    int16_t hp;
    int16_t attack;
    int16_t defense;
} Theron_SpawnStats;

/* Register/return-value witness for the visible part of UD $B0E5.  Every
 * value in this structure must come from the same authenticated HuC6280
 * execution window; this is deliberately not a seed and does not call an
 * RNG.  The helper routines at $5A76/$5B8F/$D23A and $4667 remain external
 * inputs until their live contracts are captured.
 *
 * ReDMCSB has no Theron's Quest source tree.  The byte-backed source lock is
 * docs/source-lock/theron-disassembly/theron-us-spawn-consumer.asm.
 */
struct Theron_SpawnConsumerWitness {
    int authenticated_execution;
    uint8_t category;
    uint8_t b0;
    uint8_t b1;
    uint8_t b6;
    uint16_t b4b5;
    uint16_t b8_before_branch;
    uint16_t helper_b8;
    uint8_t lb0fe_return_1;
    uint8_t lb0fe_return_2;
    uint8_t rng_common_1;
    uint8_t rng_common_2;
    uint8_t ld23a_b8;
    uint8_t ld23a_b4;
    uint16_t hp_accumulator;
    uint16_t attack_accumulator;
    uint16_t defense_accumulator;
};

typedef struct {
    int valid;
    uint16_t hp_accumulator;
    uint16_t attack_accumulator;
    uint16_t defense_accumulator;
    uint8_t object_write_1;
    uint8_t object_write_2;
    uint8_t helper_input_b8;
    uint8_t helper_input_b4;
    uint8_t helper_input_b5;
} Theron_SpawnConsumerReceipt;

/* Executes only the instruction-visible arithmetic in $B0E5-$B1EB against
 * captured register/helper values.  It is a forensic receipt, not a gameplay
 * admission path: callers must still prove the dynamic RNG owner and the
 * later creature/stat consumers before publishing these values to runtime.
 */
int theron_v1_track02_apply_spawn_consumer_witness(
    const Theron_SpawnConsumerWitness *witness,
    Theron_SpawnConsumerReceipt *out);

/* Returns 1 only after the original RNG consumer has been source-bound.
 * Current implementation returns 0 for every input. */
int theron_v1_track02_compute_spawn_stats(
    unsigned int category, uint8_t param1, uint8_t param2,
    uint16_t rand_seed, Theron_SpawnStats *out);

#ifdef __cplusplus
}
#endif

#endif /* THERON_V1_TRACK02_CREATURE_SPAWN_H */
