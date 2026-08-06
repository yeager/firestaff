#include "csb_v1_boot.h"
#include "csb_v1_fmtowns_game.h"
#include "csb_v1_fmtowns_graphics_dat.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int passed;
static int failed;

#define CHECK(cond, msg) do { \
    if (cond) { passed++; printf("  PASS: %s\n", msg); } \
    else { failed++; printf("  FAIL: %s\n", msg); } \
} while (0)

static int write_synthetic_dungeon_fixture(const char *path)
{
    static const uint8_t fixture[] = {
        0x01, 0x00, 0x10, 0x00, /* one level, 16 thing types */
        0x02, 0x02, 0x0A, 0x00, 0x00, 0x00, /* 2x2 level at offset 10 */
        0x00, 0x00, 0x01, 0x00, /* x=0 y=0..1 */
        0x02, 0x00, 0x03, 0x00  /* x=1 y=0..1 */
    };
    FILE *f = fopen(path, "wb");
    if (!f) return 0;
    if (fwrite(fixture, 1U, sizeof(fixture), f) != sizeof(fixture)) {
        fclose(f);
        return 0;
    }
    return fclose(f) == 0;
}

static void test_defaults(void)
{
    CSB_V1_BootProfile p;
    size_t skin_def_word_count = 123u;

    csb_v1_boot_profile_init(&p);
    CHECK(strcmp(p.game_id, "csb") == 0, "game id is csb");
    CHECK(p.state == CSB_V1_BOOT_STATE_PROFILE_READY, "default state is PROFILE_READY");
    CHECK(p.tick_ms == CSB_V1_TICK_MS_NOMINAL, "tick is the V1 55 ms quantum");
    CHECK(p.entrance_map_index == 255U, "entrance map is C255_MAP_INDEX_ENTRANCE");
    CHECK(p.start_map_index == 0U, "new-game map index is 0");
    CHECK(p.default_party_x == CSB_V1_START_PARTY_X, "default party x follows CSB runtime profile");
    CHECK(p.default_party_y == CSB_V1_START_PARTY_Y, "default party y follows CSB runtime profile");
    CHECK(p.default_party_dir == CSB_V1_START_PARTY_DIR, "default party dir follows CSB runtime profile");
    CHECK(p.csbgraphics_skin_def_loaded == 0 &&
          p.csbgraphics_skin_def_word_count == 0u,
          "CSBgraphics skin-def ownership starts empty");
    CHECK(csb_v1_boot_csbgraphics_skin_def_words(&p,
                                                 &skin_def_word_count) == NULL &&
          skin_def_word_count == 0u,
          "CSBgraphics skin-def accessor rejects an empty boot profile");
}

static void test_scan_missing_data(void)
{
    CSB_V1_BootProfile p;
    char diag[1024];
    size_t n;

    csb_v1_boot_profile_init(&p);
    CHECK(csb_v1_boot_scan_assets(&p, "/tmp/firestaff-csb-v1-no-assets") == -1,
          "missing data does not verify");
    CHECK(p.assets_verified == 0, "assets_verified remains false");
    CHECK(p.graphics_verified == 0, "graphics_verified remains false");
    CHECK(p.dungeon_verified == 0, "dungeon_verified remains false");
    CHECK(p.state == CSB_V1_BOOT_STATE_PROFILE_READY,
          "missing data leaves profile at PROFILE_READY");
    CHECK(csb_v1_boot_probe_available("/tmp/firestaff-csb-v1-no-assets") == 0,
          "probe_available is false without both CSB assets");
    n = csb_v1_boot_diagnostic_report(&p, diag, sizeof(diag));
    CHECK(n > 0U && strstr(diag, "CSB V1 Boot Profile") != NULL,
          "diagnostic report is populated");
}

