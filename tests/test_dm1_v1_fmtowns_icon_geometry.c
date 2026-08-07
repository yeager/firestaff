#include "dm1_v1_fmtowns_icon_geometry.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void test_byte_verified_constants(void) {
    /* Every value is a byte-verified word from the shipping EDM.EXP.
     * Any edit that drifts these must be accompanied by a matching
     * disassembly update in parity-evidence/. */
    assert(DM1_V1_FMTOWNS_ICON_SCR_X_SIZE == 320);
    assert(DM1_V1_FMTOWNS_ICON_SIZE_BYTES == 256);
    assert(DM1_V1_FMTOWNS_ICON_X_SIZE == 16);
    assert(DM1_V1_FMTOWNS_ICON_Y_SIZE == 16);
}

static void test_self_consistent(void) {
    assert(dm1_v1_fmtowns_icon_geometry_is_self_consistent_pc34() == 1);
}

static void test_get_accessor(void) {
    uint16_t b = 0, x = 0, y = 0;
    assert(dm1_v1_fmtowns_icon_geometry_get_pc34(&b, &x, &y) == 1);
    assert(b == 256);
    assert(x == 16);
    assert(y == 16);
    /* NULL gate */
    assert(dm1_v1_fmtowns_icon_geometry_get_pc34(NULL, &x, &y) == 0);
    assert(dm1_v1_fmtowns_icon_geometry_get_pc34(&b, NULL, &y) == 0);
    assert(dm1_v1_fmtowns_icon_geometry_get_pc34(&b, &x, NULL) == 0);
}

/* Real-data round-trip: if the caller supplies EDM.EXP via env
 * FIRESTAFF_DM1_FMTOWNS_EDM_EXP, read the four words at the source
 * vaddrs and confirm they match the shipped constants. Skipped
 * cleanly without game data. */
static void test_real_data_round_trip(void) {
    const char *path = getenv("FIRESTAFF_DM1_FMTOWNS_EDM_EXP");
    FILE *fp;
    uint8_t buf[8];
    if (!path || !path[0]) { puts("SKIP: FIRESTAFF_DM1_FMTOWNS_EDM_EXP not set"); return; }
    fp = fopen(path, "rb");
    if (!fp) { puts("SKIP: cannot open EDM.EXP"); return; }
    /* Phar Lap P3 load offset 0x200. Read SCR_X_SIZE at file offset
     * 0x200 + 0x26c68 = 0x26e68 (SCR_X_SIZE, ..., ICON_SIZE at 0x26c76). */
    /* Fetch 8 bytes covering SCR_X_SIZE (0x26c68) is separate from
     * ICON_SIZE (0x26c76). Do two reads. */
    if (fseek(fp, 0x200 + 0x26c68, SEEK_SET) != 0) {
        fclose(fp); puts("SKIP: seek failed"); return;
    }
    if (fread(buf, 1, 2, fp) != 2) { fclose(fp); puts("SKIP: read failed"); return; }
    uint16_t scr_x = (uint16_t)(buf[0] | (buf[1] << 8));
    assert(scr_x == DM1_V1_FMTOWNS_ICON_SCR_X_SIZE);
    if (fseek(fp, 0x200 + 0x26c76, SEEK_SET) != 0) {
        fclose(fp); puts("SKIP: seek failed"); return;
    }
    if (fread(buf, 1, 6, fp) != 6) { fclose(fp); puts("SKIP: read failed"); return; }
    uint16_t icon_size = (uint16_t)(buf[0] | (buf[1] << 8));
    uint16_t icon_x    = (uint16_t)(buf[2] | (buf[3] << 8));
    uint16_t icon_y    = (uint16_t)(buf[4] | (buf[5] << 8));
    assert(icon_size == DM1_V1_FMTOWNS_ICON_SIZE_BYTES);
    assert(icon_x    == DM1_V1_FMTOWNS_ICON_X_SIZE);
    assert(icon_y    == DM1_V1_FMTOWNS_ICON_Y_SIZE);
    fclose(fp);
    puts("PASS: real EDM.EXP icon geometry matches shipped constants");
}

int main(void) {
    test_byte_verified_constants();
    test_self_consistent();
    test_get_accessor();
    test_real_data_round_trip();
    printf("All dm1_v1_fmtowns_icon_geometry tests passed.\n");
    return 0;
}
