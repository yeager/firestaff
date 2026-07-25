#include "redmcsb_f0089_strncpy.h"

#include <assert.h>
#include <stdint.h>

static void copies_at_most_count_without_terminating(void)
{
    char destination[] = { 'x', 'x', 'x', 'x', 'x' };
    (void)destination;

    assert(redmcsb_f0089_strncpy(destination, "abcd", 3) == destination);
    assert(destination[0] == 'a');
    assert(destination[1] == 'b');
    assert(destination[2] == 'c');
    assert(destination[3] == 'x');
    assert(destination[4] == 'x');
}

static void copies_nul_without_strncpy_padding(void)
{
    char destination[] = { 'x', 'x', 'x', 'x', 'x' };
    (void)destination;

    assert(redmcsb_f0089_strncpy(destination, "a", 4) == destination);
    assert(destination[0] == 'a');
    assert(destination[1] == '\0');
    assert(destination[2] == 'x');
    assert(destination[3] == 'x');
    assert(destination[4] == 'x');
}

static void handles_non_positive_counts_without_writing(void)
{
    char destination[] = { 'x', 'x', 'x' };
    (void)destination;

    assert(redmcsb_f0089_strncpy(destination, "a", 0) == destination);
    assert(destination[0] == 'x');
    assert(redmcsb_f0089_strncpy(destination, "a", INT16_C(-1)) == destination);
    assert(destination[0] == 'x');
}

int main(void)
{
    copies_at_most_count_without_terminating();
    copies_nul_without_strncpy_padding();
    handles_non_positive_counts_without_writing();
    return 0;
}
