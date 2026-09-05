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
    if (receipt->invalidated) {
        receipt->receipt_hash = 0u;
        receipt->receipt_hash_hex[0] = '\0';
        return 0;
    }
    old_hash = receipt->receipt_hash;
    csb_v1_startup_real_copy(old_hex, sizeof(old_hex),
                             receipt->receipt_hash_hex);
    csb_v1_startup_real_store_hash(receipt);
    if (old_hash == 0u || old_hex[0] == '\0') {
        return receipt->receipt_hash != 0u &&
               receipt->receipt_hash_hex[0] != '\0';
    }
    if (old_hash != receipt->receipt_hash ||
        strcmp(old_hex, receipt->receipt_hash_hex) != 0) {
        receipt->invalidated = 1;
        receipt->receipt_hash = 0u;
        receipt->receipt_hash_hex[0] = '\0';
        return 0;
    }
    return 1;
}

static int csb_v1_startup_real_surface_matches_pc34(
    const CSB_V1_StartupRuntimeSurface_PC34 *surface, int asset_id,
    int width, int height, int transparent_color)
{
    return surface && surface->valid && surface->pixels &&
        surface->source_asset_id == asset_id && surface->width == width &&
        surface->height == height && surface->transparent_color == transparent_color;
}

static void csb_v1_startup_real_hash_surface_pc34(
    uint64_t *hash, const CSB_V1_StartupRuntimeSurface_PC34 *surface)
{
    size_t pixel_count;

    if (!hash || !surface || !surface->pixels || surface->width <= 0 ||
        surface->height <= 0) {
        return;
    }
    pixel_count = (size_t)surface->width * (size_t)surface->height;
    csb_v1_startup_real_hash_u64(hash, (uint64_t)surface->source_asset_id);
    csb_v1_startup_real_hash_u64(hash, (uint64_t)surface->source_x);
    csb_v1_startup_real_hash_u64(hash, (uint64_t)surface->source_y);
    csb_v1_startup_real_hash_u64(hash, (uint64_t)surface->width);
    csb_v1_startup_real_hash_u64(hash, (uint64_t)surface->height);
    csb_v1_startup_real_hash_u64(hash, (uint64_t)surface->transparent_color);
    csb_v1_startup_real_hash_bytes(hash, surface->pixels, pixel_count);
}

