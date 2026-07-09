#include "csb_v1_startup_real_asset_receipt.h"

#include "csb_v1_boot.h"

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#define CSB_V1_STARTUP_REAL_FNV_OFFSET 1469598103934665603ULL
#define CSB_V1_STARTUP_REAL_FNV_PRIME 1099511628211ULL
#define CSB_V1_STARTUP_REAL_SALT "firestaff:csb:v1:startup-real"

static void csb_v1_startup_real_copy(char *dst,
                                     size_t dst_size,
                                     const char *src)
{
    if (!dst || dst_size == 0u) {
        return;
    }
    if (!src) {
        src = "";
    }
    snprintf(dst, dst_size, "%s", src);
}

static void csb_v1_startup_real_hash_bytes(uint64_t *hash,
                                           const void *data,
                                           size_t size)
{
    const unsigned char *bytes = (const unsigned char *)data;
    size_t i;
    if (!hash || (!bytes && size > 0u)) {
        return;
    }
    for (i = 0u; i < size; ++i) {
        *hash ^= (uint64_t)bytes[i];
        *hash *= CSB_V1_STARTUP_REAL_FNV_PRIME;
    }
}

static void csb_v1_startup_real_hash_string(uint64_t *hash,
                                            const char *text)
{
    if (!text) {
        text = "";
    }
    csb_v1_startup_real_hash_bytes(hash, text, strlen(text) + 1u);
}

static void csb_v1_startup_real_hash_u64(uint64_t *hash, uint64_t value)
{
    unsigned int i;
    for (i = 0u; i < 8u; ++i) {
        unsigned char byte = (unsigned char)((value >> (i * 8u)) & 0xffu);
        csb_v1_startup_real_hash_bytes(hash, &byte, 1u);
    }
}

static int csb_v1_startup_real_path_size(const char *path,
                                         uint64_t *out_size)
{
    char container_path[CSB_V1_STARTUP_REAL_PATH_CAP];
    const char *archive_sep;
    struct stat st;

    if (out_size) {
        *out_size = 0u;
    }
    if (!path || path[0] == '\0') {
        return 0;
    }
    archive_sep = strstr(path, "::");
    if (archive_sep) {
        size_t len = (size_t)(archive_sep - path);
        if (len >= sizeof(container_path)) {
            return 0;
        }
        memcpy(container_path, path, len);
        container_path[len] = '\0';
        path = container_path;
    }
    if (stat(path, &st) != 0) {
        return 0;
    }
    if (out_size) {
        *out_size = (uint64_t)st.st_size;
    }
    return 1;
}

static uint64_t csb_v1_startup_real_compute_hash(
    const CSB_V1_StartupRealReceipt *receipt)
{
    uint64_t hash = CSB_V1_STARTUP_REAL_FNV_OFFSET;
    if (!receipt || !receipt->matched) {
        return 0u;
    }
    csb_v1_startup_real_hash_string(&hash, CSB_V1_STARTUP_REAL_SALT);
    csb_v1_startup_real_hash_string(&hash, receipt->asset_root);
    csb_v1_startup_real_hash_string(&hash, receipt->graphics_path);
    csb_v1_startup_real_hash_string(&hash, receipt->dungeon_path);
    csb_v1_startup_real_hash_string(&hash, receipt->graphics_md5);
    csb_v1_startup_real_hash_string(&hash, receipt->dungeon_md5);
    csb_v1_startup_real_hash_u64(&hash, receipt->graphics_size_bytes);
    csb_v1_startup_real_hash_u64(&hash, receipt->dungeon_size_bytes);
    csb_v1_startup_real_hash_u64(&hash, (uint64_t)receipt->variant_id);
    csb_v1_startup_real_hash_u64(&hash, (uint64_t)receipt->graphics_kind);
    csb_v1_startup_real_hash_u64(&hash, (uint64_t)receipt->max_depth);
    csb_v1_startup_real_hash_u64(&hash, (uint64_t)receipt->assets_verified);
    csb_v1_startup_real_hash_u64(&hash, (uint64_t)receipt->graphics_verified);
    csb_v1_startup_real_hash_u64(&hash, (uint64_t)receipt->dungeon_verified);
    return hash ? hash : 1u;
}

