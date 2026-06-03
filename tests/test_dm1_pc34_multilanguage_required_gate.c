#if defined(__APPLE__) && !defined(_DARWIN_C_SOURCE)
#define _DARWIN_C_SOURCE 1
#endif
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

#include "asset_status_m12.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <direct.h>
#define MKDIR(path) _mkdir(path)
static int test_setenv(const char* name, const char* value) {
    return _putenv_s(name, value ? value : "") == 0;
}
#else
#include <sys/stat.h>
#include <unistd.h>
#define MKDIR(path) mkdir((path), 0700)
static int test_setenv(const char* name, const char* value) {
    return setenv(name, value ? value : "", 1) == 0;
}
#endif

static int failures;

#define CHECK(expr) do { \
    if (!(expr)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr); \
        ++failures; \
    } \
} while (0)

static const char* kGraphicsPayload =
    "Firestaff synthetic DM1 PC 3.4 multilanguage GRAPHICS fixture\n";
static const char* kDungeonPayload =
    "Firestaff synthetic DM1 PC 3.4 multilanguage DUNGEON fixture\n";

static int make_dir(const char* path) {
    return MKDIR(path) == 0;
}

static int write_payload(const char* path, const char* payload) {
    FILE* fp = fopen(path, "wb");
    size_t size;
    if (!fp) {
        return 0;
    }
    size = strlen(payload);
    if (fwrite(payload, 1U, size, fp) != size) {
        fclose(fp);
        return 0;
    }
    return fclose(fp) == 0;
}

static int path_exists(const char* path) {
    FILE* fp = fopen(path, "rb");
    if (!fp) {
        return 0;
    }
    fclose(fp);
    return 1;
}

static const M12_AssetRequiredFileStatus* required_file_by_role(
    const M12_AssetStatus* status,
    const char* roleId) {
    size_t i;
    size_t count = M12_AssetStatus_GetRequiredFileCount(status, "dm1");
    for (i = 0U; i < count; ++i) {
        const M12_AssetRequiredFileStatus* file =
            M12_AssetStatus_GetRequiredFile(status, "dm1", i);
        if (file && file->roleId && strcmp(file->roleId, roleId) == 0) {
            return file;
        }
    }
    return NULL;
}

static int make_isolated_home(char* out, size_t outSize) {
#ifdef _WIN32
    int rc = snprintf(out, outSize, ".\\firestaff_dm1_ml_gate_home_%lu",
                      (unsigned long)rand());
    if (rc <= 0 || (size_t)rc >= outSize) {
        return 0;
    }
    return make_dir(out);
#else
    char templatePath[] = "/tmp/firestaff-dm1-ml-gate-home-XXXXXX";
    char* made = mkdtemp(templatePath);
    if (!made) {
        return 0;
    }
    snprintf(out, outSize, "%s", made);
    return 1;
#endif
}

static int make_case_dir(char* out, size_t outSize,
                         const char* home,
                         const char* name) {
    int rc = snprintf(out, outSize, "%s/%s", home, name);
    if (rc <= 0 || (size_t)rc >= outSize) {
        return 0;
    }
    return make_dir(out);
}

static void scan_and_check_graphics_only(const char* root,
                                         const char* graphicsMd5,
                                         const char* dungeonMd5) {
    M12_AssetStatus status;
    const M12_AssetVersionStatus* multi;
    const M12_AssetRequiredFileStatus* graphics;
    const M12_AssetRequiredFileStatus* dungeon;

    M12_AssetStatus_TestSetDm1MultilanguageSyntheticHashes(graphicsMd5, dungeonMd5);
    M12_AssetStatus_Scan(&status, root);

    multi = M12_AssetStatus_GetVersion(&status, "dm1", 1U);
    graphics = required_file_by_role(&status, "graphics");
    dungeon = required_file_by_role(&status, "dungeon");

    CHECK(M12_AssetStatus_FindVersionIndex("dm1", "pc34-multi") == 1);
    CHECK(multi != NULL);
    CHECK(multi && multi->matched == 1);
    CHECK(multi && multi->versionId && strcmp(multi->versionId, "pc34-multi") == 0);
    CHECK(multi && strcmp(multi->matchedMd5, graphicsMd5) == 0);
    CHECK(M12_AssetStatus_GameAvailable(&status, "dm1") == 0);
    CHECK(M12_AssetStatus_GetRequiredFileCount(&status, "dm1") == 2U);
    CHECK(graphics && graphics->matched == 1);
    CHECK(graphics && strcmp(graphics->matchedHash, graphicsMd5) == 0);
    CHECK(dungeon && dungeon->matched == 0);
}

