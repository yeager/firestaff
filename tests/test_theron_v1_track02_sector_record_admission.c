#include "theron_v1_track02_sector_record_admission.h"

#include <stdio.h>
#include <stdlib.h>

static int failures;

#define CHECK(condition) do { if (!(condition)) { \
    fprintf(stderr, "failed: %s (%s:%d)\n", #condition, __FILE__, __LINE__); \
    ++failures; \
} } while (0)

int main(void)
{
    Theron_V1Track02RawMediaIntakeReceipt intake = {0};
    Theron_V1Track02SectorRecordAdmissionReceipt receipt;

    CHECK(theron_v1_track02_sector_record_admit_from_trace(NULL, NULL,
                                                             &receipt));
    CHECK(receipt.status == THERON_V1_TRACK02_SECTOR_RECORD_UNAVAILABLE);
    CHECK(!receipt.dungeon_draw_allowed && !receipt.pixel_decode_allowed);

    intake.status = THERON_V1_TRACK02_MEDIA_INTAKE_REJECTED;
    CHECK(theron_v1_track02_sector_record_admit_from_trace(
        &intake, "/tmp/firestaff-theron-missing-coalesced.trace", &receipt));
    CHECK(receipt.status == THERON_V1_TRACK02_SECTOR_RECORD_REJECTED);
    CHECK(!receipt.raw_cue_bin_identity_consumed &&
          !receipt.nonstartup_record_consumed);

    puts("test_theron_v1_track02_sector_record_admission: SKIP (no local authenticated Track 02 corpus)");
    return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
