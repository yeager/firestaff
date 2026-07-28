#include "csb_v1_animation_script.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int failures = 0;

#define CHECK(condition, message) do { \
    if (condition) printf("PASS: %s\n", message); \
    else { printf("FAIL: %s\n", message); ++failures; } \
} while (0)

int main(void)
{
    const uint8_t script[] = {
        0x00, 0x03, 0x00, 0x1e, 0x00, 0x23, 0x00, 0x00,
        0x00, 0x07, 0x00, 0x05,
        0x00, 0x0e, 0x00, 0x23, 0x00, 0x05,
        0x00, 0x13, 0x00, 0x30, 0x00, 0x02,
        0x00, 0x14, 0x00, 0x30, 0x00, 0x01, 0x00, 0x02,
        0x00, 0x01
    };
    const uint8_t truncated[] = { 0x00, 0x03, 0x00 };
    const uint8_t unknown[] = { 0x00, 0x1f };
    CSB_V1_AnimationScriptInstruction instructions[8];
    size_t count = 0u;
    const char *real_path;

    CHECK(csb_v1_animation_script_parameter_count(3u) == 3 &&
              csb_v1_animation_script_parameter_count(6u) == 4 &&
              csb_v1_animation_script_parameter_count(28u) == 11 &&
              csb_v1_animation_script_parameter_count(30u) == 3,
          "documented ANIMATE.SCR opcode parameter widths are recognized");
    CHECK(csb_v1_animation_script_parse(script, sizeof(script), instructions,
              8u, &count) == CSB_V1_ANIMATION_SCRIPT_OK && count == 6u,
          "big-endian script parses through its Stop instruction");
    CHECK(instructions[0].opcode == 3u && instructions[0].parameter_count == 3u &&
              instructions[0].parameters[0] == 0x1eu &&
              instructions[0].parameters[1] == 0x23u &&
              instructions[4].opcode == 20u && instructions[4].parameter_count == 3u,
          "Load, palette/present, and multi-value instructions retain words");
    CHECK(csb_v1_animation_script_parse(truncated, sizeof(truncated), NULL,
              0u, &count) == CSB_V1_ANIMATION_SCRIPT_ERR_TRUNCATED,
          "truncated instruction parameters fail closed");
    CHECK(csb_v1_animation_script_parse(unknown, sizeof(unknown), NULL,
              0u, &count) == CSB_V1_ANIMATION_SCRIPT_ERR_OPCODE,
          "unknown instruction opcodes fail closed");
    CHECK(csb_v1_animation_script_parse(script, sizeof(script), instructions,
              2u, &count) == CSB_V1_ANIMATION_SCRIPT_ERR_CAPACITY,
          "bounded output capacity is enforced");

    real_path = getenv("FIRESTAFF_CSB_ANIMATE_SCR");
    if (real_path && real_path[0] != '\0') {
        FILE *fp = fopen(real_path, "rb");
        long length;
        uint8_t *bytes;
        CSB_V1_AnimationScriptInstruction *real_instructions;

        CHECK(fp != NULL, "real Atari ANIMATE.SCR opens");
        if (fp && fseek(fp, 0L, SEEK_END) == 0 &&
            (length = ftell(fp)) > 0 && fseek(fp, 0L, SEEK_SET) == 0 &&
            (bytes = (uint8_t *)malloc((size_t)length)) != NULL &&
            fread(bytes, 1u, (size_t)length, fp) == (size_t)length &&
            (real_instructions = (CSB_V1_AnimationScriptInstruction *)calloc(
                CSB_V1_ANIMATION_SCRIPT_MAX_INSTRUCTIONS,
                sizeof(*real_instructions))) != NULL) {
            fclose(fp);
            CHECK(csb_v1_animation_script_parse(bytes, (size_t)length,
                      real_instructions,
                      CSB_V1_ANIMATION_SCRIPT_MAX_INSTRUCTIONS, &count) ==
                      CSB_V1_ANIMATION_SCRIPT_OK && count > 1u &&
                      (real_instructions[count - 1u].opcode == 1u ||
                       real_instructions[count - 1u].opcode == 2u),
                  "real Atari ANIMATE.SCR parses through its Stop instruction");
            free(real_instructions);
            free(bytes);
        } else {
            if (fp) fclose(fp);
            CHECK(0, "real Atari ANIMATE.SCR reads within parser bounds");
        }
    }
    return failures == 0 ? 0 : 1;
}
