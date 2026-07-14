#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "redmcsb_f8131_system_query_pc34_compat.h"

typedef struct test_state {
    uint8_t label[REDMCSB_F8131_VOLUME_LABEL_BYTES_PC34];
    uint8_t seconds;
    uint8_t hundredths;
    uint8_t seen_drive;
    int found;
} test_state;

static bool find_volume_label(
    void *context,
    uint8_t drive,
    uint8_t out_label[REDMCSB_F8131_VOLUME_LABEL_BYTES_PC34])
{
    test_state *state = (test_state *)context;

    state->seen_drive = drive;
    if (!state->found) {
        return false;
    }
    memcpy(out_label, state->label, sizeof(state->label));
    return true;
}

static void get_dos_time(void *context, uint8_t *out_seconds,
                         uint8_t *out_hundredths)
{
    const test_state *state = (const test_state *)context;

    *out_seconds = state->seconds;
    *out_hundredths = state->hundredths;
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
    test_state state = {{'M', 'Y', '.', ' ', 'D', 'I', 'S', 'K', ' ', ' ', ' '},
                        59U, 99U, 0U, 1};
    char volume_name[REDMCSB_F8131_VOLUME_NAME_CAPACITY_PC34];
    int ok = 1;

    redmcsb_f8131_get_volume_name_pc34_compat(
        find_volume_label, &state, 3U, volume_name);
    ok &= check_int("volume drive", state.seen_drive, 3);
    ok &= check_int("volume filter", strcmp(volume_name, "MYDISK"), 0);

    state.found = 0;
    strcpy(volume_name, "unchanged");
    redmcsb_f8131_get_volume_name_pc34_compat(
        find_volume_label, &state, 3U, volume_name);
    ok &= check_int("missing volume clears output", volume_name[0], '\0');

    ok &= check_int("DOS time DX seed",
                    redmcsb_f8132_get_random_seed_pc34_compat(
                        get_dos_time, &state), 0x3B63);
    ok &= check_int("missing DOS time is zero",
                    redmcsb_f8132_get_random_seed_pc34_compat(0, &state), 0);

    redmcsb_f8133_read_floppy_sector_pc34_compat();
    ok &= check_int("source anchors",
                    strstr(redmcsb_f8131_system_query_source_evidence_pc34(),
                           "IBMIO.C:2259-2316") != 0, 1);

    return ok ? 0 : 1;
}