static void test_rescan_missing_data_clears_stale_handoff(void)
{
    CSB_V1_BootProfile p;

    csb_v1_boot_profile_init(&p);
    p.assets_verified = 1;
    p.graphics_verified = 1;
    p.dungeon_verified = 1;
    p.state = CSB_V1_BOOT_STATE_RUNTIME_READY;
    p.variant_id = CSB_V1_VARIANT_PC34_EN;
    p.graphics_kind = CSB_V1_ASSET_GFX_ARCHIVE_CSBGRAF;
    snprintf(p.graphics_path, sizeof(p.graphics_path),
             "%s", "/tmp/firestaff-csb-stale/CSBGRAPH.DAT");
    snprintf(p.dungeon_path, sizeof(p.dungeon_path),
             "%s", "/tmp/firestaff-csb-stale/DUNGEON.DAT");
    snprintf(p.graphics_md5, sizeof(p.graphics_md5),
             "%s", "61fbfd56887c94adc26888a9491c6611");
    snprintf(p.dungeon_md5, sizeof(p.dungeon_md5),
             "%s", "6695d2acebce49f95db1d8f3a5c733de");

    CHECK(csb_v1_boot_scan_assets(&p, "/tmp/firestaff-csb-v1-rescan-no-assets") == -1,
          "missing-data rescan fails on a reused profile");
    CHECK(p.assets_verified == 0 &&
          p.graphics_verified == 0 &&
          p.dungeon_verified == 0,
          "missing-data rescan clears all verification flags");
    CHECK(p.graphics_path[0] == '\0' &&
          p.dungeon_path[0] == '\0',
          "missing-data rescan clears stale runtime asset paths");
    CHECK(p.graphics_md5[0] == '\0' &&
          p.dungeon_md5[0] == '\0',
          "missing-data rescan clears stale hash labels");
    CHECK(p.graphics_kind == CSB_V1_ASSET_GFX_ARCHIVE_NONE,
          "missing-data rescan clears stale graphics archive kind");
    CHECK(p.variant_id == CSB_V1_VARIANT_UNKNOWN,
          "missing-data rescan clears stale CSB variant id");
    CHECK(p.state == CSB_V1_BOOT_STATE_PROFILE_READY,
          "missing-data rescan returns the profile to PROFILE_READY");
    CHECK(csb_v1_boot_enter_game(&p) == -1,
          "launch remains blocked after stale handoff fields are cleared");
}

static void test_save_root_override(void)
{
    CSB_V1_BootProfile p;
    csb_v1_boot_profile_init(&p);
    csb_v1_boot_set_save_root(&p, "/tmp/firestaff-csb-saves");
    CHECK(strcmp(p.save_root, "/tmp/firestaff-csb-saves") == 0,
          "explicit save root is preserved");
}

static void test_enter_requires_assets(void)
{
    CSB_V1_BootProfile p;
    csb_v1_boot_profile_init(&p);
    CHECK(csb_v1_boot_enter_game(&p) == -1,
          "enter_game rejects an unverified profile");
    CHECK(p.state == CSB_V1_BOOT_STATE_PROFILE_READY,
          "failed enter_game leaves state unchanged");
}

