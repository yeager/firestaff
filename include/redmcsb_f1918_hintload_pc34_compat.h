#ifndef REDMCSB_F1918_HINTLOAD_PC34_COMPAT_H
#define REDMCSB_F1918_HINTLOAD_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

/* ReDMCSB HINTLOAD.C F1910/F1913/F1914/F1918.
 *
 * This is a byte-transport boundary for the original five-part save format.
 * F1918 first consumes the header and then GLOBAL_DATA, ACTIVE_GROUP and
 * PARTY in that order.  Their record sizes are media-specific C structs, so
 * their already-admitted exact sizes belong to the caller.  No layout, seek,
 * allocation, key, checksum, DSA tail, or fallback record is inferred here. */

#define REDMCSB_F1918_PC34_HEADER_BYTES 512U
#define REDMCSB_F1918_PC34_PART_COUNT 3U
#define REDMCSB_F1918_PC34_HEADER_KEY_WORD_INDEX 29U
#define REDMCSB_F1918_PC34_HEADER_KEYS_OFFSET 312U
#define REDMCSB_F1918_PC34_HEADER_CHECKSUMS_OFFSET 344U

enum {
    REDMCSB_F1918_PC34_RESULT_OK = 100,
    REDMCSB_F1918_PC34_RESULT_HEADER_READ_FAILED = 213,
    REDMCSB_F1918_PC34_RESULT_GLOBAL_DATA_FAILED = 214,
    REDMCSB_F1918_PC34_RESULT_ACTIVE_GROUPS_FAILED = 215,
    REDMCSB_F1918_PC34_RESULT_PARTY_FAILED = 216
};

typedef int (*RedmcsbF1910ReadExactPc34)(void *context, uint8_t *destination,
                                          size_t byte_count);

typedef struct {
    uint8_t *bytes;
    size_t byte_count;
} RedmcsbF1918SavePartPc34;

typedef struct {
    uint8_t header[REDMCSB_F1918_PC34_HEADER_BYTES];
    RedmcsbF1918SavePartPc34 parts[REDMCSB_F1918_PC34_PART_COUNT];
    uint16_t keys[REDMCSB_F1918_PC34_PART_COUNT];
    uint16_t checksums[REDMCSB_F1918_PC34_PART_COUNT];
    int header_valid;
    unsigned int parts_loaded;
} RedmcsbF1918LoadReceiptPc34;

/* F1910: exact source transport.  A zero-byte request succeeds without
 * invoking the reader, exactly as the source does. */
int redmcsb_f1910_load_saved_game_part_pc34(RedmcsbF1910ReadExactPc34 read,
                                            void *context,
                                            uint8_t *destination,
                                            size_t byte_count);

/* F1913: exact read then F7055 in-place deobfuscation/checksum.  A failed
 * checksum leaves destination deobfuscated, matching the source sequence. */
int redmcsb_f1913_load_and_deobfuscate_saved_game_part_pc34(
    RedmcsbF1910ReadExactPc34 read, void *context, uint8_t *destination,
    size_t byte_count, uint16_t key, uint16_t checksum);

/* F1914: source header read and 512-byte ReDMCSB header validation. */
int redmcsb_f1914_load_and_deobfuscate_saved_game_header_pc34(
    RedmcsbF1910ReadExactPc34 read, void *context,
    uint8_t header[REDMCSB_F1918_PC34_HEADER_BYTES]);

/* F1918's source-defined initial load sequence.  The caller supplies exact
 * original record spans for GLOBAL_DATA, ACTIVE_GROUP and PARTY. */
int redmcsb_f1918_load_initial_save_parts_pc34(
    RedmcsbF1910ReadExactPc34 read, void *context,
    RedmcsbF1918LoadReceiptPc34 *receipt);

const char *redmcsb_f1918_hintload_pc34_source_evidence(void);

#endif
