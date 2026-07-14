#ifndef REDMCSB_F7062_SAVE_HEADER_PC34_COMPAT_H
#define REDMCSB_F7062_SAVE_HEADER_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#define REDMCSB_F7062_HEADER_BYTES 512U
#define REDMCSB_F7062_RANDOM_WORDS 127U

/* ReDMCSB CEDTINC6.C F7062/F0430, PC34 save-header write preparation. */

/*
 * random_words is the exact source RNG sequence consumed by F7062. The
 * encoded_header receives the bytes that F7062 writes; header's second half
 * is restored to plaintext before return. No file transport is modeled.
 */
int redmcsb_f7062_prepare_obfuscated_save_header_pc34(
    uint8_t *header, size_t header_size, uint16_t key_word_index,
    const uint16_t *random_words, size_t random_word_count,
    uint8_t *encoded_header, size_t encoded_header_size);

const char *redmcsb_f7062_save_header_pc34_source_evidence(void);

#endif
