#include "redmcsb_f0777_file_delete_pc34_compat.h"

typedef struct {
    int calls;
    const char *file_name;
} test_dos;

static void test_delete(void *context, const char *file_name)
{
    test_dos *dos = context;

    dos->calls++;
    dos->file_name = file_name;
}

int main(void)
{
    test_dos dos = {0, 0};
    const char file_name[] = "SAVEDGAME.DAT";

    redmcsb_f0777_file_delete_pc34_compat(test_delete, &dos, file_name);
    if (dos.calls != 1 || dos.file_name != file_name) {
        return 1;
    }
    return 0;
}
