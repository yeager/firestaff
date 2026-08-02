#ifndef THERON_V1_TRACK02_ACTION_PARAMS_H
#define THERON_V1_TRACK02_ACTION_PARAMS_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Source: US Track 02 BIN (MD5 f23601102138f87c33025877767ebf76).
 * Action/spell parameter records from UD 0x09EFB0.
 * 21 x 2-byte records for spell indices 20-40 (FIREBALL through THROW).
 * Field semantics: byte 0 = stamina/mana cost, byte 1 = power/damage base.
 * Exact field roles are undecoded pending PCE disassembly confirmation. */

#define THERON_TRACK02_SPELL_PARAM_COUNT  21u

typedef struct {
    uint8_t field0;
    uint8_t field1;
} Theron_Track02SpellParam;

const Theron_Track02SpellParam *theron_v1_track02_spell_param(unsigned int spell_index);
size_t theron_v1_track02_spell_param_count(void);

/* Extended action parameter block — 80 raw bytes from UD 0x09EFB0.
 * Includes the 21 spell records above plus additional data whose
 * record boundaries are not yet confirmed. */
#define THERON_TRACK02_ACTION_PARAM_RAW_SIZE  80u

const uint8_t *theron_v1_track02_action_param_raw(void);

#ifdef __cplusplus
}
#endif

#endif /* THERON_V1_TRACK02_ACTION_PARAMS_H */
