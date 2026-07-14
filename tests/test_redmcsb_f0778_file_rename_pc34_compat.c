#include "redmcsb_f0778_file_rename_pc34_compat.h"

typedef struct {
    int calls;
    const char *source_file_name;
    const char *destination_file_name;
} test_dos;

static void test_rename(void *context, const char *source_file_name,
                        const char *destination_file_name)
{
    test_dos *dos = context;

    dos->calls++;
    dos->source_file_name = source_file_name;
    dos->destination_file_name = destination_file_name;
}

int main(void)
{
    test_dos dos = {0, 0, 0};
    const char source_file_name[] = "DUNGEON.DAT";
    const char destination_file_name[] = "DUNGEON.BAK";

    redmcsb_f0778_file_rename_pc34_compat(
        test_rename, &dos, source_file_name, destination_file_name);

    if (dos.calls != 1 || dos.source_file_name != source_file_name ||
        dos.destination_file_name != destination_file_name) {
        return 1;
    }
    return 0;
}
