#include "dm2_v1_boot.h"
#include "dm2_v1_dos_real_data_manifest.h"
#include "dm2_v1_mve_stream.h"

#include <stdio.h>
#include <stdlib.h>

#define CHECK(condition) do { \
    if (!(condition)) { \
        fprintf(stderr, "FAIL: %s (%s:%d)\n", #condition, __FILE__, __LINE__); \
        dm2_v1_boot_cleanup(&profile); \
        return 1; \
    } \
} while (0)

int main(void)
{
    const char *data_root = getenv("FIRESTAFF_DM2_DATA_DIR");
    const dm2_v1_dos_file_fp_t *intro =
        dm2_v1_dos_file_fp_lookup_pc34("intro");
    DM2_V1_BootProfile profile;
    DM2_V1_MveStreamReceipt stream;
    const uint8_t *bytes = NULL;
    size_t byte_count = 0u;

    if (!data_root || !data_root[0]) {
        puts("SKIP: no DM2 DOS DATA root");
        return 0;
    }
    dm2_v1_boot_profile_init(&profile);
    CHECK(intro != NULL);
    CHECK(dm2_v1_boot_scan_assets(&profile, data_root) == 0);
    CHECK(profile.assets_verified && profile.platform == DM2_PLATFORM_PC_EN);
    CHECK(profile.dos_startup_media_verified &&
          profile.dos_startup_media.valid &&
          profile.dos_startup_media.intro_verified &&
          profile.dos_startup_media.intro_has_interplay_mve);
    if (!profile.dos_intro_mve_owner) {
        fprintf(stderr, "missing owner: header=%u receipt=%08x\n",
                profile.dos_startup_media.intro_mve_header_offset,
                profile.dos_startup_media.receipt_hash);
    }
    CHECK(dm2_v1_boot_dos_intro_mve_readonly(&profile, &bytes,
                                              &byte_count) == 1);
    CHECK(bytes != NULL && byte_count == intro->size_bytes);
    CHECK(dm2_v1_mve_stream_parse(bytes, byte_count, &stream) == 1);
    CHECK(stream.valid &&
          stream.mve_offset == profile.dos_startup_media.intro_mve_header_offset &&
          stream.width == 320u && stream.height == 200u &&
          stream.display_count == 217u && stream.audio_frame_count == 217u);

    dm2_v1_boot_cleanup(&profile);
    if (dm2_v1_boot_dos_intro_mve_readonly(&profile, &bytes,
                                            &byte_count) != 0 ||
        bytes != NULL || byte_count != 0u) {
        fprintf(stderr, "FAIL: cleanup releases DOS INTRO MVE ownership\n");
        return 1;
    }
    puts("PASS: DM2 BootProfile owns only verified DOS INTRO MVE bytes in RAM");
    return 0;
}
