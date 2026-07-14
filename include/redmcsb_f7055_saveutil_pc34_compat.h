#ifndef REDMCSB_F7055_SAVEUTIL_PC34_COMPAT_H
#define REDMCSB_F7055_SAVEUTIL_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

/*
 * ReDMCSB CEDTINC6.C F7055/F7056/F7057/F7058.
 *
 * CSB's PC34 save utility operates on native 16-bit little-endian words.
 * These APIs take byte buffers so the original word order is stable on every
 * C11 host. They reject odd-sized sections; no byte-wise substitute exists in
 * the source routine.
 */

uint16_t redmcsb_f7055_saveutil_get_checksum_and_obfuscate_pc34(
    uint8_t *buffer, size_t byte_count, uint16_t key);

uint16_t redmcsb_f7056_saveutil_get_checksum_pc34(
    const uint8_t *buffer, size_t byte_count, uint16_t key);

/* F7057: decrypt in place and accept only the supplied source checksum. */
int redmcsb_f7057_read_save_part_with_checksum_pc34(
    uint8_t *buffer, size_t byte_count, uint16_t key, uint16_t checksum);

/* F7058: report the source checksum while leaving the caller's plaintext
 * unchanged, matching the original write-then-deobfuscate transaction. */
int redmcsb_f7058_write_save_part_with_checksum_pc34(
    uint8_t *buffer, size_t byte_count, uint16_t key, uint16_t *checksum);

const char *redmcsb_f7055_saveutil_source_evidence_pc34(void);

#endif