static void test_m11_entry_rehash_rejects_changed_media(void)
{
    CSB_V1_BootProfile p;
    const char *graphics_path = "/tmp/firestaff-csb-m11-rehash-graphics.dat";
    const char *dungeon_path = "/tmp/firestaff-csb-m11-rehash-dungeon.dat";
    char reason[256];
    FILE *graphics;
    FILE *dungeon;

    remove(graphics_path);
    remove(dungeon_path);
    graphics = fopen(graphics_path, "wb");
    dungeon = fopen(dungeon_path, "wb");
    CHECK(graphics != NULL && dungeon != NULL,
          "M11 entry rehash fixtures open");
    if (!graphics || !dungeon) {
        if (graphics) fclose(graphics);
        if (dungeon) fclose(dungeon);
        remove(graphics_path);
        remove(dungeon_path);
        return;
    }
    (void)fputc(0x47, graphics);
    (void)fputc(0x44, dungeon);
    fclose(graphics);
    fclose(dungeon);

    csb_v1_boot_profile_init(&p);
    p.assets_verified = 1;
    p.graphics_verified = 1;
    p.dungeon_verified = 1;
    p.state = CSB_V1_BOOT_STATE_ASSETS_READY;
    snprintf(p.graphics_path, sizeof(p.graphics_path), "%s", graphics_path);
    snprintf(p.dungeon_path, sizeof(p.dungeon_path), "%s", dungeon_path);
    snprintf(p.graphics_md5, sizeof(p.graphics_md5), "%s",
             "61fbfd56887c94adc26888a9491c6611");
    snprintf(p.dungeon_md5, sizeof(p.dungeon_md5), "%s",
             "6695d2acebce49f95db1d8f3a5c733de");
    CHECK(csb_v1_boot_profile_m11_entry_gate(&p, reason, sizeof(reason)) == 0 &&
              strstr(reason, "GRAPHICS bytes changed after scan") != NULL,
          "M11 entry rehash rejects a stale graphics receipt");
    remove(graphics_path);
    remove(dungeon_path);
}

static void test_enter_handoff_state(void)
{
    CSB_V1_BootProfile p;
    csb_v1_boot_profile_init(&p);

    p.assets_verified = 1;
    p.graphics_verified = 1;
    p.dungeon_verified = 1;
    p.state = CSB_V1_BOOT_STATE_ASSETS_READY;
    p.variant_id = CSB_V1_VARIANT_PC34_EN;
    p.graphics_kind = CSB_V1_ASSET_GFX_ARCHIVE_CSBGRAF;
    snprintf(p.asset_root, sizeof(p.asset_root), "%s", "/tmp/firestaff-csb-v1-data");
    snprintf(p.graphics_path, sizeof(p.graphics_path), "%s", "/tmp/firestaff-csb-v1-data/CSBGRAPH.DAT");
    snprintf(p.dungeon_path, sizeof(p.dungeon_path), "%s", "/tmp/firestaff-csb-v1-data/DUNGEON.DAT");
    csb_v1_boot_set_save_root(&p, "/tmp/firestaff-csb-v1-saves");

    CHECK(csb_v1_boot_enter_game(&p) == -1,
          "enter_game rejects a verified profile whose dungeon is not materialized");
    CHECK(p.state == CSB_V1_BOOT_STATE_ASSETS_READY,
          "failed materialization leaves the boot profile at ASSETS_READY");
    CHECK(p.runtime.state == CSB_STATE_TITLE,
          "runtime starts at the CSB title/entrance state");
    CHECK(p.runtime.variant_id == CSB_V1_VARIANT_PC34_EN,
          "runtime receives the verified CSB variant");
    CHECK(p.runtime.difficulty == CSB_V1_DIFFICULTY_UNBOUND,
          "failed runtime handoff leaves map difficulty source-unbound");
    CHECK(p.runtime.data_dir == p.asset_root,
          "runtime data_dir points at the verified asset root");
    CHECK(p.runtime.save_dir == p.save_root,
          "runtime save_dir carries the boot save-root override");
    CHECK(p.runtime.dungeon_path == p.dungeon_path,
          "runtime dungeon_path points at the verified dungeon path");
    CHECK(p.runtime.graphics_path == p.graphics_path,
          "runtime graphics_path points at the verified graphics path");
    CHECK(p.runtime.dungeon_asset.path == p.dungeon_path,
          "runtime dungeon asset carries the verified path");
    CHECK(p.runtime.graphics_asset.path == p.graphics_path,
          "runtime graphics asset carries the verified path");
    CHECK(p.runtime.graphics_asset.kind == CSB_V1_ASSET_GFX_ARCHIVE_CSBGRAF,
          "runtime graphics asset carries the verified archive kind");
    CHECK(p.runtime.party_x == CSB_V1_START_PARTY_X &&
          p.runtime.party_y == CSB_V1_START_PARTY_Y &&
          p.runtime.party_dir == CSB_V1_START_PARTY_DIR,
          "runtime keeps the source-locked CSB start position");
    CHECK(p.runtime.chaos_magic.magic_initialized == 1 &&
          p.runtime.chaos_magic.spell_grid_version == 0U &&
          p.runtime.chaos_magic.chaos_level == 0U,
          "runtime initializes CSB chaos magic at handoff");
    csb_v1_boot_cleanup(&p);
}

