/*
 * Bounded decoder-state adapter for ReDMCSB LZW.C
 * F0496_LZW_OutputCharacter.
 *
 * The source routine appends one decoded byte through its global output
 * pointer. This adapter makes that mutable output state explicit and
 * caller-owned so decoding cannot write outside the supplied buffer.
 */
#ifndef FIRESTAFF_CSB_V1_F0496_LZW_OUTPUT_CHARACTER_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_F0496_LZW_OUTPUT_CHARACTER_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint8_t *output;
    size_t output_capacity;
    size_t output_count;
    uint8_t repeat_flag;
    uint8_t repeat_character;
} CsbV1F0496LzwDecoderStatePc34Compat;

/*
 * Consume one decoded character. The source stores ordinary characters and
 * treats 0x90 as a repeat escape: `0x90, 0` stores a literal 0x90, while
 * `0x90, count` repeats the previous ordinary character `count - 1` times.
 * Returns 1 after consuming the character. Returns 0 when state/storage is
 * invalid, an escape expansion would exceed capacity, or `repeat_flag` is not
 * a source-valid 0/1 value. A rejected call does not modify state or storage.
 */
int csb_v1_f0496_lzw_output_character_pc34_compat(
    CsbV1F0496LzwDecoderStatePc34Compat *state,
    uint8_t character);

int F0496_LZW_OutputCharacter(
    CsbV1F0496LzwDecoderStatePc34Compat *state,
    uint8_t character);

/* Exact source locator retained for standalone provenance checks. */
const char *csb_v1_f0496_lzw_output_character_pc34_compat_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CSB_V1_F0496_LZW_OUTPUT_CHARACTER_PC34_COMPAT_H */
