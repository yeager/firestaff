#define _POSIX_C_SOURCE 200809L

#include "nexus_v1_slev_sal_asset_discovery.h"

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static int failures;

#define CHECK(condition, message) do { \
    if (!(condition)) { \
        fprintf(stderr, "FAIL: %s\n", message); \
        ++failures; \
    } \
} while (0)

static const char *const md5s[16] = {
    "23ca472302f49b3ea5592b146a312da0", "458cea6e6d953307cac11533c2f4d03d",
    "0f5be288a5f008ab2ecd84af048edb0a", "f8d9024e1efc5c82b039694d66704264",
    "4e42d6a2302a9c53ae84b080d6c714db", "527848b310b793e41e0616f38ee85059",
    "c005d21e6f74e5515ab798ff251bcb88", "a22f54f8f1fb7a1f0cae2ce9b677d5f1",
    "608796394f9a5e7a23e4dc0e432845de", "62ef56bd00f791447b5a55b5373db2e1",
    "e5e0766271462dc6afd0ec2534382bf0", "339f18db4f791a1fab73ff9a1c167669",
    "6ecd1f1b606a74eecd1f9451923f46ff", "f4d0803271994194972edb3928efd4dd",
    "3f349bd7f413433357d94acbf19688d1", "5be3971b6ea34a76dd15daf44fbbcf1f"
};

static int write_identity(const char *dir, const char *name, char byte)
{
    char path[ASSET_PATH_MAX];
    char bytes[16];
    FILE *file;
    memset(bytes, byte, sizeof(bytes));
    if (snprintf(path, sizeof(path), "%s/%s", dir, name) >= (int)sizeof(path) ||
        !(file = fopen(path, "wb"))) return 0;
    return fwrite(bytes, 1U, sizeof(bytes), file) == sizeof(bytes) &&
        fclose(file) == 0;
}

