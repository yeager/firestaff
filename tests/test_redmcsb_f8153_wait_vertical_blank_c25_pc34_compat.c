#include "redmcsb_f8153_wait_vertical_blank_c25_pc34_compat.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    const uint8_t *values;
    size_t count;
    size_t read_count;
} StatusScript;

static int failures;

static uint8_t read_status(void *context)
{
    StatusScript *script = (StatusScript *)context;
    size_t index = script->read_count;

    ++script->read_count;
    if (index >= script->count) {
        return script->values[script->count - 1U];
    }
    return script->values[index];
}

static void expect_true(const char *name, int actual)
{
    if (!actual) {
        fprintf(stderr, "FAIL: %s\n", name);
        ++failures;
    }
}

static void expect_size(const char *name, size_t actual, size_t expected)
{
    if (actual != expected) {
        fprintf(stderr, "FAIL: %s (got %zu, expected %zu)\n", name, actual,
                expected);
        ++failures;
    }
}

int main(void)
{
    const uint8_t leave_then_enter[] = {0x08U, 0x00U, 0x00U, 0x08U};
    const uint8_t enter_directly[] = {0x00U, 0x08U};
    StatusScript script = {leave_then_enter, sizeof(leave_then_enter), 0U};

    expect_true("leaves old vertical blank and enters next blank",
                redmcsb_f8153_wait_vertical_blank_c25_pc34_compat(
                    read_status, &script));
    expect_size("all source-order status reads", script.read_count, 4U);

    script.values = enter_directly;
    script.count = sizeof(enter_directly);
    script.read_count = 0U;
    expect_true("enters blank from active display",
                redmcsb_f8153_wait_vertical_blank_c25_pc34_compat(
                    read_status, &script));
    expect_size("direct path reads", script.read_count, 2U);

    expect_true("null status reader rejected",
                !redmcsb_f8153_wait_vertical_blank_c25_pc34_compat(NULL, NULL));
    if (strstr(redmcsb_f8153_wait_vertical_blank_source_evidence_pc34(),
               "VIDEODRV.C:3163-3185") == NULL) {
        fprintf(stderr, "FAIL: evidence\n");
        ++failures;
    }
    if (failures != 0) return 1;
    puts("PASSED: ReDMCSB F8153 PC 3.4 C25 vertical-blank polling");
    return 0;
}
