#ifndef REDMCSB_F7061_SAVE_HEADER_PC34_COMPAT_H
#define REDMCSB_F7061_SAVE_HEADER_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

/* ReDMCSB CEDTINC6.C F7061/F0429, PC34 512-byte save-header check. */

/*
 * Deobfuscates header bytes 256..511 in place before returning the original
 * checksum verdict. A failed checksum therefore still leaves that half
 * deobfuscated, exactly as F7061 does. No save layout is decoded here.
 */
int redmcsb_f7061_is_read_save_header_successful_pc34(
    uint8_t *header, size_t header_size, uint16_t key_word_index);

const char *redmcsb_f7061_save_header_pc34_source_evidence(void);

#endif
