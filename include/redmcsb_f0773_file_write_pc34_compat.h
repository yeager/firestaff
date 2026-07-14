#ifndef REDMCSB_F0773_FILE_WRITE_PC34_COMPAT_H
#define REDMCSB_F0773_FILE_WRITE_PC34_COMPAT_H

#include <stdint.h>

/*
 * ReDMCSB FILE.C:551-584 writes PC-34 DOS files in 32 KiB chunks through
 * int 21h/AH=40h. The host supplies that DOS operation; this module has no
 * file-system implementation.
 */
struct RedmcsbF0773FileWriteResult {
    uint16_t transferred;
    int carry;
};

typedef struct RedmcsbF0773FileWriteResult (*RedmcsbF0773WriteCallback)(
    void *context,
    int16_t file_handle,
    const unsigned char *buffer,
    uint16_t byte_count);

int RedmcsbF0773FileWritePc34Compat(
    void *context,
    RedmcsbF0773WriteCallback write_callback,
    int16_t file_handle,
    unsigned long byte_count,
    const unsigned char *buffer);

#endif
