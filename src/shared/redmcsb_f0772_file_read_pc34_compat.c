#include "redmcsb_f0772_file_read_pc34_compat.h"

int RedmcsbF0772FileReadPc34Compat(
    void *context,
    RedmcsbF0772ReadCallback read_callback,
    int16_t file_handle,
    unsigned long byte_count,
    unsigned char *buffer)
{
    if (read_callback == NULL || (byte_count != 0UL && buffer == NULL)) {
        return 0;
    }

    while (byte_count > 0UL) {
        uint16_t chunk_size =
            byte_count > 32768UL ? UINT16_C(32768) : (uint16_t)byte_count;

        if (read_callback(context, file_handle, buffer, chunk_size) !=
            (size_t)chunk_size) {
            return 0;
        }

        byte_count -= (unsigned long)chunk_size;
        buffer += chunk_size;
    }

    return 1;
}