static void test_enter_loads_verified_dungeon_context(void)
{
    CSB_V1_BootProfile p;
    const CSB_V1_DungeonData *current;
    const char *fixture_path = "firestaff_csb_v1_handoff_fixture.dat";

    remove(fixture_path);
    CHECK(write_synthetic_dungeon_fixture(fixture_path),
          "synthetic dungeon fixture is written");

    csb_v1_boot_profile_init(&p);
    p.assets_verified = 1;
    p.graphics_verified = 1;
    p.dungeon_verified = 1;
    p.state = CSB_V1_BOOT_STATE_ASSETS_READY;
    p.variant_id = CSB_V1_VARIANT_PC34_EN;
    p.graphics_kind = CSB_V1_ASSET_GFX_ARCHIVE_GRAPHICS;
    snprintf(p.asset_root, sizeof(p.asset_root), "%s", ".");
    snprintf(p.graphics_path, sizeof(p.graphics_path), "%s", "GRAPHICS.DAT");
    snprintf(p.dungeon_path, sizeof(p.dungeon_path), "%s", fixture_path);

    CHECK(csb_v1_boot_enter_game(&p) == -1,
          "enter_game rejects the legacy synthetic file-backed fixture");
    current = csb_v1_dungeon_get_current();
    CHECK(p.runtime.dungeon_handle == NULL && current == NULL,
          "rejected fixture publishes no runtime-owned dungeon handle");
    CHECK(p.state == CSB_V1_BOOT_STATE_ASSETS_READY,
          "rejected fixture keeps the profile outside RUNTIME_READY");

    csb_v1_boot_cleanup(&p);
    CHECK(csb_v1_dungeon_get_current() == NULL,
          "boot cleanup clears the current dungeon context");
    CHECK(p.state == CSB_V1_BOOT_STATE_PROFILE_READY,
          "boot cleanup returns profile to PROFILE_READY");
    remove(fixture_path);
}

static void test_source_evidence(void)
{
    const char *e = csb_v1_boot_source_evidence();
    CHECK(e && strstr(e, "ENTRANCE.C F0806") != NULL,
          "source evidence cites ENTRANCE.C F0806");
    CHECK(e && strstr(e, "LOADSAVE.C F0435") != NULL,
          "source evidence cites LOADSAVE.C F0435");
}

static void test_fmtowns_media_registry(void)
{
    const CSB_V1_VariantInfo *english;
    const CSB_V1_VariantInfo *japanese;
    char reason[256];

    english = csb_v1_runtime_get_variant_info(CSB_V1_VARIANT_FMTOWNS_EN);
    japanese = csb_v1_runtime_get_variant_info(CSB_V1_VARIANT_FMTOWNS_JA);
    CHECK(english && strcmp(english->md5_gfx,
                            "405b757038eea3c263e60f240854d6de") == 0 &&
          strcmp(english->md5_dungeon,
                 "83c56cf1b779e7460a55c9299ebeb04b") == 0,
          "FM Towns English profile carries the original CDATA hashes");
    CHECK(japanese && strcmp(japanese->md5_gfx,
                             "761d6fc588b31aeaaa9caf3725e111b9") == 0 &&
          strcmp(japanese->md5_dungeon,
                 "7ca51c17ef8bd542ca5f0273672ec1a5") == 0,
          "FM Towns Japanese profile carries the original CJDATA hashes");
    CHECK(csb_v1_boot_graphics_dungeon_m11_entry_gate(
              english->md5_gfx, english->md5_dungeon,
              reason, sizeof(reason)) == 1,
          "M11 gate accepts the authenticated FM Towns English pair");
    CHECK(csb_v1_boot_graphics_dungeon_m11_entry_gate(
              japanese->md5_gfx, japanese->md5_dungeon,
              reason, sizeof(reason)) == 1,
          "M11 gate accepts the authenticated FM Towns Japanese pair");
}

