#include "redmcsb_f0776_file_create_pc34_compat.h"

#include <stdint.h>

typedef struct {
    int calls;
    const char *file_name;
    uint16_t attributes;
    int16_t file_handle;
    int succeeds;
} test_dos;

static bool test_create(void *context, const char *file_name,
                        uint16_t attributes, int16_t *file_handle)
{
    test_dos *dos = context;

    dos->calls++;
    dos->file_name = file_name;
    dos->attributes = attributes;
    *file_handle = dos->file_handle;
    return dos->succeeds != 0;
}

int main(void)
{
    test_dos dos = {0, 0, 0, INT16_C(31), 1};
    const char file_name[] = "SAVEDGAME.DAT";

    if (redmcsb_f0776_file_create_pc34_compat(test_create, &dos, file_name) !=
        INT16_C(31)) {
        return 1;
    }
    if (dos.calls != 1 || dos.file_name != file_name ||
        dos.attributes != REDMCSB_F0776_DOS_CREATE_ATTRIBUTES_PC34) {
        return 2;
    }

    dos.calls = 0;
    dos.succeeds = 0;
    dos.file_handle = INT16_C(7);
    if (redmcsb_f0776_file_create_pc34_compat(test_create, &dos, file_name) !=
        INT16_C(-1)) {
        return 3;
    }
    if (dos.calls != 1 || dos.attributes !=
                               REDMCSB_F0776_DOS_CREATE_ATTRIBUTES_PC34) {
        return 4;
    }
    return 0;
}