int main(void)
{
    Nexus_V1_SlevSalExpectedLevel expected[16];
    Nexus_V1_SlevSalAssetDiscoveryReceipt receipt;
    char slev[16][16], sal[16][16], map[16][16];
    char directory[128], path[ASSET_PATH_MAX];
    uint32_t level;

    memset(&receipt, 0, sizeof(receipt));
    CHECK(nexus_v1_slev_sal_assets_discover_direct(NULL, &receipt) == 0 &&
          receipt.skipped_missing_assets && receipt.direct_files_only &&
          !receipt.payload_materialized,
          "production SLEV/SAL discovery skips safely without retail files");
    CHECK(snprintf(directory, sizeof(directory), "/tmp/firestaff-nexus-slev-sal-%ld",
#ifdef _WIN32
                   (long)getpid()) > 0 && mkdir(directory) == 0,
#else
                   (long)getpid()) > 0 && mkdir(directory, 0700) == 0,
#endif
          "temporary direct asset directory exists");
    for (level = 0U; level < 16U; ++level) {
        snprintf(slev[level], sizeof(slev[level]), "SLEV%02u.BIN", level);
        snprintf(sal[level], sizeof(sal[level]), "SNDLEV%02u.SAL", level);
        snprintf(map[level], sizeof(map[level]), "SNDLEV%02u.MAP", level);
        expected[level].slev_name = slev[level]; expected[level].slev_md5 = md5s[level];
        expected[level].sal_name = sal[level]; expected[level].sal_md5 = md5s[level];
        expected[level].map_name = map[level]; expected[level].map_md5 = md5s[level];
        CHECK(write_identity(directory, slev[level], (char)('a' + level)) &&
              write_identity(directory, sal[level], (char)('a' + level)) &&
              write_identity(directory, map[level], (char)('a' + level)),
              "direct SLEV/SAL/MAP fixture writes canonical identity bytes");
    }
    CHECK(write_identity(directory, "SDDRVS.TSK", 'a'),
          "direct SDDRVS fixture writes canonical identity bytes");
    memset(&receipt, 0, sizeof(receipt));
    CHECK(failures == 0 && nexus_v1_slev_sal_assets_discover_direct_expected(
              directory, expected, 16U, "SDDRVS.TSK", md5s[0], &receipt) == 1 &&
          receipt.valid && receipt.direct_files_only && !receipt.payload_materialized &&
          !receipt.rejected_mixed_assets && receipt.levels[7].valid &&
          receipt.levels[7].slev.byte_count == 16U &&
          strstr(receipt.levels[7].sal.direct_path, "::") == NULL &&
          receipt.sound_driver.valid &&
          nexus_v1_slev_sal_direct_identity_still_matches(
              &receipt.sound_driver) &&
          nexus_v1_slev_sal_level_identities_still_match(&receipt, 3U),
          "complete direct identities produce a bounded no-payload receipt");
    receipt.levels[3].sal.fnv1a64 ^= UINT64_C(1);
    CHECK(!nexus_v1_slev_sal_level_identities_still_match(&receipt, 3U),
          "stale retained SAL FNV invalidates the level identity at consumption");
    receipt.levels[3].sal.fnv1a64 ^= UINT64_C(1);
    CHECK(write_identity(directory, "SDDRVS.TSK", 'z') &&
          !nexus_v1_slev_sal_direct_identity_still_matches(
              &receipt.sound_driver),
          "mutated SDDRVS invalidates a previously accepted direct identity");
    CHECK(write_identity(directory, "SDDRVS.TSK", 'a') &&
          nexus_v1_slev_sal_direct_identity_still_matches(
              &receipt.sound_driver),
          "restored SDDRVS can be reverified without retaining its bytes");
    CHECK(snprintf(path, sizeof(path), "%s/SDDRVS.TSK", directory) > 0 &&
          unlink(path) == 0 &&
          !nexus_v1_slev_sal_direct_identity_still_matches(
              &receipt.sound_driver),
          "missing SDDRVS invalidates the direct identity before launch");
    CHECK(write_identity(directory, "SDDRVS.TSK", 'a'),
          "SDDRVS fixture is restored for mixed and missing corpus coverage");
    CHECK(snprintf(path, sizeof(path), "%s/SNDLEV03.SAL", directory) > 0 &&
          write_identity(directory, "SNDLEV03.SAL", 'z'),
          "mutated direct SAL fixture replaces one asset");
    CHECK(!nexus_v1_slev_sal_level_identities_still_match(&receipt, 3U),
          "mutated SAL invalidates the active SLEV/SAL level identity");
    memset(&receipt, 0, sizeof(receipt));
    CHECK(nexus_v1_slev_sal_assets_discover_direct_expected(
              directory, expected, 16U, "SDDRVS.TSK", md5s[0], &receipt) == 0 &&
          !receipt.valid && receipt.rejected_mixed_assets &&
          !receipt.payload_materialized,
          "mutated SAL rejects a mixed release without retaining bytes");
    CHECK(write_identity(directory, "SNDLEV03.SAL", 'd'),
          "mutated SAL fixture is restored before missing-source coverage");
    CHECK(snprintf(path, sizeof(path), "%s/SLEV15.BIN", directory) > 0 &&
          unlink(path) == 0,
          "one direct SLEV fixture is removed");
    CHECK(!nexus_v1_slev_sal_level_identities_still_match(&receipt, 15U),
          "missing SLEV invalidates the level identity before dungeon admission");
    memset(&receipt, 0, sizeof(receipt));
    CHECK(nexus_v1_slev_sal_assets_discover_direct_expected(
              directory, expected, 16U, "SDDRVS.TSK", md5s[0], &receipt) == 0 &&
          !receipt.valid && receipt.skipped_missing_assets,
          "missing direct source skips safely");
    for (level = 0U; level < 16U; ++level) {
        snprintf(path, sizeof(path), "%s/%s", directory, slev[level]); unlink(path);
        snprintf(path, sizeof(path), "%s/%s", directory, sal[level]); unlink(path);
        snprintf(path, sizeof(path), "%s/%s", directory, map[level]); unlink(path);
    }
    snprintf(path, sizeof(path), "%s/SDDRVS.TSK", directory); unlink(path);
    rmdir(directory);
    if (failures) return 1;
    puts("test_nexus_v1_slev_sal_asset_discovery: PASS");
    return 0;
}
