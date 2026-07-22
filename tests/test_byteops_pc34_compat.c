#include "byteops_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int check(int condition, const char *label)
{
    if (condition) return 1;
    fprintf(stderr, "FAIL: %s\n", label);
    return 0;
}

int main(void)
{
    char source[] = "abcde";
    char destination[sizeof(source)] = { 0 };
    char backward_overlap[] = "abcdefg";
    char forward_overlap[] = "abcdefg";
    char same_buffer[] = "abcdefg";
    char unchanged[] = "abcdefg";
    char clear_buffer[] = "xxxxxx";
    int ok = 1;

    F0007_MAIN_CopyBytes(source, destination, (long)sizeof(source));
    ok &= check(memcmp(source, destination, sizeof(source)) == 0,
                "copies a complete forward byte range");

    F0007_MAIN_CopyBytes(backward_overlap, backward_overlap + 2, 5);
    ok &= check(memcmp(backward_overlap, "ababcde", 7) == 0,
                "preserves source bytes when destination overlaps later");

    F0007_MAIN_CopyBytes(forward_overlap + 2, forward_overlap, 5);
    ok &= check(memcmp(forward_overlap, "cdefgfg", 7) == 0,
                "preserves source bytes when destination overlaps earlier");

    F0007_MAIN_CopyBytes(same_buffer, same_buffer, 7);
    ok &= check(memcmp(same_buffer, "abcdefg", 7) == 0,
                "same-buffer copy is unchanged");

    F0007_MAIN_CopyBytes(unchanged, unchanged + 1, 0);
    F0007_MAIN_CopyBytes(unchanged, unchanged + 1, -1);
    F0007_MAIN_CopyBytes(NULL, unchanged, 3);
    F0007_MAIN_CopyBytes(unchanged, NULL, 3);
    ok &= check(memcmp(unchanged, "abcdefg", 7) == 0,
                "zero, negative, and null CopyBytes inputs are no-ops");

    F0008_MAIN_ClearBytes(clear_buffer + 1, 4UL);
    ok &= check(clear_buffer[0] == 'x' && clear_buffer[1] == 0 &&
                    clear_buffer[2] == 0 && clear_buffer[3] == 0 &&
                    clear_buffer[4] == 0 && clear_buffer[5] == 'x',
                "clears only the requested byte range");

    F0008_MAIN_ClearBytes(clear_buffer, 0UL);
    F0008_MAIN_ClearBytes(NULL, 3UL);
    ok &= check(clear_buffer[0] == 'x' && clear_buffer[5] == 'x',
                "zero and null ClearBytes inputs are no-ops");

    if (!ok) return 1;
    puts("PASS byteops_pc34_compat");
    return 0;
}