static void csb_v1_startup_real_store_hash(
    CSB_V1_StartupRealReceipt *receipt)
{
    if (!receipt || !receipt->matched) {
        if (receipt) {
            receipt->receipt_hash = 0u;
            receipt->receipt_hash_hex[0] = '\0';
        }
        return;
    }
    receipt->receipt_hash = csb_v1_startup_real_compute_hash(receipt);
    snprintf(receipt->receipt_hash_hex,
             sizeof(receipt->receipt_hash_hex),
             "%016llx",
             (unsigned long long)receipt->receipt_hash);
}

void csb_v1_startup_real_receipt_init(CSB_V1_StartupRealReceipt *receipt)
{
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
    receipt->variant_id = CSB_V1_VARIANT_UNKNOWN;
    receipt->graphics_kind = CSB_V1_ASSET_GFX_ARCHIVE_NONE;
}

int csb_v1_startup_real_receipt_from_profile_fields(
    const char *asset_root,
    const char *graphics_path,
    const char *dungeon_path,
    const char *graphics_md5,
    const char *dungeon_md5,
    uint64_t graphics_size_bytes,
    uint64_t dungeon_size_bytes,
    CSB_V1_VariantId variant_id,
    CSB_V1_AssetGfxArchiveType graphics_kind,
    int max_depth,
    int assets_verified,
    int graphics_verified,
    int dungeon_verified,
    CSB_V1_StartupRealReceipt *receipt)
{
    if (!receipt) {
        return 0;
    }
    csb_v1_startup_real_receipt_init(receipt);
    receipt->max_depth = max_depth;
    csb_v1_startup_real_copy(receipt->asset_root,
                             sizeof(receipt->asset_root),
                             asset_root);
    if (!assets_verified || !graphics_verified || !dungeon_verified ||
        !graphics_path || graphics_path[0] == '\0' ||
        !dungeon_path || dungeon_path[0] == '\0' ||
        !graphics_md5 || graphics_md5[0] == '\0' ||
        !dungeon_md5 || dungeon_md5[0] == '\0') {
        return 0;
    }
    csb_v1_startup_real_copy(receipt->graphics_path,
                             sizeof(receipt->graphics_path),
                             graphics_path);
    csb_v1_startup_real_copy(receipt->dungeon_path,
                             sizeof(receipt->dungeon_path),
                             dungeon_path);
    csb_v1_startup_real_copy(receipt->graphics_md5,
                             sizeof(receipt->graphics_md5),
                             graphics_md5);
    csb_v1_startup_real_copy(receipt->dungeon_md5,
                             sizeof(receipt->dungeon_md5),
                             dungeon_md5);
    receipt->graphics_size_bytes = graphics_size_bytes;
    receipt->dungeon_size_bytes = dungeon_size_bytes;
    receipt->variant_id = variant_id;
    receipt->graphics_kind = graphics_kind;
    receipt->assets_verified = assets_verified ? 1 : 0;
    receipt->graphics_verified = graphics_verified ? 1 : 0;
    receipt->dungeon_verified = dungeon_verified ? 1 : 0;
    receipt->matched = 1;
    csb_v1_startup_real_store_hash(receipt);
    /* ReDMCSB ENTRANCE.C F0806 line 440 selects the CSB palette/media
     * path, then LOADSAVE.C F0435 line 2192 enters the dungeon load path.
     * Detach-time M11 receipts use the already verified boot profile
     * fields here, instead of repeating filename/path inference. */
    return 1;
}

