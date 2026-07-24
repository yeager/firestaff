#include "csb_v1_boot.h"

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static int expect(int condition, const char* message) {
    if (condition) return 1;
    fprintf(stderr, "FAIL: %s\n", message);
    return 0;
}

int main(void) {
    CSB_V1_BootProfile profile;
    unsigned char source[9078];
    char root[256];
    char path[512];
    FILE* file;
    int index;
    int ok = 1;

    snprintf(root, sizeof(root), "/tmp/firestaff-csb-swsh-%ld", (long)getpid());
    snprintf(path, sizeof(path), "%s/SWSHSND.DAT", root);
    (void)mkdir(root, 0700);
    for (index = 0; index < (int)sizeof(source); ++index) {
        source[index] = (unsigned char)((index * 31 + 11) & 0xff);
    }
    file = fopen(path, "wb");
    if (!file) {
        return 1;
    }
    if (fwrite(source, 1u, sizeof(source), file) != sizeof(source) ||
        fclose(file) != 0) {
        return 1;
    }

    csb_v1_boot_profile_init(&profile);
    snprintf(profile.asset_root, sizeof(profile.asset_root), "%s", root);
    profile.assets_verified = 1;
    ok &= expect(csb_v1_boot_load_swoosh_source_pc34(&profile),
                 "selected package discovers its exact raw SWSHSND payload");
    ok &= expect(profile.swoosh_source_bound &&
                     profile.swoosh_source_fnv1a != 0u &&
                     strcmp(profile.swoosh_source_path, path) == 0 &&
                     memcmp(profile.swoosh_source_bytes, source,
                            sizeof(source)) == 0,
                 "boot profile retains only the package-owned source bytes");

    file = fopen(path, "wb");
    if (!file) {
        return 1;
    }
    if (fwrite(source, 1u, sizeof(source) - 1u, file) !=
                     sizeof(source) - 1u || fclose(file) != 0) {
        return 1;
    }
    ok &= expect(!csb_v1_boot_load_swoosh_source_pc34(&profile) &&
                     !profile.swoosh_source_bound &&
                     profile.swoosh_source_fnv1a == 0u,
                 "wrong-sized package payload cannot leave stale source audio");
    (void)unlink(path);
    (void)rmdir(root);
    return ok ? 0 : 1;
}
