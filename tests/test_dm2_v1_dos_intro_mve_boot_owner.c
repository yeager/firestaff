#include "dm2_v1_boot.h"
#include "dm2_v1_dos_real_data_manifest.h"
#include "dm2_v1_mve_stream.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    const char *data_root = getenv("FIRESTAFF_DM2_DATA_DIR");
    const dm2_v1_dos_file_fp_t *intro =
        dm2_v1_dos_file_fp_lookup_pc34("intro");
    DM2_V1_BootProfile profile;
    DM2_V1_MveStreamReceipt stream;
    const uint8_t *bytes = NULL;
    size_t byte_count = 0u;

    assert(intro != NULL);
    if (!data_root || !data_root[0]) {
        puts("SKIP: no DM2 DOS DATA root");
        return 0;
    }
    dm2_v1_boot_profile_init(&profile);
    assert(dm2_v1_boot_scan_assets(&profile, data_root) == 0);
    assert(profile.assets_verified && profile.platform == DM2_PLATFORM_PC_EN);
    assert(profile.dos_startup_media_verified &&
           profile.dos_startup_media.valid &&
           profile.dos_startup_media.intro_verified &&
           profile.dos_startup_media.intro_has_interplay_mve);
    if (!profile.dos_intro_mve_owner) {
        fprintf(stderr, "missing owner: header=%u receipt=%08x\n",
                profile.dos_startup_media.intro_mve_header_offset,
                profile.dos_startup_media.receipt_hash);
    }
    assert(dm2_v1_boot_dos_intro_mve_readonly(&profile, &bytes,
                                               &byte_count) == 1);
    assert(bytes != NULL && byte_count == intro->size_bytes);
    assert(dm2_v1_mve_stream_parse(bytes, byte_count, &stream) == 1);
    assert(stream.valid &&
           stream.mve_offset == profile.dos_startup_media.intro_mve_header_offset &&
           stream.width == 320u && stream.height == 200u &&
           stream.display_count == 217u && stream.audio_frame_count == 217u);

    dm2_v1_boot_cleanup(&profile);
    assert(dm2_v1_boot_dos_intro_mve_readonly(&profile, &bytes,
                                               &byte_count) == 0);
    assert(bytes == NULL && byte_count == 0u);
    puts("PASS: DM2 BootProfile owns only verified DOS INTRO MVE bytes in RAM");
    return 0;
}
