#include "redmcsb_f0771_file_close_pc34_compat.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef struct test_dos {
    int calls;
    int16_t file_handle;
    int ignored_status;
} test_dos;

static void test_close(void *context, int16_t file_handle)
{
    test_dos *dos = context;

    ++dos->calls;
    dos->file_handle = file_handle;
    dos->ignored_status = 1;
}

static int check(int condition, const char *label)
{
    if (condition) {
        return 0;
    }
    fprintf(stderr, "failed: %s\n", label);
    return 1;
}

int main(void)
{
    test_dos dos = { 0, 0, 0 };

    redmcsb_f0771_file_close_pc34_compat(test_close, &dos, INT16_C(-123));
    if (check(dos.calls == 1, "issues one DOS close") ||
        check(dos.file_handle == INT16_C(-123), "passes signed DOS handle") ||
        check(dos.ignored_status == 1, "does not expose DOS status") ||
        check(strstr(redmcsb_f0771_file_close_source_evidence_pc34(),
                     "FILE.C:490-497") != NULL,
              "records source evidence")) {
        return 1;
    }

    puts("ok: ReDMCSB F0771 PC 3.4 file close");
    return 0;
}
