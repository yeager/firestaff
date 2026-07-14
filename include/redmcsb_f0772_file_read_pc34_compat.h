#ifndef REDMCSB_F0772_FILE_READ_PC34_COMPAT_H
#define REDMCSB_F0772_FILE_READ_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

/*
 * ReDMCSB FILE.C:499-549 (I34E/I34M) reads DOS files in 32 KiB chunks.
 * The host supplies the DOS read operation; this module never opens files.
 */
typedef size_t (*RedmcsbF0772ReadCallback)(
    void *context,
    int16_t file_handle,
    unsigned char *buffer,
    uint16_t byte_count);

int RedmcsbF0772FileReadPc34Compat(
    void *context,
    RedmcsbF0772ReadCallback read_callback,
    int16_t file_handle,
    unsigned long byte_count,
    unsigned char *buffer);

#endif
