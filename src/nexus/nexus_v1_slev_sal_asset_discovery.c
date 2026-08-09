#include "nexus_v1_slev_sal_asset_discovery.h"

#include "nexus_v1_engine.h"

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

static uint64_t fnv1a64_update(uint64_t hash, const uint8_t *bytes, size_t size)
{
    size_t i;
    for (i = 0U; i < size; ++i) {
        hash ^= bytes[i];
        hash *= UINT64_C(1099511628211);
    }
    return hash;
}

static int direct_identity(const char *data_dir, const char *name,
                           const char *md5,
                           Nexus_V1_SlevSalDirectIdentity *out_identity,
                           int *out_missing, int *out_mixed)
{
    Nexus_V1_SlevSalDirectIdentity identity;
    uint8_t buffer[4096];
    uint64_t hash = UINT64_C(1469598103934665603);
    uint64_t total = 0U;
    char path[ASSET_PATH_MAX];
    FILE *file;
    struct stat st;
    size_t count;

    if (!out_identity || !out_missing || !out_mixed || !data_dir || !data_dir[0] ||
        !name || !md5 || strlen(md5) != 32U ||
        snprintf(path, sizeof(path), "%s/%s", data_dir, name) >=
            (int)sizeof(path)) return 0;
    memset(&identity, 0, sizeof(identity));
    if (stat(path, &st) != 0 || !S_ISREG(st.st_mode)) {
        *out_missing = 1;
        return 0;
    }
    if (!asset_file_matches_md5(path, md5)) {
        *out_mixed = 1;
        return 0;
    }
    if (!(file = fopen(path, "rb"))) return 0;
    while ((count = fread(buffer, 1U, sizeof(buffer), file)) != 0U) {
        if (UINT64_MAX - total < count) {
            fclose(file);
            return 0;
        }
        total += count;
        hash = fnv1a64_update(hash, buffer, count);
    }
    /* Close unconditionally; a read error must not short-circuit past it and
     * leak a descriptor per scanned corpus file. */
    {
        int io_failed = ferror(file) != 0;
        if (fclose(file) != 0) io_failed = 1;
        if (io_failed || !total || !hash) return 0;
    }
    identity.valid = 1;
    snprintf(identity.direct_path, sizeof(identity.direct_path), "%s", path);
    snprintf(identity.canonical_name, sizeof(identity.canonical_name), "%s", name);
    snprintf(identity.md5, sizeof(identity.md5), "%s", md5);
    identity.byte_count = total;
    identity.fnv1a64 = hash;
    *out_identity = identity;
    return 1;
}

int nexus_v1_slev_sal_assets_discover_direct_expected(
    const char *data_dir,
    const Nexus_V1_SlevSalExpectedLevel *expected_levels,
    uint32_t expected_count, const char *driver_name, const char *driver_md5,
    Nexus_V1_SlevSalAssetDiscoveryReceipt *out_receipt)
{
    Nexus_V1_SlevSalAssetDiscoveryReceipt receipt;
    uint32_t level;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    receipt.direct_files_only = 1;
    if (!data_dir || !data_dir[0]) {
        receipt.skipped_missing_assets = 1;
        *out_receipt = receipt;
        return 0;
    }
    if (!expected_levels || expected_count != NEXUS_V1_SLEV_SAL_ASSET_LEVEL_COUNT ||
        !driver_name || !driver_md5) {
        *out_receipt = receipt;
        return 0;
    }
    for (level = 0U; level < expected_count; ++level) {
        Nexus_V1_SlevSalDirectLevelIdentity *entry = &receipt.levels[level];
        const Nexus_V1_SlevSalExpectedLevel *expected = &expected_levels[level];
        int missing = 0;
        int mixed = 0;

        if (!direct_identity(data_dir, expected->slev_name, expected->slev_md5,
                             &entry->slev, &missing, &mixed) ||
            !direct_identity(data_dir, expected->sal_name, expected->sal_md5,
                             &entry->sal, &missing, &mixed) ||
            !direct_identity(data_dir, expected->map_name, expected->map_md5,
                             &entry->map, &missing, &mixed)) {
            receipt.skipped_missing_assets = missing;
            receipt.rejected_mixed_assets = mixed;
            *out_receipt = receipt;
            return 0;
        }
        entry->valid = 1;
        entry->level_index = level;
        receipt.corpus_byte_count += entry->slev.byte_count + entry->sal.byte_count +
            entry->map.byte_count;
        receipt.corpus_fnv1a64 = fnv1a64_update(
            receipt.corpus_fnv1a64 ? receipt.corpus_fnv1a64 :
                UINT64_C(1469598103934665603),
            (const uint8_t *)&entry->slev.fnv1a64, sizeof(entry->slev.fnv1a64));
        receipt.corpus_fnv1a64 = fnv1a64_update(receipt.corpus_fnv1a64,
            (const uint8_t *)&entry->sal.fnv1a64, sizeof(entry->sal.fnv1a64));
        receipt.corpus_fnv1a64 = fnv1a64_update(receipt.corpus_fnv1a64,
            (const uint8_t *)&entry->map.fnv1a64, sizeof(entry->map.fnv1a64));
    }
    {
        int missing = 0;
        int mixed = 0;
        if (!direct_identity(data_dir, driver_name, driver_md5,
                             &receipt.sound_driver, &missing, &mixed)) {
            receipt.skipped_missing_assets = missing;
            receipt.rejected_mixed_assets = mixed;
            *out_receipt = receipt;
            return 0;
        }
    }
    receipt.corpus_byte_count += receipt.sound_driver.byte_count;
    receipt.corpus_fnv1a64 = fnv1a64_update(receipt.corpus_fnv1a64,
        (const uint8_t *)&receipt.sound_driver.fnv1a64,
        sizeof(receipt.sound_driver.fnv1a64));
    if (!receipt.corpus_byte_count || !receipt.corpus_fnv1a64) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.valid = 1;
    *out_receipt = receipt;
    return 1;
}

