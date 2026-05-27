/*
 * nexus_boot_profile_smoke.c — Nexus V1 Phase 1 boot profile smoke test
 *
 * Tests only the boot profile: path resolution, default values,
 * validation diagnostics, and supported-feature flag inversion.
 * No game data required.
 */

#include "firestaff_nexus_v1_boot_profile.h"
#include <stdio.h>
#include <string.h>

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, msg) do { \
    if (cond) { g_pass++; printf("  PASS: %s\n", msg); } \
    else { g_fail++; printf("  FAIL: %s\n", msg); } \
} while (0)

int main(void) {
    printf("\n=== Nexus V1 Boot Profile Smoke Test ===\n\n");

    /* ── 1. Default profile ── */
    printf("1. Default profile:\n");
    {
        const Nexus_V1_BootProfile *def = Nexus_V1_BootProfile_GetDefault();
        CHECK(def != NULL, "GetDefault() != NULL");
        CHECK(def->tickRateMs == NEXUS_V1_TICK_RATE_MS, "tickRateMs == 55 ms");
        CHECK(def->renderRateMs == NEXUS_V1_RENDER_RATE_MS, "renderRateMs == 33 ms");
        CHECK(def->presentationMode == 0, "presentationMode == 0 (V1_ORIGINAL)");
    }

    /* ── 2. Init and path resolution ── */
    printf("\n2. Init and path resolution:\n");
    {
        Nexus_V1_BootProfile prof;
        int rc = Nexus_V1_BootProfile_Init(&prof, "/test/data", "/test/save", 0U);
        CHECK(rc == 0, "Init() returns 0");
        CHECK(prof.dataDir != NULL, "dataDir != NULL after Init");
        CHECK(prof.saveDir != NULL, "saveDir != NULL after Init");
        CHECK(prof.configPath != NULL, "configPath != NULL after Init");
        if (prof.dataDir) {
            CHECK(strstr(prof.dataDir, "nexus1") != NULL, "dataDir contains 'nexus1'");
        }
        if (prof.saveDir) {
            CHECK(strstr(prof.saveDir, "nexus1") != NULL, "saveDir contains 'nexus1'");
        }
    }

    /* ── 3. Runtime flag override ── */
    printf("\n3. Runtime flag override:\n");
    {
        Nexus_V1_BootProfile prof;
        int rc = Nexus_V1_BootProfile_Init(&prof, ".", ".", NEXUS_V1_RF_NO_3D_ENGINE);
        CHECK(rc == 0, "Init() with runtimeFlags != 0 returns 0");
        CHECK((prof.runtimeFlags & NEXUS_V1_RF_NO_3D_ENGINE) != 0, "NO_3D_ENGINE flag set");
        CHECK((prof.runtimeFlags & NEXUS_V1_RF_USE_SATURN_CD) == 0, "USE_SATURN_CD cleared when overridden");
    }

    /* ── 4. Validate assets (no data dir) ── */
    printf("\n4. Asset validation (no game data):\n");
    {
        Nexus_V1_Diagnostic diags[8];
        int count = Nexus_V1_BootProfile_ValidateAssets(NULL, diags, 8);
        /* NULL profile should return 0 diags (no crash) */
        CHECK(count == 0, "ValidateAssets(NULL) returns 0 (no crash)");
    }

    /* ── 5. Supported features (flag inversion) ── */
    printf("\n5. Supported features flag inversion:\n");
    {
        Nexus_V1_BootProfile prof;
        Nexus_V1_BootProfile_Init(&prof, ".", ".", 0U);
        unsigned int supported = Nexus_V1_BootProfile_SupportedFeatures(&prof);
        unsigned int unsupported = prof.unsupportedFeatureMask;
        /* supported = ~unsupportedFeatureMask */
        CHECK((supported & ~unsupported) == ~unsupported,
              "SupportedFeatures() == ~unsupportedFeatureMask");
        /* V2 modes should be unsupported in default profile */
        CHECK((supported & NX_UNSUPPFEAT_V2_MODES) == 0,
              "V2 modes NOT in supported set");
        CHECK((unsupported & NX_UNSUPPFEAT_V2_MODES) != 0,
              "V2 modes ARE in unsupportedFeatureMask");
    }

    /* ── 6. Diagnostic strings ── */
    printf("\n6. Diagnostic strings:\n");
    {
        const char *s = Nexus_V1_BootProfile_GetDiagnosticString(NEXUS_V1_DIAG_OK);
        CHECK(s != NULL && strcmp(s, "OK") == 0, "DIAG_OK -> 'OK'");
        s = Nexus_V1_BootProfile_GetDiagnosticString(NEXUS_V1_DIAG_MISSING_DM_BIN);
        CHECK(s != NULL && strstr(s, "DM.BIN") != NULL, "DIAG_MISSING_DM_BIN mentions DM.BIN");
        s = Nexus_V1_BootProfile_GetDiagnosticString(NEXUS_V1_DIAG_MISSING_DMDF_ARCHIVE);
        CHECK(s != NULL && strstr(s, "DMDF") != NULL, "DIAG_MISSING_DMDF_ARCHIVE mentions DMDF");
        s = Nexus_V1_BootProfile_GetDiagnosticString(NEXUS_V1_DIAG_CORRUPT_SATURN_CD_IMAGE);
        CHECK(s != NULL && strstr(s, "corrupt") != NULL, "DIAG_CORRUPT mentions corrupt");
        s = Nexus_V1_BootProfile_GetDiagnosticString(NEXUS_V1_DIAG_COUNT);
        CHECK(s != NULL && strcmp(s, "UNKNOWN") == 0, "DIAG_COUNT -> 'UNKNOWN'");
    }

    /* ── 7. Null profile getters ── */
    printf("\n7. Null-profile getters (no crash):\n");
    {
        const char *r = Nexus_V1_BootProfile_GetAssetRoot(NULL);
        CHECK(r == NULL, "GetAssetRoot(NULL) == NULL");
        r = Nexus_V1_BootProfile_GetSaveRoot(NULL);
        CHECK(r == NULL, "GetSaveRoot(NULL) == NULL");
        unsigned int f = Nexus_V1_BootProfile_SupportedFeatures(NULL);
        CHECK(f == 0U, "SupportedFeatures(NULL) == 0");
    }

    /* ── Summary ── */
    printf("\n=== Results: %d passed, %d failed ===\n\n", g_pass, g_fail);
    return g_fail > 0 ? 1 : 0;
}