int csb_v1_startup_real_package_consumption_receipt_from_session_pc34(
    const CSB_V1_StartupRealReceipt *real_asset_receipt,
    const CSB_V1_StartupRuntimeAssetSession_PC34 *session,
    CSB_V1_StartupRealPackageConsumptionReceipt_PC34 *out_receipt)
{
    CSB_V1_StartupRealReceipt checked_receipt;
    const CSB_V1_StartupRuntimeSurface_PC34 *title;
    const CSB_V1_StartupRuntimeSurface_PC34 *presents;
    const CSB_V1_StartupRuntimeSurface_PC34 *chaos;
    const CSB_V1_StartupRuntimeSurface_PC34 *strikes;
    const CSB_V1_StartupRuntimeSurface_PC34 *left_door;
    const CSB_V1_StartupRuntimeSurface_PC34 *right_door;
    const CSB_V1_StartupRuntimeSurface_PC34 *entrance;
    const CSB_V1_StartupRuntimeSurface_PC34 *credits;
    const CSB_V1_StartupRuntimeSurface_PC34 *c017;
    const CSB_V1_StartupRuntimeSurface_PC34 *c040;
    CSB_V1_StartupFullRuntimeReceipt_PC34 full_runtime;
    uint64_t hash = CSB_V1_STARTUP_REAL_FNV_OFFSET;

    if (out_receipt) {
        memset(out_receipt, 0, sizeof(*out_receipt));
    }
    if (!real_asset_receipt || !session || !out_receipt) {
        return 0;
    }
    checked_receipt = *real_asset_receipt;
    if (!checked_receipt.matched || !checked_receipt.assets_verified ||
        !checked_receipt.graphics_verified || !checked_receipt.dungeon_verified ||
        checked_receipt.variant_id != CSB_V1_VARIANT_REFERENCE_I34_EN ||
        checked_receipt.graphics_kind != CSB_V1_ASSET_GFX_ARCHIVE_GRAPHICS ||
        !csb_v1_startup_real_receipt_recompute_hash(&checked_receipt) ||
        !csb_v1_boot_startup_full_runtime_receipt_from_session_pc34(
            session, &full_runtime) || !full_runtime.valid ||
        !full_runtime.real_asset_matched || !full_runtime.title_sequence_ready ||
        !full_runtime.hud_ready || !full_runtime.title_to_hud_same_session ||
        !full_runtime.no_legacy_wrappers || session->generation == 0u) {
        return 0;
    }
    /* TITLE.C F0437 has separate zoom and full-size CHAOS waits. A resident
     * C001 region alone is not evidence that the runtime consumed both. */
    if (session->playback.title_phase_mask != 0x0f) {
        return 0;
    }
    title = &session->surfaces.surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_TITLE_PC34];
    presents = &session->surfaces.surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_PRESENTS_PC34];
    chaos = &session->surfaces.surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_CHAOS_PC34];
    strikes = &session->surfaces.surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_STRIKES_BACK_PC34];
    left_door = &session->surfaces.surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_OPENING_LEFT_PC34];
    right_door = &session->surfaces.surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_OPENING_RIGHT_PC34];
    entrance = &session->surfaces.surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_ENTRANCE_SCREEN_PC34];
    credits = &session->surfaces.surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_ENTRANCE_CREDITS_PC34];
    c017 = &session->surfaces.surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_HUD_INVENTORY_PC34];
    c040 = &session->surfaces.surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_HUD_RESURRECT_PC34];
    if (!session->surfaces.valid || !session->surfaces.real_asset_matched ||
        !session->surfaces.title_regions_ready || !session->surfaces.opening_frame_ready ||
        !session->surfaces.entrance_screen_ready || !session->surfaces.hud_surfaces_ready ||
        !csb_v1_startup_real_surface_matches_pc34(title, 1, 320, 153, -1) ||
        !csb_v1_startup_real_surface_matches_pc34(presents, 1, 320, 16, -1) ||
        !csb_v1_startup_real_surface_matches_pc34(chaos, 1, 320, 80, -1) ||
        !csb_v1_startup_real_surface_matches_pc34(strikes, 1, 320, 57, 0) ||
        /* ReDMCSB ENTRANCE.C F0806 composes C002/C003 into fixed C430/C431
         * door zones. C003 is a 128x161 source bitmap clipped at its 105 px
         * destination zone, so a non-empty replacement strip is not enough. */
        !csb_v1_startup_real_surface_matches_pc34(left_door, 2, 105, 161, -1) ||
        !csb_v1_startup_real_surface_matches_pc34(right_door, 3, 128, 161, -1) ||
        !entrance->valid || !entrance->pixels || entrance->source_asset_id != 4 ||
        entrance->width != 320 || entrance->height != 200 ||
        !credits->valid || !credits->pixels || credits->source_asset_id != 5 ||
        credits->width != 320 || credits->height != 200 ||
        !csb_v1_startup_real_surface_matches_pc34(c017, 17, 224, 136, -1) ||
        (c040->valid && c040->pixels && c040->height > 0 &&
         !csb_v1_startup_real_surface_matches_pc34(c040, 40, 144, 73, 6))) {
        return 0;
    }

    csb_v1_startup_real_hash_string(&hash, "firestaff:csb:v1:pc34-package-consumption");
    csb_v1_startup_real_hash_u64(&hash, checked_receipt.receipt_hash);
    csb_v1_startup_real_hash_surface_pc34(&hash, title);
    csb_v1_startup_real_hash_surface_pc34(&hash, presents);
    csb_v1_startup_real_hash_surface_pc34(&hash, chaos);
    csb_v1_startup_real_hash_surface_pc34(&hash, strikes);
    csb_v1_startup_real_hash_surface_pc34(&hash, left_door);
    csb_v1_startup_real_hash_surface_pc34(&hash, right_door);
    csb_v1_startup_real_hash_surface_pc34(&hash, entrance);
    csb_v1_startup_real_hash_surface_pc34(&hash, credits);
    csb_v1_startup_real_hash_surface_pc34(&hash, c017);
    if (c040->valid && c040->pixels && c040->height > 0)
        csb_v1_startup_real_hash_surface_pc34(&hash, c040);
    csb_v1_startup_real_hash_u64(&hash, session->source_tick);
    csb_v1_startup_real_hash_u64(&hash, session->generation);
    out_receipt->real_package_matched = 1;
    out_receipt->c001_title_consumed = 1;
    out_receipt->c001_presents_consumed = 1;
    out_receipt->c001_chaos_consumed = 1;
    out_receipt->c001_chaos_zoom_consumed = 1;
    out_receipt->c001_chaos_hold_consumed = 1;
    out_receipt->c001_strikes_back_consumed = 1;
    out_receipt->c002_left_door_consumed = 1;
    out_receipt->c003_right_door_consumed = 1;
    out_receipt->c004_entrance_consumed = 1;
    out_receipt->c005_credits_consumed = 1;
    out_receipt->c017_hud_consumed = 1;
    out_receipt->c040_hud_consumed =
        (c040->valid && c040->pixels && c040->height > 0) ? 1 : 0;
    out_receipt->title_to_entrance_same_session = 1;
    out_receipt->title_to_hud_same_session = 1;
    out_receipt->no_legacy_wrappers = 1;
    out_receipt->no_fallback_routes = 1;
    out_receipt->source_tick = session->source_tick;
    out_receipt->session_generation = session->generation;
    out_receipt->real_asset_receipt_hash = checked_receipt.receipt_hash;
    out_receipt->consumed_surface_hash = hash ? hash : 1u;
    out_receipt->valid = 1;
    return 1;
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