int csb_v1_startup_real_scan_and_receipt(
    const char *data_dir,
    int max_depth,
    CSB_V1_StartupRealReceipt *receipt)
{
    CSB_V1_BootProfile profile;
    struct stat st;
    int scan_rc;
    uint64_t graphics_size = 0u;
    uint64_t dungeon_size = 0u;

    if (!receipt) {
        return CSB_V1_STARTUP_REAL_ERR_ARGUMENT;
    }
    csb_v1_startup_real_receipt_init(receipt);
    if (!data_dir || data_dir[0] == '\0') {
        return CSB_V1_STARTUP_REAL_ERR_NO_DATA_DIR;
    }
    if (stat(data_dir, &st) != 0 || !S_ISDIR(st.st_mode)) {
        return CSB_V1_STARTUP_REAL_ERR_NO_DATA_DIR;
    }

    csb_v1_boot_profile_init(&profile);
    scan_rc = csb_v1_boot_scan_assets(&profile, data_dir);
    receipt->max_depth = max_depth;
    csb_v1_startup_real_copy(receipt->asset_root,
                             sizeof(receipt->asset_root),
                             profile.asset_root);
    if (scan_rc != 0 && !profile.assets_verified) {
        return CSB_V1_STARTUP_REAL_OK;
    }
    if (!profile.assets_verified || !profile.graphics_verified ||
        !profile.dungeon_verified || profile.graphics_path[0] == '\0' ||
        profile.dungeon_path[0] == '\0' || profile.graphics_md5[0] == '\0' ||
        profile.dungeon_md5[0] == '\0') {
        return CSB_V1_STARTUP_REAL_ERR_BOOT_SCAN;
    }
    if (!csb_v1_startup_real_path_size(profile.graphics_path,
                                       &graphics_size) ||
        !csb_v1_startup_real_path_size(profile.dungeon_path,
                                       &dungeon_size)) {
        return CSB_V1_STARTUP_REAL_ERR_BOOT_SCAN;
    }

    (void)csb_v1_startup_real_receipt_from_profile_fields(
        profile.asset_root,
        profile.graphics_path,
        profile.dungeon_path,
        profile.graphics_md5,
        profile.dungeon_md5,
        graphics_size,
        dungeon_size,
        profile.variant_id,
        profile.graphics_kind,
        max_depth,
        profile.assets_verified,
        profile.graphics_verified,
        profile.dungeon_verified,
        receipt);
    /* ReDMCSB ENTRANCE.C F0806 lines 409-441 selects the CSB media path
     * before LOADSAVE.C F0435 loads map 0. This receipt packages that
     * hash-verified startup capture so M11/probes do not infer it from
     * filenames or loose launcher state. */
    return CSB_V1_STARTUP_REAL_OK;
}

int csb_v1_startup_real_receipt_recompute_hash(
    CSB_V1_StartupRealReceipt *receipt)
{
    uint64_t old_hash;
    char old_hex[CSB_V1_STARTUP_REAL_HASH_HEX_CAP];

    if (!receipt) {
        return 0;
    }
    if (!receipt->matched) {
        receipt->receipt_hash = 0u;
        receipt->receipt_hash_hex[0] = '\0';
        return 1;
    }
    old_hash = receipt->receipt_hash;
    csb_v1_startup_real_copy(old_hex, sizeof(old_hex),
                             receipt->receipt_hash_hex);
    csb_v1_startup_real_store_hash(receipt);
    if (old_hash == 0u || old_hex[0] == '\0') {
        return receipt->receipt_hash != 0u &&
               receipt->receipt_hash_hex[0] != '\0';
    }
    return old_hash == receipt->receipt_hash &&
           strcmp(old_hex, receipt->receipt_hash_hex) == 0;
}

const char *csb_v1_startup_real_result_name(int result)
{
    switch (result) {
        case CSB_V1_STARTUP_REAL_OK:
            return "CSB_V1_STARTUP_REAL_OK";
        case CSB_V1_STARTUP_REAL_ERR_ARGUMENT:
            return "CSB_V1_STARTUP_REAL_ERR_ARGUMENT";
        case CSB_V1_STARTUP_REAL_ERR_NO_DATA_DIR:
            return "CSB_V1_STARTUP_REAL_ERR_NO_DATA_DIR";
        case CSB_V1_STARTUP_REAL_ERR_BOOT_SCAN:
            return "CSB_V1_STARTUP_REAL_ERR_BOOT_SCAN";
        default:
            return "CSB_V1_STARTUP_REAL_ERR_UNKNOWN";
    }
}
