#ifndef NEXUS_V1_0DMSTRT_STRUCTURE_ADMISSION_H
#define NEXUS_V1_0DMSTRT_STRUCTURE_ADMISSION_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 0DMSTRT.BIN structure admission. The SHA-256-attested retail
 * 0DMSTRT.BIN (39516 bytes) carries no RES* framing; instead the
 * observed layout is a boot-library image with an exact arithmetic
 * partition that covers the whole source with zero gap:
 *   region A  [0x0000, 0x08a8)  dense bytes, 2102 non-zero bytes
 *   gap 1     [0x08a8, 0x3640)  11672 bytes, all zero
 *   region B  [0x3640, 0x9978)  dense bytes, 24077 non-zero bytes
 *   gap 2     [0x9978, 0x99cb)  83 bytes, all zero
 *   tail      [0x99cb, 0x99fc)  0xff separator byte, a 31-byte
 *                               printable-ASCII version stamp leading
 *                               with the 7-byte class tag "GFS_SBL",
 *                               a NUL terminator, byte 0x01, the
 *                               "CD001" standard identifier, and an
 *                               ISO-style "." / ".." directory-id stub
 *   fixup A   [0x99fc, 0x9a0c)  4 tagged entries
 *   gap 3     [0x9a0c, 0x9a2c)  32 bytes, all zero
 *   fixup B   [0x9a2c, 0x9a5c)  12 tagged entries, ending exactly at
 *                               the source size
 * plus a 7-entry tagged head table at [0x0058, 0x0074) inside
 * region A, preceded by a 0xffff sentinel word at 0x0056. Every
 * fixup entry is four bytes: a BE16 tag 0x0601 followed by a BE16
 * value; the tag repeats in all 23 entries and each value table is
 * canonical. The eight region lengths sum to the source size exactly.
 * This module binds those provenance facts only: no byte or word is
 * assigned instruction, code, data, relocation, address, or
 * execution meaning, and no load, relocation, or execution route is
 * permitted. */
#define NEXUS_V1_0DMSTRT_BYTES 39516U
#define NEXUS_V1_0DMSTRT_SHA256 \
    "8a026f155af27cfd43a33b29f7da5b75ee7b09b2c4f016fc3be1ebb4787d20b6"

#define NEXUS_V1_0DMSTRT_REGION_A_OFFSET 0x0U
#define NEXUS_V1_0DMSTRT_REGION_A_END 0x8a8U
#define NEXUS_V1_0DMSTRT_REGION_A_NONZERO 2102U
#define NEXUS_V1_0DMSTRT_GAP1_OFFSET 0x8a8U
#define NEXUS_V1_0DMSTRT_GAP1_LENGTH 11672U
#define NEXUS_V1_0DMSTRT_REGION_B_OFFSET 0x3640U
#define NEXUS_V1_0DMSTRT_REGION_B_END 0x9978U
#define NEXUS_V1_0DMSTRT_REGION_B_NONZERO 24077U
#define NEXUS_V1_0DMSTRT_GAP2_OFFSET 0x9978U
#define NEXUS_V1_0DMSTRT_GAP2_LENGTH 83U
#define NEXUS_V1_0DMSTRT_TAIL_OFFSET 0x99cbU
#define NEXUS_V1_0DMSTRT_TAIL_SEPARATOR 0xffU
#define NEXUS_V1_0DMSTRT_STAMP_OFFSET 0x99ccU
#define NEXUS_V1_0DMSTRT_STAMP_LENGTH 31U
#define NEXUS_V1_0DMSTRT_STAMP_TAG_BYTES 7U
#define NEXUS_V1_0DMSTRT_ISO_STUB_OFFSET 0x99ebU
#define NEXUS_V1_0DMSTRT_ISO_STUB_LENGTH 17U
#define NEXUS_V1_0DMSTRT_ISO_MAGIC_OFFSET 0x99edU
#define NEXUS_V1_0DMSTRT_ISO_DOT_OFFSET 0x99f4U
#define NEXUS_V1_0DMSTRT_ISO_DOTDOT_OFFSET 0x99f8U
#define NEXUS_V1_0DMSTRT_ISO_STUB_FF_COUNT 5U
#define NEXUS_V1_0DMSTRT_HEAD_SENTINEL_OFFSET 0x56U
#define NEXUS_V1_0DMSTRT_HEAD_SENTINEL 0xffffU
#define NEXUS_V1_0DMSTRT_HEAD_TABLE_OFFSET 0x58U
#define NEXUS_V1_0DMSTRT_HEAD_TABLE_COUNT 7U
#define NEXUS_V1_0DMSTRT_FIXUP_TAG 0x0601U
#define NEXUS_V1_0DMSTRT_TABLE_A_OFFSET 0x99fcU
#define NEXUS_V1_0DMSTRT_TABLE_A_COUNT 4U
#define NEXUS_V1_0DMSTRT_GAP3_OFFSET 0x9a0cU
#define NEXUS_V1_0DMSTRT_GAP3_LENGTH 32U
#define NEXUS_V1_0DMSTRT_TABLE_B_OFFSET 0x9a2cU
#define NEXUS_V1_0DMSTRT_TABLE_B_COUNT 12U
#define NEXUS_V1_0DMSTRT_FIXUP_TOTAL_COUNT \
    (NEXUS_V1_0DMSTRT_HEAD_TABLE_COUNT + NEXUS_V1_0DMSTRT_TABLE_A_COUNT + \
     NEXUS_V1_0DMSTRT_TABLE_B_COUNT)
