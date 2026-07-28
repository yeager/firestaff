#include "csb_v1_animation_script.h"

static uint16_t read_be16(const uint8_t *bytes, size_t offset)
{
    return (uint16_t)(((uint16_t)bytes[offset] << 8) | bytes[offset + 1u]);
}

int csb_v1_animation_script_parameter_count(uint16_t opcode)
{
    static const uint8_t counts[] = {
        0u, 0u, 3u, 1u, 3u, 4u, 1u, 2u, 0u, 1u,
        1u, 2u, 0u, 2u, 0u, 1u, 1u, 2u, 2u, 3u,
        4u, 5u, 6u, 7u, 8u, 9u, 10u, 11u, 2u, 3u
    };

    if (opcode == 0u || opcode > 30u) return -1;
    return (int)counts[opcode - 1u];
}

int csb_v1_animation_script_parse(
    const uint8_t *bytes,
    size_t size,
    CSB_V1_AnimationScriptInstruction *out,
    size_t out_capacity,
    size_t *out_count)
{
    size_t offset = 0u;
    size_t count = 0u;
    int stopped = 0;

    if (!bytes || !out_count || (out == NULL && out_capacity != 0u)) {
        return CSB_V1_ANIMATION_SCRIPT_ERR_ARGUMENT;
    }
    while (offset < size) {
        CSB_V1_AnimationScriptInstruction instruction;
        int parameter_count;
        size_t instruction_bytes;
        size_t i;

        if (size - offset < 2u) return CSB_V1_ANIMATION_SCRIPT_ERR_TRUNCATED;
        instruction.opcode = read_be16(bytes, offset);
        parameter_count = csb_v1_animation_script_parameter_count(
            instruction.opcode);
        if (parameter_count < 0) return CSB_V1_ANIMATION_SCRIPT_ERR_OPCODE;
        instruction_bytes = 2u + (size_t)parameter_count * 2u;
        if (size - offset < instruction_bytes) {
            return CSB_V1_ANIMATION_SCRIPT_ERR_TRUNCATED;
        }
        if (count >= CSB_V1_ANIMATION_SCRIPT_MAX_INSTRUCTIONS ||
            (out != NULL && count >= out_capacity)) {
            return CSB_V1_ANIMATION_SCRIPT_ERR_CAPACITY;
        }
        instruction.parameter_count = (uint16_t)parameter_count;
        instruction.byte_offset = offset;
        for (i = 0u; i < (size_t)parameter_count; ++i) {
            instruction.parameters[i] = read_be16(bytes, offset + 2u + i * 2u);
        }
        if (out != NULL) out[count] = instruction;
        ++count;
        offset += instruction_bytes;
        if (instruction.opcode == 1u || instruction.opcode == 2u) {
            stopped = 1;
            break;
        }
    }
    *out_count = count;
    return stopped ? CSB_V1_ANIMATION_SCRIPT_OK :
        CSB_V1_ANIMATION_SCRIPT_ERR_NO_STOP;
}
