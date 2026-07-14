/*
 * ReDMCSB DECOMPCO.C F0913_DecompressPAK, PC 3.4 APPA hook route.
 *
 * The original receives a 68000 big-endian PAK block: 0x5223 at byte 0,
 * its decompressed word count at byte 4, a 1920-word table at byte 8, and
 * the nibble stream at byte 3848.  APPA.C installs it in PRIM slot 25.
 */
#ifndef FIRESTAFF_REDMCSB_F0913_DECOMPRESS_PAK_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0913_DECOMPRESS_PAK_PC34_COMPAT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    REDMCSB_F0913_DECOMPRESS_PAK_OK_PC34 = 0,
    /* The source function's sole explicit failure result. */
    REDMCSB_F0913_DECOMPRESS_PAK_BAD_SIGNATURE_PC34 = -1,
    /* Explicit portable-host boundary failures; absent from the 68k ABI. */
    REDMCSB_F0913_DECOMPRESS_PAK_HOST_INVALID_ARGUMENT_PC34 = -2,
    REDMCSB_F0913_DECOMPRESS_PAK_HOST_TRUNCATED_INPUT_PC34 = -3,
    REDMCSB_F0913_DECOMPRESS_PAK_HOST_OUTPUT_TOO_SMALL_PC34 = -4
};

enum {
    REDMCSB_F0913_PAK_SIGNATURE_PC34 = 0x5223u,
    REDMCSB_F0913_PAK_WORD_TABLE_COUNT_PC34 = 1920u,
    REDMCSB_F0913_PAK_WORD_TABLE_OFFSET_PC34 = 8u,
    REDMCSB_F0913_PAK_COMPRESSED_DATA_OFFSET_PC34 = 3848u,
    REDMCSB_F0913_PRIM_DECOMPRESS_CODE_SEGMENT_SLOT_PC34 = 25u
};

/* Decodes exactly the word count encoded in the big-endian PAK header. */
int redmcsb_f0913_decompress_pak_pc34_compat(
    const uint8_t *pak_bytes,
    size_t pak_size,
    uint16_t *decompressed_words,
    size_t decompressed_word_capacity);

typedef int (*redmcsb_f0913_prim_decompress_callback_pc34_compat)(
    const uint8_t *pak_bytes,
    size_t pak_size,
    uint16_t *decompressed_words,
    size_t decompressed_word_capacity);

/*
 * Host representation of APPA.C's PRIM_04_Library_HookFunction call. The
 * host owns library-vector mutation; this adapter only asks it to replace
 * function 25 with the source-locked decompressor.
 */
typedef bool (*redmcsb_f0913_prim_hook_callback_pc34_compat)(
    void *context,
    unsigned int function_slot,
    redmcsb_f0913_prim_decompress_callback_pc34_compat replacement);

bool redmcsb_f0913_install_prim_decompress_hook_pc34_compat(
    redmcsb_f0913_prim_hook_callback_pc34_compat hook_callback,
    void *hook_context);

const char *redmcsb_f0913_decompress_pak_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_REDMCSB_F0913_DECOMPRESS_PAK_PC34_COMPAT_H */
