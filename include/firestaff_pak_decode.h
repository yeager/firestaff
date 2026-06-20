/*
 * firestaff_pak_decode.h
 *
 * Atari ST PAK file decoder.
 *
 * START.PAK is the main executable wrapper for Atari ST
 * Dungeon Master (1987) and Chaos Strikes Back (1989).
 * The file is a proprietary FTL-compressed Atari ST
 * executable: a 4-byte PAK header + a 28-byte Atari ST
 * executable header + a 1920-word "most-frequent-words"
 * table + nibble-coded compressed code.
 *
 * The compressed code uses the same algorithm as the FTL
 * HUNK_CODE part (see docs/DMWEB_REFERENCE.md and
 * greatstone d_ftl.html / d_pak.html). Each nibble of
 * the compressed code selects one of three encodings:
 *
 *   nibble = 0xF (literal escape):
 *     next 4 nibbles are taken as 2 literal bytes
 *
 *   nibble >= 0x8 (8..E) (long dictionary reference):
 *     next 2 nibbles + this nibble form a 12-bit value
 *     between 2048 and 3839 (inclusive)
 *     word_index = value - 1920   -> 128..1919
 *     output = HIGH(most_frequent_words[word_index])
 *     output = LOW(most_frequent_words[word_index])
 *
 *   nibble < 0x8 (0..7) (short dictionary reference):
 *     next nibble + this nibble form an 8-bit value
 *     between 0 and 127
 *     output = HIGH(most_frequent_words[value])
 *     output = LOW(most_frequent_words[value])
 *
 * Total output bytes = uncompressed text-segment size from
 * the Atari ST header. The iteration count comes from the
 * PAK header: (file_size_in_words * 2) - 28.
 *
 * Provenance:
 *   - Spec: greatstone d_pak.html (DM/CSB technical doc)
 *   - Reference impl: ReDMCSB DECOMPCO.C F0913_DecompressPAK
 *   - Same algorithm: greatstone d_ftl.html (HUNK_CODE)
 *
 * Scope:
 *   * Read-only decoder. We never WRITE Atari ST executables.
 *   * Validates magic (0x601A) and bounds before decoding.
 *   * Does NOT execute the decoded Atari ST code; we only
 *     extract the raw 68000 machine code + relocation table
 *     for static analysis / asset extraction.
 *   * For DM/CSB execution we already use ReDMCSB as a
 *     source-locked reference and never launch START.PAK
 *     directly; this decoder exists for static asset
 *     analysis (e.g. understanding which icons are
 *     referenced by which code addresses) and as a step
 *     toward a future Atari ST emulator integration.
 */

#ifndef FIRESTAFF_PAK_DECODE_H
#define FIRESTAFF_PAK_DECODE_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Magic value of Atari ST executable header (big-endian on disk
 * for Atari ST PAK, since Atari ST is big-endian). */
#define FIRESTAFF_PAK_ATARI_MAGIC     0x601A

/* Most-frequent-words table size, in 16-bit words. The first
 * 128 entries are the most-frequently used words; the next
 * 1792 are less-frequent. */
#define FIRESTAFF_PAK_FREQ_TABLE_WORDS 1920

/* Size of the Atari ST executable header, in bytes (28). */
#define FIRESTAFF_PAK_ATARI_HEADER_BYTES 28

/* Decoded Atari ST executable header. All multi-byte fields
 * are big-endian as read from the PAK file (Atari ST is a
 * 68000 big-endian platform). */
typedef struct {
    uint32_t file_size_words;        /* PAK header: file size / 2 */
    uint16_t magic;                  /* should be 0x601A */
    uint32_t text_size;              /* uncompressed text segment size */
    uint32_t data_size;              /* data segment size */
    uint32_t bss_size;               /* bss segment size */
    uint32_t symbol_table_size;
    uint32_t reserved;
    uint32_t flags;                  /* PF_FASTLOAD etc. */
    uint16_t abs_flag;               /* 0 = relocatable */
} FirestaffPakHeader;

/* Decoded output from FirestaffPak_Decode. */
typedef struct {
    uint8_t* text;                   /* uncompressed text segment (code) */
    size_t   text_size;
    /* NOTE: data/bss/symbol-table segments are stored after
     * text in the file; we do not currently extract them
     * because they are loaded into RAM at runtime by the
     * Atari ST itself, not interpreted as assets. */
} FirestaffPakDecoded;

/*
 * Parse the PAK + Atari ST headers without decoding.
 *
 * Returns 0 on success, -1 on failure (truncated input,
 * bad magic, allocation failure). On success, fills
 * *out_header.
 */
int FirestaffPak_ReadHeader(const uint8_t* data, size_t data_size,
                             FirestaffPakHeader* out_header);

/*
 * Decode a PAK file to a flat buffer containing the
 * uncompressed Atari ST text segment.
 *
 * Returns 0 on success and fills *out (caller must
 * FirestaffPak_Free when done). Returns -1 on failure
 * (truncated input, bad magic, allocation failure, or
 * the decoded output would exceed text_size).
 *
 * On success, out->text_size == header.text_size.
 *
 * The decoder validates every nibble boundary against
 * data_size and every write against text_size, so a
 * hostile or corrupt PAK cannot read or write out of
 * bounds.
 */
int FirestaffPak_Decode(const uint8_t* data, size_t data_size,
                         FirestaffPakDecoded* out);

/* Release the buffers held by *decoded. Safe on a zero-
 * initialized struct. */
void FirestaffPak_Free(FirestaffPakDecoded* decoded);

/*
 * Run a series of self-tests covering the three nibble
 * encodings, malformed inputs, and zero-length edge
 * cases. Returns 0 on success, -1 on first failure.
 */
int FirestaffPak_SelfTest(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_PAK_DECODE_H */