static void test_amiga31_media_registry(void)
{
    const CSB_V1_VariantInfo *amiga31;
    char reason[256];

    amiga31 = csb_v1_runtime_get_variant_info(CSB_V1_VARIANT_AMIGA31_EN);
    CHECK(amiga31 && strcmp(amiga31->name, "Amiga 3.1 English") == 0 &&
          strcmp(amiga31->media_ref, "MEDIA37:A31E") == 0 &&
          strcmp(amiga31->md5_gfx,
                 "21197b1d4994fd835c403d5a33dcac2b") == 0 &&
          strcmp(amiga31->md5_dungeon,
                 "6695d2acebce49f95db1d8f3a5c733de") == 0,
          "Amiga 3.1 English profile preserves the original A31E hashes");
    CHECK(csb_v1_runtime_variant_from_hint("amiga31_en") ==
              CSB_V1_VARIANT_AMIGA31_EN,
          "Amiga 3.1 launcher hint selects the A31E runtime profile");
    CHECK(csb_v1_boot_graphics_dungeon_m11_entry_gate(
              amiga31->md5_gfx, amiga31->md5_dungeon,
              reason, sizeof(reason)) == 1,
          "M11 gate accepts the authenticated Amiga 3.1 English pair");
}

static void test_fmtowns_startup_surface_decode(void)
{
    const char *path = getenv("FIRESTAFF_CSB_FMTOWNS_GRAPHICS");
    unsigned char *pixels = NULL;
    uint8_t header[4];
    FILE *file;
    long file_size;
    int width = 0;
    int height = 0;
    CSB_V1_StartupGraphicDecodeReceipt_PC34 receipt;

    if (!path || path[0] == '\0') {
        printf("  SKIP: FIRESTAFF_CSB_FMTOWNS_GRAPHICS not set\n");
        return;
    }
    /* This opt-in real-media test must not quietly exercise a stale PC,
     * Atari, or Amiga cache entry merely because its leaf is GRAPHICS.DAT.
     * ReDMCSB F31E/F31J uses the 0x8001, 728-item little-endian container
     * that IMAGE2.C expands. */
    file = fopen(path, "rb");
    if (!file || fseek(file, 0, SEEK_END) != 0 ||
        (file_size = ftell(file)) < 0 || fseek(file, 0, SEEK_SET) != 0 ||
        fread(header, 1u, sizeof(header), file) != sizeof(header)) {
        if (file) fclose(file);
        printf("  SKIP: FIRESTAFF_CSB_FMTOWNS_GRAPHICS is unreadable\n");
        return;
    }
    fclose(file);
    if (file_size < (long)CSB_FMTOWNS_GRAPHICS_MIN_SIZE ||
        file_size > (long)CSB_FMTOWNS_GRAPHICS_MAX_SIZE ||
        header[0] != 0x01u || header[1] != 0x80u ||
        header[2] != (CSB_FMTOWNS_GRAPHICS_ITEM_COUNT & 0xFFu) ||
        header[3] != (CSB_FMTOWNS_GRAPHICS_ITEM_COUNT >> 8u)) {
        printf("  SKIP: FIRESTAFF_CSB_FMTOWNS_GRAPHICS is not an F31E/F31J GRAPHICS.DAT\n");
        return;
    }
#define CHECK_FMTOWNS_STARTUP_SURFACE(index, expected_width, expected_height, label) \
    do {                                                                  \
        memset(&receipt, 0, sizeof(receipt));                            \
        CHECK(csb_v1_boot_decode_graphics_dat_asset_pc34(                \
                  path, (index), &pixels, &width, &height, &receipt) && \
              pixels && width == (expected_width) &&                     \
              height == (expected_height) && receipt.valid &&           \
              receipt.ended_at_record_boundary &&                        \
              receipt.indexed_colors_are_4bit &&                         \
              receipt.compressed_record_sha256[0] != '\0',              \
              (label));                                                   \
        free(pixels);                                                     \
        pixels = NULL;                                                    \
    } while (0)

    /* ReDMCSB ENTRANCE.C F0806/F0442 selects C002..C005.  These exact
     * dimensions come from the authenticated 728-item FM Towns table. */
    CHECK_FMTOWNS_STARTUP_SURFACE(
        1u, 320, 153, "FM Towns C001 reaches the startup IMG2 surface route");
    CHECK_FMTOWNS_STARTUP_SURFACE(
        2u, 105, 161, "FM Towns C002 reaches the left-door IMG2 surface route");
    CHECK_FMTOWNS_STARTUP_SURFACE(
        3u, 128, 161, "FM Towns C003 reaches the right-door IMG2 surface route");
    CHECK_FMTOWNS_STARTUP_SURFACE(
        4u, 320, 200, "FM Towns C004 reaches the entrance IMG2 surface route");
    CHECK_FMTOWNS_STARTUP_SURFACE(
        5u, 320, 200, "FM Towns C005 reaches the credits IMG2 surface route");
#undef CHECK_FMTOWNS_STARTUP_SURFACE

    /* ReDMCSB F31E/F31J ENTRANCE.C F0807 releases the title/door sequence
     * into PANEL.C F0346/F0347, which consumes the real C017 and C040
     * terminal surfaces.  Do not enable the separate CHTWE/CHTWJ Game
     * handoff until both source records decode as bounded four-bit assets. */
#define CHECK_FMTOWNS_HUD_SURFACE(index, label)                           \
    do {                                                                   \
        memset(&receipt, 0, sizeof(receipt));                             \
        CHECK(csb_v1_boot_decode_graphics_dat_asset_pc34(                 \
                  path, (index), &pixels, &width, &height, &receipt) &&  \
              pixels && width > 0 && height > 0 && receipt.valid &&      \
              receipt.ended_at_record_boundary &&                         \
              receipt.indexed_colors_are_4bit &&                          \
              receipt.compressed_record_sha256[0] != '\0',              \
              (label));                                                    \
        free(pixels);                                                      \
        pixels = NULL;                                                     \
    } while (0)
    CHECK_FMTOWNS_HUD_SURFACE(
        17u, "FM Towns C017 decodes as source-owned live HUD material");
    CHECK_FMTOWNS_HUD_SURFACE(
        40u, "FM Towns C040 decodes as source-owned panel material");
#undef CHECK_FMTOWNS_HUD_SURFACE
}

