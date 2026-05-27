/*
 * tqr_v1_boot_profile_smoke.c — Theron's Quest V1 boot profile smoke test
 *
 * Theron's Quest V1 Phase 1 — tests boot/profile API surface.
 *
 * Run: ./build/test_tqr_v1_boot_profile_smoke
 * (No game data needed — tests API contract and struct sizes only.)
 */

#include <stdio.h>
#include <string.h>
#include "firestaff_tqr_v1_boot_profile.h"

/* ── Helpers ─────────────────────────────────────────────────── */

static int passed = 0;
static int failed = 0;

#define CHECK(cond, msg) do { \
    if (cond) { \
        printf("  PASS: %s\\n", msg); \
        passed++; \
    } else { \
        printf("  FAIL: %s\\n", msg); \
        failed++; \
    } \
} while (0)

/* ── Tests ────────────────────────────────────────────────────── */

static void test_default_profile_exists(void) {
    printf("[%s]\\n", __func__);
    const TQR_V1_BootProfile *def = TQR_V1_BootProfile_GetDefault();
    CHECK(def != NULL, "GetDefault returns non-NULL");
    CHECK(def->tickRateMs == TQR_V1_TICK_RATE_MS,
          "tick rate defaults to 55 ms");
    CHECK(def->renderRateMs == TQR_V1_RENDER_RATE_MS,
          "render rate defaults to ~16 ms");
    CHECK(def->dungeonIndex == -1,
          "default dungeon index is -1 (overworld)");
    CHECK(def->inDungeon == 0,
          "default inDungeon is 0 (not inside a dungeon)");
}

static void test_init_changes_nothing_without_override(void) {
    printf("[%s]\\n", __func__);
    TQR_V1_BootProfile p;
    int rc = TQR_V1_BootProfile_Init(&p, NULL, NULL, 0);
    CHECK(rc == 0, "Init with NULL dirs returns 0");
    CHECK(p.tickRateMs == TQR_V1_TICK_RATE_MS,
          "Init preserves default tick rate");
    CHECK(p.saveDir != NULL, "Init resolves save directory");
    CHECK(p.dataDir != NULL, "Init resolves data directory");
    CHECK(p.configPath != NULL, "Init resolves config path");
}

static void test_flags_consistent_with_defaults(void) {
    printf("[%s]\\n", __func__);
    TQR_V1_BootProfile p;
    TQR_V1_BootProfile_Init(&p, NULL, NULL, 0);
    unsigned int expected =
        TQR_V1_RF_USE_HUCARD
        | TQR_V1_RF_NO_IN_DUNGEON_SAVE
        | TQR_V1_RF_THERON_PERSISTENT
        | TQR_V1_RF_LIGHT_ITEM_SET
        | TQR_V1_RF_LIGHT_CREATURE_SET
        | TQR_V1_RF_NO_KINGS_WISDOM
        | TQR_V1_RF_NO_GANGULF_REVIVAL
        | TQR_V1_RF_NO_PARTY_SWAP
        | TQR_V1_RF_NOMINAL_SCROLL_SYSTEM;
    CHECK((p.runtimeFlags & expected) == expected,
          "All default flags are set");
    CHECK(!!(p.runtimeFlags & TQR_V1_RF_USE_CD_IMAGE) == 0,
          "USE_CD_IMAGE is not set by default (HuCard primary)");
}

static void test_cant_save_inside_dungeon(void) {
    printf("[%s]\\n", __func__);
    TQR_V1_BootProfile p;
    TQR_V1_BootProfile_Init(&p, NULL, NULL, 0);

    p.inDungeon = 0;
    CHECK(TQR_V1_BootProfile_CanSave(&p) == 1,
          "CanSave returns 1 at overworld (inDungeon=0)");

    p.inDungeon = 1;
    CHECK(TQR_V1_BootProfile_CanSave(&p) == 0,
          "CanSave returns 0 inside dungeon (inDungeon=1)");
}

static void test_diagnostic_strings_valid(void) {
    printf("[%s]\\n", __func__);
    for (int i = TQR_V1_DIAG_OK; i < TQR_V1_DIAG_COUNT; i++) {
        const char *s = TQR_V1_BootProfile_GetDiagnosticString(i);
        CHECK(s != NULL && strcmp(s, "UNKNOWN") != 0,
              "Diagnostic string valid for all codes");
    }
}

