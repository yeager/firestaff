#include "redmcsb_f0773_file_write_pc34_compat.h"

int RedmcsbF0773FileWritePc34Compat(
    void *context,
    RedmcsbF0773WriteCallback write_callback,
    int16_t file_handle,
    unsigned long byte_count,
    const unsigned char *buffer)
{
    while (byte_count > 0UL) {
        uint16_t chunk_size =
            byte_count > 32768UL ? UINT16_C(32768) : (uint16_t)byte_count;
        struct RedmcsbF0773FileWriteResult result =
            write_callback(context, file_handle, buffer, chunk_size);

        if (result.carry) {
            return 0;
        }
        if (result.transferred != chunk_size) {
            return 0;
        }
        byte_count -= (unsigned long)chunk_size;
        buffer += chunk_size;
    }

    return 1;
}
