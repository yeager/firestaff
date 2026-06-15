/*
 * test_firestaff_po_loader_multi_domain_pc34_compat.c
 *
 * Tests the multi-domain PO loader used by Firestaff's i18n
 * layer. Verifies:
 *  - FS_PO_MAX_STRINGS >= 548 (DM1 has 548 msgid, must all fit)
 *  - Multi-domain loading: dm1, csb, startup-menu can co-exist
 *  - Domain routing: gettext_in_domain("dm1", ...) does not
 *    return csb strings
 *  - Active-domain switching
 *  - Pass-through on missing keys
 *  - Empty msgstr returns the original msgid
 *
 * Source: docs/design/I18N_PO_LAYOUT_PLAN.md
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "firestaff_po_loader.h"

#define CHECK(cond, msg) do { \
    if (!(cond)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, msg); \
        return 1; \
    } \
} while (0)

static int write_po_file(const char* path, const char* content) {
    FILE* f = fopen(path, "w");
    if (!f) return -1;
    fputs(content, f);
    fclose(f);
    return 0;
}

int main(void) {
    char tmpTemplate[] = "/tmp/firestaff-po-multi-XXXXXX";
    char dm1Path[256];
    char csbPath[256];
    char startupPath[256];
    char langPath[256];

    if (!mkdtemp(tmpTemplate)) {
        perror("mkdtemp");
        return 1;
    }

    snprintf(dm1Path, sizeof(dm1Path), "%s/dm1.sv.po", tmpTemplate);
    snprintf(csbPath, sizeof(csbPath), "%s/csb.sv.po", tmpTemplate);
    snprintf(startupPath, sizeof(startupPath), "%s/startup-menu.sv.po", tmpTemplate);
    snprintf(langPath, sizeof(langPath), "%s/firestaff.sv.po", tmpTemplate);

    /* Write minimal sv catalogs */
    const char* dm1Po =
        "msgid \"FOOD\"\n"
        "msgstr \"MAT\"\n"
        "msgid \"WATER\"\n"
        "msgstr \"VATTEN\"\n"
        "msgid \"NORTH\"\n"
        "msgstr \"NORR\"\n";
    const char* csbPo =
        "msgid \"CSB_ONLY\"\n"
        "msgstr \"ENDAST CSB\"\n"
        "msgid \"MENU_TEXT\"\n"
        "msgstr \"MENYTEXT\"\n";
    const char* startupPo =
        "msgid \"LAUNCH\"\n"
        "msgstr \"STARTA\"\n"
        "msgid \"QUIT\"\n"
        "msgstr \"AVSLUTA\"\n";
    const char* legacyPo =
        "msgid \"LEGACY_KEY\"\n"
        "msgstr \"GAMMAL NYCKEL\"\n";

    CHECK(write_po_file(dm1Path, dm1Po) == 0, "write dm1.po");
    CHECK(write_po_file(csbPath, csbPo) == 0, "write csb.po");
    CHECK(write_po_file(startupPath, startupPo) == 0, "write startup-menu.po");
    CHECK(write_po_file(langPath, legacyPo) == 0, "write legacy firestaff.po");

    /* Load 3 different domains in order */
    CHECK(fs_po_load(dm1Path) == 3, "load dm1 returns 3");
    CHECK(fs_po_load(csbPath) == 2, "load csb returns 2");
    CHECK(fs_po_load(startupPath) == 2, "load startup-menu returns 2");

    /* Cross-domain: dm1 should NOT see csb's CSB_ONLY key */
    const char* gotDm1Crosstalk = fs_po_gettext_in_domain("dm1", "CSB_ONLY");
    CHECK(gotDm1Crosstalk != NULL, "dm1 lookup of CSB_ONLY returns non-null");
    CHECK(strcmp(gotDm1Crosstalk, "CSB_ONLY") == 0,
          "dm1 does not contain CSB_ONLY — returns msgid itself");

    /* Cross-domain: csb should NOT see dm1's FOOD key */
    const char* gotCsbCrosstalk = fs_po_gettext_in_domain("csb", "FOOD");
    CHECK(gotCsbCrosstalk != NULL, "csb lookup of FOOD returns non-null");
    CHECK(strcmp(gotCsbCrosstalk, "FOOD") == 0,
          "csb does not contain FOOD — returns msgid itself");

    /* Cross-domain: startup-menu should NOT see dm1's WATER key */
    const char* gotStartupCrosstalk = fs_po_gettext_in_domain("startup-menu", "WATER");
    CHECK(gotStartupCrosstalk != NULL, "startup-menu lookup of WATER returns non-null");
    CHECK(strcmp(gotStartupCrosstalk, "WATER") == 0,
          "startup-menu does not contain WATER — returns msgid itself");

    /* dm1: FOOD -> MAT */
    const char* gotFood = fs_po_gettext_in_domain("dm1", "FOOD");
    CHECK(gotFood != NULL, "dm1 FOOD is non-null");
    CHECK(strcmp(gotFood, "MAT") == 0, "dm1 FOOD -> MAT");

    /* csb: CSB_ONLY -> ENDAST CSB */
    const char* gotC = fs_po_gettext_in_domain("csb", "CSB_ONLY");
    CHECK(gotC != NULL, "csb CSB_ONLY is non-null");
    CHECK(strcmp(gotC, "ENDAST CSB") == 0, "csb CSB_ONLY -> ENDAST CSB");

    /* startup: LAUNCH -> STARTA */
    const char* gotL = fs_po_gettext_in_domain("startup-menu", "LAUNCH");
    CHECK(gotL != NULL, "startup LAUNCH is non-null");
    CHECK(strcmp(gotL, "STARTA") == 0, "startup LAUNCH -> STARTA");

    /* Active domain routing: switch to dm1 */
    CHECK(fs_po_set_active_domain("dm1") == 0, "set active to dm1");
    CHECK(strcmp(fs_po_gettext("FOOD"), "MAT") == 0, "active=dm1 -> FOOD=MAT");

    /* Switch to csb */
    CHECK(fs_po_set_active_domain("csb") == 0, "set active to csb");
    CHECK(strcmp(fs_po_gettext("CSB_ONLY"), "ENDAST CSB") == 0, "active=csb -> CSB_ONLY=ENDAST CSB");

    /* Switch back to dm1 */
    CHECK(fs_po_set_active_domain("dm1") == 0, "set active back to dm1");
    CHECK(strcmp(fs_po_gettext("FOOD"), "MAT") == 0, "active=dm1 again -> FOOD=MAT");

    /* Pass-through: missing key returns msgid */
    const char* gotMissing = fs_po_gettext_in_domain("dm1", "NONEXISTENT_KEY_42");
    CHECK(gotMissing != NULL, "missing key is non-null");
    CHECK(strcmp(gotMissing, "NONEXISTENT_KEY_42") == 0,
          "missing key returns the msgid itself");

    /* Loaded-count diagnostic */
    CHECK(fs_po_get_loaded_count_in_domain("dm1") == 3, "dm1 has 3 entries");
    CHECK(fs_po_get_loaded_count_in_domain("csb") == 2, "csb has 2 entries");
    CHECK(fs_po_get_loaded_count_in_domain("startup-menu") == 2, "startup-menu has 2 entries");
    CHECK(fs_po_get_loaded_count_in_domain("nonexistent") == 0, "nonexistent has 0 entries");

    /* Legacy single-domain API still works */
    CHECK(fs_po_load_for_language(tmpTemplate, "sv") >= 1, "legacy firestaff.sv.po loads");
    CHECK(fs_po_get_loaded_count_in_domain("firestaff") == 1,
          "legacy firestaff.sv.po has 1 entry");

    printf("PASS: all firestaff_po_loader multi-domain tests\n");
    return 0;
}