static void test_supported_features_mask_inverse(void) {
    printf("[%s]\\n", __func__);
    TQR_V1_BootProfile p;
    TQR_V1_BootProfile_Init(&p, NULL, NULL, 0);

    unsigned int sup = TQR_V1_BootProfile_SupportedFeatures(&p);
    /* unsupportedFeatureMask should be the complement of sup */
    CHECK((sup & p.unsupportedFeatureMask) == 0,
          "Supported and unsupported masks are complementary");
}

static void test_asset_validation_zero_diags_on_null(void) {
    printf("[%s]\\n", __func__);
    TQR_V1_Diagnostic diags[16];
    TQR_V1_BootProfile p;
    TQR_V1_BootProfile_Init(&p, NULL, NULL, 0);

    int n = TQR_V1_BootProfile_ValidateAssets(NULL, diags, 16);
    CHECK(n == 0, "ValidateAssets returns 0 for NULL profile");

    n = TQR_V1_BootProfile_ValidateAssets(&p, NULL, 16);
    CHECK(n == 0, "ValidateAssets returns 0 for NULL diags array");

    n = TQR_V1_BootProfile_ValidateAssets(&p, diags, 0);
    CHECK(n == 0, "ValidateAssets returns 0 for maxDiags=0");
}

static void test_game_id_label_platform(void) {
    printf("[%s]\\n", __func__);
    CHECK(strcmp(TQR_V1_GAME_ID, "theron") == 0,
          "TQR_V1_GAME_ID = \"theron\"");
    CHECK(strcmp(TQR_V1_GAME_LABEL, "THERON'S QUEST") == 0,
          "TQR_V1_GAME_LABEL = \"THERON'S QUEST\"");
    CHECK(strcmp(TQR_V1_PLATFORM_LABEL, "PC ENGINE / TURBOGRAFX-16") == 0,
          "TQR_V1_PLATFORM_LABEL = \"PC ENGINE / TURBOGRAFX-16\"");
}

static void test_struct_field_offsets(void) {
    printf("[%s]\\n", __func__);
    TQR_V1_BootProfile p;
    TQR_V1_BootProfile_Init(&p, NULL, NULL, 0);
    /* Just verify the struct is accessible and fields are sane */
    CHECK(p.tickRateMs == 55, "tickRateMs accessible and correct");
    CHECK(p.renderRateMs == 16, "renderRateMs accessible and correct");
    CHECK(p.dungeonIndex == -1, "dungeonIndex accessible and correct");
    CHECK(p.inDungeon == 0, "inDungeon accessible and correct");
    CHECK(p.runtimeFlags != 0, "runtimeFlags non-zero");
    CHECK(p.unsupportedFeatureMask != 0, "unsupportedFeatureMask non-zero");
}

static void test_no_party_swap_enforced(void) {
    printf("[%s]\\n", __func__);
    TQR_V1_BootProfile p;
    TQR_V1_BootProfile_Init(&p, NULL, NULL, 0);
    CHECK(!!(p.runtimeFlags & TQR_V1_RF_NO_PARTY_SWAP),
          "NO_PARTY_SWAP flag is set by default");
}

static void test_light_item_set_flag(void) {
    printf("[%s]\\n", __func__);
    TQR_V1_BootProfile p;
    TQR_V1_BootProfile_Init(&p, NULL, NULL, 0);
    CHECK(!!(p.runtimeFlags & TQR_V1_RF_LIGHT_ITEM_SET),
          "LIGHT_ITEM_SET flag is set by default");
    CHECK(!!(p.runtimeFlags & TQR_V1_RF_LIGHT_CREATURE_SET),
          "LIGHT_CREATURE_SET flag is set by default");
}

/* ── Main ──────────────────────────────────────────────────────── */

int main(void) {
    printf("=== Theron's Quest V1 Boot Profile — Smoke Tests ===\\n\\n");

    test_default_profile_exists();
    test_init_changes_nothing_without_override();
    test_flags_consistent_with_defaults();
    test_cant_save_inside_dungeon();
    test_diagnostic_strings_valid();
    test_supported_features_mask_inverse();
    test_asset_validation_zero_diags_on_null();
    test_game_id_label_platform();
    test_struct_field_offsets();
    test_no_party_swap_enforced();
    test_light_item_set_flag();

    printf("\\n");
    printf("  PASSED: %d\\n", passed);
    printf("  FAILED: %d\\n", failed);
    printf("\\n");

    return failed == 0 ? 0 : 1;
}