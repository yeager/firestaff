#include "f0922_custom_strcpy_compat.h"

#include <stdio.h>
#include <string.h>

static int check(int condition, const char *label)
{
    if (condition) {
        return 1;
    }

    fprintf(stderr, "FAIL: %s\n", label);
    return 0;
}

int main(void)
{
    char destination[6] = {'x', 'x', 'x', 'x', 'x', 'x'};
    char rejected[6] = {'k', 'e', 'e', 'p', '!', '\0'};
    char before[sizeof(rejected)];
    int ok = 1;

    ok &= check(f0922_custom_strcpy_compat(destination,
                                            sizeof(destination),
                                            "hello") == destination,
                "successful copy returns the original destination");
    ok &= check(memcmp(destination, "hello\0", sizeof(destination)) == 0,
                "successful copy includes the terminating NUL");

    memcpy(before, rejected, sizeof(rejected));
    ok &= check(f0922_custom_strcpy_compat(rejected,
                                            sizeof(rejected),
                                            "too long") == NULL,
                "undersized destination is rejected");
    ok &= check(memcmp(rejected, before, sizeof(rejected)) == 0,
                "undersized destination remains unchanged");

    ok &= check(f0922_custom_strcpy_compat(destination, 1u, "") == destination &&
                    destination[0] == '\0',
                "one-byte destination accepts the empty string");

    memcpy(destination, "guard", sizeof(destination));
    ok &= check(f0922_custom_strcpy_compat(destination, 0u, "x") == NULL &&
                    memcmp(destination, "guard", sizeof(destination)) == 0,
                "zero capacity is rejected without mutation");
    ok &= check(f0922_custom_strcpy_compat(NULL, 1u, "x") == NULL &&
                    f0922_custom_strcpy_compat(destination,
                                                sizeof(destination),
                                                NULL) == NULL,
                "null inputs are rejected");

    if (!ok) {
        return 1;
    }

    puts("PASS f0922_custom_strcpy_compat");
    return 0;
}