int nexus_v1_slev_sal_assets_discover_direct(
    const char *data_dir, Nexus_V1_SlevSalAssetDiscoveryReceipt *out_receipt)
{
    Nexus_V1_SlevSalExpectedLevel expected[NEXUS_V1_SLEV_SAL_ASSET_LEVEL_COUNT];
    char slev_names[NEXUS_V1_SLEV_SAL_ASSET_LEVEL_COUNT][16];
    char sal_names[NEXUS_V1_SLEV_SAL_ASSET_LEVEL_COUNT][16];
    char map_names[NEXUS_V1_SLEV_SAL_ASSET_LEVEL_COUNT][16];
    uint32_t level;

    for (level = 0U; level < NEXUS_V1_SLEV_SAL_ASSET_LEVEL_COUNT; ++level) {
        snprintf(slev_names[level], sizeof(slev_names[level]), "SLEV%02u.BIN", level);
        snprintf(sal_names[level], sizeof(sal_names[level]), "SNDLEV%02u.SAL", level);
        snprintf(map_names[level], sizeof(map_names[level]), "SNDLEV%02u.MAP", level);
        expected[level].slev_name = slev_names[level];
        expected[level].slev_md5 = nexus_v1_known_file_md5(slev_names[level]);
        expected[level].sal_name = sal_names[level];
        expected[level].sal_md5 = nexus_v1_known_file_md5(sal_names[level]);
        expected[level].map_name = map_names[level];
        expected[level].map_md5 = nexus_v1_known_file_md5(map_names[level]);
        if (!expected[level].slev_md5 || !expected[level].sal_md5 ||
            !expected[level].map_md5) {
            if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
            return 0;
        }
    }
    return nexus_v1_slev_sal_assets_discover_direct_expected(
        data_dir, expected, NEXUS_V1_SLEV_SAL_ASSET_LEVEL_COUNT,
        "SDDRVS.TSK", nexus_v1_known_file_md5("SDDRVS.TSK"), out_receipt);
}

int nexus_v1_slev_sal_direct_identity_still_matches(
    const Nexus_V1_SlevSalDirectIdentity *identity)
{
    if (!identity || !identity->valid || !identity->direct_path[0] ||
        strstr(identity->direct_path, "::") || !identity->canonical_name[0] ||
        !identity->md5[0] || !identity->byte_count || !identity->fnv1a64 ||
        strlen(identity->md5) != 32U) return 0;
    {
        uint8_t buffer[4096];
        uint64_t hash = UINT64_C(1469598103934665603);
        uint64_t total = 0U;
        FILE *file;
        struct stat st;
        size_t count;

        if (!identity || !identity->valid || !identity->direct_path[0] ||
            strstr(identity->direct_path, "::") ||
            stat(identity->direct_path, &st) != 0 || !S_ISREG(st.st_mode) ||
            !asset_file_matches_md5(identity->direct_path, identity->md5) ||
            !(file = fopen(identity->direct_path, "rb"))) return 0;
        while ((count = fread(buffer, 1U, sizeof(buffer), file)) != 0U) {
            if (UINT64_MAX - total < count) {
                fclose(file);
                return 0;
            }
            total += count;
            hash = fnv1a64_update(hash, buffer, count);
        }
        {
            int io_failed = ferror(file) != 0;
            if (fclose(file) != 0) io_failed = 1;
            if (io_failed) return 0;
        }
        return total == identity->byte_count && hash == identity->fnv1a64;
    }
}

int nexus_v1_slev_sal_level_identities_still_match(
    const Nexus_V1_SlevSalAssetDiscoveryReceipt *receipt, uint32_t level_index)
{
    const Nexus_V1_SlevSalDirectLevelIdentity *level;

    if (!receipt || !receipt->valid || receipt->payload_materialized ||
        !receipt->direct_files_only ||
        level_index >= NEXUS_V1_SLEV_SAL_ASSET_LEVEL_COUNT) return 0;
    level = &receipt->levels[level_index];
    return level->valid &&
        nexus_v1_slev_sal_direct_identity_still_matches(&level->slev) &&
        nexus_v1_slev_sal_direct_identity_still_matches(&level->sal) &&
        nexus_v1_slev_sal_direct_identity_still_matches(&level->map) &&
        nexus_v1_slev_sal_direct_identity_still_matches(&receipt->sound_driver);
}
