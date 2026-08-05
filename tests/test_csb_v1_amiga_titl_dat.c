/* Real-media verification for the Amiga TITL.DAT title timeline.
 * Set FIRESTAFF_CSB_AMIGA_TITL to an extracted TITL.DAT. */

#include "csb_v1_amiga_titl_dat.h"

#include <stdio.h>
#include <stdlib.h>

static int read_file(const char *path, uint8_t **out_data, size_t *out_size)
{
    FILE *file;
    long length;
    uint8_t *data;

    file = fopen(path, "rb");
    if (!file || fseek(file, 0, SEEK_END) != 0 || (length = ftell(file)) <= 0 ||
        fseek(file, 0, SEEK_SET) != 0) {
        if (file) fclose(file);
        return -1;
    }
    data = (uint8_t *)malloc((size_t)length);
    if (!data || fread(data, 1u, (size_t)length, file) != (size_t)length) {
        free(data);
        fclose(file);
        return -1;
    }
    fclose(file);
    *out_data = data;
    *out_size = (size_t)length;
    return 0;
}

int main(void)
{
    const char *path = getenv("FIRESTAFF_CSB_AMIGA_TITL");
    CSB_V1_AmigaTitlSchedule schedule;
    uint8_t *data;
    size_t size;

    if (!path || !*path) {
        puts("skip: FIRESTAFF_CSB_AMIGA_TITL is not set");
        return 77;
    }
    if (read_file(path, &data, &size) != 0 ||
        csb_v1_amiga_titl_dat_decode(data, size, &schedule) != 0) {
        free(data);
        fputs("FAIL: cannot decode real Amiga TITL.DAT\n", stderr);
        return 1;
    }
    free(data);

    if (schedule.width != 320u || schedule.height != 200u ||
        schedule.bit_depth != 4u || schedule.delta_count != 31u ||
        schedule.initial_duration_vbl != 126u ||
        schedule.delta_durations_vbl[9] != 18u ||
        schedule.delta_durations_vbl[25] != 12u ||
        schedule.delta_durations_vbl[30] != 282u ||
        schedule.total_duration_vbl != 606u) {
        fputs("FAIL: unexpected Amiga TITL.DAT schedule\n", stderr);
        return 1;
    }
    puts("ok: real Amiga TITL.DAT has 32 title frames over 606 VBL");
    return 0;
}