#define NEXUS_V1_0DMSTRT_REGION_SPAN_COUNT 8U

/* Live source identity. sha256_verified/sha256_hex attest the pinned
 * canonical asset class; source_fnv1a64 binds the exact live bytes
 * (real path or synthetic mirror). */
typedef struct {
    int sha256_verified;
    const char *sha256_hex;
    uint64_t source_fnv1a64;
} Nexus_V1_0DmstrtSourceIdentity;

typedef struct {
    int valid;
    int source_identity_bound;
    int size_arithmetic_bound;
    int region_a_bound;
    int gap1_zero_bound;
    int region_b_bound;
    int gap2_zero_bound;
    int tail_descriptor_bound;
    int iso_stub_bound;
    int head_table_bound;
    int fixup_table_a_bound;
    int gap3_zero_bound;
    int fixup_table_b_bound;
    int regions_cover_source;
    int code_proven;
    int data_proven;
    int relocation_proven;
    int load_permitted;
    int execution_permitted;
    int presentation_permitted;
    uint64_t source_fnv1a64;
    uint32_t region_a_nonzero;
    uint32_t region_b_nonzero;
    uint64_t region_a_fnv1a64;
    uint64_t region_b_fnv1a64;
    /* Live digests of the 8 raw partition spans in file order
     * (region A, gap 1, region B, gap 2, tail descriptor, fixup A,
     * gap 3, fixup B); they bind no span semantics. */
    uint64_t region_span_fnv1a64[NEXUS_V1_0DMSTRT_REGION_SPAN_COUNT];
} Nexus_V1_0DmstrtStructureReceipt;

typedef struct {
    uint32_t source_offset;
    uint32_t source_length;
    uint64_t source_fnv1a64;
} Nexus_V1_0DmstrtRegionSpan;

typedef struct {
    Nexus_V1_0DmstrtStructureReceipt receipt;
    uint32_t emitted;
} Nexus_V1_0DmstrtRegionSpanIterator;

/* Admits the whole 0DMSTRT.BIN source against the pinned identity and
 * the canonical partition: exact size, both dense-region non-zero
 * counts, all three zero gaps, the tail descriptor (separator, class
 * tag, printable stamp, NUL terminator, "CD001" identifier, "." and
 * ".." stub ids, 0xff population), the head sentinel, and all 23
 * tagged fixup entries with their canonical value tables, including
 * the exact source-end coverage of fixup table B. Returns 1 only for
 * a fully matching receipt, otherwise 0. */
int nexus_v1_0dmstrt_structure_admit(
    const uint8_t *source_bytes,
    size_t source_size,
    const Nexus_V1_0DmstrtSourceIdentity *identity,
    Nexus_V1_0DmstrtStructureReceipt *out_receipt);

int nexus_v1_0dmstrt_region_span_iterator_init(
    Nexus_V1_0DmstrtRegionSpanIterator *iterator,
    const Nexus_V1_0DmstrtStructureReceipt *receipt);

/* Returns 1 for each of the 8 raw partition spans in file order
 * (region A, gap 1, region B, gap 2, tail descriptor, fixup A,
 * gap 3, fixup B), 0 at end, -1 on invalid arguments/receipt. The
 * spans are bounded raw ranges only; their lengths sum to the source
 * size and the iterator never decodes instructions or data. */
int nexus_v1_0dmstrt_region_span_iterator_next(
    Nexus_V1_0DmstrtRegionSpanIterator *iterator,
    Nexus_V1_0DmstrtRegionSpan *out_span);

#ifdef __cplusplus
}
#endif

#endif
