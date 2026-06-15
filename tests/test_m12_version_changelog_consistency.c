#include "changelog_m12.h"

#include <stdio.h>
#include <string.h>

#ifndef FIRESTAFF_PROJECT_VERSION
#error "FIRESTAFF_PROJECT_VERSION must be provided by CMake"
#endif

static int fail(const char* message, const char* expected, const char* actual) {
    fprintf(stderr, "FAIL: %s\n", message);
    if (expected) {
        fprintf(stderr, "  expected: %s\n", expected);
    }
    if (actual) {
        fprintf(stderr, "  actual:   %s\n", actual);
    }
    return 1;
}

int main(void) {
    const char* version = M12_Changelog_VersionString();
    const char* line = NULL;
    char expectedLine[64];
    int i;

    if (!version || version[0] == '\0') {
        return fail("M12 changelog version string must be non-empty", NULL, version);
    }

    if (strcmp(version, FIRESTAFF_PROJECT_VERSION) != 0) {
        return fail("M12 changelog version must match CMake project version",
                    FIRESTAFF_PROJECT_VERSION,
                    version);
    }

    snprintf(expectedLine, sizeof(expectedLine), "V%s", FIRESTAFF_PROJECT_VERSION);
    for (i = 0; i < M12_Changelog_LineCount(); ++i) {
        const char* candidate = M12_Changelog_GetLine(i);
        if (candidate && strncmp(candidate, expectedLine, strlen(expectedLine)) == 0) {
            line = candidate;
            break;
        }
    }

    if (!line) {
        return fail("embedded changelog must include a top-level entry for the project version",
                    expectedLine,
                    NULL);
    }

    return 0;
}
