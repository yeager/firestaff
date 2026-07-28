/* Bounded parser for Atari ST CSB ANIMATE.SCR instruction streams. */
#ifndef FIRESTAFF_CSB_V1_ANIMATION_SCRIPT_H
#define FIRESTAFF_CSB_V1_ANIMATION_SCRIPT_H

#include <stddef.h>
#include <stdint.h>

enum {
    CSB_V1_ANIMATION_SCRIPT_MAX_INSTRUCTIONS = 4096,
    CSB_V1_ANIMATION_SCRIPT_MAX_PARAMETERS = 11
};

typedef enum {
    CSB_V1_ANIMATION_SCRIPT_OK = 0,
    CSB_V1_ANIMATION_SCRIPT_ERR_ARGUMENT = -1,
    CSB_V1_ANIMATION_SCRIPT_ERR_TRUNCATED = -2,
    CSB_V1_ANIMATION_SCRIPT_ERR_OPCODE = -3,
    CSB_V1_ANIMATION_SCRIPT_ERR_CAPACITY = -4,
    CSB_V1_ANIMATION_SCRIPT_ERR_NO_STOP = -5
} CSB_V1_AnimationScriptResult;

typedef struct {
    uint16_t opcode;
    uint16_t parameter_count;
    uint16_t parameters[CSB_V1_ANIMATION_SCRIPT_MAX_PARAMETERS];
    size_t byte_offset;
} CSB_V1_AnimationScriptInstruction;

/* Returns the documented parameter count for opcodes 1..30, or -1. */
int csb_v1_animation_script_parameter_count(uint16_t opcode);

/* Parses the big-endian word stream used by Atari ST CSB ANIMATE.SCR.
 * `out` may be NULL to query the required instruction count. A successful
 * parse requires opcode 1 or 2 as a terminating Stop instruction. */
int csb_v1_animation_script_parse(
    const uint8_t *bytes,
    size_t size,
    CSB_V1_AnimationScriptInstruction *out,
    size_t out_capacity,
    size_t *out_count);

#endif
