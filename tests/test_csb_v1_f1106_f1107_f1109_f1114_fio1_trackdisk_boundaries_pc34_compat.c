#include "csb_v1_f1106_f1107_f1109_f1114_fio1_trackdisk_boundaries_pc34_compat.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(condition)                                                        \
    do {                                                                        \
        if (!(condition)) {                                                     \
            fprintf(stderr, "CHECK failed at %s:%d: %s\n", __FILE__, __LINE__, \
                    #condition);                                                \
            exit(1);                                                            \
        }                                                                       \
    } while (0)

static void check_contains(const char *text, const char *needle)
{
    CHECK(text != NULL);
    CHECK(strstr(text, needle) != NULL);
}

static void test_no_host_trackdisk_boundaries(void)
{
    volatile uint32_t sentinel = 0x11061114U;

    CHECK(csb_v1_f1106_is_trackdisk_device_opened_pc34_compat(0) == 0);
    CHECK(csb_v1_f1106_is_trackdisk_device_opened_pc34_compat(3) == 0);
    CHECK(F1106_IsTrackdiskDeviceOpened(-1) == 0);

    CHECK(csb_v1_f1107_get_disk_change_counter_pc34_compat(0) == -1);
    CHECK(F1107_GetDiskChangeCounter(3) == -1);

    CHECK(csb_v1_f1109_get_disk_state_pc34_compat(0) == 0);
    CHECK(csb_v1_f1109_get_disk_state_pc34_compat(3) == 0);
    CHECK(F1109_GetDiskState(-1) == 0);

    csb_v1_f1114_close_trackdisk_device_pc34_compat();
    F1114_CloseTrackdiskDevice();
    CHECK(sentinel == 0x11061114U);
}

static void test_evidence_strings(void)
{
    const char *f1106 =
        csb_v1_f1106_is_trackdisk_device_opened_source_evidence_pc34();
    const char *f1107 =
        csb_v1_f1107_get_disk_change_counter_source_evidence_pc34();
    const char *f1109 = csb_v1_f1109_get_disk_state_source_evidence_pc34();
    const char *f1114 =
        csb_v1_f1114_close_trackdisk_device_source_evidence_pc34();

    check_contains(f1106, "FLOPPYAM.C:753-763");
    check_contains(f1106, "FIO1.C:737");
    check_contains(f1106, "not-opened");

    check_contains(f1107, "FLOPPYAM.C:765-778");
    check_contains(f1107, "returns -1");
    check_contains(f1107, "FIO1.C:156-157");

    check_contains(f1109, "FLOPPYAM.C:830-849");
    check_contains(f1109, "state to 0");
    check_contains(f1109, "FIO1.C:970");

    check_contains(f1114, "FLOPPYAM.C:335-342");
    check_contains(f1114, "CNFG.C:173");
    check_contains(f1114, "no opened trackdisk device");
}

int main(void)
{
    test_no_host_trackdisk_boundaries();
    test_evidence_strings();
    return 0;
}
