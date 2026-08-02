#ifndef THERON_V1_TRACK02_SPELL_DESCRIPTORS_H
#define THERON_V1_TRACK02_SPELL_DESCRIPTORS_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Source: US Track 02 BIN (MD5 f23601102138f87c33025877767ebf76).
 * Two 41-byte tables immediately after the action/spell name strings.
 * Table 1 from UD 0x09EFAF (stamina/mana cost per action/spell).
 * Table 2 from UD 0x09EFD8 (secondary property, likely experience or cooldown).
 * Indices 0-19 are combat actions, 20-40 are spells.
 * Cross-referenced with action/spell names at UD 0x09EEA3. */

#define THERON_TRACK02_ACTION_SPELL_DESCRIPTOR_COUNT  41u

uint8_t theron_v1_track02_action_spell_cost(unsigned int index);
uint8_t theron_v1_track02_action_spell_secondary(unsigned int index);

#ifdef __cplusplus
}
#endif

#endif /* THERON_V1_TRACK02_SPELL_DESCRIPTORS_H */