static void test_fmtowns_authenticated_startup_session(void)
{
    const char *data_dir = getenv("FIRESTAFF_CSB_FMTOWNS_DATA_DIR");
    CSB_V1_BootProfile profile;
    CSB_V1_StartupRuntimeAssetSession_PC34 session;

    if (!data_dir || data_dir[0] == '\0') {
        printf("  SKIP: FIRESTAFF_CSB_FMTOWNS_DATA_DIR not set\n");
        return;
    }
    csb_v1_boot_profile_init(&profile);
    csb_v1_boot_startup_runtime_asset_session_init_pc34(&session);
    /* F31E starts CHTWE.EXP after SWITCHTW.  Its C001--C005 and C017/C040
     * records must be bound from the authenticated CDATA pair, rather than
     * from the same-named stale cache files.  ReDMCSB F31E/MKFF.BAT links
     * STARTUP1.C, STARTUP2.C, PANEL.C and DRAWVIEW.C into that program. */
    CHECK(csb_v1_boot_scan_assets(&profile, data_dir) == 0 &&
              profile.variant_id == CSB_V1_VARIANT_FMTOWNS_EN &&
              profile.assets_verified && profile.graphics_verified &&
              profile.dungeon_verified,
          "FM Towns CDATA pair scans as the authenticated F31E profile");
    if (profile.variant_id == CSB_V1_VARIANT_FMTOWNS_EN &&
        profile.assets_verified) {
        CHECK(csb_v1_boot_startup_runtime_asset_session_open_pc34(
                  &profile, &session) && session.valid &&
                  session.full_startup_ready &&
                  session.surfaces.entrance_screen_ready &&
                  session.surfaces.hud_surfaces_ready &&
                  session.hud_assets_bound,
              "FM Towns game handoff opens real entrance and HUD source surfaces");
    }
    csb_v1_boot_startup_runtime_asset_session_release_pc34(&session);
    csb_v1_boot_cleanup(&profile);
}

