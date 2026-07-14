#include "redmcsb_f0770_file_open_pc34_compat.h"

#include <stdint.h>

typedef struct {
    int calls;
    const char *file_name;
    uint8_t access_mode;
    int16_t file_handle;
    int succeeds;
} test_dos;

static bool test_open(void *context, const char *file_name, uint8_t access_mode,
                      int16_t *file_handle)
{
    test_dos *dos = context;

    dos->calls++;
    dos->file_name = file_name;
    dos->access_mode = access_mode;
    *file_handle = dos->file_handle;
    return dos->succeeds != 0;
}

int main(void)
{
    test_dos dos = {0, 0, 0, INT16_C(23), 1};
    const char file_name[] = "ENTER.SNG";

    if (redmcsb_f0770_file_open_pc34_compat(test_open, &dos, file_name) !=
        INT16_C(23)) {
        return 1;
    }
    if (dos.calls != 1 || dos.file_name != file_name ||
        dos.access_mode != REDMCSB_F0770_DOS_OPEN_READ_WRITE_PC34) {
        return 2;
    }

    dos.calls = 0;
    dos.succeeds = 0;
    dos.file_handle = INT16_C(7);
    if (redmcsb_f0770_file_open_pc34_compat(test_open, &dos, file_name) !=
        INT16_C(-1)) {
        return 3;
    }
    if (dos.calls != 1 || dos.access_mode !=
                               REDMCSB_F0770_DOS_OPEN_READ_WRITE_PC34) {
        return 4;
    }
    return 0;
}
