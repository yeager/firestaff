#ifndef FIRESTAFF_CSB_V1_ATARI_MSA_H
#define FIRESTAFF_CSB_V1_ATARI_MSA_H

#include <stddef.h>
#include <stdint.h>

/* Magic Shadow Archiver (MSA) is the Atari ST disk-image container used by
 * the retail CSB save disk. Its big-endian header and per-track RLE are
 * documented in Atari Image File Formats, p. 3. This reader exposes only
 * files physically present in the decoded GEMDOS root directory. */

typedef struct {
    uint16_t sectors_per_track;
    uint16_t side_count;
    uint16_t first_track;
    uint16_t last_track;
    uint32_t decoded_disk_bytes;
    uint16_t root_file_count;
} CSB_V1_AtariMsaReceipt;

/* Validate and fully decode an MSA image without exposing a mutable disk
 * buffer. Useful for scanner and corpus admission before a particular file is
 * requested. */
int csb_v1_atari_msa_probe(const uint8_t *msa, size_t msa_size,
                           CSB_V1_AtariMsaReceipt *out_receipt);

/* Read one 8.3 root-directory file from a complete MSA image. `name` may be
 * written as "CSBGAME.DAT". With out_bytes == NULL, out_size receives the
 * exact file length. A malformed image, non-root file, FAT loop, or missing
 * name is rejected. */
int csb_v1_atari_msa_extract_root_file(const uint8_t *msa, size_t msa_size,
                                       const char *name,
                                       uint8_t *out_bytes, size_t out_capacity,
                                       size_t *out_size,
                                       CSB_V1_AtariMsaReceipt *out_receipt);

#endif /* FIRESTAFF_CSB_V1_ATARI_MSA_H */