static void test_fmtowns_game_program_handoff(void)
{
    const char *data_dir = getenv("FIRESTAFF_CSB_FMTOWNS_GAME_DATA_DIR");
    CSB_V1_BootProfile profile;
    CSB_V1_FmtownsGameHandoffReceipt receipt;
    CSB_V1_FmtownsUtilityHandoffReceipt utility;
    CSB_V1_FmtownsUtilityMenuReceipt utility_menu;
    CSB_V1_FmtownsUtilityMenuHitBox utility_hit;
    uint8_t music_track;

    if (!data_dir || data_dir[0] == '\0') {
        printf("  SKIP: FIRESTAFF_CSB_FMTOWNS_GAME_DATA_DIR not set\n");
        return;
    }
    csb_v1_boot_profile_init(&profile);
    CHECK(csb_v1_boot_scan_assets(&profile, data_dir) == 0 &&
              profile.variant_id == CSB_V1_VARIANT_FMTOWNS_EN,
          "FM Towns Game handoff scans the authenticated F31E media pair");
    if (profile.variant_id == CSB_V1_VARIANT_FMTOWNS_EN &&
        profile.assets_verified) {
        CHECK(csb_v1_fmtowns_game_handoff_open(
                  &profile, CSB_FMTOWNS_SWITCH_ENGLISH, &receipt) &&
                  receipt.valid && receipt.executable_verified &&
                  receipt.language_matches_profile &&
                  receipt.game_program_is_c03_game &&
                  strcmp(receipt.executable_name, "CHTWE.EXP") == 0 &&
                  receipt.executable_size == 283936u &&
                  receipt.executable_fnv1a == 0x3da136f6u &&
                  receipt.startup_mini_verified &&
                  receipt.startup_mini_size == 42776u &&
                  receipt.startup_mini_fnv1a == 0x494999c9u &&
                  receipt.startup_mini_header_verified &&
                  receipt.startup_mini_header_key == 0x340fu &&
                  receipt.startup_mini_header_format_id == 5u &&
                  receipt.startup_mini_header_platform == 7u &&
                  receipt.startup_mini_header_dungeon_id == 13u &&
                  receipt.startup_mini_save_parts_verified &&
                  receipt.startup_mini_party_champion_count == 1u &&
                  receipt.startup_mini_event_maximum_count == 436u &&
                  receipt.startup_mini_active_group_capacity == 60u &&
                  receipt.startup_mini_verified_save_body_offset == 8236u &&
                  receipt.startup_mini_dungeon_tail_verified &&
                  receipt.startup_mini_dungeon_map_count == 11u &&
                  receipt.startup_mini_dungeon_column_count == 296u &&
                  receipt.startup_mini_dungeon_tail_checksum == 0x62dfu &&
                  receipt.startup_mini_first_map_offset_x == 17u &&
                  receipt.startup_mini_first_map_offset_y == 14u &&
                  receipt.startup_mini_dungeon_tail_offset == 10092u &&
                  receipt.startup_mini_dungeon_tail_size == 32682u &&
                  receipt.music_table_verified &&
                  receipt.music_table_source_offset == 271144u &&
                  receipt.music_table_size ==
                      CSB_V1_FMTOWNS_GAME_MUSIC_TABLE_BYTES &&
                  receipt.music_table_fnv1a == 0x3faffb70u,
              "SWITCHTW English Game exit binds retail CHTWE.EXP and CDATA/MINI.DAT");
        CHECK(csb_v1_fmtowns_utility_handoff_open(
                  &profile, CSB_FMTOWNS_SWITCH_ENGLISH, &utility) &&
                  utility.valid && utility.executable_verified &&
                  utility.utility_program_is_c06_cedt &&
                  strcmp(utility.executable_name, "UTILE.EXP") == 0 &&
                  utility.executable_size == 152387u &&
                  utility.executable_fnv1a == 0xff240e0cu &&
                  utility.p3_header_verified &&
                  utility.p3_header_size == 384u &&
                  utility.p3_load_image_offset == 512u &&
                  utility.p3_load_image_size == 151875u &&
                  utility.p3_initial_eip == 65024u,
              "SWITCHTW English Utility exit admits only retail UTILE.EXP");
        CHECK(csb_v1_fmtowns_utility_menu_open(
                  &profile, CSB_FMTOWNS_SWITCH_ENGLISH, &utility_menu) &&
                  utility_menu.valid && utility_menu.source_virtual_offset ==
                      0x11578u && utility_menu.source_size == 76u &&
                  utility_menu.source_fnv1a == 0xfd9986bfu &&
                  memcmp(utility_menu.source_bytes, "LOAD CHAMPIONS", 14u) == 0 &&
                  utility_menu.label_offsets[5] == 68u,
              "F31E C06 Utility menu uses the original six-label P3 pool");
        CHECK(csb_v1_fmtowns_utility_menu_action_at(
                  &utility_menu, 2, 186, &utility_hit) &&
                  utility_hit.action ==
                      CSB_V1_FMTOWNS_UTILITY_ACTION_LOAD_CHAMPIONS &&
                  csb_v1_fmtowns_utility_menu_action_at(
                      &utility_menu, 316, 194, &utility_hit) &&
                  utility_hit.action ==
                      CSB_V1_FMTOWNS_UTILITY_ACTION_MAKE_NEW_ADVENTURE &&
                  !csb_v1_fmtowns_utility_menu_action_at(
                      &utility_menu, 100, 170, &utility_hit),
              "F31E C06 Utility input uses the original inclusive mouse boxes");
        music_track = 0u;
        CHECK(csb_v1_fmtowns_game_music_track_at(&receipt, 0u, 2u, 0u,
                                                  &music_track) &&
                  music_track == 7u &&
                  csb_v1_fmtowns_game_music_track_at(&receipt, 0u, 0u, 0u,
                                                      &music_track) &&
                  music_track == 0u &&
                  !csb_v1_fmtowns_game_music_track_at(&receipt, 10u, 0u, 0u,
                                                       &music_track),
              "F31E Game music selectors come only from the verified G4099 table");
        CHECK(!csb_v1_fmtowns_game_handoff_open(
                  &profile, CSB_FMTOWNS_SWITCH_JAPANESE, &receipt),
              "F31E profile rejects the F31J Game-program route");
    }
    csb_v1_boot_cleanup(&profile);
}

int main(void)
{
    printf("=== CSB V1 Boot Profile Smoke Test ===\n\n");
    test_defaults();
    test_scan_missing_data();
    test_rescan_missing_data_clears_stale_handoff();
    test_save_root_override();
    test_enter_requires_assets();
    test_m11_entry_rehash_rejects_changed_media();
    test_enter_handoff_state();
    test_enter_loads_verified_dungeon_context();
    test_source_evidence();
    test_fmtowns_media_registry();
    test_amiga31_media_registry();
    test_fmtowns_startup_surface_decode();
    test_fmtowns_authenticated_startup_session();
    test_fmtowns_game_program_handoff();
    printf("\nPASSED: %d\nFAILED: %d\n", passed, failed);
    return failed == 0 ? 0 : 1;
}