static void scan_and_check_dungeon_only(const char* root,
                                        const char* graphicsMd5,
                                        const char* dungeonMd5) {
    M12_AssetStatus status;
    const M12_AssetVersionStatus* multi;
    const M12_AssetRequiredFileStatus* graphics;
    const M12_AssetRequiredFileStatus* dungeon;

    M12_AssetStatus_TestSetDm1MultilanguageSyntheticHashes(graphicsMd5, dungeonMd5);
    M12_AssetStatus_Scan(&status, root);

    multi = M12_AssetStatus_GetVersion(&status, "dm1", 1U);
    graphics = required_file_by_role(&status, "graphics");
    dungeon = required_file_by_role(&status, "dungeon");

    CHECK(multi != NULL);
    CHECK(multi && multi->matched == 0);
    CHECK(M12_AssetStatus_GameAvailable(&status, "dm1") == 0);
    CHECK(M12_AssetStatus_GetRequiredFileCount(&status, "dm1") == 2U);
    CHECK(graphics && graphics->matched == 0);
    CHECK(dungeon && dungeon->matched == 1);
    CHECK(dungeon && strcmp(dungeon->matchedHash, dungeonMd5) == 0);
}

static void scan_and_check_complete_required(const char* root,
                                             const char* graphicsMd5,
                                             const char* dungeonMd5) {
    char optionalTitle[512];
    char optionalIntro[512];
    M12_AssetStatus status;
    const M12_AssetVersionStatus* multi;
    const M12_AssetRequiredFileStatus* graphics;
    const M12_AssetRequiredFileStatus* dungeon;

    snprintf(optionalTitle, sizeof(optionalTitle), "%s/TITLE.PAK", root);
    snprintf(optionalIntro, sizeof(optionalIntro), "%s/INTRO.DAT", root);

    M12_AssetStatus_TestSetDm1MultilanguageSyntheticHashes(graphicsMd5, dungeonMd5);
    M12_AssetStatus_Scan(&status, root);

    multi = M12_AssetStatus_GetVersion(&status, "dm1", 1U);
    graphics = required_file_by_role(&status, "graphics");
    dungeon = required_file_by_role(&status, "dungeon");

    CHECK(!path_exists(optionalTitle));
    CHECK(!path_exists(optionalIntro));
    CHECK(multi != NULL);
    CHECK(multi && multi->matched == 1);
    CHECK(multi && multi->label &&
          strcmp(multi->label, "PC 3.4 Multilanguage") == 0);
    CHECK(M12_AssetStatus_GameAvailable(&status, "dm1") == 1);
    CHECK(graphics && graphics->matched == 1);
    CHECK(dungeon && dungeon->matched == 1);
    CHECK(graphics && strcmp(graphics->matchedHash, graphicsMd5) == 0);
    CHECK(dungeon && strcmp(dungeon->matchedHash, dungeonMd5) == 0);
}

int main(void) {
    char home[512];
    char graphicsOnlyRoot[512];
    char dungeonOnlyRoot[512];
    char completeRoot[512];
    char graphicsPath[512];
    char dungeonPath[512];
    char graphicsMd5[M12_ASSET_MD5_CAPACITY];
    char dungeonMd5[M12_ASSET_MD5_CAPACITY];

    if (!make_isolated_home(home, sizeof(home)) ||
        !test_setenv("HOME", home) ||
        !test_setenv("FIRESTAFF_DATA", "") ||
        !make_case_dir(graphicsOnlyRoot, sizeof(graphicsOnlyRoot), home, "graphics-only") ||
        !make_case_dir(dungeonOnlyRoot, sizeof(dungeonOnlyRoot), home, "dungeon-only") ||
        !make_case_dir(completeRoot, sizeof(completeRoot), home, "complete")) {
        fprintf(stderr, "fixture setup failed\n");
        return 1;
    }

    snprintf(graphicsPath, sizeof(graphicsPath), "%s/ml-graphics.fixture", completeRoot);
    snprintf(dungeonPath, sizeof(dungeonPath), "%s/ml-dungeon.fixture", completeRoot);
    CHECK(write_payload(graphicsPath, kGraphicsPayload));
    CHECK(write_payload(dungeonPath, kDungeonPayload));
    CHECK(m12_file_md5_hex(graphicsPath, graphicsMd5));
    CHECK(m12_file_md5_hex(dungeonPath, dungeonMd5));

    snprintf(graphicsPath, sizeof(graphicsPath), "%s/ml-graphics.fixture", graphicsOnlyRoot);
    CHECK(write_payload(graphicsPath, kGraphicsPayload));
    snprintf(dungeonPath, sizeof(dungeonPath), "%s/ml-dungeon.fixture", dungeonOnlyRoot);
    CHECK(write_payload(dungeonPath, kDungeonPayload));

    scan_and_check_graphics_only(graphicsOnlyRoot, graphicsMd5, dungeonMd5);
    scan_and_check_dungeon_only(dungeonOnlyRoot, graphicsMd5, dungeonMd5);
    scan_and_check_complete_required(completeRoot, graphicsMd5, dungeonMd5);

    M12_AssetStatus_TestSetDm1MultilanguageSyntheticHashes(NULL, NULL);

    if (failures) {
        fprintf(stderr, "%d failure(s)\n", failures);
        return 1;
    }
    puts("ok: DM1 PC 3.4 multilanguage requires both synthetic GRAPHICS and DUNGEON hashes; optional title/intro absence does not block");
    return 0;
}
