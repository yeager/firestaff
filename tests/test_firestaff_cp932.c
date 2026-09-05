#include "firestaff_cp932.h"
#include <assert.h>
#include <string.h>

int main(void)
{
    char out[32];
    assert(firestaff_cp932_to_utf8("ACTION", 6, out, sizeof(out)) == 6);
    assert(strcmp(out, "ACTION") == 0);
    /* CP932 Japanese text and half-width katakana have distinct UTF-8 keys. */
    assert(firestaff_cp932_to_utf8("\x93\xfa\x96\x7b", 4, out, sizeof(out)) == 6);
    assert(strcmp(out, "\xe6\x97\xa5\xe6\x9c\xac") == 0);
    assert(firestaff_cp932_to_utf8("\xb6\xc0\xb6\xc5", 4, out, sizeof(out)) == 12);
    assert(firestaff_cp932_to_utf8("\x93\xfa", 2, out, 3) == -1);
    assert(out[0] == 0);
    assert(firestaff_cp932_to_utf8("\x93\xfa", 2, out, 4) == 3);
    assert(out[3] == 0);
    assert(firestaff_cp932_to_utf8("A\x81", 2, out, sizeof(out)) == -1);
    assert(out[0] == 0);
    assert(firestaff_cp932_to_utf8("\x81\x7f", 2, out, sizeof(out)) == -1);
    assert(firestaff_cp932_to_utf8("\x81\xad", 2, out, sizeof(out)) == -1);
    assert(firestaff_cp932_to_utf8("", 0, out, 1) == 0);
    assert(firestaff_cp932_to_utf8(NULL, 0, out, sizeof(out)) == -1);
    assert(firestaff_cp932_to_utf8("A", 1, NULL, 0) == -1);
    return 0;
}
