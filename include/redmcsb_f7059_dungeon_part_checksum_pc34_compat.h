#ifndef REDMCSB_F7059_DUNGEON_PART_CHECKSUM_PC34_COMPAT_H
#define REDMCSB_F7059_DUNGEON_PART_CHECKSUM_PC34_COMPAT_H

#include <stdint.h>

/* ReDMCSB CEDTINC6.C F7059/F7060, PC34 save-dungeon byte checksum. */

/*
 * The caller supplies the exact bytes that PC34 has already read or is about
 * to write. These routines deliberately do not model a host file transport.
 */
void redmcsb_f7059_read_dungeon_part_with_checksum_pc34(
    const uint8_t *buffer, uint16_t byte_count, uint16_t *checksum);

void redmcsb_f7060_write_dungeon_part_with_checksum_pc34(
    const uint8_t *buffer, uint16_t byte_count, uint16_t *checksum);

const char *redmcsb_f7059_dungeon_part_checksum_pc34_source_evidence(void);

#endif
