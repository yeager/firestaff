#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "redmcsb_f8134_execute_pc34_compat.h"

typedef struct test_state {
    const char *expected_filename;
    uint16_t exit_status_ax;
    unsigned int call_count;
    size_t parameter_tail_size;
    uint8_t parameter_tail[REDMCSB_F8134_PARAMETER_BUFFER_BYTES_PC34];
} test_state;

static bool execute_program(void *context, const char *filename,
                            const uint8_t *parameter_tail,
                            size_t parameter_tail_size,
                            uint16_t *out_exit_status_ax)
{
    test_state *state = (test_state *)context;

    if (strcmp(filename, state->expected_filename) != 0) {
        return false;
    }
    state->call_count++;
    state->parameter_tail_size = parameter_tail_size;
    memcpy(state->parameter_tail, parameter_tail, parameter_tail_size);
    *out_exit_status_ax = state->exit_status_ax;
    return true;
}

static int check_int(const char *label, int actual, int expected)
{
    if (actual == expected) {
        return 1;
    }
    fprintf(stderr, "%s: got %d, expected %d\n", label, actual, expected);
    return 0;
}

int main(void)
{
    test_state state = {"ANIM.EXE", 0x002aU, 0U, 0U, {0U}};
    char long_parameters[REDMCSB_F8134_MAX_PARAMETER_BYTES_PC34 + 2U];
    int ok = 1;

    ok &= check_int("normal exit code",
                    redmcsb_f8134_execute_program_with_parameters_pc34_compat(
                        execute_program, &state, "ANIM.EXE", "-as"), 42);
    ok &= check_int("execute calls", (int)state.call_count, 1);
    ok &= check_int("tail size", (int)state.parameter_tail_size, 5);
    ok &= check_int("tail length", state.parameter_tail[0], 3);
    ok &= check_int("tail first parameter", state.parameter_tail[1], '-');
    ok &= check_int("tail carriage return", state.parameter_tail[4], '\r');

    state.exit_status_ax = 0x0102U;
    ok &= check_int("non-normal termination",
                    redmcsb_f8134_execute_program_with_parameters_pc34_compat(
                        execute_program, &state, "ANIM.EXE", ""), -1);

    memset(long_parameters, 'x', REDMCSB_F8134_MAX_PARAMETER_BYTES_PC34 + 1U);
    long_parameters[REDMCSB_F8134_MAX_PARAMETER_BYTES_PC34 + 1U] = '\0';
    ok &= check_int("overflowing source buffer rejected",
                    redmcsb_f8134_execute_program_with_parameters_pc34_compat(
                        execute_program, &state, "ANIM.EXE", long_parameters), -1);
    ok &= check_int("no oversized execute", (int)state.call_count, 2);
    ok &= check_int("source anchors",
                    strstr(redmcsb_f8134_execute_source_evidence_pc34(),
                           "IBMIO.C:2318-2364") != 0, 1);

    return ok ? 0 : 1;
}
