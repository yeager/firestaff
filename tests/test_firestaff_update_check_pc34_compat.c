/*
 * test_firestaff_update_check_pc34_compat.c
 *
 * Self-update-check helpers regression gate.  No network
 * path; this verifies the semver compare, the JSON tag_name
 * extractor, and the evaluate() decision logic.
 */
#include "firestaff_update_check_pc34_compat.h"
#include "changelog_m12.h"

#include <stdio.h>
#include <string.h>

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, msg) do { \
    if (cond) { ++g_pass; printf("  PASS: %s\n", msg); } \
    else      { ++g_fail; printf("  FAIL: %s\n", msg); } \
} while (0)

int main(void) {
    char tag[64];
    FirestaffUpdateResult res;

    printf("=== Firestaff self-update helpers (v2.7.18) ===\n");

    /* ---- semver compare ---- */
    CHECK(firestaff_update_check_compare_semver("2.7.14", "2.7.14") == 0,
          "2.7.14 == 2.7.14");
    CHECK(firestaff_update_check_compare_semver("2.7.18", "2.7.14") == 1,
          "2.7.18 > 2.7.14");
    CHECK(firestaff_update_check_compare_semver("2.7.14", "2.7.18") == -1,
          "2.7.14 < 2.7.18");
    CHECK(firestaff_update_check_compare_semver("v2.7.18", "2.7.14") == 1,
          "v2.7.18 > 2.7.14 (v prefix stripped)");
    CHECK(firestaff_update_check_compare_semver("2.7.18-rc.1", "2.7.14") == 1,
          "2.7.18-rc.1 > 2.7.14 (pre-release stripped for base)");
    CHECK(firestaff_update_check_compare_semver("2.7.18-rc.1", "2.7.18") == 0,
          "2.7.18-rc.1 == 2.7.18 (same base)");
    CHECK(firestaff_update_check_compare_semver("3.0.0", "2.9.99") == 1,
          "3.0.0 > 2.9.99 (major wins)");
    CHECK(firestaff_update_check_compare_semver("2.9.0", "2.10.0") == -1,
          "2.9.0 < 2.10.0 (numeric not lex)");
    CHECK(firestaff_update_check_compare_semver("1.0.0", "0.99.99") == 1,
          "1.0.0 > 0.99.99");
    CHECK(firestaff_update_check_compare_semver(NULL, "2.7.14") == 0,
          "NULL a -> 0");
    CHECK(firestaff_update_check_compare_semver("2.7.14", NULL) == 0,
          "NULL b -> 0");

    /* ---- JSON tag_name extractor ---- */
    {
        const char* body =
            "{\"tag_name\":\"v2.7.19\",\"name\":\"v2.7.19\""
            ",\"published_at\":\"2026-06-14T10:00:00Z\"}";
        int rc = firestaff_update_check_extract_tag(
            body, (int)strlen(body), tag, (int)sizeof(tag));
        CHECK(rc == 1, "extract_tag from valid body -> 1");
        CHECK(strcmp(tag, "v2.7.19") == 0,
              "extracted tag == v2.7.19");
    }
    {
        /* Body without tag_name. */
        const char* body = "{\"name\":\"v2.7.19\"}";
        int rc = firestaff_update_check_extract_tag(
            body, (int)strlen(body), tag, (int)sizeof(tag));
        CHECK(rc == 0, "extract_tag from body without tag_name -> 0");
    }
    {
        /* Empty body. */
        int rc = firestaff_update_check_extract_tag("", 0, tag, (int)sizeof(tag));
        CHECK(rc == 0, "extract_tag from empty body -> 0");
    }
    {
        /* Backslash-escaped tag. */
        const char* body = "{\"tag_name\":\"v2.7.19-rc.1\\n\"}";
        int rc = firestaff_update_check_extract_tag(
            body, (int)strlen(body), tag, (int)sizeof(tag));
        CHECK(rc == 1, "extract_tag from escaped body -> 1");
        CHECK(strcmp(tag, "v2.7.19-rc.1\n") == 0,
              "extracted tag == v2.7.19-rc.1 with unescaped \\n");
    }

    /* ---- evaluate() ---- */
    {
        /* Body with newer tag. */
        const char* body = "{\"tag_name\":\"v99.0.0\"}";
        res = firestaff_update_check_evaluate(body, (int)strlen(body));
        CHECK(res == FIRESTAFF_UPDATE_NEW_AVAILABLE,
              "evaluate newer body -> NEW_AVAILABLE");
    }
    {
        /* Body with same/older tag. */
        const char* body = "{\"tag_name\":\"v0.0.0\"}";
        res = firestaff_update_check_evaluate(body, (int)strlen(body));
        CHECK(res == FIRESTAFF_UPDATE_UP_TO_DATE,
              "evaluate older body -> UP_TO_DATE");
    }
    {
        /* Body equal to current version. */
        const char cur[64];
        snprintf((char*)cur, sizeof(cur), "\"tag_name\":\"v%s\"",
                 M12_Changelog_VersionString());
        res = firestaff_update_check_evaluate(cur, (int)strlen(cur));
        CHECK(res == FIRESTAFF_UPDATE_UP_TO_DATE,
              "evaluate same-version body -> UP_TO_DATE");
    }
    {
        /* Malformed body. */
        res = firestaff_update_check_evaluate("not json", 8);
        CHECK(res == FIRESTAFF_UPDATE_PARSE_ERROR,
              "evaluate malformed body -> PARSE_ERROR");
    }

    printf("\n=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
