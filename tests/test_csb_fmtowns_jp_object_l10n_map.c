/* The F31J M564 table contains CP932 presentation text. This test verifies
 * the reviewed source-index bridge used before CSB gettext; it never changes
 * source-owned game media. */
#include <stdio.h>
#include <string.h>

#include "../src/engine/csb_fmtowns_jp_object_l10n.inc"

static int failures;

static void expect_name(int index, const char *expected)
{
    if (index < 0 ||
        index >= (int)(sizeof(m11_csb_fmtowns_jp_object_msgids) /
                       sizeof(m11_csb_fmtowns_jp_object_msgids[0])) ||
        strcmp(m11_csb_fmtowns_jp_object_msgids[index], expected) != 0) {
        fprintf(stderr, "unexpected F31J M564 bridge entry %d\n", index);
        ++failures;
    }
}

int main(void)
{
    if (sizeof(m11_csb_fmtowns_jp_object_msgids) /
            sizeof(m11_csb_fmtowns_jp_object_msgids[0]) != 177u) {
        fputs("F31J M564 bridge count changed\n", stderr);
        return 1;
    }
    expect_name(0, "COMPASS");
    expect_name(15, "DAGGER");
    expect_name(32, "CLAW BOW");
    expect_name(176, "HALTER");
    return failures ? 1 : 0;
}
