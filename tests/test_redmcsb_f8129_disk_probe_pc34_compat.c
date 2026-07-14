#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "redmcsb_f8129_disk_probe_pc34_compat.h"

typedef struct test_state {
    uint8_t device_type;
    int16_t seen_drive;
    int16_t seen_bios_drives[3];
    unsigned int read_calls;
    unsigned int reset_calls;
    unsigned int successful_read_call;
} test_state;

static bool get_device_type(void *context, int16_t drive_number,
                            uint8_t *out_device_type)
{
    test_state *state = (test_state *)context;

    state->seen_drive = drive_number;
    *out_device_type = state->device_type;
    return true;
}

static bool read_first_sector(
    void *context,
    int16_t bios_drive_number,
    uint8_t out_sector[REDMCSB_F8129_FIRST_SECTOR_BYTES_PC34])
{
    test_state *state = (test_state *)context;

    state->seen_bios_drives[state->read_calls] = bios_drive_number;
    state->read_calls++;
    out_sector[0] = (uint8_t)state->read_calls;
    return state->successful_read_call == state->read_calls;
}

static void reset_disk(void *context, int16_t bios_drive_number)
{
    test_state *state = (test_state *)context;

    state->seen_bios_drives[state->reset_calls] = bios_drive_number;
    state->reset_calls++;
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
    test_state state = {7U, 0, {0, 0, 0}, 0U, 0U, 3U};
    int16_t converted_type = -1;
    int ok = 1;

    ok &= check_int("converted device succeeds",
                    redmcsb_f8129_get_converted_device_type_pc34_compat(
                        get_device_type, &state, 4, &converted_type), 1);
    ok &= check_int("device query drive", state.seen_drive, 4);
    ok &= check_int("1.44M conversion", converted_type, 19);

    state.device_type = 8U;
    converted_type = 77;
    ok &= check_int("undefined device type rejected",
                    redmcsb_f8129_get_converted_device_type_pc34_compat(
                        get_device_type, &state, 4, &converted_type), 0);
    ok &= check_int("undefined type preserves output", converted_type, 77);

    ok &= check_int("third sector read succeeds",
                    redmcsb_f8130_get_read_first_sector_result_pc34_compat(
                        read_first_sector, reset_disk, &state, 2), 1);
    ok &= check_int("three reads", (int)state.read_calls, 3);
    ok &= check_int("two resets", (int)state.reset_calls, 2);
    ok &= check_int("read BIOS drive", state.seen_bios_drives[2], 1);
    ok &= check_int("reset BIOS drive", state.seen_bios_drives[1], 1);

    state.read_calls = 0U;
    state.reset_calls = 0U;
    state.successful_read_call = 0U;
    ok &= check_int("all reads fail",
                    redmcsb_f8130_get_read_first_sector_result_pc34_compat(
                        read_first_sector, reset_disk, &state, 1), 0);
    ok &= check_int("three failed reads", (int)state.read_calls, 3);
    ok &= check_int("three failed resets", (int)state.reset_calls, 3);
    ok &= check_int("source anchors",
                    strstr(redmcsb_f8129_disk_probe_source_evidence_pc34(),
                           "IBMIO.C:2167-2206,2209-2240") != 0, 1);

    return ok ? 0 : 1;
}
