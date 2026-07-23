#include "dm1_v1_f0449_f0450_floppy_media_guard_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int assertions;
static int failures;

#define CHECK(expression) do { \
    ++assertions; \
    if (!(expression)) { \
        ++failures; \
        fprintf(stderr, "%s:%d: %s\\n", __FILE__, __LINE__, #expression); \
    } \
} while (0)

int main(void)
{
    const uint8_t media[8] = {1};
    DM1_V1_F0449F0450FloppyMediaRequestPc34 request;
    DM1_V1_F0449F0450FloppyMediaReceiptPc34 receipt;

    memset(&request, 0, sizeof(request));
    request.raw_media_bytes = media;
    request.raw_media_byte_count = sizeof(media);
    request.raw_media_fingerprint = 1u;
    request.original_pc34_media_verified = 1;
    request.no_emulated_media = 1;
    request.no_synthetic_availability = 1;

    CHECK(!dm1_v1_f0449_is_disk_write_protected_guard_pc34(&request, &receipt));
    CHECK(!receipt.source_body_applicable && !receipt.media_operation_permitted &&
          receipt.fail_closed && receipt.write_protection_probe_suppressed &&
          !receipt.media_change_probe_suppressed && receipt.suppress_synthetic_availability);

    CHECK(!dm1_v1_f0450_force_media_change_detection_guard_pc34(&request, &receipt));
    CHECK(!receipt.source_body_applicable && !receipt.media_operation_permitted &&
          receipt.fail_closed && !receipt.write_protection_probe_suppressed &&
          receipt.media_change_probe_suppressed && receipt.suppress_synthetic_availability);

    request.no_emulated_media = 0;
    CHECK(!dm1_v1_f0449_is_disk_write_protected_guard_pc34(&request, &receipt));
    CHECK(!receipt.fail_closed && !receipt.source_evidence);

    CHECK(strstr(dm1_v1_f0449_f0450_floppy_media_guard_source_evidence_pc34(),
                 "no PC34 F0449/F0450 body") != NULL);
    printf("test_dm1_v1_f0449_f0450_floppy_media_guard_pc34_compat: %d assertions, %d failures\\n",
           assertions, failures);
    return failures == 0 ? 0 : 1;
}
