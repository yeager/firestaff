#include "redmcsb_f0450_floppy_force_media_change_detection_pc34_compat.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

static int check(int condition, const char *label)
{
    if (condition) {
        return 1;
    }
    fprintf(stderr, "FAIL: %s\n", label);
    return 0;
}

int main(void)
{
    const char *evidence;
    uint16_t drive_type = UINT16_C(1);
    int ok = 1;

    redmcsb_f0450_floppy_force_media_change_detection_pc34_compat(drive_type);
    redmcsb_f0450_floppy_force_media_change_detection_pc34_compat(UINT16_C(2));
    redmcsb_f0450_floppy_force_media_change_detection_pc34_compat(UINT16_C(0));

    evidence =
        redmcsb_f0450_floppy_force_media_change_detection_source_evidence_pc34();
    ok &= check(evidence != NULL, "source evidence is available");
    ok &= check(strstr(evidence, "no F0450 state mutation") != NULL,
                "mapping documents the PC 3.4 no-state contract");
    ok &= check(strstr(evidence, "callback behavior") != NULL,
                "mapping documents the PC 3.4 no-callback contract");

    if (!ok) {
        return 1;
    }
    puts("PASS redmcsb_f0450_floppy_force_media_change_detection_pc34_compat");
    return 0;
}
