#include "csb_v1_x68k_startup_handoff.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int read_file(const char *path, uint8_t **out_bytes, size_t *out_size)
{
    FILE *file = NULL;
    long length;
    uint8_t *bytes = NULL;
    if (!path || !out_bytes || !out_size || !(file = fopen(path, "rb")) ||
        fseek(file, 0, SEEK_END) != 0 || (length = ftell(file)) <= 0 ||
        fseek(file, 0, SEEK_SET) != 0 ||
        !(bytes = (uint8_t *)malloc((size_t)length)) ||
        fread(bytes, 1u, (size_t)length, file) != (size_t)length) {
        if (file) fclose(file);
        free(bytes);
        return 0;
    }
    fclose(file);
    *out_bytes = bytes;
    *out_size = (size_t)length;
    return 1;
}

int main(void)
{
    const char *path = getenv("FIRESTAFF_CSB_X68K_HDM");
    CSB_V1_X68kStartupHandoff handoff;
    uint8_t *hdm = NULL;
    size_t hdm_size = 0u;
    int admit_result = CSB_V1_X68K_STARTUP_HANDOFF_ERR_ARGUMENT;
    int ok;

    memset(&handoff, 0, sizeof(handoff));
    if (csb_v1_x68k_startup_handoff_admit(NULL, NULL, 0u) !=
        CSB_V1_X68K_STARTUP_HANDOFF_ERR_ARGUMENT) {
        puts("test_csb_v1_x68k_startup_handoff: argument contract failed");
        return 1;
    }
    if (!path || !path[0]) {
        puts("test_csb_v1_x68k_startup_handoff: SKIP FIRESTAFF_CSB_X68K_HDM unset");
        return 0;
    }
    ok = read_file(path, &hdm, &hdm_size) &&
        (admit_result = csb_v1_x68k_startup_handoff_admit(
             &handoff, hdm, hdm_size)) == CSB_V1_X68K_STARTUP_HANDOFF_OK &&
        handoff.admitted && handoff.x68000_identity_bound &&
        !handoff.host_program_execution_permitted &&
        handoff.graphics.csbX68k && !handoff.graphics.csbAmiga &&
        handoff.graphics.graphicCount == 732u && handoff.dungeon.level_count == 2 &&
        handoff.initial_level == 0 && handoff.initial_x == 9 &&
        handoff.initial_y == 0 && handoff.initial_direction == 2;
    csb_v1_x68k_startup_handoff_cleanup(&handoff);
    free(hdm);
    if (!ok) {
        fprintf(stderr, "test_csb_v1_x68k_startup_handoff: original media mismatch (%d)\n",
                admit_result);
        return 1;
    }
    puts("test_csb_v1_x68k_startup_handoff: PASS");
    return 0;
}
